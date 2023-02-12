/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "threadexecutor.h"

#include "color.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errorlogger.h"
#include "importproject.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <numeric>
#include <mutex>
#include <system_error>
#include <utility>
#include <vector>

ThreadExecutor::ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : Executor(files, settings, errorLogger)
{}

ThreadExecutor::~ThreadExecutor()
{}

class ThreadExecutor::Data
{
public:
    Data(const ThreadExecutor &threadExecutor) : mProcessedFiles(0), mTotalFiles(0), mProcessedSize(0)
    {
        const std::map<std::string, std::size_t>& files = threadExecutor.mFiles;
        mItNextFile = files.begin();
        mItNextFileSettings = threadExecutor.mSettings.project.fileSettings.begin();

        mTotalFiles = files.size() + threadExecutor.mSettings.project.fileSettings.size();
        mTotalFileSize = std::accumulate(files.cbegin(), files.cend(), std::size_t(0), [](std::size_t v, const std::pair<std::string, std::size_t>& p) {
            return v + p.second;
        });
    }

    std::map<std::string, std::size_t>::const_iterator mItNextFile;
    std::list<ImportProject::FileSettings>::const_iterator mItNextFileSettings;

    std::size_t mProcessedFiles;
    std::size_t mTotalFiles;
    std::size_t mProcessedSize;
    std::size_t mTotalFileSize;

    std::mutex mFileSync;
};

class ThreadExecutor::SyncLogForwarder : public ErrorLogger
{
public:
    explicit SyncLogForwarder(ThreadExecutor &threadExecutor)
        : mThreadExecutor(threadExecutor) {}

    void reportOut(const std::string &outmsg, Color c) override
    {
        std::lock_guard<std::mutex> lg(mReportSync);

        mThreadExecutor.mErrorLogger.reportOut(outmsg, c);
    }

    void reportErr(const ErrorMessage &msg) override {
        report(msg, MessageType::REPORT_ERROR);
    }

    void reportInfo(const ErrorMessage &msg) override {
        report(msg, MessageType::REPORT_INFO);
    }

    ThreadExecutor &mThreadExecutor;

    std::mutex mErrorSync;
    std::mutex mReportSync;

private:
    enum class MessageType {REPORT_ERROR, REPORT_INFO};

    void report(const ErrorMessage &msg, MessageType msgType)
    {
        if (mThreadExecutor.mSettings.nomsg.isSuppressed(msg))
            return;

        // Alert only about unique errors
        bool reportError = false;

        {
            std::string errmsg = msg.toString(mThreadExecutor.mSettings.verbose);

            std::lock_guard<std::mutex> lg(mErrorSync);
            if (std::find(mThreadExecutor.mErrorList.cbegin(), mThreadExecutor.mErrorList.cend(), errmsg) == mThreadExecutor.mErrorList.cend()) {
                mThreadExecutor.mErrorList.emplace_back(std::move(errmsg));
                reportError = true;
            }
        }

        if (reportError) {
            std::lock_guard<std::mutex> lg(mReportSync);

            switch (msgType) {
            case MessageType::REPORT_ERROR:
                mThreadExecutor.mErrorLogger.reportErr(msg);
                break;
            case MessageType::REPORT_INFO:
                mThreadExecutor.mErrorLogger.reportInfo(msg);
                break;
            }
        }
    }
};

unsigned int ThreadExecutor::check()
{
    std::vector<std::future<unsigned int>> threadFutures;
    threadFutures.reserve(mSettings.jobs);

    Data data(*this);
    SyncLogForwarder logforwarder(*this);

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        try {
            threadFutures.emplace_back(std::async(std::launch::async, threadProc, &data, &logforwarder));
        }
        catch (const std::system_error &e) {
            std::cerr << "#### ThreadExecutor::check exception :" << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return std::accumulate(threadFutures.begin(), threadFutures.end(), 0U, [](unsigned int v, std::future<unsigned int>& f) {
        return v + f.get();
    });
}

unsigned int STDCALL ThreadExecutor::threadProc(Data *data, SyncLogForwarder* logForwarder)
{
    unsigned int result = 0;

    std::map<std::string, std::size_t>::const_iterator &itFile = data->mItNextFile;
    std::list<ImportProject::FileSettings>::const_iterator &itFileSettings = data->mItNextFileSettings;

    // guard static members of CppCheck against concurrent access
    data->mFileSync.lock();

    for (;;) {
        if (itFile == logForwarder->mThreadExecutor.mFiles.cend() && itFileSettings == logForwarder->mThreadExecutor.mSettings.project.fileSettings.cend()) {
            data->mFileSync.unlock();
            break;
        }

        CppCheck fileChecker(*logForwarder, false, CppCheckExecutor::executeCommand);
        fileChecker.settings() = logForwarder->mThreadExecutor.mSettings;

        std::size_t fileSize = 0;
        if (itFile != logForwarder->mThreadExecutor.mFiles.end()) {
            const std::string &file = itFile->first;
            fileSize = itFile->second;
            ++itFile;

            data->mFileSync.unlock();

            // Read file from a file
            result += fileChecker.check(file);
        } else { // file settings..
            const ImportProject::FileSettings &fs = *itFileSettings;
            ++itFileSettings;
            data->mFileSync.unlock();
            result += fileChecker.check(fs);
            if (logForwarder->mThreadExecutor.mSettings.clangTidy)
                fileChecker.analyseClangTidy(fs);
        }

        data->mFileSync.lock();

        data->mProcessedSize += fileSize;
        data->mProcessedFiles++;
        if (!logForwarder->mThreadExecutor.mSettings.quiet) {
            std::lock_guard<std::mutex> lg(logForwarder->mReportSync);
            CppCheckExecutor::reportStatus(data->mProcessedFiles, data->mTotalFiles, data->mProcessedSize, data->mTotalFileSize);
        }
    }
    return result;
}

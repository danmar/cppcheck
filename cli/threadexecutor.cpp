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
#include "config.h"
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

class Data
{
public:
    Data(const std::map<std::string, std::size_t> &files, const std::list<ImportProject::FileSettings> &fileSettings)
        : mFiles(files), mFileSettings(fileSettings), mProcessedFiles(0), mProcessedSize(0)
    {
        mItNextFile = mFiles.begin();
        mItNextFileSettings = mFileSettings.begin();

        mTotalFiles = mFiles.size() + mFileSettings.size();
        mTotalFileSize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const std::pair<std::string, std::size_t>& p) {
            return v + p.second;
        });
    }

    bool finished() {
        std::lock_guard<std::mutex> l(mFileSync);
        return mItNextFile == mFiles.cend() && mItNextFileSettings == mFileSettings.cend();
    }

    bool next(const std::string *&file, const ImportProject::FileSettings *&fs, std::size_t &fileSize) {
        std::lock_guard<std::mutex> l(mFileSync);
        if (mItNextFile != mFiles.end()) {
            file = &mItNextFile->first;
            fileSize = mItNextFile->second;
            ++mItNextFile;
            return true;
        }
        if (mItNextFileSettings != mFileSettings.end()) {
            fs = &(*mItNextFileSettings);
            fileSize = 0;
            ++mItNextFileSettings;
            return true;
        }

        return false;
    }

private:
    const std::map<std::string, std::size_t> &mFiles;
    std::map<std::string, std::size_t>::const_iterator mItNextFile;
    const std::list<ImportProject::FileSettings> &mFileSettings;
    std::list<ImportProject::FileSettings>::const_iterator mItNextFileSettings;

public:
    std::size_t mProcessedFiles;
    std::size_t mTotalFiles;
    std::size_t mProcessedSize;
    std::size_t mTotalFileSize;

    std::mutex mFileSync;
};

class SyncLogForwarder : public ErrorLogger
{
public:
    explicit SyncLogForwarder(ThreadExecutor &threadExecutor, ErrorLogger &errorLogger)
        : mThreadExecutor(threadExecutor), mErrorLogger(errorLogger) {}

    void reportOut(const std::string &outmsg, Color c) override
    {
        std::lock_guard<std::mutex> lg(mReportSync);

        mErrorLogger.reportOut(outmsg, c);
    }

    void reportErr(const ErrorMessage &msg) override {
        if (!mThreadExecutor.hasToLog(msg))
            return;

        std::lock_guard<std::mutex> lg(mReportSync);
        mErrorLogger.reportErr(msg);
    }

    std::mutex mReportSync;

private:
    ThreadExecutor &mThreadExecutor;
    ErrorLogger &mErrorLogger;
};

static unsigned int STDCALL threadProc(Data *data, SyncLogForwarder* logForwarder, const Settings &settings)
{
    unsigned int result = 0;

    for (;;) {
        if (data->finished()) {
            break;
        }

        const std::string *file = nullptr;
        const ImportProject::FileSettings *fs = nullptr;
        std::size_t fileSize;
        if (!data->next(file, fs, fileSize))
            break;

        CppCheck fileChecker(*logForwarder, false, CppCheckExecutor::executeCommand);
        fileChecker.settings() = settings;

        if (fs) {
            // file settings..
            result += fileChecker.check(*fs);
            if (settings.clangTidy)
                fileChecker.analyseClangTidy(*fs);
        } else {
            // Read file from a file
            result += fileChecker.check(*file);
        }

        {
            std::lock_guard<std::mutex> l(data->mFileSync);
            data->mProcessedSize += fileSize;
            data->mProcessedFiles++;
            if (!settings.quiet) {
                std::lock_guard<std::mutex> lg(logForwarder->mReportSync);
                CppCheckExecutor::reportStatus(data->mProcessedFiles, data->mTotalFiles, data->mProcessedSize, data->mTotalFileSize);
            }
        }
    }
    return result;
}

unsigned int ThreadExecutor::check()
{
    std::vector<std::future<unsigned int>> threadFutures;
    threadFutures.reserve(mSettings.jobs);

    Data data(mFiles, mSettings.project.fileSettings);
    SyncLogForwarder logforwarder(*this, mErrorLogger);

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        try {
            threadFutures.emplace_back(std::async(std::launch::async, &threadProc, &data, &logforwarder, mSettings));
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

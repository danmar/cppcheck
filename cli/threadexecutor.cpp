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

#include "config.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errorlogger.h"
#include "importproject.h"
#include "settings.h"

#include <algorithm>
#include <cassert>
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

enum class Color;

ThreadExecutor::ThreadExecutor(const std::map<std::string, std::size_t> &files, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger)
    : Executor(files, settings, suppressions, errorLogger)
{
    assert(mSettings.jobs > 1);
}

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

    void reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal) {
        std::lock_guard<std::mutex> lg(mReportSync);
        mThreadExecutor.reportStatus(fileindex, filecount, sizedone, sizetotal);
    }

private:
    std::mutex mReportSync;
    ThreadExecutor &mThreadExecutor;
    ErrorLogger &mErrorLogger;
};

class ThreadData
{
public:
    ThreadData(ThreadExecutor &threadExecutor, ErrorLogger &errorLogger, const Settings &settings, const std::map<std::string, std::size_t> &files, const std::list<ImportProject::FileSettings> &fileSettings)
        : mFiles(files), mFileSettings(fileSettings), mSettings(settings), logForwarder(threadExecutor, errorLogger)
    {
        mItNextFile = mFiles.begin();
        mItNextFileSettings = mFileSettings.begin();

        mTotalFiles = mFiles.size() + mFileSettings.size();
        mTotalFileSize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const std::pair<std::string, std::size_t>& p) {
            return v + p.second;
        });
    }

    bool next(const std::string *&file, const ImportProject::FileSettings *&fs, std::size_t &fileSize) {
        std::lock_guard<std::mutex> l(mFileSync);
        if (mItNextFile != mFiles.end()) {
            file = &mItNextFile->first;
            fs = nullptr;
            fileSize = mItNextFile->second;
            ++mItNextFile;
            return true;
        }
        if (mItNextFileSettings != mFileSettings.end()) {
            file = nullptr;
            fs = &(*mItNextFileSettings);
            fileSize = 0;
            ++mItNextFileSettings;
            return true;
        }

        return false;
    }

    unsigned int check(ErrorLogger &errorLogger, const std::string *file, const ImportProject::FileSettings *fs) const {
        CppCheck fileChecker(errorLogger, false, CppCheckExecutor::executeCommand);
        fileChecker.settings() = mSettings; // this is a copy

        unsigned int result;
        if (fs) {
            // file settings..
            result = fileChecker.check(*fs);
            if (fileChecker.settings().clangTidy)
                fileChecker.analyseClangTidy(*fs);
        } else {
            // Read file from a file
            result = fileChecker.check(*file);
            // TODO: call analyseClangTidy()?
        }
        return result;
    }

    void status(std::size_t fileSize) {
        std::lock_guard<std::mutex> l(mFileSync);
        mProcessedSize += fileSize;
        mProcessedFiles++;
        if (!mSettings.quiet)
            logForwarder.reportStatus(mProcessedFiles, mTotalFiles, mProcessedSize, mTotalFileSize);
    }

private:
    const std::map<std::string, std::size_t> &mFiles;
    std::map<std::string, std::size_t>::const_iterator mItNextFile;
    const std::list<ImportProject::FileSettings> &mFileSettings;
    std::list<ImportProject::FileSettings>::const_iterator mItNextFileSettings;

    std::size_t mProcessedFiles{};
    std::size_t mTotalFiles{};
    std::size_t mProcessedSize{};
    std::size_t mTotalFileSize{};

    std::mutex mFileSync;
    const Settings &mSettings;

public:
    SyncLogForwarder logForwarder;
};

static unsigned int STDCALL threadProc(ThreadData *data)
{
    unsigned int result = 0;

    const std::string *file;
    const ImportProject::FileSettings *fs;
    std::size_t fileSize;

    while (data->next(file, fs, fileSize)) {
        result += data->check(data->logForwarder, file, fs);

        data->status(fileSize);
    }

    return result;
}

unsigned int ThreadExecutor::check()
{
    std::vector<std::future<unsigned int>> threadFutures;
    threadFutures.reserve(mSettings.jobs);

    ThreadData data(*this, mErrorLogger, mSettings, mFiles, mSettings.project.fileSettings);

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        try {
            threadFutures.emplace_back(std::async(std::launch::async, &threadProc, &data));
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

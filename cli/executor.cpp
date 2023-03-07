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

#include "executor.h"

#include "color.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errorlogger.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <sstream> // IWYU pragma: keep
#include <utility>

Executor::Executor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : mFiles(files), mSettings(settings), mErrorLogger(errorLogger)
{}

Executor::~Executor()
{}

bool Executor::hasToLog(const ErrorMessage &msg)
{
    if (!mSettings.nomsg.isSuppressed(msg))
    {
        std::string errmsg = msg.toString(mSettings.verbose);

        std::lock_guard<std::mutex> lg(mErrorListSync);
        if (std::find(mErrorList.cbegin(), mErrorList.cend(), errmsg) == mErrorList.cend()) {
            mErrorList.emplace_back(std::move(errmsg));
            return true;
        }
    }
    return false;
}

class SyncLogForwarder : public ErrorLogger
{
public:
    explicit SyncLogForwarder(Executor &executor, ErrorLogger &errorLogger)
        : mExecutor(executor), mErrorLogger(errorLogger) {}

    void reportOut(const std::string &outmsg, Color c) override
    {
        std::lock_guard<std::mutex> lg(mReportSync);

        mErrorLogger.reportOut(outmsg, c);
    }

    void reportErr(const ErrorMessage &msg) override {
        if (!mExecutor.hasToLog(msg))
            return;

        std::lock_guard<std::mutex> lg(mReportSync);
        mErrorLogger.reportErr(msg);
    }

    void reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal) {
        std::lock_guard<std::mutex> lg(mReportSync);
        mExecutor.reportStatus(fileindex, filecount, sizedone, sizetotal);
    }

private:
    std::mutex mReportSync;
    Executor &mExecutor;
    ErrorLogger &mErrorLogger;
};

class Executor::ThreadData::ThreadDataImpl
{
public:
    ThreadDataImpl(Executor &executor, ErrorLogger &errorLogger, const Settings &settings, const std::map<std::string, std::size_t> &files, const std::list<ImportProject::FileSettings> &fileSettings)
        : mFiles(files), mFileSettings(fileSettings), mProcessedFiles(0), mProcessedSize(0), mSettings(settings), logForwarder(executor, errorLogger) {
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

    unsigned int check(ErrorLogger &errorLogger, const std::string *file, const ImportProject::FileSettings *fs) {
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

    std::size_t mProcessedFiles;
    std::size_t mTotalFiles;
    std::size_t mProcessedSize;
    std::size_t mTotalFileSize;

    std::mutex mFileSync;
    const Settings &mSettings;

public:
    SyncLogForwarder logForwarder;
};

Executor::ThreadData::ThreadData(Executor &executor, ErrorLogger &errorLogger, const Settings &settings, const std::map<std::string, std::size_t> &files, const std::list<ImportProject::FileSettings> &fileSettings)
    : mImpl(new ThreadDataImpl(executor, errorLogger, settings, files, fileSettings))
{}

bool Executor::ThreadData::next(const std::string *&file, const ImportProject::FileSettings *&fs, std::size_t &fileSize) {
    return mImpl->next(file, fs, fileSize);
}

unsigned int Executor::ThreadData::check(ErrorLogger &errorLogger, const std::string *file, const ImportProject::FileSettings *fs) {
    return mImpl->check(errorLogger, file, fs);
}

void Executor::ThreadData::status(std::size_t fileSize) {
    return mImpl->status(fileSize);
}

ErrorLogger& Executor::ThreadData::logForwarder() {
    return mImpl->logForwarder;
}

unsigned int Executor::check()
{
    std::vector<std::future<unsigned int>> threadFutures;
    threadFutures.reserve(mSettings.jobs);

    ThreadData data(*this, mErrorLogger, mSettings, mFiles, mSettings.project.fileSettings);

    for (unsigned int i = 0; i < mSettings.jobs; ++i) {
        try {
            threadFutures.emplace_back(std::async(std::launch::async, std::mem_fn(&Executor::threadProc), this, &data));
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

void Executor::reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal)
{
    if (filecount > 1) {
        std::ostringstream oss;
        const unsigned long percentDone = (sizetotal > 0) ? (100 * sizedone) / sizetotal : 0;
        oss << fileindex << '/' << filecount
            << " files checked " << percentDone
            << "% done";
        mErrorLogger.reportOut(oss.str(), Color::FgBlue);
    }
}

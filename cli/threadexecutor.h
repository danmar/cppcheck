/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#ifndef THREADEXECUTOR_H
#define THREADEXECUTOR_H

#include "config.h"
#include "errorlogger.h"

#include <cstddef>
#include <list>
#include <map>
#include <string>

#if (defined(__GNUC__) || defined(__sun)) && !defined(__MINGW32__) && !defined(__CYGWIN__)
#define THREADING_MODEL_FORK
#elif defined(_WIN32)
#define THREADING_MODEL_WIN
#include <windows.h>

#include "importproject.h"
#endif

class Settings;

/// @addtogroup CLI
/// @{

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class ThreadExecutor : public ErrorLogger {
public:
    ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger);
    ThreadExecutor(const ThreadExecutor &) = delete;
    ~ThreadExecutor() OVERRIDE;
    void operator=(const ThreadExecutor &) = delete;
    unsigned int check();

    void reportOut(const std::string &outmsg) OVERRIDE;
    void reportErr(const ErrorMessage &msg) OVERRIDE;
    void reportInfo(const ErrorMessage &msg) OVERRIDE;
    void bughuntingReport(const std::string &str) OVERRIDE;

    /**
     * @brief Add content to a file, to be used in unit testing.
     *
     * @param path File name (used as a key to link with real file).
     * @param content If the file would be a real file, this should be
     * the content of the file.
     */
    void addFileContent(const std::string &path, const std::string &content);

private:
    const std::map<std::string, std::size_t> &mFiles;
    Settings &mSettings;
    ErrorLogger &mErrorLogger;
    unsigned int mFileCount;

#if defined(THREADING_MODEL_FORK)

    /** @brief Key is file name, and value is the content of the file */
    std::map<std::string, std::string> mFileContents;
private:
    enum PipeSignal {REPORT_OUT='1',REPORT_ERROR='2', REPORT_INFO='3', REPORT_VERIFICATION='4', CHILD_END='5'};

    /**
     * Read from the pipe, parse and handle what ever is in there.
     *@return -1 in case of error
     *         0 if there is nothing in the pipe to be read
     *         1 if we did read something
     */
    int handleRead(int rpipe, unsigned int &result);
    void writeToPipe(PipeSignal type, const std::string &data);
    /**
     * Write end of status pipe, different for each child.
     * Not used in master process.
     */
    std::list<std::string> mErrorList;
    int mWpipe;

    /**
     * @brief Check load average condition
     * @param nchildren - count of currently ran children
     * @return true - if new process can be started
     */
    bool checkLoadAverage(size_t nchildren);

    /**
     * @brief Reports internal errors related to child processes
     * @param msg The error message
     */
    void reportInternalChildErr(const std::string &childname, const std::string &msg);

public:
    /**
     * @return true if support for threads exist.
     */
    static bool isEnabled() {
        return true;
    }

#elif defined(THREADING_MODEL_WIN)

private:
    enum class MessageType {REPORT_ERROR, REPORT_INFO};

    std::map<std::string, std::string> mFileContents;
    std::map<std::string, std::size_t>::const_iterator mItNextFile;
    std::list<ImportProject::FileSettings>::const_iterator mItNextFileSettings;
    std::size_t mProcessedFiles;
    std::size_t mTotalFiles;
    std::size_t mProcessedSize;
    std::size_t mTotalFileSize;
    CRITICAL_SECTION mFileSync;

    std::list<std::string> mErrorList;
    CRITICAL_SECTION mErrorSync;

    CRITICAL_SECTION mReportSync;

    void report(const ErrorMessage &msg, MessageType msgType);

    static unsigned __stdcall threadProc(void*);

public:
    /**
     * @return true if support for threads exist.
     */
    static bool isEnabled() {
        return true;
    }
#else
public:
    /**
     * @return true if support for threads exist.
     */
    static bool isEnabled() {
        return false;
    }
#endif
};

/// @}

#endif // THREADEXECUTOR_H

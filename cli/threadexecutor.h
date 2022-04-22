/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include <cstddef>
#include <list>
#include <map>
#include <string>

class Settings;
class ErrorLogger;

/// @addtogroup CLI
/// @{

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class ThreadExecutor {
public:
    ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger);
    ThreadExecutor(const ThreadExecutor &) = delete;
    ~ThreadExecutor();
    void operator=(const ThreadExecutor &) = delete;
    unsigned int check();

private:
    const std::map<std::string, std::size_t> &mFiles;
    Settings &mSettings;
    ErrorLogger &mErrorLogger;
    std::list<std::string> mErrorList;

#if defined(THREADING_MODEL_FORK)

    /**
     * Read from the pipe, parse and handle what ever is in there.
     *@return -1 in case of error
     *         0 if there is nothing in the pipe to be read
     *         1 if we did read something
     */
    int handleRead(int rpipe, unsigned int &result);

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

#elif defined(THREADING_MODEL_THREAD)

    class SyncLogForwarder;
    static unsigned int STDCALL threadProc(SyncLogForwarder *logforwarder);

#endif

public:
    /**
     * @return true if support for threads exist.
     */
    static bool isEnabled();
};

/// @}

#endif // THREADEXECUTOR_H

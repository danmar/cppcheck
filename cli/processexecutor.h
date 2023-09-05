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

#ifndef PROCESSEXECUTOR_H
#define PROCESSEXECUTOR_H

#include "executor.h"

#include <cstddef>
#include <map>
#include <string>

class Settings;
class ErrorLogger;
class Suppressions;

/// @addtogroup CLI
/// @{

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class ProcessExecutor : public Executor {
public:
    ProcessExecutor(const std::map<std::string, std::size_t> &files, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger);
    ProcessExecutor(const ProcessExecutor &) = delete;
    void operator=(const ProcessExecutor &) = delete;

    unsigned int check() override;

private:
    /**
     * Read from the pipe, parse and handle what ever is in there.
     * @return False in case of an recoverable error - will exit process on others
     */
    bool handleRead(int rpipe, unsigned int &result, const std::string& filename);

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
};

/// @}

#endif // PROCESSEXECUTOR_H

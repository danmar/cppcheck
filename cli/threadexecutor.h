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

#ifndef THREADEXECUTOR_H
#define THREADEXECUTOR_H

#include "config.h"

#include "executor.h"

#include <cstddef>
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
class ThreadExecutor : public Executor {
public:
    ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger);
    ThreadExecutor(const ThreadExecutor &) = delete;
    ~ThreadExecutor() override;
    void operator=(const ThreadExecutor &) = delete;

    unsigned int check() override;

private:
    class Data;
    class SyncLogForwarder;
    static unsigned int STDCALL threadProc(Data *data, SyncLogForwarder *logForwarder, const Settings &settings);
};

/// @}

#endif // THREADEXECUTOR_H

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
#include "importproject.h"

#include <cassert>

enum class Color;

ThreadExecutor::ThreadExecutor(const std::map<std::string, std::size_t> &files, Settings &settings, ErrorLogger &errorLogger)
    : Executor(files, settings, errorLogger)
{
    assert(mSettings.jobs > 1);
}

ThreadExecutor::~ThreadExecutor()
{}

unsigned int STDCALL ThreadExecutor::threadProc(ThreadData *data)
{
    unsigned int result = 0;

    const std::string *file;
    const ImportProject::FileSettings *fs;
    std::size_t fileSize;

    while (data->next(file, fs, fileSize)) {
        result += data->check(data->logForwarder(), file, fs);

        data->status(fileSize);
    }

    return result;
}

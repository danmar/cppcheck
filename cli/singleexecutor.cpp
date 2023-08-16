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

#include "singleexecutor.h"

#include "cppcheck.h"
#include "importproject.h"
#include "library.h"
#include "settings.h"

#include <cassert>
#include <list>
#include <numeric>
#include <utility>

class ErrorLogger;

SingleExecutor::SingleExecutor(CppCheck &cppcheck, const std::map<std::string, std::size_t> &files, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger)
    : Executor(files, settings, suppressions, errorLogger)
    , mCppcheck(cppcheck)
{
    assert(mSettings.jobs == 1);
}

// TODO: markup handling is not performed with multiple jobs
unsigned int SingleExecutor::check()
{
    unsigned int result = 0;

    const std::size_t totalfilesize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const std::pair<std::string, std::size_t>& f) {
        return v + f.second;
    });

    std::size_t processedsize = 0;
    unsigned int c = 0;
    // TODO: processes either mSettings.project.fileSettings or mFiles - process/thread implementations process both
    // TODO: thread/process implementations process fileSettings first
    if (mSettings.project.fileSettings.empty()) {
        for (std::map<std::string, std::size_t>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i) {
            if (!mSettings.library.markupFile(i->first)
                || !mSettings.library.processMarkupAfterCode(i->first)) {
                result += mCppcheck.check(i->first);
                processedsize += i->second;
                if (!mSettings.quiet)
                    reportStatus(c + 1, mFiles.size(), processedsize, totalfilesize);
                // TODO: call analyseClangTidy()?
                c++;
            }
        }
    } else {
        // filesettings
        // check all files of the project
        for (const ImportProject::FileSettings &fs : mSettings.project.fileSettings) {
            if (!mSettings.library.markupFile(fs.filename)
                || !mSettings.library.processMarkupAfterCode(fs.filename)) {
                result += mCppcheck.check(fs);
                ++c;
                if (!mSettings.quiet)
                    reportStatus(c, mSettings.project.fileSettings.size(), c, mSettings.project.fileSettings.size());
                if (mSettings.clangTidy)
                    mCppcheck.analyseClangTidy(fs);
            }
        }
    }

    // second loop to parse all markup files which may not work until all
    // c/cpp files have been parsed and checked
    // TODO: get rid of duplicated code
    if (mSettings.project.fileSettings.empty()) {
        for (std::map<std::string, std::size_t>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i) {
            if (mSettings.library.markupFile(i->first)
                && mSettings.library.processMarkupAfterCode(i->first)) {
                result += mCppcheck.check(i->first);
                processedsize += i->second;
                if (!mSettings.quiet)
                    reportStatus(c + 1, mFiles.size(), processedsize, totalfilesize);
                // TODO: call analyseClangTidy()?
                c++;
            }
        }
    }
    else {
        for (const ImportProject::FileSettings &fs : mSettings.project.fileSettings) {
            if (mSettings.library.markupFile(fs.filename)
                && mSettings.library.processMarkupAfterCode(fs.filename)) {
                result += mCppcheck.check(fs);
                ++c;
                if (!mSettings.quiet)
                    reportStatus(c, mSettings.project.fileSettings.size(), c, mSettings.project.fileSettings.size());
                if (mSettings.clangTidy)
                    mCppcheck.analyseClangTidy(fs);
            }
        }
    }
    if (mCppcheck.analyseWholeProgram())
        result++;

    return result;
}

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
#include "errorlogger.h"
#include "settings.h"
#include "suppressions.h"

#include <algorithm>
#include <sstream> // IWYU pragma: keep
#include <utility>

Executor::Executor(const std::map<std::string, std::size_t> &files, const Settings &settings, Suppressions &suppressions, ErrorLogger &errorLogger)
    : mFiles(files), mSettings(settings), mSuppressions(suppressions), mErrorLogger(errorLogger)
{}

bool Executor::hasToLog(const ErrorMessage &msg)
{
    if (!mSuppressions.isSuppressed(msg))
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


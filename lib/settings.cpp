/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "settings.h"

#include "valueflow.h"

bool Settings::mTerminated;

Settings::Settings()
    : mEnabled(0),
      debugSimplified(false),
      debugnormal(false),
      debugwarnings(false),
      dump(false),
      exceptionHandling(false),
      inconclusive(false),
      jointSuppressionReport(false),
      experimental(false),
      quiet(false),
      inlineSuppressions(false),
      verbose(false),
      force(false),
      relativePaths(false),
      xml(false), xml_version(2),
      jobs(1),
      loadAverage(0),
      exitCode(0),
      showtime(SHOWTIME_NONE),
      preprocessOnly(false),
      maxConfigs(12),
      enforcedLang(None),
      reportProgress(false),
      checkConfiguration(false),
      checkLibrary(false)
{
}

std::string Settings::addEnabled(const std::string &str)
{
    // Enable parameters may be comma separated...
    if (str.find(',') != std::string::npos) {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(',', pos)) != std::string::npos) {
            if (pos == prevPos)
                return std::string("cppcheck: --enable parameter is empty");
            const std::string errmsg(addEnabled(str.substr(prevPos, pos - prevPos)));
            if (!errmsg.empty())
                return errmsg;
            ++pos;
            prevPos = pos;
        }
        if (prevPos >= str.length())
            return std::string("cppcheck: --enable parameter is empty");
        return addEnabled(str.substr(prevPos));
    }

    if (str == "all") {
        mEnabled |= WARNING | STYLE | PERFORMANCE | PORTABILITY | INFORMATION | UNUSED_FUNCTION | MISSING_INCLUDE;
    } else if (str == "warning") {
        mEnabled |= WARNING;
    } else if (str == "style") {
        mEnabled |= STYLE;
    } else if (str == "performance") {
        mEnabled |= PERFORMANCE;
    } else if (str == "portability") {
        mEnabled |= PORTABILITY;
    } else if (str == "information") {
        mEnabled |= INFORMATION | MISSING_INCLUDE;
    } else if (str == "unusedFunction") {
        mEnabled |= UNUSED_FUNCTION;
    } else if (str == "missingInclude") {
        mEnabled |= MISSING_INCLUDE;
    }
#ifdef CHECK_INTERNAL
    else if (str == "internal") {
        mEnabled |= INTERNAL;
    }
#endif
    else {
        if (str.empty())
            return std::string("cppcheck: --enable parameter is empty");
        else
            return std::string("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }

    return std::string();
}

bool Settings::isEnabled(Severity::SeverityType severity) const
{
    switch (severity) {
    case Severity::none:
        return true;
    case Severity::error:
        return true;
    case Severity::warning:
        return isEnabled(WARNING);
    case Severity::style:
        return isEnabled(STYLE);
    case Severity::performance:
        return isEnabled(PERFORMANCE);
    case Severity::portability:
        return isEnabled(PORTABILITY);
    case Severity::information:
        return isEnabled(INFORMATION);
    case Severity::debug:
        return false;
    default:
        return false;
    }
}

bool Settings::isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck) const
{
    if (!isEnabled(Settings::WARNING) && (value->condition || value->defaultArg))
        return false;
    if (!inconclusive && (inconclusiveCheck || value->isInconclusive()))
        return false;
    return true;
}

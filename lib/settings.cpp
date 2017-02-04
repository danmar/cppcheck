/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "preprocessor.h"       // Preprocessor
#include "utils.h"

#include <fstream>
#include <set>

bool Settings::_terminated;

Settings::Settings()
    : debug(false),
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
      xml(false), xml_version(1),
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

namespace {
    const std::set<std::string> id = make_container< std::set<std::string> > ()
                                     << "warning"
                                     << "style"
                                     << "performance"
                                     << "portability"
                                     << "information"
                                     << "missingInclude"
                                     << "unusedFunction"
#ifdef CHECK_INTERNAL
                                     << "internal"
#endif
                                     ;
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
        for (std::set<std::string>::const_iterator it = id.cbegin(); it != id.cend(); ++it) {
            if (*it == "internal")
                continue;

            _enabled.insert(*it);
        }
    } else if (id.find(str) != id.end()) {
        _enabled.insert(str);
        if (str == "information") {
            _enabled.insert("missingInclude");
        }
    } else {
        if (str.empty())
            return std::string("cppcheck: --enable parameter is empty");
        else
            return std::string("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }

    return std::string();
}


bool Settings::append(const std::string &filename)
{
    std::ifstream fin(filename.c_str());
    if (!fin.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(fin, line)) {
        _append += line + "\n";
    }
    Preprocessor::preprocessWhitespaces(_append);
    return true;
}

const std::string &Settings::append() const
{
    return _append;
}

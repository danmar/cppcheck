/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "path.h"
#include "preprocessor.h"       // Preprocessor

#include <fstream>
#include <set>

Settings::Settings()
    : _terminate(false),
      debug(false),
      debugwarnings(false),
      debugFalsePositive(false),
      dump(false),
      exceptionHandling(false),
      inconclusive(false),
      jointSuppressionReport(false),
      experimental(false),
      _errorsOnly(false),
      _inlineSuppressions(false),
      _verbose(false),
      _force(false),
      _relativePaths(false),
      _xml(false), _xml_version(1),
      _jobs(1),
      _loadAverage(0),
      _exitCode(0),
      _showtime(SHOWTIME_NONE),
      _maxConfigs(12),
      enforcedLang(None),
      reportProgress(false),
      checkConfiguration(false),
      checkLibrary(false)
{
    // This assumes the code you are checking is for the same architecture this is compiled on.
#if defined(_WIN64)
    platform(Win64);
#elif defined(_WIN32)
    platform(Win32A);
#else
    platform(Unspecified);
#endif
}

std::string Settings::addEnabled(const std::string &str)
{
    // Enable parameters may be comma separated...
    if (str.find(",") != std::string::npos) {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(",", pos)) != std::string::npos) {
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

    bool handled = false;

    static std::set<std::string> id;
    if (id.empty()) {
        id.insert("warning");
        id.insert("style");
        id.insert("performance");
        id.insert("portability");
        id.insert("information");
        id.insert("missingInclude");
        id.insert("unusedFunction");
#ifdef CHECK_INTERNAL
        id.insert("internal");
#endif
    }

    if (str == "all") {
        std::set<std::string>::const_iterator it;
        for (it = id.begin(); it != id.end(); ++it) {
            if (*it == "internal")
                continue;

            _enabled.insert(*it);
        }
    } else if (id.find(str) != id.end()) {
        _enabled.insert(str);
        if (str == "information") {
            _enabled.insert("missingInclude");
        }
    } else if (!handled) {
        if (str.empty())
            return std::string("cppcheck: --enable parameter is empty");
        else
            return std::string("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }

    return std::string("");
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

bool Settings::platform(PlatformType type)
{
    switch (type) {
    case Unspecified: // same as system this code was compile on
        platformType = type;
        sizeof_bool = sizeof(bool);
        sizeof_short = sizeof(short);
        sizeof_int = sizeof(int);
        sizeof_long = sizeof(long);
        sizeof_long_long = sizeof(long long);
        sizeof_float = sizeof(float);
        sizeof_double = sizeof(double);
        sizeof_long_double = sizeof(long double);
        sizeof_wchar_t = sizeof(wchar_t);
        sizeof_size_t = sizeof(std::size_t);
        sizeof_pointer = sizeof(void *);
        return true;
    case Win32W:
    case Win32A:
        platformType = type;
        sizeof_bool = 1; // 4 in Visual C++ 4.2
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_wchar_t = 2;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        return true;
    case Win64:
        platformType = type;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_wchar_t = 2;
        sizeof_size_t = 8;
        sizeof_pointer = 8;
        return true;
    case Unix32:
        platformType = type;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 12;
        sizeof_wchar_t = 4;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        return true;
    case Unix64:
        platformType = type;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 8;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 16;
        sizeof_wchar_t = 4;
        sizeof_size_t = 8;
        sizeof_pointer = 8;
        return true;
    }

    // unsupported platform
    return false;
}

bool Settings::platformFile(const std::string &filename)
{
    (void)filename;
    /** @todo TBD */

    return false;
}

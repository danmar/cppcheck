/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>   // std::isdigit, std::isalnum, etc
#include <set>
#include <stack>

Settings::Settings()
{
    debug = debugwarnings = false;
    debugFalsePositive = false;
    _errorsOnly = false;
    _inlineSuppressions = false;
    _verbose = false;
    _force = false;
    _xml = false;
    _xml_version = 1;
    _jobs = 1;
    _exitCode = 0;
    _showtime = 0; // TODO: use enum
    _append = "";
    _terminate = false;
    inconclusive = false;
    experimental = false;
    test_2_pass = false;
    reportProgress = false;
    ifcfg = false;
    checkConfiguration = false;
    posix = false;

    // This assumes the code you are checking is for the same architecture this is compiled on.
#if defined(_WIN32)
    platform(Win32);
#elif defined(_WIN64)
    platform(Win64);
#else
    platform(Host);
#endif
}

std::string Settings::addEnabled(const std::string &str)
{
    // Enable parameters may be comma separated...
    if (str.find(",") != std::string::npos)
    {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(",", pos)) != std::string::npos)
        {
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

    std::set<std::string> id;
    id.insert("style");
    id.insert("performance");
    id.insert("portability");
    id.insert("information");
    id.insert("missingInclude");
    id.insert("unusedFunction");

    if (str == "all")
    {
        std::set<std::string>::const_iterator it;
        for (it = id.begin(); it != id.end(); ++it)
            _enabled.insert(*it);
    }
    else if (id.find(str) != id.end())
    {
        _enabled.insert(str);
    }
    else if (!handled)
    {
        if (str.empty())
            return std::string("cppcheck: --enable parameter is empty");
        else
            return std::string("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }

    return std::string("");
}

bool Settings::isEnabled(const std::string &str) const
{
    return bool(_enabled.find(str) != _enabled.end());
}


void Settings::append(const std::string &filename)
{
    _append = "\n";
    std::ifstream fin(filename.c_str());
    std::string line;
    while (std::getline(fin, line))
    {
        _append += line + "\n";
    }
}

std::string Settings::append() const
{
    return _append;
}

bool Settings::platform(PlatformType type)
{
    switch (type)
    {
    case Host: // same as system this code was compile on
        platformType = Host;
        sizeof_bool = sizeof(bool);
        sizeof_short = sizeof(short);
        sizeof_int = sizeof(int);
        sizeof_long = sizeof(long);
        sizeof_long_long = sizeof(long long);
        sizeof_float = sizeof(float);
        sizeof_double = sizeof(double);
        sizeof_long_double = sizeof(long double);
        sizeof_size_t = sizeof(size_t);
        sizeof_pointer = sizeof(void *);
        return true;
    case Win32:
        platformType = Win32;
        sizeof_bool = 1; // 4 in Visual C++ 4.2
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        return true;
    case Win64:
        platformType = Win64;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_size_t = 8;
        sizeof_pointer = 8;
        return true;
    case Unix32:
        platformType = Unix32;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 12;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        return true;
    case Unix64:
        platformType = Unix64;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 8;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 16;
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

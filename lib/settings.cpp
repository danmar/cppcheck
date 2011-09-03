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

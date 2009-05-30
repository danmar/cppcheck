/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include "settings.h"

#include <algorithm>

Settings::Settings()
{
    _debug = false;
    _showAll = false;
    _checkCodingStyle = false;
    _errorsOnly = false;
    _verbose = false;
    _force = false;
    _xml = false;
    _unusedFunctions = false;
    _security = false;
    _jobs = 1;
    _exitCode = 0;
}

Settings::~Settings()
{

}


void Settings::autoDealloc(std::istream &istr)
{
    std::string line;
    while (getline(istr, line))
    {
        // Check if line has a valid classname..
        if (line.empty())
            continue;

        // Add classname to list
        _autoDealloc.push_back(line);
    }
}


bool Settings::isAutoDealloc(const char classname[]) const
{
    return (std::find(_autoDealloc.begin(), _autoDealloc.end(), classname) != _autoDealloc.end());
}


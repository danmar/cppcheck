/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

Settings::Settings()
{
    _debug = false;
    _checkCodingStyle = false;
    _errorsOnly = false;
    _inlineSuppressions = false;
    _verbose = false;
    _force = false;
    _xml = false;
    _jobs = 1;
    _exitCode = 0;
    _showtime = 0; // TODO: use enum
    _append = "";
    _terminate = false;
    inconclusive = false;
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
        _autoDealloc.insert(line);
    }
}

bool Settings::Suppressions::parseFile(std::istream &istr)
{
    std::string line;
    while (getline(istr, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        std::istringstream lineStream(line);
        std::string id;
        std::string file;
        unsigned int lineNumber = 0;
        if (std::getline(lineStream, id, ':'))
        {
            if (std::getline(lineStream, file, ':'))
            {
                lineStream >> lineNumber;
            }
        }

        // We could perhaps check if the id is valid and return error if it is not
        addSuppression(id, file, lineNumber);
    }

    return true;
}

void Settings::Suppressions::addSuppression(const std::string &errorId, const std::string &file, unsigned int line)
{
    _suppressions[errorId][file].push_back(line);
    _suppressions[errorId][file].sort();
}

bool Settings::Suppressions::isSuppressed(const std::string &errorId, const std::string &file, unsigned int line)
{
    if (_suppressions.find(errorId) == _suppressions.end())
        return false;

    // Check are all errors of this type filtered out
    if (_suppressions[errorId].find("") != _suppressions[errorId].end())
        return true;

    if (_suppressions[errorId].find(file) == _suppressions[errorId].end())
        return false;

    // Check should all errors in this file be filtered out
    if (std::find(_suppressions[errorId][file].begin(), _suppressions[errorId][file].end(), 0) != _suppressions[errorId][file].end())
        return true;

    if (std::find(_suppressions[errorId][file].begin(), _suppressions[errorId][file].end(), line) == _suppressions[errorId][file].end())
        return false;

    return true;
}

void Settings::addEnabled(const std::string &str)
{
    // Enable parameters may be comma separated...
    if (str.find(",") != std::string::npos)
    {
        std::string::size_type prevPos = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(",", pos)) != std::string::npos)
        {
            if (pos == prevPos)
                throw std::runtime_error("cppcheck: --enable parameter is empty");
            addEnabled(str.substr(prevPos, pos - prevPos));
            ++pos;
            prevPos = pos;
        }
        if (prevPos >= str.length())
            throw std::runtime_error("cppcheck: --enable parameter is empty");
        addEnabled(str.substr(prevPos));
        return;
    }

    bool handled = false;

    if (str == "all")
        handled = _checkCodingStyle = true;
    else if (str == "style")
        handled = _checkCodingStyle = true;

    std::set<std::string> id;
    id.insert("exceptNew");
    id.insert("exceptRealloc");
    id.insert("unusedFunctions");

    if (str == "all")
    {
        std::set<std::string>::const_iterator it;
        for (it = id.begin(); it != id.end(); ++it)
            _enabled[*it] = true;
    }
    else if (id.find(str) != id.end())
    {
        _enabled[str] = true;
    }
    else if (!handled)
    {
        if (str.empty())
            throw std::runtime_error("cppcheck: --enable parameter is empty");
        else
            throw std::runtime_error("cppcheck: there is no --enable parameter with the name '" + str + "'");
    }
}

bool Settings::isEnabled(const std::string &str) const
{
    return bool(_enabled.find(str) != _enabled.end());
}

void Settings::addAutoAllocClass(const std::string &name)
{
    _autoDealloc.insert(name);
}

bool Settings::isAutoDealloc(const std::string &classname) const
{
    return (_autoDealloc.find(classname) != _autoDealloc.end());
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

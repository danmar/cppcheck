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
    _checkCodingStyle = false;
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
    test_2_pass = false;
    reportProgress = false;
    ifcfg = false;
}

std::string Settings::Suppressions::parseFile(std::istream &istr)
{
    // Change '\r' to '\n' in the istr
    std::string filedata;
    std::string line;
    while (std::getline(istr, line))
        filedata += line + "\n";
    while (filedata.find("\r") != std::string::npos)
        filedata[filedata.find("\r")] = '\n';

    // Parse filedata..
    std::istringstream istr2(filedata);
    while (std::getline(istr2, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Skip comments
        if (line.length() >= 2 && line[0] == '/' && line[1] == '/')
            continue;

        const std::string errmsg(addSuppressionLine(line));
        if (!errmsg.empty())
            return errmsg;
    }

    return "";
}

std::string Settings::Suppressions::addSuppressionLine(const std::string &line)
{
    std::istringstream lineStream(line);
    std::string id;
    std::string file;
    unsigned int lineNumber = 0;
    if (std::getline(lineStream, id, ':'))
    {
        if (std::getline(lineStream, file))
        {
            // If there is not a dot after the last colon in "file" then
            // the colon is a separator and the contents after the colon
            // is a line number..

            // Get position of last colon
            const std::string::size_type pos = file.rfind(":");

            // if a colon is found and there is no dot after it..
            if (pos != std::string::npos &&
                file.find(".", pos) == std::string::npos)
            {
                // Try to parse out the line number
                try
                {
                    std::istringstream istr1(file.substr(pos+1));
                    istr1 >> lineNumber;
                }
                catch (...)
                {
                    lineNumber = 0;
                }

                if (lineNumber > 0)
                {
                    file.erase(pos);
                }
            }
        }
    }

    // We could perhaps check if the id is valid and return error if it is not
    const std::string errmsg(addSuppression(id, file, lineNumber));
    if (!errmsg.empty())
        return errmsg;

    return "";
}

bool Settings::Suppressions::FileMatcher::match(const std::string &pattern, const std::string &name)
{
    const char *p = pattern.c_str();
    const char *n = name.c_str();
    std::stack<std::pair<const char *, const char *> > backtrack;

    for (;;)
    {
        bool matching = true;
        while (*p != '\0' && matching)
        {
            switch (*p)
            {
            case '*':
                // Step forward until we match the next character after *
                while (*n != '\0' && *n != p[1])
                {
                    n++;
                }
                if (*n != '\0')
                {
                    // If this isn't the last possibility, save it for later
                    backtrack.push(std::make_pair(p, n));
                }
                break;
            case '?':
                // Any character matches unless we're at the end of the name
                if (*n != '\0')
                {
                    n++;
                }
                else
                {
                    matching = false;
                }
                break;
            default:
                // Non-wildcard characters match literally
                if (*n == *p)
                {
                    n++;
                }
                else
                {
                    matching = false;
                }
                break;
            }
            p++;
        }

        // If we haven't failed matching and we've reached the end of the name, then success
        if (matching && *n == '\0')
        {
            return true;
        }

        // If there are no other paths to tray, then fail
        if (backtrack.empty())
        {
            return false;
        }

        // Restore pointers from backtrack stack
        p = backtrack.top().first;
        n = backtrack.top().second;
        backtrack.pop();

        // Advance name pointer by one because the current position didn't work
        n++;
    }
}

std::string Settings::Suppressions::FileMatcher::addFile(const std::string &name, unsigned int line)
{
    if (name.find_first_of("*?") != std::string::npos)
    {
        for (std::string::const_iterator i = name.begin(); i != name.end(); ++i)
        {
            if (*i == '*')
            {
                std::string::const_iterator j = i + 1;
                if (j != name.end() && (*j == '*' || *j == '?'))
                {
                    return "Failed to add suppression. Syntax error in glob.";
                }
            }
        }
        _globs[name].insert(line);
    }
    else if (name.empty())
    {
        _globs["*"].insert(0U);
    }
    else
    {
        _files[name].insert(line);
    }
    return "";
}

bool Settings::Suppressions::FileMatcher::isSuppressed(const std::string &file, unsigned int line)
{
    std::set<unsigned int> lineset;

    std::map<std::string, std::set<unsigned int> >::const_iterator f = _files.find(file);
    if (f != _files.end())
    {
        lineset.insert(f->second.begin(), f->second.end());
    }
    for (std::map<std::string, std::set<unsigned int> >::iterator g = _globs.begin(); g != _globs.end(); ++g)
    {
        if (match(g->first, file))
        {
            lineset.insert(g->second.begin(), g->second.end());
        }
    }

    if (lineset.empty())
        return false;

    // Check should all errors in this file be filtered out
    if (lineset.find(0U) != lineset.end())
        return true;

    if (lineset.find(line) == lineset.end())
        return false;

    return true;
}

std::string Settings::Suppressions::addSuppression(const std::string &errorId, const std::string &file, unsigned int line)
{
    // Check that errorId is valid..
    if (errorId.empty())
    {
        return "Failed to add suppression. No id.";
    }
    for (std::string::size_type pos = 0; pos < errorId.length(); ++pos)
    {
        if (errorId[pos] < 0 || !std::isalnum(errorId[pos]))
        {
            return "Failed to add suppression. Invalid id \"" + errorId + "\"";
        }
        if (pos == 0 && std::isdigit(errorId[pos]))
        {
            return "Failed to add suppression. Invalid id \"" + errorId + "\"";
        }
    }

    return _suppressions[errorId].addFile(file, line);
}

bool Settings::Suppressions::isSuppressed(const std::string &errorId, const std::string &file, unsigned int line)
{
    if (_suppressions.find(errorId) == _suppressions.end())
        return false;

    return _suppressions[errorId].isSuppressed(file, line);
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

    if (str == "all")
        handled = _checkCodingStyle = true;
    else if (str == "style")
        handled = _checkCodingStyle = true;

    std::set<std::string> id;
    id.insert("missingInclude");
    id.insert("unusedFunction");
    id.insert("information");

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

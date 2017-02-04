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

#include "pathmatch.h"
#include "path.h"
#include <algorithm>
#include <ctype.h>

PathMatch::PathMatch(const std::vector<std::string> &excludedPaths, bool caseSensitive)
    : _excludedPaths(excludedPaths), _caseSensitive(caseSensitive)
{
    if (!_caseSensitive)
        for (std::vector<std::string>::iterator i = _excludedPaths.begin(); i != _excludedPaths.end(); ++i)
            std::transform(i->begin(), i->end(), i->begin(), ::tolower);
    _workingDirectory.push_back(Path::getCurrentPath());
}

bool PathMatch::Match(const std::string &path) const
{
    if (path.empty())
        return false;

    for (std::vector<std::string>::const_iterator i = _excludedPaths.begin(); i != _excludedPaths.end(); ++i) {
        const std::string excludedPath((!Path::isAbsolute(path) && Path::isAbsolute(*i)) ? Path::getRelativePath(*i, _workingDirectory) : *i);

        std::string findpath = Path::fromNativeSeparators(path);
        if (!_caseSensitive)
            std::transform(findpath.begin(), findpath.end(), findpath.begin(), ::tolower);

        // Filtering directory name
        if (excludedPath.back() == '/') {
            if (findpath.back() != '/')
                findpath = RemoveFilename(findpath);

            if (excludedPath.length() > findpath.length())
                continue;
            // Match relative paths starting with mask
            // -isrc matches src/foo.cpp
            if (findpath.compare(0, excludedPath.size(), excludedPath) == 0)
                return true;
            // Match only full directory name in middle or end of the path
            // -isrc matches myproject/src/ but does not match
            // myproject/srcfiles/ or myproject/mysrc/
            if (findpath.find("/" + excludedPath) != std::string::npos)
                return true;
        }
        // Filtering filename
        else {
            if (excludedPath.length() > findpath.length())
                continue;
            // Check if path ends with mask
            // -ifoo.cpp matches (./)foo.c, src/foo.cpp and proj/src/foo.cpp
            // -isrc/file.cpp matches src/foo.cpp and proj/src/foo.cpp
            if (findpath.compare(findpath.size() - excludedPath.size(), findpath.size(), excludedPath) == 0)
                return true;

        }
    }
    return false;
}

std::string PathMatch::RemoveFilename(const std::string &path)
{
    const std::size_t ind = path.find_last_of('/');
    return path.substr(0, ind + 1);
}

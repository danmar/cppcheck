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

#include "pathmatch.h"

PathMatch::PathMatch(const std::vector<std::string> &masks)
    : _masks(masks)
{
}

bool PathMatch::Match(const std::string &path)
{
    if (path.empty())
        return false;

    std::vector<std::string>::const_iterator iterMask;
    for (iterMask = _masks.begin(); iterMask != _masks.end(); ++iterMask)
    {
        std::string findpath(path);
        if (findpath[findpath.length() - 1] != '/')
            findpath = RemoveFilename(findpath);

        if (findpath.find(*iterMask) != std::string::npos)
            return true;
    }
    return false;
}

std::string PathMatch::RemoveFilename(const std::string &path)
{
    const size_t ind = path.find_last_of('/');
    return path.substr(0, ind + 1);
}

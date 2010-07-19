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

#include <algorithm>
#include "path.h"

std::string Path::toNativeSeparators(const std::string &path)
{
#if defined(_WIN32)
    char separ = '/';
    char native = '\\';
#else
    char separ = '\\';
    char native = '/';
#endif
    std::string modified(path);
    std::replace(modified.begin(), modified.end(), separ, native);
    return modified;
}

std::string Path::fromNativeSeparators(const std::string &path)
{
    char nonnative = '\\';
    char newsepar = '/';
    std::string modified(path);
    std::replace(modified.begin(), modified.end(), nonnative, newsepar);
    return modified;
}

std::string Path::simplifyPath(const std::string &path)
{
    std::string f(path);

    // Remove './' from begin of the path
    if (f.size() > 2 && f[0] == '.' && f[1] == '/')
        f = f.erase(0, 2);

    // replace "/ab/../" with "/"..
    std::string::size_type pos = 0;
    while ((pos = f.find("..", pos + 1)) != std::string::npos)
    {
        // position must be at least 4..
        if (pos < 4)
            continue;

        // Previous char must be a separator..
        if (f[pos-1] != '/')
            continue;

        // Next char must be a separator..
        if (f[pos+2] != '/')
            continue;

        // Locate previous separator..
        std::string::size_type sep = f.find_last_of("/", pos - 2);
        if (sep == std::string::npos)
            continue;

        // Delete substring..
        f.erase(sep, pos + 2 - sep);
        pos = sep;
    }

    return f;
}
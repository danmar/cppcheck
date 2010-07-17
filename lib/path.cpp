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

#include "path.h"

static std::string replace_chars(const std::string &str, char from, char to)
{
    std::string modified(str);
    size_t pos = modified.find(from);
    if (pos!= std::string::npos)
    {
        std::string::iterator iter = modified.begin() + pos;
        std::string::iterator end = modified.end();
        while (iter != end)
        {
            if (*iter == from)
                *iter = to;
            ++iter;
        }
    }
    return modified;
}

std::string Path::toNativeSeparators(const std::string &path)
{
#if defined(_WIN32)
    char separ = '/';
    char native = '\\';
#else
    char separ = '\\';
    char native = '/';
#endif
    return replace_chars(path, separ, native);
}

std::string Path::fromNativeSeparators(const std::string &path)
{
    char nonnative = '\\';
    char internal = '/';
    return replace_chars(path, nonnative, internal);
}

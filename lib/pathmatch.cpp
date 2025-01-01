/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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
#include "utils.h"

#include <cstddef>
#include <utility>

PathMatch::PathMatch(std::vector<std::string> paths, bool caseSensitive)
    : mPaths(std::move(paths)), mCaseSensitive(caseSensitive)
{
    for (std::string& p : mPaths)
    {
        p = Path::fromNativeSeparators(p);
        if (!mCaseSensitive)
            strTolower(p);
    }
    // TODO: also make lowercase?
    mWorkingDirectory.push_back(Path::fromNativeSeparators(Path::getCurrentPath()));
}

bool PathMatch::match(const std::string &path) const
{
    if (path.empty())
        return false;

    std::string findpath = Path::fromNativeSeparators(path);
    if (!mCaseSensitive)
        strTolower(findpath);

    const bool is_absolute = Path::isAbsolute(path);

    // TODO: align the match logic with ImportProject::ignorePaths()
    for (auto i = mPaths.cbegin(); i != mPaths.cend(); ++i) {
        const std::string pathToMatch((!is_absolute && Path::isAbsolute(*i)) ? Path::getRelativePath(*i, mWorkingDirectory) : *i);

        // Filtering directory name
        if (endsWith(pathToMatch,'/')) {
            if (!endsWith(findpath,'/'))
                findpath = removeFilename(findpath);

            if (pathToMatch.length() > findpath.length())
                continue;
            // Match relative paths starting with mask
            // -isrc matches src/foo.cpp
            if (findpath.compare(0, pathToMatch.size(), pathToMatch) == 0)
                return true;
            // Match only full directory name in middle or end of the path
            // -isrc matches myproject/src/ but does not match
            // myproject/srcfiles/ or myproject/mysrc/
            if (findpath.find("/" + pathToMatch) != std::string::npos)
                return true;
        }
        // Filtering filename
        else {
            if (pathToMatch.length() > findpath.length())
                continue;
            // Check if path ends with mask
            // -ifoo.cpp matches (./)foo.c, src/foo.cpp and proj/src/foo.cpp
            // -isrc/file.cpp matches src/foo.cpp and proj/src/foo.cpp
            if (findpath.compare(findpath.size() - pathToMatch.size(), findpath.size(), pathToMatch) == 0)
                return true;
        }
    }
    return false;
}

std::string PathMatch::removeFilename(const std::string &path)
{
    const std::size_t ind = path.find_last_of('/');
    return path.substr(0, ind + 1);
}

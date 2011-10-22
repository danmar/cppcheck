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

#include <algorithm>
#include <vector>
#include <sstream>
#include <cstring>
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

std::string Path::simplifyPath(const char *originalPath)
{
    // Skip ./ at the beginning
    if (strlen(originalPath) > 2 && originalPath[0] == '.' &&
        originalPath[1] == '/') {
        originalPath += 2;
    }

    std::string subPath = "";
    std::vector<std::string> pathParts;
    for (; *originalPath; ++originalPath) {
        if (*originalPath == '/' || *originalPath == '\\') {
            if (subPath.length() > 0) {
                pathParts.push_back(subPath);
                subPath = "";
            }

            pathParts.push_back(std::string(1 , *originalPath));
        } else
            subPath.append(1, *originalPath);
    }

    if (subPath.length() > 0)
        pathParts.push_back(subPath);

    for (unsigned int i = 0; i < pathParts.size(); ++i) {
        if (i > 1 && pathParts[i-2] != ".." && pathParts[i] == ".." && pathParts.size() > i + 1) {
            pathParts.erase(pathParts.begin() + static_cast<int>(i) + 1);
            pathParts.erase(pathParts.begin() + static_cast<int>(i));
            pathParts.erase(pathParts.begin() + static_cast<int>(i) - 1);
            pathParts.erase(pathParts.begin() + static_cast<int>(i) - 2);
            i = 0;
        } else if (i > 0 && pathParts[i] == ".") {
            pathParts.erase(pathParts.begin() + static_cast<int>(i));
            i = 0;
        } else if (pathParts[i] == "/" && i > 0 && pathParts[i-1] == "/") {
            pathParts.erase(pathParts.begin() + static_cast<int>(i) - 1);
            i = 0;
        }
    }

    std::ostringstream oss;
    for (std::vector<std::string>::size_type i = 0; i < pathParts.size(); ++i) {
        oss << pathParts[i];
    }

    return oss.str();
}

bool Path::sameFileName(const std::string &fname1, const std::string &fname2)
{
#if defined(__linux__) || defined(__sun) || defined(__hpux)
    return bool(fname1 == fname2);
#elif defined(__GNUC__)
    return bool(strcasecmp(fname1.c_str(), fname2.c_str()) == 0);
#elif defined(__BORLANDC__)
    return bool(stricmp(fname1.c_str(), fname2.c_str()) == 0);
#elif defined(_MSC_VER)
    return bool(_stricmp(fname1.c_str(), fname2.c_str()) == 0);
#else
#error Platform filename compare function needed
#endif
}

std::string Path::removeQuotationMarks(const std::string &path)
{
    std::string editPath(path);
    editPath.erase(std::remove(editPath.begin(), editPath.end(), '\"'), editPath.end());
    return editPath;
}

std::string Path::getFilenameExtension(const std::string &path)
{
    const std::string::size_type dotLocation = path.find_last_of('.');
    if (dotLocation == std::string::npos)
        return "";

    const std::string extension = path.substr(dotLocation);
    return extension;
}

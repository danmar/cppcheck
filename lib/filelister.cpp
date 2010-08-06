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

#include <sstream>
#include <vector>
#include <cstring>
#include <string>
#include <cctype>
#include <algorithm>
#include "filelister.h"

#if defined(_WIN32)
#include "filelister_win32.h"
#else // POSIX-style system
#include "filelister_unix.h"
#endif

// We have one singleton FileLister.

static FileLister *fileLister;

FileLister * getFileLister()
{
    if (fileLister == NULL)
    {
#if defined(_WIN32)
        fileLister = new FileListerWin32;
#else // POSIX-style system
        fileLister = new FileListerUnix;
#endif
        return fileLister;
    }
    return fileLister;
}

std::string FileLister::simplifyPath(const char *originalPath)
{
    std::string subPath = "";
    std::vector<std::string> pathParts;
    for (; *originalPath; ++originalPath)
    {
        if (*originalPath == '/' || *originalPath == '\\')
        {
            if (subPath.length() > 0)
            {
                pathParts.push_back(subPath);
                subPath = "";
            }

            pathParts.push_back(std::string(1 , *originalPath));
        }
        else
            subPath.append(1, *originalPath);
    }

    if (subPath.length() > 0)
        pathParts.push_back(subPath);

    for (unsigned int i = 0; i < pathParts.size(); ++i)
    {
        if (pathParts[i] == ".." && i > 1 && pathParts.size() > i + 1)
        {
            pathParts.erase(pathParts.begin() + static_cast<int>(i) + 1);
            pathParts.erase(pathParts.begin() + static_cast<int>(i));
            pathParts.erase(pathParts.begin() + static_cast<int>(i) - 1);
            pathParts.erase(pathParts.begin() + static_cast<int>(i) - 2);
            i = 0;
        }
        else if (i > 0 && pathParts[i] == ".")
        {
            pathParts.erase(pathParts.begin() + static_cast<int>(i));
            i = 0;
        }
        else if (pathParts[i] == "/" && i > 0 && pathParts[i-1] == "/")
        {
            pathParts.erase(pathParts.begin() + static_cast<int>(i) - 1);
            i = 0;
        }
    }

    std::ostringstream oss;
    for (std::vector<std::string>::size_type i = 0; i < pathParts.size(); ++i)
    {
        oss << pathParts[i];
    }

    return oss.str();
}

// This wrapper exists because Sun's CC does not allow a static_cast
// from extern "C" int(*)(int) to int(*)(int).
static int tolowerWrapper(int c)
{
    return std::tolower(c);
}

bool FileLister::acceptFile(const std::string &filename)
{
    std::string::size_type dotLocation = filename.find_last_of('.');
    if (dotLocation == std::string::npos)
        return false;

    std::string extension = filename.substr(dotLocation);
    std::transform(extension.begin(), extension.end(), extension.begin(), tolowerWrapper);

    if (extension == ".cpp" ||
        extension == ".cxx" ||
        extension == ".cc" ||
        extension == ".c" ||
        extension == ".c++" ||
        extension == ".txx")
    {
        return true;
    }

    return false;
}

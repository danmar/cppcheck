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
        extension == ".tpp" ||
        extension == ".txx" ||
        extension == ".m" ||        // Objective C
        extension == ".mm" ||       // Objective C++
        extension == ".java" ||     // Java source file
        extension == ".cs")         // C# source file
    {
        return true;
    }

    return false;
}

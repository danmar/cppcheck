/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

#include "filelister.h"
#include "fileLister_win32.h"
#include <sstream>
#include <vector>
#include <cstring>
#include <string>
#include <cctype>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#ifndef __BORLANDC__
#include <shlwapi.h>
#endif
#else // POSIX-style system
#include <glob.h>
#include <unistd.h>
#endif

static FileLister *fileLister;

FileLister * getFileLister()
{
	if (fileLister == NULL)
	{
		fileLister = new FileListerWin32;
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

    for (std::vector<std::string>::size_type i = 0; i < pathParts.size(); ++i)
    {
        if (pathParts[i] == ".." && i > 1 && pathParts.size() > i + 1)
        {
            pathParts.erase(pathParts.begin() + i + 1);
            pathParts.erase(pathParts.begin() + i);
            pathParts.erase(pathParts.begin() + i - 1);
            pathParts.erase(pathParts.begin() + i - 2);
            i = 0;
        }
        else if (i > 0 && pathParts[i] == ".")
        {
            pathParts.erase(pathParts.begin() + i);
            i = 0;
        }
        else if (pathParts[i] == "/" && i > 0 && pathParts[i-1] == "/")
        {
            pathParts.erase(pathParts.begin() + i - 1);
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


///////////////////////////////////////////////////////////////////////////////
////// This code is for Microsoft Windows /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)

#else // other than _WIN32

///////////////////////////////////////////////////////////////////////////////
////// This code is POSIX-style systems ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FileLister::recursiveAddFiles(std::vector<std::string> &filenames, const std::string &path, bool recursive)
{
    std::ostringstream oss;
    oss << path;
    if (path.length() > 0 && path[path.length()-1] == '/')
        oss << "*";

    glob_t glob_results;
    glob(oss.str().c_str(), GLOB_MARK, 0, &glob_results);
    for (unsigned int i = 0; i < glob_results.gl_pathc; i++)
    {
        std::string filename = glob_results.gl_pathv[i];
        if (filename == "." || filename == ".." || filename.length() == 0)
            continue;

        if (filename[filename.length()-1] != '/')
        {
            // File

            // If recursive is not used, accept all files given by user
            if (!recursive || FileLister::acceptFile(filename))
                filenames.push_back(filename);
        }
        else if (recursive)
        {
            // Directory
            FileLister::recursiveAddFiles(filenames, filename, recursive);
        }
    }
    globfree(&glob_results);
}
#endif

//---------------------------------------------------------------------------

bool FileLister::sameFileName(const std::string &fname1, const std::string &fname2)
{
#if defined(__linux__) || defined(__sun)
    return bool(fname1 == fname2);
#endif
#ifdef __GNUC__
    return bool(strcasecmp(fname1.c_str(), fname2.c_str()) == 0);
#endif
#ifdef __BORLANDC__
    return bool(stricmp(fname1.c_str(), fname2.c_str()) == 0);
#endif
#ifdef _MSC_VER
    return bool(_stricmp(fname1.c_str(), fname2.c_str()) == 0);
#endif
}

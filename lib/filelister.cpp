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
#include <sstream>
#include <vector>
#include <cstring>
#include <string>
#include <cctype>
#include <algorithm>

#if defined(__GNUC__) && !defined(__MINGW32__)
#include <glob.h>
#include <unistd.h>
#endif
#if defined(__BORLANDC__) || defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#ifndef __BORLANDC__
#include <shlwapi.h>
#endif
#endif

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
        if (pathParts[i] == ".." && i > 1)
        {
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



bool FileLister::acceptFile(const std::string &filename)
{
    std::string::size_type dotLocation = filename.find_last_of('.');
    if (dotLocation == std::string::npos)
        return false;

    std::string extension = filename.substr(dotLocation);
    std::transform(extension.begin(), extension.end(), extension.begin(), static_cast < int(*)(int) > (std::tolower));

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
////// This code is for __GNUC__ only /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__) && !defined(__MINGW32__)
// gcc / cygwin..
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

///////////////////////////////////////////////////////////////////////////////
////// This code is for Borland C++ and Visual C++ ////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if (defined(__BORLANDC__) || defined(_MSC_VER) || defined(__MINGW32__))

// Here is the catch: cppcheck core is Ansi code (using char type).
// When compiling Unicode targets WinAPI automatically uses *W Unicode versions
// of called functions. So we must convert data given to WinAPI functions from
// ANSI to Unicode. Likewise we must convert data we get from WinAPI from
// Unicode to ANSI.

#if defined(UNICODE)

static bool TransformUcs2ToAnsi(LPCWSTR psUcs, LPSTR psAnsi, int nAnsi)
{
    WideCharToMultiByte(CP_ACP, 0, psUcs, -1, psAnsi, nAnsi, NULL, NULL);
    return true;
}

static bool TransformAnsiToUcs2(LPCSTR psAnsi, LPWSTR psUcs, UINT nUcs)
{
    MultiByteToWideChar(CP_ACP, 0, psAnsi, -1, psUcs, nUcs);
    return true;
}

static BOOL MyIsDirectory(std::string path)
{
    WCHAR * unicodeCleanPath = new WCHAR[path.size() + 1];
    TransformAnsiToUcs2(path.c_str(), unicodeCleanPath,
                        (path.size() * sizeof(WCHAR)) + 1);
    // See http://msdn.microsoft.com/en-us/library/bb773621(VS.85).aspx
    BOOL res = PathIsDirectory(unicodeCleanPath);
    delete [] unicodeCleanPath;
    return res;
}

static HANDLE MyFindFirstFile(std::string path, LPWIN32_FIND_DATA findData)
{
    WCHAR * unicodeOss = new wchar_t[path.size() + 1];
    TransformAnsiToUcs2(path.c_str(), unicodeOss, (path.size() + 1) * sizeof(WCHAR));
    HANDLE hFind = FindFirstFile(unicodeOss, findData);
    delete [] unicodeOss;
    return hFind;
}

#else // defined(UNICODE)

static BOOL MyIsDirectory(std::string path)
{
#ifdef __BORLANDC__
    return (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY);
#else
// See http://msdn.microsoft.com/en-us/library/bb773621(VS.85).aspx
return PathIsDirectory(path.c_str());
#endif
}

static HANDLE MyFindFirstFile(std::string path, LPWIN32_FIND_DATA findData)
{
    HANDLE hFind = FindFirstFile(path.c_str(), findData);
    return hFind;
}

#endif // defined(UNICODE)

void FileLister::recursiveAddFiles(std::vector<std::string> &filenames, const std::string &path, bool recursive)
{
    // oss is the search string passed into FindFirst and FindNext.
    // bdir is the base directory which is used to form pathnames.
    // It always has a trailing backslash available for concatenation.
    std::ostringstream bdir, oss;

    std::string cleanedPath = path;
    std::replace(cleanedPath.begin(), cleanedPath.end(), '/', '\\');

    oss << cleanedPath;

    if (MyIsDirectory(cleanedPath.c_str()))
    {
        char c = cleanedPath[ cleanedPath.size()-1 ];
        switch (c)
        {
        case '\\':
            oss << '*';
            bdir << cleanedPath;
            break;
        case '*':
            bdir << cleanedPath.substr(0, cleanedPath.length() - 1);
            break;
        default:
            oss << "\\*";
            bdir << cleanedPath << '\\';
        }
    }
    else
    {
        std::string::size_type pos;
        pos = cleanedPath.find_last_of('\\');
        if (std::string::npos != pos)
        {
            bdir << cleanedPath.substr(0, pos + 1);
        }
    }

    WIN32_FIND_DATA ffd;
    HANDLE hFind = MyFindFirstFile(oss.str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind)
        return;

    do
    {
        if (ffd.cFileName[0] == '.' || ffd.cFileName[0] == '\0')
            continue;

#if defined(UNICODE)
        char * ansiFfd = new char[wcslen(ffd.cFileName) + 1];
        TransformUcs2ToAnsi(ffd.cFileName, ansiFfd, wcslen(ffd.cFileName) + 1);
#else // defined(UNICODE)
        char * ansiFfd = &ffd.cFileName[0];
#endif // defined(UNICODE)

        std::ostringstream fname;
        fname << bdir.str().c_str() << ansiFfd;

        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            // File

            // If recursive is not used, accept all files given by user
            if (!recursive || FileLister::acceptFile(ansiFfd))
                filenames.push_back(fname.str());
        }
        else if (recursive)
        {
            // Directory
            FileLister::recursiveAddFiles(filenames, fname.str().c_str(), recursive);
        }
#if defined(UNICODE)
        delete [] ansiFfd;
#endif // defined(UNICODE)
    }
    while (FindNextFile(hFind, &ffd) != FALSE);

    if (INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}

#endif

//---------------------------------------------------------------------------

bool FileLister::sameFileName(const char fname1[], const char fname2[])
{
#ifdef __linux__
    return bool(strcmp(fname1, fname2) == 0);
#endif
#ifdef __GNUC__
    return bool(strcasecmp(fname1, fname2) == 0);
#endif
#ifdef __BORLANDC__
    return bool(stricmp(fname1, fname2) == 0);
#endif
#ifdef _MSC_VER
    return bool(_stricmp(fname1, fname2) == 0);
#endif
}

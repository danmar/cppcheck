/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include <sstream>
#include "filelister.h"
#include "path.h"


#ifdef _WIN32

///////////////////////////////////////////////////////////////////////////////
////// This code is WIN32 systems /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#ifndef __BORLANDC__
#include <shlwapi.h>
#endif

// Here is the catch: cppcheck core is Ansi code (using char type).
// When compiling Unicode targets WinAPI automatically uses *W Unicode versions
// of called functions. So we must convert data given to WinAPI functions from
// ANSI to Unicode. Likewise we must convert data we get from WinAPI from
// Unicode to ANSI.

// Note that qmake creates VS project files that define UNICODE but don't
// define _UNICODE! Which means e.g. TCHAR macros don't work properly.

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

static BOOL MyFileExists(std::string path)
{
    WCHAR * unicodeOss = new wchar_t[path.size() + 1];
    TransformAnsiToUcs2(path.c_str(), unicodeOss, (path.size() + 1) * sizeof(WCHAR));
    BOOL result = PathFileExists(unicodeOss);
    delete [] unicodeOss;
    return result;
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

static BOOL MyFileExists(std::string path)
{
#ifdef __BORLANDC__
    DWORD fa = GetFileAttributes(path.c_str());
    BOOL result = FALSE;
    if (fa != INVALID_FILE_ATTRIBUTES && !(fa & FILE_ATTRIBUTE_DIRECTORY))
        result = TRUE;
#else
    BOOL result = PathFileExists(path.c_str());
#endif
    return result;
}

#endif // defined(UNICODE)

void FileLister::recursiveAddFiles(std::vector<std::string> &filenames, std::map<std::string, long> &filesizes, const std::string &path)
{
    // oss is the search string passed into FindFirst and FindNext.
    // bdir is the base directory which is used to form pathnames.
    // It always has a trailing backslash available for concatenation.
    std::ostringstream bdir, oss;

    std::string cleanedPath = Path::toNativeSeparators(path);

    oss << cleanedPath;

    if (MyIsDirectory(cleanedPath)) {
        char c = cleanedPath[ cleanedPath.size()-1 ];
        switch (c) {
        case '\\':
            oss << '*';
            bdir << cleanedPath;
            break;
        case '*':
            bdir << cleanedPath.substr(0, cleanedPath.length() - 1);
            break;
        default:
            oss << "\\*";
            if (cleanedPath != ".")
                bdir << cleanedPath << '\\';
        }
    } else {
        std::string::size_type pos = cleanedPath.find_last_of('\\');
        if (std::string::npos != pos) {
            bdir << cleanedPath.substr(0, pos + 1);
        }
    }

    WIN32_FIND_DATA ffd;
    HANDLE hFind = MyFindFirstFile(oss.str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind)
        return;

    do {
        if (ffd.cFileName[0] == '.' || ffd.cFileName[0] == '\0')
            continue;

#if defined(UNICODE)
        char * ansiFfd = new char[wcslen(ffd.cFileName) + 1];
        TransformUcs2ToAnsi(ffd.cFileName, ansiFfd, wcslen(ffd.cFileName) + 1);
#else // defined(UNICODE)
        const char * ansiFfd = &ffd.cFileName[0];
        if (strchr(ansiFfd,'?')) {
            ansiFfd = &ffd.cAlternateFileName[0];
        }
#endif // defined(UNICODE)

        std::ostringstream fname;
        fname << bdir.str() << ansiFfd;

        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            // File

            // If recursive is not used, accept all files given by user
            if (Path::sameFileName(path,ansiFfd) || Path::acceptFile(ansiFfd)) {
                const std::string nativename = Path::fromNativeSeparators(fname.str());
                filenames.push_back(nativename);
                // Limitation: file sizes are assumed to fit in a 'long'
                filesizes[nativename] = ffd.nFileSizeLow;
            }
        } else {
            // Directory
            FileLister::recursiveAddFiles(filenames, filesizes, fname.str());
        }
#if defined(UNICODE)
        delete [] ansiFfd;
#endif // defined(UNICODE)
    } while (FindNextFile(hFind, &ffd) != FALSE);

    if (INVALID_HANDLE_VALUE != hFind) {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}

bool FileLister::isDirectory(const std::string &path)
{
    return (MyIsDirectory(path) != FALSE);
}

bool FileLister::fileExists(const std::string &path)
{
    return (MyFileExists(path) == TRUE);
}


#else

///////////////////////////////////////////////////////////////////////////////
////// This code is POSIX-style systems ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <glob.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>

// Get absolute path. Returns empty string if path does not exist or other error.
std::string FileLister::getAbsolutePath(const std::string& path)
{
    std::string absolute_path;

#ifdef PATH_MAX
    char buf[PATH_MAX];
    if (realpath(path.c_str(), buf) != NULL)
        absolute_path = buf;
#else
    char *dynamic_buf;
    if ((dynamic_buf = realpath(path.c_str(), NULL)) != NULL) {
        absolute_path = dynamic_buf;
        free(dynamic_buf);
    }
#endif

    return absolute_path;
}

void FileLister::recursiveAddFiles2(std::vector<std::string> &relative,
                                    std::set<std::string> &seen_paths,
                                    std::map<std::string, long> &filesizes,
                                    const std::string &path)
{
    std::ostringstream oss;
    oss << path;
    if (path.length() > 0 && path[path.length()-1] == '/')
        oss << "*";

    glob_t glob_results;
    glob(oss.str().c_str(), GLOB_MARK, 0, &glob_results);
    for (unsigned int i = 0; i < glob_results.gl_pathc; i++) {
        const std::string filename = glob_results.gl_pathv[i];
        if (filename == "." || filename == ".." || filename.length() == 0)
            continue;

        // Determine absolute path. Empty filename if path does not exist
        const std::string absolute_path = getAbsolutePath(filename);
        if (absolute_path.empty())
            continue;

        // Did we already process this entry?
        if (seen_paths.find(absolute_path) != seen_paths.end())
            continue;

        if (filename[filename.length()-1] != '/') {
            // File

            if (Path::sameFileName(path,filename) || Path::acceptFile(filename)) {
                relative.push_back(filename);
                seen_paths.insert(absolute_path);

                struct stat sb;
                if (stat(absolute_path.c_str(), &sb) == 0) {
                    // Limitation: file sizes are assumed to fit in a 'long'
                    filesizes[filename] = static_cast<long>(sb.st_size);
                }
            }
        } else {
            // Directory

            seen_paths.insert(absolute_path);
            recursiveAddFiles2(relative, seen_paths, filesizes, filename);
        }
    }
    globfree(&glob_results);
}


void FileLister::recursiveAddFiles(std::vector<std::string> &filenames, std::map<std::string, long> &filesizes, const std::string &path)
{
    std::set<std::string> seen_paths;
    recursiveAddFiles2(filenames, seen_paths, filesizes, path);
}

bool FileLister::isDirectory(const std::string &path)
{
    bool ret = false;

    glob_t glob_results;
    glob(path.c_str(), GLOB_MARK, 0, &glob_results);
    if (glob_results.gl_pathc == 1) {
        const std::string glob_path = glob_results.gl_pathv[0];
        if (!glob_path.empty() && glob_path[glob_path.size() - 1] == '/') {
            ret = true;
        }
    }
    globfree(&glob_results);

    return ret;
}

bool FileLister::fileExists(const std::string &path)
{
    struct stat statinfo;
    int result = stat(path.c_str(), &statinfo);

    if (result < 0) { // Todo: should check errno == ENOENT?
        // File not found
        return false;
    }

    // Check if file is regular file
    if ((statinfo.st_mode & S_IFMT) == S_IFREG)
        return true;

    return false;
}

#endif

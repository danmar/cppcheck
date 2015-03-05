/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include "path.h"
#include <cstring>
#include <string>
#include <sstream>


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
// of called functions. Thus, we explicitly call *A versions of the functions.

static BOOL MyIsDirectory(const std::string& path)
{
#ifdef __BORLANDC__
    return (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY);
#else
// See http://msdn.microsoft.com/en-us/library/bb773621(VS.85).aspx
    return PathIsDirectoryA(path.c_str());
#endif
}

static HANDLE MyFindFirstFile(const std::string& path, LPWIN32_FIND_DATAA findData)
{
    HANDLE hFind = FindFirstFileA(path.c_str(), findData);
    return hFind;
}

static BOOL MyFileExists(const std::string& path)
{
#ifdef __BORLANDC__
    DWORD fa = GetFileAttributes(path.c_str());
    BOOL result = FALSE;
    if (fa != INVALID_FILE_ATTRIBUTES && !(fa & FILE_ATTRIBUTE_DIRECTORY))
        result = TRUE;
#else
    BOOL result = PathFileExistsA(path.c_str());
#endif
    return result;
}

void FileLister::recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra)
{
    const std::string cleanedPath = Path::toNativeSeparators(path);

    // basedir is the base directory which is used to form pathnames.
    // It always has a trailing backslash available for concatenation.
    std::string basedir;

    // searchPattern is the search string passed into FindFirst and FindNext.
    std::string searchPattern = cleanedPath;

    // The user wants to check all files in a dir
    const bool checkAllFilesInDir = (MyIsDirectory(cleanedPath) != FALSE);

    if (checkAllFilesInDir) {
        char c = cleanedPath[ cleanedPath.size()-1 ];
        switch (c) {
        case '\\':
            searchPattern += '*';
            basedir = cleanedPath;
            break;
        case '*':
            basedir = cleanedPath.substr(0, cleanedPath.length() - 1);
            break;
        default:
            searchPattern += "\\*";
            if (cleanedPath != ".")
                basedir = cleanedPath + '\\';
        }
    } else {
        std::string::size_type pos = cleanedPath.find_last_of('\\');
        if (std::string::npos != pos) {
            basedir = cleanedPath.substr(0, pos + 1);
        }
    }

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = MyFindFirstFile(searchPattern, &ffd);
    if (INVALID_HANDLE_VALUE == hFind)
        return;

    do {
        if (ffd.cFileName[0] == '.' || ffd.cFileName[0] == '\0')
            continue;

        const char* ansiFfd = ffd.cFileName;
        if (strchr(ansiFfd,'?')) {
            ansiFfd = ffd.cAlternateFileName;
        }

        const std::string fname(basedir + ansiFfd);

        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            // File
            const std::string nativename = Path::fromNativeSeparators(fname);

            if (!checkAllFilesInDir || Path::acceptFile(fname, extra)) {
                // Limitation: file sizes are assumed to fit in a 'size_t'
#ifdef _WIN64
                files[nativename] = (static_cast<std::size_t>(ffd.nFileSizeHigh) << 32) | ffd.nFileSizeLow;
#else
                files[nativename] = ffd.nFileSizeLow;
#endif
            }
        } else {
            // Directory
            FileLister::recursiveAddFiles(files, fname, extra);
        }
    } while (FindNextFileA(hFind, &ffd) != FALSE);

    if (INVALID_HANDLE_VALUE != hFind) {
        FindClose(hFind);
    }
}

bool FileLister::isDirectory(const std::string &path)
{
    return (MyIsDirectory(path) != FALSE);
}

bool FileLister::fileExists(const std::string &path)
{
    return (MyFileExists(path) != FALSE);
}


#else

///////////////////////////////////////////////////////////////////////////////
////// This code is POSIX-style systems ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__CYGWIN__)
#undef __STRICT_ANSI__
#endif

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

void FileLister::addFiles2(std::set<std::string> &seen_paths,
                           std::map<std::string, std::size_t> &files,
                           const std::string &path,
                           const std::set<std::string> &extra,
                           bool recursive
                          )
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

            if (Path::sameFileName(path,filename) || Path::acceptFile(filename, extra)) {
                seen_paths.insert(absolute_path);

                struct stat sb;
                if (stat(absolute_path.c_str(), &sb) == 0) {
                    // Limitation: file sizes are assumed to fit in a 'size_t'
                    files[filename] = static_cast<std::size_t>(sb.st_size);
                } else
                    files[filename] = 0;
            }
        } else if (recursive) {
            // Directory

            seen_paths.insert(absolute_path);
            addFiles2(seen_paths, files, filename, extra, recursive);
        }
    }
    globfree(&glob_results);
}


void FileLister::recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra)
{
    std::set<std::string> seen_paths;
    addFiles2(seen_paths, files, path, extra, true);
}

void FileLister::addFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, bool recursive)
{
    std::set<std::string> seen_paths;
    addFiles2(seen_paths, files, path, extra, recursive);
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

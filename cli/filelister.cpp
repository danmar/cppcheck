/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "pathmatch.h"
#include <cstring>
#include <string>


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

void FileLister::recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, const PathMatch& ignored)
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
        char c = cleanedPath.back();
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
        if (std::strchr(ansiFfd,'?')) {
            ansiFfd = ffd.cAlternateFileName;
        }

        const std::string fname(basedir + ansiFfd);

        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            // File
            if ((!checkAllFilesInDir || Path::acceptFile(fname, extra)) && !ignored.Match(fname)) {
                const std::string nativename = Path::fromNativeSeparators(fname);

                // Limitation: file sizes are assumed to fit in a 'size_t'
#ifdef _WIN64
                files[nativename] = (static_cast<std::size_t>(ffd.nFileSizeHigh) << 32) | ffd.nFileSizeLow;
#else
                files[nativename] = ffd.nFileSizeLow;
#endif
            }
        } else {
            // Directory
            if (!ignored.Match(fname))
                FileLister::recursiveAddFiles(files, fname, extra, ignored);
        }
    } while (FindNextFileA(hFind, &ffd) != FALSE);

    FindClose(hFind);
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

#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>


static void addFiles2(std::map<std::string, std::size_t> &files,
                      const std::string &path,
                      const std::set<std::string> &extra,
                      bool recursive,
                      const PathMatch& ignored
                     )
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != -1) {
        if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
            DIR * dir = opendir(path.c_str());

            if (!dir)
                return;

            dirent entry;
            dirent * dir_result;

            std::string new_path;
            new_path.reserve(path.length() + 100);// prealloc some memory to avoid constant new/deletes in loop

            while ((readdir_r(dir, &entry, &dir_result) == 0) && (dir_result != NULL)) {

                if ((std::strcmp(dir_result->d_name, ".") == 0) ||
                    (std::strcmp(dir_result->d_name, "..") == 0))
                    continue;

                new_path = path + '/' + dir_result->d_name;

                if (dir_result->d_type == DT_DIR || (dir_result->d_type == DT_UNKNOWN && FileLister::isDirectory(new_path))) {
                    if (recursive && !ignored.Match(new_path)) {
                        addFiles2(files, new_path, extra, recursive, ignored);
                    }
                } else {
                    if (Path::acceptFile(new_path, extra) && !ignored.Match(new_path)) {
                        files[new_path] = file_stat.st_size;
                    }
                }
            }
            closedir(dir);
        } else
            files[path] = file_stat.st_size;
    }
}

void FileLister::recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, const PathMatch& ignored)
{
    addFiles(files, path, extra, true, ignored);
}

void FileLister::addFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, bool recursive, const PathMatch& ignored)
{
    if (!path.empty()) {
        std::string corrected_path = path;
        if (corrected_path.back() == '/')
            corrected_path.erase(corrected_path.end() - 1);

        addFiles2(files, corrected_path, extra, recursive, ignored);
    }
}

bool FileLister::isDirectory(const std::string &path)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != -1)
        return ((file_stat.st_mode & S_IFMT) == S_IFDIR);

    return false;
}

bool FileLister::fileExists(const std::string &path)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != -1)
        return ((file_stat.st_mode & S_IFMT) == S_IFREG);

    return false;
}

#endif

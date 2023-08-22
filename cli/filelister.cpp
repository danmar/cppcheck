/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "config.h"
#include "path.h"
#include "pathmatch.h"
#include "utils.h"

#include <cstddef>
#include <cstring>
// fix NAME_MAX not found on macOS GCC8.1
#include <climits>
#include <memory>

#ifdef _WIN32

///////////////////////////////////////////////////////////////////////////////
////// This code is WIN32 systems /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <shlwapi.h>

// Here is the catch: cppcheck core is Ansi code (using char type).
// When compiling Unicode targets WinAPI automatically uses *W Unicode versions
// of called functions. Thus, we explicitly call *A versions of the functions.

std::string FileLister::recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, const PathMatch& ignored)
{
    return addFiles(files, path, extra, true, ignored);
}

std::string FileLister::addFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, bool recursive, const PathMatch& ignored)
{
    if (path.empty())
        return "no path specified";

    const std::string cleanedPath = Path::toNativeSeparators(path);

    // basedir is the base directory which is used to form pathnames.
    // It always has a trailing backslash available for concatenation.
    std::string basedir;

    // searchPattern is the search string passed into FindFirst and FindNext.
    std::string searchPattern = cleanedPath;

    // The user wants to check all files in a dir
    const bool checkAllFilesInDir = Path::isDirectory(cleanedPath);

    if (checkAllFilesInDir) {
        const char c = cleanedPath.back();
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
        const std::string::size_type pos = cleanedPath.find_last_of('\\');
        if (std::string::npos != pos) {
            basedir = cleanedPath.substr(0, pos + 1);
        }
    }

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        const DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            // no files matched
            return "";
        }
        return "finding files failed (error: " + std::to_string(err) + ")";
    }
    std::unique_ptr<void, decltype(&FindClose)> hFind_deleter(hFind, FindClose);

    do {
        if (ffd.cFileName[0] != '.' && ffd.cFileName[0] != '\0')
        {
            const char* ansiFfd = ffd.cFileName;
            if (std::strchr(ansiFfd,'?')) {
                ansiFfd = ffd.cAlternateFileName;
            }

            const std::string fname(basedir + ansiFfd);

            if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                // File
                if ((!checkAllFilesInDir || Path::acceptFile(fname, extra)) && !ignored.match(fname)) {
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
                if (recursive) {
                    if (!ignored.match(fname)) {
                        std::string err = FileLister::recursiveAddFiles(files, fname, extra, ignored);
                        if (!err.empty())
                            return err;
                    }
                }
            }
        }

        if (!FindNextFileA(hFind, &ffd)) {
            const DWORD err = GetLastError();
            // no more files matched
            if (err != ERROR_NO_MORE_FILES)
                return "failed to get next file (error: " + std::to_string(err) + ")";
            break;
        }
    } while (true);

    return "";
}

#else

///////////////////////////////////////////////////////////////////////////////
////// This code is POSIX-style systems ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__CYGWIN__)
#undef __STRICT_ANSI__
#endif

#include <dirent.h>
#include <sys/stat.h>
#include <cerrno>

#ifndef NAME_MAX
#ifdef MAXNAMLEN
#define NAME_MAX MAXNAMLEN
#endif
#endif


static std::string addFiles2(std::map<std::string, std::size_t> &files,
                             const std::string &path,
                             const std::set<std::string> &extra,
                             bool recursive,
                             const PathMatch& ignored
                             )
{
    if (ignored.match(path))
        return "";

    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != -1) {
        if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
            DIR * dir = opendir(path.c_str());
            if (!dir) {
                const int err = errno;
                return "could not open directory '" + path + "' (errno: " + std::to_string(err) + ")";
            }
            std::unique_ptr<DIR, decltype(&closedir)> dir_deleter(dir, closedir);

            std::string new_path = path;
            new_path += '/';

            while (const struct dirent* dir_result = readdir(dir)) {
                if ((std::strcmp(dir_result->d_name, ".") == 0) ||
                    (std::strcmp(dir_result->d_name, "..") == 0))
                    continue;

                new_path.erase(path.length() + 1);
                new_path += dir_result->d_name;

#if defined(_DIRENT_HAVE_D_TYPE) || defined(_BSD_SOURCE)
                const bool path_is_directory = (dir_result->d_type == DT_DIR || (dir_result->d_type == DT_UNKNOWN && Path::isDirectory(new_path)));
#else
                const bool path_is_directory = Path::isDirectory(new_path);
#endif
                if (path_is_directory) {
                    if (recursive && !ignored.match(new_path)) {
                        std::string err = addFiles2(files, new_path, extra, recursive, ignored);
                        if (!err.empty()) {
                            return err;
                        }
                    }
                } else {
                    if (Path::acceptFile(new_path, extra) && !ignored.match(new_path)) {
                        if (stat(new_path.c_str(), &file_stat) != -1)
                            files[new_path] = file_stat.st_size;
                        else {
                            const int err = errno;
                            return "could not stat file '" + new_path + "' (errno: " + std::to_string(err) + ")";
                        }
                    }
                }
            }
        } else
            files[path] = file_stat.st_size;
    }
    return "";
}

std::string FileLister::recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, const PathMatch& ignored)
{
    return addFiles(files, path, extra, true, ignored);
}

std::string FileLister::addFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, bool recursive, const PathMatch& ignored)
{
    if (path.empty())
        return "no path specified";

    std::string corrected_path = path;
    if (endsWith(corrected_path, '/'))
        corrected_path.erase(corrected_path.end() - 1);

    return addFiles2(files, corrected_path, extra, recursive, ignored);
}

#endif

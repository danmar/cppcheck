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

#if defined(__GNUC__) && (defined(_WIN32) || defined(__CYGWIN__))
#undef __STRICT_ANSI__
#endif

#include "path.h"
#include "utils.h"

#include <algorithm>
#include <cstdlib>
#include <sys/stat.h>
#include <utility>

#include <simplecpp.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>
#include <windows.h>
#endif
#if defined(__CYGWIN__)
#include <strings.h>
#endif
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif


/** Is the filesystem case insensitive? */
static constexpr bool caseInsensitiveFilesystem()
{
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
    // Windows is case insensitive
    // MacOS is case insensitive by default (also supports case sensitivity)
    return true;
#else
    // TODO: Non-windows filesystems might be case insensitive
    return false;
#endif
}

std::string Path::toNativeSeparators(std::string path)
{
#if defined(_WIN32)
    const char separ = '/';
    const char native = '\\';
#else
    const char separ = '\\';
    const char native = '/';
#endif
    std::replace(path.begin(), path.end(), separ, native);
    return path;
}

std::string Path::fromNativeSeparators(std::string path)
{
    const char nonnative = '\\';
    const char newsepar = '/';
    std::replace(path.begin(), path.end(), nonnative, newsepar);
    return path;
}

std::string Path::simplifyPath(std::string originalPath)
{
    return simplecpp::simplifyPath(std::move(originalPath));
}

std::string Path::getPathFromFilename(const std::string &filename)
{
    const std::size_t pos = filename.find_last_of("\\/");

    if (pos != std::string::npos)
        return filename.substr(0, 1 + pos);

    return "";
}

bool Path::sameFileName(const std::string &fname1, const std::string &fname2)
{
    return caseInsensitiveFilesystem() ? (caseInsensitiveStringCompare(fname1, fname2) == 0) : (fname1 == fname2);
}

std::string Path::removeQuotationMarks(std::string path)
{
    path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());
    return path;
}

std::string Path::getFilenameExtension(const std::string &path, bool lowercase)
{
    const std::string::size_type dotLocation = path.find_last_of('.');
    if (dotLocation == std::string::npos)
        return "";

    std::string extension = path.substr(dotLocation);
    if (lowercase || caseInsensitiveFilesystem()) {
        // on a case insensitive filesystem the case doesn't matter so
        // let's return the extension in lowercase
        strTolower(extension);
    }
    return extension;
}

std::string Path::getFilenameExtensionInLowerCase(const std::string &path)
{
    return getFilenameExtension(path, true);
}

std::string Path::getCurrentPath()
{
    char currentPath[4096];

#ifndef _WIN32
    if (getcwd(currentPath, 4096) != nullptr)
#else
    if (_getcwd(currentPath, 4096) != nullptr)
#endif
        return std::string(currentPath);

    return "";
}

std::string Path::getCurrentExecutablePath(const char* fallback)
{
    char buf[4096] = {};
    bool success{};
#ifdef _WIN32
    success = (GetModuleFileNameA(nullptr, buf, sizeof(buf)) < sizeof(buf));
#elif defined(__APPLE__)
    uint32_t size = sizeof(buf);
    success = (_NSGetExecutablePath(buf, &size) == 0);
#else
    const char* procPath =
#ifdef __SVR4 // Solaris
        "/proc/self/path/a.out";
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
        "/proc/curproc/file";
#else // Linux
        "/proc/self/exe";
#endif
    success = (readlink(procPath, buf, sizeof(buf)) != -1);
#endif
    return success ? std::string(buf) : std::string(fallback);
}

bool Path::isAbsolute(const std::string& path)
{
    const std::string& nativePath = toNativeSeparators(path);

#ifdef _WIN32
    if (path.length() < 2)
        return false;

    // On Windows, 'C:\foo\bar' is an absolute path, while 'C:foo\bar' is not
    return startsWith(nativePath, "\\\\") || (std::isalpha(nativePath[0]) != 0 && nativePath.compare(1, 2, ":\\") == 0);
#else
    return !nativePath.empty() && nativePath[0] == '/';
#endif
}

std::string Path::getRelativePath(const std::string& absolutePath, const std::vector<std::string>& basePaths)
{
    for (const std::string &bp : basePaths) {
        if (absolutePath == bp || bp.empty()) // Seems to be a file, or path is empty
            continue;

        if (absolutePath.compare(0, bp.length(), bp) != 0)
            continue;

        if (endsWith(bp,'/'))
            return absolutePath.substr(bp.length());
        if (absolutePath.size() > bp.size() && absolutePath[bp.length()] == '/')
            return absolutePath.substr(bp.length() + 1);
    }
    return absolutePath;
}

bool Path::isC(const std::string &path)
{
    // In unix, ".C" is considered C++ file
    const std::string extension = getFilenameExtension(path);
    return extension == ".c" ||
           extension == ".cl";
}

bool Path::isCPP(const std::string &path)
{
    const std::string extension = getFilenameExtensionInLowerCase(path);
    return extension == ".cpp" ||
           extension == ".cxx" ||
           extension == ".cc" ||
           extension == ".c++" ||
           extension == ".hpp" ||
           extension == ".hxx" ||
           extension == ".hh" ||
           extension == ".tpp" ||
           extension == ".txx" ||
           extension == ".ipp" ||
           extension == ".ixx" ||
           getFilenameExtension(path) == ".C"; // In unix, ".C" is considered C++ file
}

bool Path::acceptFile(const std::string &path, const std::set<std::string> &extra)
{
    return !Path::isHeader(path) && (Path::isCPP(path) || Path::isC(path) || extra.find(getFilenameExtension(path)) != extra.end());
}

bool Path::isHeader(const std::string &path)
{
    const std::string extension = getFilenameExtensionInLowerCase(path);
    return startsWith(extension, ".h");
}

std::string Path::getAbsoluteFilePath(const std::string& filePath)
{
    std::string absolute_path;
#ifdef _WIN32
    char absolute[_MAX_PATH];
    if (_fullpath(absolute, filePath.c_str(), _MAX_PATH))
        absolute_path = absolute;
#elif defined(__linux__) || defined(__sun) || defined(__hpux) || defined(__GNUC__) || defined(__CPPCHECK__)
    char * absolute = realpath(filePath.c_str(), nullptr);
    if (absolute)
        absolute_path = absolute;
    free(absolute);
#else
#error Platform absolute path function needed
#endif
    return absolute_path;
}

std::string Path::stripDirectoryPart(const std::string &file)
{
#if defined(_WIN32) && !defined(__MINGW32__)
    const char native = '\\';
#else
    const char native = '/';
#endif

    const std::string::size_type p = file.rfind(native);
    if (p != std::string::npos) {
        return file.substr(p + 1);
    }
    return file;
}

#ifdef _WIN32
using mode_t = unsigned short;
#endif

static mode_t file_type(const std::string &path)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) == -1)
        return 0;
    return file_stat.st_mode & S_IFMT;
}

bool Path::isFile(const std::string &path)
{
    return file_type(path) == S_IFREG;
}

bool Path::isDirectory(const std::string &path)
{
    return file_type(path) == S_IFDIR;
}

std::string Path::join(std::string path1, std::string path2) {
    if (path1.empty() || path2.empty())
        return path1 + path2;
    if (path2.front() == '/')
        return path2;
    return ((path1.back() == '/') ? path1 : (path1 + "/")) + path2;
}

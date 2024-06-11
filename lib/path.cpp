/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

//#define LOG_EMACS_MARKER

#include "path.h"
#include "utils.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#ifdef LOG_EMACS_MARKER
#include <iostream>
#endif
#include <sys/stat.h>
#include <unordered_set>
#include <utility>

#include <simplecpp.h>

#ifndef _WIN32
#include <sys/types.h>
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
    constexpr char separ = '/';
    constexpr char native = '\\';
#else
    constexpr char separ = '\\';
    constexpr char native = '/';
#endif
    std::replace(path.begin(), path.end(), separ, native);
    return path;
}

std::string Path::fromNativeSeparators(std::string path)
{
    constexpr char nonnative = '\\';
    constexpr char newsepar = '/';
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

static const std::unordered_set<std::string> cpp_src_exts = {
    ".cpp", ".cxx", ".cc", ".c++", ".tpp", ".txx", ".ipp", ".ixx"
};

static const std::unordered_set<std::string> c_src_exts = {
    ".c", ".cl"
};

static const std::unordered_set<std::string> header_exts = {
    ".h", ".hpp", ".h++", ".hxx", ".hh"
};

bool Path::acceptFile(const std::string &path, const std::set<std::string> &extra)
{
    bool header = false;
    return (identify(path, false, &header) != Standards::Language::None && !header) || extra.find(getFilenameExtension(path)) != extra.end();
}

static bool hasEmacsCppMarker(const char* path)
{
    // TODO: identify is called three times for each file
    // Preprocessor::loadFiles() -> createDUI()
    // Preprocessor::preprocess() -> createDUI()
    // TokenList::createTokens() -> TokenList::determineCppC()
#ifdef LOG_EMACS_MARKER
    std::cout << path << '\n';
#endif

    FILE *fp = fopen(path, "rt");
    if (!fp)
        return false;
    std::string buf(128, '\0');
    {
        // TODO: read the whole first line only
        const char * const res = fgets(const_cast<char*>(buf.data()), buf.size(), fp);
        fclose(fp);
        fp = nullptr;
        if (!res)
            return false; // failed to read file
    }
    // TODO: replace with regular expression
    const auto pos1 = buf.find("-*-");
    if (pos1 == std::string::npos)
        return false; // no start marker
    const auto pos_nl = buf.find_first_of("\r\n");
    if (pos_nl != std::string::npos && (pos_nl < pos1)) {
#ifdef LOG_EMACS_MARKER
        std::cout << path << " - Emacs marker not on the first line" << '\n';
#endif
        return false; // not on first line
    }
    const auto pos2 = buf.find("-*-", pos1 + 3);
    // TODO: make sure we have read the whole line before bailing out
    if (pos2 == std::string::npos) {
#ifdef LOG_EMACS_MARKER
        std::cout << path << " - Emacs marker not terminated" << '\n';
#endif
        return false; // no end marker
    }
#ifdef LOG_EMACS_MARKER
    std::cout << "Emacs marker: '"  << buf.substr(pos1, (pos2 + 3) - pos1) << "'" << '\n';
#endif
    // TODO: support /* */ comments
    const std::string buf_trim = trim(buf); // trim whitespaces
    if (buf_trim[0] == '/' && buf_trim[1] == '*') {
        const auto pos_cmt = buf.find("*/", 2);
        if (pos_cmt != std::string::npos && pos_cmt < (pos2 + 3))
        {
 #ifdef LOG_EMACS_MARKER
            std::cout << path << " - Emacs marker not contained in C-style comment block: '"  << buf.substr(pos1, (pos2 + 3) - pos1) << "'" << '\n';
 #endif
            return false; // not in a comment
        }
    }
    else if (buf_trim[0] != '/' || buf_trim[1] != '/') {
#ifdef LOG_EMACS_MARKER
        std::cout << path << " - Emacs marker not in a comment: '"  << buf.substr(pos1, (pos2 + 3) - pos1) << "'" << '\n';
#endif
        return false; // not in a comment
    }

    // there are more variations with lowercase and no whitespaces
    // -*- C++ -*-
    // -*- Mode: C++; -*-
    // -*- Mode: C++; c-basic-offset: 8 -*-
    std::string marker = trim(buf.substr(pos1 + 3, pos2 - pos1 - 3), " ;");
    // cut off additional attributes
    const auto pos_semi = marker.find(';');
    if (pos_semi != std::string::npos)
        marker.resize(pos_semi);
    findAndReplace(marker, "mode:", "");
    findAndReplace(marker, "Mode:", "");
    marker = trim(marker);
    if (marker == "C++" || marker == "c++") {
        // NOLINTNEXTLINE(readability-simplify-boolean-expr) - TODO: FP
        return true; // C++ marker found
    }

    //if (marker == "C" || marker == "c")
    //    return false;
#ifdef LOG_EMACS_MARKER
    std::cout << path << " - unmatched Emacs marker: '"  << marker << "'" << '\n';
#endif

    return false; // marker is not a C++ one
}

Standards::Language Path::identify(const std::string &path, bool cppHeaderProbe, bool *header)
{
    // cppcheck-suppress uninitvar - TODO: FP
    if (header)
        *header = false;

    std::string ext = getFilenameExtension(path);
    // standard library headers have no extension
    if (cppHeaderProbe && ext.empty()) {
        if (hasEmacsCppMarker(path.c_str())) {
            if (header)
                *header = true;
            return Standards::Language::CPP;
        }
        return Standards::Language::None;
    }
    if (ext == ".C")
        return Standards::Language::CPP;
    if (c_src_exts.find(ext) != c_src_exts.end())
        return Standards::Language::C;
    // cppcheck-suppress knownConditionTrueFalse - TODO: FP
    if (!caseInsensitiveFilesystem())
        strTolower(ext);
    if (ext == ".h") {
        if (header)
            *header = true;
        if (cppHeaderProbe && hasEmacsCppMarker(path.c_str()))
            return Standards::Language::CPP;
        return Standards::Language::C;
    }
    if (cpp_src_exts.find(ext) != cpp_src_exts.end())
        return Standards::Language::CPP;
    if (header_exts.find(ext) != header_exts.end()) {
        if (header)
            *header = true;
        return Standards::Language::CPP;
    }
    return Standards::Language::None;
}

bool Path::isHeader(const std::string &path)
{
    bool header;
    (void)identify(path, false, &header);
    return header;
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
    constexpr char native = '\\';
#else
    constexpr char native = '/';
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

std::string Path::join(const std::string& path1, const std::string& path2) {
    if (path1.empty() || path2.empty())
        return path1 + path2;
    if (path2.front() == '/')
        return path2;
    return ((path1.back() == '/') ? path1 : (path1 + "/")) + path2;
}

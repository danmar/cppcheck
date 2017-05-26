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

//---------------------------------------------------------------------------
#ifndef pathH
#define pathH
//---------------------------------------------------------------------------

#include "config.h"

#include <set>
#include <string>
#include <vector>

/// @addtogroup Core
/// @{


/**
 * @brief Path handling routines.
 * Internally cppcheck wants to store paths with / separator which is also
 * native separator for Unix-derived systems. When giving path to user
 * or for other functions we convert path separators back to native type.
 */
class CPPCHECKLIB Path {
public:
    /**
     * Convert path to use native separators.
     * @param path Path string to convert.
     * @return converted path.
     */
    static std::string toNativeSeparators(std::string path);

    /**
      * Convert path to use internal path separators.
      * @param path Path string to convert.
      * @return converted path.
      */
    static std::string fromNativeSeparators(std::string path);

    /**
     * @brief Simplify path "foo/bar/.." => "foo"
     * @param originalPath path to be simplified, must have / -separators.
     * @return simplified path
     */
    static std::string simplifyPath(std::string originalPath);

    /**
     * @brief Lookup the path part from a filename (e.g., '/tmp/a.h' -> '/tmp/', 'a.h' -> '')
     * @param filename filename to lookup, must have / -separators.
     * @return path part of the filename
     */
    static std::string getPathFromFilename(const std::string &filename);

    /**
     * @brief Compare filenames to see if they are the same.
     * On Linux the comparison is case-sensitive. On Windows it is case-insensitive.
     * @param fname1 one filename
     * @param fname2 other filename
     * @return true if the filenames match on the current platform
     */
    static bool sameFileName(const std::string &fname1, const std::string &fname2);

    /**
     * @brief Remove quotation marks (") from the path.
     * @param path path to be cleaned.
     * @return Cleaned path without quotation marks.
     */
    static std::string removeQuotationMarks(std::string path);

    /**
      * @brief Get an extension of the filename.
      * @param path Path containing filename.
      * @return Filename extension (containing the dot, e.g. ".h" or ".CPP").
      */
    static std::string getFilenameExtension(const std::string &path);

    /**
      * @brief Get an extension of the filename in lower case.
      * @param path Path containing filename.
      * @return Filename extension (containing the dot, e.g. ".h").
      */
    static std::string getFilenameExtensionInLowerCase(const std::string &path);

    /**
     * @brief Returns the absolute path of current working directory
     * @return absolute path of current working directory
     */
    static const std::string getCurrentPath();

    /**
     * @brief Check if given path is absolute
     * @param path Path to check
     * @return true if given path is absolute
     */
    static bool isAbsolute(const std::string& path);

    /**
      * @brief Create a relative path from an absolute one, if absolute path is inside the basePaths.
      * @param absolutePath Path to be made relative.
      * @param basePaths Paths to which it may be made relative.
      * @return relative path, if possible. Otherwise absolutePath is returned unchanged
      */
    static std::string getRelativePath(const std::string& absolutePath, const std::vector<std::string>& basePaths);

    /**
      * @brief Get an absolute file path from a relative one.
      * @param filePath File path to be made absolute.
      * @return absolute path, if possible. Otherwise an empty path is returned
      */
    static std::string getAbsoluteFilePath(const std::string& filePath);

    /**
     * @brief Check if the file extension indicates that it's a C/C++ source file.
     * Check if the file has source file extension: *.c;*.cpp;*.cxx;*.c++;*.cc;*.txx
     * @param filename filename to check. path info is optional
     * @return true if the file extension indicates it should be checked
     */
    static bool acceptFile(const std::string &filename) {
        const std::set<std::string> extra;
        return acceptFile(filename, extra);
    }

    /**
     * @brief Check if the file extension indicates that it's a C/C++ source file.
     * Check if the file has source file extension: *.c;*.cpp;*.cxx;*.c++;*.cc;*.txx
     * @param path filename to check. path info is optional
     * @param extra    extra file extensions
     * @return true if the file extension indicates it should be checked
     */
    static bool acceptFile(const std::string &path, const std::set<std::string> &extra);

    /**
     * @brief Identify language based on file extension.
     * @param path filename to check. path info is optional
     * @return true if extension is meant for C files
     */
    static bool isC(const std::string &path);

    /**
     * @brief Identify language based on file extension.
     * @param path filename to check. path info is optional
     * @return true if extension is meant for C++ files
     */
    static bool isCPP(const std::string &path);

    /**
     * @brief Is filename a header based on file extension
     * @param path filename to check. path info is optional
     * @return true if filename extension is meant for headers
     */
    static bool isHeader(const std::string &path);
};

/// @}
//---------------------------------------------------------------------------
#endif // pathH

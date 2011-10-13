/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#ifndef PATH_H_INCLUDED
#define PATH_H_INCLUDED

#include <string>

/// @addtogroup Core
/// @{


/**
 * @brief Path handling routines.
 * Internally cppcheck wants to store paths with / separator which is also
 * native separator for Unix-derived systems. When giving path to user
 * or for other functions we convert path separators back to native type.
 */
class Path {
public:
    /**
     * Convert path to use native separators.
     * @param path Path string to convert.
     * @return converted path.
     */
    static std::string toNativeSeparators(const std::string &path);

    /**
      * Convert path to use internal path separators.
      * @param path Path string to convert.
      * @return converted path.
      */
    static std::string fromNativeSeparators(const std::string &path);

    /**
     * @brief Simplify path "foo/bar/.." => "foo"
     * @param originalPath path to be simplified, must have / -separators.
     * @return simplified path
     */
    static std::string simplifyPath(const char *originalPath);

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
    static std::string removeQuotationMarks(const std::string &path);

    /**
      * @brief Get an extension of the filename.
      * @param path Path containing filename.
      * @return Filename extension (containing the dot, e.g. ".h").
      */
    static std::string getFilenameExtension(const std::string &path);
};

/// @}

#endif // PATH_H_INCLUDED

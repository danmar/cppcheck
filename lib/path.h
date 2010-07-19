/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
class Path
{
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
      * Simplify given path.
      * This method simplifies the path removing reduntant parts.
      * @param path Path string to simplify.
      * @return simplified path.
      */
    static std::string simplifyPath(const std::string &path);
};

/// @}

#endif // PATH_H_INCLUDED

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

#ifndef FileListerH
#define FileListerH

#include <vector>
#include <string>

/// @addtogroup Core
/// @{


// Check that the compiler are supported
// This program should be compiled with either GCC/BORLAND/MSC to work..
#ifndef __GNUC__
#ifndef  __BORLANDC__
#ifndef _MSC_VER
#error "Cppcheck must be compiled by either GCC/BORLAND/MSC to work fully.\n"
#error "Please report that you couldn't compile cppcheck through the web page:\n"
#error "      https://sourceforge.net/projects/cppcheck/"
#endif
#endif
#endif


class FileLister
{
public:
    static void recursiveAddFiles(std::vector<std::string> &filenames, const std::string &path, bool recursive);
    static std::string simplifyPath(const char *originalPath);
    static bool sameFileName(const char fname1[], const char fname2[]);
    static bool acceptFile(const std::string &filename);
private:

};

/// @}

#endif // #ifndef FILELISTER_H

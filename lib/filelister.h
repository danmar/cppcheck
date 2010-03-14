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

/** @brief Base class for Cppcheck filelisters. Used to recursively search for source files. This class defines a platform independant interface and subclasses will provide platform dependant implementation. */
class FileLister
{
public:
    /**
     * @brief Add source files to a vector (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx)
     * @param filenames output vector that filenames are written to
     * @param path root path
     * @param recursive Should files be added recursively or not?
     */
    virtual void recursiveAddFiles(std::vector<std::string> &filenames, const std::string &path, bool recursive) = 0;

    /**
     * @brief simplify path "foo/bar/.." => "foo"
     * @param originalPath path to be simplified
     * @return simplified path
     */
    virtual std::string simplifyPath(const char *originalPath);

    /**
     * @brief compare filenames to see if they are the same. On Linux the comparison is case-sensitive. On Windows it is case-insensitive.
     * @param fname1 one filename
     * @param fname2 other filename
     * @return true if the filenames match on the current platform
     */
    virtual bool sameFileName(const std::string &fname1, const std::string &fname2) = 0;

    /**
     * @brief check if the file extension indicates that it's a source file - *.c;*.cpp;*.cxx;*.c++;*.cc;*.txx
     * @param filename filename
     * @return returns true if the file extension indicates it should be checked
     */
    virtual bool acceptFile(const std::string &filename);

};

/** @brief get filelister (platform dependent implementation) */
FileLister * getFileLister();

/// @}

#endif // #ifndef FileListerH

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

#ifndef FileListerH
#define FileListerH

#include <vector>
#include <string>

/// @addtogroup Core
/// @{

/**
 * @brief Base class for Cppcheck filelisters.
 * Used to recursively search for source files. This class defines a platform
 * independant interface and subclasses will provide platform dependant
 * implementation. */
class FileLister
{
public:
    /**
     * @brief destructor of class filelister
     */
    virtual ~FileLister() {}

    /**
     * @brief Recursively add source files to a vector.
     * Add source files from given directory and all subdirectries to the
     * given vector. Only files with accepted extensions
     * (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx) are added.
     * @param filenames output vector that filenames are written to
     * @param path root path
     */
    virtual void recursiveAddFiles(std::vector<std::string> &filenames,
                                   const std::string &path) = 0;

    /**
     * @brief Compare filenames to see if they are the same.
     * On Linux the comparison is case-sensitive. On Windows it is case-insensitive.
     * @param fname1 one filename
     * @param fname2 other filename
     * @return true if the filenames match on the current platform
     */
    virtual bool sameFileName(const std::string &fname1, const std::string &fname2) = 0;

    /**
     * @brief Check if the file extension indicates that it's a source file.
     * Check if the file has source file extension: *.c;*.cpp;*.cxx;*.c++;*.cc;*.txx
     * @param filename filename to check
     * @return returns true if the file extension indicates it should be checked
     */
    virtual bool acceptFile(const std::string &filename);

};

/** @brief get filelister (platform dependent implementation) */
FileLister * getFileLister();

/// @}

#endif // #ifndef FileListerH

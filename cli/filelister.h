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

#ifndef filelisterH
#define filelisterH

#include <vector>
#include <string>
#include <map>

/// @addtogroup CLI
/// @{

/** @brief Cross-platform FileLister */
class FileLister {
public:
    /**
     * @brief Recursively add source files to a vector.
     * Add source files from given directory and all subdirectries to the
     * given vector. Only files with accepted extensions
     * (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx) are added.
     * @param filenames output vector that filenames are written to
     * @param filesizes output map that contains the size of each file
     * @param path root path
     */
    static void recursiveAddFiles(std::vector<std::string> &filenames,
                                  std::map<std::string, long> &filesizes,
                                  const std::string &path);

    /**
     * @brief Check if the file extension indicates that it's a source file.
     * Check if the file has source file extension: *.c;*.cpp;*.cxx;*.c++;*.cc;*.txx
     * @param filename filename to check
     * @return returns true if the file extension indicates it should be checked
     */
    static bool acceptFile(const std::string &filename);

    /**
     * @brief Is given path a directory?
     * @return returns true if the path is a directory
     */
    static bool isDirectory(const std::string &path);

    /**
      * @brief Check if the given path is a file and if it exists?
      * @return true if path points to file and the file exists.
      */
    static bool fileExists(const std::string &path);

#ifndef _WIN32
    static void recursiveAddFiles2(std::vector<std::string> &relative,
                                   std::vector<std::string> &absolute,
                                   std::map<std::string, long> &filesizes,
                                   const std::string &path);
#endif
};

/// @}

#endif // #ifndef filelisterH

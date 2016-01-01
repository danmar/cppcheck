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

#ifndef filelisterH
#define filelisterH

#include <string>
#include <set>
#include <map>

class PathMatch;

/// @addtogroup CLI
/// @{

/** @brief Cross-platform FileLister */
class FileLister {
public:
    /**
     * @brief Recursively add source files to a map.
     * Add source files from given directory and all subdirectries to the
     * given map. Only files with accepted extensions
     * (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx) are added.
     * @param files output map that associates the size of each file with its name
     * @param path root path
     * @param ignored ignored paths
     */
    static void recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const PathMatch& ignored) {
        const std::set<std::string> extra;
        recursiveAddFiles(files, path, extra, ignored);
    }

    /**
     * @brief Recursively add source files to a map.
     * Add source files from given directory and all subdirectries to the
     * given map. Only files with accepted extensions
     * (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx) are added.
     * @param files output map that associates the size of each file with its name
     * @param path root path
     * @param extra Extra file extensions
     * @param ignored ignored paths
     */
    static void recursiveAddFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, const PathMatch& ignored);

    /**
     * @brief (Recursively) add source files to a map.
     * Add source files from given directory and all subdirectries to the
     * given map. Only files with accepted extensions
     * (*.c;*.cpp;*.cxx;*.c++;*.cc;*.txx) are added.
     * @param files output map that associates the size of each file with its name
     * @param path root path
     * @param extra Extra file extensions
     * @param recursive Enable recursion
     * @param ignored ignored paths
     */
    static void addFiles(std::map<std::string, std::size_t> &files, const std::string &path, const std::set<std::string> &extra, bool recursive, const PathMatch& ignored);

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
};

/// @}

#endif // #ifndef filelisterH

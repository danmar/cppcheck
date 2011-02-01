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

#ifndef FileListerUnixH
#define FileListerUnixH

#include <vector>
#include <string>
#include "filelister.h"

/// @addtogroup CLI
/// @{


class FileListerUnix : public FileLister
{
public:
    virtual void recursiveAddFiles(std::vector<std::string> &filenames, const std::string &path);
    virtual bool isDirectory(const std::string &path);
private:
#ifndef _WIN32
    void recursiveAddFiles2(std::vector<std::string> &relative,
                            std::vector<std::string> &absolute,
                            const std::string &path);
#endif
};

/// @}

#endif // #ifndef FileListerUnixH

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

#include <sstream>
#include <vector>
#include <cstring>
#include <string>
#include <algorithm>

#ifndef _WIN32 // POSIX-style system
#include <glob.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#endif

#ifndef _WIN32

#include "path.h"
#include "filelister.h"
#include "filelister_unix.h"

///////////////////////////////////////////////////////////////////////////////
////// This code is POSIX-style systems ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void FileListerUnix::recursiveAddFiles2(std::vector<std::string> &relative,
                                        std::vector<std::string> &absolute,
                                        const std::string &path)
{
    std::ostringstream oss;
    oss << path;
    if (path.length() > 0 && path[path.length()-1] == '/')
        oss << "*";

    glob_t glob_results;
    glob(oss.str().c_str(), GLOB_MARK, 0, &glob_results);
    for (unsigned int i = 0; i < glob_results.gl_pathc; i++)
    {
        const std::string filename = glob_results.gl_pathv[i];
        if (filename == "." || filename == ".." || filename.length() == 0)
            continue;

        if (filename[filename.length()-1] != '/')
        {
            // File
            char fname[PATH_MAX];
            if (realpath(filename.c_str(), fname) == NULL)
            {
                continue;
            }

            // Does absolute path exist? then bail out
            if (std::find(absolute.begin(), absolute.end(), std::string(fname)) != absolute.end())
            {
                continue;
            }

            if (Path::sameFileName(path,filename) || FileListerUnix::acceptFile(filename))
            {
                relative.push_back(filename);
                absolute.push_back(fname);
            }
        }
        else
        {
            // Directory
            recursiveAddFiles2(relative, absolute, filename);
        }
    }
    globfree(&glob_results);
}


void FileListerUnix::recursiveAddFiles(std::vector<std::string> &filenames, const std::string &path)
{
    std::vector<std::string> abs;
    recursiveAddFiles2(filenames, abs, path);
}

bool FileListerUnix::isDirectory(const std::string &path)
{
    bool ret = false;

    glob_t glob_results;
    glob(path.c_str(), GLOB_MARK, 0, &glob_results);
    if (glob_results.gl_pathc == 1)
    {
        const std::string glob_path = glob_results.gl_pathv[0];
        if (!glob_path.empty() && glob_path[glob_path.size() - 1] == '/')
        {
            ret = true;
        }
    }
    globfree(&glob_results);

    return ret;
}

#endif // _WIN32

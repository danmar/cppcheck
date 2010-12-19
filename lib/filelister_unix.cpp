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

#include <sstream>
#include <vector>
#include <cstring>
#include <string>
#include <algorithm>

#ifndef _WIN32 // POSIX-style system
#include <glob.h>
#include <unistd.h>
#include <stdlib.h>		// canonicalize_file_name
#endif

#ifndef _WIN32

#include "filelister.h"
#include "filelister_unix.h"

///////////////////////////////////////////////////////////////////////////////
////// This code is POSIX-style systems ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FileListerUnix::recursiveAddFiles(std::vector<std::string> &filenames, const std::string &path)
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
            char * const fname = canonicalize_file_name(filename.c_str());
            if (!fname)
                continue;

            if (std::find(filenames.begin(), filenames.end(), std::string(fname)) != filenames.end())
            {
                free(fname);
                continue;
            }

            if (sameFileName(path,fname) || FileLister::acceptFile(filename))
                filenames.push_back(fname);

            free(fname);
        }
        else
        {
            // Directory
            getFileLister()->recursiveAddFiles(filenames, filename);
        }
    }
    globfree(&glob_results);
}

bool FileListerUnix::sameFileName(const std::string &fname1, const std::string &fname2)
{
#if defined(__linux__) || defined(__sun)
    return bool(fname1 == fname2);
#endif
#ifdef __GNUC__
    return bool(strcasecmp(fname1.c_str(), fname2.c_str()) == 0);
#endif
}

#endif // _WIN32

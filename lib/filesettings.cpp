/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "filesettings.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>

#ifdef _WIN32

#ifdef _WIN64

std::string FileWithDetails::updateSize() {
    struct _stati64 buf;
    if (_stati64(mPath.c_str(), &buf) < 0) {
        return "could not stat file '" + mPath + "': (errno: " + std::to_string(errno) + ")";
    }
    mSize = buf.st_size;
    return "";
}

#else

std::string FileWithDetails::updateSize() {
    struct _stat buf;
    if (_stat(mPath.c_str(), &buf) < 0) {
        return "could not stat file '" + mPath + "': (errno: " + std::to_string(errno) + ")";
    }
    mSize = buf.st_size;
    return "";
}

#endif

#else

std::string FileWithDetails::updateSize() {
    struct stat buf;
    if (stat(mPath.c_str(), &buf) < 0) {
        return "could not stat file '" + mPath + "': (errno: " + std::to_string(errno) + ")";
    }
    mSize = buf.st_size;
    return "";
}

#endif

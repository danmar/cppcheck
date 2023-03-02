/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "color.h"

#ifndef _WIN32
#include <unistd.h>
#include <cstddef>
#include <sstream> // IWYU pragma: keep
#endif

bool gDisableColors = false;

#ifndef _WIN32
static bool isStdOutATty()
{
    // TODO: handle piping into file as well as other pipes like stderr
    static const bool stdout_tty = isatty(STDOUT_FILENO);
    return stdout_tty;
}
#endif

std::ostream& operator<<(std::ostream & os, const Color& c)
{
#ifndef _WIN32
    if (!gDisableColors && isStdOutATty())
        return os << "\033[" << static_cast<std::size_t>(c) << "m";
#else
    (void)c;
#endif
    return os;
}

std::string toString(const Color& c)
{
#ifndef _WIN32
    std::stringstream ss;
    ss << c;
    return ss.str();
#else
    (void)c;
    return "";
#endif
}


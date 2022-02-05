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
#endif
#include <cstddef>
#include <sstream> // IWYU pragma: keep

#ifdef _WIN32
std::ostream& operator<<(std::ostream& os, const Color& /*c*/)
{
#else
std::ostream& operator<<(std::ostream & os, const Color& c)
{
    static const bool use_color = isatty(STDOUT_FILENO);
    if (use_color)
        return os << "\033[" << static_cast<std::size_t>(c) << "m";
#endif
    return os;
}

std::string toString(const Color& c)
{
    std::stringstream ss;
    ss << c;
    return ss.str();
}


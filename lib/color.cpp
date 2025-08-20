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

#include "color.h"

#include <cstdlib>
#include <sstream>
#include <iostream>

bool getForcedColorSetting(bool &colorSetting)
{
    // See https://bixense.com/clicolors/
    static const bool color_forced_off = (nullptr != std::getenv("NO_COLOR"));
    if (color_forced_off)
    {
        colorSetting = false;
        return true;
    }
    static const bool color_forced_on = (nullptr != std::getenv("CLICOLOR_FORCE"));
    if (color_forced_on)
    {
        colorSetting = true;
        return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream & os, Color c)
{
    return os << "\033[" << static_cast<std::size_t>(c) << "m";
}

std::string toString(Color c)
{
    std::ostringstream ss;
    ss << c;
    return ss.str();
}


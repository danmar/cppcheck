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

#include <iostream>
#include <fstream>

#include "type2.h"

int main(int argc, char **argv)
{
    const char *filename = argc==2 ? argv[1] : nullptr;

    if (!filename) {
        std::cout << "Invalid args, no filename\n";
        return 1;
    }

    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cout << "failed to open file:" << filename << "\n";
        return 1;
    }

    std::string str((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>());

    std::cout << generateCode2(reinterpret_cast<const uint8_t *>(str.data()), str.size()) << std::endl;

    return 0;
}


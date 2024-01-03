/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#ifndef xmlH
#define xmlH

#include "config.h"

SUPPRESS_WARNING_CLANG_PUSH("-Wzero-as-null-pointer-constant")
SUPPRESS_WARNING_CLANG_PUSH("-Wextra-semi-stmt")
SUPPRESS_WARNING_CLANG_PUSH("-Wsuggest-override")
SUPPRESS_WARNING_CLANG_PUSH("-Wsuggest-destructor-override")

#include <tinyxml2.h>

SUPPRESS_WARNING_CLANG_POP
SUPPRESS_WARNING_CLANG_POP
SUPPRESS_WARNING_CLANG_POP
SUPPRESS_WARNING_CLANG_POP

#endif // xmlH

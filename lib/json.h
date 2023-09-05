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

#ifndef jsonH
#define jsonH

SUPPRESS_WARNING_PUSH("-Wfloat-equal")
SUPPRESS_WARNING_CLANG_PUSH("-Wtautological-type-limit-compare")
SUPPRESS_WARNING_GCC_PUSH("-Wparentheses")

#define PICOJSON_USE_INT64
#include <picojson.h>

SUPPRESS_WARNING_GCC_POP
SUPPRESS_WARNING_CLANG_POP
SUPPRESS_WARNING_POP

#endif // jsonH

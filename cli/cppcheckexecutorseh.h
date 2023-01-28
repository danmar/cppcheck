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

#ifndef CPPCHECKEXECUTORSEH_H
#define CPPCHECKEXECUTORSEH_H

#include "config.h" // IWYU pragma: keep

#ifdef USE_WINDOWS_SEH

class CppCheckExecutor;
class CppCheck;

int check_wrapper_seh(CppCheckExecutor& executor, int (CppCheckExecutor::*f)(CppCheck&), CppCheck& cppcheck);

#endif

#endif // CPPCHECKEXECUTORSEH_H

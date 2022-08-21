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

#ifndef STACKTRACE_H
#define STACKTRACE_H

#include "config.h"

#ifdef USE_UNIX_BACKTRACE_SUPPORT

#include <cstdio>

/*
 * Try to print the callstack.
 * That is very sensitive to the operating system, hardware, compiler and runtime.
 * The code is not meant for production environment!
 * One reason is named first: it's using functions not whitelisted for usage in a signal handler function.
 */
void print_stacktrace(FILE* output, bool demangling, int maxdepth, bool lowMem);

#endif

#endif // STACKTRACE_H

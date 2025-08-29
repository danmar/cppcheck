/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

// For a release version x.y.z the MAJOR should be x and both MINOR and DEVMINOR should be y.
// After a release the DEVMINOR is incremented. MAJOR=x MINOR=y, DEVMINOR=y+1

#ifndef versionH
#define versionH

#define CPPCHECK_MAJOR_VERSION 2
#define CPPCHECK_MINOR_VERSION 18
#define CPPCHECK_DEVMINOR_VERSION 18
#define CPPCHECK_BUGFIX_VERSION 0

#define STRINGIFY(x) STRING(x)
#define STRING(VER) #VER
#if CPPCHECK_BUGFIX_VERSION < 99
#define CPPCHECK_VERSION_STRING "2.18.2"
#define CPPCHECK_VERSION 2,18,2,0
#else
#define CPPCHECK_VERSION_STRING "2.18.2"
#define CPPCHECK_VERSION 2,18,2,0
#endif
#define LEGALCOPYRIGHT L"Copyright (C) 2007-2025 Cppcheck team."

#endif

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Marek Zmys³owski and Cppcheck team.
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
#ifndef StandardsH
#define StandardsH

/// @addtogroup Core
/// @{


/**
 * @brief This is just a container for standards settings.
 * This struct contains all possible standards that cppcheck recognize.
 */
struct Standards {
    /** Code is C99 standard */
    bool c99;

    /** Code is posix */
    bool posix;

    /** Code follows C++11 standard */
    bool cpp11;

    /** This constructor clear all the variables **/
    Standards() : c99(false), posix(false), cpp11(false) {};
};

/// @}

#endif // STANDARDS_H
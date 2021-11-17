/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

//---------------------------------------------------------------------------
#ifndef standardsH
#define standardsH
//---------------------------------------------------------------------------

#include <string>

/// @addtogroup Core
/// @{


/**
 * @brief This is just a container for standards settings.
 * This struct contains all possible standards that cppcheck recognize.
 */
struct Standards {
    /** C code C89/C99/C11 standard */
    enum cstd_t { C89, C99, C11, CLatest=C11 } c;

    /** C++ code standard */
    enum cppstd_t { CPP03, CPP11, CPP14, CPP17, CPP20, CPPLatest=CPP20 } cpp;

    /** --std value given on command line */
    std::string stdValue;

    /** This constructor clear all the variables **/
    Standards() : c(C11), cpp(CPPLatest) {}

    bool setC(const std::string& str) {
        stdValue = str;
        if (str == "c89" || str == "C89") {
            c = C89;
            return true;
        }
        if (str == "c99" || str == "C99") {
            c = C99;
            return true;
        }
        if (str == "c11" || str == "C11") {
            c = C11;
            return true;
        }
        return false;
    }
    const std::string getC() const {
        switch (c) {
        case C89:
            return "c89";
        case C99:
            return "c99";
        case C11:
            return "c11";
        }
        return "";
    }
    bool setCPP(const std::string& str) {
        stdValue = str;
        if (str == "c++03" || str == "C++03") {
            cpp = CPP03;
            return true;
        }
        if (str == "c++11" || str == "C++11") {
            cpp = CPP11;
            return true;
        }
        if (str == "c++14" || str == "C++14") {
            cpp = CPP14;
            return true;
        }
        if (str == "c++17" || str == "C++17") {
            cpp = CPP17;
            return true;
        }
        if (str == "c++20" || str == "C++20") {
            cpp = CPP20;
            return true;
        }
        return false;
    }
    std::string getCPP() const {
        switch (cpp) {
        case CPP03:
            return "c++03";
        case CPP11:
            return "c++11";
        case CPP14:
            return "c++14";
        case CPP17:
            return "c++17";
        case CPP20:
            return "c++20";
        }
        return "";
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // standardsH

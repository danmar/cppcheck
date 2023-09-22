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

//---------------------------------------------------------------------------
#ifndef standardsH
#define standardsH
//---------------------------------------------------------------------------

#include "utils.h"

#include <string>

/// @addtogroup Core
/// @{


/**
 * @brief This is just a container for standards settings.
 * This struct contains all possible standards that cppcheck recognize.
 */
struct Standards {
    /** C code standard */
    enum cstd_t { C89, C99, C11, CLatest = C11 } c = CLatest;

    /** C++ code standard */
    enum cppstd_t { CPP03, CPP11, CPP14, CPP17, CPP20, CPP23, CPPLatest = CPP23 } cpp = CPPLatest;

    /** --std value given on command line */
    std::string stdValue;

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
    static cstd_t getC(const std::string &std) {
        if (std == "c89") {
            return Standards::C89;
        }
        if (std == "c99") {
            return Standards::C99;
        }
        if (std == "c11") {
            return Standards::C11;
        }
        return Standards::CLatest;
    }
    bool setCPP(std::string str) {
        stdValue = str;
        strTolower(str);
        cpp = getCPP(str);
        return !stdValue.empty() && str == getCPP();
    }
    std::string getCPP() const {
        return getCPP(cpp);
    }
    static std::string getCPP(cppstd_t std) {
        switch (std) {
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
        case CPP23:
            return "c++23";
        }
        return "";
    }
    static cppstd_t getCPP(const std::string &std) {
        if (std == "c++03") {
            return Standards::CPP03;
        }
        if (std == "c++11") {
            return Standards::CPP11;
        }
        if (std == "c++14") {
            return Standards::CPP14;
        }
        if (std == "c++17") {
            return Standards::CPP17;
        }
        if (std == "c++20") {
            return Standards::CPP20;
        }
        if (std == "c++23") {
            return Standards::CPP23;
        }
        return Standards::CPPLatest;
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // standardsH

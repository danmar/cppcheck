/* -*- C++ -*-
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

//---------------------------------------------------------------------------
#ifndef standardsH
#define standardsH
//---------------------------------------------------------------------------

#include "utils.h"

#include <cstdint>
#include <string>

/// @addtogroup Core
/// @{


/**
 * @brief This is just a container for standards settings.
 * This struct contains all possible standards that cppcheck recognize.
 */
struct Standards {
    enum Language : std::uint8_t { None, C, CPP };

    /** C code standard */
    enum cstd_t : std::uint8_t { C89, C99, C11, C17, C23, CLatest = C23 } c = CLatest;

    /** C++ code standard */
    enum cppstd_t : std::uint8_t { CPP03, CPP11, CPP14, CPP17, CPP20, CPP23, CPP26, CPPLatest = CPP26 } cpp = CPPLatest;

    /** --std value given on command line */
    std::string stdValue;

    bool setC(std::string str) {
        stdValue = str;
        strTolower(str);
        c = getC(str);
        return !stdValue.empty() && str == getC();
    }
    std::string getC() const {
        switch (c) {
        case C89:
            return "c89";
        case C99:
            return "c99";
        case C11:
            return "c11";
        case C17:
            return "c17";
        case C23:
            return "c23";
        }
        return "";
    }
    static cstd_t getC(const std::string &std) {
        bool _unused;
        return getC(std, _unused);
    }
    static cstd_t getC(const std::string &std, bool &unknown) {
        unknown = false;
        if (std == "c89") {
            return Standards::C89;
        }
        if (std == "c99") {
            return Standards::C99;
        }
        if (std == "c11") {
            return Standards::C11;
        }
        if (std == "c17") {
            return Standards::C17;
        }
        if (std == "c23") {
            return Standards::C23;
        }
        unknown = true;
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
        case CPP26:
            return "c++26";
        }
        return "";
    }
    static cppstd_t getCPP(const std::string &std) {
        bool _unused;
        return getCPP(std, _unused);
    }
    static cppstd_t getCPP(const std::string &std, bool &unknown) {
        unknown = false;
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
        if (std == "c++26") {
            return Standards::CPP26;
        }
        unknown = true;
        return Standards::CPPLatest;
    }
    static cppstd_t getGnuCPP(const std::string &std, bool &unknown) {
        // treat gnu++XX as c++XX
        unknown = false;
        if (std == "gnu++03") {
            return Standards::CPP03;
        }
        if (std == "gnu++11") {
            return Standards::CPP11;
        }
        if (std == "gnu++14") {
            return Standards::CPP14;
        }
        if (std == "gnu++17") {
            return Standards::CPP17;
        }
        if (std == "gnu++20") {
            return Standards::CPP20;
        }
        if (std == "gnu++23") {
            return Standards::CPP23;
        }
        if (std == "gnu++26") {
            return Standards::CPP26;
        }
        unknown = true;
        return Standards::CPPLatest;
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // standardsH

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

#include <simplecpp.h>

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
    std::string stdValueC;

    /** --std value given on command line */
    std::string stdValueCPP;

    bool setC(std::string str) {
        if (str.empty())
            return false;
        const simplecpp::cstd_t c_new = simplecpp::getCStd(str);
        const bool b = (c_new != simplecpp::CUnknown);
        if (b) {
            c = mapC(c_new);
            stdValueC = std::move(str);
        }
        return b;
    }
    std::string getC() const {
        return getC(c);
    }
    static std::string getC(cstd_t c_std) {
        switch (c_std) {
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
        return mapC(simplecpp::getCStd(std));
    }
    bool setCPP(std::string str) {
        if (str.empty())
            return false;
        const simplecpp::cppstd_t cpp_new = simplecpp::getCppStd(str);
        const bool b = (cpp_new != simplecpp::CPPUnknown);
        if (b) {
            cpp = mapCPP(cpp_new);
            stdValueCPP = std::move(str);
        }
        return b;
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
        return mapCPP(simplecpp::getCppStd(std));
    }
    bool setStd(const std::string& str) {
        return setC(str) || setCPP(str);
    }

private:
    static cstd_t mapC(simplecpp::cstd_t cstd) {
        switch (cstd)
        {
        case simplecpp::C89:
            return C89;
        case simplecpp::C99:
            return C99;
        case simplecpp::C11:
            return C11;
        case simplecpp::C17:
            return C17;
        case simplecpp::C23:
            return C23;
        case simplecpp::CUnknown:
            return CLatest; // TODO: bail out?
        }
    }

    static cppstd_t mapCPP(simplecpp::cppstd_t cppstd) {
        switch (cppstd)
        {
        case simplecpp::CPP03:
            return CPP03;
        case simplecpp::CPP11:
            return CPP11;
        case simplecpp::CPP14:
            return CPP14;
        case simplecpp::CPP17:
            return CPP17;
        case simplecpp::CPP20:
            return CPP20;
        case simplecpp::CPP23:
            return CPP23;
        case simplecpp::CPP26:
            return CPP26;
        case simplecpp::CPPUnknown:
            return CPPLatest; // TODO: bail out?
        }
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // standardsH

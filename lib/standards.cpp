/*
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

#include "standards.h"

#include <utility>

bool Standards::setC(std::string str)
{
    if (str.empty())
        return false;
    const cstd_t c_new = getC(str);
    const bool b = (str == getC(c_new));
    if (b) {
        c = c_new;
        stdValueC = std::move(str);
    }
    return b;
}

std::string Standards::getC() const
{
    return getC(c);
}

std::string Standards::getC(cstd_t c_std)
{
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

Standards::cstd_t Standards::getC(const std::string &std)
{
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
    return Standards::CLatest;
}

bool Standards::setCPP(std::string str)
{
    if (str.empty())
        return false;
    const cppstd_t cpp_new = getCPP(str);
    const bool b = (str == getCPP(cpp_new));
    if (b) {
        cpp = cpp_new;
        stdValueCPP = std::move(str);
    }
    return b;
}

std::string Standards::getCPP() const
{
    return getCPP(cpp);
}

std::string Standards::getCPP(cppstd_t std)
{
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

Standards::cppstd_t Standards::getCPP(const std::string &std)
{
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
    return Standards::CPPLatest;
}

bool Standards::setStd(const std::string& str)
{
    return setC(str) || setCPP(str);
}

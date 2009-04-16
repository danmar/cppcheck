/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki, Nguyen Duong Tuan
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



#include "mathlib.h"


#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>

long MathLib::toLongNumber(const std::string &str)
{
    if (strncmp(str.c_str(), "0x", 2) == 0)
    {
        return strtoul(str.c_str(), '\0', 16);
    }
    if (strncmp(str.c_str(), "0", 1) == 0)
    {
        return strtoul(str.c_str(), '\0', 8);
    }
    return atol(str.c_str());
}

double MathLib::toDoubleNumber(const std::string &str)
{
    return atof(str.c_str());
}

template <typename T>
std::string MathLib::toString(T d)
{
    std::ostringstream result;
    result << d;
    return result.str();
}

bool MathLib::isInt(const std::string & str)
{
    if (str.find(".", 0) != std::string::npos || str.find("e", 0) != std::string::npos
        || str.find("E", 0) != std::string::npos)
    {
        return false;
    }
    return true;
}

std::string MathLib::add(const std::string & first, const std::string & second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second))
    {
        return toString<long>(toLongNumber(first) + toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) + toDoubleNumber(second));
}

std::string MathLib::subtract(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second))
    {
        return toString<long>(toLongNumber(first) - toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) - toDoubleNumber(second));
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second))
    {
        return toString<long>(toLongNumber(first) / toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) / toDoubleNumber(second));
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second))
    {
        return toString<long>(toLongNumber(first) * toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) * toDoubleNumber(second));
}


std::string MathLib::sin(const std::string &tok)
{
    return toString<double>(::sin(toDoubleNumber(tok)));
}


std::string MathLib::cos(const std::string &tok)
{
    return toString<double>(::cos(toDoubleNumber(tok)));
}

std::string MathLib::tan(const std::string &tok)
{
    return toString<double>(::tan(toDoubleNumber(tok)));
}


std::string MathLib::abs(const std::string &tok)
{
    return toString<double>(::abs(toDoubleNumber(tok)));
}

bool MathLib::isGreater(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) > toDoubleNumber(second);
}



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

double MathLib::toNumber(const std::string &str)
{
    return atof(str.c_str());
}

std::string MathLib::toString(double d)
{
    std::ostringstream result;
    result << d;
    return result.str();
}

std::string MathLib::add(const std::string & first, const std::string & second)
{
    return toString(toNumber(first) + toNumber(second));
}

std::string MathLib::subtract(const std::string &first, const std::string &second)
{
    return toString(toNumber(first) - toNumber(second));
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
    return toString(toNumber(first) / toNumber(second));
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
    return toString(toNumber(first) * toNumber(second));
}


std::string MathLib::sin(const std::string &tok)
{
    return toString(::sin(toNumber(tok)));
}


std::string MathLib::cos(const std::string &tok)
{
    return toString(::cos(toNumber(tok)));
}

std::string MathLib::tan(const std::string &tok)
{
    return toString(::tan(toNumber(tok)));
}


std::string MathLib::abs(const std::string &tok)
{
    return toString(::abs(toNumber(tok)));
}

bool MathLib::isGreater(const std::string &first, const std::string &second)
{
    return toNumber(first) > toNumber(second);
}



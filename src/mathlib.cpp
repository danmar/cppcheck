/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
#include <cctype>

long MathLib::toLongNumber(const std::string &str)
{
    if (strncmp(str.c_str(), "0x", 2) == 0)
    {
        return std::strtoul(str.c_str(), '\0', 16);
    }
    if (strncmp(str.c_str(), "0", 1) == 0)
    {
        return std::strtoul(str.c_str(), '\0', 8);
    }
    return (str.find("E", 0) != std::string::npos || str.find("e", 0) != std::string::npos)
           ? static_cast<long>(std::atof(str.c_str()))
           : std::atol(str.c_str());
}

double MathLib::toDoubleNumber(const std::string &str)
{
    if (strncmp(str.c_str(), "0x", 2) == 0)
    {
        return std::strtoul(str.c_str(), '\0', 16);
    }
    return std::atof(str.c_str());
}

template <typename T>
std::string MathLib::toString(T d)
{
    std::ostringstream result;
    result << d;
    return result.str();
}

bool MathLib::isInt(const std::string & s)
{
    // perform prechecks:
    // ------------------
    // first check, if a point is found, it is an floating point value
    if (s.find(".", 0) != std::string::npos) return false;
    // check for scientific notation e.g. NumberE-Number this is obvious an floating point value
    else if (s.find("E-", 0) != std::string::npos || s.find("e-", 0) != std::string::npos) return false;


    // prechecking has nothing found,...
    // gather information
    enum Representation
    {
        eScientific = 0  // NumberE+Number or NumberENumber
        , eOctal        // starts with 0
        , eHex          // starts with 0x
        , eDefault      // Numbers with a (possible) trailing u or U or l or L for unsigned or long datatypes
    };
    // create an instance
    Representation Mode = eDefault;


    // remember position
    unsigned long n = 0;
    // eat up whitespace
    while (std::isspace(s[n])) ++n;

    // determine type
    if (s.find("E", 0) != std::string::npos)
    {
        Mode = eScientific;
    }
    else if (s.find("0x", n, 2) != std::string::npos)
    {
        Mode = eHex;
    }
    else if (s.find("0", n, 1) != std::string::npos && std::isdigit(s[n+1]))
    {
        Mode = eOctal;
    }

    // check sign
    if (s[n] == '-' || s[n] == '+') ++n;

    // check scientific notation
    if (Mode == eScientific)
    {
        // check digits
        while (std::isdigit(s[n])) ++n;

        // check scientific notation
        if (std::tolower(s[n]) == 'e')
        {
            ++n;
            // check positive exponent
            if (s[n] == '+') ++n;
            // floating pointer number e.g. 124E-2
            if (s[n] == '-') return false;
            // check digits of the exponent
            while (std::isdigit(s[n])) ++n;
        }
    }
    // check hex notation
    else if (Mode == eHex)
    {
        ++n; // 0
        ++n; // x
        while (std::isxdigit(s[n]))
            ++n;
    }
    // check octal notation
    else if (Mode == eOctal)
    {
        while (s[n] == '0' || s[n] == '1' || s[n] == '2' || s[n] == '3' || s[n] == '4' || s[n] == '5' || s[n] == '6' || s[n] == '7')
            ++n;
    }
    else if (Mode == eDefault)
    {
        while (std::isdigit(s[n])) ++n;
        // unsigned or long
        while (std::tolower(s[n]) == 'u' || std::tolower(s[n]) == 'l') ++n;
    }
    // eat up whitespace
    while (std::isspace(s[n]))
        ++n;

    // if everything goes good, we are at the end of the string and no digits/character
    // is here --> return true, but if something was found eg. 12E+12AA return false
    if (s[n])
        return false;
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

std::string MathLib::calculate(const std::string &first, const std::string &second, char action)
{
    std::string result("0");

    switch (action)
    {
    case '+':
        result = MathLib::add(first, second);
        break;

    case '-':
        result = MathLib::subtract(first, second);
        break;

    case '*':
        result = MathLib::multiply(first, second);
        break;

    case '/':
        result = MathLib::divide(first, second);
        break;

    default:
        std::cerr << "##### If you see this, there is a bug: "
                  << "MathLib::calculate() was called with unknown action '"
                  << action
                  << "' #####"
                  << std::endl;
        break;
    }

    return result;
}

std::string MathLib::sin(const std::string &tok)
{
    return toString<double>(std::sin(toDoubleNumber(tok)));
}


std::string MathLib::cos(const std::string &tok)
{
    return toString<double>(std::cos(toDoubleNumber(tok)));
}

std::string MathLib::tan(const std::string &tok)
{
    return toString<double>(std::tan(toDoubleNumber(tok)));
}


std::string MathLib::abs(const std::string &tok)
{
    return toString<double>(std::abs(toDoubleNumber(tok)));
}

bool MathLib::isGreater(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) > toDoubleNumber(second);
}



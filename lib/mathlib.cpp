/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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



#include "mathlib.h"
#include "tokenize.h"

#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <cctype>

MathLib::bigint MathLib::toLongNumber(const std::string &str)
{
    // hexadecimal numbers:
    if (str.compare(0, 2, "0x") == 0
        || str.compare(0, 3, "+0x") == 0
        || str.compare(0, 3, "-0x") == 0) {
        bigint ret = 0;
        std::istringstream istr(str.substr((str[0]=='0') ? 2U : 3U));
        istr >> std::hex >> ret;
        return (str[0]=='-') ? -ret : ret;
    }

    // octal numbers:
    if (str.compare(0, 1, "0") == 0
        ||  str.compare(0, 2, "+0") == 0
        ||  str.compare(0, 2, "-0") == 0) {
        bigint ret = 0;
        std::istringstream istr(str.substr((str[0]=='0') ? 1U : 2U));
        istr >> std::oct >> ret;
        return (str[0]=='-') ? -ret : ret;
    }

    if (str.find_first_of("eE") != std::string::npos)
        return static_cast<bigint>(std::atof(str.c_str()));

    bigint ret = 0;
    std::istringstream istr(str);
    istr >> ret;
    return ret;
}

double MathLib::toDoubleNumber(const std::string &str)
{
    if (str.compare(0, 2, "0x") == 0) {
        return std::strtoul(str.c_str(), '\0', 16);
    }
    // nullcheck
    else if (isNullValue(str))
        return 0.0;
    // otherwise, convert to double
    std::istringstream istr(str.c_str());
    double ret;
    istr >> ret;
    return ret;
}

bool MathLib::isFloat(const std::string &s)
{
    // every number that contains a . is a float
    if (s.find("." , 0) != std::string::npos)
        return true;
    // scientific notation
    else if (s.find("E-", 0) != std::string::npos
             || s.find("e-", 0) != std::string::npos)
        return true;

    return false;
}

bool MathLib::isNegative(const std::string &s)
{
    // remember position
    unsigned long n = 0;
    // eat up whitespace
    while (std::isspace(s[n])) ++n;
    // every negative number has a negative sign
    if (s[n] == '-')
        return true;

    return false;
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
    enum Representation {
        eScientific = 0,  // NumberE+Number or NumberENumber
        eOctal,        // starts with 0
        eHex,          // starts with 0x
        eDefault      // Numbers with a (possible) trailing u or U or l or L for unsigned or long datatypes
    };
    // create an instance
    Representation Mode = eDefault;


    // remember position
    unsigned long n = 0;
    // eat up whitespace
    while (std::isspace(s[n])) ++n;

    // determine type
    if (s.find("E", 0) != std::string::npos) {
        Mode = eScientific;
    } else if (s.find("0x", n, 2) != std::string::npos) {
        Mode = eHex;
    } else if (s.length() > 1 && s[0] == '0' && std::isdigit(s[1])) {
        Mode = eOctal;
    }

    // check sign
    if (s[n] == '-' || s[n] == '+') ++n;

    // check scientific notation
    if (Mode == eScientific) {
        // check digits
        while (std::isdigit(s[n])) ++n;

        // check scientific notation
        if (std::tolower(s[n]) == 'e') {
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
    else if (Mode == eHex) {
        ++n; // 0
        ++n; // x
        while (std::isxdigit(s[n]))
            ++n;
    }
    // check octal notation
    else if (Mode == eOctal) {
        while (isOctalDigit(s[n]))
            ++n;
    } else if (Mode == eDefault) {
        // starts with digit
        bool bStartsWithDigit=false;
        while (std::isdigit(s[n])) {
            bStartsWithDigit=true;
            ++n;
        };
        // unsigned or long
        while (std::tolower(s[n]) == 'u' || std::tolower(s[n]) == 'l') ++n;

        if (bStartsWithDigit==false)
            return false;
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
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString<bigint>(toLongNumber(first) + toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) + toDoubleNumber(second));
}

std::string MathLib::subtract(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString<bigint>(toLongNumber(first) - toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) - toDoubleNumber(second));
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString<bigint>(toLongNumber(first) / toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) / toDoubleNumber(second));
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString<bigint>(toLongNumber(first) * toLongNumber(second));
    }
    return toString<double>(toDoubleNumber(first) * toDoubleNumber(second));
}

std::string MathLib::calculate(const std::string &first, const std::string &second, char action, const Tokenizer *tokenizer)
{
    std::string result("0");

    switch (action) {
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
        tokenizer->cppcheckError(0);
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

bool MathLib::isEqual(const std::string &first, const std::string &second)
{
    // this conversion is needed for formating
    // e.g. if first=0.1 and second=1.0E-1, the direct comparison of the strings whould fail
    return toString<double>(toDoubleNumber(first)) == toString<double>(toDoubleNumber(second));
}

bool MathLib::isNotEqual(const std::string &first, const std::string &second)
{
    return !isEqual(first, second);
}

bool MathLib::isGreater(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) > toDoubleNumber(second);
}

bool MathLib::isGreaterEqual(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) >= toDoubleNumber(second);
}

bool MathLib::isLess(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) < toDoubleNumber(second);
}

bool MathLib::isLessEqual(const std::string &first, const std::string &second)
{
    return toDoubleNumber(first) <= toDoubleNumber(second);
}

bool MathLib::isNullValue(const std::string &str)
{
    return (str == "-0"      || str == "-0.0"
            ||  str == "0"
            ||  str == "-0."     || str == "-0E-00"
            ||  str == "-0E+00"  || str == "+0E+00"
            ||  str == "+0E-00"  || str == "+0"
            ||  str == "+0.0"    || str == "+0."
            ||  str == "0.0"     || str == "-0e-00"
            ||  str == "+0e+00"  || str == "-0e+00"
            ||  str == "+0e-00"  || str == "-0e-00"
            ||  str == "-0E-0"   || str == "+0E-00");
}

bool MathLib::isOctalDigit(char c)
{
    if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7')
        return true;

    return false;
}


/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
#include "errorlogger.h"

#include <string>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <limits>

MathLib::bigint MathLib::toLongNumber(const std::string &str)
{
    // hexadecimal numbers:
    if (isHex(str)) {
        if (str[0] == '-') {
            bigint ret = 0;
            std::istringstream istr(str);
            istr >> std::hex >> ret;
            return ret;
        } else {
            unsigned long long ret = 0;
            std::istringstream istr(str);
            istr >> std::hex >> ret;
            return (bigint)ret;
        }
    }

    // octal numbers:
    if (isOct(str)) {
        bigint ret = 0;
        std::istringstream istr(str);
        istr >> std::oct >> ret;
        return ret;
    }

    // binary numbers:
    if (isBin(str)) {
        bigint ret = 0;
        for (std::string::size_type i = str[0] == '0'?2:3; i < str.length(); i++) {
            ret <<= 1;
            if (str[i] == '1')
                ret |= 1;
        }
        if (str[0] == '-')
            ret = -ret;
        return ret;
    }

    if (isFloat(str))
        return static_cast<bigint>(std::atof(str.c_str()));

    bigint ret = 0;
    std::istringstream istr(str);
    istr >> ret;
    return ret;
}

double MathLib::toDoubleNumber(const std::string &str)
{
    if (isHex(str))
        return static_cast<double>(toLongNumber(str));
    // nullcheck
    else if (isNullValue(str))
        return 0.0;
    // otherwise, convert to double
    std::istringstream istr(str);
    double ret;
    istr >> ret;
    return ret;
}

template<> std::string MathLib::toString(double value)
{
    std::ostringstream result;
    result.precision(12);
    result << value;
    if (result.str() == "-0")
        return "0.0";
    if (result.str().find(".") == std::string::npos)
        return result.str() + ".0";
    return result.str();
}

bool MathLib::isFloat(const std::string &s)
{
    if (s.empty())
        return false;
    enum {START, BASE_PLUSMINUS, BASE_DIGITS1, LEADING_DECIMAL, TRAILING_DECIMAL, BASE_DIGITS2, E, MANTISSA_PLUSMINUS, MANTISSA_DIGITS, F} state = START;
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        switch (state) {
        case START:
            if (*it=='+' || *it=='-')
                state=BASE_PLUSMINUS;
            else if (*it=='.')
                state=LEADING_DECIMAL;
            else if (std::isdigit(*it))
                state=BASE_DIGITS1;
            else
                return false;
            break;
        case BASE_PLUSMINUS:
            if (*it=='.')
                state=LEADING_DECIMAL;
            else if (std::isdigit(*it))
                state=BASE_DIGITS1;
            else if (*it=='e' || *it=='E')
                state=E;
            else
                return false;
            break;
        case LEADING_DECIMAL:
            if (std::isdigit(*it))
                state=BASE_DIGITS2;
            else if (*it=='e' || *it=='E')
                state=E;
            else
                return false;
            break;
        case BASE_DIGITS1:
            if (*it=='e' || *it=='E')
                state=E;
            else if (*it=='.')
                state=TRAILING_DECIMAL;
            else if (!std::isdigit(*it))
                return false;
            break;
        case TRAILING_DECIMAL:
            if (*it=='e' || *it=='E')
                state=E;
            else if (std::isdigit(*it))
                state=BASE_DIGITS2;
            else
                return false;
            break;
        case BASE_DIGITS2:
            if (*it=='e' || *it=='E')
                state=E;
            else if (*it=='f' || *it=='F')
                state=F;
            else if (!std::isdigit(*it))
                return false;
            break;
        case E:
            if (*it=='+' || *it=='-')
                state=MANTISSA_PLUSMINUS;
            else if (std::isdigit(*it))
                state=MANTISSA_DIGITS;
            else
                return false;
            break;
        case MANTISSA_PLUSMINUS:
            if (!std::isdigit(*it))
                return false;
            else
                state=MANTISSA_DIGITS;
            break;
        case MANTISSA_DIGITS:
            if (*it=='f' || *it=='F')
                state=F;
            else if (!std::isdigit(*it))
                return false;
            break;
        case F:
            return false;
        }
    }
    return (state==BASE_DIGITS2 || state == MANTISSA_DIGITS || state == TRAILING_DECIMAL || state == F);
}

bool MathLib::isNegative(const std::string &s)
{
    // remember position
    std::string::size_type n = 0;
    // eat up whitespace
    while (std::isspace(s[n])) ++n;
    // every negative number has a negative sign
    return (s[n] == '-');
}

bool MathLib::isPositive(const std::string &s)
{
    return !MathLib::isNegative(s);
}

bool MathLib::isOct(const std::string& str)
{
    const bool sign = str[0]=='-' || str[0]=='+';
    return (str[sign?1:0] == '0' && (str.size() == 1 || isOctalDigit(str[sign?2:1])) && !isFloat(str));
}

bool MathLib::isHex(const std::string& str)
{
    const bool sign = str[0]=='-' || str[0]=='+';
    return (str.compare(sign?1:0, 2, "0x") == 0 || str.compare(sign?1:0, 2, "0X") == 0);
}

bool MathLib::isBin(const std::string& str)
{
    const bool sign = str[0]=='-' || str[0]=='+';
    return ((str.compare(sign?1:0, 2, "0b") == 0 || str.compare(sign?1:0, 2, "0B") == 0) && str.find_first_not_of("10bB", 1) == std::string::npos);
}

bool MathLib::isInt(const std::string & s)
{
    // perform prechecks:
    // ------------------
    // first check, if a point is found, it is an floating point value
    const std::string charsToIndicateAFloat=".eE";
    if (s.find_last_of(charsToIndicateAFloat) != std::string::npos)
        return false;

    // prechecking has nothing found,...
    // gather information
    enum Representation {
        eOctal,          // starts with 0
        eHex,            // starts with 0x
        eDefault         // Numbers with a (possible) trailing u or U or l or L for unsigned or long datatypes
    };
    // create an instance
    Representation Mode = eDefault;


    // remember position
    unsigned long n = 0;
    // eat up whitespace
    while (std::isspace(s[n])) ++n;

    // determine type
    if (isHex(s)) {
        Mode = eHex;
    } else if (isOct(s)) {
        Mode = eOctal;
    }

    // check sign
    if (s[n] == '-' || s[n] == '+') ++n;

    if (Mode == eHex) {
        ++n; // 0
        ++n; // x
        while (std::isxdigit(s[n]))
            ++n;

        while (std::tolower(s[n]) == 'u' || std::tolower(s[n]) == 'l') ++n; // unsigned or long (long)
    }
    // check octal notation
    else if (Mode == eOctal) {
        ++n; // 0
        while (isOctalDigit(s[n]))
            ++n;

        while (std::tolower(s[n]) == 'u' || std::tolower(s[n]) == 'l') ++n; // unsigned or long (long)
    } else if (Mode == eDefault) {
        // starts with digit
        bool bStartsWithDigit=false;
        while (std::isdigit(s[n])) {
            bStartsWithDigit=true;
            ++n;
        };

        while (std::tolower(s[n]) == 'u' || std::tolower(s[n]) == 'l') ++n; // unsigned or long (long)

        if (!bStartsWithDigit)
            return false;
    }
    // eat up whitespace
    while (std::isspace(s[n]))
        ++n;

    // if everything goes good, we are at the end of the string and no digits/character
    // is here --> return true, but if something was found e.g. 12E+12AA return false
    return (n >= s.length());
}

std::string MathLib::add(const std::string & first, const std::string & second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString(toLongNumber(first) + toLongNumber(second));
    }

    double d1 = toDoubleNumber(first);
    double d2 = toDoubleNumber(second);

    int count = 0;
    while (d1 > 100000.0 * d2 && toString(d1+d2)==first && ++count<5)
        d2 *= 10.0;
    while (d2 > 100000.0 * d1 && toString(d1+d2)==second && ++count<5)
        d1 *= 10.0;

    return toString(d1 + d2);
}

std::string MathLib::subtract(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString(toLongNumber(first) - toLongNumber(second));
    }

    if (first == second)
        return "0.0" ;

    double d1 = toDoubleNumber(first);
    double d2 = toDoubleNumber(second);

    int count = 0;
    while (d1 > 100000.0 * d2 && toString(d1-d2)==first && ++count<5)
        d2 *= 10.0;
    while (d2 > 100000.0 * d1 && toString(d1-d2)==second && ++count<5)
        d1 *= 10.0;

    return toString(d1 - d2);
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        const bigint a = toLongNumber(first);
        const bigint b = toLongNumber(second);
        if (a == std::numeric_limits<bigint>::min())
            throw InternalError(0, "Internal Error: Division overflow");
        if (b == 0)
            throw InternalError(0, "Internal Error: Division by zero");
        return toString(toLongNumber(first) / b);
    } else if (second == "0.0") {
        if (first=="0.0" || first=="+0.0")
            return "nan.0";
        if (first=="-0.0")
            return "-nan.0";
        return (first[0] == '-') ? "-inf.0" : "inf.0";
    }
    return toString(toDoubleNumber(first) / toDoubleNumber(second));
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString(toLongNumber(first) * toLongNumber(second));
    }
    return toString(toDoubleNumber(first) * toDoubleNumber(second));
}

std::string MathLib::mod(const std::string &first, const std::string &second)
{
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        bigint b = toLongNumber(second);
        if (b == 0)
            throw InternalError(0, "Internal Error: Division by zero");
        return toString(toLongNumber(first) % b);
    }
    return toString(std::fmod(toDoubleNumber(first),toDoubleNumber(second)));
}

std::string MathLib::calculate(const std::string &first, const std::string &second, char action)
{
    switch (action) {
    case '+':
        return MathLib::add(first, second);

    case '-':
        return MathLib::subtract(first, second);

    case '*':
        return MathLib::multiply(first, second);

    case '/':
        return MathLib::divide(first, second);

    case '%':
        return MathLib::mod(first, second);

    case '&':
        return MathLib::toString(MathLib::toLongNumber(first) & MathLib::toLongNumber(second));

    case '|':
        return MathLib::toString(MathLib::toLongNumber(first) | MathLib::toLongNumber(second));

    case '^':
        return MathLib::toString(MathLib::toLongNumber(first) ^ MathLib::toLongNumber(second));

    default:
        throw InternalError(0, std::string("Unexpected action '") + action + "' in MathLib::calculate(). Please report this to Cppcheck developers.");
    }
}

std::string MathLib::sin(const std::string &tok)
{
    return toString(std::sin(toDoubleNumber(tok)));
}


std::string MathLib::cos(const std::string &tok)
{
    return toString(std::cos(toDoubleNumber(tok)));
}

std::string MathLib::tan(const std::string &tok)
{
    return toString(std::tan(toDoubleNumber(tok)));
}


std::string MathLib::abs(const std::string &tok)
{
    return toString(std::abs(toDoubleNumber(tok)));
}

bool MathLib::isEqual(const std::string &first, const std::string &second)
{
    // this conversion is needed for formatting
    // e.g. if first=0.1 and second=1.0E-1, the direct comparison of the strings would fail
    return toString(toDoubleNumber(first)) == toString(toDoubleNumber(second));
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
    return (str == "-0"        || str == "0"      || str == "+0"
            || str == "-0.0"   || str == "0.0"    || str == "+0.0"
            || str == "-0."    || str == "+0."
            || str == "-0E-00" || str == "-0E+00" || str == "+0E+00" || str == "+0E-00"
            || str == "-0e-00" || str == "-0e+00" || str == "+0e+00" || str == "+0e-00"
            || str == "-0E-0");
}

bool MathLib::isOctalDigit(char c)
{
    return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7');
}

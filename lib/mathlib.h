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


#ifndef mathlibH
#define mathlibH

#include <string>
#include <sstream>

/// @addtogroup Core
/// @{

/** @brief simple math functions that uses operands stored in std::string. useful when performing math on tokens. */

class Tokenizer;

class MathLib {
public:
    typedef long long bigint;

    static bigint toLongNumber(const std::string & str);
    static double toDoubleNumber(const std::string & str);

    template<typename T>
    static std::string toString(const T &d) {
        std::ostringstream result;
        result << d;
        std::string strResult(result.str());
        if (strResult == "-0"
            || strResult == "+0"
            || strResult == "-0."
            || strResult == "+0.")
            return std::string("0");
        return result.str();
    }

    static bool isInt(const std::string & str);
    static bool isFloat(const std::string &str);
    static bool isNegative(const std::string &str);

    static std::string add(const std::string & first, const std::string & second);
    static std::string subtract(const std::string & first, const std::string & second);
    static std::string multiply(const std::string & first, const std::string & second);
    static std::string divide(const std::string & first, const std::string & second);
    static std::string calculate(const std::string & first, const std::string & second, char action, const Tokenizer *tokenizer);

    static std::string sin(const std::string & tok);
    static std::string cos(const std::string & tok);
    static std::string tan(const std::string & tok);
    static std::string abs(const std::string & tok);
    static bool isEqual(const std::string & first, const std::string & second);
    static bool isNotEqual(const std::string & first, const std::string & second);
    static bool isGreater(const std::string & first, const std::string & second);
    static bool isGreaterEqual(const std::string & first, const std::string & second);
    static bool isLess(const std::string & first, const std::string & second);
    static bool isLessEqual(const std::string & first, const std::string & second);
    static bool isNullValue(const std::string &tok);
    /**
     * Return true if given character is 0,1,2,3,4,5,6 or 7.
     * @param c The character to check
     * @return true if given character is octal digit.
     */
    static bool isOctalDigit(char c);
};

/// @}

#endif

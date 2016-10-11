/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#ifndef mathlibH
#define mathlibH
//---------------------------------------------------------------------------

#include <string>
#include <sstream>
#include "config.h"

/// @addtogroup Core
/// @{

/** @brief simple math functions that uses operands stored in std::string. useful when performing math on tokens. */

class CPPCHECKLIB MathLib {
    friend class TestMathLib;

public:
    /** @brief value class */
    class value {
    private:
        long long intValue;
        double doubleValue;
        enum { INT, LONG, LONGLONG, FLOAT } type;
        bool isUnsigned;

        void promote(const value &v);

    public:
        explicit value(const std::string &s);
        std::string str() const;
        bool isInt() const {
            return type != FLOAT;
        }
        bool isFloat() const {
            return type == FLOAT;
        }

        double getDoubleValue() const {
            return isFloat() ? doubleValue : (double)intValue;
        }

        static value calc(char op, const value &v1, const value &v2);
        int compare(const value &v) const;
        value add(int v) const;
        value shiftLeft(const value &v) const;
        value shiftRight(const value &v) const;
    };

    typedef long long bigint;
    typedef unsigned long long biguint;

    static bigint toLongNumber(const std::string & str);
    static biguint toULongNumber(const std::string & str);

    template<class T> static std::string toString(T value) {
        std::ostringstream result;
        result << value;
        return result.str();
    }
    static double toDoubleNumber(const std::string & str);

    static bool isInt(const std::string & str);
    static bool isFloat(const std::string &str);
    static bool isDecimalFloat(const std::string &str);
    static bool isNegative(const std::string &str);
    static bool isPositive(const std::string &str);
    static bool isDec(const std::string & str);
    static bool isFloatHex(const std::string& str);
    static bool isIntHex(const std::string& str);
    static bool isOct(const std::string& str);
    static bool isBin(const std::string& str);

    static bool isValidIntegerSuffix(std::string::const_iterator it, std::string::const_iterator end);

    static std::string add(const std::string & first, const std::string & second);
    static std::string subtract(const std::string & first, const std::string & second);
    static std::string multiply(const std::string & first, const std::string & second);
    static std::string divide(const std::string & first, const std::string & second);
    static std::string mod(const std::string & first, const std::string & second);
    static std::string incdec(const std::string & var, const std::string & op);
    static std::string calculate(const std::string & first, const std::string & second, char action);

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
     * @param[in] c The character to check
     * @return true if given character is octal digit.
     */
    static bool isOctalDigit(char c);

    /**
     * \param[in] str character literal
     * @return Number of internal representation of the character literal
     * */
    static MathLib::bigint characterLiteralToLongNumber(const std::string& str);

    /**
     * \param[in] iCode Code being considered
     * \param[in] iPos A posision within iCode
     * \return Whether iCode[iPos] is a C++14 digit separator
     */
    static bool isDigitSeparator(const std::string& iCode, std::string::size_type iPos);

private:
    /*
     * \param iLiteral A character literal
     * \return The equivalent character literal with all escapes interpreted
     */
    static std::string normalizeCharacterLiteral(const std::string& iLiteral);
};

MathLib::value operator+(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator-(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator*(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator/(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator%(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator&(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator|(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator^(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator<<(const MathLib::value &v1, const MathLib::value &v2);
MathLib::value operator>>(const MathLib::value &v1, const MathLib::value &v2);

template<> CPPCHECKLIB std::string MathLib::toString(double value); // Declare specialization to avoid linker problems

/// @}
//---------------------------------------------------------------------------
#endif // mathlibH

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



#include "mathlib.h"
#include "errorlogger.h"

#include <algorithm> // VS2013: std::min and std::max
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <limits>


#if defined(_MSC_VER) && _MSC_VER <= 1700  // VS2012 doesn't have std::isinf and std::isnan
#define ISINF(x)      (!_finite(x))
#define ISNAN(x)      (_isnan(x))
#elif defined(__INTEL_COMPILER)
#define ISINF(x)      (isinf(x))
#define ISNAN(x)      (isnan(x))
#else  // Use C++11 functions
#define ISINF(x)      (std::isinf(x))
#define ISNAN(x)      (std::isnan(x))
#endif

MathLib::value::value(const std::string &s) :
    intValue(0), doubleValue(0), isUnsigned(false)
{
    if (MathLib::isFloat(s)) {
        type = MathLib::value::FLOAT;
        doubleValue = MathLib::toDoubleNumber(s);
        return;
    }

    if (!MathLib::isInt(s))
        throw InternalError(0, "Invalid value: " + s);

    type = MathLib::value::INT;
    intValue = MathLib::toLongNumber(s);

    if (isIntHex(s) && intValue < 0)
        isUnsigned = true;

    // read suffix
    if (s.size() >= 2U) {
        for (std::size_t i = s.size() - 1U; i > 0U; --i) {
            char c = s[i];
            if (c == 'u' || c == 'U')
                isUnsigned = true;
            else if (c == 'l' || c == 'L') {
                if (type == MathLib::value::INT)
                    type = MathLib::value::LONG;
                else if (type == MathLib::value::LONG)
                    type = MathLib::value::LONGLONG;
            } else if (i > 2U && c == '4' && s[i-1] == '6' && s[i-2] == 'i')
                type = MathLib::value::LONGLONG;
        }
    }
}

std::string MathLib::value::str() const
{
    std::ostringstream ostr;
    if (type == MathLib::value::FLOAT) {
        if (ISNAN(doubleValue))
            return "nan.0";
        if (ISINF(doubleValue))
            return (doubleValue > 0) ? "inf.0" : "-inf.0";

        ostr.precision(9);
        ostr << std::fixed << doubleValue;

        // remove trailing zeros
        std::string ret(ostr.str());
        std::string::size_type pos = ret.size() - 1U;
        while (ret[pos] == '0')
            pos--;
        if (ret[pos] == '.')
            ++pos;

        return ret.substr(0, pos+1);
    }

    if (isUnsigned)
        ostr << static_cast<biguint>(intValue) << "U";
    else
        ostr << intValue;
    if (type == MathLib::value::LONG)
        ostr << "L";
    else if (type == MathLib::value::LONGLONG)
        ostr << "LL";
    return ostr.str();
}

void MathLib::value::promote(const MathLib::value &v)
{
    if (isInt() && v.isInt()) {
        if (type < v.type) {
            type = v.type;
            isUnsigned = v.isUnsigned;
        } else if (type == v.type) {
            isUnsigned |= v.isUnsigned;
        }
    } else if (!isFloat()) {
        isUnsigned = false;
        doubleValue = intValue;
        type = MathLib::value::FLOAT;
    }
}


MathLib::value MathLib::value::calc(char op, const MathLib::value &v1, const MathLib::value &v2)
{
    value temp(v1);
    temp.promote(v2);
    if (temp.isFloat()) {
        switch (op) {
        case '+':
            temp.doubleValue += v2.getDoubleValue();
            break;
        case '-':
            temp.doubleValue -= v2.getDoubleValue();
            break;
        case '*':
            temp.doubleValue *= v2.getDoubleValue();
            break;
        case '/':
            temp.doubleValue /= v2.getDoubleValue();
            break;
        case '%':
        case '&':
        case '|':
        case '^':
            throw InternalError(0, "Invalid calculation");
        default:
            throw InternalError(0, "Unhandled calculation");
        }
    } else if (temp.isUnsigned) {
        switch (op) {
        case '+':
            temp.intValue += (unsigned long long)v2.intValue;
            break;
        case '-':
            temp.intValue -= (unsigned long long)v2.intValue;
            break;
        case '*':
            temp.intValue *= (unsigned long long)v2.intValue;
            break;
        case '/':
            if (v2.intValue == 0)
                throw InternalError(0, "Internal Error: Division by zero");
            if (v1.intValue == std::numeric_limits<bigint>::min() && std::abs(v2.intValue)<=1)
                throw InternalError(0, "Internal Error: Division overflow");
            temp.intValue /= (unsigned long long)v2.intValue;
            break;
        case '%':
            if (v2.intValue == 0)
                throw InternalError(0, "Internal Error: Division by zero");
            temp.intValue %= (unsigned long long)v2.intValue;
            break;
        case '&':
            temp.intValue &= (unsigned long long)v2.intValue;
            break;
        case '|':
            temp.intValue |= (unsigned long long)v2.intValue;
            break;
        case '^':
            temp.intValue ^= (unsigned long long)v2.intValue;
            break;
        default:
            throw InternalError(0, "Unhandled calculation");
        }
    } else {
        switch (op) {
        case '+':
            temp.intValue += v2.intValue;
            break;
        case '-':
            temp.intValue -= v2.intValue;
            break;
        case '*':
            temp.intValue *= v2.intValue;
            break;
        case '/':
            if (v2.intValue == 0)
                throw InternalError(0, "Internal Error: Division by zero");
            if (v1.intValue == std::numeric_limits<bigint>::min() && std::abs(v2.intValue)<=1)
                throw InternalError(0, "Internal Error: Division overflow");
            temp.intValue /= v2.intValue;
            break;
        case '%':
            if (v2.intValue == 0)
                throw InternalError(0, "Internal Error: Division by zero");
            temp.intValue %= v2.intValue;
            break;
        case '&':
            temp.intValue &= v2.intValue;
            break;
        case '|':
            temp.intValue |= v2.intValue;
            break;
        case '^':
            temp.intValue ^= v2.intValue;
            break;
        default:
            throw InternalError(0, "Unhandled calculation");
        }
    }
    return temp;
}


int MathLib::value::compare(const MathLib::value &v) const
{
    value temp(*this);
    temp.promote(v);

    if (temp.isFloat()) {
        if (temp.doubleValue < v.getDoubleValue())
            return -1;
        if (temp.doubleValue > v.getDoubleValue())
            return 1;
        return 0;
    }

    if (temp.isUnsigned) {
        if ((unsigned long long)intValue < (unsigned long long)v.intValue)
            return -1;
        if ((unsigned long long)intValue > (unsigned long long)v.intValue)
            return 1;
        return 0;
    }

    if (intValue < v.intValue)
        return -1;
    if (intValue > v.intValue)
        return 1;
    return 0;
}

MathLib::value MathLib::value::add(int v) const
{
    MathLib::value temp(*this);
    if (temp.isInt())
        temp.intValue += v;
    else
        temp.doubleValue += v;
    return temp;
}

MathLib::value MathLib::value::shiftLeft(const MathLib::value &v) const
{
    if (!isInt() || !v.isInt())
        throw InternalError(0, "Shift operand is not integer");
    MathLib::value ret(*this);
    ret.intValue <<= v.intValue;
    return ret;
}

MathLib::value MathLib::value::shiftRight(const MathLib::value &v) const
{
    if (!isInt() || !v.isInt())
        throw InternalError(0, "Shift operand is not integer");
    MathLib::value ret(*this);
    ret.intValue >>= v.intValue;
    return ret;
}


MathLib::biguint MathLib::toULongNumber(const std::string & str)
{
    // hexadecimal numbers:
    if (isIntHex(str)) {
        if (str[0] == '-') {
            biguint ret = 0;
            std::istringstream istr(str);
            istr >> std::hex >> ret;
            return ret;
        } else {
            unsigned long long ret = 0;
            std::istringstream istr(str);
            istr >> std::hex >> ret;
            return (biguint)ret;
        }
    }

    // octal numbers:
    if (isOct(str)) {
        biguint ret = 0;
        std::istringstream istr(str);
        istr >> std::oct >> ret;
        return ret;
    }

    // binary numbers:
    if (isBin(str)) {
        biguint ret = 0;
        for (std::string::size_type i = str[0] == '0'?2:3; i < str.length(); i++) {
            ret <<= 1;
            if (str[i] == '1')
                ret |= 1;
        }
        /* if (str[0] == '-')
                ret = -ret; */
        return ret;
    }

    if (isFloat(str)) {
        // Things are going to be less precise now: the value can't b represented in the biguint type.
        // Use min/max values as an approximation. See #5843
        const double doubleval = std::atof(str.c_str());
        if (doubleval > (double)std::numeric_limits<biguint>::max())
            return std::numeric_limits<biguint>::max();
        else
            return static_cast<biguint>(doubleval);
    }

    biguint ret = 0;
    std::istringstream istr(str);
    istr >> ret;
    return ret;
}

static unsigned int encodeMultiChar(const std::string& str)
{
    unsigned int retval(str.front());
    for (std::string::const_iterator it=str.begin()+1; it!=str.end(); ++it) {
        retval = retval<<8 | *it;
    }
    return retval;
}

static bool isoctal(int c)
{
    return c>='0' && c<='7';
}

MathLib::bigint MathLib::characterLiteralToLongNumber(const std::string& str)
{
    if (str.empty())
        return 0; // <- only possible in unit testing

    // '\xF6'
    if (str.size() == 4 && str.compare(0,2,"\\x")==0 && std::isxdigit(str[2]) && std::isxdigit(str[3])) {
        return std::strtoul(str.substr(2).c_str(), NULL, 16);
    }

    // '\123'
    if (str.size() == 4 && str[0] == '\\' && isoctal(str[1]) && isoctal(str[2]) && isoctal(str[3])) {
        return (char)std::strtoul(str.substr(1).c_str(), NULL, 8);
    }

    // C99 6.4.4.4
    // The value of an integer character constant containing more than one character (e.g., 'ab'),
    // or containing a character or escape sequence that does not map to a single-byte execution character,
    // is implementation-defined.
    // clang and gcc seem to use the following encoding: 'AB' as (('A' << 8) | 'B')
    const std::string& normStr = normalizeCharacterLiteral(str);
    return encodeMultiChar(normStr);
}

std::string MathLib::normalizeCharacterLiteral(const std::string& iLiteral)
{
    std::string normalizedLiteral;
    const std::string::size_type iLiteralLen = iLiteral.size();
    for (std::string::size_type idx = 0; idx < iLiteralLen ; ++idx) {
        if (iLiteral[idx] != '\\') {
            normalizedLiteral.push_back(iLiteral[idx]);
            continue;
        }
        ++idx;
        if (idx == iLiteralLen) {
            throw InternalError(0, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
        }
        switch (iLiteral[idx]) {
        case 'x':
            // Hexa-decimal number: skip \x and interpret the next two characters
        {
            if (++idx == iLiteralLen)
                throw InternalError(0, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
            std::string tempBuf;
            tempBuf.push_back(iLiteral[idx]);
            if (++idx != iLiteralLen)
                tempBuf.push_back(iLiteral[idx]);
            normalizedLiteral.push_back(static_cast<char>(MathLib::toULongNumber("0x" + tempBuf)));
            continue;
        }
        case 'u':
        case 'U':
            // Unicode string; just skip the \u or \U
            if (idx + 1 == iLiteralLen)
                throw InternalError(0, "Internal Error. MathLib::characterLiteralToLongNumber: Unhandled char constant '" + iLiteral + "'.");
            continue;
        }
        // Single digit octal number
        if (1 == iLiteralLen - idx) {
            switch (iLiteral[idx]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                normalizedLiteral.push_back(iLiteral[idx]-'0');
                break;
            case 'a':
                normalizedLiteral.push_back('\a');
                break;
            case 'b':
                normalizedLiteral.push_back('\b');
                break;
            case 'e':
                normalizedLiteral.push_back(0x1B); // clang, gcc, tcc interpnormalizedLiteral this as 0x1B - escape character
                break;
            case 'f':
                normalizedLiteral.push_back('\f');
                break;
            case 'n':
                normalizedLiteral.push_back('\n');
                break;
            case 'r':
                normalizedLiteral.push_back('\r');
                break;
            case 't':
                normalizedLiteral.push_back('\t');
                break;
            case 'v':
                normalizedLiteral.push_back('\v');
                break;
            case '\\':
            case '\?':
            case '\'':
            case '\"':
                normalizedLiteral.push_back(iLiteral[idx]);
                break;
            default:
                throw InternalError(0, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
            }
            continue;
        }
        // 2-3 digit octal number
        if (!MathLib::isOctalDigit(iLiteral[idx]))
            throw InternalError(0, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
        std::string tempBuf;
        tempBuf.push_back(iLiteral[idx]);
        ++idx;
        if (MathLib::isOctalDigit(iLiteral[idx])) {
            tempBuf.push_back(iLiteral[idx]);
            ++idx;
            if (MathLib::isOctalDigit(iLiteral[idx])) {
                tempBuf.push_back(iLiteral[idx]);
            }
        }
        normalizedLiteral.push_back(static_cast<char>(MathLib::toLongNumber("0" + tempBuf)));
    }
    return normalizedLiteral;
}

MathLib::bigint MathLib::toLongNumber(const std::string & str)
{
    // hexadecimal numbers:
    if (isIntHex(str)) {
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

    if (isFloat(str)) {
        // Things are going to be less precise now: the value can't be represented in the bigint type.
        // Use min/max values as an approximation. See #5843
        const double doubleval = toDoubleNumber(str);
        if (doubleval > (double)std::numeric_limits<bigint>::max())
            return std::numeric_limits<bigint>::max();
        else if (doubleval < (double)std::numeric_limits<bigint>::min())
            return std::numeric_limits<bigint>::min();
        else
            return static_cast<bigint>(doubleval);
    }

    if (str[0] == '\'' && str.size() >= 3U && str.back() == '\'') {
        return characterLiteralToLongNumber(str.substr(1,str.size()-2));
    }

    bigint ret = 0;
    std::istringstream istr(str);
    istr >> ret;
    return ret;
}


double MathLib::toDoubleNumber(const std::string &str)
{
    if (str[0] == '\'' && str.size() >= 3U && str.back() == '\'')
        return characterLiteralToLongNumber(str.substr(1,str.size()-2));
    if (isIntHex(str))
        return static_cast<double>(toLongNumber(str));
    // nullcheck
    if (isNullValue(str))
        return 0.0;
#ifdef __clang__
    if (isFloat(str)) // Workaround libc++ bug at http://llvm.org/bugs/show_bug.cgi?id=17782
        // TODO : handle locale
        return std::strtod(str.c_str(), 0);
#endif
    // otherwise, convert to double
    std::istringstream istr(str);
    istr.imbue(std::locale::classic());
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
    if (result.str().find('.') == std::string::npos)
        return result.str() + ".0";
    return result.str();
}

bool MathLib::isFloat(const std::string &str)
{
    return isDecimalFloat(str) || isFloatHex(str);
}

bool MathLib::isDecimalFloat(const std::string &str)
{
    if (str.empty())
        return false;
    enum State {
        START, BASE_PLUSMINUS, BASE_DIGITS1, LEADING_DECIMAL, TRAILING_DECIMAL, BASE_DIGITS2, E, MANTISSA_PLUSMINUS, MANTISSA_DIGITS, SUFFIX_F, SUFFIX_L
    } state = START;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        switch (state) {
        case START:
            if (*it=='+' || *it=='-')
                state=BASE_PLUSMINUS;
            else if (*it=='.')
                state=LEADING_DECIMAL;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state=BASE_DIGITS1;
            else
                return false;
            break;
        case BASE_PLUSMINUS:
            if (*it=='.')
                state=LEADING_DECIMAL;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state=BASE_DIGITS1;
            else if (*it=='e' || *it=='E')
                state=E;
            else
                return false;
            break;
        case LEADING_DECIMAL:
            if (std::isdigit(static_cast<unsigned char>(*it)))
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
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case TRAILING_DECIMAL:
            if (*it=='e' || *it=='E')
                state=E;
            else if (*it=='f' || *it=='F')
                state=SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state=SUFFIX_L;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state=BASE_DIGITS2;
            else
                return false;
            break;
        case BASE_DIGITS2:
            if (*it=='e' || *it=='E')
                state=E;
            else if (*it=='f' || *it=='F')
                state=SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state=SUFFIX_L;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case E:
            if (*it=='+' || *it=='-')
                state=MANTISSA_PLUSMINUS;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state=MANTISSA_DIGITS;
            else
                return false;
            break;
        case MANTISSA_PLUSMINUS:
            if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            else
                state=MANTISSA_DIGITS;
            break;
        case MANTISSA_DIGITS:
            if (*it=='f' || *it=='F')
                state=SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state=SUFFIX_L;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case SUFFIX_F:
            return false;
        case SUFFIX_L:
            return false;
        }
    }
    return (state==BASE_DIGITS2 || state==MANTISSA_DIGITS || state==TRAILING_DECIMAL || state==SUFFIX_F || state==SUFFIX_L);
}

bool MathLib::isNegative(const std::string &str)
{
    if (str.empty())
        return false;
    return (str[0] == '-');
}

bool MathLib::isPositive(const std::string &str)
{
    if (str.empty())
        return false;
    return !MathLib::isNegative(str);
}

/*! \brief Does the string represent an octal number?
 * In case leading or trailing white space is provided, the function
 * returns false.
 * Additional information can be found here:
 * http://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html
 *
 * \param str The string to check. In case the string is empty, the function returns false.
 * \return Return true in case a octal number is provided and false otherwise.
 **/
bool MathLib::isOct(const std::string& str)
{
    enum Status {
        START, PLUSMINUS, OCTAL_PREFIX, DIGITS
    } state = START;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        switch (state) {
        case START:
            if (*it == '+' || *it == '-')
                state = PLUSMINUS;
            else if (*it == '0')
                state = OCTAL_PREFIX;
            else
                return false;
            break;
        case PLUSMINUS:
            if (*it == '0')
                state = OCTAL_PREFIX;
            else
                return false;
            break;
        case OCTAL_PREFIX:
            if (isOctalDigit(static_cast<unsigned char>(*it)))
                state = DIGITS;
            else
                return false;
            break;
        case DIGITS:
            if (isOctalDigit(static_cast<unsigned char>(*it)))
                state = DIGITS;
            else
                return isValidIntegerSuffix(it,str.end());
            break;
        }
    }
    return state == DIGITS;
}

bool MathLib::isIntHex(const std::string& str)
{
    enum Status {
        START, PLUSMINUS, HEX_PREFIX, DIGIT, DIGITS
    } state = START;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        switch (state) {
        case START:
            if (*it == '+' || *it == '-')
                state = PLUSMINUS;
            else if (*it == '0')
                state = HEX_PREFIX;
            else
                return false;
            break;
        case PLUSMINUS:
            if (*it == '0')
                state = HEX_PREFIX;
            else
                return false;
            break;
        case HEX_PREFIX:
            if (*it == 'x' || *it == 'X')
                state = DIGIT;
            else
                return false;
            break;
        case DIGIT:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = DIGITS;
            else
                return false;
            break;
        case DIGITS:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = DIGITS;
            else
                return isValidIntegerSuffix(it,str.end());
            break;
        }
    }
    return state == DIGITS;
}

bool MathLib::isFloatHex(const std::string& str)
{
    enum Status {
        START, PLUSMINUS, HEX_PREFIX, WHOLE_NUMBER_DIGIT, WHOLE_NUMBER_DIGITS, FRACTION, EXPONENT_DIGIT, EXPONENT_DIGITS
    } state = START;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        switch (state) {
        case START:
            if (*it == '+' || *it == '-')
                state = PLUSMINUS;
            else if (*it == '0')
                state = HEX_PREFIX;
            else
                return false;
            break;
        case PLUSMINUS:
            if (*it == '0')
                state = HEX_PREFIX;
            else
                return false;
            break;
        case HEX_PREFIX:
            if (*it == 'x' || *it == 'X')
                state = WHOLE_NUMBER_DIGIT;
            else
                return false;
            break;
        case WHOLE_NUMBER_DIGIT:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = WHOLE_NUMBER_DIGITS;
            else
                return false;
            break;
        case WHOLE_NUMBER_DIGITS:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = WHOLE_NUMBER_DIGITS;
            else if (*it=='.')
                state = FRACTION;
            else if (*it=='p' || *it=='P')
                state = EXPONENT_DIGIT;
            else
                return false;
            break;
        case FRACTION:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = FRACTION;
            else if (*it=='p' || *it=='P')
                state = EXPONENT_DIGIT;
            break;
        case EXPONENT_DIGIT:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = EXPONENT_DIGITS;
            else if (*it=='+' || *it=='-')
                state = EXPONENT_DIGITS;
            else
                return false;
            break;
        case EXPONENT_DIGITS:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = EXPONENT_DIGITS;
            else
                return *it=='f'||*it=='F'||*it=='l'||*it=='L';
            break;
        }
    }
    return state==EXPONENT_DIGITS;
}

bool MathLib::isValidIntegerSuffix(std::string::const_iterator it, std::string::const_iterator end)
{
    enum {START, SUFFIX_U, SUFFIX_UL, SUFFIX_ULL, SUFFIX_L, SUFFIX_LU, SUFFIX_LL, SUFFIX_LLU, SUFFIX_I, SUFFIX_I6, SUFFIX_I64, SUFFIX_UI, SUFFIX_UI6, SUFFIX_UI64} state = START;
    for (; it != end; ++it) {
        switch (state) {
        case START:
            if (*it == 'u' || *it == 'U')
                state = SUFFIX_U;
            else if (*it == 'l' || *it == 'L')
                state = SUFFIX_L;
            else if (*it == 'i')
                state = SUFFIX_I;
            else
                return false;
            break;
        case SUFFIX_U:
            if (*it == 'l' || *it == 'L')
                state = SUFFIX_UL; // UL
            else if (*it == 'i')
                state = SUFFIX_UI;
            else
                return false;
            break;
        case SUFFIX_UL:
            if (*it == 'l' || *it == 'L')
                state = SUFFIX_ULL; // ULL
            else
                return false;
            break;
        case SUFFIX_L:
            if (*it == 'u' || *it == 'U')
                state = SUFFIX_LU; // LU
            else if (*it == 'l' || *it == 'L')
                state = SUFFIX_LL; // LL
            else
                return false;
            break;
        case SUFFIX_LU:
            return false;
        case SUFFIX_LL:
            if (*it == 'u' || *it == 'U')
                state = SUFFIX_LLU; // LLU
            else
                return false;
            break;
        case SUFFIX_I:
            if (*it == '6')
                state = SUFFIX_I6;
            else
                return false;
            break;
        case SUFFIX_I6:
            if (*it == '4')
                state = SUFFIX_I64;
            else
                return false;
            break;
        case SUFFIX_UI:
            if (*it == '6')
                state = SUFFIX_UI6;
            else
                return false;
            break;
        case SUFFIX_UI6:
            if (*it == '4')
                state = SUFFIX_UI64;
            else
                return false;
            break;
        default:
            return false;
        }
    }
    return ((state == SUFFIX_U)   ||
            (state == SUFFIX_L)   ||
            (state == SUFFIX_UL)  ||
            (state == SUFFIX_LU)  ||
            (state == SUFFIX_LL)  ||
            (state == SUFFIX_ULL) ||
            (state == SUFFIX_LLU) ||
            (state == SUFFIX_I64) ||
            (state == SUFFIX_UI64));
}

/*! \brief Does the string represent a binary number?
 * In case leading or trailing white space is provided, the function
 * returns false.
 * Additional information can be found here:
 * http://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html
 *
 * \param str The string to check. In case the string is empty, the function returns false.
 * \return Return true in case a binary number is provided and false otherwise.
 **/
bool MathLib::isBin(const std::string& str)
{
    enum Status {
        START, PLUSMINUS, GNU_BIN_PREFIX, DIGIT, DIGITS
    } state = START;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        switch (state) {
        case START:
            if (*it == '+' || *it == '-')
                state = PLUSMINUS;
            else if (*it == '0')
                state = GNU_BIN_PREFIX;
            else
                return false;
            break;
        case PLUSMINUS:
            if (*it == '0')
                state = GNU_BIN_PREFIX;
            else
                return false;
            break;
        case GNU_BIN_PREFIX:
            if (*it == 'b' || *it == 'B')
                state = DIGIT;
            else
                return false;
            break;
        case DIGIT:
            if (*it == '0' || *it == '1')
                state = DIGITS;
            else
                return false;
            break;
        case DIGITS:
            if (*it == '0' || *it == '1')
                state = DIGITS;
            else
                return isValidIntegerSuffix(it,str.end());
            break;
        }
    }
    return state == DIGITS;
}

bool MathLib::isDec(const std::string & str)
{
    enum Status {
        START, PLUSMINUS, DIGIT
    } state = START;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        switch (state) {
        case START:
            if (*it == '+' || *it == '-')
                state = PLUSMINUS;
            else if (isdigit(static_cast<unsigned char>(*it)))
                state = DIGIT;
            else
                return false;
            break;
        case PLUSMINUS:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = DIGIT;
            else
                return false;
            break;
        case DIGIT:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = DIGIT;
            else
                return isValidIntegerSuffix(it,str.end());
            break;
        }
    }
    return state == DIGIT;
}

bool MathLib::isInt(const std::string & str)
{
    return isDec(str) || isIntHex(str) || isOct(str) || isBin(str);
}

static std::string getsuffix(const std::string& value)
{
    if (value.size() > 3 && value[value.size() - 3] == 'i' && value[value.size() - 2] == '6' && value[value.size() - 1] == '4') {
        if (value[value.size() - 4] == 'u')
            return "ULL";
        return "LL";
    }
    bool isUnsigned = false;
    unsigned int longState = 0;
    for (std::size_t i = 1U; i < value.size(); ++i) {
        char c = value[value.size() - i];
        if (c == 'u' || c == 'U')
            isUnsigned = true;
        else if (c == 'L' || c == 'l')
            longState++;
        else break;
    }
    if (longState == 0)
        return isUnsigned ? "U" : "";
    if (longState == 1)
        return isUnsigned ? "UL" : "L";
    if (longState == 2)
        return isUnsigned ? "ULL" : "LL";
    else return "";
}

static std::string intsuffix(const std::string & first, const std::string & second)
{
    std::string suffix1 = getsuffix(first);
    std::string suffix2 = getsuffix(second);
    if (suffix1 == "ULL" || suffix2 == "ULL")
        return "ULL";
    if (suffix1 == "LL" || suffix2 == "LL")
        return "LL";
    if (suffix1 == "UL" || suffix2 == "UL")
        return "UL";
    if (suffix1 == "L" || suffix2 == "L")
        return "L";
    if (suffix1 == "U" || suffix2 == "U")
        return "U";

    return suffix1.empty() ? suffix2 : suffix1;
}

std::string MathLib::add(const std::string & first, const std::string & second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) + value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString(toLongNumber(first) + toLongNumber(second)) + intsuffix(first, second);
    }

    double d1 = toDoubleNumber(first);
    double d2 = toDoubleNumber(second);

    int count = 0;
    while (d1 > 100000.0 * d2 && toString(d1+d2)==first && ++count<5)
        d2 *= 10.0;
    while (d2 > 100000.0 * d1 && toString(d1+d2)==second && ++count<5)
        d1 *= 10.0;

    return toString(d1 + d2);
#endif
}

std::string MathLib::subtract(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) - value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString(toLongNumber(first) - toLongNumber(second)) + intsuffix(first, second);
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
#endif
}

std::string MathLib::incdec(const std::string & var, const std::string & op)
{
#ifdef TEST_MATHLIB_VALUE
    if (op == "++")
        return value(var).add(1).str();
    else if (op == "--")
        return value(var).add(-1).str();
#else
    if (op == "++")
        return MathLib::add(var, "1");
    else if (op == "--")
        return MathLib::subtract(var, "1");
#endif

    throw InternalError(0, std::string("Unexpected operation '") + op + "' in MathLib::incdec(). Please report this to Cppcheck developers.");
}

std::string MathLib::divide(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) / value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        const bigint a = toLongNumber(first);
        const bigint b = toLongNumber(second);
        if (b == 0)
            throw InternalError(0, "Internal Error: Division by zero");
        if (a == std::numeric_limits<bigint>::min() && std::abs(b)<=1)
            throw InternalError(0, "Internal Error: Division overflow");
        return toString(toLongNumber(first) / b) + intsuffix(first, second);
    } else if (isNullValue(second)) {
        if (isNullValue(first))
            return "nan.0";
        return isPositive(first) ? "inf.0" : "-inf.0";
    }
    return toString(toDoubleNumber(first) / toDoubleNumber(second));
#endif
}

std::string MathLib::multiply(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) * value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        return toString(toLongNumber(first) * toLongNumber(second)) + intsuffix(first, second);
    }
    return toString(toDoubleNumber(first) * toDoubleNumber(second));
#endif
}

std::string MathLib::mod(const std::string &first, const std::string &second)
{
#ifdef TEST_MATHLIB_VALUE
    return (value(first) % value(second)).str();
#else
    if (MathLib::isInt(first) && MathLib::isInt(second)) {
        const bigint b = toLongNumber(second);
        if (b == 0)
            throw InternalError(0, "Internal Error: Division by zero");
        return toString(toLongNumber(first) % b) + intsuffix(first, second);
    }
    return toString(std::fmod(toDoubleNumber(first),toDoubleNumber(second)));
#endif
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
        return MathLib::toString(MathLib::toLongNumber(first) & MathLib::toLongNumber(second)) + intsuffix(first,second);

    case '|':
        return MathLib::toString(MathLib::toLongNumber(first) | MathLib::toLongNumber(second)) + intsuffix(first,second);

    case '^':
        return MathLib::toString(MathLib::toLongNumber(first) ^ MathLib::toLongNumber(second)) + intsuffix(first,second);

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

/*! \brief Does the string represent the numerical value of 0?
 * In case leading or trailing white space is provided, the function
 * returns false.
 * Requirement for this function:
 * - This code is allowed to be slow because of simplicity of the code.
 *
 * \param[in] str The string to check. In case the string is empty, the function returns false.
 * \return Return true in case the string represents a numerical null value.
 **/
bool MathLib::isNullValue(const std::string &str)
{
    if (str.empty() || (!std::isdigit(static_cast<unsigned char>(str[0])) && (str.size() < 1 || (str[0] != '.' && str[0] != '-' && str[0] != '+'))))
        return false; // Has to be a number

    for (size_t i = 0; i < str.size(); i++) {
        if (std::isdigit(static_cast<unsigned char>(str[i])) && str[i] != '0') // May not contain digits other than 0
            return false;
        if (str[i] == 'E' || str[i] == 'e')
            return true;
    }
    return true;
}

bool MathLib::isOctalDigit(char c)
{
    return (c >= '0' && c <= '7');
}

bool MathLib::isDigitSeparator(const std::string& iCode, std::string::size_type iPos)
{
    if (iPos == 0 || iPos >= iCode.size() || iCode[iPos] != '\'')
        return false;
    std::string::size_type i = iPos - 1;
    while (std::isxdigit(iCode[i])) {
        if (i == 0)
            return true; // Only xdigits before '
        --i;
    }
    if (i == iPos - 1) { // No xdigit before '
        return false;
    } else {
        switch (iCode[i]) {
        case ' ':
        case '.':
        case ',':
        case 'x':
        case '(':
        case '{':
        case '+':
        case '-':
        case '*':
        case '%':
        case '/':
        case '&':
        case '|':
        case '^':
        case '~':
        case '=':
            return true;
        case '\'':
            return isDigitSeparator(iCode, i);
        default:
            return false;
        }
    }
}

MathLib::value operator+(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('+',v1,v2);
}

MathLib::value operator-(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('-',v1,v2);
}

MathLib::value operator*(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('*',v1,v2);
}

MathLib::value operator/(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('/',v1,v2);
}

MathLib::value operator%(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('%',v1,v2);
}

MathLib::value operator&(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('&',v1,v2);
}

MathLib::value operator|(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('|',v1,v2);
}

MathLib::value operator^(const MathLib::value &v1, const MathLib::value &v2)
{
    return MathLib::value::calc('^',v1,v2);
}

MathLib::value operator<<(const MathLib::value &v1, const MathLib::value &v2)
{
    return v1.shiftLeft(v2);
}

MathLib::value operator>>(const MathLib::value &v1, const MathLib::value &v2)
{
    return v1.shiftRight(v2);
}

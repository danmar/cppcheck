/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#include "errortypes.h"
#include "utils.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <locale>

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

const int MathLib::bigint_bits = 64;

MathLib::value::value(const std::string &s) :
    mIntValue(0), mDoubleValue(0), mIsUnsigned(false)
{
    if (MathLib::isFloat(s)) {
        mType = MathLib::value::Type::FLOAT;
        mDoubleValue = MathLib::toDoubleNumber(s);
        return;
    }

    if (!MathLib::isInt(s))
        throw InternalError(nullptr, "Invalid value: " + s);

    mType = MathLib::value::Type::INT;
    mIntValue = MathLib::toLongNumber(s);

    if (isIntHex(s) && mIntValue < 0)
        mIsUnsigned = true;

    // read suffix
    if (s.size() >= 2U) {
        for (std::size_t i = s.size() - 1U; i > 0U; --i) {
            const char c = s[i];
            if (c == 'u' || c == 'U')
                mIsUnsigned = true;
            else if (c == 'l' || c == 'L') {
                if (mType == MathLib::value::Type::INT)
                    mType = MathLib::value::Type::LONG;
                else if (mType == MathLib::value::Type::LONG)
                    mType = MathLib::value::Type::LONGLONG;
            } else if (i > 2U && c == '4' && s[i-1] == '6' && s[i-2] == 'i')
                mType = MathLib::value::Type::LONGLONG;
        }
    }
}

std::string MathLib::value::str() const
{
    std::ostringstream ostr;
    if (mType == MathLib::value::Type::FLOAT) {
        if (ISNAN(mDoubleValue))
            return "nan.0";
        if (ISINF(mDoubleValue))
            return (mDoubleValue > 0) ? "inf.0" : "-inf.0";

        ostr.precision(9);
        ostr << std::fixed << mDoubleValue;

        // remove trailing zeros
        std::string ret(ostr.str());
        std::string::size_type pos = ret.size() - 1U;
        while (ret[pos] == '0')
            pos--;
        if (ret[pos] == '.')
            ++pos;

        return ret.substr(0, pos+1);
    }

    if (mIsUnsigned)
        ostr << static_cast<biguint>(mIntValue) << "U";
    else
        ostr << mIntValue;
    if (mType == MathLib::value::Type::LONG)
        ostr << "L";
    else if (mType == MathLib::value::Type::LONGLONG)
        ostr << "LL";
    return ostr.str();
}

void MathLib::value::promote(const MathLib::value &v)
{
    if (isInt() && v.isInt()) {
        if (mType < v.mType) {
            mType = v.mType;
            mIsUnsigned = v.mIsUnsigned;
        } else if (mType == v.mType) {
            mIsUnsigned |= v.mIsUnsigned;
        }
    } else if (!isFloat()) {
        mIsUnsigned = false;
        mDoubleValue = mIntValue;
        mType = MathLib::value::Type::FLOAT;
    }
}


MathLib::value MathLib::value::calc(char op, const MathLib::value &v1, const MathLib::value &v2)
{
    value temp(v1);
    temp.promote(v2);
    if (temp.isFloat()) {
        switch (op) {
        case '+':
            temp.mDoubleValue += v2.getDoubleValue();
            break;
        case '-':
            temp.mDoubleValue -= v2.getDoubleValue();
            break;
        case '*':
            temp.mDoubleValue *= v2.getDoubleValue();
            break;
        case '/':
            temp.mDoubleValue /= v2.getDoubleValue();
            break;
        case '%':
        case '&':
        case '|':
        case '^':
            throw InternalError(nullptr, "Invalid calculation");
        default:
            throw InternalError(nullptr, "Unhandled calculation");
        }
    } else if (temp.mIsUnsigned) {
        switch (op) {
        case '+':
            temp.mIntValue += (unsigned long long)v2.mIntValue;
            break;
        case '-':
            temp.mIntValue -= (unsigned long long)v2.mIntValue;
            break;
        case '*':
            temp.mIntValue *= (unsigned long long)v2.mIntValue;
            break;
        case '/':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            if (v1.mIntValue == std::numeric_limits<bigint>::min() && std::abs(v2.mIntValue)<=1)
                throw InternalError(nullptr, "Internal Error: Division overflow");
            temp.mIntValue /= (unsigned long long)v2.mIntValue;
            break;
        case '%':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            temp.mIntValue %= (unsigned long long)v2.mIntValue;
            break;
        case '&':
            temp.mIntValue &= (unsigned long long)v2.mIntValue;
            break;
        case '|':
            temp.mIntValue |= (unsigned long long)v2.mIntValue;
            break;
        case '^':
            temp.mIntValue ^= (unsigned long long)v2.mIntValue;
            break;
        default:
            throw InternalError(nullptr, "Unhandled calculation");
        }
    } else {
        switch (op) {
        case '+':
            temp.mIntValue += v2.mIntValue;
            break;
        case '-':
            temp.mIntValue -= v2.mIntValue;
            break;
        case '*':
            temp.mIntValue *= v2.mIntValue;
            break;
        case '/':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            if (v1.mIntValue == std::numeric_limits<bigint>::min() && std::abs(v2.mIntValue)<=1)
                throw InternalError(nullptr, "Internal Error: Division overflow");
            temp.mIntValue /= v2.mIntValue;
            break;
        case '%':
            if (v2.mIntValue == 0)
                throw InternalError(nullptr, "Internal Error: Division by zero");
            temp.mIntValue %= v2.mIntValue;
            break;
        case '&':
            temp.mIntValue &= v2.mIntValue;
            break;
        case '|':
            temp.mIntValue |= v2.mIntValue;
            break;
        case '^':
            temp.mIntValue ^= v2.mIntValue;
            break;
        default:
            throw InternalError(nullptr, "Unhandled calculation");
        }
    }
    return temp;
}


int MathLib::value::compare(const MathLib::value &v) const
{
    value temp(*this);
    temp.promote(v);

    if (temp.isFloat()) {
        if (temp.mDoubleValue < v.getDoubleValue())
            return -1;
        if (temp.mDoubleValue > v.getDoubleValue())
            return 1;
        return 0;
    }

    if (temp.mIsUnsigned) {
        if ((unsigned long long)mIntValue < (unsigned long long)v.mIntValue)
            return -1;
        if ((unsigned long long)mIntValue > (unsigned long long)v.mIntValue)
            return 1;
        return 0;
    }

    if (mIntValue < v.mIntValue)
        return -1;
    if (mIntValue > v.mIntValue)
        return 1;
    return 0;
}

MathLib::value MathLib::value::add(int v) const
{
    MathLib::value temp(*this);
    if (temp.isInt())
        temp.mIntValue += v;
    else
        temp.mDoubleValue += v;
    return temp;
}

MathLib::value MathLib::value::shiftLeft(const MathLib::value &v) const
{
    if (!isInt() || !v.isInt())
        throw InternalError(nullptr, "Shift operand is not integer");
    MathLib::value ret(*this);
    if (v.mIntValue >= MathLib::bigint_bits) {
        return ret;
    }
    ret.mIntValue <<= v.mIntValue;
    return ret;
}

MathLib::value MathLib::value::shiftRight(const MathLib::value &v) const
{
    if (!isInt() || !v.isInt())
        throw InternalError(nullptr, "Shift operand is not integer");
    MathLib::value ret(*this);
    if (v.mIntValue >= MathLib::bigint_bits) {
        return ret;
    }
    ret.mIntValue >>= v.mIntValue;
    return ret;
}


MathLib::biguint MathLib::toULongNumber(const std::string & str)
{
    // hexadecimal numbers:
    if (isIntHex(str)) {
        const biguint ret = std::stoull(str, nullptr, 16);
        return ret;
    }

    // octal numbers:
    if (isOct(str)) {
        const biguint ret = std::stoull(str, nullptr, 8);
        return ret;
    }

    // binary numbers:
    if (isBin(str)) {
        biguint ret = 0;
        for (std::string::size_type i = str[0] == '0'?2:3; i < str.length(); i++) {
            if (str[i] != '1' && str[i] != '0')
                break;
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

    const biguint ret = std::stoull(str, nullptr, 10);
    return ret;
}

unsigned int MathLib::encodeMultiChar(const std::string& str)
{
    unsigned int retval = 0;
    for (char it : str) {
        retval = (retval << 8) | it;
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
        return std::strtoul(str.substr(2).c_str(), nullptr, 16);
    }

    // '\123'
    if (str.size() == 4 && str[0] == '\\' && isoctal(str[1]) && isoctal(str[2]) && isoctal(str[3])) {
        return (char)std::strtoul(str.substr(1).c_str(), nullptr, 8);
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
            throw InternalError(nullptr, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
        }
        switch (iLiteral[idx]) {
        case 'x':
            // Hexa-decimal number: skip \x and interpret the next two characters
        {
            if (++idx == iLiteralLen)
                throw InternalError(nullptr, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
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
                throw InternalError(nullptr, "Internal Error. MathLib::characterLiteralToLongNumber: Unhandled char constant '" + iLiteral + "'.");
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
                throw InternalError(nullptr, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
            }
            continue;
        }
        // 2-3 digit octal number
        if (!MathLib::isOctalDigit(iLiteral[idx]))
            throw InternalError(nullptr, "Internal Error. MathLib::normalizeCharacterLiteral: Unhandled char constant '" + iLiteral + "'.");
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
            const bigint ret = std::stoll(str, nullptr, 16);
            return ret;
        } else {
            const biguint ret = std::stoull(str, nullptr, 16);
            return (bigint)ret;
        }
    }

    // octal numbers:
    if (isOct(str)) {
        const bigint ret = std::stoll(str, nullptr, 8);
        return ret;
    }

    // binary numbers:
    if (isBin(str)) {
        bigint ret = 0;
        for (std::string::size_type i = str[0] == '0'?2:3; i < str.length(); i++) {
            if (str[i] != '1' && str[i] != '0')
                break;
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

    if (isCharLiteral(str)) {
        return characterLiteralToLongNumber(getCharLiteral(str));
    }

    if (str[0] == '-') {
        const bigint ret = std::stoll(str, nullptr, 10);
        return ret;
    }

    const biguint ret = std::stoull(str, nullptr, 10);
    return ret;
}

// in-place conversion of (sub)string to double. Requires no heap.
static double myStod(const std::string& str, std::string::const_iterator from, std::string::const_iterator to, int base)
{
    double result = 0.;
    bool positivesign = true;
    std::string::const_iterator it;
    if ('+' == *from) {
        it = from + 1;
    } else if ('-' == *from) {
        it = from + 1;
        positivesign = false;
    } else
        it = from;
    const std::size_t decimalsep = str.find('.', it-str.begin());
    int distance;
    if (std::string::npos == decimalsep) {
        distance = to - it;
    } else  if (decimalsep > (to - str.begin()))
        return 0.; // error handling??
    else
        distance = int(decimalsep)-(from - str.begin());
    auto digitval = [&](char c) {
        if ((10 < base) && (c > '9'))
            return 10 + std::tolower(c) - 'a';
        else
            return c - '0';
    };
    for (; it!=to; ++it) {
        if ('.' == *it)
            continue;
        else
            --distance;
        result += digitval(*it)* std::pow(base, distance);
    }
    return (positivesign)?result:-result;
}


// Assuming a limited support of built-in hexadecimal floats (see C99, C++17) that is a fall-back implementation.
// Performance has been optimized WRT to heap activity, however the calculation part is not optimized.
static double floatHexToDoubleNumber(const std::string& str)
{
    const std::size_t p = str.find_first_of("pP",3);
    const double factor1 = myStod(str, str.begin() + 2, str.begin()+p, 16);
    const bool suffix = (str.back() == 'f') || (str.back() == 'F') || (str.back() == 'l') || (str.back() == 'L');
    const double exponent = myStod(str, str.begin() + p + 1, (suffix)?str.end()-1:str.end(), 10);
    const double factor2 = std::pow(2, exponent);
    return factor1 * factor2;
}

double MathLib::toDoubleNumber(const std::string &str)
{
    if (isCharLiteral(str))
        return characterLiteralToLongNumber(getCharLiteral(str));
    if (isIntHex(str))
        return static_cast<double>(toLongNumber(str));
    // nullcheck
    if (isNullValue(str))
        return 0.0;
#ifdef __clang__
    if (isFloat(str)) // Workaround libc++ bug at http://llvm.org/bugs/show_bug.cgi?id=17782
        // TODO : handle locale
        return std::strtod(str.c_str(), nullptr);
#endif
    if (isFloatHex(str))
        return floatHexToDoubleNumber(str);
    // otherwise, convert to double
    std::istringstream istr(str);
    istr.imbue(std::locale::classic());
    double ret;
    istr >> ret;
    return ret;
}

template<> std::string MathLib::toString<double>(double value)
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
    enum class State {
        START, BASE_DIGITS1, LEADING_DECIMAL, TRAILING_DECIMAL, BASE_DIGITS2, E, MANTISSA_PLUSMINUS, MANTISSA_DIGITS, SUFFIX_F, SUFFIX_L
    } state = State::START;
    std::string::const_iterator it = str.begin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.end(); ++it) {
        switch (state) {
        case State::START:
            if (*it=='.')
                state = State::LEADING_DECIMAL;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::BASE_DIGITS1;
            else
                return false;
            break;
        case State::LEADING_DECIMAL:
            if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::BASE_DIGITS2;
            else
                return false;
            break;
        case State::BASE_DIGITS1:
            if (*it=='e' || *it=='E')
                state = State::E;
            else if (*it=='.')
                state = State::TRAILING_DECIMAL;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case State::TRAILING_DECIMAL:
            if (*it=='e' || *it=='E')
                state = State::E;
            else if (*it=='f' || *it=='F')
                state = State::SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state = State::SUFFIX_L;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::BASE_DIGITS2;
            else
                return false;
            break;
        case State::BASE_DIGITS2:
            if (*it=='e' || *it=='E')
                state = State::E;
            else if (*it=='f' || *it=='F')
                state = State::SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state = State::SUFFIX_L;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case State::E:
            if (*it=='+' || *it=='-')
                state = State::MANTISSA_PLUSMINUS;
            else if (std::isdigit(static_cast<unsigned char>(*it)))
                state = State::MANTISSA_DIGITS;
            else
                return false;
            break;
        case State::MANTISSA_PLUSMINUS:
            if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            else
                state = State::MANTISSA_DIGITS;
            break;
        case State::MANTISSA_DIGITS:
            if (*it=='f' || *it=='F')
                state = State::SUFFIX_F;
            else if (*it=='l' || *it=='L')
                state = State::SUFFIX_L;
            else if (!std::isdigit(static_cast<unsigned char>(*it)))
                return false;
            break;
        case State::SUFFIX_F:
            return false;
        case State::SUFFIX_L:
            return false;
        }
    }
    return (state==State::BASE_DIGITS2 || state==State::MANTISSA_DIGITS || state==State::TRAILING_DECIMAL || state==State::SUFFIX_F || state==State::SUFFIX_L);
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

static bool isValidIntegerSuffixIt(std::string::const_iterator it, std::string::const_iterator end, bool supportMicrosoftExtensions=true)
{
    enum class Status { START, SUFFIX_U, SUFFIX_UL, SUFFIX_ULL, SUFFIX_L, SUFFIX_LU, SUFFIX_LL, SUFFIX_LLU, SUFFIX_I, SUFFIX_I6, SUFFIX_I64, SUFFIX_UI, SUFFIX_UI6, SUFFIX_UI64 } state = Status::START;
    for (; it != end; ++it) {
        switch (state) {
        case Status::START:
            if (*it == 'u' || *it == 'U')
                state = Status::SUFFIX_U;
            else if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_L;
            else if (supportMicrosoftExtensions && (*it == 'i' || *it == 'I'))
                state = Status::SUFFIX_I;
            else
                return false;
            break;
        case Status::SUFFIX_U:
            if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_UL; // UL
            else if (supportMicrosoftExtensions && (*it == 'i' || *it == 'I'))
                state = Status::SUFFIX_UI;
            else
                return false;
            break;
        case Status::SUFFIX_UL:
            if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_ULL; // ULL
            else
                return false;
            break;
        case Status::SUFFIX_L:
            if (*it == 'u' || *it == 'U')
                state = Status::SUFFIX_LU; // LU
            else if (*it == 'l' || *it == 'L')
                state = Status::SUFFIX_LL; // LL
            else
                return false;
            break;
        case Status::SUFFIX_LU:
            return false;
        case Status::SUFFIX_LL:
            if (*it == 'u' || *it == 'U')
                state = Status::SUFFIX_LLU; // LLU
            else
                return false;
            break;
        case Status::SUFFIX_I:
            if (*it == '6')
                state = Status::SUFFIX_I6;
            else
                return false;
            break;
        case Status::SUFFIX_I6:
            if (*it == '4')
                state = Status::SUFFIX_I64;
            else
                return false;
            break;
        case Status::SUFFIX_UI:
            if (*it == '6')
                state = Status::SUFFIX_UI6;
            else
                return false;
            break;
        case Status::SUFFIX_UI6:
            if (*it == '4')
                state = Status::SUFFIX_UI64;
            else
                return false;
            break;
        default:
            return false;
        }
    }
    return ((state == Status::SUFFIX_U) ||
            (state == Status::SUFFIX_L) ||
            (state == Status::SUFFIX_UL) ||
            (state == Status::SUFFIX_LU) ||
            (state == Status::SUFFIX_LL) ||
            (state == Status::SUFFIX_ULL) ||
            (state == Status::SUFFIX_LLU) ||
            (state == Status::SUFFIX_I64) ||
            (state == Status::SUFFIX_UI64));
}

bool MathLib::isValidIntegerSuffix(const std::string& str, bool supportMicrosoftExtensions)
{
    return isValidIntegerSuffixIt(str.begin(), str.end(), supportMicrosoftExtensions);
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
    enum class Status {
        START, OCTAL_PREFIX, DIGITS
    } state = Status::START;
    if (str.empty())
        return false;
    std::string::const_iterator it = str.begin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.end(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::OCTAL_PREFIX;
            else
                return false;
            break;
        case Status::OCTAL_PREFIX:
            if (isOctalDigit(static_cast<unsigned char>(*it)))
                state = Status::DIGITS;
            else
                return false;
            break;
        case Status::DIGITS:
            if (isOctalDigit(static_cast<unsigned char>(*it)))
                state = Status::DIGITS;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return state == Status::DIGITS;
}

bool MathLib::isIntHex(const std::string& str)
{
    enum class Status {
        START, HEX_0, HEX_X, DIGIT
    } state = Status::START;
    if (str.empty())
        return false;
    std::string::const_iterator it = str.begin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.end(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::HEX_0;
            else
                return false;
            break;
        case Status::HEX_0:
            if (*it == 'x' || *it == 'X')
                state = Status::HEX_X;
            else
                return false;
            break;
        case Status::HEX_X:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = Status::DIGIT;
            else
                return false;
            break;
        case Status::DIGIT:
            if (isxdigit(static_cast<unsigned char>(*it)))
                ; //  state = Status::DIGIT;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return Status::DIGIT == state;
}

bool MathLib::isFloatHex(const std::string& str)
{
    enum class Status {
        START, HEX_0, HEX_X, WHOLE_NUMBER_DIGIT, POINT, FRACTION, EXPONENT_P, EXPONENT_SIGN, EXPONENT_DIGITS, EXPONENT_SUFFIX
    } state = Status::START;
    if (str.empty())
        return false;
    std::string::const_iterator it = str.begin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.end(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::HEX_0;
            else
                return false;
            break;
        case Status::HEX_0:
            if (*it == 'x' || *it == 'X')
                state = Status::HEX_X;
            else
                return false;
            break;
        case Status::HEX_X:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = Status::WHOLE_NUMBER_DIGIT;
            else if (*it == '.')
                state = Status::POINT;
            else
                return false;
            break;
        case Status::WHOLE_NUMBER_DIGIT:
            if (isxdigit(static_cast<unsigned char>(*it)))
                ; // state = Status::WHOLE_NUMBER_DIGITS;
            else if (*it=='.')
                state = Status::FRACTION;
            else if (*it=='p' || *it=='P')
                state = Status::EXPONENT_P;
            else
                return false;
            break;
        case Status::POINT:
        case Status::FRACTION:
            if (isxdigit(static_cast<unsigned char>(*it)))
                state = Status::FRACTION;
            else if (*it == 'p' || *it == 'P')
                state = Status::EXPONENT_P;
            else
                return false;
            break;
        case Status::EXPONENT_P:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::EXPONENT_DIGITS;
            else if (*it == '+' || *it == '-')
                state = Status::EXPONENT_SIGN;
            else
                return false;
            break;
        case Status::EXPONENT_SIGN:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::EXPONENT_DIGITS;
            else
                return false;
            break;
        case Status::EXPONENT_DIGITS:
            if (isdigit(static_cast<unsigned char>(*it)))
                ; //  state = Status::EXPONENT_DIGITS;
            else if (*it == 'f' || *it == 'F' || *it == 'l' || *it == 'L')
                state = Status::EXPONENT_SUFFIX;
            else
                return false;
            break;
        case Status::EXPONENT_SUFFIX:
            return false;
        }
    }
    return (Status::EXPONENT_DIGITS == state) || (Status::EXPONENT_SUFFIX == state);
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
    enum class Status {
        START, GNU_BIN_PREFIX_0, GNU_BIN_PREFIX_B, DIGIT
    } state = Status::START;
    if (str.empty())
        return false;
    std::string::const_iterator it = str.begin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.end(); ++it) {
        switch (state) {
        case Status::START:
            if (*it == '0')
                state = Status::GNU_BIN_PREFIX_0;
            else
                return false;
            break;
        case Status::GNU_BIN_PREFIX_0:
            if (*it == 'b' || *it == 'B')
                state = Status::GNU_BIN_PREFIX_B;
            else
                return false;
            break;
        case Status::GNU_BIN_PREFIX_B:
            if (*it == '0' || *it == '1')
                state = Status::DIGIT;
            else
                return false;
            break;
        case Status::DIGIT:
            if (*it == '0' || *it == '1')
                ; //  state = Status::DIGIT;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return state == Status::DIGIT;
}

bool MathLib::isDec(const std::string & str)
{
    enum class Status {
        START, DIGIT
    } state = Status::START;
    if (str.empty())
        return false;
    std::string::const_iterator it = str.begin();
    if ('+' == *it || '-' == *it)
        ++it;
    for (; it != str.end(); ++it) {
        switch (state) {
        case Status::START:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::DIGIT;
            else
                return false;
            break;
        case Status::DIGIT:
            if (isdigit(static_cast<unsigned char>(*it)))
                state = Status::DIGIT;
            else
                return isValidIntegerSuffixIt(it,str.end());
            break;
        }
    }
    return state == Status::DIGIT;
}

bool MathLib::isInt(const std::string & str)
{
    return isDec(str) || isIntHex(str) || isOct(str) || isBin(str);
}

std::string MathLib::getSuffix(const std::string& value)
{
    if (value.size() > 3 && value[value.size() - 3] == 'i' && value[value.size() - 2] == '6' && value[value.size() - 1] == '4') {
        if (value[value.size() - 4] == 'u')
            return "ULL";
        return "LL";
    }
    bool isUnsigned = false;
    unsigned int longState = 0;
    for (std::size_t i = 1U; i < value.size(); ++i) {
        const char c = value[value.size() - i];
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
    const std::string suffix1 = MathLib::getSuffix(first);
    const std::string suffix2 = MathLib::getSuffix(second);
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

    throw InternalError(nullptr, std::string("Unexpected operation '") + op + "' in MathLib::incdec(). Please report this to Cppcheck developers.");
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
            throw InternalError(nullptr, "Internal Error: Division by zero");
        if (a == std::numeric_limits<bigint>::min() && std::abs(b)<=1)
            throw InternalError(nullptr, "Internal Error: Division overflow");
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
            throw InternalError(nullptr, "Internal Error: Division by zero");
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
        throw InternalError(nullptr, std::string("Unexpected action '") + action + "' in MathLib::calculate(). Please report this to Cppcheck developers.");
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
    if (isNegative(tok))
        return tok.substr(1, tok.length() - 1);
    return tok;
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
    if (str.empty() || (!std::isdigit(static_cast<unsigned char>(str[0])) && (str[0] != '.' && str[0] != '-' && str[0] != '+')))
        return false; // Has to be a number

    if (!isInt(str) && !isFloat(str))
        return false;
    bool isHex = isIntHex(str) || isFloatHex(str);
    for (char i : str) {
        if (std::isdigit(static_cast<unsigned char>(i)) && i != '0') // May not contain digits other than 0
            return false;
        if (i == 'p' || i == 'P' || (!isHex && (i == 'E' || i == 'e')))
            return true;
        if (isHex && isxdigit(i) && i != '0')
            return false;
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

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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
#ifndef utilsH
#define utilsH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstddef>
#include <string>
#include <vector>

struct SelectMapKeys {
    template <class Pair>
    typename Pair::first_type operator()(const Pair& p) const {
        return p.first;
    }
};

struct SelectMapValues {
    template <class Pair>
    typename Pair::second_type operator()(const Pair& p) const {
        return p.second;
    }
};

inline bool endsWith(const std::string &str, char c)
{
    return str[str.size()-1U] == c;
}

inline bool endsWith(const std::string &str, const char end[], std::size_t endlen)
{
    return (str.size() >= endlen) && (str.compare(str.size()-endlen, endlen, end)==0);
}

inline static bool isPrefixStringCharLiteral(const std::string &str, char q, const std::string& p)
{
    if (!endsWith(str, q))
        return false;
    if ((str.length() + 1) > p.length() && (str.compare(0, p.size() + 1, p + q) == 0))
        return true;
    return false;
}

inline static bool isStringCharLiteral(const std::string &str, char q)
{
    static const std::vector<std::string> suffixes{"", "u8", "u", "U", "L"};
    for (const std::string & p: suffixes) {
        if (isPrefixStringCharLiteral(str, q, p))
            return true;
    }
    return false;
}

inline static bool isStringLiteral(const std::string &str)
{
    return isStringCharLiteral(str, '"');
}

inline static bool isCharLiteral(const std::string &str)
{
    return isStringCharLiteral(str, '\'');
}

inline static std::string getStringCharLiteral(const std::string &str, char q)
{
    const std::size_t quotePos = str.find(q);
    return str.substr(quotePos + 1U, str.size() - quotePos - 2U);
}

inline static std::string getStringLiteral(const std::string &str)
{
    if (isStringLiteral(str))
        return getStringCharLiteral(str, '"');
    return "";
}

inline static std::string getCharLiteral(const std::string &str)
{
    if (isCharLiteral(str))
        return getStringCharLiteral(str, '\'');
    return "";
}

inline static const char *getOrdinalText(int i)
{
    if (i == 1)
        return "st";
    if (i == 2)
        return "nd";
    if (i == 3)
        return "rd";
    return "th";
}

CPPCHECKLIB int caseInsensitiveStringCompare(const std::string& lhs, const std::string& rhs);

CPPCHECKLIB bool isValidGlobPattern(const std::string& pattern);

CPPCHECKLIB bool matchglob(const std::string& pattern, const std::string& name);

#define UNUSED(x) (void)(x)

// Use the nonneg macro when you want to assert that a variable/argument is not negative
#ifdef __CPPCHECK__
#define nonneg   __cppcheck_low__(0)
#elif defined(NONNEG)
// Enable non-negative values checking
// TODO : investigate using annotations/contracts for stronger value checking
#define nonneg   unsigned
#else
// Disable non-negative values checking
#define nonneg
#endif

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define ASAN 1
#endif
#endif

#ifndef ASAN
#ifdef  __SANITIZE_ADDRESS__
#define ASAN 1
#else
#define ASAN 0
#endif
#endif

#endif

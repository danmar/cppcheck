/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>

struct SelectMapKeys {
    template<class Pair>
    typename Pair::first_type operator()(const Pair& p) const {
        return p.first;
    }
};

struct SelectMapValues {
    template<class Pair>
    typename Pair::second_type operator()(const Pair& p) const {
        return p.second;
    }
};

template<class Range, class T>
bool contains(const Range& r, const T& x)
{
    return std::find(r.cbegin(), r.cend(), x) != r.cend();
}

template<class T>
bool contains(const std::initializer_list<T>& r, const T& x)
{
    return std::find(r.begin(), r.end(), x) != r.end();
}

template<class T, class U>
bool contains(const std::initializer_list<T>& r, const U& x)
{
    return std::find(r.begin(), r.end(), x) != r.end();
}

template<class T, class ... Ts>
inline std::array<T, sizeof...(Ts) + 1> makeArray(T x, Ts... xs)
{
    return {std::move(x), std::move(xs)...};
}

// Enum hash for C++11. This is not needed in C++14
struct EnumClassHash {
    template<typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

inline bool endsWith(const std::string &str, char c)
{
    return !str.empty() && str.back() == c;
}

inline bool endsWith(const std::string &str, const char end[], std::size_t endlen)
{
    return (str.size() >= endlen) && (str.compare(str.size()-endlen, endlen, end)==0);
}

template<std::size_t N>
bool endsWith(const std::string& str, const char (&end)[N])
{
    return endsWith(str, end, N - 1);
}

inline static bool isPrefixStringCharLiteral(const std::string &str, char q, const std::string& p)
{
    // str must be at least the prefix plus the start and end quote
    if (str.length() < p.length() + 2)
        return false;

    // check for end quote
    if (!endsWith(str, q))
        return false;

    // check for start quote
    if (str[p.size()] != q)
        return false;

    // check for prefix
    if (str.compare(0, p.size(), p) != 0)
        return false;

    return true;
}

inline static bool isStringCharLiteral(const std::string &str, char q)
{
    // early out to avoid the loop
    if (!endsWith(str, q))
        return false;

    static const std::array<std::string, 5> suffixes{"", "u8", "u", "U", "L"};
    return std::any_of(suffixes.cbegin(), suffixes.cend(), [&](const std::string& p) {
        return isPrefixStringCharLiteral(str, q, p);
    });
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

CPPCHECKLIB bool matchglobs(const std::vector<std::string> &patterns, const std::string &name);

CPPCHECKLIB void strTolower(std::string& str);

/**
 *  Simple helper function:
 * \return size of array
 * */
template<typename T, int size>
std::size_t getArrayLength(const T (& /*unused*/)[size])
{
    return size;
}

#endif

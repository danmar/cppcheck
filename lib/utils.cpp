/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "utils.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <stack>
#include <utility>


int caseInsensitiveStringCompare(const std::string &lhs, const std::string &rhs)
{
    if (lhs.size() != rhs.size())
        return (lhs.size() < rhs.size()) ? -1 : 1;
    for (unsigned int i = 0; i < lhs.size(); ++i) {
        const int c1 = std::toupper(lhs[i]);
        const int c2 = std::toupper(rhs[i]);
        if (c1 != c2)
            return (c1 < c2) ? -1 : 1;
    }
    return 0;
}

bool isValidGlobPattern(const std::string& pattern)
{
    for (auto i = pattern.cbegin(); i != pattern.cend(); ++i) {
        if (*i == '*' || *i == '?') {
            const auto j = i + 1;
            if (j != pattern.cend() && (*j == '*' || *j == '?')) {
                return false;
            }
        }
    }
    return true;
}

bool matchglob(const std::string& pattern, const std::string& name)
{
    const char* p = pattern.c_str();
    const char* n = name.c_str();
    std::stack<std::pair<const char*, const char*>, std::vector<std::pair<const char*, const char*>>> backtrack;

    for (;;) {
        bool matching = true;
        while (*p != '\0' && matching) {
            switch (*p) {
            case '*':
                // Step forward until we match the next character after *
                while (*n != '\0' && *n != p[1]) {
                    n++;
                }
                if (*n != '\0') {
                    // If this isn't the last possibility, save it for later
                    backtrack.emplace(p, n);
                }
                break;
            case '?':
                // Any character matches unless we're at the end of the name
                if (*n != '\0') {
                    n++;
                } else {
                    matching = false;
                }
                break;
            default:
                // Non-wildcard characters match literally
                if (*n == *p) {
                    n++;
                } else if (*n == '\\' && *p == '/') {
                    n++;
                } else if (*n == '/' && *p == '\\') {
                    n++;
                } else {
                    matching = false;
                }
                break;
            }
            p++;
        }

        // If we haven't failed matching and we've reached the end of the name, then success
        if (matching && *n == '\0') {
            return true;
        }

        // If there are no other paths to try, then fail
        if (backtrack.empty()) {
            return false;
        }

        // Restore pointers from backtrack stack
        p = backtrack.top().first;
        n = backtrack.top().second;
        backtrack.pop();

        // Advance name pointer by one because the current position didn't work
        n++;
    }
}

bool matchglobs(const std::vector<std::string> &patterns, const std::string &name) {
    return std::any_of(begin(patterns), end(patterns), [&name](const std::string &pattern) {
        return matchglob(pattern, name);
    });
}

void strTolower(std::string& str)
{
    // This wrapper exists because Sun's CC does not allow a static_cast
    // from extern "C" int(*)(int) to int(*)(int).
    std::transform(str.cbegin(), str.cend(), str.begin(), [](int c) {
        return std::tolower(c);
    });
}

std::string trim(const std::string& s, const std::string& t)
{
    const std::string::size_type beg = s.find_first_not_of(t);
    if (beg == std::string::npos)
        return "";
    const std::string::size_type end = s.find_last_not_of(t);
    return s.substr(beg, end - beg + 1);
}

void findAndReplace(std::string &source, const std::string &searchFor, const std::string &replaceWith)
{
    std::string::size_type index = 0;
    while ((index = source.find(searchFor, index)) != std::string::npos) {
        source.replace(index, searchFor.length(), replaceWith);
        index += replaceWith.length();
    }
}

std::string replaceEscapeSequences(const std::string &source) {
    std::string result;
    result.reserve(source.size());
    for (std::size_t i = 0; i < source.size(); ++i) {
        if (source[i] != '\\' || i + 1 >= source.size())
            result += source[i];
        else {
            ++i;
            if (source[i] == 'n') {
                result += '\n';
            } else if (source[i] == 'r') {
                result += '\r';
            } else if (source[i] == 't') {
                result += '\t';
            } else if (source[i] == 'x') {
                std::string value = "0";
                if (i + 1 < source.size() && std::isxdigit(source[i+1]))
                    value += source[i++ + 1];
                if (i + 1 < source.size() && std::isxdigit(source[i+1]))
                    value += source[i++ + 1];
                result += static_cast<char>(std::stoi(value, nullptr, 16));
            } else if (source[i] == '0') {
                std::string value = "0";
                if (i + 1 < source.size() && source[i+1] >= '0' && source[i+1] <= '7')
                    value += source[i++ + 1];
                if (i + 1 < source.size() && source[i+1] >= '0' && source[i+1] <= '7')
                    value += source[i++ + 1];
                result += static_cast<char>(std::stoi(value, nullptr, 8));
            } else {
                result += source[i];
            }
        }
    }
    return result;
}


std::list<std::string> splitString(const std::string& str, char sep)
{
    if (std::strchr(str.c_str(), sep) == nullptr)
        return {str};

    std::list<std::string> l;
    std::string p(str);
    for (;;) {
        const std::string::size_type pos = p.find(sep);
        if (pos == std::string::npos)
            break;
        l.push_back(p.substr(0,pos));
        p = p.substr(pos+1);
    }
    l.push_back(std::move(p));
    return l;
}

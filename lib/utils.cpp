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

#include "utils.h"

#include <algorithm>
#include <cctype>
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
    for (std::string::const_iterator i = pattern.cbegin(); i != pattern.cend(); ++i) {
        if (*i == '*' || *i == '?') {
            const std::string::const_iterator j = i + 1;
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

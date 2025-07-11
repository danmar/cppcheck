/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "pathmatch.h"

#include "path.h"

#include <algorithm>
#include <stack>
#include <string>
#include <vector>


PathMatch::PathMatch(std::vector<std::string> patterns, std::string basepath, Mode mode) :
    mPatterns(std::move(patterns)), mBasepath(std::move(basepath)), mMode(mode)
{}

bool PathMatch::match(const std::string &path) const
{
    return std::any_of(mPatterns.cbegin(), mPatterns.cend(), [=] (const std::string &pattern) {
        return match(pattern, path, mBasepath, mMode);
    });
}

bool PathMatch::match(const std::string &pattern, const std::string &path, const std::string &basepath, Mode mode)
{
    if (pattern.empty())
        return false;

    if (pattern == "*" || pattern == "**")
        return true;

    bool real = Path::isAbsolute(pattern) || pattern[0] == '.';

    PathIterator s = PathIterator::from_pattern(pattern, basepath, mode == Mode::icase);
    PathIterator t = PathIterator::from_path(path, basepath, mode == Mode::icase);
    PathIterator p = s;
    PathIterator q = t;

    std::stack<std::pair<PathIterator::Pos, PathIterator::Pos>> b;

    for (;;) {
        switch (*s) {
        case '*': {
            bool slash = false;
            ++s;
            if (*s == '*') {
                slash = true;
                ++s;
            }
            b.emplace(s.getpos(), t.getpos());
            while (*t != '\0' && (slash || *t != '/')) {
                if (*s == *t)
                    b.emplace(s.getpos(), t.getpos());
                ++t;
            }
            continue;
        }
        case '?': {
            if (*t != '\0' && *t != '/') {
                ++s;
                ++t;
                continue;
            }
            break;
        }
        case '\0': {
            if (*t == '\0' || (*t == '/' && !real))
                return true;
            break;
        }
        default: {
            if (*s == *t) {
                ++s;
                ++t;
                continue;
            }
            break;
        }
        }

        if (!b.empty()) {
            const auto &bp = b.top();
            b.pop();
            s.setpos(bp.first);
            t.setpos(bp.second);
            continue;
        }

        while (*q != '\0' && *q != '/')
            ++q;

        if (*q == '/') {
            ++q;
            s = p;
            t = q;
            continue;
        }

        return false;
    }
}

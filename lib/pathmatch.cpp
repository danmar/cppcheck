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
#include <cctype>
#include <cstdio>
#include <stack>
#include <string>
#include <vector>

struct Pathstr
{
    static Pathstr from_pattern(const std::string &pattern, const std::string &basepath, bool icase)
    {
        if (!pattern.empty() && pattern[0] == '.')
            return Pathstr(basepath.c_str(), pattern.c_str(), icase);
        return Pathstr(pattern.c_str(), nullptr, icase);
    }

    static Pathstr from_path(const std::string &path, const std::string &basepath, bool icase)
    {
        if (Path::isAbsolute(path))
            return Pathstr(path.c_str(), nullptr, icase);
        return Pathstr(basepath.c_str(), path.c_str(), icase);
    }

    explicit Pathstr(const char *a = nullptr, const char *b = nullptr, bool lowercase = false) :
        s{a, b}, lcase(lowercase)
    {
        for (int i = 0; i < 2; i++) {
            e[i] = s[i];

            if (s[i] == nullptr || *s[i] == '\0')
                continue;

            if (st.l != 0)
                st.l++;

            while (*e[i] != '\0') {
                e[i]++;
                st.l++;
            }

            st.p = e[i];
        }

        if (st.l == 0)
            st.c = '\0';

        simplify(false);
    }

    std::size_t left() const
    {
        return st.l;
    }

    char current() const
    {
        if (st.c != EOF)
            return st.c;

        char c = st.p[-1];

        if (c == '\\')
            return '/';

        if (lcase)
            return std::tolower(c);

        return c;
    }

    void simplify(bool leadsep)
    {
        while (left() != 0) {
            State rst = st;

            if (leadsep) {
                if (current() != '/')
                    break;
                nextc();
            }

            char c = current();
            if (c == '.') {
                nextc();
                c = current();
                if (c == '.') {
                    nextc();
                    c = current();
                    if (c == '/') {
                        /* Skip '<name>/../' */
                        nextc();
                        simplify(false);
                        while (left() != 0 && current() != '/')
                            nextc();
                        continue;
                    }
                } else if (c == '/') {
                    /* Skip '/./' */
                    continue;
                } else if (c == '\0') {
                    /* Skip leading './' */
                    break;
                }
            } else if (c == '/' && left() != 1) {
                /* Skip double separator (keep root) */
                nextc();
                leadsep = false;
                continue;
            }

            st = rst;
            break;
        }
    }

    void advance()
    {
        nextc();

        if (current() == '/')
            simplify(true);
    }

    void nextc()
    {
        if (st.l == 0)
            return;

        st.l--;

        if (st.l == 0)
            st.c = '\0';
        else if (st.c != EOF) {
            st.c = EOF;
        } else {
            st.p--;
            if (st.p == s[1]) {
                st.p = e[0];
                st.c = '/';
            }
        }
    }

    void operator++()
    {
        advance();
    }

    char operator*() const
    {
        return current();
    }

    struct State
    {
        const char *p;
        std::size_t l;
        int c {EOF};
    };

    const char *s[2] {};
    const char *e[2] {};
    State st {};
    bool lcase;
};


static bool match_one(const std::string &pattern, const std::string &path, const std::string &basepath, bool icase)
{
    if (pattern.empty())
        return false;

    if (pattern == "*" || pattern == "**")
        return true;

    bool real = Path::isAbsolute(pattern) || pattern[0] == '.';

    Pathstr s = Pathstr::from_pattern(pattern, basepath, icase);
    Pathstr t = Pathstr::from_path(path, basepath, icase);

    Pathstr p = s;
    Pathstr q = t;

    std::stack<std::pair<Pathstr::State, Pathstr::State>> b;

    for (;;) {
        switch (*s) {
        case '*': {
            bool slash = false;
            ++s;
            if (*s == '*') {
                slash = true;
                ++s;
            }
            b.emplace(s.st, t.st);
            while (*t != '\0' && (slash || *t != '/')) {
                if (*s == *t)
                    b.emplace(s.st, t.st);
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
            s.st = bp.first;
            t.st = bp.second;
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


PathMatch::PathMatch(std::vector<std::string> patterns, std::string basepath, Mode mode) :
    mPatterns(std::move(patterns)), mBasepath(std::move(basepath)), mMode(mode)
{}

bool PathMatch::match(const std::string &path) const
{
    return std::any_of(mPatterns.cbegin(), mPatterns.cend(), [=] (const std::string &pattern) {
        return match_one(pattern, path, mBasepath, mMode == Mode::icase);
    });
}

bool PathMatch::match(const std::string &pattern, const std::string &path, const std::string &basepath, Mode mode)
{
    return match_one(pattern, path, basepath, mode == Mode::icase);
}

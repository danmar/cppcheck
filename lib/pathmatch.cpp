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

#include <cstddef>
#include <cstring>
#include <string>
#include <regex>

/* Escape regex special chars and translate globs to equivalent regex */
static std::string translate(const std::string &s)
{
    std::string r;
    std::size_t i = 0;

    while (i != s.size()) {
        int c = s[i++];

        if (std::strchr("\\[](){}+^$|", c) != nullptr) {
            r.push_back('\\');
            r.push_back(c);
        } else if (c == '*') {
            if (i != s.size() && s[i] == '*') {
                r.append(".*");
                i++;
            }
            else {
                r.append("[^/]*");
            }
        } else if (c == '?') {
            r.append("[^/]");
        } else {
            r.push_back(c);
        }
    }

    return r;
}

PathMatch::PathMatch(const std::vector<std::string> &paths, const std::string &basepath, Mode mode)
{
    if (mode == Mode::platform) {
#ifdef _WIN32
        mode = Mode::icase;
#else
        mode = Mode::scase;
#endif
    }

    std::string regex_string;

    for (auto p : paths) {
        if (p.empty())
            continue;

        if (!regex_string.empty())
            regex_string.push_back('|');

        if (p.front() == '.') {
            if (Path::isAbsolute(basepath))
                p = basepath + "/" + p;
            else
                p = Path::getCurrentPath() + "/" + basepath + "/" + p;
        }

        p = Path::fromNativeSeparators(p);

        if (Path::isAbsolute(p)) {
            p = Path::simplifyPath(p);

            if (p.back() == '/')
                regex_string.append("^" + translate(p));
            else
                regex_string.append("^" + translate(p) + "$");
        } else {
            if (p.back() == '/')
                regex_string.append("/" + translate(p));
            else
                regex_string.append("/" + translate(p) + "$");
        }
    }

    if (regex_string.empty())
        regex_string = "^$";

    if (mode == Mode::icase)
        mRegex = std::regex(regex_string, std::regex_constants::extended | std::regex_constants::icase);
    else
        mRegex = std::regex(regex_string, std::regex_constants::extended);
}

bool PathMatch::match(const std::string &path, const std::string &basepath) const
{
    std::string p;
    std::smatch m;

    if (Path::isAbsolute(path))
        p = Path::fromNativeSeparators(Path::simplifyPath(path));
    else if (Path::isAbsolute(basepath))
        p = Path::fromNativeSeparators(Path::simplifyPath(basepath + "/" + path));
    else
        p = Path::fromNativeSeparators(Path::simplifyPath(Path::getCurrentPath() + "/" + basepath + "/" + path));

    return std::regex_search(p, m, mRegex, std::regex_constants::match_any | std::regex_constants::match_not_null);
}

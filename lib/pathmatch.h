/* -*- C++ -*-
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

#ifndef PATHMATCH_H
#define PATHMATCH_H

#include "config.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "path.h"

/// @addtogroup CLI
/// @{

/**
 *  Path matching rules:
 *  - All patterns are simplified first (path separators vary by platform):
 *    - '/./' => '/'
 *    - '/dir/../' => '/'
 *    - '//' => '/'
 *    - Trailing slashes are removed
 *  - Patterns can contain globs:
 *    - '**' matches any number of characters including path separators.
 *    - '*' matches any number of characters except path separators.
 *    - '?' matches any single character except path separators.
 *  - If a pattern looks like an absolute path (e.g. starts with '/', but varies by platform):
 *    - Match all files where the pattern matches the start of the file's simplified absolute path up until a path
 *      separator or the end of the pathname.
 *  - If a pattern starts with '.':
 *    - The pattern is interpreted as a path relative to `basepath` and then converted to an absolute path and
 *      treated as such according to the above procedure. If the pattern is relative to some other directory, it should
 *      be modified to be relative to `basepath` first (this should be done with patterns in project files, for example).
 *  - Otherwise:
 *    - Match all files where the pattern matches any part of the file's simplified absolute path up until a
 *      path separator or the end of the pathname, and the matching part directly follows a path separator.
 **/

/**
 * @brief Simple path matching for ignoring paths in CLI.
 */
class CPPCHECKLIB PathMatch {
public:

    /**
     * @brief Match mode.
     *
     * scase: Case sensitive.
     * icase: Case insensitive.
     **/
    enum class Mode : std::uint8_t {
        scase,
        icase,
    };

    /**
     * @brief The default mode for the current platform.
     **/
#ifdef _WIN32
    static constexpr Mode platform_mode = Mode::icase;
#else
    static constexpr Mode platform_mode = Mode::scase;
#endif

    /**
     * The constructor.
     *
     * @param patterns List of patterns.
     * @param basepath Path to which patterns and matched paths are relative, when applicable.
     * @param mode Case sensitivity mode.
     */
    explicit PathMatch(std::vector<std::string> patterns = {}, std::string basepath = std::string(), Mode mode = platform_mode);

    /**
     * @brief Match path against list of patterns.
     *
     * @param path Path to match.
     * @return true if any of the masks match the path, false otherwise.
     */
    bool match(const std::string &path) const;

    /**
     * @brief Match path against a single pattern.
     *
     * @param pattern Pattern to use.
     * @param path Path to match.
     * @param basepath Path to which the pattern and path is relative, when applicable.
     * @param mode Case sensitivity mode.
     * @return true if the pattern matches the path, false otherwise.
     */
    static bool match(const std::string &pattern, const std::string &path, const std::string &basepath = std::string(), Mode mode = platform_mode);

private:
    friend class TestPathMatch;
    class PathIterator;

    std::vector<std::string> mPatterns;
    std::string mBasepath;
    Mode mMode;
};

/**
 * A more correct and less convenient name for this class would be PathStringsCanonicalReverseIterator.
 *
 * This class takes two path strings and iterates their concatenation in reverse while doing canonicalization,
 * i.e. collapsing double-dots, removing extra slashes, dot-slashes, and trailing slashes, as well as converting
 * native slashes to forward slashes and optionally converting characters to lowercase.
 *
 * Both strings are optional. If both strings are present, then they're concatenated with a slash
 * (subject to canonicalization).
 *
 * Double-dots at the root level are removed. The root slash is preserved, other trailing slashes are removed.
 *
 * Doing the iteration in reverse allows canonicalization to be performed without lookahead. This is useful
 * for comparing path strings, potentially relative to different base paths, without having to do prior string
 * processing or extra allocations.
 *
 * The length of the output is at most strlen(a) + strlen(b) + 1.
 *
 * Example:
 *   - input:  "/hello/universe/.", "../world//"
 *   - output: "dlrow/olleh/"
 **/
class PathMatch::PathIterator {
public:
    /* Create from a pattern and base path, patterns must begin with '.' to be considered relative */
    static PathIterator from_pattern(const std::string &pattern, const std::string &basepath, bool icase)
    {
        if (!pattern.empty() && pattern[0] == '.')
            return PathIterator(basepath.c_str(), pattern.c_str(), icase);
        return PathIterator(pattern.c_str(), nullptr, icase);
    }

    /* Create from path and base path */
    static PathIterator from_path(const std::string &path, const std::string &basepath, bool icase)
    {
        if (Path::isAbsolute(path))
            return PathIterator(path.c_str(), nullptr, icase);
        return PathIterator(basepath.c_str(), path.c_str(), icase);
    }

    /* Constructor */
    explicit PathIterator(const char *path_a = nullptr, const char *path_b = nullptr, bool lower = false) :
        mStart{path_a, path_b}, mLower(lower)
    {
        for (int i = 0; i < 2; i++) {
            mEnd[i] = mStart[i];

            if (mStart[i] == nullptr || *mStart[i] == '\0')
                continue;

            if (mPos.l != 0)
                mPos.l++;

            while (*mEnd[i] != '\0') {
                mEnd[i]++;
                mPos.l++;
            }

            mPos.p = mEnd[i];
        }

        if (mPos.l == 0)
            mPos.c = '\0';

        skips(false);
    }

    /* Position struct */
    struct Pos {
        /* String pointer */
        const char *p;
        /* Raw characters left */
        std::size_t l;
        /* Buffered character */
        int c {EOF};
    };

    /* Save the current position */
    const Pos &getpos() const
    {
        return mPos;
    }

    /* Restore a saved position */
    void setpos(const Pos &pos)
    {
        mPos = pos;
    }

    /* Read the current character */
    char operator*() const
    {
        return current();
    }

    /* Go to the next character */
    void operator++()
    {
        advance();
    }

    /* Consume remaining characters into an std::string and reverse, use for testing */
    std::string read()
    {
        std::string str;

        while (current() != '\0') {
            str.insert(0, 1, current());
            advance();
        }

        return str;
    }

private:
    /* Read the current character */
    char current() const
    {
        if (mPos.c != EOF)
            return mPos.c;

        char c = mPos.p[-1];

        if (c == '\\')
            return '/';

        if (mLower)
            return std::tolower(c);

        return c;
    }

    /* Do canonicalization on a path component boundary */
    void skips(bool leadsep)
    {
        while (mPos.l != 0) {
            Pos pos = mPos;

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
                        skips(false);
                        while (mPos.l != 0 && current() != '/')
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
            } else if (c == '/' && mPos.l != 1) {
                /* Skip double separator (keep root) */
                nextc();
                leadsep = false;
                continue;
            }

            mPos = pos;
            break;
        }
    }

    /* Go to the next character, doing skips on path separators */
    void advance()
    {
        nextc();

        if (current() == '/')
            skips(true);
    }

    /* Go to the next character */
    void nextc()
    {
        if (mPos.l == 0)
            return;

        mPos.l--;

        if (mPos.l == 0)
            mPos.c = '\0';
        else if (mPos.c != EOF) {
            mPos.c = EOF;
        } else {
            mPos.p--;
            if (mPos.p == mStart[1]) {
                mPos.p = mEnd[0];
                mPos.c = '/';
            }
        }
    }

    /* String start pointers */
    const char *mStart[2] {};
    /* String end pointers */
    const char *mEnd[2] {};
    /* Current position */
    Pos mPos {};
    /* Lowercase conversion flag */
    bool mLower;
};

/// @}

#endif // PATHMATCH_H

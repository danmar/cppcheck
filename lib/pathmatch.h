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

#include <cstdint>
#include <string>
#include <vector>

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
     * @brief Case sensitivity mode.
     *
     * platform: Use the platform default.
     * scase: Case sensitive.
     * icase: Case insensitive.
     **/
    enum class Mode : std::uint8_t {
        scase,
        icase,
    };

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
    explicit PathMatch(std::vector<std::string> patterns, std::string basepath = std::string(), Mode mode = platform_mode);

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
     */
    static bool match(const std::string &pattern, const std::string &path, const std::string &basepath = std::string(), Mode mode = platform_mode);

private:
    std::vector<std::string> mPatterns;
    std::string mBasepath;
    Mode mMode;
};

/// @}

#endif // PATHMATCH_H

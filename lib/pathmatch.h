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

#include <regex>
#include <string>
#include <vector>

/// @addtogroup CLI
/// @{

/**
 *  Path matching rules:
 *  - If a rule looks like an absolute path (e.g. starts with '/', but varies by platform):
 *    - The rule will be simplified (path separators vary by platform):
 *      - '/./' => '/'
 *      - '/dir/../' => '/'
 *      - '//' => '/'
 *    - If the rule ends with a path separator, match all files where the rule matches the start of the file's
 *      simplified absolute path. Globs are allowed in the rule.
 *    - Otherwise, match all files where the rule matches the file's simplified absolute path.
 *      Globs are allowed in the rule.
 *  - If a rule starts with '.':
 *    - The rule is interpreted as a path relative to the execution directory (when passed to the CLI),
 *      or the directory containing the project file (when imported), and then converted to an absolute path and
 *      treatesd as such according to the above procedure.
 *  - Otherwise:
 *    - No simplification is done to the rule.
 *    - If the rule ends with a path separator:
 *      - Match all files where the rule matches any part of the file's simplified absolute path, and the matching
 *        part directly follows a path separator. Globs are allowed in the rule.
 *    - Otherwise:
 *      - Match all files where the rules matches the end of the file's simplified absolute path, and the matching
 *        part directly follows a path separator. Globs are allowed in the rule.
 *
 * - Glob rules:
 *   - '**' matches any number of characters including path separators.
 *   - '*' matches any number of characters except path separators.
 *   - '?' matches any single character except path separators.
 **/

/**
 * @brief Simple path matching for ignoring paths in CLI.
 */
class CPPCHECKLIB PathMatch {
public:

    /**
     * The constructor.
     *
     * If a path is a directory it needs to end with a file separator.
     *
     * @param paths List of masks.
     * @param caseSensitive Match the case of the characters when
     *   matching paths?
     */
    explicit PathMatch(const std::vector<std::string> &paths, bool caseSensitive = true);

    /**
     * @brief Match path against list of masks.
     *
     * If you want to match a directory the given path needs to end with a path separator.
     *
     * @param path Path to match.
     * @return true if any of the masks match the path, false otherwise.
     */
    bool match(const std::string &path) const;

private:
    std::regex mRegex;
};

/// @}

#endif // PATHMATCH_H

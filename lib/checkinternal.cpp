/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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

#ifndef NDEBUG

#include "checkinternal.h"
#include "symboldatabase.h"
#include <string>
#include <set>

// Register this check class (by creating a static instance of it).
// Disabled in release builds
namespace {
    CheckInternal instance;
}

void CheckInternal::checkTokenMatchPatterns()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "Token :: Match (") && !Token::simpleMatch(tok, "Token :: findmatch ("))
            continue;

        const std::string funcname = tok->strAt(2);

        // Get pattern string
        const Token *pattern_tok = tok->tokAt(4)->nextArgument();
        if (!pattern_tok || !Token::Match(pattern_tok, "%str%"))
            continue;

        const std::string pattern = pattern_tok->strValue();
        if (pattern.empty()) {
            simplePatternError(tok, pattern, funcname);
            continue;
        }

        // Check for signs of complex patterns
        if (pattern.find_first_of("[|%") != std::string::npos)
            continue;
        else if (pattern.find("!!") != std::string::npos)
            continue;

        simplePatternError(tok, pattern, funcname);
    }
}

void CheckInternal::checkTokenSimpleMatchPatterns()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "Token :: simpleMatch (") && !Token::simpleMatch(tok, "Token :: findsimplematch ("))
            continue;

        const std::string funcname = tok->strAt(2);

        // Get pattern string
        const Token *pattern_tok = tok->tokAt(4)->nextArgument();
        if (!pattern_tok || !Token::Match(pattern_tok, "%str%"))
            continue;

        const std::string pattern = pattern_tok->strValue();
        if (pattern.empty()) {
            complexPatternError(tok, pattern, funcname);
            continue;
        }

        // Check for [xyz] usage - but exclude standalone square brackets
        unsigned int char_count = 0;
        for (std::string::size_type pos = 0; pos < pattern.size(); ++pos) {
            char c = pattern[pos];

            if (c == ' ') {
                char_count = 0;
            } else if (c == ']') {
                if (char_count > 0) {
                    complexPatternError(tok, pattern, funcname);
                    continue;
                }
            } else {
                ++char_count;
            }
        }

        // Check | usage: Count characters before the symbol
        char_count = 0;
        for (std::string::size_type pos = 0; pos < pattern.size(); ++pos) {
            char c = pattern[pos];

            if (c == ' ') {
                char_count = 0;
            } else if (c == '|') {
                if (char_count > 0) {
                    complexPatternError(tok, pattern, funcname);
                    continue;
                }
            } else {
                ++char_count;
            }
        }

        // Check for real errors
        if (pattern.find_first_of("%") != std::string::npos || pattern.find("!!") != std::string::npos)
            complexPatternError(tok, pattern, funcname);
    }
}

void CheckInternal::checkMissingPercentCharacter()
{
    static std::set<std::string> magics;
    if (magics.empty()) {
        magics.insert("%any%");
        magics.insert("%var%");
        magics.insert("%type%");
        magics.insert("%num%");
        magics.insert("%bool%");
        magics.insert("%str%");
        magics.insert("%varid%");
        magics.insert("%or%");
        magics.insert("%oror%");
    }

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "Token :: Match (") && !Token::simpleMatch(tok, "Token :: findmatch ("))
            continue;

        const std::string funcname = tok->strAt(2);

        // Get pattern string
        const Token *pattern_tok = tok->tokAt(4)->nextArgument();
        if (!pattern_tok || !Token::Match(pattern_tok, "%str%"))
            continue;

        const std::string pattern = pattern_tok->strValue();

        std::set<std::string>::const_iterator magic, magics_end = magics.end();
        for (magic = magics.begin(); magic != magics_end; ++magic) {
            const std::string broken_magic = (*magic).substr(0, (*magic).size()-1);

            std::string::size_type pos = 0;
            while ((pos = pattern.find(broken_magic, pos)) != std::string::npos) {
                // Check if it's the full pattern
                if (pattern.find(*magic, pos) != pos) {
                    // Known whitelist of substrings
                    if ((broken_magic == "%var" && pattern.find("%varid%", pos) == pos) ||
                        (broken_magic == "%or" && pattern.find("%oror%", pos) == pos)) {
                        ++pos;
                        continue;
                    }

                    missingPercentCharacterError(tok, pattern, funcname);
                }

                ++pos;
            }
        }
    }
}

void CheckInternal::simplePatternError(const Token* tok, const std::string& pattern, const std::string &funcname)
{
    reportError(tok, Severity::warning, "simplePatternError",
                "Found simple pattern inside Token::" + funcname + "() call: \"" + pattern + "\""
               );
}

void CheckInternal::complexPatternError(const Token* tok, const std::string& pattern, const std::string &funcname)
{
    reportError(tok, Severity::error, "complexPatternError",
                "Found complex pattern inside Token::" + funcname + "() call: \"" + pattern + "\""
               );
}

void CheckInternal::missingPercentCharacterError(const Token* tok, const std::string& pattern, const std::string& funcname)
{
    reportError(tok, Severity::error, "missingPercentCharacter",
                "Missing percent end character in Token::" + funcname + "() pattern: \"" + pattern + "\""
               );
}

#endif // #ifndef NDEBUG

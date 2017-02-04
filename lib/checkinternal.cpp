/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

#ifdef CHECK_INTERNAL

#include "checkinternal.h"
#include "symboldatabase.h"
#include "utils.h"
#include <string>
#include <set>
#include <cstring>

// Register this check class (by creating a static instance of it).
// Disabled in release builds
namespace {
    CheckInternal instance;
}

void CheckInternal::checkTokenMatchPatterns()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "Token :: Match (") && !Token::simpleMatch(tok, "Token :: findmatch ("))
                continue;

            const std::string& funcname = tok->strAt(2);

            // Get pattern string
            const Token *patternTok = tok->tokAt(4)->nextArgument();
            if (!patternTok || patternTok->tokType() != Token::eString)
                continue;

            const std::string pattern = patternTok->strValue();
            if (pattern.empty()) {
                simplePatternError(tok, pattern, funcname);
                continue;
            }

            if (pattern.find("||") != std::string::npos || pattern.find(" | ") != std::string::npos || pattern[0] == '|' || (pattern[pattern.length() - 1] == '|' && pattern[pattern.length() - 2] == ' '))
                orInComplexPattern(tok, pattern, funcname);

            // Check for signs of complex patterns
            if (pattern.find_first_of("[|") != std::string::npos)
                continue;
            else if (pattern.find("!!") != std::string::npos)
                continue;

            bool complex = false;
            size_t index = pattern.find('%');
            while (index != std::string::npos) {
                if (pattern.length() <= index + 2) {
                    complex = true;
                    break;
                }
                if (pattern[index + 1] == 'o' && pattern[index + 2] == 'r') // %or% or %oror%
                    index = pattern.find('%', index + 1);
                else {
                    complex = true;
                    break;
                }
                index = pattern.find('%', index + 1);
            }
            if (!complex)
                simplePatternError(tok, pattern, funcname);
        }
    }
}

void CheckInternal::checkTokenSimpleMatchPatterns()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "Token :: simpleMatch (") && !Token::simpleMatch(tok, "Token :: findsimplematch ("))
                continue;

            const std::string& funcname = tok->strAt(2);

            // Get pattern string
            const Token *patternTok = tok->tokAt(4)->nextArgument();
            if (!patternTok || patternTok->tokType() != Token::eString)
                continue;

            const std::string pattern = patternTok->strValue();
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
                const char c = pattern[pos];

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
            if (pattern.length() > 1) {
                for (size_t j = 0; j < pattern.length() - 1; j++) {
                    if (pattern[j] == '%' && pattern[j + 1] != ' ')
                        complexPatternError(tok, pattern, funcname);
                    else if (pattern[j] == '!' && pattern[j + 1] == '!')
                        complexPatternError(tok, pattern, funcname);
                }
            }
        }
    }
}

namespace {
    const std::set<std::string> knownPatterns = make_container< std::set<std::string> > ()
            << "%any%"
            << "%assign%"
            << "%bool%"
            << "%char%"
            << "%comp%"
            << "%num%"
            << "%op%"
            << "%cop%"
            << "%or%"
            << "%oror%"
            << "%str%"
            << "%type%"
            << "%name%"
            << "%var%"
            << "%varid%";
}

void CheckInternal::checkMissingPercentCharacter()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "Token :: Match (") && !Token::simpleMatch(tok, "Token :: findmatch ("))
                continue;

            const std::string& funcname = tok->strAt(2);

            // Get pattern string
            const Token *patternTok = tok->tokAt(4)->nextArgument();
            if (!patternTok || patternTok->tokType() != Token::eString)
                continue;

            const std::string pattern = patternTok->strValue();

            std::set<std::string>::const_iterator knownPattern, knownPatternsEnd = knownPatterns.end();
            for (knownPattern = knownPatterns.begin(); knownPattern != knownPatternsEnd; ++knownPattern) {
                const std::string brokenPattern = (*knownPattern).substr(0, (*knownPattern).size() - 1);

                std::string::size_type pos = 0;
                while ((pos = pattern.find(brokenPattern, pos)) != std::string::npos) {
                    // Check if it's the full pattern
                    if (pattern.find(*knownPattern, pos) != pos) {
                        // Known whitelist of substrings
                        if ((brokenPattern == "%var" && pattern.find("%varid%", pos) == pos) ||
                            (brokenPattern == "%or" && pattern.find("%oror%", pos) == pos)) {
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
}

void CheckInternal::checkUnknownPattern()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "Token :: Match (") && !Token::simpleMatch(tok, "Token :: findmatch ("))
                continue;

            // Get pattern string
            const Token *patternTok = tok->tokAt(4)->nextArgument();
            if (!patternTok || patternTok->tokType() != Token::eString)
                continue;

            const std::string pattern = patternTok->strValue();
            bool inBrackets = false;

            for (std::string::size_type j = 0; j < pattern.length() - 1; j++) {
                if (pattern[j] == '[' && (j == 0 || pattern[j - 1] == ' '))
                    inBrackets = true;
                else if (pattern[j] == ']')
                    inBrackets = false;
                else if (pattern[j] == '%' && pattern[j + 1] != ' ' && pattern[j + 1] != '|' && !inBrackets) {
                    const std::string::size_type end = pattern.find('%', j + 1);
                    if (end != std::string::npos) {
                        const std::string s = pattern.substr(j, end - j + 1);
                        if (knownPatterns.find(s) == knownPatterns.end())
                            unknownPatternError(tok, s);
                    }
                }
            }
        }
    }
}

void CheckInternal::checkRedundantNextPrevious()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() != ".")
                continue;
            tok = tok->next();

            if (Token::Match(tok, "previous ( ) . next|tokAt|strAt|linkAt (") || Token::Match(tok, "next ( ) . previous|tokAt|strAt|linkAt (") ||
                (Token::simpleMatch(tok, "tokAt (") && Token::Match(tok->linkAt(1), ") . previous|next|tokAt|strAt|linkAt|str|link ("))) {
                const std::string& func1 = tok->str();
                const std::string& func2 = tok->linkAt(1)->strAt(2);

                if ((func2 == "previous" || func2 == "next" || func2 == "str" || func2 == "link") && tok->linkAt(1)->strAt(4) != ")")
                    continue;

                redundantNextPreviousError(tok, func1, func2);
            } else if (Token::Match(tok, "next|previous ( ) . next|previous ( ) . next|previous|linkAt|strAt|link|str (")) {
                const std::string& func1 = tok->str();
                const std::string& func2 = tok->strAt(8);

                if ((func2 == "previous" || func2 == "next" || func2 == "str" || func2 == "link") && tok->strAt(10) != ")")
                    continue;

                redundantNextPreviousError(tok, func1, func2);
            }
        }
    }
}

void CheckInternal::checkExtraWhitespace()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (std::size_t i = 0; i < symbolDatabase->functionScopes.size(); ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "Token :: simpleMatch|findsimplematch|Match|findmatch ("))
                continue;

            const std::string& funcname = tok->strAt(2);

            // Get pattern string
            const Token *patternTok = tok->tokAt(4)->nextArgument();
            if (!patternTok || patternTok->tokType() != Token::eString)
                continue;

            const std::string pattern = patternTok->strValue();
            if (!pattern.empty() && (pattern[0] == ' ' || *pattern.rbegin() == ' '))
                extraWhitespaceError(tok, pattern, funcname);

            // two whitespaces or more
            if (pattern.find("  ") != std::string::npos)
                extraWhitespaceError(tok, pattern, funcname);
        }
    }
}

void CheckInternal::multiComparePatternError(const Token* tok, const std::string& pattern, const std::string &funcname)
{
    reportError(tok, Severity::error, "multiComparePatternError",
                "Bad multicompare pattern (a %cmd% must be first unless it is %or%,%op%,%cop%,%name%,%oror%) inside Token::" + funcname + "() call: \"" + pattern + "\""
               );
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

void CheckInternal::unknownPatternError(const Token* tok, const std::string& pattern)
{
    reportError(tok, Severity::error, "unknownPattern",
                "Unknown pattern used: \"" + pattern + "\"");
}

void CheckInternal::redundantNextPreviousError(const Token* tok, const std::string& func1, const std::string& func2)
{
    reportError(tok, Severity::style, "redundantNextPrevious",
                "Call to 'Token::" + func1 + "()' followed by 'Token::" + func2 + "()' can be simplified.");
}

void CheckInternal::orInComplexPattern(const Token* tok, const std::string& pattern, const std::string &funcname)
{
    reportError(tok, Severity::error, "orInComplexPattern",
                "Token::" + funcname + "() pattern \"" + pattern + "\" contains \"||\" or \"|\". Replace it by \"%oror%\" or \"%or%\".");
}

void CheckInternal::extraWhitespaceError(const Token* tok, const std::string& pattern, const std::string &funcname)
{
    reportError(tok, Severity::warning, "extraWhitespaceError",
                "Found extra whitespace inside Token::" + funcname + "() call: \"" + pattern + "\""
               );
}

#endif // #ifdef CHECK_INTERNAL

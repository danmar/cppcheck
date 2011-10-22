/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

//---------------------------------------------------------------------------
// Find non reentrant functions
//---------------------------------------------------------------------------

#include "checknonreentrantfunctions.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckNonReentrantFunctions instance;
}

void CheckNonReentrantFunctions::nonReentrantFunctions()
{
    if (!_settings->standards.posix || !_settings->isEnabled("portability"))
        return;

    // Don't check C# and Java code
    if (_tokenizer->isJavaOrCSharp())
        return;

    std::map<std::string,std::string>::const_iterator nonReentrant_end = _nonReentrantFunctions.end();
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Look for function invocations
        if (!tok->isName() || tok->strAt(1) != "(" || tok->varId() != 0)
            continue;

        // Check for non-reentrant function name
        std::map<std::string,std::string>::const_iterator it = _nonReentrantFunctions.find(tok->str());
        if (it == nonReentrant_end)
            continue;

        const Token *prev = tok->previous();
        if (prev) {
            // Ignore function definitions, class members or class definitions
            if (prev->isName() || Token::Match(prev, ".|:"))
                continue;

            // Check for "std" or global namespace, ignore other namespaces
            if (prev->str() == "::" && prev->previous() && prev->previous()->str() != "std" && prev->previous()->isName())
                continue;
        }

        // Only affecting multi threaded code, therefore this is "portability"
        reportError(tok, Severity::portability, "nonreentrantFunctions"+it->first, it->second);
    }
}
//---------------------------------------------------------------------------


/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
// Dangerous functions
//---------------------------------------------------------------------------

#include "checkdangerousfunctions.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace
{
CheckDangerousFunctions instance;
}

void CheckDangerousFunctions::dangerousFunctions()
{
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "mktemp ("))
        {
            dangerousFunctionmktemp(tok);
        }
        else if (Token::simpleMatch(tok, "gets ("))
        {
            dangerousFunctiongets(tok);
        }
        else if (Token::simpleMatch(tok, "scanf ("))
        {
            dangerousFunctionscanf(tok);
        }
    }
}
//---------------------------------------------------------------------------


void CheckDangerousFunctions::dangerousFunctionmktemp(const Token *tok)
{
    reportError(tok, Severity::style, "dangerousFunctionmktemp", "Found 'mktemp'. You should use 'mkstemp' instead");
}

void CheckDangerousFunctions::dangerousFunctiongets(const Token *tok)
{
    reportError(tok, Severity::style, "dangerousFunctiongets", "Found 'gets'. You should use 'fgets' instead");
}

void CheckDangerousFunctions::dangerousFunctionscanf(const Token *tok)
{
    reportError(tok, Severity::style, "dangerousFunctionscanf", "Found 'scanf'. You should use 'fgets' instead");
}

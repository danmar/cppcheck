/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
#include "checkexceptionsafety.h"

#include "tokenize.h"
#include "token.h"
#include "errorlogger.h"

//---------------------------------------------------------------------------

// Register CheckExceptionSafety..
namespace
{
CheckExceptionSafety instance;
}


//---------------------------------------------------------------------------

void CheckExceptionSafety::destructors()
{
    // This is a style error..
    if (!_settings->_checkCodingStyle)
        return;

    // Perform check..
    for (const Token * tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, ") {"))
            tok = tok->next()->link();

        if (!Token::Match(tok, "~ %var% ( ) {"))
            continue;

        // Inspect this destructor..
        unsigned int indentlevel = 0;
        for (const Token *tok2 = tok->tokAt(5); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
            {
                ++indentlevel;
            }

            else if (tok2->str() == "}")
            {
                if (indentlevel <= 1)
                    break;
                --indentlevel;
            }

            else if (tok2->str() == "throw")
            {
                destructorsError(tok2);
                break;
            }
        }
    }
}


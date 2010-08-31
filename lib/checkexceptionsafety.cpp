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
#include "checkexceptionsafety.h"

#include "token.h"

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




void CheckExceptionSafety::deallocThrow()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() != "delete")
            continue;

        // Check if this is something similar with: "delete p;"
        tok = tok->next();
        if (Token::simpleMatch(tok, "[ ]"))
            tok = tok->tokAt(2);
        if (!tok)
            break;
        if (!Token::Match(tok, "%var% ;"))
            continue;
        const unsigned int varid(tok->varId());
        if (varid == 0)
            continue;

        // is this variable a global variable?
        {
            bool globalVar = false;
            for (const Token *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next())
            {
                if (tok->varId() == varid)
                {
                    globalVar = true;
                    break;
                }

                if (tok2->str() == "class")
                {
                    while (tok2 && tok2->str() != ";" && tok2->str() != "{")
                        tok2 = tok2->next();
                    tok2 = tok2 ? tok2->next() : 0;
                    if (!tok2)
                        break;
                }

                if (tok2->str() == "{")
                {
                    tok2 = tok2->link();
                    if (!tok2)
                        break;
                }
            }
            if (!globalVar)
                continue;
        }

        // is there a throw after the deallocation?
        unsigned int indentlevel = 0;
        const Token *ThrowToken = 0;
        for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
                ++indentlevel;
            else if (tok2->str() == "}")
            {
                if (indentlevel == 0)
                    break;
                --indentlevel;
            }

            if (tok2->str() == "throw")
                ThrowToken = tok2;

            else if (Token::Match(tok2, "%varid% =", varid))
            {
                if (ThrowToken)
                    deallocThrowError(ThrowToken, tok->str());
                break;
            }
        }
    }
}



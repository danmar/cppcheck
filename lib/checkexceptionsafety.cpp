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
#include "checkexceptionsafety.h"

#include "token.h"

//---------------------------------------------------------------------------

// Register CheckExceptionSafety..
namespace {
    CheckExceptionSafety instance;
}


//---------------------------------------------------------------------------

void CheckExceptionSafety::destructors()
{
    // This is a style error..
    if (!_settings->isEnabled("style"))
        return;

    // Perform check..
    for (const Token * tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // Skip executable scopes
        if (Token::simpleMatch(tok, ") {"))
            tok = tok->next()->link();

        // only looking for destructors
        if (!Token::Match(tok, "~ %var% ( ) {"))
            continue;

        // Inspect this destructor..
        unsigned int indentlevel = 0;
        for (const Token *tok2 = tok->tokAt(5); tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{") {
                ++indentlevel;
            }

            else if (tok2->str() == "}") {
                if (indentlevel <= 1)
                    break;
                --indentlevel;
            }

            // throw found within a destructor
            else if (tok2->str() == "throw") {
                destructorsError(tok2);
                break;
            }
        }
    }
}




void CheckExceptionSafety::deallocThrow()
{
    // Deallocate a global/member pointer and then throw exception
    // the pointer will be a dead pointer
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        // only looking for delete now
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
            // TODO: Isn't it better to use symbol database instead?
            bool globalVar = false;
            for (const Token *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next()) {
                if (tok2->varId() == varid) {
                    globalVar = true;
                    break;
                }

                if (tok2->str() == "class") {
                    while (tok2 && tok2->str() != ";" && tok2->str() != "{")
                        tok2 = tok2->next();
                    tok2 = tok2 ? tok2->next() : 0;
                    if (!tok2)
                        break;
                }

                if (tok2->str() == "{") {
                    tok2 = tok2->link();
                    if (!tok2)
                        break;
                }
            }

            // Not a global variable.. skip checking this var.
            if (!globalVar)
                continue;
        }

        // indentlevel..
        unsigned int indentlevel = 0;

        // Token where throw occurs
        const Token *ThrowToken = 0;

        // is there a throw after the deallocation?
        for (const Token *tok2 = tok; tok2; tok2 = tok2->next()) {
            if (tok2->str() == "{")
                ++indentlevel;
            else if (tok2->str() == "}") {
                if (indentlevel == 0)
                    break;
                --indentlevel;
            }

            if (tok2->str() == "throw")
                ThrowToken = tok2;

            // if the variable is not assigned after the throw then it
            // is assumed that it is not the intention that it is a dead pointer.
            else if (Token::Match(tok2, "%varid% =", varid)) {
                if (ThrowToken)
                    deallocThrowError(ThrowToken, tok->str());
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------
//      catch(const exception & err)
//      {
//         throw err;            // <- should be just "throw;"
//      }
//---------------------------------------------------------------------------
void CheckExceptionSafety::checkRethrowCopy()
{
    if (!_settings->isEnabled("style"))
        return;
    const char catchPattern[] = "catch ( const| %type% &|*| %var% ) { %any%";

    const Token *tok = Token::findmatch(_tokenizer->tokens(), catchPattern);
    while (tok) {
        const Token *startBlockTok = tok->next()->link()->next();
        const Token *endBlockTok = startBlockTok->link();
        const unsigned int varid = startBlockTok->tokAt(-2)->varId();

        const Token* rethrowTok = Token::findmatch(startBlockTok, "throw %varid%", endBlockTok, varid);
        if (rethrowTok) {
            rethrowCopyError(rethrowTok, startBlockTok->tokAt(-2)->str());
        }

        tok = Token::findmatch(endBlockTok->next(), catchPattern);
    }
}


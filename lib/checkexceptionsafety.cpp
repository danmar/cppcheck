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


static bool autodealloc(const Token * const C, const Token * const tokens)
{
    if (C->isStandardType())
        return false;
    return !Token::findmatch(tokens, ("class " + C->str() + " {").c_str());
}

void CheckExceptionSafety::unsafeNew()
{
    // Check that "--exception-safety" was given
    if (!_settings->_exceptionSafety)
        return;

    // Inspect initializer lists..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() != ")")
            continue;
        tok = tok->next();
        if (tok->str() != ":")
            continue;

        // count "new" and check that it's an initializer list..
        unsigned int countNew = 0;
        for (tok = tok->next(); tok; tok = tok->next())
        {
            if (!Token::Match(tok, "%var% ("))
                break;
            tok = tok->next();
            if (Token::Match(tok->next(), "new %type%"))
            {
                if (countNew > 0 || !autodealloc(tok->tokAt(2), _tokenizer->tokens()))
                {
                    ++countNew;
                }
            }
            tok = tok->link();
            tok = tok ? tok->next() : 0;
            if (!tok)
                break;
            if (tok->str() == "{")
            {
                if (countNew > 1)
                    unsafeNewError(tok);
                break;
            }
            else if (tok->str() != ",")
                break;
        }
        if (!tok)
            break;
    }


    // Inspect constructors..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // match this pattern.. "C :: C ( .. ) {"
        if (!tok->next() || tok->next()->str() != "::")
            continue;
        if (!Token::Match(tok, "%var% :: %var% ("))
            continue;
        if (tok->str() != tok->strAt(2))
            continue;
        if (!Token::simpleMatch(tok->tokAt(3)->link(), ") {"))
            continue;

        // inspect the constructor..
        unsigned int countNew = 0;
        for (tok = tok->tokAt(3)->link()->tokAt(2); tok; tok = tok->next())
        {
            if (tok->str() == "{" || tok->str() == "}")
                break;
            // some variable declaration..
            if (Token::Match(tok->previous(), "[{;] %type% * %var% ;"))
                break;
            // allocating with new..
            if (Token::Match(tok, "%var% = new %type%"))
            {
                if (countNew > 0 || !autodealloc(tok->tokAt(3), _tokenizer->tokens()))
                {
                    ++countNew;
                    if (countNew > 1)
                    {
                        unsafeNewError(tok);
                        break;
                    }
                }
            }
        }
    }

    // allocating multiple local variables..
    std::set<unsigned int> localVars;
    unsigned int countNew = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{" || tok->str() == "}")
        {
            localVars.clear();
            countNew = 0;
        }

        if (Token::Match(tok, "[;{}] %type% * %var% ;"))
        {
            tok = tok->tokAt(3);
            if (tok->varId())
                localVars.insert(tok->varId());
        }

        if (Token::Match(tok, "; %var% = new"))
        {
            if (tok->next()->varId() && localVars.find(tok->next()->varId()) != localVars.end())
                ++countNew;
            if (countNew >= 2)
                unsafeNewError(tok->next());
        }
    }
}

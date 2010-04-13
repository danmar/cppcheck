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


void CheckExceptionSafety::unsafeNew()
{
    if (!_settings->isEnabled("exceptNew"))
        return;

    // Inspect initializer lists..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() != ")")
            continue;
        tok = tok->next();
        if (!tok)
            break;
        if (tok->str() != ":")
            continue;

        // multiple "new" in an initializer list..
        std::string varname;
        for (tok = tok->next(); tok; tok = tok->next())
        {
            if (!Token::Match(tok, "%var% ("))
                break;
            tok = tok->next();
            if (Token::Match(tok->next(), "new %type%"))
            {
                if (!varname.empty())
                {
                    unsafeNewError(tok->previous(), varname);
                    break;
                }
                if (!_settings->isAutoDealloc(tok->strAt(2)))
                {
                    varname = tok->strAt(-1);
                }
            }
            tok = tok->link();
            tok = tok ? tok->next() : 0;
            if (!tok)
                break;
            if (tok->str() != ",")
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
        std::string varname;
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
                if (!varname.empty())
                {
                    unsafeNewError(tok, varname);
                    break;
                }
                if (!_settings->isAutoDealloc(tok->strAt(3)))
                    varname = tok->str();
            }
        }
    }

    // allocating multiple local variables..
    std::set<unsigned int> localVars;
    std::string varname;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{" || tok->str() == "}")
        {
            localVars.clear();
            varname = "";
        }

        if (Token::Match(tok, "[;{}] %type% * %var% ;") &&
            !_settings->isAutoDealloc(tok->strAt(1)))
        {
            tok = tok->tokAt(3);
            if (tok->varId())
                localVars.insert(tok->varId());
        }

        if (Token::Match(tok, "[;{}] %var% = new %type%"))
        {
            if (!varname.empty())
            {
                unsafeNewError(tok->next(), varname);
                break;
            }
            if (tok->next()->varId() && localVars.find(tok->next()->varId()) != localVars.end())
                varname = tok->strAt(1);
        }

        else if (Token::Match(tok, "[;{}] %var% ("))
        {
            for (tok = tok->next(); tok && !Token::Match(tok, "[;{}]"); tok = tok->next())
            {
                if (tok->str() == varname)
                {
                    varname = "";
                }
            }
            if (!tok)
                break;
        }

        else if (tok->str() == "delete")
        {
            if (Token::simpleMatch(tok->next(), varname.c_str()) ||
                Token::simpleMatch(tok->next(), ("[ ] " + varname).c_str()))
            {
                varname = "";
            }
        }
    }
}


void CheckExceptionSafety::realloc()
{
    if (!_settings->isEnabled("exceptRealloc"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "try {"))
        {
            tok = tok->next()->link();
            if (!tok)
                break;
        }

        if (!Token::Match(tok, "[{};] delete"))
            continue;

        tok = tok->tokAt(2);
        if (Token::simpleMatch(tok, "[ ]"))
            tok = tok->tokAt(2);
        if (!tok)
            break;

        // reallocating..
        if (!Token::Match(tok, "%var% ; %var% = new %type%"))
            continue;

        // variable id of deallocated pointer..
        const unsigned int varid(tok->varId());
        if (varid == 0)
            continue;

        // variable id of allocated pointer must match..
        if (varid != tok->tokAt(2)->varId())
            continue;

        // is is a class member variable..
        const Token *tok1 = Token::findmatch(_tokenizer->tokens(), "%varid%", varid);
        while (0 != (tok1 = tok1 ? tok1->previous() : 0))
        {
            if (tok1->str() == "}")
                tok1 = tok1->link();
            else if (tok1->str() == "{")
            {
                if (tok1->previous() && tok1->previous()->isName())
                {
                    while (0 != (tok1 = tok1 ? tok1->previous() : 0))
                    {
                        if (!tok1->isName() && tok1->str() != ":" && tok1->str() != ",")
                            break;
                        if (tok1->str() == "class")
                        {
                            reallocError(tok->tokAt(2), tok->str());
                            break;
                        }
                    }
                }
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



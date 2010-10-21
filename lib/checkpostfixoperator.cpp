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
// You should use ++ and -- as prefix whenever possible as these are more
// efficient than postfix operators
//---------------------------------------------------------------------------

#include "checkpostfixoperator.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace
{
CheckPostfixOperator instance;
}

void CheckPostfixOperator::postfixOperator()
{
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        bool result = false;
        if (Token::Match(tok, "++|--"))
        {
            if (Token::Match(tok->previous()->previous(), ";|{|}") && Token::Match(tok->next(), ";|)|,"))
            {
                result = true;
            }
            else if (tok->strAt(-2) == ",")
            {
                int i(1);
                while (tok->strAt(i) != ")")
                {
                    if (tok->strAt(i) == ";")
                    {
                        result = true;
                        break;
                    }
                    ++i;
                }
            }
            else if (tok->strAt(-2) == "<<" && tok->strAt(1) == "<<")
            {
                result = true;
            }
        }

        if (result && tok->previous()->varId())
        {
            const Token *decltok = Token::findmatch(_tokenizer->tokens(), "%varid%", tok->previous()->varId());
            if (decltok && Token::Match(decltok->previous(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator"))
            {
                // the variable is an iterator
                postfixOperatorError(tok);
            }
            else
            {
                const std::string classDef = std::string("class ") + std::string(decltok->previous()->strAt(0));
                if (Token::findmatch(_tokenizer->tokens(), classDef.c_str()))
                {
                    // the variable is an instance of class
                    postfixOperatorError(tok);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------


void CheckPostfixOperator::postfixOperatorError(const Token *tok)
{
    reportError(tok, Severity::performance, "postfixOperator", "You should use ++ and -- as prefix whenever possible as these are more efficient than postfix operators");
}

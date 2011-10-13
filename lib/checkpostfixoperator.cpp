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
// You should use ++ and -- as prefix whenever possible as these are more
// efficient than postfix operators
//---------------------------------------------------------------------------

#include "checkpostfixoperator.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckPostfixOperator instance;
}

void CheckPostfixOperator::postfixOperator()
{
    if (!_settings->isEnabled("performance"))
        return;

    const Token *tok = _tokenizer->tokens();
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // prevent crash if first token is ++ or --
    if (Token::Match(tok, "++|--"))
        tok = tok->next();

    for (; tok; tok = tok->next()) {
        bool result = false;
        if (Token::Match(tok, "++|--")) {
            if (Token::Match(tok->previous()->previous(), ";|{|}") && Token::Match(tok->next(), ";|)|,")) {
                result = true;
            } else if (tok->strAt(-2) == ",") {
                int i(1);
                while (tok->strAt(i) != ")" && tok->tokAt(i) != 0) {
                    if (tok->strAt(i) == ";") {
                        result = true;
                        break;
                    }
                    ++i;
                }
            } else if (tok->strAt(-2) == "<<" && tok->strAt(1) == "<<") {
                result = true;
            }
        }

        if (result && tok->previous()->varId()) {
            const Variable *var = symbolDatabase->getVariableFromVarId(tok->previous()->varId());
            if (!var || !Token::Match(var->typeEndToken(), "%type%"))
                continue;

            const Token *decltok = var->nameToken();

            if (Token::Match(decltok->previous(), "iterator|const_iterator|reverse_iterator|const_reverse_iterator")) {
                // the variable is an iterator
                postfixOperatorError(tok);
            } else if (var->type()) {
                // the variable is an instance of class
                postfixOperatorError(tok);
            }
        }
    }
}
//---------------------------------------------------------------------------


void CheckPostfixOperator::postfixOperatorError(const Token *tok)
{
    reportError(tok, Severity::performance, "postfixOperator",
                "Prefer prefix ++/-- operators for non-primitive types.\n"
                "Prefix ++/-- operators should be preferred for non-primitive types. "
                "Pre-increment/decrement can be more efficient than "
                "post-increment/decrement. Post-increment/decrement usually "
                "involves keeping a copy of the previous value around and "
                "adds a little extra code.");
}

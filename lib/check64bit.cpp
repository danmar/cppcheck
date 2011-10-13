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
// 64-bit portability
//---------------------------------------------------------------------------

#include "check64bit.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    Check64BitPortability instance;
}

/** Is given variable a pointer or array? */
static bool isaddr(const Variable *var)
{
    const Token *nametok = var ? var->nameToken() : 0;
    return (var && (nametok->strAt(-1) == "*" || nametok->strAt(1) == "["));
}

/** Is given variable an integer variable */
static bool isint(const Variable *var)
{
    return (var && Token::Match(var->nameToken()->previous(), "int|long|DWORD %var% !!["));
}

void Check64BitPortability::pointerassignment()
{
    if (!_settings->isEnabled("portability"))
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "[;{}] %var% = %var% [;+]")) {
            const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

            const Variable *var1(symbolDatabase->getVariableFromVarId(tok->tokAt(1)->varId()));
            const Variable *var2(symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId()));

            if (isaddr(var1) && isint(var2) && tok->strAt(4) != "+")
                assignmentIntegerToAddressError(tok->next());

            else if (isint(var1) && isaddr(var2) && !tok->tokAt(3)->isPointerCompare()) {
                // assigning address => warning
                // some trivial addition => warning
                if (Token::Match(tok->tokAt(4), "+ %any% !!;"))
                    continue;

                assignmentAddressToIntegerError(tok->next());
            }
        }
    }
}

void Check64BitPortability::assignmentAddressToIntegerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "AssignmentAddressToInteger",
                "Assigning an address value to the integer (int/long/etc) type is not portable\n"
                "Assigning an address value to the integer (int/long/etc) type is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 64-bit address to 32-bit integer. The safe "
                "way is to always assign addresses only to pointer types (or typedefs).");
}

void Check64BitPortability::assignmentIntegerToAddressError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "AssignmentIntegerToAddress",
                "Assigning an integer (int/long/etc) to a pointer is not portable\n"
                "Assigning an integer (int/long/etc) to a pointer is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 32-bit integer to 64-bit pointer. The safe "
                "way is to always assign address to pointer.");
}

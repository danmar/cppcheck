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
namespace
{
Check64BitPortability instance;
}

void Check64BitPortability::pointerassignment()
{
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;{}] %var% = %var% ;"))
        {
            const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

            const Variable *var1(symbolDatabase->getVariableFromVarId(tok->tokAt(1)->varId()));
            const Variable *var2(symbolDatabase->getVariableFromVarId(tok->tokAt(3)->varId()));
            if (!var1 || !var2)
                continue;

            // Check if var1 is an int/long/DWORD variable
            if (!Token::Match(var1->nameToken()->previous(), "int|long|DWORD"))
                continue;

            // Check if var2 is a pointer variable
            if (!Token::simpleMatch(var2->nameToken()->previous(), "*"))
                continue;

            pointerassignmentError(tok->next());
        }
    }
}

void Check64BitPortability::pointerassignmentError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "addresstoint", 
                "Assigning an address value to the integer (int/long/etc) type is not portable\n"
                "Assigning an address value to the integer (int/long/etc) type is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 64-bit address to 32-bit integer. The safe "
                "way is to always assign addresses only to pointer types (or typedefs).");
}

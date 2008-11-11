/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



//---------------------------------------------------------------------------
#ifndef CheckOtherH
#define CheckOtherH
//---------------------------------------------------------------------------

#include "tokenize.h"

class CheckOther
{
public:
    // Casting
    void WarningOldStylePointerCast();

    // Use standard functions instead
    void WarningIsDigit();

    // Use standard functions instead
    void WarningIsAlpha();

    // Redundant code
    void WarningRedundantCode();

    // Warning upon: if (condition);
    void WarningIf();

    // Assignment in condition
    void CheckIfAssignment();

    // Using dangerous functions
    void WarningDangerousFunctions();

    // Invalid function usage..
    void InvalidFunctionUsage();

    // Check for unsigned division that might create bad results
    void CheckUnsignedDivision();

    // Check scope of variables
    void CheckVariableScope();

    // Check for constant function parameter
    void CheckConstantFunctionParameter();

    // Check that all struct members are used
    void CheckStructMemberUsage();

    // Using char variable as array index / as operand in bit operation
    void CheckCharVariable();

    // Incomplete statement. A statement that only contains a constant or variable
    void CheckIncompleteStatement();
private:
    void CheckVariableScope_LookupVar( const TOKEN *tok1, const char varname[] );
};

//---------------------------------------------------------------------------
#endif


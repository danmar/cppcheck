/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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
#include "errorlogger.h"

class CheckOther
{
public:
    CheckOther(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger);
    ~CheckOther();

    // Casting
    void WarningOldStylePointerCast();

    // Redundant code
    void WarningRedundantCode();

    // Warning upon: if (condition);
    void WarningIf();

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

    /** str plus char */
    void strPlusChar();

#ifndef UNIT_TESTING
private:
#endif
    void CheckVariableScope_LookupVar(const Token *tok1, const char varname[]);

    // Redundant condition
    // if (haystack.find(needle) != haystack.end())
    //    haystack.remove(needle);
    void redundantCondition2();

    const Tokenizer *_tokenizer;
    ErrorLogger *_errorLogger;
    const Settings &_settings;
};

//---------------------------------------------------------------------------
#endif


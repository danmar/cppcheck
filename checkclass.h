/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
#ifndef CheckClassH
#define CheckClassH
//---------------------------------------------------------------------------

#include "tokenize.h"
#include "settings.h"
#include "errorlogger.h"

class CheckClass
{
public:
    CheckClass(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger);
    ~CheckClass();

    void constructors();

    void privateFunctions();

    void noMemset();

    void operatorEq();    // Warning upon "void operator=(.."

    // The destructor in a base class should be virtual
    void virtualDestructor();

private:
    struct VAR
    {
        const char *name;
        bool        init;
        struct VAR *next;
    };

    void ClassChecking_VarList_Initialize(const Token *tok1, const Token *ftok, struct VAR *varlist, const char classname[], std::list<std::string> &callstack);
    void InitVar(struct VAR *varlist, const char varname[]);
    const Token *FindClassFunction(const Token *tok, const char classname[], const char funcname[], int &indentlevel);
    struct VAR *ClassChecking_GetVarList(const Token *tok1);

    // Check constructors for a specified class
    void CheckConstructors(const Token *tok1, struct VAR *varlist, const char funcname[]);

    const Tokenizer *_tokenizer;
    Settings _settings;
    ErrorLogger *_errorLogger;
};
//---------------------------------------------------------------------------
#endif


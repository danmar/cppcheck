/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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

#include "check.h"
#include "settings.h"

class Token;

class CheckClass : public Check
{
public:
    CheckClass() : Check()
    { }

    CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    // TODO: run noMemset

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckClass checkClass(tokenizer, settings, errorLogger);

        if (settings->_checkCodingStyle)
        {
            checkClass.constructors();
            checkClass.operatorEq();
            checkClass.privateFunctions();
        }
        checkClass.virtualDestructor();
    }


    // Check that all class constructors are ok.
    void constructors();

    // Check that all private functions are called.
    void privateFunctions();

    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    void noMemset();

    // 'operator=' should return something..
    void operatorEq();    // Warning upon "void operator=(.."

    // The destructor in a base class should be virtual
    void virtualDestructor();

private:
    struct VAR
    {
        VAR(const char *name = 0, bool init = false, struct VAR *next = 0)
        {
            this->name = name;
            this->init = init;
            this->next = next;
        }

        const char *name;
        bool        init;
        struct VAR *next;
    };

    void ClassChecking_VarList_Initialize(const Token *tok1, const Token *ftok, struct VAR *varlist, const char classname[], std::list<std::string> &callstack);
    void InitVar(struct VAR *varlist, const char varname[]);
    struct VAR *ClassChecking_GetVarList(const Token *tok1);

    // Check constructors for a specified class
    void CheckConstructors(const Token *tok1, struct VAR *varlist, const char funcname[]);
};
//---------------------------------------------------------------------------
#endif


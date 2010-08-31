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
#ifndef checkexceptionsafetyH
#define checkexceptionsafetyH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"

class Token;

/// @addtogroup Checks
/// @{



/**
 * @brief %Check exception safety (exceptions shouldn't cause leaks nor corrupt data)
 *
 * The problem with these checks is that Cppcheck can't determine what the valid
 * values are for variables. But in some cases (dead pointers) it can be determined
 * that certain variable values are corrupt.
 */

class CheckExceptionSafety : public Check
{
public:
    /** This constructor is used when registering the CheckClass */
    CheckExceptionSafety() : Check()
    { }

    /** This constructor is used when running checks. */
    CheckExceptionSafety(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckExceptionSafety checkExceptionSafety(tokenizer, settings, errorLogger);
        checkExceptionSafety.destructors();
        checkExceptionSafety.deallocThrow();
    }


    /** Don't throw exceptions in destructors */
    void destructors();

    /** deallocating memory and then throw */
    void deallocThrow();

private:
    /** Don't throw exceptions in destructors */
    void destructorsError(const Token * const tok)
    {
        reportError(tok, Severity::style, "exceptThrowInDestructor", "Throwing exception in destructor");
    }

    void deallocThrowError(const Token * const tok, const std::string &varname)
    {
        reportError(tok, Severity::error, "exceptDeallocThrow", "Throwing exception in invalid state, " + varname + " points at deallocated memory");
    }

    /** Generate all possible errors (for --errorlist) */
    void getErrorMessages()
    {
        destructorsError(0);
        deallocThrowError(0, "p");
    }

    /** Short description of class (for --doc) */
    std::string name() const
    {
        return "Exception Safety";
    }

    /** wiki formatted description of the class (for --doc) */
    std::string classInfo() const
    {
        return "Checking exception safety\n"
               "* Throwing exceptions in destructors\n"
               "* Throwing exception during invalid state";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


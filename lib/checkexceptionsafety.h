/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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


class CheckExceptionSafety : public Check
{
public:
    /** This constructor is used when registering the CheckClass */
    CheckExceptionSafety() : Check()
    { }

    /** This constructor is used when running checks.. */
    CheckExceptionSafety(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckExceptionSafety checkExceptionSafety(tokenizer, settings, errorLogger);
        checkExceptionSafety.destructors();
    }


    /** Don't throw exceptions in destructors */
    void destructors();

    /** unsafe use of "new" */
    void unsafeNew();

private:
    /** Don't throw exceptions in destructors */
    void destructorsError(const Token * const tok)
    {
        reportError(tok, Severity::style, "throwInDestructor", "Throwing exception in destructor is not recommended");
    }

    /** Unsafe use of new */
    void unsafeNewError(const Token * const tok)
    {
        reportError(tok, Severity::style, "unsafeNew", "Exception safety: unsafe use of 'new'");
    }

    void getErrorMessages()
    {
        destructorsError(0);
        unsafeNewError(0);
    }

    std::string name() const
    {
        return "Exception Safety";
    }

    std::string classInfo() const
    {
        return "Checking exception safety\n"
               " * Don't throw exceptions in destructors"
               " * Unsafe use of 'new'";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


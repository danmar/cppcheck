/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
#include "config.h"

#include <string>

class Settings;
class ErrorLogger;
class Tokenizer;

/// @addtogroup Checks
/// @{


/**
 * @brief %Check exception safety (exceptions shouldn't cause leaks nor corrupt data)
 *
 * The problem with these checks is that Cppcheck can't determine what the valid
 * values are for variables. But in some cases (dead pointers) it can be determined
 * that certain variable values are corrupt.
 */

class CPPCHECKLIB CheckExceptionSafety : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckExceptionSafety() : Check("Exception Safety") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** Generate all possible errors (for --errorlist) */
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    /** wiki formatted description of the class (for --doc) */
    std::string classInfo() const override {
        return "Checking exception safety\n"
               "- Throwing exceptions in destructors\n"
               "- Throwing exception during invalid state\n"
               "- Throwing a copy of a caught exception instead of rethrowing the original exception\n"
               "- Exception caught by value instead of by reference\n"
               "- Throwing exception in noexcept, nothrow(), __attribute__((nothrow)) or __declspec(nothrow) function\n"
               "- Unhandled exception specification when calling function foo()\n"
               "- Rethrow without currently handled exception\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkexceptionsafetyH

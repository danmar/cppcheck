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
#ifndef CheckPostfixOperatorH
#define CheckPostfixOperatorH
//---------------------------------------------------------------------------

#include "check.h"

/// @addtogroup Checks
/// @{

/**
 * @brief Using postfix operators ++ or -- rather than postfix operator.
 */

class CheckPostfixOperator : public Check
{
public:
    /** This constructor is used when registering the CheckPostfixOperator */
    CheckPostfixOperator() : Check()
    { }

    /** This constructor is used when running checks. */
    CheckPostfixOperator(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckPostfixOperator checkPostfixOperator(tokenizer, settings, errorLogger);
        checkPostfixOperator.postfixOperator();
    }

    /** Check postfix operators */
    void postfixOperator();

private:
    /** Report Error */
    void postfixOperatorError(const Token *tok);

    void getErrorMessages()
    {
        postfixOperatorError(0);
    }

    std::string name() const
    {
        return "Using postfix operators";
    }

    std::string classInfo() const
    {
        return "Warn if using postfix operators ++ or -- rather than prefix operator\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


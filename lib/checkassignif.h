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
#ifndef checkassignifH
#define checkassignifH
//---------------------------------------------------------------------------

#include "check.h"


/// @addtogroup Checks
/// @{

/**
 * @brief Check for assignment / condition mismatches
 */

class CheckAssignIf : public Check {
public:
    /** This constructor is used when registering the CheckAssignIf */
    CheckAssignIf() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckAssignIf(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckAssignIf checkAssignIf(tokenizer, settings, errorLogger);
        checkAssignIf.assignIf();
        checkAssignIf.comparison();
        checkAssignIf.multiCondition();
    }

    /** mismatching assignment / comparison */
    void assignIf();

    /** mismatching lhs and rhs in comparison */
    void comparison();

    /** match 'if' and 'else if' conditions */
    void multiCondition();

private:

    void assignIfError(const Token *tok, bool result);
    void comparisonError(const Token *tok, bool result);
    void multiConditionError(const Token *tok, unsigned int line1);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) {
        CheckAssignIf c(0, settings, errorLogger);
        c.assignIfError(0, false);
        c.comparisonError(0, false);
        c.multiConditionError(0,1);
    }

    std::string myName() const {
        return "match assignments and conditions";
    }

    std::string classInfo() const {
        return "Match assignments and conditions:\n"
               " * Mismatching assignment and comparison => comparison is always true/false\n"
               " * Mismatching lhs and rhs in comparison => comparison is always true/false\n"
               " * Detect matching 'if' and 'else if' conditions";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


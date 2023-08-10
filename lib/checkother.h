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
#ifndef checkotherH
#define checkotherH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "errortypes.h"
#include "tokenize.h"

#include <string>

namespace ValueFlow {
    class Value;
}

class Settings;
class Token;
class Function;
class Variable;
class ErrorLogger;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckOther : public Check {
    friend class TestCharVar;
    friend class TestIncompleteStatement;
    friend class TestOther;

public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckOther() : Check("Other") {}

    /** Is expression a comparison that checks if a nonzero (unsigned/pointer) expression is less than zero? */
    static bool comparisonNonZeroExpressionLessThanZero(const Token *tok, const ValueFlow::Value **zeroValue, const Token **nonZeroExpr);

    /** Is expression a comparison that checks if a nonzero (unsigned/pointer) expression is positive? */
    static bool testIfNonZeroExpressionIsPositive(const Token *tok, const ValueFlow::Value **zeroValue, const Token **nonZeroExpr);

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    std::string classInfo() const override {
        return "Other checks\n"

               // error
               "- division with zero\n"
               "- scoped object destroyed immediately after construction\n"
               "- assignment in an assert statement\n"
               "- free() or delete of an invalid memory location\n"
               "- bitwise operation with negative right operand\n"
               "- cast the return values of getc(),fgetc() and getchar() to character and compare it to EOF\n"
               "- race condition with non-interlocked access after InterlockedDecrement() call\n"
               "- expression 'x = x++;' depends on order of evaluation of side effects\n"
               "- overlapping write of union\n"

               // warning
               "- either division by zero or useless condition\n"
               "- access of moved or forwarded variable.\n"

               // performance
               "- redundant data copying for const variable\n"
               "- subsequent assignment or copying to a variable or buffer\n"
               "- passing parameter by value\n"

               // portability
               "- Passing NULL pointer to function with variable number of arguments leads to UB.\n"

               // style
               "- C-style pointer cast in C++ code\n"
               "- casting between incompatible pointer types\n"
               "- [Incomplete statement](IncompleteStatement)\n"
               "- [check how signed char variables are used](CharVar)\n"
               "- variable scope can be limited\n"
               "- unusual pointer arithmetic. For example: \"abc\" + 'd'\n"
               "- redundant assignment, increment, or bitwise operation in a switch statement\n"
               "- redundant strcpy in a switch statement\n"
               "- Suspicious case labels in switch()\n"
               "- assignment of a variable to itself\n"
               "- Comparison of values leading always to true or false\n"
               "- Clarify calculation with parentheses\n"
               "- suspicious comparison of '\\0' with a char\\* variable\n"
               "- duplicate break statement\n"
               "- unreachable code\n"
               "- testing if unsigned variable is negative/positive\n"
               "- Suspicious use of ; at the end of 'if/for/while' statement.\n"
               "- Array filled incompletely using memset/memcpy/memmove.\n"
               "- NaN (not a number) value used in arithmetic expression.\n"
               "- comma in return statement (the comma can easily be misread as a semicolon).\n"
               "- prefer erfc, expm1 or log1p to avoid loss of precision.\n"
               "- identical code in both branches of if/else or ternary operator.\n"
               "- redundant pointer operation on pointer like &\\*some_ptr.\n"
               "- find unused 'goto' labels.\n"
               "- function declaration and definition argument names different.\n"
               "- function declaration and definition argument order different.\n"
               "- shadow variable.\n"
               "- variable can be declared const.\n"
               "- calculating modulo of one.\n"
               "- known function argument, suspicious calculation.\n";
    }

    // for testing
    static void checkCharVariable(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void warningOldStylePointerCast(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void invalidPointerCast(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    static void checkIncompleteStatement(Tokenizer* tokenizer, const Settings *settings, ErrorLogger *errorLogger);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkotherH

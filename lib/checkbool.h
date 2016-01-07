/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#ifndef checkboolH
#define checkboolH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

class Function;
class Variable;

/// @addtogroup Checks
/// @{


/** @brief checks dealing with suspicous usage of boolean type (not for evaluating conditions) */

class CPPCHECKLIB CheckBool : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckBool() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckBool(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckBool checkBool(tokenizer, settings, errorLogger);

        // Checks
        checkBool.checkComparisonOfBoolExpressionWithInt();
        checkBool.checkComparisonOfBoolWithInt();
        checkBool.checkAssignBoolToFloat();
        checkBool.pointerArithBool();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckBool checkBool(tokenizer, settings, errorLogger);

        // Checks
        checkBool.checkComparisonOfFuncReturningBool();
        checkBool.checkComparisonOfBoolWithBool();
        checkBool.checkIncrementBoolean();
        checkBool.checkAssignBoolToPointer();
        checkBool.checkBitwiseOnBoolean();
    }

    /** @brief %Check for comparison of function returning bool*/
    void checkComparisonOfFuncReturningBool();

    /** @brief %Check for comparison of variable of type bool*/
    void checkComparisonOfBoolWithBool();

    /** @brief %Check for using postfix increment on bool */
    void checkIncrementBoolean();

    /** @brief %Check for suspicious comparison of a bool and a non-zero (and non-one) value (e.g. "if (!x==4)") */
    void checkComparisonOfBoolWithInt();

    /** @brief assigning bool to pointer */
    void checkAssignBoolToPointer();

    /** @brief assigning bool to float */
    void checkAssignBoolToFloat();

    /** @brief %Check for using bool in bitwise expression */
    void checkBitwiseOnBoolean();

    /** @brief %Check for comparing a bool expression with an integer other than 0 or 1 */
    void checkComparisonOfBoolExpressionWithInt();

    /** @brief %Check for 'if (p+1)' etc. either somebody forgot to dereference, or else somebody uses pointer overflow */
    void pointerArithBool();
    void pointerArithBoolCond(const Token *tok);

private:
    // Error messages..
    void comparisonOfFuncReturningBoolError(const Token *tok, const std::string &expression);
    void comparisonOfTwoFuncsReturningBoolError(const Token *tok, const std::string &expression1, const std::string &expression2);
    void comparisonOfBoolWithBoolError(const Token *tok, const std::string &expression);
    void incrementBooleanError(const Token *tok);
    void comparisonOfBoolWithIntError(const Token *tok, const std::string &expression, bool n0o1);
    void comparisonOfBoolWithInvalidComparator(const Token *tok, const std::string &expression);
    void assignBoolToPointerError(const Token *tok);
    void assignBoolToFloatError(const Token *tok);
    void bitwiseOnBooleanError(const Token *tok, const std::string &varname, const std::string &op);
    void comparisonOfBoolExpressionWithIntError(const Token *tok, bool n0o1);
    void pointerArithBoolError(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckBool c(0, settings, errorLogger);

        c.assignBoolToPointerError(0);
        c.assignBoolToFloatError(0);
        c.comparisonOfFuncReturningBoolError(0, "func_name");
        c.comparisonOfTwoFuncsReturningBoolError(0, "func_name1", "func_name2");
        c.comparisonOfBoolWithBoolError(0, "var_name");
        c.incrementBooleanError(0);
        c.comparisonOfBoolWithIntError(0, "varname", true);
        c.bitwiseOnBooleanError(0, "varname", "&&");
        c.comparisonOfBoolExpressionWithIntError(0, true);
        c.pointerArithBoolError(0);
    }

    static std::string myName() {
        return "Boolean";
    }

    std::string classInfo() const {
        return "Boolean type checks\n"
               "- using increment on boolean\n"
               "- comparison of a boolean with a non-zero integer\n"
               "- comparison of a boolean expression with an integer other than 0 or 1\n"
               "- comparison of a function returning boolean value using relational operator\n"
               "- comparison of a boolean value with boolean value using relational operator\n"
               "- using bool in bitwise expression\n"
               "- pointer addition in condition (either dereference is forgot or pointer overflow is required to make the condition false)\n"
               "- Assigning bool value to pointer or float\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkboolH

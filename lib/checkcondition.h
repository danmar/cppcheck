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
#ifndef checkconditionH
#define checkconditionH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include "mathlib.h"

/// @addtogroup Checks
/// @{

/**
 * @brief Check for condition mismatches
 */

class CPPCHECKLIB CheckCondition : public Check {
public:
    /** This constructor is used when registering the CheckAssignIf */
    CheckCondition() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckCondition(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckCondition checkCondition(tokenizer, settings, errorLogger);
        checkCondition.multiCondition();
        checkCondition.clarifyCondition();   // not simplified because ifAssign
        checkCondition.oppositeInnerCondition();
        checkCondition.checkIncorrectLogicOperator();
        checkCondition.checkInvalidTestForOverflow();
        checkCondition.alwaysTrueFalse();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckCondition checkCondition(tokenizer, settings, errorLogger);
        checkCondition.assignIf();
        checkCondition.checkBadBitmaskCheck();
        checkCondition.comparison();
        checkCondition.checkModuloAlwaysTrueFalse();
    }

    /** mismatching assignment / comparison */
    void assignIf();

    /** parse scopes recursively */
    bool assignIfParseScope(const Token * const assignTok,
                            const Token * const startTok,
                            const unsigned int varid,
                            const bool islocal,
                            const char bitop,
                            const MathLib::bigint num);

    /** check bitmask using | instead of & */
    void checkBadBitmaskCheck();

    /** mismatching lhs and rhs in comparison */
    void comparison();

    /** match 'if' and 'else if' conditions */
    void multiCondition();

    /** To check the dead code in a program, which is inaccessible due to the counter-conditions check in nested-if statements **/
    void oppositeInnerCondition();

    /** @brief %Check for testing for mutual exclusion over ||*/
    void checkIncorrectLogicOperator();

    /** @brief %Check for suspicious usage of modulo (e.g. "if(var % 4 == 4)") */
    void checkModuloAlwaysTrueFalse();

    /** @brief Suspicious condition (assignment+comparison) */
    void clarifyCondition();

    /** @brief Condition is always true/false */
    void alwaysTrueFalse();

    /** @brief %Check for invalid test for overflow 'x+100 < x' */
    void checkInvalidTestForOverflow();

private:

    bool isOverlappingCond(const Token * const cond1, const Token * const cond2, bool pure) const;
    void assignIfError(const Token *tok1, const Token *tok2, const std::string &condition, bool result);
    void mismatchingBitAndError(const Token *tok1, const MathLib::bigint num1, const Token *tok2, const MathLib::bigint num2);
    void badBitmaskCheckError(const Token *tok);
    void comparisonError(const Token *tok,
                         const std::string &bitop,
                         MathLib::bigint value1,
                         const std::string &op,
                         MathLib::bigint value2,
                         bool result);
    void multiConditionError(const Token *tok, unsigned int line1);

    void oppositeInnerConditionError(const Token *tok1, const Token* tok2);

    void incorrectLogicOperatorError(const Token *tok, const std::string &condition, bool always, bool inconclusive);
    void redundantConditionError(const Token *tok, const std::string &text, bool inconclusive);

    void moduloAlwaysTrueFalseError(const Token* tok, const std::string& maxVal);

    void clarifyConditionError(const Token *tok, bool assign, bool boolop);

    void alwaysTrueFalseError(const Token *tok, bool knownResult);

    void invalidTestForOverflow(const Token* tok, bool result);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckCondition c(nullptr, settings, errorLogger);

        c.assignIfError(nullptr, 0, emptyString, false);
        c.badBitmaskCheckError(nullptr);
        c.comparisonError(nullptr, "&", 6, "==", 1, false);
        c.multiConditionError(nullptr,1);
        c.mismatchingBitAndError(nullptr, 0xf0, 0, 1);
        c.oppositeInnerConditionError(nullptr, 0);
        c.incorrectLogicOperatorError(nullptr, "foo > 3 && foo < 4", true, false);
        c.redundantConditionError(nullptr, "If x > 11 the condition x > 10 is always true.", false);
        c.moduloAlwaysTrueFalseError(nullptr, "1");
        c.clarifyConditionError(nullptr, true, false);
        c.alwaysTrueFalseError(nullptr, true);
        c.invalidTestForOverflow(nullptr, false);
    }

    static std::string myName() {
        return "Condition";
    }

    std::string classInfo() const {
        return "Match conditions with assignments and other conditions:\n"
               "- Mismatching assignment and comparison => comparison is always true/false\n"
               "- Mismatching lhs and rhs in comparison => comparison is always true/false\n"
               "- Detect usage of | where & should be used\n"
               "- Detect matching 'if' and 'else if' conditions\n"
               "- Mismatching bitand (a &= 0xf0; a &= 1; => a = 0)\n"
               "- Find dead code which is inaccessible due to the counter-conditions check in nested if statements\n"
               "- Condition that is always true/false\n"
               "- Mutual exclusion over || always evaluating to true\n"
               "- Comparisons of modulo results that are always true/false.\n"
               "- Known variable values => condition is always true/false\n"
               "- Invalid test for overflow (for example 'ptr+u < ptr'). Condition is always false unless there is overflow, and overflow is UB.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkconditionH

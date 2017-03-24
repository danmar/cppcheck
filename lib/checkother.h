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
#ifndef checkotherH
#define checkotherH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

class Function;
class Variable;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckOther : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckOther() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckOther(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        // Checks
        checkOther.warningOldStylePointerCast();
        checkOther.invalidPointerCast();
        checkOther.checkCharVariable();
        checkOther.checkRedundantAssignment();
        checkOther.checkRedundantAssignmentInSwitch();
        checkOther.checkSuspiciousCaseInSwitch();
        checkOther.checkDuplicateBranch();
        checkOther.checkDuplicateExpression();
        checkOther.checkUnreachableCode();
        checkOther.checkSuspiciousSemicolon();
        checkOther.checkVariableScope();
        checkOther.checkSignOfUnsignedVariable();  // don't ignore casts (#3574)
        checkOther.checkIncompleteArrayFill();
        checkOther.checkVarFuncNullUB();
        checkOther.checkNanInArithmeticExpression();
        checkOther.checkCommaSeparatedReturn();
        checkOther.checkRedundantPointerOp();
        checkOther.checkZeroDivision();
        checkOther.checkNegativeBitwiseShift();
        checkOther.checkInterlockedDecrement();
        checkOther.checkUnusedLabel();
        checkOther.checkEvaluationOrder();
        checkOther.checkFuncArgNamesDifferent();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        // Checks
        checkOther.clarifyCalculation();
        checkOther.clarifyStatement();
        checkOther.checkPassByReference();
        checkOther.checkIncompleteStatement();
        checkOther.checkCastIntToCharAndBack();

        checkOther.checkMisusedScopedObject();
        checkOther.checkMemsetZeroBytes();
        checkOther.checkMemsetInvalid2ndParam();
        checkOther.checkPipeParameterSize();

        checkOther.checkInvalidFree();
        checkOther.checkRedundantCopy();
        checkOther.checkSuspiciousEqualityComparison();
        checkOther.checkComparisonFunctionIsAlwaysTrueOrFalse();
        checkOther.checkAccessOfMovedVariable();
    }

    /** @brief Clarify calculation for ".. a * b ? .." */
    void clarifyCalculation();

    /** @brief Suspicious statement like '*A++;' */
    void clarifyStatement();

    /** @brief Are there C-style pointer casts in a c++ file? */
    void warningOldStylePointerCast();

    /** @brief Check for pointer casts to a type with an incompatible binary data representation */
    void invalidPointerCast();

    /** @brief %Check scope of variables */
    void checkVariableScope();
    static bool checkInnerScope(const Token *tok, const Variable* var, bool& used);

    /** @brief %Check for comma separated statements in return */
    void checkCommaSeparatedReturn();

    /** @brief %Check for function parameters that should be passed by reference */
    void checkPassByReference();

    /** @brief Using char variable as array index / as operand in bit operation */
    void checkCharVariable();

    /** @brief Incomplete statement. A statement that only contains a constant or variable */
    void checkIncompleteStatement();

    /** @brief %Check zero division*/
    void checkZeroDivision();

    /** @brief Check for NaN (not-a-number) in an arithmetic expression */
    void checkNanInArithmeticExpression();

    /** @brief copying to memory or assigning to a variable twice */
    void checkRedundantAssignment();

    /** @brief %Check for assigning to the same variable twice in a switch statement*/
    void checkRedundantAssignmentInSwitch();

    /** @brief %Check for code like 'case A||B:'*/
    void checkSuspiciousCaseInSwitch();

    /** @brief %Check for code like 'case A||B:'*/
    void checkSuspiciousEqualityComparison();

    /** @brief %Check for objects that are destroyed immediately */
    void checkMisusedScopedObject();

    /** @brief %Check for filling zero bytes with memset() */
    void checkMemsetZeroBytes();

    /** @brief %Check for invalid 2nd parameter of memset() */
    void checkMemsetInvalid2ndParam();

    /** @brief %Check for suspicious code where if and else branch are the same (e.g "if (a) b = true; else b = true;") */
    void checkDuplicateBranch();

    /** @brief %Check for suspicious code with the same expression on both sides of operator (e.g "if (a && a)") */
    void checkDuplicateExpression();

    /** @brief %Check for code that gets never executed, such as duplicate break statements */
    void checkUnreachableCode();

    /** @brief %Check for testing sign of unsigned variable */
    void checkSignOfUnsignedVariable();

    /** @brief %Check for suspicious use of semicolon */
    void checkSuspiciousSemicolon();

    /** @brief %Check for free() operations on invalid memory locations */
    void checkInvalidFree();
    void invalidFreeError(const Token *tok, bool inconclusive);

    /** @brief %Check for code creating redundant copies */
    void checkRedundantCopy();

    /** @brief %Check for bitwise shift with negative right operand */
    void checkNegativeBitwiseShift();

    /** @brief %Check for buffers that are filled incompletely with memset and similar functions */
    void checkIncompleteArrayFill();

    /** @brief %Check that variadic function calls don't use NULL. If NULL is \#defined as 0 and the function expects a pointer, the behaviour is undefined. */
    void checkVarFuncNullUB();

    /** @brief %Check that calling the POSIX pipe() system call is called with an integer array of size two. */
    void checkPipeParameterSize();

    /** @brief %Check to avoid casting a return value to unsigned char and then back to integer type.  */
    void checkCastIntToCharAndBack();

    /** @brief %Check for using of comparison functions evaluating always to true or false. */
    void checkComparisonFunctionIsAlwaysTrueOrFalse();

    /** @brief %Check for redundant pointer operations */
    void checkRedundantPointerOp();

    /** @brief %Check for race condition with non-interlocked access after InterlockedDecrement() */
    void checkInterlockedDecrement();

    /** @brief %Check for unused labels */
    void checkUnusedLabel();

    /** @brief %Check for expression that depends on order of evaluation of side effects */
    void checkEvaluationOrder();

    /** @brief %Check for access of moved or forwarded variable */
    void checkAccessOfMovedVariable();

    /** @brief %Check if function declaration and definition argument names different */
    void checkFuncArgNamesDifferent();

private:
    // Error messages..
    void checkComparisonFunctionIsAlwaysTrueOrFalseError(const Token* tok, const std::string &strFunctionName, const std::string &varName, const bool result);
    void checkCastIntToCharAndBackError(const Token *tok, const std::string &strFunctionName);
    void checkPipeParameterSizeError(const Token *tok, const std::string &strVarName, const std::string &strDim);
    void clarifyCalculationError(const Token *tok, const std::string &op);
    void clarifyStatementError(const Token* tok);
    void cstyleCastError(const Token *tok);
    void invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to, bool inconclusive);
    void passedByValueError(const Token *tok, const std::string &parname, bool inconclusive);
    void constStatementError(const Token *tok, const std::string &type);
    void signedCharArrayIndexError(const Token *tok);
    void unknownSignCharArrayIndexError(const Token *tok);
    void charBitOpError(const Token *tok);
    void variableScopeError(const Token *tok, const std::string &varname);
    void zerodivError(const Token *tok, bool inconclusive);
    void zerodivcondError(const Token *tokcond, const Token *tokdiv, bool inconclusive);
    void nanInArithmeticExpressionError(const Token *tok);
    void redundantAssignmentError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive);
    void redundantAssignmentInSwitchError(const Token *tok1, const Token *tok2, const std::string &var);
    void redundantCopyError(const Token *tok1, const Token* tok2, const std::string& var);
    void redundantCopyInSwitchError(const Token *tok1, const Token* tok2, const std::string &var);
    void redundantBitwiseOperationInSwitchError(const Token *tok, const std::string &varname);
    void suspiciousCaseInSwitchError(const Token* tok, const std::string& operatorString);
    void suspiciousEqualityComparisonError(const Token* tok);
    void selfAssignmentError(const Token *tok, const std::string &varname);
    void misusedScopeObjectError(const Token *tok, const std::string &varname);
    void memsetZeroBytesError(const Token *tok);
    void memsetFloatError(const Token *tok, const std::string &var_value);
    void memsetValueOutOfRangeError(const Token *tok, const std::string &value);
    void duplicateBranchError(const Token *tok1, const Token *tok2);
    void duplicateExpressionError(const Token *tok1, const Token *tok2, const std::string &op);
    void duplicateExpressionTernaryError(const Token *tok);
    void duplicateBreakError(const Token *tok, bool inconclusive);
    void unreachableCodeError(const Token* tok, bool inconclusive);
    void unsignedLessThanZeroError(const Token *tok, const std::string &varname, bool inconclusive);
    void pointerLessThanZeroError(const Token *tok, bool inconclusive);
    void unsignedPositiveError(const Token *tok, const std::string &varname, bool inconclusive);
    void pointerPositiveError(const Token *tok, bool inconclusive);
    void SuspiciousSemicolonError(const Token *tok);
    void negativeBitwiseShiftError(const Token *tok, int op);
    void redundantCopyError(const Token *tok, const std::string &varname);
    void incompleteArrayFillError(const Token* tok, const std::string& buffer, const std::string& function, bool boolean);
    void varFuncNullUBError(const Token *tok);
    void commaSeparatedReturnError(const Token *tok);
    void redundantPointerOpError(const Token* tok, const std::string& varname, bool inconclusive);
    void raceAfterInterlockedDecrementError(const Token* tok);
    void unusedLabelError(const Token* tok, bool inSwitch);
    void unknownEvaluationOrder(const Token* tok);
    static bool isMovedParameterAllowedForInconclusiveFunction(const Token * tok);
    void accessMovedError(const Token *tok, const std::string &varname, ValueFlow::Value::MoveKind moveKind, bool inconclusive);
    void funcArgNamesDifferent(const std::string & functionName, size_t index, const Token* declaration, const Token* definition);
    void funcArgOrderDifferent(const std::string & functionName, const Token * declaration, const Token * definition, const std::vector<const Token*> & declarations, const std::vector<const Token*> & definitions);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckOther c(nullptr, settings, errorLogger);

        // error
        c.zerodivError(nullptr,  false);
        c.zerodivcondError(nullptr, 0,false);
        c.misusedScopeObjectError(nullptr, "varname");
        c.invalidPointerCastError(nullptr,  "float", "double", false);
        c.negativeBitwiseShiftError(nullptr, 1);
        c.negativeBitwiseShiftError(nullptr, 2);
        c.checkPipeParameterSizeError(nullptr,  "varname", "dimension");
        c.raceAfterInterlockedDecrementError(nullptr);

        //performance
        c.redundantCopyError(nullptr,  "varname");
        c.redundantCopyError(nullptr,  0, "var");
        c.redundantAssignmentError(nullptr,  0, "var", false);

        // style/warning
        c.checkComparisonFunctionIsAlwaysTrueOrFalseError(nullptr, "isless","varName",false);
        c.checkCastIntToCharAndBackError(nullptr, "func_name");
        c.cstyleCastError(nullptr);
        c.passedByValueError(nullptr,  "parametername", false);
        c.constStatementError(nullptr,  "type");
        c.signedCharArrayIndexError(nullptr);
        c.unknownSignCharArrayIndexError(nullptr);
        c.charBitOpError(nullptr);
        c.variableScopeError(nullptr,  "varname");
        c.redundantAssignmentInSwitchError(nullptr,  0, "var");
        c.redundantCopyInSwitchError(nullptr,  0, "var");
        c.suspiciousCaseInSwitchError(nullptr,  "||");
        c.suspiciousEqualityComparisonError(nullptr);
        c.selfAssignmentError(nullptr,  "varname");
        c.memsetZeroBytesError(nullptr);
        c.memsetFloatError(nullptr,  "varname");
        c.memsetValueOutOfRangeError(nullptr,  "varname");
        c.clarifyCalculationError(nullptr,  "+");
        c.clarifyStatementError(nullptr);
        c.duplicateBranchError(nullptr,  0);
        c.duplicateExpressionError(nullptr,  0, "&&");
        c.duplicateExpressionTernaryError(nullptr);
        c.duplicateBreakError(nullptr,  false);
        c.unreachableCodeError(nullptr,  false);
        c.unsignedLessThanZeroError(nullptr,  "varname", false);
        c.unsignedPositiveError(nullptr,  "varname", false);
        c.pointerLessThanZeroError(nullptr,  false);
        c.pointerPositiveError(nullptr,  false);
        c.SuspiciousSemicolonError(nullptr);
        c.incompleteArrayFillError(nullptr,  "buffer", "memset", false);
        c.varFuncNullUBError(nullptr);
        c.nanInArithmeticExpressionError(nullptr);
        c.commaSeparatedReturnError(nullptr);
        c.redundantPointerOpError(nullptr,  "varname", false);
        c.unusedLabelError(nullptr,  true);
        c.unusedLabelError(nullptr,  false);
        c.unknownEvaluationOrder(nullptr);
        c.accessMovedError(nullptr, "v", ValueFlow::Value::MovedVariable, false);
        c.accessMovedError(nullptr, "v", ValueFlow::Value::ForwardedVariable, false);
        c.funcArgNamesDifferent("function", 1, nullptr, nullptr);

        std::vector<const Token *> nullvec;
        c.funcArgOrderDifferent("function", nullptr, nullptr, nullvec, nullvec);
    }

    static std::string myName() {
        return "Other";
    }

    std::string classInfo() const {
        return "Other checks\n"

               // error
               "- division with zero\n"
               "- scoped object destroyed immediately after construction\n"
               "- assignment in an assert statement\n"
               "- free() or delete of an invalid memory location\n"
               "- bitwise operation with negative right operand\n"
               "- provide wrong dimensioned array to pipe() system command (--std=posix)\n"
               "- cast the return values of getc(),fgetc() and getchar() to character and compare it to EOF\n"
               "- race condition with non-interlocked access after InterlockedDecrement() call\n"
               "- expression 'x = x++;' depends on order of evaluation of side effects\n"

               // warning
               "- either division by zero or useless condition\n"
               "- memset() with a value out of range as the 2nd parameter\n"
               "- access of moved or forwarded variable.\n"

               // performance
               "- redundant data copying for const variable\n"
               "- subsequent assignment or copying to a variable or buffer\n"
               "- passing parameter by value\n"

               // portability
               "- memset() with a float as the 2nd parameter\n"
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
               "- suspicious comparison of '\\0' with a char* variable\n"
               "- duplicate break statement\n"
               "- unreachable code\n"
               "- testing if unsigned variable is negative/positive\n"
               "- Suspicious use of ; at the end of 'if/for/while' statement.\n"
               "- Array filled incompletely using memset/memcpy/memmove.\n"
               "- NaN (not a number) value used in arithmetic expression.\n"
               "- comma in return statement (the comma can easily be misread as a semicolon).\n"
               "- prefer erfc, expm1 or log1p to avoid loss of precision.\n"
               "- identical code in both branches of if/else or ternary operator.\n"
               "- redundant pointer operation on pointer like &*some_ptr.\n"
               "- find unused 'goto' labels.\n"
               "- function declaration and definition argument names different.\n"
               "- function declaration and definition argument order different.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkotherH

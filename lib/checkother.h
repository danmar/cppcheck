/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

/** Is expressions same? */
bool isSameExpression(const Token *tok1, const Token *tok2, const std::set<std::string> &constFunctions);

/** Is expression of floating point type? */
bool astIsFloat(const Token *tok, bool unknown);


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
        checkOther.checkIgnoredReturnValue();
        checkOther.checkRedundantPointerOp();

        // --check-library : functions with nonmatching configuration
        checkOther.checkLibraryMatchFunctions();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        // Checks
        checkOther.clarifyCalculation();
        checkOther.clarifyStatement();
        checkOther.checkConstantFunctionParameter();
        checkOther.checkIncompleteStatement();
        checkOther.checkCastIntToCharAndBack();

        checkOther.invalidFunctionUsage();
        checkOther.checkZeroDivision();
        checkOther.checkMathFunctions();

        checkOther.checkMisusedScopedObject();
        checkOther.checkMemsetZeroBytes();
        checkOther.checkMemsetInvalid2ndParam();
        checkOther.checkSwitchCaseFallThrough();
        checkOther.checkPipeParameterSize();

        checkOther.checkInvalidFree();
        checkOther.checkRedundantCopy();
        checkOther.checkNegativeBitwiseShift();
        checkOther.checkSuspiciousEqualityComparison();
        checkOther.checkComparisonFunctionIsAlwaysTrueOrFalse();
    }

    /** @brief Clarify calculation for ".. a * b ? .." */
    void clarifyCalculation();

    /** @brief Suspicious statement like '*A++;' */
    void clarifyStatement();

    /** @brief Are there C-style pointer casts in a c++ file? */
    void warningOldStylePointerCast();

    /** @brief Check for pointer casts to a type with an incompatible binary data representation */
    void invalidPointerCast();

    /**
     * @brief Invalid function usage (invalid input value / overlapping data)
     *
     * %Check that given function parameters are valid according to the standard
     * - wrong radix given for strtol/strtoul
     * - overlapping data when using sprintf/snprintf
     * - wrong input value according to library
     */
    void invalidFunctionUsage();

    /** @brief %Check scope of variables */
    void checkVariableScope();
    static bool checkInnerScope(const Token *tok, const Variable* var, bool& used);

    /** @brief %Check for comma separated statements in return */
    void checkCommaSeparatedReturn();

    /** @brief %Check for constant function parameter */
    void checkConstantFunctionParameter();

    /** @brief Using char variable as array index / as operand in bit operation */
    void checkCharVariable();

    /** @brief Incomplete statement. A statement that only contains a constant or variable */
    void checkIncompleteStatement();

    /** @brief %Check zero division*/
    void checkZeroDivision();

    /** @brief %Check zero division / useless condition */
    void checkZeroDivisionOrUselessCondition();

    /** @brief Check for NaN (not-a-number) in an arithmetic expression */
    void checkNanInArithmeticExpression();

    /** @brief %Check for parameters given to math function that do not make sense*/
    void checkMathFunctions();

    /** @brief copying to memory or assigning to a variable twice */
    void checkRedundantAssignment();

    /** @brief %Check for assigning to the same variable twice in a switch statement*/
    void checkRedundantAssignmentInSwitch();

    /** @brief %Check for code like 'case A||B:'*/
    void checkSuspiciousCaseInSwitch();

    /** @brief %Check for code like 'case A||B:'*/
    void checkSuspiciousEqualityComparison();

    /** @brief %Check for switch case fall through without comment */
    void checkSwitchCaseFallThrough();

    /** @brief %Check for objects that are destroyed immediately */
    void checkMisusedScopedObject();

    /** @brief %Check for filling zero bytes with memset() */
    void checkMemsetZeroBytes();

    /** @brief %Check for invalid 2nd parameter of memset() */
    void checkMemsetInvalid2ndParam();

    /** @brief %Check for suspicious code where multiple if have the same expression (e.g "if (a) { } else if (a) { }") */
    void checkDuplicateIf();

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

    /** @brief %Check for ignored return values. */
    void checkIgnoredReturnValue();

    /** @brief %Check for redundant pointer operations */
    void checkRedundantPointerOp();

    /** @brief --check-library: warn for unconfigured function calls */
    void checkLibraryMatchFunctions();

private:
    // Error messages..
    void checkComparisonFunctionIsAlwaysTrueOrFalseError(const Token* tok, const std::string &strFunctionName, const std::string &varName, const bool result);
    void checkCastIntToCharAndBackError(const Token *tok, const std::string &strFunctionName);
    void checkPipeParameterSizeError(const Token *tok, const std::string &strVarName, const std::string &strDim);
    void clarifyCalculationError(const Token *tok, const std::string &op);
    void clarifyStatementError(const Token* tok);
    void redundantGetAndSetUserIdError(const Token *tok);
    void cstyleCastError(const Token *tok);
    void invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to, bool inconclusive);
    void invalidFunctionArgError(const Token *tok, const std::string &functionName, int argnr, const std::string &validstr);
    void invalidFunctionArgBoolError(const Token *tok, const std::string &functionName, int argnr);
    void passedByValueError(const Token *tok, const std::string &parname);
    void constStatementError(const Token *tok, const std::string &type);
    void charArrayIndexError(const Token *tok);
    void charBitOpError(const Token *tok);
    void variableScopeError(const Token *tok, const std::string &varname);
    void zerodivError(const Token *tok, bool inconclusive);
    void zerodivcondError(const Token *tokcond, const Token *tokdiv, bool inconclusive);
    void nanInArithmeticExpressionError(const Token *tok);
    void mathfunctionCallWarning(const Token *tok, const unsigned int numParam = 1);
    void mathfunctionCallWarning(const Token *tok, const std::string& oldexp, const std::string& newexp);
    void redundantAssignmentError(const Token *tok1, const Token* tok2, const std::string& var, bool inconclusive);
    void redundantAssignmentInSwitchError(const Token *tok1, const Token *tok2, const std::string &var);
    void redundantCopyError(const Token *tok1, const Token* tok2, const std::string& var);
    void redundantCopyInSwitchError(const Token *tok1, const Token* tok2, const std::string &var);
    void redundantBitwiseOperationInSwitchError(const Token *tok, const std::string &varname);
    void switchCaseFallThrough(const Token *tok);
    void suspiciousCaseInSwitchError(const Token* tok, const std::string& operatorString);
    void suspiciousEqualityComparisonError(const Token* tok);
    void selfAssignmentError(const Token *tok, const std::string &varname);
    void misusedScopeObjectError(const Token *tok, const std::string &varname);
    void memsetZeroBytesError(const Token *tok, const std::string &varname);
    void memsetFloatError(const Token *tok, const std::string &var_value);
    void memsetValueOutOfRangeError(const Token *tok, const std::string &value);
    void duplicateIfError(const Token *tok1, const Token *tok2);
    void duplicateBranchError(const Token *tok1, const Token *tok2);
    void duplicateExpressionError(const Token *tok1, const Token *tok2, const std::string &op);
    void duplicateExpressionTernaryError(const Token *tok);
    void alwaysTrueFalseStringCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void alwaysTrueStringVariableCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void duplicateBreakError(const Token *tok, bool inconclusive);
    void unreachableCodeError(const Token* tok, bool inconclusive);
    void unsignedLessThanZeroError(const Token *tok, const std::string &varname, bool inconclusive);
    void pointerLessThanZeroError(const Token *tok, bool inconclusive);
    void unsignedPositiveError(const Token *tok, const std::string &varname, bool inconclusive);
    void pointerPositiveError(const Token *tok, bool inconclusive);
    void SuspiciousSemicolonError(const Token *tok);
    void negativeBitwiseShiftError(const Token *tok);
    void redundantCopyError(const Token *tok, const std::string &varname);
    void incompleteArrayFillError(const Token* tok, const std::string& buffer, const std::string& function, bool boolean);
    void varFuncNullUBError(const Token *tok);
    void commaSeparatedReturnError(const Token *tok);
    void ignoredReturnValueError(const Token* tok, const std::string& function);
    void redundantPointerOpError(const Token* tok, const std::string& varname, bool inconclusive);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckOther c(0, settings, errorLogger);

        // error
        c.invalidFunctionArgError(0, "func_name", 1, "1-4");
        c.invalidFunctionArgBoolError(0, "func_name", 1);
        c.zerodivError(0, false);
        c.zerodivcondError(0,0,false);
        c.misusedScopeObjectError(NULL, "varname");
        c.invalidPointerCastError(0, "float", "double", false);
        c.negativeBitwiseShiftError(0);
        c.checkPipeParameterSizeError(0, "varname", "dimension");

        //performance
        c.redundantCopyError(0, "varname");
        c.redundantCopyError(0, 0, "var");
        c.redundantAssignmentError(0, 0, "var", false);

        // style/warning
        c.checkComparisonFunctionIsAlwaysTrueOrFalseError(0,"isless","varName",false);
        c.checkCastIntToCharAndBackError(0,"func_name");
        c.cstyleCastError(0);
        c.passedByValueError(0, "parametername");
        c.constStatementError(0, "type");
        c.charArrayIndexError(0);
        c.charBitOpError(0);
        c.variableScopeError(0, "varname");
        c.redundantAssignmentInSwitchError(0, 0, "var");
        c.redundantCopyInSwitchError(0, 0, "var");
        c.switchCaseFallThrough(0);
        c.suspiciousCaseInSwitchError(0, "||");
        c.suspiciousEqualityComparisonError(0);
        c.selfAssignmentError(0, "varname");
        c.mathfunctionCallWarning(0);
        c.mathfunctionCallWarning(0, "1 - erf(x)", "erfc(x)");
        c.memsetZeroBytesError(0, "varname");
        c.memsetFloatError(0, "varname");
        c.memsetValueOutOfRangeError(0, "varname");
        c.clarifyCalculationError(0, "+");
        c.clarifyStatementError(0);
        c.duplicateBranchError(0, 0);
        c.duplicateExpressionError(0, 0, "&&");
        c.duplicateExpressionTernaryError(0);
        c.duplicateBreakError(0, false);
        c.unreachableCodeError(0, false);
        c.unsignedLessThanZeroError(0, "varname", false);
        c.unsignedPositiveError(0, "varname", false);
        c.pointerLessThanZeroError(0, false);
        c.pointerPositiveError(0, false);
        c.SuspiciousSemicolonError(0);
        c.incompleteArrayFillError(0, "buffer", "memset", false);
        c.varFuncNullUBError(0);
        c.nanInArithmeticExpressionError(0);
        c.commaSeparatedReturnError(0);
        c.ignoredReturnValueError(0, "malloc");
        c.redundantPointerOpError(0, "varname", false);
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
               "- invalid input values for functions\n"

               // warning
               "- either division by zero or useless condition\n"
               "- memset() with a value out of range as the 2nd parameter\n"
               "- return value of certain functions not used\n"

               // performance
               "- redundant data copying for const variable\n"
               "- subsequent assignment or copying to a variable or buffer\n"

               // portability
               "- memset() with a float as the 2nd parameter\n"
               "- Passing NULL pointer to function with variable number of arguments leads to UB.\n"

               // style
               "- C-style pointer cast in C++ code\n"
               "- casting between incompatible pointer types\n"
               "- passing parameter by value\n"
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
               "- redundant get and set function of user id (--std=posix).\n"
               "- NaN (not a number) value used in arithmetic expression.\n"
               "- comma in return statement (the comma can easily be misread as a semicolon).\n"
               "- prefer erfc, expm1 or log1p to avoid loss of precision.\n"
               "- identical code in both branches of if/else or ternary operator.\n"
               "- redundant pointer operation on pointer like &*some_ptr.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkotherH

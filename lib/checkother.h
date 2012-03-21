/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#ifndef CheckOtherH
#define CheckOtherH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"

class Token;
class Function;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CheckOther : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckOther() : Check(myName())
    { }

    /** @brief This constructor is used when running checks. */
    CheckOther(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        // Coding style checks
        checkOther.warningOldStylePointerCast();
        checkOther.invalidPointerCast();
        checkOther.checkUnsignedDivision();
        checkOther.checkCharVariable();
        checkOther.strPlusChar();
        checkOther.sizeofsizeof();
        checkOther.sizeofCalculation();
        checkOther.checkRedundantAssignmentInSwitch();
        checkOther.checkAssignmentInAssert();
        checkOther.checkSizeofForArrayParameter();
        checkOther.checkSizeofForStrncmpSize();
        checkOther.checkSizeofForMallocSize();
        checkOther.checkSizeofForNumericParameter();
        checkOther.checkSelfAssignment();
        checkOther.checkDuplicateIf();
        checkOther.checkDuplicateBranch();
        checkOther.checkDuplicateExpression();
        checkOther.checkUnreachableCode();
        checkOther.checkSuspiciousSemicolon();
        checkOther.checkWrongPrintfScanfArguments();

        // information checks
        checkOther.checkVariableScope();

        checkOther.clarifyCondition();   // not simplified because ifAssign
        checkOther.checkComparisonOfBoolExpressionWithInt();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        checkOther.clarifyCalculation();

        // Coding style checks
        checkOther.checkConstantFunctionParameter();
        checkOther.checkIncompleteStatement();

        checkOther.invalidFunctionUsage();
        checkOther.checkZeroDivision();
        checkOther.checkMathFunctions();
        checkOther.checkCCTypeFunctions();
        checkOther.checkFflushOnInputStream();
        checkOther.invalidScanf();

        checkOther.checkCoutCerrMisusage();
        checkOther.checkIncorrectLogicOperator();
        checkOther.checkMisusedScopedObject();
        checkOther.checkMemsetZeroBytes();
        checkOther.checkIncorrectStringCompare();
        checkOther.checkIncrementBoolean();
        checkOther.checkComparisonOfBoolWithInt();
        checkOther.checkSwitchCaseFallThrough();
        checkOther.checkAlwaysTrueOrFalseStringCompare();

        checkOther.checkAssignBoolToPointer();
        checkOther.checkSignOfUnsignedVariable();
        checkOther.checkBitwiseOnBoolean();
        checkOther.checkDoubleFree();
    }

    /** @brief Clarify calculation for ".. a * b ? .." */
    void clarifyCalculation();

    /** @brief Suspicious condition (assignment+comparison) */
    void clarifyCondition();

    /** @brief Are there C-style pointer casts in a c++ file? */
    void warningOldStylePointerCast();

    /** @brief Check for pointer casts to a type with an incompatible binary data representation */
    void invalidPointerCast();

    /**
     * @brief Invalid function usage (invalid radix / overlapping data)
     *
     * %Check that given function parameters are valid according to the standard
     * - wrong radix given for strtol/strtoul
     * - overlapping data when using sprintf/snprintf
     */
    void invalidFunctionUsage();

    /** @brief %Check for unsigned division */
    void checkUnsignedDivision();

    /** @brief %Check scope of variables */
    void checkVariableScope();

    /** @brief %Check for constant function parameter */
    void checkConstantFunctionParameter();

    /** @brief Using char variable as array index / as operand in bit operation */
    void checkCharVariable();

    /** @brief Incomplete statement. A statement that only contains a constant or variable */
    void checkIncompleteStatement();

    /** @brief str plus char (unusual pointer arithmetic) */
    void strPlusChar();

    /** @brief %Check zero division*/
    void checkZeroDivision();

    /** @brief %Check for parameters given to math function that do not make sense*/
    void checkMathFunctions();

    /** @brief %Check for parameters given to cctype function that do make error*/
    void checkCCTypeFunctions();

    void lookupVar(const Token *tok1, const std::string &varname);

    /** @brief %Check for using fflush() on an input stream*/
    void checkFflushOnInputStream();

    /** @brief %Check for 'sizeof sizeof ..' */
    void sizeofsizeof();

    /** @brief %Check for calculations inside sizeof */
    void sizeofCalculation();

    /** @brief scanf can crash if width specifiers are not used */
    void invalidScanf();

    /** @brief %Checks type and number of arguments given to functions like printf or scanf*/
    void checkWrongPrintfScanfArguments();

    /** @brief %Check for assigning to the same variable twice in a switch statement*/
    void checkRedundantAssignmentInSwitch();

    /** @brief %Check for switch case fall through without comment */
    void checkSwitchCaseFallThrough();

    /** @brief %Check for missusage of std::cout */
    void checkCoutCerrMisusage();

    /** @brief %Check for assigning a variable to itself*/
    void checkSelfAssignment();

    /** @brief %Check for assignment to a variable in an assert test*/
    void checkAssignmentInAssert();

    /** @brief %Check for testing for mutual exclusion over ||*/
    void checkIncorrectLogicOperator();

    /** @brief %Check for objects that are destroyed immediately */
    void checkMisusedScopedObject();

    /** @brief %Check for filling zero bytes with memset() */
    void checkMemsetZeroBytes();

    /** @brief %Check for using sizeof with array given as function argument */
    void checkSizeofForArrayParameter();

    /** @brief %Check for using sizeof with a char pointer */
    void checkSizeofForStrncmpSize();

    /** @brief %Check for using sizeof of a variable when allocating it */
    void checkSizeofForMallocSize();

    /** @brief %Check for using sizeof with numeric given as function argument */
    void checkSizeofForNumericParameter();

    /** @brief %Check for using bad usage of strncmp and substr */
    void checkIncorrectStringCompare();

    /** @brief %Check for using postfix increment on bool */
    void checkIncrementBoolean();

    /** @brief %Check for suspicious comparison of a bool and a non-zero (and non-one) value (e.g. "if (!x==4)") */
    void checkComparisonOfBoolWithInt();

    /** @brief %Check for suspicious code where multiple if have the same expression (e.g "if (a) { } else if (a) { }") */
    void checkDuplicateIf();

    /** @brief %Check for suspicious code where if and else branch are the same (e.g "if (a) b = true; else b = true;") */
    void checkDuplicateBranch();

    /** @brief %Check for suspicious code with the same expression on both sides of operator (e.g "if (a && a)") */
    void checkDuplicateExpression();

    /** @brief %Check for suspicious code that compares string literals for equality */
    void checkAlwaysTrueOrFalseStringCompare();

    /** @brief %Check for code that gets never executed, such as duplicate break statements */
    void checkUnreachableCode();

    /** @brief assigning bool to pointer */
    void checkAssignBoolToPointer();

    /** @brief %Check for testing sign of unsigned variable */
    void checkSignOfUnsignedVariable();

    /** @brief %Check for using bool in bitwise expression */
    void checkBitwiseOnBoolean();

    /** @brief %Check for comparing a bool expression with an integer other than 0 or 1 */
    void checkComparisonOfBoolExpressionWithInt();

    /** @brief %Check for suspicious use of semicolon */
    void checkSuspiciousSemicolon();

    /** @brief %Check for double free or double close operations */
    void checkDoubleFree();

private:
    // Error messages..
    void clarifyCalculationError(const Token *tok, const std::string &op);
    void clarifyConditionError(const Token *tok, bool assign, bool boolop);
    void sizeofsizeofError(const Token *tok);
    void sizeofCalculationError(const Token *tok);
    void invalidScanfError(const Token *tok);
    void wrongPrintfScanfArgumentsError(const Token* tok,
                                        const std::string &function,
                                        unsigned int numFormat,
                                        unsigned int numFunction);
    void invalidScanfArgTypeError(const Token* tok, const std::string &functionName, unsigned int numFormat);
    void invalidPrintfArgTypeError_s(const Token* tok, unsigned int numFormat);
    void invalidPrintfArgTypeError_n(const Token* tok, unsigned int numFormat);
    void invalidPrintfArgTypeError_p(const Token* tok, unsigned int numFormat);
    void invalidPrintfArgTypeError_int(const Token* tok, unsigned int numFormat, char c);
    void invalidPrintfArgTypeError_float(const Token* tok, unsigned int numFormat, char c);
    void cstyleCastError(const Token *tok);
    void invalidPointerCastError(const Token* tok, const std::string& from, const std::string& to);
    void dangerousUsageStrtolError(const Token *tok);
    void sprintfOverlappingDataError(const Token *tok, const std::string &varname);
    void udivError(const Token *tok, bool inconclusive);
    void passedByValueError(const Token *tok, const std::string &parname);
    void constStatementError(const Token *tok, const std::string &type);
    void charArrayIndexError(const Token *tok);
    void charBitOpError(const Token *tok);
    void variableScopeError(const Token *tok, const std::string &varname);
    void strPlusCharError(const Token *tok);
    void zerodivError(const Token *tok);
    void coutCerrMisusageError(const Token* tok, const std::string& streamName);
    void mathfunctionCallError(const Token *tok, const unsigned int numParam = 1);
    void cctypefunctionCallError(const Token *tok, const std::string &functionName, const std::string &value);
    void fflushOnInputStreamError(const Token *tok, const std::string &varname);
    void redundantAssignmentInSwitchError(const Token *tok, const std::string &varname);
    void redundantStrcpyInSwitchError(const Token *tok, const std::string &varname);
    void switchCaseFallThrough(const Token *tok);
    void selfAssignmentError(const Token *tok, const std::string &varname);
    void assignmentInAssertError(const Token *tok, const std::string &varname);
    void incorrectLogicOperatorError(const Token *tok, const std::string &condition, bool always);
    void redundantConditionError(const Token *tok, const std::string &text);
    void misusedScopeObjectError(const Token *tok, const std::string &varname);
    void memsetZeroBytesError(const Token *tok, const std::string &varname);
    void sizeofForArrayParameterError(const Token *tok);
    void sizeofForStrncmpError(const Token *tok);
    void sizeofForMallocError(const Token *tok, const std::string &varname);
    void sizeofForNumericParameterError(const Token *tok);
    void incorrectStringCompareError(const Token *tok, const std::string& func, const std::string &string, const std::string &len);
    void incorrectStringBooleanError(const Token *tok, const std::string& string);
    void incrementBooleanError(const Token *tok);
    void comparisonOfBoolWithIntError(const Token *tok, const std::string &expression);
    void duplicateIfError(const Token *tok1, const Token *tok2);
    void duplicateBranchError(const Token *tok1, const Token *tok2);
    void duplicateExpressionError(const Token *tok1, const Token *tok2, const std::string &op);
    void alwaysTrueFalseStringCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void alwaysTrueStringVariableCompareError(const Token *tok, const std::string& str1, const std::string& str2);
    void duplicateBreakError(const Token *tok);
    void unreachableCodeError(const Token* tok);
    void assignBoolToPointerError(const Token *tok);
    void unsignedLessThanZeroError(const Token *tok, const std::string &varname, bool inconclusive);
    void unsignedPositiveError(const Token *tok, const std::string &varname, bool inconclusive);
    void bitwiseOnBooleanError(const Token *tok, const std::string &varname, const std::string &op);
    void comparisonOfBoolExpressionWithIntError(const Token *tok);
    void SuspiciousSemicolonError(const Token *tok);
    void doubleFreeError(const Token *tok, const std::string &varname);
    void doubleCloseDirError(const Token *tok, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckOther c(0, settings, errorLogger);

        // error
        c.assignBoolToPointerError(0);
        c.sprintfOverlappingDataError(0, "varname");
        c.udivError(0, false);
        c.zerodivError(0);
        c.mathfunctionCallError(0);
        c.fflushOnInputStreamError(0, "stdin");
        c.misusedScopeObjectError(NULL, "varname");
        c.sizeofForArrayParameterError(0);
        c.sizeofForStrncmpError(0);
        c.sizeofForMallocError(0, "varname");
        c.sizeofForNumericParameterError(0);
        c.coutCerrMisusageError(0, "cout");
        c.doubleFreeError(0, "varname");
        c.invalidPointerCastError(0, "float", "double");

        // style/warning
        c.cstyleCastError(0);
        c.dangerousUsageStrtolError(0);
        c.passedByValueError(0, "parametername");
        c.constStatementError(0, "type");
        c.charArrayIndexError(0);
        c.charBitOpError(0);
        c.variableScopeError(0, "varname");
        c.strPlusCharError(0);
        c.sizeofsizeofError(0);
        c.sizeofCalculationError(0);
        c.redundantAssignmentInSwitchError(0, "varname");
        c.switchCaseFallThrough(0);
        c.selfAssignmentError(0, "varname");
        c.assignmentInAssertError(0, "varname");
        c.invalidScanfError(0);
        c.incorrectLogicOperatorError(0, "foo > 3 && foo < 4", true);
        c.redundantConditionError(0, "If x > 10 the condition x > 11 is always true.");
        c.memsetZeroBytesError(0, "varname");
        c.clarifyCalculationError(0, "+");
        c.clarifyConditionError(0, true, false);
        c.incorrectStringCompareError(0, "substr", "\"Hello World\"", "12");
        c.incorrectStringBooleanError(0, "\"Hello World\"");
        c.incrementBooleanError(0);
        c.comparisonOfBoolWithIntError(0, "varname");
        c.duplicateIfError(0, 0);
        c.duplicateBranchError(0, 0);
        c.duplicateExpressionError(0, 0, "&&");
        c.alwaysTrueFalseStringCompareError(0, "str1", "str2");
        c.alwaysTrueStringVariableCompareError(0, "varname1", "varname2");
        c.duplicateBreakError(0);
        c.unreachableCodeError(0);
        c.unsignedLessThanZeroError(0, "varname", false);
        c.unsignedPositiveError(0, "varname", false);
        c.bitwiseOnBooleanError(0, "varname", "&&");
        c.comparisonOfBoolExpressionWithIntError(0);
        c.SuspiciousSemicolonError(0);
        c.wrongPrintfScanfArgumentsError(0,"printf",3,2);
        c.invalidScanfArgTypeError(0, "scanf", 1);
        c.invalidPrintfArgTypeError_s(0, 1);
        c.invalidPrintfArgTypeError_n(0, 1);
        c.invalidPrintfArgTypeError_p(0, 1);
        c.invalidPrintfArgTypeError_int(0, 1, 'u');
        c.invalidPrintfArgTypeError_float(0, 1, 'f');
        c.cctypefunctionCallError(0, "funname", "value");
    }

    std::string myName() const {
        return "Other";
    }

    std::string classInfo() const {
        return "Other checks\n"

               // error
               "* Assigning bool value to pointer (converting bool value to address)\n"
               "* [[OverlappingData|bad usage of the function 'sprintf' (overlapping data)]]\n"
               "* division with zero\n"
               "* using fflush() on an input stream\n"
               "* scoped object destroyed immediately after construction\n"
               "* assignment in an assert statement\n"
               "* sizeof for array given as function argument\n"
               "* sizeof for numeric given as function argument\n"
               "* using sizeof(pointer) for its own allocation\n"
               "* incorrect length arguments for 'substr' and 'strncmp'\n"
               "* invalid usage of output stream. For example: std::cout << std::cout;'\n"
               "* wrong number of arguments given to 'printf' or 'scanf;'\n"
               "* double free() or double closedir()\n"

               // style
               "* C-style pointer cast in cpp file\n"
               "* casting between incompatible pointer types\n"
               "* redundant if\n"
               "* bad usage of the function 'strtol'\n"
               "* [[CheckUnsignedDivision|unsigned division]]\n"
               "* Dangerous usage of 'scanf'\n"
               "* passing parameter by value\n"
               "* [[IncompleteStatement|Incomplete statement]]\n"
               "* [[charvar|check how signed char variables are used]]\n"
               "* variable scope can be limited\n"
               "* condition that is always true/false\n"
               "* unusal pointer arithmetic. For example: \"abc\" + 'd'\n"
               "* redundant assignment in a switch statement\n"
               "* redundant strcpy in a switch statement\n"
               "* look for 'sizeof sizeof ..'\n"
               "* look for calculations inside sizeof()\n"
               "* assignment of a variable to itself\n"
               "* mutual exclusion over || always evaluating to true\n"
               "* Clarify calculation with parentheses\n"
               "* using increment on boolean\n"
               "* comparison of a boolean with a non-zero integer\n"
               "* comparison of a boolean expression with an integer other than 0 or 1\n"
               "* suspicious condition (assignment+comparison)\n"
               "* suspicious condition (runtime comparison of string literals)\n"
               "* suspicious condition (string literals as boolean)\n"
               "* duplicate break statement\n"
               "* unreachable code\n"
               "* testing if unsigned variable is negative\n"
               "* testing is unsigned variable is positive\n"
               "* using bool in bitwise expression\n"
               "* Suspicious use of ; at the end of 'if/for/while' statement.\n"
               "* incorrect usage of functions from ctype library.\n"

               // optimisations
               "* optimisation: detect post increment/decrement\n";
    }

    /**
     * @brief Used in warningRedundantCode()
     * Iterates through the %var% tokens in a fully qualified name and concatenates them.
     */
    std::string concatNames(const Token **tok) const {
        std::string varname;
        while (Token::Match(*tok, "%var% ::|.")) {
            varname.append((*tok)->str());
            varname.append((*tok)->next()->str());
            *tok = (*tok)->tokAt(2);
        }

        if ((*tok)->isName())
            varname.append((*tok)->str());

        return varname;
    }

    void checkExpressionRange(const std::list<Function> &constFunctions,
                              const Token *start,
                              const Token *end,
                              const std::string &toCheck);

    void complexDuplicateExpressionCheck(const std::list<Function> &constFunctions,
                                         const Token *classStart,
                                         const std::string &toCheck,
                                         const std::string &alt);
};
/// @}
//---------------------------------------------------------------------------
#endif


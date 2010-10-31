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
#ifndef CheckOtherH
#define CheckOtherH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"

class Token;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CheckOther : public Check
{
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckOther() : Check()
    { }

    /** @brief This constructor is used when running checks. */
    CheckOther(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(tokenizer, settings, errorLogger)
    { }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        // Coding style checks
        checkOther.warningOldStylePointerCast();
        checkOther.checkUnsignedDivision();
        checkOther.checkCharVariable();
        checkOther.functionVariableUsage();
        checkOther.checkVariableScope();
        checkOther.checkStructMemberUsage();
        checkOther.strPlusChar();
        checkOther.sizeofsizeof();
        checkOther.sizeofCalculation();
        checkOther.checkRedundantAssignmentInSwitch();
        checkOther.checkAssignmentInAssert();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        // Coding style checks
        checkOther.checkConstantFunctionParameter();
        checkOther.checkIncompleteStatement();
        checkOther.checkEmptyStringTest();

        checkOther.invalidFunctionUsage();
        checkOther.checkZeroDivision();
        checkOther.checkMathFunctions();
        checkOther.checkFflushOnInputStream();
        checkOther.invalidScanf();

        checkOther.checkSelfAssignment();
        checkOther.checkIncorrectLogicOperator();
        checkOther.checkMisusedScopedObject();
    }

    /** @brief Are there C-style pointer casts in a c++ file? */
    void warningOldStylePointerCast();

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

    /** @brief %Check for unused function variables */
    void functionVariableUsage();
    void unusedVariableError(const Token *tok, const std::string &varname);
    void unreadVariableError(const Token *tok, const std::string &varname);
    void unassignedVariableError(const Token *tok, const std::string &varname);

    /** @brief %Check scope of variables */
    void checkVariableScope();

    /** @brief %Check for constant function parameter */
    void checkConstantFunctionParameter();

    /** @brief %Check that all struct members are used */
    void checkStructMemberUsage();

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

    void lookupVar(const Token *tok1, const std::string &varname);

    /** @brief %Check for inefficient empty string test*/
    void checkEmptyStringTest();

    /** @brief %Check for using fflush() on an input stream*/
    void checkFflushOnInputStream();

    /** @brief %Check for 'sizeof sizeof ..' */
    void sizeofsizeof();
    void sizeofsizeofError(const Token *tok);

    /** @brief %Check for calculations inside sizeof */
    void sizeofCalculation();
    void sizeofCalculationError(const Token *tok);

    /** @brief scanf can crash if width specifiers are not used */
    void invalidScanf();
    void invalidScanfError(const Token *tok);

    /** @brief %Check for assigning to the same variable twice in a switch statement*/
    void checkRedundantAssignmentInSwitch();

    /** @brief %Check for assigning a variable to itself*/
    void checkSelfAssignment();

    /** @brief %Check for assignment to a variable in an assert test*/
    void checkAssignmentInAssert();

    /** @brief %Check for testing for mutual exclusion over ||*/
    void checkIncorrectLogicOperator();

    /** @brief %Check for objects that are destroyed immediately */
    void checkMisusedScopedObject();

    // Error messages..
    void cstyleCastError(const Token *tok);
    void dangerousUsageStrtolError(const Token *tok);
    void sprintfOverlappingDataError(const Token *tok, const std::string &varname);
    void udivError(const Token *tok);
    void unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname);
    void passedByValueError(const Token *tok, const std::string &parname);
    void constStatementError(const Token *tok, const std::string &type);
    void charArrayIndexError(const Token *tok);
    void charBitOpError(const Token *tok);
    void variableScopeError(const Token *tok, const std::string &varname);
    void conditionAlwaysTrueFalse(const Token *tok, const std::string &truefalse);
    void strPlusChar(const Token *tok);
    void zerodivError(const Token *tok);
    void mathfunctionCallError(const Token *tok, const unsigned int numParam = 1);
    void emptyStringTestError(const Token *tok, const std::string &var_name, const bool isTestForEmpty);
    void fflushOnInputStreamError(const Token *tok, const std::string &varname);
    void redundantAssignmentInSwitchError(const Token *tok, const std::string &varname);
    void selfAssignmentError(const Token *tok, const std::string &varname);
    void assignmentInAssertError(const Token *tok, const std::string &varname);
    void incorrectLogicOperatorError(const Token *tok);
    void misusedScopeObjectError(const Token *tok, const std::string &varname);

    void getErrorMessages()
    {
        // error
        sprintfOverlappingDataError(0, "varname");
        udivError(0);
        zerodivError(0);
        mathfunctionCallError(0);
        fflushOnInputStreamError(0, "stdin");
        misusedScopeObjectError(NULL, "varname");

        // style/warning
        cstyleCastError(0);
        dangerousUsageStrtolError(0);
        unusedStructMemberError(0, "structname", "variable");
        passedByValueError(0, "parametername");
        constStatementError(0, "type");
        charArrayIndexError(0);
        charBitOpError(0);
        variableScopeError(0, "varname");
        conditionAlwaysTrueFalse(0, "true/false");
        strPlusChar(0);
        sizeofsizeofError(0);
        sizeofCalculationError(0);
        redundantAssignmentInSwitchError(0, "varname");
        selfAssignmentError(0, "varname");
        assignmentInAssertError(0, "varname");
        invalidScanfError(0);
        incorrectLogicOperatorError(0);
        unusedVariableError(0, "varname");
        unreadVariableError(0, "varname");
        unassignedVariableError(0, "varname");

        // performance
        emptyStringTestError(0, "varname", true);
    }

    std::string name() const
    {
        return "Other";
    }

    std::string classInfo() const
    {
        return "Other checks\n"

               // error
               "* [[OverlappingData|bad usage of the function 'sprintf' (overlapping data)]]\n"
               "* division with zero\n"
               "* using fflush() on an input stream\n"
               "* scoped object destroyed immediately after construction\n"
               "* assignment in an assert statement\n"

               // style
               "* C-style pointer cast in cpp file\n"
               "* redundant if\n"
               "* bad usage of the function 'strtol'\n"
               "* [[CheckUnsignedDivision|unsigned division]]\n"
               "* Dangerous usage of 'scanf'\n"
               "* unused struct member\n"
               "* passing parameter by value\n"
               "* [[IncompleteStatement|Incomplete statement]]\n"
               "* [[charvar|check how signed char variables are used]]\n"
               "* variable scope can be limited\n"
               "* condition that is always true/false\n"
               "* unusal pointer arithmetic. For example: \"abc\" + 'd'\n"
               "* redundant assignment in a switch statement\n"
               "* look for 'sizeof sizeof ..'\n"
               "* look for calculations inside sizeof()\n"
               "* assignment of a variable to itself\n"
               "* mutual exclusion over || always evaluating to true\n"

               // optimisations
               "* optimisation: detect post increment/decrement\n"
               "* optimisation: simplify empty string tests\n";
    }

private:

    /**
     * @brief Used in warningRedundantCode()
     * Iterates through the %var% tokens in a fully qualified name and concatenates them.
     */
    std::string concatNames(const Token **tok) const
    {
        std::string varname;
        while (Token::Match(*tok, "%var% ::|."))
        {
            varname.append((*tok)->str());
            varname.append((*tok)->next()->str());
            *tok = (*tok)->tokAt(2);
        }

        if (Token::Match(*tok, "%var%"))
            varname.append((*tok)->str());

        return varname;
    }

    /**
     * @brief query type of identifier
     * @param tok Token of the identifier
     * @return true if the identifier is of type 'class' or 'struct',
     *         false otherwise.
     */
    bool isIdentifierObjectType(const Token* const tok);

    std::map<std::string, bool> isClassResults;
};
/// @}
//---------------------------------------------------------------------------
#endif


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

        checkOther.nullPointer();

        // Coding style checks
        checkOther.warningOldStylePointerCast();
        checkOther.checkUnsignedDivision();
        checkOther.checkCharVariable();
        checkOther.functionVariableUsage();
        checkOther.checkVariableScope();
        checkOther.checkStructMemberUsage();
        checkOther.strPlusChar();
        checkOther.sizeofsizeof();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckOther checkOther(tokenizer, settings, errorLogger);

        // Coding style checks
        checkOther.warningRedundantCode();
        checkOther.checkConstantFunctionParameter();
        checkOther.checkIncompleteStatement();
        checkOther.checkEmptyStringTest();
        checkOther.postIncrement();

        checkOther.invalidFunctionUsage();
        checkOther.checkZeroDivision();
        checkOther.checkMathFunctions();
        checkOther.checkFflushOnInputStream();

        // New type of check: Check execution paths
        checkOther.executionPaths();
    }


    /**
     * @brief Uninitialized variables: analyse functions to see how they work with uninitialized variables
     * @param tokens [in] the token list
     * @param func [out] names of functions that don't handle uninitialized variables well. the function names are added to the set. No clearing is made.
     * @param showAll [in] enable --all checking
     */
    static void analyseFunctions(const Token * const tokens, std::set<std::string> &func, bool showAll);

    /** @brief Are there C-style pointer casts in a c++ file? */
    void warningOldStylePointerCast();

    /** @brief Redundant code: if (p) delete p; */
    void warningRedundantCode();

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

    /** @brief possible null pointer dereference */
    void nullPointer();

    /** @brief new type of check: check execution paths */
    void executionPaths();

    /** @brief %Check zero division*/
    void checkZeroDivision();

    /** @brief %Check for parameters given to math function that do not make sense*/
    void checkMathFunctions();

    /** @brief %Check for post increment/decrement in for loop*/
    void postIncrement();

    void lookupVar(const Token *tok1, const std::string &varname);

    // Redundant condition
    // if (haystack.find(needle) != haystack.end())
    //    haystack.remove(needle);
    void redundantCondition2();

    /** @brief %Check for inefficient empty string test*/
    void checkEmptyStringTest();

    /** @brief %Check for using fflush() on an input stream*/
    void checkFflushOnInputStream();

    /** @brief %Check for 'sizeof sizeof ..' */
    void sizeofsizeof();
    void sizeofsizeofError(const Token *tok);

    // Error messages..
    void cstyleCastError(const Token *tok);
    void redundantIfDelete0Error(const Token *tok);
    void redundantIfRemoveError(const Token *tok);
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
    void nullPointerError(const Token *tok);  // variable name unknown / doesn't exist
    void nullPointerError(const Token *tok, const std::string &varname);
    void nullPointerError(const Token *tok, const std::string &varname, const int line);
    void uninitstringError(const Token *tok, const std::string &varname);
    void uninitdataError(const Token *tok, const std::string &varname);
    void uninitvarError(const Token *tok, const std::string &varname);
    void zerodivError(const Token *tok);
    void mathfunctionCallError(const Token *tok, const unsigned int numParam = 1);
    void postIncrementError(const Token *tok, const std::string &var_name, const bool isIncrement);
    void emptyStringTestError(const Token *tok, const std::string &var_name, const bool isTestForEmpty);
    void fflushOnInputStreamError(const Token *tok, const std::string &varname);

    void getErrorMessages()
    {
        // error
        sprintfOverlappingDataError(0, "varname");
        udivError(0);
        nullPointerError(0, "pointer");
        uninitstringError(0, "varname");
        uninitdataError(0, "varname");
        uninitvarError(0, "varname");
        zerodivError(0);
        mathfunctionCallError(0);
        fflushOnInputStreamError(0, "stdin");

        // style
        cstyleCastError(0);
        redundantIfDelete0Error(0);
        redundantIfRemoveError(0);
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

        // optimisations
        postIncrementError(0, "varname", true);
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
               "* null pointer dereferencing\n"
               "* using uninitialized variables and data\n"
               "* using fflush() on an input stream\n"

               // style
               "* C-style pointer cast in cpp file\n"
               "* redundant if\n"
               "* bad usage of the function 'strtol'\n"
               "* [[CheckUnsignedDivision|unsigned division]]\n"
               "* unused struct member\n"
               "* passing parameter by value\n"
               "* [[IncompleteStatement|Incomplete statement]]\n"
               "* [[charvar|check how signed char variables are used]]\n"
               "* variable scope can be limited\n"
               "* condition that is always true/false\n"
               "* unusal pointer arithmetic. For example: \"abc\" + 'd'\n"

               // optimisations
               "* optimisation: detect post increment/decrement\n"
               "* optimisation: simplify empty string tests\n";
    }

private:

    /**
     * @brief Does one part of the check for nullPointer().
     * Locate insufficient null-pointer handling after loop
     */
    void nullPointerAfterLoop();

    /**
     * @brief Does one part of the check for nullPointer().
     * looping through items in a linked list in a inner loop..
     */
    void nullPointerLinkedList();

    /**
     * @brief Does one part of the check for nullPointer().
     * Dereferencing a struct pointer and then checking if it's NULL..
     */
    void nullPointerStructByDeRefAndChec();

    /**
     * @brief Does one part of the check for nullPointer().
     * Dereferencing a pointer and then checking if it's NULL..
     */
    void nullPointerByDeRefAndChec();

    /**
     * @brief Does one part of the check for nullPointer().
     * -# initialize pointer to 0
     * -# conditionally assign pointer
     * -# dereference pointer
     */
    void nullPointerConditionalAssignment();

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
};
/// @}
//---------------------------------------------------------------------------
#endif


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
#ifndef CheckAutoVariablesH
#define CheckAutoVariablesH
//---------------------------------------------------------------------------

#include "check.h"
#include "token.h"
#include <set>

/// @addtogroup Checks
/// @{


class CheckAutoVariables : public Check
{
public:
    /** This constructor is used when registering the CheckClass */
    CheckAutoVariables() : Check(myName())
    { }

    /** This constructor is used when running checks. */
    CheckAutoVariables(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckAutoVariables checkAutoVariables(tokenizer, settings, errorLogger);
        checkAutoVariables.returnReference();
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckAutoVariables checkAutoVariables(tokenizer, settings, errorLogger);
        checkAutoVariables.autoVariables();
        checkAutoVariables.returnPointerToLocalArray();
        checkAutoVariables.returncstr();
    }

    /** Check auto variables */
    void autoVariables();

    /** Returning pointer to local array */
    void returnPointerToLocalArray();

    /** Returning reference to local/temporary variable */
    void returnReference();

    /** Returning c_str to local variable */
    void returncstr();

private:
    bool errorAv(const Token* left, const Token* right);
    bool isAutoVar(unsigned int varId);
    bool isAutoVarArray(unsigned int varId);

    /**
     * Returning a temporary object?
     * @param tok pointing at the "return" token
     * @return true if a temporary object is returned
     */
    bool returnTemporary(const Token *tok) const;

    void errorReturnAddressToAutoVariable(const Token *tok);
    void errorReturnPointerToLocalArray(const Token *tok);
    void errorAutoVariableAssignment(const Token *tok, bool inconclusive);
    void errorReturnReference(const Token *tok);
    void errorReturnTempReference(const Token *tok);
    void errorReturnAutocstr(const Token *tok);
    void errorReturnTempPointer(const Token *tok);
    void errorInvalidDeallocation(const Token *tok);
    void errorReturnAddressOfFunctionParameter(const Token *tok, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
    {
        CheckAutoVariables c(0,settings,errorLogger);
        c.errorAutoVariableAssignment(0, false);
        c.errorReturnAddressToAutoVariable(0);
        c.errorReturnPointerToLocalArray(0);
        c.errorReturnReference(0);
        c.errorReturnTempReference(0);
        c.errorReturnAutocstr(0);
        c.errorReturnTempPointer(0);
        c.errorInvalidDeallocation(0);
        c.errorReturnAddressOfFunctionParameter(0, "parameter");
    }

    std::string myName() const
    {
        return "Auto Variables";
    }

    std::string classInfo() const
    {
        return "A pointer to a variable is only valid as long as the variable is in scope.\n"
               "Check:\n"
               "* returning a pointer to auto or temporary variable\n"
               "* assigning address of an variable to an effective parameter of a function\n"
               "* returning reference to local/temporary variable\n"
               "* returning address of function parameter\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


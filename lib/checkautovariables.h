/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
    CheckAutoVariables() : Check()
    { }

    /** This constructor is used when running checks.. */
    CheckAutoVariables(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckAutoVariables checkAutoVariables(tokenizer, settings, errorLogger);
        checkAutoVariables.autoVariables();
        checkAutoVariables.returnPointerToLocalArray();
        checkAutoVariables.returnReference();
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
    std::set<std::string> fp_list;
    std::set<unsigned int> vd_list;
    std::set<unsigned int> vda_list;
    bool errorAv(const Token* left, const Token* right);
    bool isAutoVar(unsigned int varId);
    bool isAutoVarArray(unsigned int varId);
    void addVD(unsigned int varId);
    void addVDA(unsigned int varId);



    void errorReturnPointerToLocalArray(const Token *tok);
    void errorAutoVariableAssignment(const Token *tok);
    void errorReturnReference(const Token *tok);
    void errorReturnAutocstr(const Token *tok);

    void getErrorMessages()
    {
        errorAutoVariableAssignment(0);
        errorReturnPointerToLocalArray(0);
        errorReturnReference(0);
        errorReturnAutocstr(0);
    }

    std::string name() const
    {
        return "Auto Variables";
    }

    std::string classInfo() const
    {
        return "A pointer to a variable is only valid as long as the variable is in scope.\n"
               "Check:\n"
               "* returning a pointer to auto variable\n"
               "* assigning address of an variable to an effective parameter of a function\n"
               "* returning reference to local/temporary variable\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


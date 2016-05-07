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
#ifndef checkautovariablesH
#define checkautovariablesH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

/// @addtogroup Checks
/** @brief Various small checks for automatic variables */
/// @{


class CPPCHECKLIB CheckAutoVariables : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckAutoVariables() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckAutoVariables(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckAutoVariables checkAutoVariables(tokenizer, settings, errorLogger);
        checkAutoVariables.assignFunctionArg();
        checkAutoVariables.returnReference();
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckAutoVariables checkAutoVariables(tokenizer, settings, errorLogger);
        checkAutoVariables.autoVariables();
        checkAutoVariables.returnPointerToLocalArray();
    }

    /** assign function argument */
    void assignFunctionArg();

    /** Check auto variables */
    void autoVariables();

    /** Returning pointer to local array */
    void returnPointerToLocalArray();

    /** Returning reference to local/temporary variable */
    void returnReference();

private:
    static bool isPtrArg(const Token *tok);
    static bool isArrayArg(const Token *tok);
    static bool isRefPtrArg(const Token *tok);
    static bool isNonReferenceArg(const Token *tok);
    static bool isAutoVar(const Token *tok);
    static bool isAutoVarArray(const Token *tok);

    /**
     * Returning a temporary object?
     * @param tok pointing at the "return" token
     * @return true if a temporary object is returned
     */
    static bool returnTemporary(const Token *tok);

    void errorReturnAddressToAutoVariable(const Token *tok);
    void errorReturnPointerToLocalArray(const Token *tok);
    void errorAutoVariableAssignment(const Token *tok, bool inconclusive);
    void errorReturnReference(const Token *tok);
    void errorReturnTempReference(const Token *tok);
    void errorInvalidDeallocation(const Token *tok);
    void errorReturnAddressOfFunctionParameter(const Token *tok, const std::string &varname);
    void errorUselessAssignmentArg(const Token *tok);
    void errorUselessAssignmentPtrArg(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckAutoVariables c(nullptr,settings,errorLogger);
        c.errorAutoVariableAssignment(nullptr, false);
        c.errorReturnAddressToAutoVariable(nullptr);
        c.errorReturnPointerToLocalArray(nullptr);
        c.errorReturnReference(nullptr);
        c.errorReturnTempReference(nullptr);
        c.errorInvalidDeallocation(nullptr);
        c.errorReturnAddressOfFunctionParameter(nullptr, "parameter");
        c.errorUselessAssignmentArg(nullptr);
        c.errorUselessAssignmentPtrArg(nullptr);
    }

    static std::string myName() {
        return "Auto Variables";
    }

    std::string classInfo() const {
        return "A pointer to a variable is only valid as long as the variable is in scope.\n"
               "Check:\n"
               "- returning a pointer to auto or temporary variable\n"
               "- assigning address of an variable to an effective parameter of a function\n"
               "- returning reference to local/temporary variable\n"
               "- returning address of function parameter\n"
               "- suspicious assignment of pointer argument\n"
               "- useless assignment of function argument\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkautovariablesH

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "check.h"
#include "config.h"
#include "errortypes.h"

#include <string>

class Settings;
class Token;
class Tokenizer;
class ErrorLogger;
class Variable;

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
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) OVERRIDE {
        CheckAutoVariables checkAutoVariables(tokenizer, settings, errorLogger);
        checkAutoVariables.assignFunctionArg();
        checkAutoVariables.checkVarLifetime();
        checkAutoVariables.autoVariables();
    }

    /** assign function argument */
    void assignFunctionArg();

    /** Check auto variables */
    void autoVariables();

    /**
     * Check variable assignment.. value must be changed later or there will be a error reported
     * @return true if error is reported */
    bool checkAutoVariableAssignment(const Token *expr, bool inconclusive, const Token *startToken = nullptr);

    void checkVarLifetime();

    void checkVarLifetimeScope(const Token * start, const Token * end);

private:
    void errorReturnAddressToAutoVariable(const Token *tok);
    void errorReturnAddressToAutoVariable(const Token *tok, const ValueFlow::Value *value);
    void errorReturnPointerToLocalArray(const Token *tok);
    void errorAutoVariableAssignment(const Token *tok, bool inconclusive);
    void errorReturnDanglingLifetime(const Token *tok, const ValueFlow::Value* val);
    void errorInvalidLifetime(const Token *tok, const ValueFlow::Value* val);
    void errorDanglngLifetime(const Token *tok, const ValueFlow::Value *val);
    void errorDanglingTemporaryLifetime(const Token* tok, const ValueFlow::Value* val);
    void errorReturnReference(const Token* tok, ErrorPath errorPath, bool inconclusive);
    void errorDanglingReference(const Token *tok, const Variable *var, ErrorPath errorPath);
    void errorDanglingTempReference(const Token* tok, ErrorPath errorPath, bool inconclusive);
    void errorReturnTempReference(const Token* tok, ErrorPath errorPath, bool inconclusive);
    void errorInvalidDeallocation(const Token *tok, const ValueFlow::Value *val);
    void errorReturnAddressOfFunctionParameter(const Token *tok, const std::string &varname);
    void errorUselessAssignmentArg(const Token *tok);
    void errorUselessAssignmentPtrArg(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const OVERRIDE {
        ErrorPath errorPath;
        CheckAutoVariables c(nullptr,settings,errorLogger);
        c.errorAutoVariableAssignment(nullptr, false);
        c.errorReturnAddressToAutoVariable(nullptr);
        c.errorReturnPointerToLocalArray(nullptr);
        c.errorReturnReference(nullptr, errorPath, false);
        c.errorDanglingReference(nullptr, nullptr, errorPath);
        c.errorReturnTempReference(nullptr, errorPath, false);
        c.errorDanglingTempReference(nullptr, errorPath, false);
        c.errorInvalidDeallocation(nullptr, nullptr);
        c.errorReturnAddressOfFunctionParameter(nullptr, "parameter");
        c.errorUselessAssignmentArg(nullptr);
        c.errorUselessAssignmentPtrArg(nullptr);
        c.errorReturnDanglingLifetime(nullptr, nullptr);
        c.errorInvalidLifetime(nullptr, nullptr);
        c.errorDanglngLifetime(nullptr, nullptr);
        c.errorDanglingTemporaryLifetime(nullptr, nullptr);
    }

    static std::string myName() {
        return "Auto Variables";
    }

    std::string classInfo() const OVERRIDE {
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

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
#include "tokenize.h"

#include <string>
#include <set>

class Settings;
class Token;
class ErrorLogger;
class Variable;

namespace ValueFlow {
    class Value;
}

/// @addtogroup Checks
/** @brief Various small checks for automatic variables */
/// @{


class CPPCHECKLIB CheckAutoVariables : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckAutoVariables() : Check(myName()) {}

    /** This constructor is used when running checks. */
    CheckAutoVariables(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override {
        CheckAutoVariables checkAutoVariables(&tokenizer, tokenizer.getSettings(), errorLogger);
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
    void errorAutoVariableAssignment(const Token *tok, bool inconclusive);
    void errorReturnDanglingLifetime(const Token *tok, const ValueFlow::Value* val);
    void errorInvalidLifetime(const Token *tok, const ValueFlow::Value* val);
    void errorDanglngLifetime(const Token *tok, const ValueFlow::Value *val);
    void errorDanglingTemporaryLifetime(const Token* tok, const ValueFlow::Value* val, const Token* tempTok);
    void errorReturnReference(const Token* tok, ErrorPath errorPath, bool inconclusive);
    void errorDanglingReference(const Token *tok, const Variable *var, ErrorPath errorPath);
    void errorDanglingTempReference(const Token* tok, ErrorPath errorPath, bool inconclusive);
    void errorReturnTempReference(const Token* tok, ErrorPath errorPath, bool inconclusive);
    void errorInvalidDeallocation(const Token *tok, const ValueFlow::Value *val);
    void errorUselessAssignmentArg(const Token *tok);
    void errorUselessAssignmentPtrArg(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override {
        ErrorPath errorPath;
        CheckAutoVariables c(nullptr,settings,errorLogger);
        c.errorAutoVariableAssignment(nullptr, false);
        c.errorReturnReference(nullptr, errorPath, false);
        c.errorDanglingReference(nullptr, nullptr, errorPath);
        c.errorReturnTempReference(nullptr, errorPath, false);
        c.errorDanglingTempReference(nullptr, errorPath, false);
        c.errorInvalidDeallocation(nullptr, nullptr);
        c.errorUselessAssignmentArg(nullptr);
        c.errorUselessAssignmentPtrArg(nullptr);
        c.errorReturnDanglingLifetime(nullptr, nullptr);
        c.errorInvalidLifetime(nullptr, nullptr);
        c.errorDanglngLifetime(nullptr, nullptr);
        c.errorDanglingTemporaryLifetime(nullptr, nullptr, nullptr);
    }

    static std::string myName() {
        return "Auto Variables";
    }

    std::string classInfo() const override {
        return "A pointer to a variable is only valid as long as the variable is in scope.\n"
               "Check:\n"
               "- returning a pointer to auto or temporary variable\n"
               "- assigning address of an variable to an effective parameter of a function\n"
               "- returning reference to local/temporary variable\n"
               "- returning address of function parameter\n"
               "- suspicious assignment of pointer argument\n"
               "- useless assignment of function argument\n";
    }

    /** returns true if tokvalue has already been diagnosed */
    bool diag(const Token* tokvalue);

    std::set<const Token*> mDiagDanglingTemp;
};
/// @}
//---------------------------------------------------------------------------
#endif // checkautovariablesH

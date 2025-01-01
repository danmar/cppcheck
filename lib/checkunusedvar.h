/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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
#ifndef checkunusedvarH
#define checkunusedvarH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

#include <list>
#include <map>
#include <string>

class ErrorLogger;
class Scope;
class Settings;
class Token;
class Type;
class Variables;
class Variable;
class Function;
class Tokenizer;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckUnusedVar : public Check {
    friend class TestUnusedVar;

public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckUnusedVar() : Check(myName()) {}

private:
    /** @brief This constructor is used when running checks. */
    CheckUnusedVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief %Check for unused function variables */
    void checkFunctionVariableUsage_iterateScopes(const Scope* scope, Variables& variables);
    void checkFunctionVariableUsage();

    /** @brief %Check that all struct members are used */
    void checkStructMemberUsage();

    bool isRecordTypeWithoutSideEffects(const Type* type);
    bool isVariableWithoutSideEffects(const Variable& var, const Type* type = nullptr);
    bool isEmptyType(const Type* type);
    bool isFunctionWithoutSideEffects(const Function& func, const Token* functionUsageToken,
                                      std::list<const Function*> checkedFuncs);

    // Error messages..
    void unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname, const std::string& prefix = "struct");
    void unusedVariableError(const Token *tok, const std::string &varname);
    void allocatedButUnusedVariableError(const Token *tok, const std::string &varname);
    void unreadVariableError(const Token *tok, const std::string &varname, bool modified);
    void unassignedVariableError(const Token *tok, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static std::string myName() {
        return "UnusedVar";
    }

    std::string classInfo() const override {
        return "UnusedVar checks\n"

               // style
               "- unused variable\n"
               "- allocated but unused variable\n"
               "- unread variable\n"
               "- unassigned variable\n"
               "- unused struct member\n";
    }

    std::map<const Type *,bool> mIsRecordTypeWithoutSideEffectsMap;

    std::map<const Type *,bool> mIsEmptyTypeMap;

};
/// @}
//---------------------------------------------------------------------------
#endif // checkunusedvarH

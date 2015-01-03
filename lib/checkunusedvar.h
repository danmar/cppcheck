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
#ifndef checkunusedvarH
#define checkunusedvarH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

#include <map>

class Type;
class Scope;
class Variables;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckUnusedVar : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckUnusedVar() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckUnusedVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckUnusedVar checkUnusedVar(tokenizer, settings, errorLogger);

        // Coding style checks
        checkUnusedVar.checkStructMemberUsage();
        checkUnusedVar.checkFunctionVariableUsage();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        (void)tokenizer;
        (void)settings;
        (void)errorLogger;
    }

    /** @brief %Check for unused function variables */
    void checkFunctionVariableUsage_iterateScopes(const Scope* const scope, Variables& variables, bool insideLoop);
    void checkVariableUsage(const Scope* const scope, const Token* start, Variables& variables);
    void checkFunctionVariableUsage();

    /** @brief %Check that all struct members are used */
    void checkStructMemberUsage();

private:
    bool isRecordTypeWithoutSideEffects(const Type* type);
    bool isEmptyType(const Type* type);

    // Error messages..
    void unusedStructMemberError(const Token *tok, const std::string &structname, const std::string &varname);
    void unusedVariableError(const Token *tok, const std::string &varname);
    void allocatedButUnusedVariableError(const Token *tok, const std::string &varname);
    void unreadVariableError(const Token *tok, const std::string &varname);
    void unassignedVariableError(const Token *tok, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckUnusedVar c(0, settings, errorLogger);

        // style/warning
        c.unusedVariableError(0, "varname");
        c.allocatedButUnusedVariableError(0, "varname");
        c.unreadVariableError(0, "varname");
        c.unassignedVariableError(0, "varname");
        c.unusedStructMemberError(0, "structname", "variable");
    }

    static std::string myName() {
        return "UnusedVar";
    }

    std::string classInfo() const {
        return "UnusedVar checks\n"

               // style
               "- unused variable\n"
               "- allocated but unused variable\n"
               "- unred variable\n"
               "- unassigned variable\n"
               "- unused struct member\n";
    }

    std::map<const Type *,bool> isRecordTypeWithoutSideEffectsMap;

    std::map<const Type *,bool> isEmptyTypeMap;

};
/// @}
//---------------------------------------------------------------------------
#endif // checkunusedvarH

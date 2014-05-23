/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
#ifndef checkuninitvarH
#define checkuninitvarH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

class Scope;
class Variable;

/// @addtogroup Checks
/// @{


/** @brief Checking for uninitialized variables */

class CPPCHECKLIB CheckUninitVar : public Check {
public:
    /** @brief This constructor is used when registering the CheckUninitVar */
    CheckUninitVar() : Check(myName()), testrunner(false) {
    }

    /** @brief This constructor is used when running checks. */
    CheckUninitVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger), testrunner(false) {
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckUninitVar checkUninitVar(tokenizer, settings, errorLogger);
        checkUninitVar.executionPaths();
        checkUninitVar.check();
    }

    /** Check for uninitialized variables */
    void check();
    void checkScope(const Scope* scope);
    void checkStruct(const Scope* scope, const Token *tok, const Variable &structvar);
    bool checkScopeForVariable(const Scope* scope, const Token *tok, const Variable& var, bool * const possibleInit, bool * const noreturn, bool * const alloc, const std::string &membervar);
    bool checkIfForWhileHead(const Token *startparentheses, const Variable& var, bool suppressErrors, bool isuninit, bool alloc, const std::string &membervar);
    bool checkLoopBody(const Token *tok, const Variable& var, const bool alloc, const std::string &membervar, const bool suppressErrors);
    void checkRhs(const Token *tok, const Variable &var, bool alloc, const std::string &membervar);
    static bool isVariableUsage(const Token *vartok, bool ispointer, bool alloc, bool cpp);
    static bool isMemberVariableAssignment(const Token *tok, const std::string &membervar);
    bool isMemberVariableUsage(const Token *tok, bool isPointer, bool alloc, const std::string &membervar) const;

    /**
     * @brief Uninitialized variables: analyse functions to see how they work with uninitialized variables
     * @param tokens [in] the token list
     * @param func [out] names of functions that don't handle uninitialized variables well. the function names are added to the set. No clearing is made.
     */
    void analyse(const Token * tokens, std::set<std::string> &func) const;

    /** Save analysis results */
    void saveAnalysisData(const std::set<std::string> &data) const;

    /** @brief new type of check: check execution paths */
    void executionPaths();

    void uninitstringError(const Token *tok, const std::string &varname, bool strncpy_);
    void uninitdataError(const Token *tok, const std::string &varname);
    void uninitvarError(const Token *tok, const std::string &varname);
    void uninitStructMemberError(const Token *tok, const std::string &membername);

    /** testrunner: (don't abort() when assertion fails, just write error message) */
    bool testrunner;

private:
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckUninitVar c(0, settings, errorLogger);

        // error
        c.uninitstringError(0, "varname", true);
        c.uninitdataError(0, "varname");
        c.uninitvarError(0, "varname");
        c.uninitStructMemberError(0, "a.b");
    }

    static std::string myName() {
        return "Uninitialized variables";
    }

    std::string classInfo() const {
        return "Uninitialized variables\n"
               "* using uninitialized variables and data\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkuninitvarH

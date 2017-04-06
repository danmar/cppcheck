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
    CheckUninitVar() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckUninitVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckUninitVar checkUninitVar(tokenizer, settings, errorLogger);
        checkUninitVar.check();
        checkUninitVar.deadPointer();
    }

    /** Check for uninitialized variables */
    void check();
    void checkScope(const Scope* scope, const std::set<std::string> &arrayTypeDefs);
    void checkStruct(const Token *tok, const Variable &structvar);
    enum Alloc { NO_ALLOC, NO_CTOR_CALL, CTOR_CALL, ARRAY };
    bool checkScopeForVariable(const Token *tok, const Variable& var, bool* const possibleInit, bool* const noreturn, Alloc* const alloc, const std::string &membervar);
    bool checkIfForWhileHead(const Token *startparentheses, const Variable& var, bool suppressErrors, bool isuninit, Alloc alloc, const std::string &membervar);
    bool checkLoopBody(const Token *tok, const Variable& var, const Alloc alloc, const std::string &membervar, const bool suppressErrors);
    void checkRhs(const Token *tok, const Variable &var, Alloc alloc, unsigned int number_of_if, const std::string &membervar);
    bool isVariableUsage(const Token *vartok, bool pointer, Alloc alloc) const;
    int isFunctionParUsage(const Token *vartok, bool pointer, Alloc alloc) const;
    bool isMemberVariableAssignment(const Token *tok, const std::string &membervar) const;
    bool isMemberVariableUsage(const Token *tok, bool isPointer, Alloc alloc, const std::string &membervar) const;

    /** ValueFlow-based checking for dead pointer usage */
    void deadPointer();
    void deadPointerError(const Token *pointer, const Token *alias);

    /* data for multifile checking */
    class MyFileInfo : public Check::FileInfo {
    public:
        /* functions that must have initialized data */
        std::set<std::string>  uvarFunctions;

        /* functions calls with uninitialized data */
        std::set<std::string>  functionCalls;
    };

    void uninitstringError(const Token *tok, const std::string &varname, bool strncpy_);
    void uninitdataError(const Token *tok, const std::string &varname);
    void uninitvarError(const Token *tok, const std::string &varname);
    void uninitvarError(const Token *tok, const std::string &varname, Alloc alloc) {
        if (alloc == NO_CTOR_CALL || alloc == CTOR_CALL)
            uninitdataError(tok, varname);
        else
            uninitvarError(tok, varname);
    }
    void uninitStructMemberError(const Token *tok, const std::string &membername);

private:
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckUninitVar c(nullptr, settings, errorLogger);

        // error
        c.uninitstringError(nullptr, "varname", true);
        c.uninitdataError(nullptr, "varname");
        c.uninitvarError(nullptr, "varname");
        c.uninitStructMemberError(nullptr, "a.b");
        c.deadPointerError(nullptr, nullptr);
    }

    static std::string myName() {
        return "Uninitialized variables";
    }

    std::string classInfo() const {
        return "Uninitialized variables\n"
               "- using uninitialized local variables\n"
               "- using allocated data before it has been initialized\n"
               "- using dead pointer\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkuninitvarH

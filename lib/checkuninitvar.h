/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2017 Cppcheck team.
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

#include "check.h"
#include "config.h"

#include <set>
#include <string>

class ErrorLogger;
class Scope;
class Settings;
class Token;
class Tokenizer;
class Variable;


struct VariableValue {
    explicit VariableValue(MathLib::bigint val = 0) : value(val), notEqual(false) {}
    MathLib::bigint value;
    bool notEqual;
};

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
        checkUninitVar.valueFlowUninit();
    }

    /** Check for uninitialized variables */
    void check();
    void checkScope(const Scope* scope, const std::set<std::string> &arrayTypeDefs);
    void checkStruct(const Token *tok, const Variable &structvar);
    enum Alloc { NO_ALLOC, NO_CTOR_CALL, CTOR_CALL, ARRAY };
    bool checkScopeForVariable(const Token *tok, const Variable& var, bool* const possibleInit, bool* const noreturn, Alloc* const alloc, const std::string &membervar, std::map<unsigned int, VariableValue> variableValue);
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

    /** ValueFlow-based checking for uninitialized variables */
    void valueFlowUninit();

    /* data for multifile checking */
    class MyFileInfo : public Check::FileInfo {
    public:
        struct FunctionArg {
            FunctionArg(const std::string &id_, const std::string &functionName_, unsigned int argnr_, const std::string &fileName, unsigned int linenr, const std::string &varname)
                : id(id_),
                  functionName(functionName_),
                  argnr(argnr_),
                  argnr2(0),
                  variableName(varname) {
                location.fileName = fileName;
                location.linenr   = linenr;
            }

            FunctionArg(const Tokenizer *tokenizer, const Scope *scope, unsigned int argnr_, const Token *tok);

            std::string id;
            std::string id2;
            std::string functionName;
            unsigned int argnr;
            unsigned int argnr2;
            std::string variableName;
            struct {
                std::string fileName;
                unsigned int linenr;
            } location;
        };

        /** uninitialized function args */
        std::list<FunctionArg> uninitialized;

        /** function arguments that data are unconditionally read */
        std::list<FunctionArg> readData;

        /** null pointer function args */
        std::list<FunctionArg> nullPointer;

        /** function arguments that are unconditionally dereferenced */
        std::list<FunctionArg> dereferenced;

        /** function calls other function.. */
        std::list<FunctionArg> nestedCall;

        /** Convert MyFileInfo data into xml string */
        std::string toString() const;
    };

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const;

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger);

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
    Check::FileInfo *getFileInfo() const;
    bool isUnsafeFunction(const Scope *scope, int argnr, const Token **tok) const;

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

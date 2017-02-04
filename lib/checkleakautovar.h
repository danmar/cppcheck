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
#ifndef checkleakautovarH
#define checkleakautovarH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

#include <map>
#include <set>


class CPPCHECKLIB VarInfo {
public:
    enum AllocStatus { DEALLOC = -1, NOALLOC = 0, ALLOC = 1 };
    struct AllocInfo {
        AllocStatus status;
        int type;
        AllocInfo(int type_ = 0, AllocStatus status_ = NOALLOC) : status(status_), type(type_) {}
    };
    std::map<unsigned int, AllocInfo> alloctype;
    std::map<unsigned int, std::string> possibleUsage;
    std::set<unsigned int> conditionalAlloc;
    std::set<unsigned int> referenced;

    void clear() {
        alloctype.clear();
        possibleUsage.clear();
        conditionalAlloc.clear();
        referenced.clear();
    }

    void erase(unsigned int varid) {
        alloctype.erase(varid);
        possibleUsage.erase(varid);
        conditionalAlloc.erase(varid);
        referenced.erase(varid);
    }

    void swap(VarInfo &other) {
        alloctype.swap(other.alloctype);
        possibleUsage.swap(other.possibleUsage);
        conditionalAlloc.swap(other.conditionalAlloc);
        referenced.swap(other.referenced);
    }

    /** set possible usage for all variables */
    void possibleUsageAll(const std::string &functionName);

    void print();
};


/// @addtogroup Checks
/// @{

/**
 * @brief Check for leaks
 */

class CPPCHECKLIB CheckLeakAutoVar : public Check {
public:
    /** This constructor is used when registering the CheckLeakAutoVar */
    CheckLeakAutoVar() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckLeakAutoVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckLeakAutoVar checkLeakAutoVar(tokenizer, settings, errorLogger);
        checkLeakAutoVar.check();
    }

private:

    /** check for leaks in all scopes */
    void check();

    /** check for leaks in a function scope */
    void checkScope(const Token * const startToken,
                    VarInfo *varInfo,
                    std::set<unsigned int> notzero);

    /** parse function call */
    void functionCall(const Token *tok, VarInfo *varInfo, const VarInfo::AllocInfo& allocation, const Library::AllocFunc* af);

    /** parse changes in allocation status */
    void changeAllocStatus(VarInfo *varInfo, const VarInfo::AllocInfo& allocation, const Token* tok, const Token* arg);

    /** return. either "return" or end of variable scope is seen */
    void ret(const Token *tok, const VarInfo &varInfo);

    /** if variable is allocated then there is a leak */
    void leakIfAllocated(const Token *vartok, const VarInfo &varInfo);

    void leakError(const Token* tok, const std::string &varname, int type);
    void mismatchError(const Token* tok, const std::string &varname);
    void deallocUseError(const Token *tok, const std::string &varname);
    void deallocReturnError(const Token *tok, const std::string &varname);
    void doubleFreeError(const Token *tok, const std::string &varname, int type);

    /** message: user configuration is needed to complete analysis */
    void configurationInfo(const Token* tok, const std::string &functionName);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckLeakAutoVar c(nullptr, settings, errorLogger);
        c.deallocReturnError(nullptr, "p");
        c.configurationInfo(nullptr, "f");  // user configuration is needed to complete analysis
        c.doubleFreeError(nullptr, "varname", 0);
    }

    static std::string myName() {
        return "Leaks (auto variables)";
    }

    std::string classInfo() const {
        return "Detect when a auto variable is allocated but not deallocated or deallocated twice.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkleakautovarH

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#ifndef checkmemoryleakH
#define checkmemoryleakH
//---------------------------------------------------------------------------

/**
 * @file
 *
 * %Check for memory leaks
 *
 * The checking is split up into three specialized classes.
 * - CheckMemoryLeakInFunction can detect when a function variable is allocated but not deallocated properly.
 * - CheckMemoryLeakInClass can detect when a class variable is allocated but not deallocated properly.
 * - CheckMemoryLeakStructMember checks allocation/deallocation of structs and struct members
 */

#include "check.h"
#include "config.h"
#include "errorlogger.h"
#include "tokenize.h"

#include <list>
#include <string>

class Function;
class Scope;
class Settings;
class SymbolDatabase;
class Token;
class Variable;

/// @addtogroup Core
/// @{

/** @brief Base class for memory leaks checking */
class CPPCHECKLIB CheckMemoryLeak {
private:
    /** For access to the tokens */
    const Tokenizer * const mTokenizer_;

    /** ErrorLogger used to report errors */
    ErrorLogger * const mErrorLogger_;

    /** Enabled standards */
    const Settings * const mSettings_;

    /**
     * Report error. Similar with the function Check::reportError
     * @param tok the token where the error occurs
     * @param severity the severity of the bug
     * @param id type of message
     * @param msg text
     * @param cwe cwe number
     */
    void reportErr(const Token *tok, Severity::SeverityType severity, const std::string &id, const std::string &msg, const CWE &cwe) const;

    /**
     * Report error. Similar with the function Check::reportError
     * @param callstack callstack of error
     * @param severity the severity of the bug
     * @param id type of message
     * @param msg text
     * @param cwe cwe number
     */
    void reportErr(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg, const CWE &cwe) const;

public:
    CheckMemoryLeak() = delete;
    CheckMemoryLeak(const CheckMemoryLeak &) = delete;
    void operator=(const CheckMemoryLeak &) = delete;

    CheckMemoryLeak(const Tokenizer *t, ErrorLogger *e, const Settings *s)
        : mTokenizer_(t), mErrorLogger_(e), mSettings_(s) {
    }

    /** @brief What type of allocation are used.. the "Many" means that several types of allocation and deallocation are used */
    enum AllocType { No, Malloc, New, NewArray, File, Fd, Pipe, OtherMem, OtherRes, Many };

    void memoryLeak(const Token *tok, const std::string &varname, AllocType alloctype) const;

    /**
     * @brief Get type of deallocation at given position
     * @param tok position
     * @param varid variable id
     * @return type of deallocation
     */
    AllocType getDeallocationType(const Token *tok, unsigned int varid) const;

    /**
     * @brief Get type of allocation at given position
     */
    AllocType getAllocationType(const Token *tok2, unsigned int varid, std::list<const Function*> *callstack = nullptr) const;

    /**
     * @brief Get type of reallocation at given position
     */
    static AllocType getReallocationType(const Token *tok2, unsigned int varid);

    /**
     * @brief Is a typename the name of a class?
     * @param tok type token
     * @param varid variable id
     * @return true if the type name is the name of a class
     */
    bool isclass(const Token *tok, unsigned int varid) const;

    /**
     * Report that there is a memory leak (new/malloc/etc)
     * @param tok token where memory is leaked
     * @param varname name of variable
     */
    void memleakError(const Token *tok, const std::string &varname) const;

    /**
     * Report that there is a resource leak (fopen/popen/etc)
     * @param tok token where resource is leaked
     * @param varname name of variable
     */
    void resourceLeakError(const Token *tok, const std::string &varname) const;

    /**
     * @brief Report error: deallocating a deallocated pointer
     * @param tok token where error occurs
     * @param varname name of variable
     */
    void deallocDeallocError(const Token *tok, const std::string &varname) const;
    void deallocuseError(const Token *tok, const std::string &varname) const;
    void mismatchSizeError(const Token *tok, const std::string &sz) const;
    void mismatchAllocDealloc(const std::list<const Token *> &callstack, const std::string &varname) const;
    void memleakUponReallocFailureError(const Token *tok, const std::string &varname) const;

    /** What type of allocated memory does the given function return? */
    AllocType functionReturnType(const Function* func, std::list<const Function*> *callstack = nullptr) const;

    /** Function allocates pointed-to argument (a la asprintf)? */
    const char *functionArgAlloc(const Function *func, unsigned int targetpar, AllocType &allocType) const;
};

/// @}



/// @addtogroup Checks
/// @{


/**
 * @brief %CheckMemoryLeakInFunction detects when a function variable is allocated but not deallocated properly.
 *
 * The checking is done by looking at each function variable separately. By repeating these 4 steps over and over:
 * -# locate a function variable
 * -# create a simple token list that describes the usage of the function variable.
 * -# simplify the token list.
 * -# finally, check if the simplified token list contain any leaks.
 */

class CPPCHECKLIB CheckMemoryLeakInFunction : private Check, public CheckMemoryLeak {
public:
    /** @brief This constructor is used when registering this class */
    CheckMemoryLeakInFunction() : Check(myName()), CheckMemoryLeak(nullptr, nullptr, nullptr) {
    }

    /** @brief This constructor is used when running checks */
    CheckMemoryLeakInFunction(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger), CheckMemoryLeak(tokenizer, errorLogger, settings) {
    }

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) OVERRIDE {
        CheckMemoryLeakInFunction checkMemoryLeak(tokenizer, settings, errorLogger);
        checkMemoryLeak.checkReallocUsage();
    }

    /** @brief Unit testing : testing the white list */
    static bool test_white_list(const std::string &funcname, const Settings *settings, bool cpp);

    /**
     * Checking for a memory leak caused by improper realloc usage.
     */
    void checkReallocUsage();

private:
    /** Report all possible errors (for the --errorlist) */
    void getErrorMessages(ErrorLogger *e, const Settings *settings) const OVERRIDE {
        CheckMemoryLeakInFunction c(nullptr, settings, e);

        c.memleakError(nullptr, "varname");
        c.resourceLeakError(nullptr, "varname");

        c.deallocDeallocError(nullptr, "varname");
        c.deallocuseError(nullptr, "varname");
        c.mismatchSizeError(nullptr, "sz");
        const std::list<const Token *> callstack;
        c.mismatchAllocDealloc(callstack, "varname");
        c.memleakUponReallocFailureError(nullptr, "varname");
    }

    /**
     * Get name of class (--doc)
     * @return name of class
     */
    static std::string myName() {
        return "Memory leaks (function variables)";
    }

    /**
     * Get class information (--doc)
     * @return Wiki formatted information about this class
     */
    std::string classInfo() const OVERRIDE {
        return "Is there any allocated memory when a function goes out of scope\n";
    }
};



/**
 * @brief %Check class variables, variables that are allocated in the constructor should be deallocated in the destructor
 */

class CPPCHECKLIB CheckMemoryLeakInClass : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakInClass() : Check(myName()), CheckMemoryLeak(nullptr, nullptr, nullptr) {
    }

    CheckMemoryLeakInClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger), CheckMemoryLeak(tokenizer, errorLogger, settings) {
    }

    void runChecks(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog) OVERRIDE {
        if (!tokenizr->isCPP())
            return;

        CheckMemoryLeakInClass checkMemoryLeak(tokenizr, settings, errLog);
        checkMemoryLeak.check();
    }

    void check();

private:
    void variable(const Scope *scope, const Token *tokVarname);

    /** Public functions: possible double-allocation */
    void checkPublicFunctions(const Scope *scope, const Token *classtok);
    void publicAllocationError(const Token *tok, const std::string &varname);

    void unsafeClassError(const Token *tok, const std::string &classname, const std::string &varname);

    void getErrorMessages(ErrorLogger *e, const Settings *settings) const OVERRIDE {
        CheckMemoryLeakInClass c(nullptr, settings, e);
        c.publicAllocationError(nullptr, "varname");
        c.unsafeClassError(nullptr, "class", "class::varname");
    }

    static std::string myName() {
        return "Memory leaks (class variables)";
    }

    std::string classInfo() const OVERRIDE {
        return "If the constructor allocate memory then the destructor must deallocate it.\n";
    }
};



/** @brief detect simple memory leaks for struct members */

class CPPCHECKLIB CheckMemoryLeakStructMember : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakStructMember() : Check(myName()), CheckMemoryLeak(nullptr, nullptr, nullptr) {
    }

    CheckMemoryLeakStructMember(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger), CheckMemoryLeak(tokenizer, errorLogger, settings) {
    }

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) OVERRIDE {
        CheckMemoryLeakStructMember checkMemoryLeak(tokenizer, settings, errorLogger);
        checkMemoryLeak.check();
    }

    void check();

private:

    /** Is local variable allocated with malloc? */
    static bool isMalloc(const Variable *variable);

    void checkStructVariable(const Variable * const variable);

    void getErrorMessages(ErrorLogger * /*errorLogger*/, const Settings * /*settings*/) const OVERRIDE {
    }

    static std::string myName() {
        return "Memory leaks (struct members)";
    }

    std::string classInfo() const OVERRIDE {
        return "Don't forget to deallocate struct members\n";
    }
};



/** @brief detect simple memory leaks (address not taken) */

class CPPCHECKLIB CheckMemoryLeakNoVar : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakNoVar() : Check(myName()), CheckMemoryLeak(nullptr, nullptr, nullptr) {
    }

    CheckMemoryLeakNoVar(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger), CheckMemoryLeak(tokenizer, errorLogger, settings) {
    }

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) OVERRIDE {
        CheckMemoryLeakNoVar checkMemoryLeak(tokenizer, settings, errorLogger);
        checkMemoryLeak.check();
    }

    void check();

private:
    /**
     * @brief %Check if a call to an allocation function like malloc() is made and its return value is not assigned.
     * @param scope     The scope of the function to check.
     */
    void checkForUnusedReturnValue(const Scope *scope);

    /**
     * @brief %Check if an exception could cause a leak in an argument constructed with shared_ptr/unique_ptr.
     * @param scope     The scope of the function to check.
     */
    void checkForUnsafeArgAlloc(const Scope *scope);

    void functionCallLeak(const Token *loc, const std::string &alloc, const std::string &functionCall);
    void returnValueNotUsedError(const Token* tok, const std::string &alloc);
    void unsafeArgAllocError(const Token *tok, const std::string &funcName, const std::string &ptrType, const std::string &objType);

    void getErrorMessages(ErrorLogger *e, const Settings *settings) const OVERRIDE {
        CheckMemoryLeakNoVar c(nullptr, settings, e);

        c.functionCallLeak(nullptr, "funcName", "funcName");
        c.returnValueNotUsedError(nullptr, "funcName");
        c.unsafeArgAllocError(nullptr, "funcName", "shared_ptr", "int");
    }

    static std::string myName() {
        return "Memory leaks (address not taken)";
    }

    std::string classInfo() const OVERRIDE {
        return "Not taking the address to allocated memory\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkmemoryleakH

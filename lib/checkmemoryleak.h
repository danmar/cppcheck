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

#include "config.h"
#include "check.h"

#include <list>
#include <string>

class Scope;
class Function;
class Variable;

/// @addtogroup Core
/// @{

/** @brief Base class for memory leaks checking */
class CPPCHECKLIB CheckMemoryLeak {
protected:
    /** For access to the tokens */
    const Tokenizer * const tokenizer;

private:
    /** ErrorLogger used to report errors */
    ErrorLogger * const errorLogger;

    /** Enabled standards */
    const Settings * const settings1;

    /** Disable the default constructors */
    CheckMemoryLeak();

    /** Disable the default constructors */
    CheckMemoryLeak(const CheckMemoryLeak &);

    /** disable assignment operator */
    void operator=(const CheckMemoryLeak &);

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
    CheckMemoryLeak(const Tokenizer *t, ErrorLogger *e, const Settings *s)
        : tokenizer(t), errorLogger(e), settings1(s) {
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
    CheckMemoryLeakInFunction() : Check(myName()), CheckMemoryLeak(0, 0, 0), symbolDatabase(nullptr) {
    }

    /** @brief This constructor is used when running checks */
    CheckMemoryLeakInFunction(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog)
        : Check(myName(), tokenizr, settings, errLog), CheckMemoryLeak(tokenizr, errLog, settings) {
        // get the symbol database
        if (tokenizr)
            symbolDatabase = tokenizr->getSymbolDatabase();
        else
            symbolDatabase = 0;
    }

    /** @brief run all simplified checks */
    void runSimplifiedChecks(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog) {
        CheckMemoryLeakInFunction checkMemoryLeak(tokenizr, settings, errLog);
        checkMemoryLeak.checkReallocUsage();
        checkMemoryLeak.check();
    }

    /** @brief Unit testing : testing the white list */
    static bool test_white_list(const std::string &funcname, const Settings *settings, bool cpp);

    /** @brief Perform checking */
    void check();

    /**
     * Checking for a memory leak caused by improper realloc usage.
     */
    void checkReallocUsage();

    /**
     * Inspect a function call. the call_func and getcode are recursive
     * @param tok          token where the function call occurs
     * @param callstack    callstack
     * @param varid        variable id to check
     * @param alloctype    if memory is allocated, this indicates the type of allocation
     * @param dealloctype  if memory is deallocated, this indicates the type of deallocation
     * @param allocpar     if function allocates varid parameter
     * @param sz           not used by call_func - see getcode
     * @return These are the possible return values:
     * - NULL : no significant code
     * - "recursive" : recursive function
     * - "alloc" : the function returns allocated memory
     * - "dealloc" : the function deallocates the variable
     * - "dealloc_"
     * - "use" : the variable is used (unknown usage of the variable => the checking bails out)
     * - "callfunc" : a function call with unknown side effects
     * - "&use"
     */
    const char * call_func(const Token *tok, std::list<const Token *> callstack, const unsigned int varid, AllocType &alloctype, AllocType &dealloctype, bool &allocpar, unsigned int sz);

    /**
     * Extract a new tokens list that is easier to parse than the "_tokenizer->tokens()", the
     * extracted tokens list describes how the given variable is used.
     * The getcode and call_func are recursive
     * @param tok start parse token
     * @param callstack callstack
     * @param varid variable id
     * @param alloctype keep track of what type of allocation is used
     * @param dealloctype keeps track of what type of deallocation is used
     * @param classmember should be set if the inspected function is a class member
     * @param sz size of type, used to check for mismatching size of allocation. for example "int *a;" => the sz is "sizeof(int)"
     * @return Newly allocated token array. Caller needs to release reserved
     * memory by calling TokenList::deleteTokens(returnValue);
     * Returned tokens:
     * - "alloc" : the variable is allocated
     * - "assign" : the variable is assigned a new value
     * - "break" : corresponds to "break"
     * - "callfunc" : a function call with unknown side effects
     * - "continue" : corresponds to "continue"
     * - "dealloc" : the variable is deallocated
     * - "goto" : corresponds to a "goto"
     * - "if" : there is an "if"
     * - "if(var)" : corresponds with "if ( var != 0 )"
     * - "if(!var)" : corresponds with "if ( var == 0 )"
     * - "ifv" : the variable is used in some way in a "if"
     * - "loop" : corresponds to either a "for" or a "while"
     * - "realloc" : the variable is reallocated
     * - "return" : corresponds to a "return"
     * - "use" : unknown usage -> bail out checking of this execution path
     * - "&use" : the address of the variable is taken
     * - "::use" : calling member function of class
     * - "use_" : content of variable is accessed (used to warn access after dealloc)
     */
    Token *getcode(const Token *tok, std::list<const Token *> callstack, const unsigned int varid, AllocType &alloctype, AllocType &dealloctype, bool classmember, unsigned int sz);

    /**
     * Simplify code e.g. by replacing empty "{ }" with ";"
     * @param tok first token. The tokens list can be modified.
     */
    void simplifycode(Token *tok) const;

    static const Token *findleak(const Token *tokens);

    /**
     * Checking the variable varname
     * @param startTok start token
     * @param varname name of variable (for error messages)
     * @param varid variable id
     * @param classmember is the scope inside a class member function
     * @param sz size of type.. if the variable is a "int *" then sz should be "sizeof(int)"
     */
    void checkScope(const Token *startTok, const std::string &varname, unsigned int varid, bool classmember, unsigned int sz);

private:
    /** Report all possible errors (for the --errorlist) */
    void getErrorMessages(ErrorLogger *e, const Settings *settings) const {
        CheckMemoryLeakInFunction c(nullptr, settings, e);

        c.memleakError(nullptr, "varname");
        c.resourceLeakError(nullptr, "varname");

        c.deallocDeallocError(nullptr, "varname");
        c.deallocuseError(nullptr, "varname");
        c.mismatchSizeError(nullptr, "sz");
        std::list<const Token *> callstack;
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
    std::string classInfo() const {
        return "Is there any allocated memory when a function goes out of scope\n";
    }

    const SymbolDatabase *symbolDatabase;
};



/**
 * @brief %Check class variables, variables that are allocated in the constructor should be deallocated in the destructor
 */

class CPPCHECKLIB CheckMemoryLeakInClass : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakInClass() : Check(myName()), CheckMemoryLeak(0, 0, 0) {
    }

    CheckMemoryLeakInClass(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog)
        : Check(myName(), tokenizr, settings, errLog), CheckMemoryLeak(tokenizr, errLog, settings) {
    }

    void runSimplifiedChecks(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog) {
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

    void getErrorMessages(ErrorLogger *e, const Settings *settings) const {
        CheckMemoryLeakInClass c(0, settings, e);
        c.publicAllocationError(0, "varname");
        c.unsafeClassError(0, "class", "class::varname");
    }

    static std::string myName() {
        return "Memory leaks (class variables)";
    }

    std::string classInfo() const {
        return "If the constructor allocate memory then the destructor must deallocate it.\n";
    }
};



/** @brief detect simple memory leaks for struct members */

class CPPCHECKLIB CheckMemoryLeakStructMember : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakStructMember() : Check(myName()), CheckMemoryLeak(0, 0, 0) {
    }

    CheckMemoryLeakStructMember(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog)
        : Check(myName(), tokenizr, settings, errLog), CheckMemoryLeak(tokenizr, errLog, settings) {
    }

    void runSimplifiedChecks(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog) {
        CheckMemoryLeakStructMember checkMemoryLeak(tokenizr, settings, errLog);
        checkMemoryLeak.check();
    }

    void check();

private:

    /** Is local variable allocated with malloc? */
    static bool isMalloc(const Variable *variable);

    void checkStructVariable(const Variable * const variable);

    void getErrorMessages(ErrorLogger * /*errorLogger*/, const Settings * /*settings*/) const {
    }

    static std::string myName() {
        return "Memory leaks (struct members)";
    }

    std::string classInfo() const {
        return "Don't forget to deallocate struct members\n";
    }
};



/** @brief detect simple memory leaks (address not taken) */

class CPPCHECKLIB CheckMemoryLeakNoVar : private Check, private CheckMemoryLeak {
public:
    CheckMemoryLeakNoVar() : Check(myName()), CheckMemoryLeak(0, 0, 0) {
    }

    CheckMemoryLeakNoVar(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog)
        : Check(myName(), tokenizr, settings, errLog), CheckMemoryLeak(tokenizr, errLog, settings) {
    }

    void runSimplifiedChecks(const Tokenizer *tokenizr, const Settings *settings, ErrorLogger *errLog) {
        CheckMemoryLeakNoVar checkMemoryLeak(tokenizr, settings, errLog);
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

    void getErrorMessages(ErrorLogger *e, const Settings *settings) const {
        CheckMemoryLeakNoVar c(nullptr, settings, e);

        c.functionCallLeak(nullptr, "funcName", "funcName");
        c.returnValueNotUsedError(nullptr, "funcName");
        c.unsafeArgAllocError(nullptr, "funcName", "shared_ptr", "int");
    }

    static std::string myName() {
        return "Memory leaks (address not taken)";
    }

    std::string classInfo() const {
        return "Not taking the address to allocated memory\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkmemoryleakH

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



//---------------------------------------------------------------------------
#ifndef checkmemoryleakH
#define checkmemoryleakH
//---------------------------------------------------------------------------

/**
 * Check for memory leaks
 *
 * The checking is split up into two specialized classes.
 * CheckMemoryLeakInFunction can detect when a function variable is allocated but not deallocated properly.
 * CheckMemoryLeakInClass can detect when a class variable is allocated but not deallocated properly.
 */

#include "check.h"

#include <list>
#include <string>
#include <vector>

class Token;

/** Base class for memory leaks checking */

class CheckMemoryLeak
{
public:
    CheckMemoryLeak() { }

    /** What type of allocation are used.. the "Many" means that several types of allocation and deallocation are used */
    enum AllocType { No, Malloc, gMalloc, New, NewArray, File, Pipe, Dir, Many };

    void MemoryLeak(const Token *tok, const char varname[], AllocType alloctype, bool all);
    void MismatchError(const Token *Tok1, const std::list<const Token *> &callstack, const char varname[]);
    AllocType GetDeallocationType(const Token *tok, const char *varnames[]);
    AllocType GetAllocationType(const Token *tok2) const;
    AllocType GetReallocationType(const Token *tok2);
    bool isclass(const Tokenizer *_tokenizer, const Token *typestr) const;

    void memleakError(const Token *tok, const std::string &varname);
    void memleakallError(const Token *tok, const std::string &varname);
    void resourceLeakError(const Token *tok, const std::string &varname);

    void deallocDeallocError(const Token *tok, const std::string &varname);
    void deallocuseError(const Token *tok, const std::string &varname);
    void mismatchSizeError(const Token *tok, const std::string &sz);
    void mismatchAllocDealloc(const std::list<const Token *> &callstack, const std::string &varname);

    // error message
    virtual void error(const Token *tok, const std::string &severity, const std::string &id, const std::string &msg) = 0;
    virtual void error(const std::list<const Token *> &callstack, const std::string &severity, const std::string &id, const std::string &msg) = 0;

    /** What type of allocated memory does the given function return? */
    AllocType functionReturnType(const Token *tok) const;
};



/**
 * Check function variables.
 *
 * The checking is done by looking at each function variable separately. By repeating these 4 steps over and over:
 * 1. locate a function variable
 * 2. create a simple token list that describes the usage of the function variable.
 * 3. simplify the token list.
 * 4. finally, check if the simplified token list contain any leaks.
 */

class CheckMemoryLeakInFunction : public CheckMemoryLeak, public Check
{
public:
    CheckMemoryLeakInFunction() : Check()
    { }

    CheckMemoryLeakInFunction(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckMemoryLeakInFunction checkMemoryLeak(tokenizer, settings, errorLogger);
        checkMemoryLeak.check();
    }

#ifndef UNIT_TESTING
private:
#endif
    void check();

private:

    bool MatchFunctionsThatReturnArg(const Token *tok, const std::string &varname);

    /**
     * Check if there is a "!var" match inside a condition
     * @param tok      first token to match
     * @param varnames the varname
     * @param endpar   if this is true the "!var" must be followed by ")"
     * @return true if match
     */
    bool notvar(const Token *tok, const char *varnames[], bool endpar = false);


    const char * call_func(const Token *tok, std::list<const Token *> callstack, const char *varnames[], AllocType &alloctype, AllocType &dealloctype, bool &all, unsigned int sz);

    /**
     * Extract a new tokens list that is easier to parse than the "tokens"
     * @param tok start parse token
     * @param callstack callstack
     * @param varname name of variable
     * @param alloctype
     * @param dealloctype
     * @return Newly allocated token array. Caller needs to release reserved
     * memory by calling Tokenizer::deleteTokens(returnValue);
     */
    Token *getcode(const Token *tok, std::list<const Token *> callstack, const char varname[], AllocType &alloctype, AllocType &dealloctype, bool classmember, bool &all, unsigned int sz);

    /**
     * Simplify code e.g. by replacing empty "{ }" with ";"
     * @param tok first token. The tokens list can be modified.
     */
    void simplifycode(Token *tok, bool &all);

    void checkScope(const Token *Tok1, const char varname[], bool classmember, unsigned int sz);

    void error(const Token *tok, const std::string &severity, const std::string &id, const std::string &msg)
    {
        reportError(tok, severity, id, msg);
    }

    void error(const std::list<const Token *> &callstack, const std::string &severity, const std::string &id, const std::string &msg)
    {
        reportError(callstack, severity, id, msg);
    }

    void getErrorMessages()
    { }

    std::string name() const
    {
        return "Memory leaks (function variables)";
    }

    std::string classInfo() const
    {
        return "Is there any allocated memory when a function goes out of scope";
    }
};



/**
 * Check class variables
 * variables that are allocated in the constructor should be deallocated in the destructor
 */

class CheckMemoryLeakInClass : public CheckMemoryLeak, public Check
{
public:
    CheckMemoryLeakInClass() : Check()
    { }

    CheckMemoryLeakInClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckMemoryLeakInClass checkMemoryLeak(tokenizer, settings, errorLogger);
        checkMemoryLeak.check();
    }

#ifndef UNIT_TESTING
private:
#endif
    void check();

private:
    void parseClass(const Token *tok1, std::vector<const char *> &classname);
    void variable(const char classname[], const Token *tokVarname);

    void error(const Token *tok, const std::string &severity, const std::string &id, const std::string &msg)
    {
        reportError(tok, severity, id, msg);
    }

    void error(const std::list<const Token *> &callstack, const std::string &severity, const std::string &id, const std::string &msg)
    {
        reportError(callstack, severity, id, msg);
    }

    void getErrorMessages()
    { }

    std::string name() const
    {
        return "Memory leaks (class variables)";
    }

    std::string classInfo() const
    {
        return "If the constructor allocate memory then the destructor must deallocate it.";
    }
};


//---------------------------------------------------------------------------
#endif

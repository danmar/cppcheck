/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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
#ifndef CheckMemoryLeakH
#define CheckMemoryLeakH
//---------------------------------------------------------------------------

/** \brief Check for memory leaks */

#include "tokenize.h"
#include "settings.h"
#include "errorlogger.h"
#include <list>
#include <vector>

class CheckMemoryLeakClass
{
public:
    CheckMemoryLeakClass(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger);
    ~CheckMemoryLeakClass();
    void CheckMemoryLeak();

private:

    enum AllocType { No, Malloc, gMalloc, New, NewArray, FOPEN, POPEN };

    // Extra allocation..
    class AllocFunc
    {
    public:
        const char *funcname;
        AllocType   alloctype;

        AllocFunc(const char f[], AllocType a)
        {
            funcname = f;
            alloctype = a;
        }
    };

    void CheckMemoryLeak_ClassMembers_Variable(const std::vector<const char *> &classname, const char varname[]);
    void CheckMemoryLeak_ClassMembers_ParseClass(const Token *tok1, std::vector<const char *> &classname);
    void CheckMemoryLeak_ClassMembers();
    void CheckMemoryLeak_InFunction();
    void CheckMemoryLeak_CheckScope(const Token *Tok1, const char varname[], bool classmember);

    /**
     * Simplify code e.g. by replacing empty "{ }" with ";"
     * @param tok first token. The tokens list can be modified.
     */
    void simplifycode(Token *tok);

    /**
     * Delete tokens between begin and end. E.g. if begin = 1
     * and end = 5, tokens 2,3 and 4 would be erased.
     *
     * @param begin Tokens after this will be erased.
     * @param end Tokens before this will be erased.
     */
    void erase(Token *begin, const Token *end);

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
    Token *getcode(const Token *tok, std::list<const Token *> callstack, const char varname[], AllocType &alloctype, AllocType &dealloctype, bool classmember, bool &all);

    /**
     * Check if there is a "!var" match inside a condition
     * @param tok      first token to match
     * @param varnames the varname
     * @param endpar   if this is true the "!var" must be followed by ")"
     * @return true if match
     */
    bool notvar(const Token *tok, const char *varnames[], bool endpar = false);

    bool MatchFunctionsThatReturnArg(const Token *tok, const std::string &varname);
    void MemoryLeak(const Token *tok, const char varname[], AllocType alloctype, bool all);
    void MismatchError(const Token *Tok1, const std::list<const Token *> &callstack, const char varname[]);
    const char * call_func(const Token *tok, std::list<const Token *> callstack, const char *varnames[], AllocType &alloctype, AllocType &dealloctype, bool &all);
    AllocType GetDeallocationType(const Token *tok, const char *varnames[]);
    AllocType GetAllocationType(const Token *tok2);
    AllocType GetReallocationType(const Token *tok2);
    bool isclass(const Token *typestr);

    const Tokenizer *_tokenizer;
    ErrorLogger *_errorLogger;
    const Settings _settings;
    std::list<AllocFunc> _listAllocFunc;

// Experimental functionality..
#ifdef UNIT_TESTING
public:
#endif
    Token *functionParameterCode(const Token *ftok, int parameter);
};

//---------------------------------------------------------------------------
#endif

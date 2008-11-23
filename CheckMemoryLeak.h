/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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
    CheckMemoryLeakClass( const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger );
    ~CheckMemoryLeakClass();
    void CheckMemoryLeak();

private:

    enum AllocType { No, Malloc, gMalloc, New, NewA, FOPEN, POPEN };

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

    void CheckMemoryLeak_ClassMembers_Variable( const std::vector<const char *> &classname, const char varname[] );
    void CheckMemoryLeak_ClassMembers_ParseClass( const TOKEN *tok1, std::vector<const char *> &classname );
    void CheckMemoryLeak_ClassMembers();
    void CheckMemoryLeak_InFunction();
    void CheckMemoryLeak_CheckScope( const TOKEN *Tok1, const char varname[] );
    void simplifycode(TOKEN *tok);
    void erase(TOKEN *begin, const TOKEN *end);

    TOKEN *getcode(const TOKEN *tok, std::list<const TOKEN *> callstack, const char varname[], AllocType &alloctype, AllocType &dealloctype);
    bool notvar(const TOKEN *tok, const char *varnames[]);
    void instoken(TOKEN *tok, const char str[]);
    void MemoryLeak( const TOKEN *tok, const char varname[] );
    void MismatchError( const TOKEN *Tok1, const std::list<const TOKEN *> &callstack, const char varname[] );
    const char * call_func( const TOKEN *tok, std::list<const TOKEN *> callstack, const char *varnames[], AllocType &alloctype, AllocType &dealloctype );
    AllocType GetDeallocationType( const TOKEN *tok, const char *varnames[]);
    AllocType GetAllocationType( const TOKEN *tok2 );
    bool isclass( const std::string &typestr );

    const Tokenizer *_tokenizer;
    ErrorLogger *_errorLogger;
    Settings _settings;
    std::list<AllocFunc> _listAllocFunc;
};

//---------------------------------------------------------------------------
#endif

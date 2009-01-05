/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
#ifndef CheckBufferOverrunH
#define CheckBufferOverrunH
//---------------------------------------------------------------------------

#include "tokenize.h"
#include "errorlogger.h"

class CheckBufferOverrunClass
{
public:
    CheckBufferOverrunClass(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger);
    ~CheckBufferOverrunClass();

    // Buffer overrun..
    void bufferOverrun();


    // Dangerous functions that can cause buffer overruns
    void dangerousFunctions();
private:
    void CheckBufferOverrun_StructVariable();
    void CheckBufferOverrun_LocalVariable();
    void CheckBufferOverrun_CheckScope(const Token *tok, const char *varname[], const int size, const int total_size, unsigned int varid);
    void ReportError(const Token *tok, const char errmsg[]);

    const Tokenizer *_tokenizer;
    const Settings _settings;
    ErrorLogger *_errorLogger;
    std::list<const Token *> _callStack;
};

//---------------------------------------------------------------------------
#endif


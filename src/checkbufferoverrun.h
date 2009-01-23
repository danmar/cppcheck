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

    /** Check for buffer overruns */
    void bufferOverrun();

    /** Check that the code doesn't use dangerous functions that can cause buffer overruns (scanf and gets) */
    void dangerousFunctions();
private:

    /** Check for buffer overruns - locate struct variables and check them with the .._CheckScope function */
    void CheckBufferOverrun_StructVariable();

    /** Check for buffer overruns - locate local function variables and check them with the .._CheckScope function */
    void CheckBufferOverrun_LocalVariable();

    /** Check for buffer overruns - this is the function that performs the actual checking */
    void CheckBufferOverrun_CheckScope(const Token *tok, const char *varname[], const int size, const int total_size, unsigned int varid);

    /** Report error using the callstack */
    void ReportError(const std::string &errmsg);

    const Tokenizer *_tokenizer;
    const Settings _settings;
    ErrorLogger *_errorLogger;

    /** callstack - used during intra-function checking */
    std::list<const Token *> _callStack;
};

//---------------------------------------------------------------------------
#endif


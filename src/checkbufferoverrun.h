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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------
#ifndef CheckBufferOverrunH
#define CheckBufferOverrunH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"
#include <list>

class ErrorLogger;
class Token;
class Tokenizer;

/// @addtogroup Checks
/// @{


class CheckBufferOverrun : public Check
{
public:

    /** This constructor is used when registering the CheckClass */
    CheckBufferOverrun() : Check()
    { }

    /** This constructor is used when running checks.. */
    CheckBufferOverrun(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckBufferOverrun checkBufferOverrun(tokenizer, settings, errorLogger);
        if (settings->_showAll)
            checkBufferOverrun.bufferOverrun();
    }

    /** Check for buffer overruns */
    void bufferOverrun();
    static int count(const std::string &input_string);

private:

    /** Check for buffer overruns - locate struct variables and check them with the .._CheckScope function */
    void checkStructVariable();

    /** Check for buffer overruns - locate global variables and local function variables and check them with the checkScope function */
    void checkGlobalAndLocalVariable();

    /** Check for buffer overruns - this is the function that performs the actual checking */
    void checkScope(const Token *tok, const char *varname[], const int size, const int total_size, unsigned int varid);

    /** callstack - used during intra-function checking */
    std::list<const Token *> _callStack;

    void arrayIndexOutOfBounds(const Token *tok);
    void arrayIndexOutOfBounds();
    void bufferOverrun(const Token *tok);
    void dangerousStdCin(const Token *tok);
    void strncatUsage(const Token *tok);
    void outOfBounds(const Token *tok, const std::string &what);
    void sizeArgumentAsChar(const Token *tok);

    void getErrorMessages()
    {
        arrayIndexOutOfBounds(0);
        bufferOverrun(0);
        dangerousStdCin(0);
        strncatUsage(0);
        outOfBounds(0, "index");
        sizeArgumentAsChar(0);
    }

    std::string name() const
    {
        return "Bounds checking";
    }

    std::string classInfo() const
    {
        return "out of bounds checking";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif




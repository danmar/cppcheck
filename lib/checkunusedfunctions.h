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
#ifndef checkunusedfunctionsH
#define checkunusedfunctionsH
//---------------------------------------------------------------------------

#include "check.h"
#include "tokenize.h"
#include "errorlogger.h"

/// @addtogroup Checks
/// @{

class CheckUnusedFunctions: public Check
{
public:
    CheckUnusedFunctions(ErrorLogger *errorLogger = 0);
    ~CheckUnusedFunctions();

    /**
     * Errors found by this class are forwarded to the given
     * errorlogger.
     * @param errorLogger The errorlogger to be used.
     */
    void setErrorLogger(ErrorLogger *errorLogger);

    // Parse current tokens and determine..
    // * Check what functions are used
    // * What functions are declared
    void parseTokens(const Tokenizer &tokenizer);


    void check();

private:

    void getErrorMessages()
    {
        unusedFunctionError(0);
    }

    /**
     * Dummy implementation, just to provide error for --errorlist
     */
    void unusedFunctionError(const Token *tok);

    /**
     * Dummy implementation, just to provide error for --errorlist
     */
    void runSimplifiedChecks(const Tokenizer *, const Settings *, ErrorLogger *)
    {

    }

    std::string name() const
    {
        return "Unused functions";
    }

    std::string classInfo() const
    {
        return "Check for functions that are never called\n";
    }

    ErrorLogger *_errorLogger;


    class FunctionUsage
    {
    public:
        FunctionUsage() : usedSameFile(false), usedOtherFile(false)
        { }

        std::string filename;
        bool   usedSameFile;
        bool   usedOtherFile;
    };

    std::map<std::string, FunctionUsage> _functions;
};
/// @}
//---------------------------------------------------------------------------
#endif


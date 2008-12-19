/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
#ifndef CheckFunctionUsageH
#define CheckFunctionUsageH
//---------------------------------------------------------------------------

#include "tokenize.h"
#include "errorlogger.h"

class CheckFunctionUsage
{
public:
    CheckFunctionUsage( ErrorLogger *errorLogger = 0 );
    ~CheckFunctionUsage();

    /**
     * Errors found by this class are forwarded to the given
     * errorlogger.
     * @param errorLogger The errorlogger to be used.
     */
    void setErrorLogger( ErrorLogger *errorLogger );

    // Parse current tokens and determine..
    // * Check what functions are used
    // * What functions are declared
    void parseTokens( const Tokenizer &tokenizer );


    void check();

private:
    ErrorLogger *_errorLogger;


    class FunctionUsage
    {
    public:
        FunctionUsage()
        {
            filename = "";
            usedOtherFile = false;
            usedSameFile = false;
        }

        std::string filename;
        bool   usedSameFile;
        bool   usedOtherFile;
    };

    std::map<std::string, FunctionUsage> _functions;
};

//---------------------------------------------------------------------------
#endif


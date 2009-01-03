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
#include "checkfunctionusage.h"
#include "tokenize.h"
#include <sstream>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// FUNCTION USAGE - Check for unused functions etc
//---------------------------------------------------------------------------

CheckFunctionUsage::CheckFunctionUsage( ErrorLogger *errorLogger )
{
    _errorLogger = errorLogger;
}

CheckFunctionUsage::~CheckFunctionUsage()
{

}

void CheckFunctionUsage::setErrorLogger( ErrorLogger *errorLogger )
{
    _errorLogger = errorLogger;
}

void CheckFunctionUsage::parseTokens( const Tokenizer &tokenizer )
{
    // Function declarations..
    for ( const Token *tok = tokenizer.tokens(); tok; tok = tok->next() )
    {
        if ( tok->fileIndex() != 0 )
            continue;

        const Token *funcname = 0;

        if ( Token::Match( tok, "%type% %var% (" )  )
            funcname = tok->tokAt(1);
        else if ( Token::Match(tok, "%type% * %var% (") )
            funcname = tok->tokAt(2);
        else if ( Token::Match(tok, "%type% :: %var% (") && !Token::Match(tok, tok->strAt(2)) )
            funcname = tok->tokAt(2);

        // Check that ") {" is found..
        for (const Token *tok2 = funcname; tok2; tok2 = tok2->next())
        {
            if ( Token::Match(tok2, ")") )
            {
                if ( ! Token::Match(tok2, ") {") && ! Token::Match(tok2, ") const {") )
                    funcname = NULL;
                break;
            }
        }

        if ( funcname )
        {
            FunctionUsage &func = _functions[ funcname->str() ];

            // No filename set yet..
            if (func.filename.empty())
                func.filename = tokenizer.getFiles()->at(0);

            // Multiple files => filename = "+"
            else if (func.filename != tokenizer.getFiles()->at(0))
            {
                func.filename = "+";
                func.usedOtherFile |= func.usedSameFile;
            }
        }
    }

    // Function usage..
    for ( const Token *tok = tokenizer.tokens(); tok; tok = tok->next() )
    {
        const Token *funcname = 0;

        if ( Token::Match( tok, "[;{}.,()[=+-/&|!?:] %var% [(),;:}]" ) ||
             Token::Match(tok, ":: %var% (") ||
             Token::Match(tok, "|= %var% (") ||
             Token::Match(tok, "&= %var% (") ||
             Token::Match(tok, "&& %var% (") ||
             Token::Match(tok, "|| %var% (") ||
             Token::Match(tok, "else %var% (") ||
             Token::Match(tok, "return %var% (") )
            funcname = tok->next();

        // funcname ( => Assert that the end paranthesis isn't followed by {
        if ( Token::Match(funcname, "%var% (") )
        {
            int parlevel = 0;
            for ( const Token *tok2 = funcname; tok2; tok2 = tok2->next() )
            {
                if (tok2->str() == "(")
                    ++parlevel;

                else if (tok2->str() == ")")
                {
                    --parlevel;
                    if (parlevel == 0 && (Token::Match(tok2, ") {") || Token::Match(tok2, ") const")))
                        funcname = NULL;
                    if ( parlevel <= 0 )
                        break;
                }
            }
        }

        if ( funcname )
        {
            FunctionUsage &func = _functions[ funcname->str() ];

            if ( func.filename.empty() || func.filename == "+" )
                func.usedOtherFile = true;

            else
                func.usedSameFile = true;
        }
    }
}




void CheckFunctionUsage::check()
{
    for ( std::map<std::string, FunctionUsage>::const_iterator it = _functions.begin(); it != _functions.end(); ++it )
    {
        const FunctionUsage &func = it->second;
        if ( func.usedOtherFile || func.filename.empty() )
            continue;
        if ( ! func.usedSameFile )
        {
            std::ostringstream errmsg;
            if ( func.filename != "+" )
                errmsg << "[" << func.filename << "] ";
            errmsg << "The function '" << it->first << "' is never used.";
            _errorLogger->reportErr( errmsg.str() );
        }
        else if ( ! func.usedOtherFile )
        {
            /* TODO - add error message "function is only used in <file> it can be static"
            std::ostringstream errmsg;
            errmsg << "The function '" << it->first << "' is only used in the file it was declared in so it should have local linkage.";
            _errorLogger->reportErr( errmsg.str() );
            */
        }
    }
}




/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include "checkmutex.h"

#include "tokenize.h"
#include "token.h"
#include "errorlogger.h"
#include "symboldatabase.h"

#include <cctype>
#include <cstdlib>
#include <stdio.h>
//---------------------------------------------------------------------------

// Register CheckMutex..
namespace {
    CheckMutex instance;
}

void CheckMutex::checkFunction(const Token *tok)	{
        bool lock = false ;
        bool unlock = false ;

        Token * functionName = tok->link()->tokAt(-1)->link()->tokAt(-1);

        for ( const Token *tok2 = tok->link() ; tok2 && tok2 != tok; tok2 = tok2->next() ) {
           if ( tok2->str() ==  "pthread_mutex_lock" ) 	{
		lock = true ; 
           } else if ( tok2->str() == "pthread_mutex_unlock" )  {
                unlock = true ; 
           }
        }
        if (lock != unlock)	{
	   checkMutexUsageError(functionName, functionName->str()) ;
        }
}

void CheckMutex::checkMutexUsageError(const Token *tok, const std::string & fName) {
    reportError(tok, Severity::error, "pthreadLockUnlockMismatch", "A pthread_mutex_lock call doesnt have a related unlock call in function " + fName + ".");
}

void CheckMutex::checkMutexUsage()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase() ;
    std::list<Scope>::const_iterator scope;

    for (scope = symbolDatabase->scopeList.begin() ; scope != symbolDatabase->scopeList.end(); ++scope) {
        
         if (scope->type != Scope::eFunction) // check only functions
             continue ;
   
         const Token *tok = scope->classEnd;
         checkFunction(tok);    
   } 
}


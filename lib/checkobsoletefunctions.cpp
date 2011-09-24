/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
// Obsolete functions
//---------------------------------------------------------------------------

#include "checkobsoletefunctions.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace
{
CheckObsoleteFunctions instance;
}

void CheckObsoleteFunctions::obsoleteFunctions()
{
    if (!_settings->isEnabled("style"))
        return;

    // Don't check C# and Java code
    if (_tokenizer->isJavaOrCSharp())
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->isName() && tok->varId()==0 && tok->strAt(1) == "(" && !Token::Match(tok->previous(), ".|::|:|,"))
        {
            // function declaration?
            if (symbolDatabase->findFunctionByToken(tok))
            {
                _obsoleteStandardFunctions.erase(tok->str());
                _obsoletePosixFunctions.erase(tok->str());
                continue;
            }

            std::map<std::string,std::string>::const_iterator it = _obsoleteStandardFunctions.find(tok->str());
            if (it != _obsoleteStandardFunctions.end())
            {
                // If checking an old code base it might be uninteresting to update obsolete functions.
                // Therefore this is "information"
                reportError(tok->tokAt(1), Severity::information, "obsoleteFunctions"+it->first, it->second);
                break;
            }
            else if (_settings->posix)
            {
                it = _obsoletePosixFunctions.find(tok->str());
                if (it != _obsoletePosixFunctions.end())
                {
                    // If checking an old code base it might be uninteresting to update obsolete functions.
                    // Therefore this is "information"
                    reportError(tok->tokAt(1), Severity::information, "obsoleteFunctions"+it->first, it->second);
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------


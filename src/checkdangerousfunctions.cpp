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
// Buffer overrun..
//---------------------------------------------------------------------------

#include "checkdangerousfunctions.h"
#include "errormessage.h"

#include <algorithm>
#include <sstream>
#include <list>
#include <cstring>


#include <cstdlib>     // <- strtoul

//---------------------------------------------------------------------------

// _callStack used when parsing into subfunctions.


CheckDangerousFunctionsClass::CheckDangerousFunctionsClass(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        :  _settings(settings)
{
    _tokenizer = tokenizer;
    _errorLogger = errorLogger;
}

CheckDangerousFunctionsClass::~CheckDangerousFunctionsClass()
{

}

// Modified version of 'ReportError' that also reports the callstack
void CheckDangerousFunctionsClass::ReportError(const std::string &errmsg)
{
    std::ostringstream ostr;
    std::list<const Token *>::const_iterator it;
    for (it = _callStack.begin(); it != _callStack.end(); it++)
        ostr << _tokenizer->fileLine(*it) << " -> ";
    ostr << errmsg;
    _errorLogger->reportErr(ostr.str());
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Dangerous functions
//---------------------------------------------------------------------------

void CheckDangerousFunctionsClass::dangerousFunctions()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "mktemp ("))
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Found 'mktemp'. You should use 'mkstemp' instead";
            _errorLogger->reportErr(ostr.str());
        }
        else if (Token::Match(tok, "gets|scanf ("))
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Found '" << tok->str() << "'. You should use 'fgets' instead";
            _errorLogger->reportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------





/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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

#include "tokenize.h"
#include "token.h"

#include <algorithm>
#include <sstream>
#include <list>
#include <cstring>


#include <cstdlib>     // <- strtoul

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace
{
CheckDangerousFunctionsClass instance;
}

//---------------------------------------------------------------------------
// Dangerous functions
//---------------------------------------------------------------------------

void CheckDangerousFunctionsClass::dangerousFunctions()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "mktemp ("))
        {
            _errorLogger->dangerousFunctionmktemp(_tokenizer, tok);
        }
        else if (Token::simpleMatch(tok, "gets ("))
        {
            _errorLogger->dangerousFunctiongets(_tokenizer, tok);
        }
        else if (Token::simpleMatch(tok, "scanf ("))
        {
            _errorLogger->dangerousFunctionscanf(_tokenizer, tok);
        }
    }
}
//---------------------------------------------------------------------------





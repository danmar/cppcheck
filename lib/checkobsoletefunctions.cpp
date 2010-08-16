/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace
{
CheckObsoleteFunctions instance;
}

void CheckObsoleteFunctions::obsoleteFunctions()
{
    if (!_settings->_checkCodingStyle)
        return;

    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        std::list< std::pair<const std::string, const std::string> >::const_iterator it(_obsoleteFunctions.begin()), itend(_obsoleteFunctions.end());
        for (; it!=itend; ++it)
        {
            if (tok->strAt(1) == it->first && tok->strAt(2) == "(" && tok->tokAt(1)->varId() == 0 && !tok->tokAt(0)->isName() && tok->strAt(0) != "." && tok->strAt(0) != "::")
            {
                reportError(tok->tokAt(1), Severity::style, "obsoleteFunctions"+it->first, it->second);
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------


/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
// Obsolescent functions
//---------------------------------------------------------------------------

#include "checkobsolescentfunctions.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckObsolescentFunctions instance;
}

void CheckObsolescentFunctions::obsolescentFunctions()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Functions defined somewhere?
    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        _obsolescentStandardFunctions.erase(scope->className);
        _obsolescentPosixFunctions.erase(scope->className);
        _obsolescentC99Functions.erase(scope->className);
    }

    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->isName() && tok->varId()==0 && (tok->next() && tok->next()->str() == "(") &&
                (!Token::Match(tok->previous(), ".|::") || Token::simpleMatch(tok->tokAt(-2), "std ::"))) {

                std::map<std::string,std::string>::const_iterator it = _obsolescentStandardFunctions.find(tok->str());
                if (it != _obsolescentStandardFunctions.end()) {
                    // If checking an old code base it might be uninteresting to update obsolescent functions.
                    reportError(tok, Severity::style, "obsoleteFunctions"+it->first, it->second);
                } else {
                    if (_settings->standards.posix) {
                        it = _obsolescentPosixFunctions.find(tok->str());
                        if (it != _obsolescentPosixFunctions.end()) {
                            // If checking an old code base it might be uninteresting to update obsolescent functions.
                            reportError(tok, Severity::style, "obsoleteFunctions"+it->first, it->second);
                        }
                    }
                    if (_settings->standards.c >= Standards::C99) {
                        // alloca : this function is obsolescent in C but not in C++ (#4382)
                        it = _obsolescentC99Functions.find(tok->str());
                        if (it != _obsolescentC99Functions.end() && !(tok->str() == "alloca" && _tokenizer->isCPP())) {
                            reportError(tok, Severity::style, "obsoleteFunctions"+it->first, it->second);
                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------

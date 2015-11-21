/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Cppcheck team.
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
// Check functions
//---------------------------------------------------------------------------

#include "checkfunctions.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckFunctions instance;
}

void CheckFunctions::check()
{
    const bool checkAlloca = (_settings->standards.c >= Standards::C99 && _tokenizer->isC()) || _settings->standards.cpp >= Standards::CPP11;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->isName() && tok->varId() == 0 && tok->strAt(1) == "(") {
                // alloca() is special as it depends on the code being C or C++, so it is not in Library
                if (checkAlloca && Token::Match(tok, "alloca (") && (!tok->function() || tok->function()->nestedIn->type == Scope::eGlobal)) {
                    if (_tokenizer->isC())
                        reportError(tok, Severity::warning, "allocaCalled",
                                    "Obsolete function 'alloca' called. In C99 and later it is recommended to use a variable length array instead.\n"
                                    "The obsolete function 'alloca' is called. In C99 and later it is recommended to use a variable length array or "
                                    "a dynamically allocated array instead. The function 'alloca' is dangerous for many reasons "
                                    "(http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).");
                    else
                        reportError(tok, Severity::warning, "allocaCalled",
                                    "Obsolete function 'alloca' called. In C++11 and later it is recommended to use std::array<> instead.\n"
                                    "The obsolete function 'alloca' is called. In C++11 and later it is recommended to use std::array<> or "
                                    "a dynamically allocated array instead. The function 'alloca' is dangerous for many reasons "
                                    "(http://stackoverflow.com/questions/1018853/why-is-alloca-not-considered-good-practice and http://linux.die.net/man/3/alloca).");
                } else {
                    if (tok->function() && tok->function()->hasBody())
                        continue;

                    const Library::WarnInfo* wi = _settings->library.getWarnInfo(tok);
                    if (wi) {
                        if (_settings->isEnabled(Severity::toString(wi->severity)) && _settings->standards.c >= wi->standards.c && _settings->standards.cpp >= wi->standards.cpp) {
                            reportError(tok, wi->severity, tok->str() + "Called", wi->message);
                        }
                    }
                }
            }
        }
    }
}

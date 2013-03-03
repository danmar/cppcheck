/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "checkexceptionsafety.h"
#include "symboldatabase.h"
#include "token.h"

//---------------------------------------------------------------------------

// Register CheckExceptionSafety..
namespace {
    CheckExceptionSafety instance;
}


//---------------------------------------------------------------------------

void CheckExceptionSafety::destructors()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Perform check..
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        const Function * j = scope->function;
        if (j) {
            // only looking for destructors
            if (j->type == Function::eDestructor) {
                // Inspect this destructor..
                for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
                    // Skip try blocks
                    if (Token::simpleMatch(tok, "try {")) {
                        tok = tok->next()->link();
                    }

                    // throw found within a destructor
                    if (tok->str() == "throw") {
                        destructorsError(tok);
                        break;
                    }
                }
            }
        }
    }
}




void CheckExceptionSafety::deallocThrow()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Deallocate a global/member pointer and then throw exception
    // the pointer will be a dead pointer
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            // only looking for delete now
            if (tok->str() != "delete")
                continue;

            // Check if this is something similar with: "delete p;"
            tok = tok->next();
            if (Token::simpleMatch(tok, "[ ]"))
                tok = tok->tokAt(2);
            if (!tok)
                break;
            if (!Token::Match(tok, "%var% ;"))
                continue;

            // we only look for global variables
            const Variable *var = tok->variable();
            if (!var || !(var->isGlobal() || var->isStatic()))
                continue;

            const unsigned int varid(tok->varId());

            // Token where throw occurs
            const Token *ThrowToken = 0;

            // is there a throw after the deallocation?
            const Token* const end2 = tok->scope()->classEnd;
            for (const Token *tok2 = tok; tok2 != end2; tok2 = tok2->next()) {
                // Throw after delete -> Dead pointer
                if (tok2->str() == "throw") {
                    if (_settings->inconclusive) { // For inconclusive checking, throw directly.
                        deallocThrowError(tok2, tok->str());
                        break;
                    }
                    ThrowToken = tok2;
                }

                // Variable is assigned -> Bail out
                else if (Token::Match(tok2, "%varid% =", varid)) {
                    if (ThrowToken) // For non-inconclusive checking, wait until we find an assignment to it. Otherwise we assume it is safe to leave a dead pointer.
                        deallocThrowError(ThrowToken, tok2->str());
                    break;
                }
                // Variable passed to function. Assume it becomes assigned -> Bail out
                else if (Token::Match(tok2, "[,(] &| %varid% [,)]", varid)) // TODO: No bailout if passed by value or as const reference
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------
//      catch(const exception & err)
//      {
//         throw err;            // <- should be just "throw;"
//      }
//---------------------------------------------------------------------------
void CheckExceptionSafety::checkRethrowCopy()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eCatch)
            continue;

        const unsigned int varid = i->classStart->tokAt(-2)->varId();
        if (varid) {
            for (const Token* tok = i->classStart->next(); tok && tok != i->classEnd; tok = tok->next()) {
                if (Token::simpleMatch(tok, "catch (") && tok->next()->link() && tok->next()->link()->next()) // Don't check inner catch - it is handled in another iteration of outer loop.
                    tok = tok->next()->link()->next()->link();
                else if (Token::Match(tok, "throw %varid% ;", varid))
                    rethrowCopyError(tok, tok->strAt(1));
            }
        }
    }
}

//---------------------------------------------------------------------------
//    try {} catch (std::exception err) {} <- Should be "std::exception& err"
//---------------------------------------------------------------------------
void CheckExceptionSafety::checkCatchExceptionByValue()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i) {
        if (i->type != Scope::eCatch)
            continue;

        // Find a pass-by-value declaration in the catch(), excluding basic types
        // e.g. catch (std::exception err)
        const Variable *var = i->classStart->tokAt(-2)->variable();
        if (var && var->isClass() && !var->isPointer() && !var->isReference())
            catchExceptionByValueError(i->classDef);
    }
}

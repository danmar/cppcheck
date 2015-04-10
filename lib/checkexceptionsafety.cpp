/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

//---------------------------------------------------------------------------

// Register CheckExceptionSafety..
namespace {
    CheckExceptionSafety instance;
}


//---------------------------------------------------------------------------

void CheckExceptionSafety::destructors()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    // Perform check..
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        const Function * function = scope->function;
        if (function) {
            // only looking for destructors
            if (function->type == Function::eDestructor) {
                // Inspect this destructor.
                for (const Token *tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
                    // Skip try blocks
                    if (Token::simpleMatch(tok, "try {")) {
                        tok = tok->next()->link();
                    }

                    // Skip uncaught execptions
                    else if (Token::simpleMatch(tok, "if ( ! std :: uncaught_exception ( ) ) {")) {
                        tok = tok->next()->link(); // end of if ( ... )
                        tok = tok->next()->link(); // end of { ... }
                    }

                    // throw found within a destructor
                    else if (tok->str() == "throw") {
                        destructorsError(tok, scope->className);
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

    const bool printInconclusive = _settings->inconclusive;
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
            if (!tok || tok == scope->classEnd)
                break;
            if (!Token::Match(tok, "%var% ;"))
                continue;

            // we only look for global variables
            const Variable *var = tok->variable();
            if (!var || !(var->isGlobal() || var->isStatic()))
                continue;

            const unsigned int varid(tok->varId());

            // Token where throw occurs
            const Token *throwToken = nullptr;

            // is there a throw after the deallocation?
            const Token* const end2 = tok->scope()->classEnd;
            for (const Token *tok2 = tok; tok2 != end2; tok2 = tok2->next()) {
                // Throw after delete -> Dead pointer
                if (tok2->str() == "throw") {
                    if (printInconclusive) { // For inconclusive checking, throw directly.
                        deallocThrowError(tok2, tok->str());
                        break;
                    }
                    throwToken = tok2;
                }

                // Variable is assigned -> Bail out
                else if (Token::Match(tok2, "%varid% =", varid)) {
                    if (throwToken) // For non-inconclusive checking, wait until we find an assignment to it. Otherwise we assume it is safe to leave a dead pointer.
                        deallocThrowError(throwToken, tok2->str());
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


static const Token * functionThrowsRecursive(const Function * function, std::set<const Function *> & recursive)
{
    // check for recursion and bail if found
    if (!recursive.insert(function).second)
        return nullptr;

    if (!function->functionScope)
        return nullptr;

    for (const Token *tok = function->functionScope->classStart->next();
         tok != function->functionScope->classEnd; tok = tok->next()) {
        if (tok->str() == "try") {
            // just bail for now
            break;
        }
        if (tok->str() == "throw") {
            return tok;
        } else if (tok->function()) {
            const Function * called = tok->function();
            // check if called function has an exception specification
            if (called->isThrow() && called->throwArg) {
                return tok;
            } else if (called->isNoExcept() && called->noexceptArg &&
                       called->noexceptArg->str() != "true") {
                return tok;
            } else if (functionThrowsRecursive(called, recursive)) {
                return tok;
            }
        }
    }

    return nullptr;
}

static const Token * functionThrows(const Function * function)
{
    std::set<const Function *>  recursive;

    return functionThrowsRecursive(function, recursive);
}

//--------------------------------------------------------------------------
//    void func() noexcept { throw x; }
//    void func() throw() { throw x; }
//    void func() __attribute__((nothrow)); void func() { throw x; }
//--------------------------------------------------------------------------
void CheckExceptionSafety::nothrowThrows()
{
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        const Function* function = scope->function;
        if (!function)
            continue;

        // check noexcept and noexcept(true) functions
        if (function->isNoExcept() &&
            (!function->noexceptArg || function->noexceptArg->str() == "true")) {
            const Token *throws = functionThrows(function);
            if (throws)
                noexceptThrowError(throws);
        }

        // check throw() functions
        else if (function->isThrow() && !function->throwArg) {
            const Token *throws = functionThrows(function);
            if (throws)
                noexceptThrowError(throws);
        }

        // check __attribute__((nothrow)) or __declspec(nothrow) functions
        else if (function->isAttributeNothrow()) {
            const Token *throws = functionThrows(function);
            if (throws)
                noexceptThrowError(throws);
        }
    }
}

//--------------------------------------------------------------------------
//    void func() { functionWithExceptionSpecification(); }
//--------------------------------------------------------------------------
void CheckExceptionSafety::unhandledExceptionSpecification()
{
    if (!_settings->isEnabled("style") || !_settings->inconclusive)
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        // only check functions without exception epecification
        if (scope->function && !scope->function->isThrow() &&
            scope->className != "main" && scope->className != "wmain" &&
            scope->className != "_tmain" && scope->className != "WinMain") {
            for (const Token *tok = scope->function->functionScope->classStart->next();
                 tok != scope->function->functionScope->classEnd; tok = tok->next()) {
                if (tok->str() == "try") {
                    break;
                } else if (tok->function()) {
                    const Function * called = tok->function();
                    // check if called function has an exception specification
                    if (called->isThrow() && called->throwArg) {
                        unhandledExceptionSpecificationError(tok, called->tokenDef, scope->function->name());
                        break;
                    }
                }
            }
        }
    }
}


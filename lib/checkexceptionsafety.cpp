/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include "settings.h"
#include "symboldatabase.h"

#include <set>
#include <utility>

//---------------------------------------------------------------------------

// Register CheckExceptionSafety..
namespace {
    CheckExceptionSafety instance;
}


//---------------------------------------------------------------------------

void CheckExceptionSafety::destructors()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    // Perform check..
    for (const Scope * scope : symbolDatabase->functionScopes) {
        const Function * function = scope->function;
        if (!function)
            continue;
        // only looking for destructors
        if (function->type == Function::eDestructor) {
            // Inspect this destructor.
            for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
                // Skip try blocks
                if (Token::simpleMatch(tok, "try {")) {
                    tok = tok->next()->link();
                }

                // Skip uncaught exceptions
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

void CheckExceptionSafety::destructorsError(const Token * const tok, const std::string &className)
{
    reportError(tok, Severity::warning, "exceptThrowInDestructor",
                "Class " + className + " is not safe, destructor throws exception\n"
                "The class " + className + " is not safe because its destructor "
                "throws an exception. If " + className + " is used and an exception "
                "is thrown that is caught in an outer scope the program will terminate.", CWE398, Certainty::normal);
}


void CheckExceptionSafety::deallocThrow()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    const bool printInconclusive = mSettings->certainty.isEnabled(Certainty::inconclusive);
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    // Deallocate a global/member pointer and then throw exception
    // the pointer will be a dead pointer
    for (const Scope * scope : symbolDatabase->functionScopes) {
        for (const Token *tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
            // only looking for delete now
            if (tok->str() != "delete")
                continue;

            // Check if this is something similar with: "delete p;"
            tok = tok->next();
            if (Token::simpleMatch(tok, "[ ]"))
                tok = tok->tokAt(2);
            if (!tok || tok == scope->bodyEnd)
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
            const Token* const end2 = tok->scope()->bodyEnd;
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

void CheckExceptionSafety::deallocThrowError(const Token * const tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "exceptDeallocThrow", "Exception thrown in invalid state, '" +
                varname + "' points at deallocated memory.", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
//      catch(const exception & err)
//      {
//         throw err;            // <- should be just "throw;"
//      }
//---------------------------------------------------------------------------
void CheckExceptionSafety::checkRethrowCopy()
{
    if (!mSettings->severity.isEnabled(Severity::style))
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != Scope::eCatch)
            continue;

        const unsigned int varid = scope.bodyStart->tokAt(-2)->varId();
        if (varid) {
            for (const Token* tok = scope.bodyStart->next(); tok && tok != scope.bodyEnd; tok = tok->next()) {
                if (Token::simpleMatch(tok, "catch (") && tok->next()->link() && tok->next()->link()->next()) { // Don't check inner catch - it is handled in another iteration of outer loop.
                    tok = tok->next()->link()->next()->link();
                    if (!tok)
                        break;
                } else if (Token::Match(tok, "%varid% .", varid)) {
                    const Token *parent = tok->astParent();
                    while (Token::simpleMatch(parent->astParent(), "."))
                        parent = parent->astParent();
                    if (Token::Match(parent->astParent(), "%assign%|++|--|(") && parent == parent->astParent()->astOperand1())
                        break;
                } else if (Token::Match(tok, "throw %varid% ;", varid))
                    rethrowCopyError(tok, tok->strAt(1));
            }
        }
    }
}

void CheckExceptionSafety::rethrowCopyError(const Token * const tok, const std::string &varname)
{
    reportError(tok, Severity::style, "exceptRethrowCopy",
                "Throwing a copy of the caught exception instead of rethrowing the original exception.\n"
                "Rethrowing an exception with 'throw " + varname + ";' creates an unnecessary copy of '" + varname + "'. "
                "To rethrow the caught exception without unnecessary copying or slicing, use a bare 'throw;'.", CWE398, Certainty::normal);
}

//---------------------------------------------------------------------------
//    try {} catch (std::exception err) {} <- Should be "std::exception& err"
//---------------------------------------------------------------------------
void CheckExceptionSafety::checkCatchExceptionByValue()
{
    if (!mSettings->severity.isEnabled(Severity::style))
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope &scope : symbolDatabase->scopeList) {
        if (scope.type != Scope::eCatch)
            continue;

        // Find a pass-by-value declaration in the catch(), excluding basic types
        // e.g. catch (std::exception err)
        const Variable *var = scope.bodyStart->tokAt(-2)->variable();
        if (var && var->isClass() && !var->isPointer() && !var->isReference())
            catchExceptionByValueError(scope.classDef);
    }
}

void CheckExceptionSafety::catchExceptionByValueError(const Token *tok)
{
    reportError(tok, Severity::style,
                "catchExceptionByValue", "Exception should be caught by reference.\n"
                "The exception is caught by value. It could be caught "
                "as a (const) reference which is usually recommended in C++.", CWE398, Certainty::normal);
}


static const Token * functionThrowsRecursive(const Function * function, std::set<const Function *> & recursive)
{
    // check for recursion and bail if found
    if (!recursive.insert(function).second)
        return nullptr;

    if (!function->functionScope)
        return nullptr;

    for (const Token *tok = function->functionScope->bodyStart->next();
         tok != function->functionScope->bodyEnd; tok = tok->next()) {
        if (Token::simpleMatch(tok, "try {"))
            tok = tok->linkAt(1);  // skip till start of catch clauses
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
    std::set<const Function *> recursive;

    return functionThrowsRecursive(function, recursive);
}

//--------------------------------------------------------------------------
//    void func() noexcept { throw x; }
//    void func() throw() { throw x; }
//    void func() __attribute__((nothrow)); void func() { throw x; }
//--------------------------------------------------------------------------
void CheckExceptionSafety::nothrowThrows()
{
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope * scope : symbolDatabase->functionScopes) {
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

void CheckExceptionSafety::noexceptThrowError(const Token * const tok)
{
    reportError(tok, Severity::error, "throwInNoexceptFunction", "Exception thrown in function declared not to throw exceptions.", CWE398, Certainty::normal);
}

//--------------------------------------------------------------------------
//    void func() { functionWithExceptionSpecification(); }
//--------------------------------------------------------------------------
void CheckExceptionSafety::unhandledExceptionSpecification()
{
    if (!mSettings->severity.isEnabled(Severity::style) || !mSettings->certainty.isEnabled(Certainty::inconclusive))
        return;

    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();

    for (const Scope * scope : symbolDatabase->functionScopes) {
        // only check functions without exception epecification
        if (scope->function && !scope->function->isThrow() &&
            scope->className != "main" && scope->className != "wmain" &&
            scope->className != "_tmain" && scope->className != "WinMain") {
            for (const Token *tok = scope->function->functionScope->bodyStart->next();
                 tok != scope->function->functionScope->bodyEnd; tok = tok->next()) {
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

void CheckExceptionSafety::unhandledExceptionSpecificationError(const Token * const tok1, const Token * const tok2, const std::string & funcname)
{
    const std::string str1(tok1 ? tok1->str() : "foo");
    const std::list<const Token*> locationList = { tok1, tok2 };
    reportError(locationList, Severity::style, "unhandledExceptionSpecification",
                "Unhandled exception specification when calling function " + str1 + "().\n"
                "Unhandled exception specification when calling function " + str1 + "(). "
                "Either use a try/catch around the function call, or add a exception specification for " + funcname + "() also.", CWE703, Certainty::inconclusive);
}

//--------------------------------------------------------------------------
// 7.6.18.4 If no exception is presently being handled, evaluating a throw-expression with no operand calls std​::​​terminate().
//--------------------------------------------------------------------------
void CheckExceptionSafety::rethrowNoCurrentException()
{
    const SymbolDatabase* const symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {
        const Function* function = scope->function;
        if (!function)
            continue;

        // Rethrow can be used in 'exception dispatcher' idiom which is FP in such case
        // https://isocpp.org/wiki/faq/exceptions#throw-without-an-object
        // We check the beginning of the function with idiom pattern
        if (Token::simpleMatch(function->functionScope->bodyStart->next(), "try { throw ; } catch ("))
            continue;

        for (const Token *tok = function->functionScope->bodyStart->next();
             tok != function->functionScope->bodyEnd; tok = tok->next()) {
            if (Token::simpleMatch(tok, "catch (")) {
                tok = tok->linkAt(1);       // skip catch argument
                if (Token::simpleMatch(tok, ") {"))
                    tok = tok->linkAt(1);   // skip catch scope
                else
                    break;
            }
            if (Token::simpleMatch(tok, "throw ;")) {
                rethrowNoCurrentExceptionError(tok);
            }
        }
    }
}

void CheckExceptionSafety::rethrowNoCurrentExceptionError(const Token *tok)
{
    reportError(tok, Severity::error, "rethrowNoCurrentException",
                "Rethrowing current exception with 'throw;', it seems there is no current exception to rethrow."
                " If there is no current exception this calls std::terminate()."
                " More: https://isocpp.org/wiki/faq/exceptions#throw-without-an-object",
                CWE480, Certainty::normal);
}

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
// 64-bit portability
//---------------------------------------------------------------------------

#include "check64bit.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    Check64BitPortability instance;
}

/** Is given variable a pointer or array? */
static bool isaddr(const Variable *var)
{
    return (var && (var->isPointer() || var->isArray()));
}

/** Is given variable an integer variable */
static bool isint(const Variable *var)
{
    return (var && var->isIntegralType() && !var->isArrayOrPointer() && var->typeStartToken()->str() != "bool");
}

void Check64BitPortability::pointerassignment()
{
    if (!_settings->isEnabled("portability"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Check return values
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        if (scope->function == 0 || !scope->function->hasBody()) // We only look for functions with a body
            continue;

        bool retPointer = false;
        if (scope->function->token->strAt(-1) == "*") // Function returns a pointer
            retPointer = true;
        else if (Token::Match(scope->function->token->previous(), "int|long|DWORD")) // Function returns an integer
            ;
        else
            continue;

        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "return %name%|%num% [;+]") && !Token::simpleMatch(tok, "return 0 ;")) {
                enum { NO, INT, PTR, PTRDIFF } type = NO;
                for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                    if ((type == NO || type == INT) && Token::Match(tok2, "%var% [+;]") && isaddr(tok2->variable()))
                        type = PTR;
                    else if (type == NO && (tok2->isNumber() || isint(tok2->variable())))
                        type = INT;
                    else if (type == PTR && Token::Match(tok2, "- %var%") && isaddr(tok2->next()->variable()))
                        type = PTRDIFF;
                    else if (tok2->str() == "(") {
                        // TODO: handle parentheses
                        type = NO;
                        break;
                    } else if (type == PTR && Token::simpleMatch(tok2, "."))
                        type = NO; // Reset after pointer reference, see #4642
                    else if (tok2->str() == ";")
                        break;
                }

                if (retPointer && (type == INT || type == PTRDIFF))
                    returnIntegerError(tok);
                else if (!retPointer && type == PTR)
                    returnPointerError(tok);
            }
        }
    }

    // Check assignments
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "[;{}] %var% = %name%")) {
                const Token* tok2 = tok->tokAt(3);
                while (Token::Match(tok2->next(), ".|::"))
                    tok2 = tok2->tokAt(2);
                if (!Token::Match(tok2, "%var% ;|+"))
                    continue;

                const Variable *var1(tok->next()->variable());
                const Variable *var2(tok2->variable());

                if (isaddr(var1) && isint(var2) && tok2->strAt(1) != "+")
                    assignmentIntegerToAddressError(tok->next());

                else if (isint(var1) && isaddr(var2) && !tok2->isPointerCompare()) {
                    // assigning address => warning
                    // some trivial addition => warning
                    if (Token::Match(tok2->next(), "+ %any% !!;"))
                        continue;

                    assignmentAddressToIntegerError(tok->next());
                }
            }
        }
    }
}

void Check64BitPortability::assignmentAddressToIntegerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "AssignmentAddressToInteger",
                "Assigning a pointer to an integer is not portable.\n"
                "Assigning a pointer to an integer (int/long/etc) is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 64-bit address to 32-bit integer. The safe "
                "way is to store addresses only in pointer types (or typedefs like uintptr_t).");
}

void Check64BitPortability::assignmentIntegerToAddressError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "AssignmentIntegerToAddress",
                "Assigning an integer to a pointer is not portable.\n"
                "Assigning an integer (int/long/etc) to a pointer is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 64-bit integer to 32-bit pointer. The safe "
                "way is to store addresses only in pointer types (or typedefs like uintptr_t).");
}

void Check64BitPortability::returnPointerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "CastAddressToIntegerAtReturn",
                "Returning an address value in a function with integer return type is not portable.\n"
                "Returning an address value in a function with integer (int/long/etc) return type is not portable across "
                "different platforms and compilers. For example in 32-bit Windows and Linux they are same width, but in "
                "64-bit Windows and Linux they are of different width. In worst case you end up casting 64-bit address down "
                "to 32-bit integer. The safe way is to always return an integer.");
}

void Check64BitPortability::returnIntegerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "CastIntegerToAddressAtReturn",
                "Returning an integer in a function with pointer return type is not portable.\n"
                "Returning an integer (int/long/etc) in a function with pointer return type is not portable across different "
                "platforms and compilers. For example in 32-bit Windows and Linux they are same width, but in 64-bit Windows "
                "and Linux they are of different width. In worst case you end up casting 64-bit integer down to 32-bit pointer. "
                "The safe way is to always return a pointer.");
}

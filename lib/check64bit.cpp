/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

// CWE ids used
static const struct CWE CWE398(398U);   // Indicator of Poor Code Quality
static const struct CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior

// Register this check class (by creating a static instance of it)
namespace {
    Check64BitPortability instance;
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
            // skip nested functions
            if (tok->str() == "{") {
                if (tok->scope()->type == Scope::ScopeType::eFunction || tok->scope()->type == Scope::ScopeType::eLambda)
                    tok = tok->link();
            }

            if (tok->str() != "return")
                continue;

            if (!tok->astOperand1() || tok->astOperand1()->isNumber())
                continue;

            const ValueType * const returnType = tok->astOperand1()->valueType();
            if (!returnType)
                continue;

            if (retPointer && !returnType->typeScope && returnType->pointer == 0U)
                returnIntegerError(tok);

            if (!retPointer && returnType->pointer >= 1U)
                returnPointerError(tok);
        }
    }

    // Check assignments
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() != "=")
                continue;

            const ValueType *lhstype = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
            const ValueType *rhstype = tok->astOperand2() ? tok->astOperand2()->valueType() : nullptr;
            if (!lhstype || !rhstype)
                continue;

            // Assign integer to pointer..
            if (lhstype->pointer >= 1U &&
                !tok->astOperand2()->isNumber() &&
                rhstype->pointer == 0U &&
                rhstype->originalTypeName.empty() &&
                rhstype->type == ValueType::Type::INT)
                assignmentIntegerToAddressError(tok);

            // Assign pointer to integer..
            if (rhstype->pointer >= 1U &&
                lhstype->pointer == 0U &&
                lhstype->originalTypeName.empty() &&
                lhstype->isIntegral() &&
                lhstype->type >= ValueType::Type::CHAR &&
                lhstype->type <= ValueType::Type::INT)
                assignmentAddressToIntegerError(tok);
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
                "way is to store addresses only in pointer types (or typedefs like uintptr_t).", CWE758, false);
}

void Check64BitPortability::assignmentIntegerToAddressError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "AssignmentIntegerToAddress",
                "Assigning an integer to a pointer is not portable.\n"
                "Assigning an integer (int/long/etc) to a pointer is not portable across different platforms and "
                "compilers. For example in 32-bit Windows and linux they are same width, but in 64-bit Windows and linux "
                "they are of different width. In worst case you end up assigning 64-bit integer to 32-bit pointer. The safe "
                "way is to store addresses only in pointer types (or typedefs like uintptr_t).", CWE758, false);
}

void Check64BitPortability::returnPointerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "CastAddressToIntegerAtReturn",
                "Returning an address value in a function with integer return type is not portable.\n"
                "Returning an address value in a function with integer (int/long/etc) return type is not portable across "
                "different platforms and compilers. For example in 32-bit Windows and Linux they are same width, but in "
                "64-bit Windows and Linux they are of different width. In worst case you end up casting 64-bit address down "
                "to 32-bit integer. The safe way is to always return an integer.", CWE758, false);
}

void Check64BitPortability::returnIntegerError(const Token *tok)
{
    reportError(tok, Severity::portability,
                "CastIntegerToAddressAtReturn",
                "Returning an integer in a function with pointer return type is not portable.\n"
                "Returning an integer (int/long/etc) in a function with pointer return type is not portable across different "
                "platforms and compilers. For example in 32-bit Windows and Linux they are same width, but in 64-bit Windows "
                "and Linux they are of different width. In worst case you end up casting 64-bit integer down to 32-bit pointer. "
                "The safe way is to always return a pointer.", CWE758, false);
}

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
#include "checktype.h"
#include "mathlib.h"
#include "symboldatabase.h"

#include <stack>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckType instance;
}

//---------------------------------------------------------------------------
// Checking for shift by too many bits
//---------------------------------------------------------------------------

void CheckType::checkTooBigBitwiseShift()
{
    const bool printWarnings = _settings->isEnabled("warning");
    const bool printInconclusive = _settings->inconclusive;

    // unknown sizeof(int) => can't run this checker
    if (_settings->platformType == Settings::Unspecified)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!Token::Match(tok, "<<|>>|<<=|>>="))
                continue;

            if (!tok->astOperand1() || !tok->astOperand2())
                continue;

            // get number of bits of lhs
            const ValueType *lhstype = tok->astOperand1()->valueType();
            if (!lhstype || !lhstype->isIntegral() || lhstype->pointer >= 1U)
                continue;
            int lhsbits = 0;
            if (lhstype->type <= ValueType::Type::INT)
                lhsbits = 8 * _settings->sizeof_int;
            else if (lhstype->type == ValueType::Type::LONG)
                lhsbits = 8 * _settings->sizeof_long;
            else if (lhstype->type == ValueType::Type::LONGLONG)
                lhsbits = 8 * _settings->sizeof_long_long;
            else
                continue;

            // Get biggest rhs value. preferably a value which doesn't have 'condition'.
            const ValueFlow::Value *value = tok->astOperand2()->getValueGE(lhsbits, _settings);
            if (!value)
                continue;
            if (value->condition && !printWarnings)
                continue;
            if (value->inconclusive && !printInconclusive)
                continue;
            tooBigBitwiseShiftError(tok, lhsbits, *value);
        }
    }
}

void CheckType::tooBigBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits)
{
    std::list<const Token*> callstack;
    callstack.push_back(tok);
    if (rhsbits.condition)
        callstack.push_back(rhsbits.condition);
    std::ostringstream errmsg;
    errmsg << "Shifting " << lhsbits << "-bit value by " << rhsbits.intvalue << " bits is undefined behaviour";
    if (rhsbits.condition)
        errmsg << ". See condition at line " << rhsbits.condition->linenr() << ".";
    reportError(callstack, rhsbits.condition ? Severity::warning : Severity::error, "shiftTooManyBits", errmsg.str(), 0U, rhsbits.inconclusive);
}

//---------------------------------------------------------------------------
// Checking for integer overflow
//---------------------------------------------------------------------------

void CheckType::checkIntegerOverflow()
{
    // unknown sizeof(int) => can't run this checker
    if (_settings->platformType == Settings::Unspecified)
        return;

    // max int value according to platform settings.
    const MathLib::bigint maxint = (1LL << (8 * _settings->sizeof_int - 1)) - 1;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isArithmeticalOp())
                continue;

            // is there a overflow result value
            const ValueFlow::Value *value = tok->getValueGE(maxint + 1, _settings);
            if (!value)
                value = tok->getValueLE(-maxint - 2, _settings);
            if (!value)
                continue;

            // is result signed integer?
            const ValueType *vt = tok->valueType();
            if (vt && vt->type == ValueType::Type::INT && vt->sign == ValueType::Sign::SIGNED)
                integerOverflowError(tok, *value);
        }
    }
}

void CheckType::integerOverflowError(const Token *tok, const ValueFlow::Value &value)
{
    const std::string expr(tok ? tok->expressionString() : "");

    std::string msg;
    if (value.condition)
        msg = ValueFlow::eitherTheConditionIsRedundant(value.condition) +
              " or there is signed integer overflow for expression '" + expr + "'.";
    else
        msg = "Signed integer overflow for expression '" + expr + "'.";

    reportError(tok,
                value.condition ? Severity::warning : Severity::error,
                "integerOverflow",
                msg,
                0U,
                value.inconclusive);
}

//---------------------------------------------------------------------------
// Checking for sign conversion when operand can be negative
//---------------------------------------------------------------------------

void CheckType::checkSignConversion()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isArithmeticalOp() || Token::Match(tok,"+|-"))
                continue;

            // Is result unsigned?
            if (!(tok->valueType() && tok->valueType()->sign == ValueType::Sign::UNSIGNED))
                continue;

            // Check if an operand can be negative..
            std::stack<const Token *> tokens;
            tokens.push(tok->astOperand1());
            tokens.push(tok->astOperand2());
            while (!tokens.empty()) {
                const Token *tok1 = tokens.top();
                tokens.pop();
                if (!tok1)
                    continue;
                if (!tok1->getValueLE(-1,_settings))
                    continue;
                if (tok1->valueType() && tok1->valueType()->sign != ValueType::Sign::UNSIGNED)
                    signConversionError(tok1);
            }
        }
    }
}

void CheckType::signConversionError(const Token *tok)
{
    const std::string varname(tok ? tok->str() : "var");

    reportError(tok,
                Severity::warning,
                "signConversion",
                "Suspicious code: sign conversion of " + varname + " in calculation, even though " + varname + " can have a negative value");
}


//---------------------------------------------------------------------------
// Checking for long cast of int result   const long x = var1 * var2;
//---------------------------------------------------------------------------

void CheckType::checkLongCast()
{
    if (!_settings->isEnabled("style"))
        return;

    // Assignments..
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "=" || !Token::Match(tok->astOperand2(), "*|<<"))
            continue;

        const ValueType *lhstype = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
        const ValueType *rhstype = tok->astOperand2()->valueType();

        if (!lhstype || !rhstype)
            continue;

        // assign int result to long/longlong const nonpointer?
        if (rhstype->type == ValueType::Type::INT &&
            rhstype->pointer == 0U &&
            rhstype->originalTypeName.empty() &&
            (lhstype->type == ValueType::Type::LONG || lhstype->type == ValueType::Type::LONGLONG) &&
            lhstype->pointer == 0U &&
            lhstype->constness == 1U &&
            lhstype->originalTypeName.empty())
            longCastAssignError(tok);
    }

    // Return..
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        // function must return long data
        const Token * def = scope->classDef;
        bool islong = false;
        while (Token::Match(def, "%type%|::")) {
            if (def->str() == "long" && def->originalName().empty()) {
                islong = true;
                break;
            }
            def = def->previous();
        }
        if (!islong)
            continue;

        // return statements
        const Token *ret = nullptr;
        for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() == "return") {
                if (Token::Match(tok->astOperand1(), "<<|*")) {
                    const ValueType *type = tok->astOperand1()->valueType();
                    if (type && type->type == ValueType::Type::INT && type->pointer == 0U && type->originalTypeName.empty())
                        ret = tok;
                }
                // All return statements must have problem otherwise no warning
                if (ret != tok) {
                    ret = nullptr;
                    break;
                }
            }
        }

        if (ret)
            longCastReturnError(ret);
    }
}

void CheckType::longCastAssignError(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "truncLongCastAssignment",
                "int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information.\n"
                "int result is assigned to long variable. If the variable is long to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to long, for example 'l = a * b;' => 'l = (long)a * b;'.");
}

void CheckType::longCastReturnError(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "truncLongCastReturn",
                "int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information.\n"
                "int result is returned as long value. If the return value is long to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to long, for example 'return a*b;' => 'return (long)a*b'.");
}

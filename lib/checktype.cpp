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
//

// CWE ids used:
static const struct CWE CWE195(195U);   // Signed to Unsigned Conversion Error
static const struct CWE CWE197(197U);   // Numeric Truncation Error
static const struct CWE CWE758(758U);   // Reliance on Undefined, Unspecified, or Implementation-Defined Behavior
static const struct CWE CWE190(190U);   // Integer Overflow or Wraparound


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
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            // C++ and macro: OUT(x<<y)
            if (_tokenizer->isCPP() && Token::Match(tok, "[;{}] %name% (") && Token::simpleMatch(tok->linkAt(2), ") ;") && tok->next()->isUpperCaseName() && !tok->next()->function())
                tok = tok->linkAt(2);

            if (!tok->astOperand1() || !tok->astOperand2())
                continue;

            if (!Token::Match(tok, "<<|>>|<<=|>>="))
                continue;

            // get number of bits of lhs
            const ValueType *lhstype = tok->astOperand1()->valueType();
            if (!lhstype || !lhstype->isIntegral() || lhstype->pointer >= 1U)
                continue;
            int lhsbits = 0;
            if (lhstype->type <= ValueType::Type::INT)
                lhsbits = _settings->int_bit;
            else if (lhstype->type == ValueType::Type::LONG)
                lhsbits = _settings->long_bit;
            else if (lhstype->type == ValueType::Type::LONGLONG)
                lhsbits = _settings->long_long_bit;
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
    reportError(callstack, rhsbits.condition ? Severity::warning : Severity::error, "shiftTooManyBits", errmsg.str(), CWE758, rhsbits.inconclusive);
}

//---------------------------------------------------------------------------
// Checking for integer overflow
//---------------------------------------------------------------------------

void CheckType::checkIntegerOverflow()
{
    // unknown sizeof(int) => can't run this checker
    if (_settings->platformType == Settings::Unspecified || _settings->int_bit >= 64)
        return;

    // max int value according to platform settings.
    const MathLib::bigint maxint = (1LL << (_settings->int_bit - 1)) - 1;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isArithmeticalOp())
                continue;

            // is result signed integer?
            const ValueType *vt = tok->valueType();
            if (!vt || vt->type != ValueType::Type::INT || vt->sign != ValueType::Sign::SIGNED)
                continue;

            // is there a overflow result value
            const ValueFlow::Value *value = tok->getValueGE(maxint + 1, _settings);
            if (!value)
                value = tok->getValueLE(-maxint - 2, _settings);
            if (!value)
                continue;

            // For left shift, it's common practice to shift into the sign bit
            if (tok->str() == "<<" && value->intvalue > 0 && value->intvalue < (1LL << _settings->int_bit))
                continue;

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
                CWE190,
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
                    signConversionError(tok1, tok1->isNumber());
            }
        }
    }
}

void CheckType::signConversionError(const Token *tok, const bool constvalue)
{
    const std::string varname(tok ? tok->str() : "var");

    reportError(tok,
                Severity::warning,
                "signConversion",
                (constvalue) ?
                "Suspicious code: sign conversion of " + varname + " in calculation because '" + varname + "' has a negative value" :
                "Suspicious code: sign conversion of " + varname + " in calculation, even though " + varname + " can have a negative value", CWE195, false);
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
                "int result is assigned to long variable. If the variable is long to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to long, for example 'l = a * b;' => 'l = (long)a * b;'.", CWE197, false);
}

void CheckType::longCastReturnError(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "truncLongCastReturn",
                "int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information.\n"
                "int result is returned as long value. If the return value is long to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to long, for example 'return a*b;' => 'return (long)a*b'.", CWE197, false);
}

//---------------------------------------------------------------------------
// Checking for float to integer overflow
//---------------------------------------------------------------------------

void CheckType::checkFloatToIntegerOverflow()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() != "(")
                continue;

            if (!tok->astOperand1() || tok->astOperand2())
                continue;

            // is result integer?
            const ValueType *vt = tok->valueType();
            if (!vt || !vt->isIntegral())
                continue;

            // is value float?
            const ValueType *vt1 = tok->astOperand1()->valueType();
            if (!vt1 || !vt1->isFloat())
                continue;

            const Token *op1 = tok->astOperand1();
            for (std::list<ValueFlow::Value>::const_iterator it = op1->values().begin(); it != op1->values().end(); ++it) {
                if (it->valueType != ValueFlow::Value::FLOAT)
                    continue;
                if (it->inconclusive && !_settings->inconclusive)
                    continue;
                if (it->floatValue > ~0ULL)
                    floatToIntegerOverflowError(tok, *it);
                else if ((-it->floatValue) > (1ULL<<62))
                    floatToIntegerOverflowError(tok, *it);
                else if (_settings->platformType != Settings::Unspecified) {
                    int bits = 0;
                    if (vt->type == ValueType::Type::CHAR)
                        bits = _settings->char_bit;
                    else if (vt->type == ValueType::Type::SHORT)
                        bits = _settings->short_bit;
                    else if (vt->type == ValueType::Type::INT)
                        bits = _settings->int_bit;
                    else if (vt->type == ValueType::Type::LONG)
                        bits = _settings->long_bit;
                    else if (vt->type == ValueType::Type::LONGLONG)
                        bits = _settings->long_long_bit;
                    else
                        continue;
                    if (bits < 64 && it->floatValue >= (1ULL << bits))
                        floatToIntegerOverflowError(tok, *it);
                }
            }
        }
    }
}

void CheckType::floatToIntegerOverflowError(const Token *tok, const ValueFlow::Value &value)
{
    std::ostringstream errmsg;
    errmsg << "Undefined behaviour: float (" << value.floatValue << ") conversion overflow.";
    reportError(tok,
                Severity::error,
                "floatConversionOverflow",
                errmsg.str(), CWE190, value.inconclusive);
}

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "errortypes.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"

#include <cmath>
#include <list>
#include <ostream>
#include <vector>

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
    // unknown sizeof(int) => can't run this checker
    if (mSettings->platformType == Settings::Unspecified)
        return;

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        // C++ and macro: OUT(x<<y)
        if (mTokenizer->isCPP() && Token::Match(tok, "[;{}] %name% (") && Token::simpleMatch(tok->linkAt(2), ") ;") && tok->next()->isUpperCaseName() && !tok->next()->function())
            tok = tok->linkAt(2);

        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        if (!Token::Match(tok, "<<|>>|<<=|>>="))
            continue;

        // get number of bits of lhs
        const ValueType * const lhstype = tok->astOperand1()->valueType();
        if (!lhstype || !lhstype->isIntegral() || lhstype->pointer >= 1)
            continue;
        // C11 Standard, section 6.5.7 Bitwise shift operators, states:
        //   The integer promotions are performed on each of the operands.
        //   The type of the result is that of the promoted left operand.
        int lhsbits;
        if ((lhstype->type == ValueType::Type::CHAR) ||
            (lhstype->type == ValueType::Type::SHORT) ||
            (lhstype->type == ValueType::Type::WCHAR_T) ||
            (lhstype->type == ValueType::Type::BOOL) ||
            (lhstype->type == ValueType::Type::INT))
            lhsbits = mSettings->int_bit;
        else if (lhstype->type == ValueType::Type::LONG)
            lhsbits = mSettings->long_bit;
        else if (lhstype->type == ValueType::Type::LONGLONG)
            lhsbits = mSettings->long_long_bit;
        else
            continue;

        // Get biggest rhs value. preferably a value which doesn't have 'condition'.
        const ValueFlow::Value * value = tok->astOperand2()->getValueGE(lhsbits, mSettings);
        if (value && mSettings->isEnabled(value, false))
            tooBigBitwiseShiftError(tok, lhsbits, *value);
        else if (lhstype->sign == ValueType::Sign::SIGNED) {
            value = tok->astOperand2()->getValueGE(lhsbits-1, mSettings);
            if (value && mSettings->isEnabled(value, false))
                tooBigSignedBitwiseShiftError(tok, lhsbits, *value);
        }
    }
}

void CheckType::tooBigBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits)
{
    const char id[] = "shiftTooManyBits";

    if (!tok) {
        reportError(tok, Severity::error, id, "Shifting 32-bit value by 40 bits is undefined behaviour", CWE758, Certainty::normal);
        return;
    }

    const ErrorPath errorPath = getErrorPath(tok, &rhsbits, "Shift");

    std::ostringstream errmsg;
    errmsg << "Shifting " << lhsbits << "-bit value by " << rhsbits.intvalue << " bits is undefined behaviour";
    if (rhsbits.condition)
        errmsg << ". See condition at line " << rhsbits.condition->linenr() << ".";

    reportError(errorPath, rhsbits.errorSeverity() ? Severity::error : Severity::warning, id, errmsg.str(), CWE758, rhsbits.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

void CheckType::tooBigSignedBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits)
{
    const char id[] = "shiftTooManyBitsSigned";

    const bool cpp14 = mSettings->standards.cpp >= Standards::CPP14;

    std::string behaviour = "undefined";
    if (cpp14)
        behaviour = "implementation-defined";
    if (!tok) {
        reportError(tok, Severity::error, id, "Shifting signed 32-bit value by 31 bits is " + behaviour + " behaviour", CWE758, Certainty::normal);
        return;
    }

    const ErrorPath errorPath = getErrorPath(tok, &rhsbits, "Shift");

    std::ostringstream errmsg;
    errmsg << "Shifting signed " << lhsbits << "-bit value by " << rhsbits.intvalue << " bits is " + behaviour + " behaviour";
    if (rhsbits.condition)
        errmsg << ". See condition at line " << rhsbits.condition->linenr() << ".";

    Severity::SeverityType severity = rhsbits.errorSeverity() ? Severity::error : Severity::warning;
    if (cpp14)
        severity = Severity::portability;

    if ((severity == Severity::portability) && !mSettings->severity.isEnabled(Severity::portability))
        return;
    reportError(errorPath, severity, id, errmsg.str(), CWE758, rhsbits.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------
// Checking for integer overflow
//---------------------------------------------------------------------------

void CheckType::checkIntegerOverflow()
{
    // unknown sizeof(int) => can't run this checker
    if (mSettings->platformType == Settings::Unspecified || mSettings->int_bit >= MathLib::bigint_bits)
        return;

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isArithmeticalOp())
            continue;

        // is result signed integer?
        const ValueType *vt = tok->valueType();
        if (!vt || !vt->isIntegral() || vt->sign != ValueType::Sign::SIGNED)
            continue;

        unsigned int bits;
        if (vt->type == ValueType::Type::INT)
            bits = mSettings->int_bit;
        else if (vt->type == ValueType::Type::LONG)
            bits = mSettings->long_bit;
        else if (vt->type == ValueType::Type::LONGLONG)
            bits = mSettings->long_long_bit;
        else
            continue;

        if (bits >= MathLib::bigint_bits)
            continue;

        // max value according to platform settings.
        const MathLib::bigint maxvalue = (((MathLib::biguint)1) << (bits - 1)) - 1;

        // is there a overflow result value
        const ValueFlow::Value *value = tok->getValueGE(maxvalue + 1, mSettings);
        if (!value)
            value = tok->getValueLE(-maxvalue - 2, mSettings);
        if (!value || !mSettings->isEnabled(value,false))
            continue;

        // For left shift, it's common practice to shift into the sign bit
        //if (tok->str() == "<<" && value->intvalue > 0 && value->intvalue < (((MathLib::bigint)1) << bits))
        //    continue;

        integerOverflowError(tok, *value);
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

    if (value.safe)
        msg = "Safe checks: " + msg;

    reportError(getErrorPath(tok, &value, "Integer overflow"),
                value.errorSeverity() ? Severity::error : Severity::warning,
                getMessageId(value, "integerOverflow").c_str(),
                msg,
                CWE190,
                value.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

//---------------------------------------------------------------------------
// Checking for sign conversion when operand can be negative
//---------------------------------------------------------------------------

void CheckType::checkSignConversion()
{
    if (!mSettings->severity.isEnabled(Severity::warning))
        return;

    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (!tok->isArithmeticalOp() || Token::Match(tok,"+|-"))
            continue;

        // Is result unsigned?
        if (!(tok->valueType() && tok->valueType()->sign == ValueType::Sign::UNSIGNED))
            continue;

        // Check if an operand can be negative..
        const Token * astOperands[] = { tok->astOperand1(), tok->astOperand2() };
        for (const Token * tok1 : astOperands) {
            if (!tok1)
                continue;
            const ValueFlow::Value* negativeValue =
                ValueFlow::findValue(tok1->values(), mSettings, [&](const ValueFlow::Value& v) {
                return !v.isImpossible() && v.isIntValue() && (v.intvalue <= -1 || v.wideintvalue <= -1);
            });
            if (!negativeValue)
                continue;
            if (tok1->valueType() && tok1->valueType()->sign != ValueType::Sign::UNSIGNED)
                signConversionError(tok1, negativeValue, tok1->isNumber());
        }
    }
}

void CheckType::signConversionError(const Token *tok, const ValueFlow::Value *negativeValue, const bool constvalue)
{
    const std::string expr(tok ? tok->expressionString() : "var");

    std::ostringstream msg;
    if (tok && tok->isName())
        msg << "$symbol:" << expr << "\n";
    if (constvalue)
        msg << "Expression '" << expr << "' has a negative value. That is converted to an unsigned value and used in an unsigned calculation.";
    else
        msg << "Expression '" << expr << "' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation.";

    if (!negativeValue)
        reportError(tok, Severity::warning, "signConversion", msg.str(), CWE195, Certainty::normal);
    else {
        const ErrorPath &errorPath = getErrorPath(tok,negativeValue,"Negative value is converted to an unsigned value");
        reportError(errorPath,
                    Severity::warning,
                    Check::getMessageId(*negativeValue, "signConversion").c_str(),
                    msg.str(),
                    CWE195,
                    negativeValue->isInconclusive() ? Certainty::inconclusive : Certainty::normal);
    }
}


//---------------------------------------------------------------------------
// Checking for long cast of int result   const long x = var1 * var2;
//---------------------------------------------------------------------------

void CheckType::checkLongCast()
{
    if (!mSettings->severity.isEnabled(Severity::style))
        return;

    // Assignments..
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->str() != "=" || !Token::Match(tok->astOperand2(), "*|<<"))
            continue;

        if (tok->astOperand2()->hasKnownIntValue()) {
            const ValueFlow::Value &v = tok->astOperand2()->values().front();
            if (mSettings->isIntValue(v.intvalue))
                continue;
        }

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
    const SymbolDatabase *symbolDatabase = mTokenizer->getSymbolDatabase();
    for (const Scope * scope : symbolDatabase->functionScopes) {

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
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
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
                "int result is assigned to long variable. If the variable is long to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to long, for example 'l = a * b;' => 'l = (long)a * b;'.", CWE197, Certainty::normal);
}

void CheckType::longCastReturnError(const Token *tok)
{
    reportError(tok,
                Severity::style,
                "truncLongCastReturn",
                "int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information.\n"
                "int result is returned as long value. If the return value is long to avoid loss of information, then there is loss of information. To avoid loss of information you must cast a calculation operand to long, for example 'return a*b;' => 'return (long)a*b'.", CWE197, Certainty::normal);
}

//---------------------------------------------------------------------------
// Checking for float to integer overflow
//---------------------------------------------------------------------------

void CheckType::checkFloatToIntegerOverflow()
{
    for (const Token *tok = mTokenizer->tokens(); tok; tok = tok->next()) {
        const ValueType *vtint, *vtfloat;
        const std::list<ValueFlow::Value> *floatValues;

        // Explicit cast
        if (Token::Match(tok, "( %name%") && tok->astOperand1() && !tok->astOperand2()) {
            vtint = tok->valueType();
            vtfloat = tok->astOperand1()->valueType();
            floatValues = &tok->astOperand1()->values();
            checkFloatToIntegerOverflow(tok, vtint, vtfloat, floatValues);
        }

        // Assignment
        else if (tok->str() == "=" && tok->astOperand1() && tok->astOperand2()) {
            vtint = tok->astOperand1()->valueType();
            vtfloat = tok->astOperand2()->valueType();
            floatValues = &tok->astOperand2()->values();
            checkFloatToIntegerOverflow(tok, vtint, vtfloat, floatValues);
        }

        else if (tok->str() == "return" && tok->astOperand1() && tok->astOperand1()->valueType() && tok->astOperand1()->valueType()->isFloat()) {
            const Scope *scope = tok->scope();
            while (scope && scope->type != Scope::ScopeType::eLambda && scope->type != Scope::ScopeType::eFunction)
                scope = scope->nestedIn;
            if (scope && scope->type == Scope::ScopeType::eFunction && scope->function && scope->function->retDef) {
                const ValueType &valueType = ValueType::parseDecl(scope->function->retDef, mSettings);
                vtfloat = tok->astOperand1()->valueType();
                floatValues = &tok->astOperand1()->values();
                checkFloatToIntegerOverflow(tok, &valueType, vtfloat, floatValues);
            }
        }
    }
}

void CheckType::checkFloatToIntegerOverflow(const Token *tok, const ValueType *vtint, const ValueType *vtfloat, const std::list<ValueFlow::Value> *floatValues)
{
    // Conversion of float to integer?
    if (!vtint || !vtint->isIntegral())
        return;
    if (!vtfloat || !vtfloat->isFloat())
        return;

    for (const ValueFlow::Value &f : *floatValues) {
        if (f.valueType != ValueFlow::Value::ValueType::FLOAT)
            continue;
        if (!mSettings->isEnabled(&f, false))
            continue;
        if (f.floatValue >= std::exp2(mSettings->long_long_bit))
            floatToIntegerOverflowError(tok, f);
        else if ((-f.floatValue) > std::exp2(mSettings->long_long_bit - 1))
            floatToIntegerOverflowError(tok, f);
        else if (mSettings->platformType != Settings::Unspecified) {
            int bits = 0;
            if (vtint->type == ValueType::Type::CHAR)
                bits = mSettings->char_bit;
            else if (vtint->type == ValueType::Type::SHORT)
                bits = mSettings->short_bit;
            else if (vtint->type == ValueType::Type::INT)
                bits = mSettings->int_bit;
            else if (vtint->type == ValueType::Type::LONG)
                bits = mSettings->long_bit;
            else if (vtint->type == ValueType::Type::LONGLONG)
                bits = mSettings->long_long_bit;
            else
                continue;
            if (bits < MathLib::bigint_bits && f.floatValue >= (((MathLib::biguint)1) << bits))
                floatToIntegerOverflowError(tok, f);
        }
    }
}

void CheckType::floatToIntegerOverflowError(const Token *tok, const ValueFlow::Value &value)
{
    std::ostringstream errmsg;
    errmsg << "Undefined behaviour: float (" << value.floatValue << ") to integer conversion overflow.";
    reportError(getErrorPath(tok, &value, "float to integer conversion"),
                value.errorSeverity() ? Severity::error : Severity::warning,
                "floatConversionOverflow",
                errmsg.str(), CWE190, value.isInconclusive() ? Certainty::inconclusive : Certainty::normal);
}

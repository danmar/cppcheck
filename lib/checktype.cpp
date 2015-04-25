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
#include "checktype.h"
#include "mathlib.h"
#include "symboldatabase.h"

#include <stack>
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckType instance;
}

static bool astGetSizeSign(const Settings *settings, const Token *tok, unsigned int *size, char *sign)
{
    if (!tok)
        return false;
    if (tok->isArithmeticalOp()) {
        if (!astGetSizeSign(settings, tok->astOperand1(), size, sign))
            return false;
        return !tok->astOperand2() || astGetSizeSign(settings, tok->astOperand2(), size, sign);
    }
    if (tok->isNumber() && MathLib::isInt(tok->str())) {
        if (tok->str().find("L") != std::string::npos)
            return false;
        MathLib::bigint value = MathLib::toLongNumber(tok->str());
        unsigned int sz;
        if (value >= -(1<<7) && value <= (1<<7)-1)
            sz = 8;
        else if (value >= -(1<<15) && value <= (1<<15)-1)
            sz = 16;
        else if (value >= -(1LL<<31) && value <= (1LL<<31)-1)
            sz = 32;
        else
            return false;
        if (sz < 8 * settings->sizeof_int)
            sz = 8 * settings->sizeof_int;
        if (*size < sz)
            *size = sz;
        if (tok->str().find('U') != std::string::npos)
            *sign = 'u';
        if (*sign != 'u')
            *sign = 's';
        return true;
    }
    if (tok->isName()) {
        const Variable *var = tok->variable();
        if (!var)
            return false;
        unsigned int sz = 0;
        for (const Token *type = var->typeStartToken(); type; type = type->next()) {
            if (type->str() == "*")
                return false;  // <- FIXME: handle pointers
            if (Token::Match(type, "char|short|int")) {
                sz = 8 * settings->sizeof_int;
                if (type->isUnsigned())
                    *sign = 'u';
                else if (*sign != 'u')
                    *sign = 's';
            } else if (Token::Match(type, "float|double|long")) {
                return false;
            } else {
                // TODO: try to lookup type info in library
            }
            if (type == var->typeEndToken())
                break;
        }
        if (sz == 0)
            return false;
        if (*size < sz)
            *size = sz;
        return true;
    }
    return false;
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
            if (tok->str() != "<<" && tok->str() != ">>")
                continue;

            if (!tok->astOperand1() || !tok->astOperand2())
                continue;

            // get number of bits of lhs
            const Variable *var = tok->astOperand1()->variable();
            if (!var)
                continue;
            int lhsbits = 0;
            for (const Token *type = var->typeStartToken(); type; type = type->next()) {
                if (Token::Match(type,"char|short|int") && !type->isLong()) {
                    lhsbits = _settings->sizeof_int * 8;
                    break;
                }
                if (type == var->typeEndToken() || type->str() == "<")
                    break;
            }
            if (lhsbits == 0)
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

            // get size and sign of result..
            unsigned int size = 0;
            char sign = 0;
            if (!astGetSizeSign(_settings, tok, &size, &sign))
                continue;
            if (sign != 's')  // only signed integer overflow is UB
                continue;

            integerOverflowError(tok, *value);
        }
    }
}

void CheckType::integerOverflowError(const Token *tok, const ValueFlow::Value &value)
{
    const std::string expr(tok ? tok->expressionString() : "");
    const std::string cond(value.condition ?
                           ". See condition at line " + MathLib::toString(value.condition->linenr()) + "." :
                           "");

    reportError(tok,
                value.condition ? Severity::warning : Severity::error,
                "integerOverflow",
                "Signed integer overflow for expression '"+expr+"'"+cond,
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

            unsigned int size = 0;
            char sign = 0;
            if (!astGetSizeSign(_settings, tok, &size, &sign))
                continue;

            if (sign != 'u')
                continue;

            // Check if there are signed operands that can be negative..
            std::stack<const Token *> tokens;
            tokens.push(tok->astOperand1());
            tokens.push(tok->astOperand2());
            while (!tokens.empty()) {
                const Token *tok1 = tokens.top();
                tokens.pop();
                if (!tok1)
                    continue;
                if (tok1->str() == "(")
                    continue; // Todo: properly handle casts, function calls, etc
                const Variable *var = tok1->variable();
                if (var && tok1->getValueLE(-1,_settings)) {
                    bool signedvar = true;  // assume that variable is signed since it can have a negative value
                    for (const Token *type = var->typeStartToken();; type = type->next()) {
                        if (type->isUnsigned()) {
                            signedvar = false;
                            break;
                        }
                        if (type->isSigned())
                            break;
                        if (type->isName() && !Token::Match(type, "char|short|int|long|const")) {
                            signedvar = false;
                            break;
                        }
                        if (type == var->typeEndToken())
                            break;
                    }
                    if (signedvar) {
                        signConversionError(tok1);
                        break;
                    }
                }
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

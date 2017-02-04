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
#include "checkbool.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "astutils.h"
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckBool instance;
}

static const CWE CWE398(398U);  // Indicator of Poor Code Quality
static const CWE CWE571(571U);  // Expression is Always True
static const CWE CWE587(587U);  // Assignment of a Fixed Address to a Pointer
static const CWE CWE704(704U);  // Incorrect Type Conversion or Cast

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckBool::checkIncrementBoolean()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% ++")) {
                const Variable *var = tok->variable();
                if (var && var->typeEndToken()->str() == "bool")
                    incrementBooleanError(tok);
            }
        }
    }
}

void CheckBool::incrementBooleanError(const Token *tok)
{
    reportError(
        tok,
        Severity::style,
        "incrementboolean",
        "Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead.\n"
        "The operand of a postfix increment operator may be of type bool but it is deprecated by C++ Standard (Annex D-1) and the operand is always set to true. You should assign it the value 'true' instead.",
        CWE398, false
    );
}

//---------------------------------------------------------------------------
// if (bool & bool) -> if (bool && bool)
// if (bool | bool) -> if (bool || bool)
//---------------------------------------------------------------------------
void CheckBool::checkBitwiseOnBoolean()
{
    if (!_settings->isEnabled("style"))
        return;

    // danmar: this is inconclusive because I don't like that there are
    //         warnings for calculations. Example: set_flag(a & b);
    if (!_settings->inconclusive)
        return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "(|.|return|&&|%oror%|throw|, %var% [&|]")) {
                const Variable *var = tok->next()->variable();
                if (var && var->typeEndToken()->str() == "bool") {
                    bitwiseOnBooleanError(tok->next(), var->name(), tok->strAt(2) == "&" ? "&&" : "||");
                    tok = tok->tokAt(2);
                }
            } else if (Token::Match(tok, "[&|] %var% )|.|return|&&|%oror%|throw|,") && (!tok->previous() || !tok->previous()->isExtendedOp() || tok->strAt(-1) == ")" || tok->strAt(-1) == "]")) {
                const Variable *var = tok->next()->variable();
                if (var && var->typeEndToken()->str() == "bool") {
                    bitwiseOnBooleanError(tok->next(), var->name(), tok->str() == "&" ? "&&" : "||");
                    tok = tok->tokAt(2);
                }
            }
        }
    }
}

void CheckBool::bitwiseOnBooleanError(const Token *tok, const std::string &varname, const std::string &op)
{
    reportError(tok, Severity::style, "bitwiseOnBoolean",
                "Boolean variable '" + varname + "' is used in bitwise operation. Did you mean '" + op + "'?",
                CWE398,
                true);
}

//---------------------------------------------------------------------------
//    if (!x==3) <- Probably meant to be "x!=3"
//---------------------------------------------------------------------------

static bool isBool(const Variable* var)
{
    return (var && var->typeEndToken()->str() == "bool");
}
static bool isNonBoolStdType(const Variable* var)
{
    return (var && var->typeEndToken()->isStandardType() && var->typeEndToken()->str() != "bool");
}
void CheckBool::checkComparisonOfBoolWithInt()
{
    if (!_settings->isEnabled("warning") || !_tokenizer->isCPP())
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            const Token* const left = tok->astOperand1();
            const Token* const right = tok->astOperand2();
            if (left && right && tok->isComparisonOp()) {
                if (left->isBoolean() && right->varId()) { // Comparing boolean constant with variable
                    if (tok->str() != "==" && tok->str() != "!=") {
                        comparisonOfBoolWithInvalidComparator(right, left->str());
                    }
                } else if (left->varId() && right->isBoolean()) { // Comparing variable with boolean constant
                    if (tok->str() != "==" && tok->str() != "!=") {
                        comparisonOfBoolWithInvalidComparator(right, left->str());
                    }
                }
            }
        }
    }
}

void CheckBool::comparisonOfBoolWithInvalidComparator(const Token *tok, const std::string &expression)
{
    reportError(tok, Severity::warning, "comparisonOfBoolWithInvalidComparator",
                "Comparison of a boolean value using relational operator (<, >, <= or >=).\n"
                "The result of the expression '" + expression + "' is of type 'bool'. "
                "Comparing 'bool' value using relational (<, >, <= or >=)"
                " operator could cause unexpected results.");
}

//-------------------------------------------------------------------------------
// Comparing functions which are returning value of type bool
//-------------------------------------------------------------------------------

static bool tokenIsFunctionReturningBool(const Token* tok)
{
    const Function* func = tok->function();
    if (func && Token::Match(tok, "%name% (")) {
        if (func->tokenDef && func->tokenDef->strAt(-1) == "bool") {
            return true;
        }
    }
    return false;
}

void CheckBool::checkComparisonOfFuncReturningBool()
{
    if (!_settings->isEnabled("style"))
        return;

    if (!_tokenizer->isCPP())
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functionsCount = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functionsCount; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->tokType() != Token::eComparisonOp || tok->str() == "==" || tok->str() == "!=")
                continue;
            const Token *firstToken = tok->previous();
            if (tok->strAt(-1) == ")") {
                firstToken = firstToken->link()->previous();
            }
            const Token *secondToken = tok->next();
            while (secondToken->str() == "!") {
                secondToken = secondToken->next();
            }
            const bool firstIsFunctionReturningBool = tokenIsFunctionReturningBool(firstToken);
            const bool secondIsFunctionReturningBool = tokenIsFunctionReturningBool(secondToken);
            if (firstIsFunctionReturningBool && secondIsFunctionReturningBool) {
                comparisonOfTwoFuncsReturningBoolError(firstToken->next(), firstToken->str(), secondToken->str());
            } else if (firstIsFunctionReturningBool) {
                comparisonOfFuncReturningBoolError(firstToken->next(), firstToken->str());
            } else if (secondIsFunctionReturningBool) {
                comparisonOfFuncReturningBoolError(secondToken->previous(), secondToken->str());
            }
        }
    }
}

void CheckBool::comparisonOfFuncReturningBoolError(const Token *tok, const std::string &expression)
{
    reportError(tok, Severity::style, "comparisonOfFuncReturningBoolError",
                "Comparison of a function returning boolean value using relational (<, >, <= or >=) operator.\n"
                "The return type of function '" + expression + "' is 'bool' "
                "and result is of type 'bool'. Comparing 'bool' value using relational (<, >, <= or >=)"
                " operator could cause unexpected results.", CWE398, false);
}

void CheckBool::comparisonOfTwoFuncsReturningBoolError(const Token *tok, const std::string &expression1, const std::string &expression2)
{
    reportError(tok, Severity::style, "comparisonOfTwoFuncsReturningBoolError",
                "Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator.\n"
                "The return type of function '" + expression1 + "' and function '" + expression2 + "' is 'bool' "
                "and result is of type 'bool'. Comparing 'bool' value using relational (<, >, <= or >=)"
                " operator could cause unexpected results.", CWE398, false);
}

//-------------------------------------------------------------------------------
// Comparison of bool with bool
//-------------------------------------------------------------------------------

void CheckBool::checkComparisonOfBoolWithBool()
{
    // FIXME: This checking is "experimental" because of the false positives
    //        when self checking lib/tokenize.cpp (#2617)
    if (!_settings->experimental)
        return;

    if (!_settings->isEnabled("style"))
        return;

    if (!_tokenizer->isCPP())
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->tokType() != Token::eComparisonOp || tok->str() == "==" || tok->str() == "!=")
                continue;
            bool firstTokenBool = false;

            const Token *firstToken = tok->previous();
            if (firstToken->varId()) {
                if (isBool(firstToken->variable())) {
                    firstTokenBool = true;
                }
            }
            if (!firstTokenBool)
                continue;

            bool secondTokenBool = false;
            const Token *secondToken = tok->next();
            if (secondToken->varId()) {
                if (isBool(secondToken->variable())) {
                    secondTokenBool = true;
                }
            }
            if (secondTokenBool) {
                comparisonOfBoolWithBoolError(firstToken->next(), secondToken->str());
            }
        }
    }
}

void CheckBool::comparisonOfBoolWithBoolError(const Token *tok, const std::string &expression)
{
    reportError(tok, Severity::style, "comparisonOfBoolWithBoolError",
                "Comparison of a variable having boolean value using relational (<, >, <= or >=) operator.\n"
                "The variable '" + expression + "' is of type 'bool' "
                "and comparing 'bool' value using relational (<, >, <= or >=)"
                " operator could cause unexpected results.", CWE398, false);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckBool::checkAssignBoolToPointer()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() == "=" && astIsBool(tok->astOperand2())) {
                const Token *lhs = tok->astOperand1();
                while (lhs && (lhs->str() == "." || lhs->str() == "::"))
                    lhs = lhs->astOperand2();
                if (!lhs || !lhs->variable() || !lhs->variable()->isPointer())
                    continue;

                assignBoolToPointerError(tok);
            }
        }
    }
}

void CheckBool::assignBoolToPointerError(const Token *tok)
{
    reportError(tok, Severity::error, "assignBoolToPointer",
                "Boolean value assigned to pointer.", CWE587, false);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CheckBool::checkComparisonOfBoolExpressionWithInt()
{
    if (!_settings->isEnabled("warning"))
        return;

    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (!tok->isComparisonOp())
                continue;

            const Token* numTok = 0;
            const Token* boolExpr = 0;
            bool numInRhs;
            if (astIsBool(tok->astOperand1())) {
                boolExpr = tok->astOperand1();
                numTok = tok->astOperand2();
                numInRhs = true;
            } else if (astIsBool(tok->astOperand2())) {
                boolExpr = tok->astOperand2();
                numTok = tok->astOperand1();
                numInRhs = false;
            } else {
                continue;
            }

            if (!numTok || !boolExpr)
                continue;

            if (boolExpr->isOp() && numTok->isName() && Token::Match(tok, "==|!="))
                // there is weird code such as:  ((a<b)==c)
                // but it is probably written this way by design.
                continue;

            if (numTok->isNumber()) {
                const MathLib::bigint num = MathLib::toLongNumber(numTok->str());
                if (num==0 &&
                    (numInRhs ? Token::Match(tok, ">|==|!=")
                     : Token::Match(tok, "<|==|!=")))
                    continue;
                if (num==1 &&
                    (numInRhs ? Token::Match(tok, "<|==|!=")
                     : Token::Match(tok, ">|==|!=")))
                    continue;
                comparisonOfBoolExpressionWithIntError(tok, true);
            } else if (isNonBoolStdType(numTok->variable()) && _tokenizer->isCPP())
                comparisonOfBoolExpressionWithIntError(tok, false);
        }
    }
}

void CheckBool::comparisonOfBoolExpressionWithIntError(const Token *tok, bool n0o1)
{
    if (n0o1)
        reportError(tok, Severity::warning, "compareBoolExpressionWithInt",
                    "Comparison of a boolean expression with an integer other than 0 or 1.", CWE398, false);
    else
        reportError(tok, Severity::warning, "compareBoolExpressionWithInt",
                    "Comparison of a boolean expression with an integer.", CWE398, false);
}


void CheckBool::pointerArithBool()
{
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();

    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        if (scope->type != Scope::eIf && scope->type != Scope::eWhile && scope->type != Scope::eDo && scope->type != Scope::eFor)
            continue;
        const Token* tok = scope->classDef->next()->astOperand2();
        if (scope->type == Scope::eFor) {
            tok = Token::findsimplematch(scope->classDef->tokAt(2), ";");
            if (tok)
                tok = tok->astOperand2();
            if (tok)
                tok = tok->astOperand1();
        } else if (scope->type == Scope::eDo)
            tok = (scope->classEnd->tokAt(2)) ? scope->classEnd->tokAt(2)->astOperand2() : nullptr;

        pointerArithBoolCond(tok);
    }
}

void CheckBool::pointerArithBoolCond(const Token *tok)
{
    if (!tok)
        return;
    if (Token::Match(tok, "&&|%oror%")) {
        pointerArithBoolCond(tok->astOperand1());
        pointerArithBoolCond(tok->astOperand2());
        return;
    }
    if (tok->str() != "+" && tok->str() != "-")
        return;

    if (tok->astOperand1() &&
        tok->astOperand2() &&
        tok->astOperand1()->isName() &&
        tok->astOperand1()->variable() &&
        tok->astOperand1()->variable()->isPointer() &&
        tok->astOperand2()->isNumber())
        pointerArithBoolError(tok);
}

void CheckBool::pointerArithBoolError(const Token *tok)
{
    reportError(tok,
                Severity::error,
                "pointerArithBool",
                "Converting pointer arithmetic result to bool. The bool is always true unless there is undefined behaviour.\n"
                "Converting pointer arithmetic result to bool. The boolean result is always true unless there is pointer arithmetic overflow, and overflow is undefined behaviour. Probably a dereference is forgotten.", CWE571, false);
}

void CheckBool::checkAssignBoolToFloat()
{
    if (!_tokenizer->isCPP())
        return;
    if (!_settings->isEnabled("style"))
        return;
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            if (tok->str() == "=" && astIsBool(tok->astOperand2())) {
                const Token *lhs = tok->astOperand1();
                while (lhs && (lhs->str() == "." || lhs->str() == "::"))
                    lhs = lhs->astOperand2();
                if (!lhs || !lhs->variable())
                    continue;
                const Variable* var = lhs->variable();
                if (var && var->isFloatingType() && !var->isArrayOrPointer())
                    assignBoolToFloatError(tok->next());
            }
        }
    }
}

void CheckBool::assignBoolToFloatError(const Token *tok)
{
    reportError(tok, Severity::style, "assignBoolToFloat",
                "Boolean value assigned to floating point variable.", CWE704, false);
}

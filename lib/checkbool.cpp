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
#include "checkbool.h"
#include "mathlib.h"
#include "symboldatabase.h"
//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckBool instance;
}

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
                if (tok->varId()) {
                    const Variable *var = tok->variable();

                    if (var && var->typeEndToken()->str() == "bool")
                        incrementBooleanError(tok);
                }
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
        "The operand of a postfix increment operator may be of type bool but it is deprecated by C++ Standard (Annex D-1) and the operand is always set to true. You should assign it the value 'true' instead."
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
                "Boolean variable '" + varname + "' is used in bitwise operation. Did you mean '" + op + "'?", true);
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
            if ((!Token::Match(tok->previous(), "%cop%")) && Token::Match(tok->next(), "%comp%") && (!Token::Match(tok->tokAt(3), "%cop%"))) {
                const Token* const right = tok->tokAt(2);
                if ((tok->varId() && right->isNumber()) || (tok->isNumber() && right->varId())) { // Comparing variable with number
                    const Token* varTok = tok;
                    const Token* numTok = right;
                    if (tok->isNumber() && right->varId()) // num with var
                        std::swap(varTok, numTok);
                    if (isBool(varTok->variable()) && // Variable has to be a boolean
                        ((tok->strAt(1) != "==" && tok->strAt(1) != "!=") ||
                         (MathLib::toLongNumber(numTok->str()) != 0 && MathLib::toLongNumber(numTok->str()) != 1))) { // == 0 and != 0 are allowed, for C also == 1 and != 1
                        comparisonOfBoolWithIntError(varTok, numTok->str(), tok->strAt(1) == "==" || tok->strAt(1) == "!=");
                    }
                } else if (tok->isBoolean() && right->varId()) { // Comparing boolean constant with variable
                    if (isNonBoolStdType(right->variable())) { // Variable has to be of non-boolean standard type
                        comparisonOfBoolWithIntError(right, tok->str(), false);
                    } else if (tok->strAt(1) != "==" && tok->strAt(1) != "!=") {
                        comparisonOfBoolWithInvalidComparator(right, tok->str());
                    }
                } else if (tok->varId() && right->isBoolean()) { // Comparing variable with boolean constant
                    if (isNonBoolStdType(tok->variable())) { // Variable has to be of non-boolean standard type
                        comparisonOfBoolWithIntError(tok, right->str(), false);
                    } else if (tok->strAt(1) != "==" && tok->strAt(1) != "!=") {
                        comparisonOfBoolWithInvalidComparator(right, tok->str());
                    }
                } else if (tok->isNumber() && right->isBoolean()) { // number constant with boolean constant
                    comparisonOfBoolWithIntError(tok, right->str(), false);
                } else if (tok->isBoolean() && right->isNumber()) { // number constant with boolean constant
                    comparisonOfBoolWithIntError(tok, tok->str(), false);
                } else if (tok->varId() && right->varId()) { // Comparing two variables, one of them boolean, one of them integer
                    const Variable* var1 = right->variable();
                    const Variable* var2 = tok->variable();
                    if (isBool(var1) && isNonBoolStdType(var2)) // Comparing boolean with non-bool standard type
                        comparisonOfBoolWithIntError(tok, var1->name(), false);
                    else if (isNonBoolStdType(var1) && isBool(var2)) // Comparing non-bool standard type with boolean
                        comparisonOfBoolWithIntError(tok, var2->name(), false);
                }
            }
        }
    }
}

void CheckBool::comparisonOfBoolWithIntError(const Token *tok, const std::string &expression, bool n0o1)
{
    if (n0o1)
        reportError(tok, Severity::warning, "comparisonOfBoolWithInt",
                    "Comparison of a boolean with an integer that is neither 1 nor 0.\n"
                    "The expression '" + expression + "' is of type 'bool' "
                    "and it is compared against an integer value that is "
                    "neither 1 nor 0.");
    else
        reportError(tok, Severity::warning, "comparisonOfBoolWithInt",
                    "Comparison of a boolean with an integer.\n"
                    "The expression '" + expression + "' is of type 'bool' "
                    "and it is compared against an integer value.");
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

void CheckBool::checkComparisonOfFuncReturningBool()
{
    if (!_settings->isEnabled("style"))
        return;

    if (!_tokenizer->isCPP())
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token* tok = scope->classStart->next(); tok != scope->classEnd; tok = tok->next()) {
            if (tok->type() != Token::eComparisonOp || tok->str() == "==" || tok->str() == "!=")
                continue;
            const Token *first_token;
            bool first_token_func_of_type_bool = false;
            if (tok->strAt(-1) == ")") {
                first_token = tok->previous()->link()->previous();
            } else {
                first_token = tok->previous();
            }
            if (Token::Match(first_token, "%var% (") && !Token::Match(first_token->previous(), "::|.")) {
                const Function* func = first_token->function();
                if (func && func->tokenDef && func->tokenDef->strAt(-1) == "bool") {
                    first_token_func_of_type_bool = true;
                }
            }

            Token *second_token = tok->next();
            bool second_token_func_of_type_bool = false;
            while (second_token->str()=="!") {
                second_token = second_token->next();
            }
            if (Token::Match(second_token, "%var% (") && !Token::Match(second_token->previous(), "::|.")) {
                const Function* func = second_token->function();
                if (func && func->tokenDef && func->tokenDef->strAt(-1) == "bool") {
                    second_token_func_of_type_bool = true;
                }
            }

            if ((first_token_func_of_type_bool == true) && (second_token_func_of_type_bool == true)) {
                comparisonOfTwoFuncsReturningBoolError(first_token->next(), first_token->str(), second_token->str());
            } else if (first_token_func_of_type_bool == true) {
                comparisonOfFuncReturningBoolError(first_token->next(), first_token->str());
            } else if (second_token_func_of_type_bool == true) {
                comparisonOfFuncReturningBoolError(second_token->previous(), second_token->str());
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
                " operator could cause unexpected results.");
}

void CheckBool::comparisonOfTwoFuncsReturningBoolError(const Token *tok, const std::string &expression1, const std::string &expression2)
{
    reportError(tok, Severity::style, "comparisonOfTwoFuncsReturningBoolError",
                "Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator.\n"
                "The return type of function '" + expression1 + "' and function '" + expression2 + "' is 'bool' "
                "and result is of type 'bool'. Comparing 'bool' value using relational (<, >, <= or >=)"
                " operator could cause unexpected results.");
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
            if (tok->type() != Token::eComparisonOp || tok->str() == "==" || tok->str() == "!=")
                continue;
            bool first_token_bool = false;
            bool second_token_bool = false;

            const Token *first_token = tok->previous();
            if (first_token->varId()) {
                if (isBool(first_token->variable())) {
                    first_token_bool = true;
                }
            }
            const Token *second_token = tok->next();
            if (second_token->varId()) {
                if (isBool(second_token->variable())) {
                    second_token_bool = true;
                }
            }
            if ((first_token_bool == true) && (second_token_bool == true)) {
                comparisonOfBoolWithBoolError(first_token->next(), first_token->str());
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
                " operator could cause unexpected results.");
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
            if (Token::Match(tok, "%var% = %bool% ;")) {
                // check if there is a deref
                // *x.p = true;  // <- don't warn
                // x.p = true;   // <- warn
                const Token *prev = tok;
                while (Token::Match(prev->tokAt(-2), "%var% ."))
                    prev = prev->tokAt(-2);
                if (Token::Match(prev->previous(), "[*.)]"))
                    continue;

                // Is variable a pointer?
                const Variable *var1(tok->variable());
                if (var1 && var1->isPointer())
                    assignBoolToPointerError(tok);
            }
        }
    }
}

void CheckBool::assignBoolToPointerError(const Token *tok)
{
    reportError(tok, Severity::error, "assignBoolToPointer",
                "Boolean value assigned to pointer.");
}

/**
 * @brief Is the result of the LHS expression non-bool?
 * @param tok last token in lhs
 * @return true => lhs result is non-bool. false => lhs result type is unknown or bool
 */
static bool isNonBoolLHSExpr(const Token *tok)
{
    // return value. only return true if we "know" it's a non-bool expression
    bool nonBoolExpr = false;

    for (; tok; tok = tok->previous()) {
        if (tok->str() == ")") {
            if (!Token::Match(tok->link()->previous(), "&&|%oror%|( ("))
                tok = tok->link();
        } else if (tok->str() == "(" || tok->str() == "[")
            break;
        else if (tok->isNumber())
            nonBoolExpr = true;
        else if (tok->isArithmeticalOp()) {
            return true;
        } else if (tok->isComparisonOp() || (tok->str() == "!" && tok->previous()->str()=="("))
            return false;
        else if (Token::Match(tok,"[;{}=?:&|^,]"))
            break;
        else if (Token::Match(tok, "&&|%oror%|and|or"))
            break;
    }

    return nonBoolExpr;
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
            // Skip template parameters
            if (tok->str() == "<" && tok->link()) {
                tok = tok->link();
                continue;
            }

            const Token* numTok = 0;
            const Token* opTok = 0;
            char op = 0;
            if (Token::Match(tok, "&&|%oror% %any% ) %comp% %any%")) {
                numTok = tok->tokAt(4);
                opTok = tok->tokAt(3);
                if (Token::Match(opTok, "<|>"))
                    op = opTok->str()[0];
            } else if (Token::Match(tok, "%any% %comp% ( %any% &&|%oror%")) {
                numTok = tok;
                opTok = tok->next();
                if (Token::Match(opTok, "<|>"))
                    op = opTok->str()[0]=='>'?'<':'>';
            }

            else if (Token::Match(tok, "! %var% %comp% %any%") && !isNonBoolLHSExpr(tok)) {
                numTok = tok->tokAt(3);
                opTok = tok->tokAt(2);
                if (Token::Match(opTok, "<|>"))
                    op = opTok->str()[0];
            } else if (Token::Match(tok->previous(), "(|&&|%oror% %num% %comp% !")) {
                const Token *rhs = tok->tokAt(3);
                while (rhs) {
                    if (rhs->str() == "!") {
                        if (Token::simpleMatch(rhs, "! ("))
                            rhs = rhs->next()->link();
                        rhs = rhs->next();
                    } else if (rhs->isName() || rhs->isNumber())
                        rhs = rhs->next();
                    else
                        break;
                }
                if (Token::Match(rhs, "&&|%oror%|)")) {
                    numTok = tok;
                    opTok = tok->next();
                    if (Token::Match(opTok, "<|>"))
                        op = opTok->str()[0]=='>'?'<':'>';
                }
            }

            // boolean result in lhs compared with <|<=|>|>=
            else if (tok->isComparisonOp() && !Token::Match(tok,"==|!=") && !isNonBoolLHSExpr(tok->previous())) {
                const Token *lhs = tok;
                while (NULL != (lhs = lhs->previous())) {
                    if ((lhs->isName() && !Token::Match(lhs,"or|and")) || lhs->isNumber())
                        continue;
                    if (lhs->isArithmeticalOp())
                        continue;
                    if (Token::Match(lhs, ")|]")) {
                        if (Token::Match(lhs->link()->previous(), "%var% ("))
                            lhs = lhs->link();
                        continue;
                    }
                    break;
                }
                if (lhs && (lhs->isComparisonOp() || lhs->str() == "!")) {
                    if (_tokenizer->isCPP() && tok->str() == ">" &&
                        (Token::Match(lhs->previous(), "%var% <") || lhs->str() == ">"))
                        continue;
                    while (NULL != (lhs = lhs->previous())) {
                        if ((lhs->isName() && lhs->str() != "return") || lhs->isNumber())
                            continue;
                        if (Token::Match(lhs,"[+-*/.]"))
                            continue;
                        if (Token::Match(lhs, ")|]")) {
                            lhs = lhs->previous();
                            continue;
                        }
                        break;
                    }

                    std::string expression;
                    for (const Token *t = lhs ? lhs->next() : _tokenizer->tokens(); t != tok; t = t->next()) {
                        if (!expression.empty())
                            expression += ' ';
                        expression += t->str();
                    }

                    comparisonOfBoolWithInvalidComparator(tok, expression);
                }
            }

            if (numTok && opTok) {
                if (numTok->isNumber()) {
                    if (((numTok->str() != "0" && numTok->str() != "1") || !Token::Match(opTok, "!=|==")) && !((op == '<' && numTok->str() == "1") || (op == '>' && numTok->str() == "0")))
                        comparisonOfBoolExpressionWithIntError(tok, true);
                } else if (isNonBoolStdType(numTok->variable()))
                    comparisonOfBoolExpressionWithIntError(tok, false);
            }
        }
    }
}

void CheckBool::comparisonOfBoolExpressionWithIntError(const Token *tok, bool n0o1)
{
    if (n0o1)
        reportError(tok, Severity::warning, "compareBoolExpressionWithInt",
                    "Comparison of a boolean expression with an integer other than 0 or 1.");
    else
        reportError(tok, Severity::warning, "compareBoolExpressionWithInt",
                    "Comparison of a boolean expression with an integer.");
}

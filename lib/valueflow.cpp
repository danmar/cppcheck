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

#include "valueflow.h"
#include "errorlogger.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"

#include <iostream>


static void printvalues(const Token *tok)
{
    if (tok->values.empty())
        std::cout << "empty";
    for (std::list<ValueFlow::Value>::const_iterator it = tok->values.begin(); it != tok->values.end(); ++it)
        std::cout << " " << (it->intvalue);
    std::cout << std::endl;
}

static void bailout(TokenList *tokenlist, ErrorLogger *errorLogger, const Token *tok, const std::string &what)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
    callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(tok,tokenlist));
    ErrorLogger::ErrorMessage errmsg(callstack, Severity::debug, "ValueFlow bailout: " + what, "valueFlowBailout", false);
    errorLogger->reportErr(errmsg);
}

static bool bailoutFunctionPar(const Token *tok)
{
    // passing variable to subfunction?
    const bool addr = tok && Token::Match(tok->previous(), "&");
    if (!tok || !Token::Match(tok->tokAt(addr?-2:-1), "[(,] &| %var% [,)]"))
        return false;

    // goto start of function call and get argnr
    unsigned int argnr = 0;
    while (tok && tok->str() != "(") {
        if (tok->str() == ",")
            ++argnr;
        else if (tok->str() == ")")
            tok = tok->link();
        tok = tok->previous();
    }
    tok = tok ? tok->previous() : NULL;
    if (!Token::Match(tok,"%var% ("))
        return false; // not a function => dont bailout

    if (!tok->function())
        return true; // unknown function bailout

    const Variable *arg = tok->function()->getArgumentVar(argnr);

    return arg && !arg->isConst() && arg->isReference();
}

static const Token * skipValueInConditionalExpression(const Token *tok)
{
    while (tok && !Token::Match(tok, "%oror%|&&|?|:")) {
        while (Token::Match(tok->astParent(), "%oror%|&&|?") && tok->astParent()->astOperand1() == tok)
            tok = tok->astParent();
        tok = tok->astParent();
    }
    return tok;
}

static bool bailoutSelfAssignment(const Token * const tok)
{
    const Token *parent = tok;
    while (parent) {
        const Token *op = parent;
        parent = parent->astParent();

        // Assignment where lhs variable exists in rhs => return true
        if (parent                       != NULL         &&
            parent->astOperand2()        == op           &&
            parent->astOperand1()        != NULL         &&
            parent->str()                == "="          &&
            parent->astOperand1()->str() == tok->str())
            return true;
    }
    return false;
}

static void valueFlowBeforeCondition(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        unsigned int varid;
        MathLib::bigint num;
        const Variable *var;
        if (tok->isComparisonOp() && tok->astOperand1() && tok->astOperand2()) {
            if (tok->astOperand1()->isName() && tok->astOperand2()->isNumber()) {
                varid = tok->astOperand1()->varId();
                var = tok->astOperand1()->variable();
                num = MathLib::toLongNumber(tok->astOperand2()->str());
            } else if (tok->astOperand1()->isNumber() && tok->astOperand2()->isName()) {
                varid = tok->astOperand2()->varId();
                var = tok->astOperand2()->variable();
                num = MathLib::toLongNumber(tok->astOperand1()->str());
            } else {
                continue;
            }
        } else if (Token::Match(tok->previous(), "if|while ( %var% %oror%|&&|)") ||
                   Token::Match(tok, "%oror%|&& %var% %oror%|&&|)")) {
            varid = tok->next()->varId();
            var = tok->next()->variable();
            num = 0;
        } else if (tok->str() == "!" && tok->astOperand1() && tok->astOperand1()->isName()) {
            varid = tok->astOperand1()->varId();
            var = tok->astOperand1()->variable();
            num = 0;
        } else {
            continue;
        }

        if (varid == 0U)
            continue;

        // bailout: global non-const variables
        if (var && var->isGlobal() && !var->isConst()) {
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok, "global variable " + var->nameToken()->str());
            continue;
        }

        const ValueFlow::Value val(tok, num);
        ValueFlow::Value val2;
        if (var && num==1U && Token::Match(tok,"<=|>=")) {
            bool isunsigned = var->typeEndToken()->isUnsigned();
            for (const Token* type = var->typeStartToken(); type != var->typeEndToken(); type = type->next())
                isunsigned |= type->isUnsigned();
            if (isunsigned)
                val2 = ValueFlow::Value(tok,0);
        }

        for (Token *tok2 = tok->previous(); ; tok2 = tok2->previous()) {
            if (!tok2) {
                if (settings->debugwarnings) {
                    std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
                    callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(tok,tokenlist));
                    ErrorLogger::ErrorMessage errmsg(callstack, Severity::debug, "iterated too far", "debugValueFlowBeforeCondition", false);
                    errorLogger->reportErr(errmsg);
                }
                break;
            }

            if (tok2->varId() == varid) {
                // bailout: assignment
                if (Token::Match(tok2->previous(), "!!* %var% =")) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "assignment of " + tok2->str());
                    break;
                }

                // bailout: variable is used in rhs in assignment to itself
                if (bailoutSelfAssignment(tok2)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + tok2->str() + " is used in rhs in assignment to itself");
                    break;
                }

                // assigned by subfunction?
                if (bailoutFunctionPar(tok2)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "possible assignment of " + tok2->str() + " by subfunction");
                    break;
                }

                // skip if variable is conditionally used in ?: expression
                if (const Token *parent = skipValueInConditionalExpression(tok2)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                tok2,
                                "no simplification of " + tok2->str() + " within " + (Token::Match(parent,"[?:]") ? "?:" : parent->str()) + " expression");
                    continue;
                }

                tok2->values.push_back(val);
                if (val2.condition)
                    tok2->values.push_back(val2);
                if (var && tok2 == var->nameToken())
                    break;
            }

            if (tok2->str() == "}") {
                if (Token::findmatch(tok2->link(), "%varid%", tok2, varid)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " stopping on }");
                    break;
                } else {
                    tok2 = tok2->link();
                }
            } else if (var && var->isGlobal() && tok2->str() == "{") {
                if (!Token::Match(tok2->previous(), ")|else {"))
                    break;
                if (Token::Match(tok2->previous(), ") {") &&
                    !Token::Match(tok2->linkAt(-1)->previous(), "if|for|while ("))
                    break;
            }
        }
    }
}

static void valueFlowForLoop(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "for ("))
            continue;

        tok = tok->tokAt(2);
        if (!Token::Match(tok,"%type%| %var% = %num% ;")) { // TODO: don't use %num%
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok, "For loop not handled");
            continue;
        }
        Token * const vartok = tok->tokAt(Token::Match(tok, "%var% =") ? 0 : 1);
        const MathLib::bigint num1 = MathLib::toLongNumber(vartok->strAt(2));
        if (vartok->varId() == 0U)
            continue;
        tok = vartok->tokAt(4);
        const Token *num2tok = 0;
        if (Token::Match(tok, "%varid% <|<=|!=", vartok->varId())) {
            tok = tok->next();
            num2tok = tok->astOperand2();
            if (num2tok && num2tok->str() == "(" && !num2tok->astOperand2())
                num2tok = num2tok->astOperand1();
            if (!Token::Match(num2tok, "%num%"))
                num2tok = 0;
        }
        if (!num2tok) {
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok, "For loop not handled");
            continue;
        }
        const MathLib::bigint num2 = MathLib::toLongNumber(num2tok ? num2tok->str() : "0") - ((tok->str()=="<=") ? 0 : 1);
        while (tok && tok->str() != ";")
            tok = tok->next();
        if (!num2tok || !Token::Match(tok, "; %varid% ++ ) {", vartok->varId())) {
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok, "For loop not handled");
            continue;
        }

        Token * const bodyStart = tok->tokAt(4);
        const Token * const bodyEnd   = bodyStart->link();

        // Is variable modified inside for loop
        bool modified = false;
        for (const Token *tok2 = bodyStart->next(); tok2 != bodyEnd; tok2 = tok2->next()) {
            if (Token::Match(tok2, "%varid% =", vartok->varId())) {
                modified = true;
                break;
            }
        }
        if (modified)
            continue;

        for (Token *tok2 = bodyStart->next(); tok2 != bodyEnd; tok2 = tok2->next()) {
            if (tok2->varId() == vartok->varId()) {
                tok2->values.push_back(ValueFlow::Value(num1));
                tok2->values.push_back(ValueFlow::Value(num2));
            }

            if (tok2->str() == "{") {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop variable " + vartok->str() + " stopping on {");
                break;
            }
        }
    }
}

static void valueFlowSubFunction(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    std::list<ValueFlow::Value> argvalues;
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "[(,]"))
            continue;

        // passing value(s) to function
        if (Token::Match(tok, "[(,] %var% [,)]") && !tok->next()->values.empty())
            argvalues = tok->next()->values;
        else if (Token::Match(tok, "[(,] %num% [,)]")) {
            argvalues.clear();
            argvalues.push_back(ValueFlow::Value(MathLib::toLongNumber(tok->next()->str())));
        } else {
            // bool operator => values 1/0 are passed to function..
            const Token *op = tok->next();
            while (op && op->astParent() && !Token::Match(op->astParent(), "[(,]"))
                op = op->astParent();
            if (Token::Match(op, "%comp%|%oror%|&&|!")) {
                argvalues.clear();
                argvalues.push_back(ValueFlow::Value(0));
                argvalues.push_back(ValueFlow::Value(1));
            } else {
                // possible values are unknown..
                continue;
            }
        }

        const Token * const argumentToken = tok->next();

        // is this a function call?
        const Token *ftok = tok;
        while (ftok && ftok->str() != "(")
            ftok = ftok->astParent();
        if (!ftok || !ftok->astOperand1() || !ftok->astOperand2() || !ftok->astOperand1()->function())
            continue;

        // Get argument nr
        unsigned int argnr = 0;
        for (const Token *argtok = ftok->next(); argtok && argtok != argumentToken; argtok = argtok->nextArgument())
            ++ argnr;

        // Get function argument, and check if parameter is passed by value
        const Function * const function = ftok->astOperand1()->function();
        const Variable * const arg = function ? function->getArgumentVar(argnr) : NULL;
        if (!Token::Match(arg ? arg->typeStartToken() : NULL, "const| %type% %var% ,|)"))
            continue;

        // Function scope..
        const Scope * const functionScope = function ? function->functionScope : NULL;
        if (!functionScope)
            continue;

        // Set value in function scope..
        const unsigned int varid2 = arg->nameToken()->varId();
        for (const Token *tok2 = functionScope->classStart->next(); tok2 != functionScope->classEnd; tok2 = tok2->next()) {
            if (Token::Match(tok2, "%varid% !!=", varid2)) {
                std::list<ValueFlow::Value> &values = const_cast<Token*>(tok2)->values;
                values.insert(values.begin(), argvalues.begin(), argvalues.end());
            } else if (tok2->str() == "{") {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "parameter " + arg->nameToken()->str());
                break;
            }
        }
    }
}

void ValueFlow::setValues(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next())
        tok->values.clear();

    valueFlowForLoop(tokenlist, errorLogger, settings);
    valueFlowBeforeCondition(tokenlist, errorLogger, settings);
    valueFlowSubFunction(tokenlist, errorLogger, settings);
}

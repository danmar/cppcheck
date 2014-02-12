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
#include <stack>


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

static bool bailoutFunctionPar(const Token *tok, const ValueFlow::Value &value, const Settings *settings, bool *inconclusive)
{
    if (!tok)
        return false;

    // address of variable
    const bool addressOf = tok && Token::Match(tok->previous(), "&");

    // passing variable to subfunction?
    if (Token::Match(tok->tokAt(-2), ") & %var% [,)]") && Token::Match(tok->linkAt(-2)->previous(), "[,(] ("))
        ;
    else if (Token::Match(tok->tokAt(addressOf?-2:-1), "[(,] &| %var% [,)]"))
        ;
    else
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

    if (!tok->function()) {
        // if value is 0 and the library says 0 is invalid => dont bailout
        if (value.intvalue==0 && settings->library.isnullargbad(tok->str(), 1+argnr))
            return false;
        // inconclusive => don't bailout
        if (inconclusive && !addressOf && settings->inconclusive) {
            *inconclusive = true;
            return false;
        }
        return true;
    }

    const Variable *arg = tok->function()->getArgumentVar(argnr);

    if (addressOf && !(arg && arg->isConst()))
        return true;

    return arg && !arg->isConst() && arg->isReference();
}

/**
 * Should value be skipped because it's hidden inside && || or ?: expression.
 * Example: ((x!=NULL) && (*x == 123))
 * If 'valuetok' points at the x in '(*x == 123)'. Then the '&&' will be returned.
 * @param valuetok original variable token
 * @return NULL=>don't skip, non-NULL=>The operator token that cause the skip. For instance the '&&'.
 * */
static const Token * skipValueInConditionalExpression(const Token * const valuetok)
{
    // Walk up the ast
    const Token *prev = valuetok;
    for (const Token *tok = valuetok->astParent(); tok; tok = tok->astParent()) {
        const bool prevIsLhs = (prev == tok->astOperand1());
        prev = tok;

        if (prevIsLhs || !Token::Match(tok, "%oror%|&&|?|:"))
            continue;

        // Is variable protected in LHS..
        std::stack<const Token *> tokens;
        tokens.push(tok->astOperand1());
        while (!tokens.empty()) {
            const Token * const tok2 = tokens.top();
            tokens.pop();
            if (!tok2)
                continue;
            if (tok2 != valuetok && tok2->str() == valuetok->str())
                return tok;
            tokens.push(tok2->astOperand2());
            tokens.push(tok2->astOperand1());
        }
    }
    return NULL;
}

static bool bailoutSelfAssignment(const Token * const tok)
{
    const Token *parent = tok;
    while (parent) {
        const Token *op = parent;
        parent = parent->astParent();

        // Assignment where lhs variable exists in rhs => return true
        if (parent                         != NULL         &&
            parent->astOperand2()          == op           &&
            parent->astOperand1()          != NULL         &&
            parent->str()                  == "=") {
            for (const Token *lhs = parent->astOperand1(); lhs; lhs = lhs->astOperand1()) {
                if (lhs->varId() == tok->varId())
                    return true;
                if (lhs->astOperand2() && lhs->astOperand2()->varId() == tok->varId())
                    return true;
            }
        }
    }
    return false;
}

/** set ValueFlow value and perform calculations if possible */
static void setTokenValue(Token* tok, const ValueFlow::Value &value)
{
    // if value already exists, don't add it again
    std::list<ValueFlow::Value>::iterator it;
    for (it = tok->values.begin(); it != tok->values.end(); ++it) {
        if (it->intvalue == value.intvalue) {
            if (it->inconclusive && !value.inconclusive) {
                *it = value;
                break;
            }
            return;
        }
    }

    if (it == tok->values.end()) {
        tok->values.push_back(value);
        it = tok->values.end();
        --it;
    }

    Token *parent = const_cast<Token*>(tok->astParent());

    // Cast..
    if (parent && parent->str() == "(" && tok == parent->link()->next()) {
        setTokenValue(parent,value);
    }

    // Calculations..
    else if (parent && parent->isArithmeticalOp() && parent->astOperand1() && parent->astOperand2()) {
        std::list<ValueFlow::Value>::const_iterator value1, value2;
        for (value1 = parent->astOperand1()->values.begin(); value1 != parent->astOperand1()->values.end(); ++value1) {
            for (value2 = parent->astOperand2()->values.begin(); value2 != parent->astOperand2()->values.end(); ++value2) {
                if (value1->varId == 0U || value2->varId == 0U ||
                    (value1->varId == value2->varId && value1->varvalue == value2->varvalue)) {
                    ValueFlow::Value result(0);
                    result.condition = value1->condition ? value1->condition : value2->condition;
                    result.inconclusive = value1->inconclusive | value2->inconclusive;
                    result.varId = (value1->varId != 0U) ? value1->varId : value2->varId;
                    result.varvalue = (result.varId == value1->varId) ? value1->intvalue : value2->intvalue;
                    switch (parent->str()[0]) {
                    case '+':
                        result.intvalue = value1->intvalue + value2->intvalue;
                        setTokenValue(parent, result);
                        break;
                    case '-':
                        result.intvalue = value1->intvalue - value2->intvalue;
                        setTokenValue(parent, result);
                        break;
                    case '*':
                        result.intvalue = value1->intvalue * value2->intvalue;
                        setTokenValue(parent, result);
                        break;
                    case '/':
                        if (value2->intvalue == 0)
                            break;
                        result.intvalue = value1->intvalue / value2->intvalue;
                        setTokenValue(parent, result);
                        break;
                    case '%':
                        if (value2->intvalue == 0)
                            break;
                        result.intvalue = value1->intvalue % value2->intvalue;
                        setTokenValue(parent, result);
                        break;
                    }
                }
            }
        }
    }
}

static void valueFlowNumber(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->isNumber() && MathLib::isInt(tok->str()))
            setTokenValue(tok, ValueFlow::Value(MathLib::toLongNumber(tok->str())));
    }
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

        if (varid == 0U || !var)
            continue;

        // bailout: global non-const variables
        if (!(var->isLocal() || var->isArgument()) && !var->isConst()) {
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok, "global variable " + var->nameToken()->str());
            continue;
        }

        // bailout: for/while-condition, variable is changed in while loop
        for (const Token *tok2 = tok; tok2; tok2 = tok2->astParent()) {
            if (tok2->astParent() || tok2->str() != "(" || !Token::simpleMatch(tok2->link(), ") {"))
                continue;

            if (Token::Match(tok2->previous(), "for|while (")) {
                const Token *start = tok2->link()->next();
                const Token *end   = start->link();

                if (tok2->astOperand2()->str() == ";" &&
                    tok2->astOperand2()->astOperand2() &&
                    tok2->astOperand2()->astOperand2()->str() == ";")
                    start = tok2->astOperand2()->astOperand2();

                if (Token::findmatch(start,"++|-- %varid%",end,varid) ||
                    Token::findmatch(start,"%varid% ++|--|=",end,varid)) {
                    varid = 0U;
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok, "variable " + var->nameToken()->str() + " used in loop");
                }
            }

            // if,macro => bailout
            else if (Token::simpleMatch(tok2->previous(), "if (") && tok2->previous()->isExpandedMacro()) {
                varid = 0U;
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok, "variable " + var->nameToken()->str() + ", condition is defined in macro");
            }
        }
        if (varid == 0U)
            continue;

        // extra logic for unsigned variables 'i>=1' => possible value can also be 0
        ValueFlow::Value val(tok, num);
        val.varId = varid;
        ValueFlow::Value val2;
        if (num==1U && Token::Match(tok,"<=|>=")) {
            bool isunsigned = false;
            for (const Token* type = var->typeStartToken(); type && type->varId() == 0U; type = type->next())
                isunsigned |= type->isUnsigned();
            if (isunsigned) {
                val2 = ValueFlow::Value(tok,0);
                val2.varId = varid;
            }
        }
        if (Token::Match(tok,"<|>")) {
            if (num!=0)
                continue;
            bool isunsigned = false;
            for (const Token* type = var->typeStartToken(); type && type->varId() == 0U; type = type->next())
                isunsigned |= type->isUnsigned();
            if (!isunsigned)
                continue;
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

                // increment/decrement
                if (Token::Match(tok2->previous(), "[;{}] %var% ++|-- ;"))
                    val.intvalue += (tok2->strAt(1)=="++") ? -1 : 1;
                else if (Token::Match(tok2->tokAt(-2), "[;{}] ++|-- %var% ;"))
                    val.intvalue += (tok2->strAt(-1)=="++") ? -1 : 1;
                else if (Token::Match(tok2->previous(), "++|-- %var%") || Token::Match(tok2, "%var% ++|--")) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "increment/decrement of " + tok2->str());
                    break;
                }

                // bailout: variable is used in rhs in assignment to itself
                if (bailoutSelfAssignment(tok2)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + tok2->str() + " is used in rhs in assignment to itself");
                    break;
                }

                // assigned by subfunction?
                bool inconclusive = false;
                if (bailoutFunctionPar(tok2,val2.condition ? val2 : val, settings, &inconclusive)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "possible assignment of " + tok2->str() + " by subfunction");
                    break;
                }
                val.inconclusive |= inconclusive;
                val2.inconclusive |= inconclusive;

                // skip if variable is conditionally used in ?: expression
                if (const Token *parent = skipValueInConditionalExpression(tok2)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                tok2,
                                "no simplification of " + tok2->str() + " within " + (Token::Match(parent,"[?:]") ? "?:" : parent->str()) + " expression");
                    continue;
                }

                setTokenValue(tok2, val);
                if (val2.condition)
                    setTokenValue(tok2,val2);
                if (var && tok2 == var->nameToken())
                    break;
            }

            // skip sizeof..
            if (tok2->str() == ")" && Token::Match(tok2->link()->previous(), "typeof|sizeof ("))
                tok2 = tok2->link();

            if (tok2->str() == "}") {
                if (Token::findmatch(tok2->link(), "%varid%", tok2, varid)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " stopping on }");
                    break;
                } else {
                    tok2 = tok2->link();
                }
            } else if (tok2->str() == "{") {
                // if variable is assigned in loop don't look before the loop
                if (tok2->previous() &&
                    (Token::Match(tok2->previous(), "do") ||
                     (tok2->strAt(-1) == ")" && Token::Match(tok2->linkAt(-1)->previous(), "for|while (")))) {

                    const Token *start = tok2;
                    const Token *end   = start->link();
                    if (Token::findmatch(start,"++|--| %varid% ++|--|=",end,varid)) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " is assigned in loop. so valueflow analysis bailout when start of loop is reached.");
                        break;
                    }
                }

                // Global variable : stop when leaving the function scope
                if (!var->isLocal()) {
                    if (!Token::Match(tok2->previous(), ")|else|do {"))
                        break;
                    if (Token::Match(tok2->previous(), ") {") &&
                        !Token::Match(tok2->linkAt(-1)->previous(), "if|for|while ("))
                        break;
                }
            } else if (tok2->str() == ";") {
                const Token *parent = tok2->previous();
                while (parent && !Token::Match(parent, "return|break|continue|goto"))
                    parent = parent->astParent();
                // reaching a break/continue/return
                if (parent) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " stopping on " + parent->str());
                    break;
                }
            }

            // goto label
            if (Token::Match(tok2, "[;{}] %var% :")) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " stopping on goto label");
                break;
            }
        }
    }
}

static void valueFlowAfterAssign(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        // Assignment
        if ((tok->str() != "=") || (tok->astParent()))
            continue;

        // Lhs should be a variable
        if (!tok->astOperand1() || !tok->astOperand1()->isName())
            continue;
        const unsigned int varid = tok->astOperand1()->varId();
        if (varid == 0U)
            continue;
        const Variable *var = tok->astOperand1()->variable();
        if (!var || !var->isLocal())
            continue;
        const Token * endToken = 0;
        for (const Token *tok2 = var->typeStartToken(); tok2; tok2 = tok2->previous()) {
            if (tok2->str() == "{") {
                endToken = tok2->link();
                break;
            }
        }

        // Rhs values..
        if (!tok->astOperand2() || tok->astOperand2()->values.empty())
            continue;
        std::list<ValueFlow::Value> values = tok->astOperand2()->values;

        unsigned int number_of_if = 0;

        for (Token *tok2 = tok; tok2 && tok2 != endToken; tok2 = tok2->next()) {
            if (Token::Match(tok2, "sizeof|typeof|typeid ("))
                tok2 = tok2->linkAt(1);

            // conditional block of code that assigns variable..
            if (Token::Match(tok2, "%var% (") && Token::Match(tok2->linkAt(1), ") {")) {
                Token * const start = tok2->linkAt(1)->next();
                Token * const end   = start->link();
                if (Token::findmatch(start, "%varid%", end, varid)) {
                    // TODO: don't check noreturn scopes
                    if (number_of_if > 0U) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " is assigned in conditional code");
                        break;
                    }

                    // Remove conditional values
                    std::list<ValueFlow::Value>::iterator it;
                    for (it = values.begin(); it != values.end();) {
                        if (it->condition || it->conditional)
                            values.erase(it++);
                        else
                            ++it;
                    }
                }

                // noreturn scopes..
                if (number_of_if > 0 &&
                    (Token::findmatch(start, "return|continue|break", end) ||
                     (Token::Match(end,"} else {") && Token::findmatch(end, "return|continue|break", end->linkAt(2))))) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + ". noreturn conditional scope.");
                    break;
                }

                if (Token::findmatch(start, "++|-- %varid%", end, varid) ||
                    Token::findmatch(start, "%varid% ++|--|=", end, varid)) {
                    if (number_of_if == 0 &&
                        Token::simpleMatch(tok2, "if (") &&
                        !(Token::simpleMatch(end, "} else {") &&
                          (Token::findmatch(end, "%varid%", end->linkAt(2), varid) ||
                           Token::findmatch(end, "return|continue|break", end->linkAt(2))))) {
                        ++number_of_if;
                        tok2 = end;
                    } else {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " is assigned in conditional code");
                        break;
                    }
                }
            }

            else if (tok2->str() == "}") {
                ++number_of_if;

                // Set "conditional" flag for all values
                std::list<ValueFlow::Value>::iterator it;
                for (it = values.begin(); it != values.end(); ++it)
                    it->conditional = true;

                if (Token::simpleMatch(tok2,"} else {"))
                    tok2 = tok2->linkAt(2);
            }

            if (tok2->varId() == varid) {
                // bailout: assignment
                if (Token::Match(tok2->previous(), "!!* %var% =")) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "assignment of " + tok2->str());
                    break;
                }

                // bailout increment/decrement for now..
                if (Token::Match(tok2->previous(), "++|-- %var%") || Token::Match(tok2, "%var% ++|--")) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "increment/decrement of " + tok2->str());
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

                {
                    std::list<ValueFlow::Value>::const_iterator it;
                    for (it = values.begin(); it != values.end(); ++it)
                        setTokenValue(tok2, *it);
                }

                // assigned by subfunction?
                bool inconclusive = false;
                if (bailoutFunctionPar(tok2, ValueFlow::Value(), settings, &inconclusive)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "possible assignment of " + tok2->str() + " by subfunction");
                    break;
                }
                if (inconclusive) {
                    std::list<ValueFlow::Value>::iterator it;
                    for (it = values.begin(); it != values.end(); ++it)
                        it->inconclusive = true;
                }
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
                ValueFlow::Value value1(num1);
                value1.varId = tok2->varId();
                setTokenValue(tok2, value1);

                ValueFlow::Value value2(num2);
                value2.varId = tok2->varId();
                setTokenValue(tok2, value2);
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
        if (!Token::Match(arg ? arg->typeStartToken() : NULL, "%type% %var% ,|)"))
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

    valueFlowNumber(tokenlist);
    valueFlowForLoop(tokenlist, errorLogger, settings);
    valueFlowBeforeCondition(tokenlist, errorLogger, settings);
    valueFlowAfterAssign(tokenlist, errorLogger, settings);
    valueFlowSubFunction(tokenlist, errorLogger, settings);
}

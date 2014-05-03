/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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


static void execute(const Token *expr,
                    std::map<unsigned int, MathLib::bigint> * const programMemory,
                    MathLib::bigint *result,
                    bool *error);

//static void printvalues(const Token *tok)
//{
//    if (tok->values.empty())
//        std::cout << "empty";
//    for (std::list<ValueFlow::Value>::const_iterator it = tok->values.begin(); it != tok->values.end(); ++it)
//        std::cout << " " << (it->intvalue);
//    std::cout << std::endl;
//}

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
    const bool addressOf = tok && Token::simpleMatch(tok->previous(), "&");

    // passing variable to subfunction?
    if (Token::Match(tok->tokAt(-2), ") & %var% [,)]") && Token::Match(tok->linkAt(-2)->previous(), "[,(] ("))
        ;
    else if (Token::Match(tok->tokAt(addressOf?-2:-1), "[(,] &| %var% [,)]"))
        ;
    else
        return false;

    // reinterpret_cast etc..
    if (Token::Match(tok->tokAt(-3), "> ( & %var% ) [,)]") &&
        tok->linkAt(-3) &&
        Token::Match(tok->linkAt(-3)->tokAt(-2), "[,(] %type% <"))
        tok = tok->linkAt(-3);

    // goto start of function call and get argnr
    unsigned int argnr = 0;
    while (tok && tok->str() != "(") {
        if (tok->str() == ",")
            ++argnr;
        else if (tok->str() == ")")
            tok = tok->link();
        tok = tok->previous();
    }
    tok = tok ? tok->previous() : nullptr;
    if (!Token::Match(tok,"%var% ("))
        return false; // not a function => do not bailout

    if (!tok->function()) {
        // if value is 0 and the library says 0 is invalid => do not bailout
        if (value.intvalue==0 && settings->library.isnullargbad(tok->str(), 1+argnr))
            return false;
        // addressOf => inconclusive
        if (!addressOf) {
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
 * Is condition always false when variable has given value?
 * \param condition   top ast token in condition
 * \param varid       variable id for variable
 * \param value       value of variable
 */
static bool conditionIsFalse(const Token *condition, unsigned int varid, const ValueFlow::Value &value)
{
    if (!condition)
        return false;
    if (condition->str() == "&&") {
        bool result1 = conditionIsFalse(condition->astOperand1(), varid, value);
        bool result2 = result1 ? true : conditionIsFalse(condition->astOperand2(), varid, value);
        return result2;
    }
    std::map<unsigned int, MathLib::bigint> programMemory;
    programMemory[varid] = value.intvalue;
    MathLib::bigint result = 0;
    bool error = false;
    execute(condition, &programMemory, &result, &error);
    return !error && result == 0;
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
    return nullptr;
}

static bool bailoutSelfAssignment(const Token * const tok)
{
    const Token *parent = tok;
    while (parent) {
        const Token *op = parent;
        parent = parent->astParent();

        // Assignment where lhs variable exists in rhs => return true
        if (parent                         != nullptr      &&
            parent->astOperand2()          == op           &&
            parent->astOperand1()          != nullptr      &&
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

static bool isVariableChanged(const Token *start, const Token *end, const unsigned int varid)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() == varid) {
            if (Token::Match(tok, "%var% ="))
                return true;

            const Token *parent = tok->astParent();
            while (parent && parent->str() == ".")
                parent = parent->astParent();
            if (Token::Match(parent, "++|--"))
                return true;
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

static void valueFlowBitAnd(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->str() != "&")
            continue;

        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        MathLib::bigint number;
        if (MathLib::isInt(tok->astOperand1()->str()))
            number = MathLib::toLongNumber(tok->astOperand1()->str());
        else if (MathLib::isInt(tok->astOperand2()->str()))
            number = MathLib::toLongNumber(tok->astOperand2()->str());
        else
            continue;

        int bit = 0;
        while (bit <= 60 && ((1LL<<bit) < number))
            ++bit;

        if ((1LL<<bit) == number) {
            setTokenValue(tok, ValueFlow::Value(0));
            setTokenValue(tok, ValueFlow::Value(number));
        }
    }
}

static void valueFlowBeforeCondition(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        unsigned int varid=0;
        MathLib::bigint num=0;
        const Variable *var=0;
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
                const Token * const start = tok2->link()->next();
                const Token * const end   = start->link();

                if (isVariableChanged(start,end,varid)) {
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

                if (Token::Match(tok2->previous(), "sizeof|.")) {
                    const Token *prev = tok2->previous();
                    while (Token::Match(prev,"%var%|.") && prev->str() != "sizeof")
                        prev = prev->previous();
                    if (Token::Match(prev,"sizeof"))
                        continue;
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

            // goto label
            if (Token::Match(tok2, "[;{}] %var% :")) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2->next(), "variable " + var->nameToken()->str() + " stopping on goto label");
                break;
            }

            if (tok2->str() == "}") {
                if (Token::findmatch(tok2->link(), "%varid%", tok2, varid)) {
                    if (settings->debugwarnings) {
                        std::string errmsg = "variable ";
                        if (var)
                            errmsg += var->nameToken()->str() + " ";
                        errmsg += "stopping on }";
                        bailout(tokenlist, errorLogger, tok2, errmsg);
                    }
                    break;
                } else {
                    tok2 = tok2->link();
                }
            } else if (tok2->str() == "{") {
                // if variable is assigned in loop don't look before the loop
                if (tok2->previous() &&
                    (Token::simpleMatch(tok2->previous(), "do") ||
                     (tok2->strAt(-1) == ")" && Token::Match(tok2->linkAt(-1)->previous(), "for|while (")))) {

                    const Token *start = tok2;
                    const Token *end   = start->link();
                    if (isVariableChanged(start,end,varid)) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " is assigned in loop. so valueflow analysis bailout when start of loop is reached.");
                        break;
                    }
                }

                // Global variable : stop when leaving the function scope
                if (!var->isLocal()) {
                    if (!Token::Match(tok2->previous(), ")|else|do {"))
                        break;
                    if (Token::simpleMatch(tok2->previous(), ") {") &&
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

        const bool constValue = tok->astOperand2()->isNumber();
        int indentlevel = 0;
        unsigned int number_of_if = 0;
        int varusagelevel = -1;
        bool returnStatement = false;  // current statement is a return, stop analysis at the ";"

        for (Token *tok2 = tok; tok2 && tok2 != endToken; tok2 = tok2->next()) {
            if (indentlevel >= 0 && tok2->str() == "{")
                ++indentlevel;
            else if (indentlevel >= 0 && tok2->str() == "}")
                --indentlevel;

            if (Token::Match(tok2, "sizeof|typeof|typeid ("))
                tok2 = tok2->linkAt(1);

            // conditional block of code that assigns variable..
            else if (Token::Match(tok2, "%var% (") && Token::simpleMatch(tok2->linkAt(1), ") {")) {
                // Should scope be skipped because variable value is checked?
                bool skip = false;
                for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end(); ++it) {
                    if (conditionIsFalse(tok2->next()->astOperand2(), varid, *it)) {
                        skip = true;
                        break;
                    }
                }
                if (skip) {
                    // goto '{'
                    tok2 = tok2->linkAt(1)->next();
                    // goto '}'
                    tok2 = tok2->link();
                    continue;
                }

                Token * const start = tok2->linkAt(1)->next();
                Token * const end   = start->link();
                bool varusage = (indentlevel >= 0 && constValue && number_of_if == 0U) ?
                                isVariableChanged(start,end,varid) :
                                (nullptr != Token::findmatch(start, "%varid%", end, varid));
                if (varusage) {
                    varusagelevel = indentlevel;

                    // TODO: don't check noreturn scopes
                    if (number_of_if > 0U || Token::findmatch(tok2, "%varid%", start, varid)) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " is assigned in conditional code");
                        break;
                    }

                    if (var->isStatic()) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + " bailout when conditional code that contains var is seen");
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
                if ((number_of_if > 0 || Token::findmatch(tok2, "%varid%", start, varid)) &&
                    (Token::findmatch(start, "return|continue|break", end) ||
                     (Token::simpleMatch(end,"} else {") && Token::findmatch(end, "return|continue|break", end->linkAt(2))))) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + ". noreturn conditional scope.");
                    break;
                }

                if (isVariableChanged(start, end, varid)) {
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

            else if (tok2->str() == "}" && indentlevel == varusagelevel) {
                ++number_of_if;

                // Set "conditional" flag for all values
                std::list<ValueFlow::Value>::iterator it;
                for (it = values.begin(); it != values.end(); ++it)
                    it->conditional = true;

                if (Token::simpleMatch(tok2,"} else {"))
                    tok2 = tok2->linkAt(2);
            }

            else if (indentlevel <= 0 && Token::Match(tok2, "break|continue|goto")) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "variable " + var->nameToken()->str() + ". noreturn conditional scope.");
                break;
            }

            else if (indentlevel <= 0 && tok2->str() == "return")
                returnStatement = true;

            else if (returnStatement && tok2->str() == ";")
                break;

            if (tok2->varId() == varid) {
                // bailout: assignment
                if (Token::Match(tok2->previous(), "!!* %var% %op%") && tok2->next()->isAssignmentOp()) {
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

                // bailout: possible assignment using >>
                if (Token::Match(tok2->previous(), ">> %var% >>|;")) {
                    const Token *parent = tok2->previous();
                    while (Token::simpleMatch(parent,">>"))
                        parent = parent->astParent();
                    if (!parent) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "Possible assignment of " + tok2->str() + " using >>");
                        break;
                    }
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

static void execute(const Token *expr,
                    std::map<unsigned int, MathLib::bigint> * const programMemory,
                    MathLib::bigint *result,
                    bool *error)
{
    if (!expr)
        *error = true;

    else if (expr->isNumber())
        *result = MathLib::toLongNumber(expr->str());

    else if (expr->varId() > 0) {
        const std::map<unsigned int, MathLib::bigint>::const_iterator var = programMemory->find(expr->varId());
        if (var == programMemory->end())
            *error = true;
        else
            *result = var->second;
    }

    else if (expr->isComparisonOp()) {
        MathLib::bigint result1(0), result2(0);
        execute(expr->astOperand1(), programMemory, &result1, error);
        execute(expr->astOperand2(), programMemory, &result2, error);
        if (expr->str() == "<")
            *result = result1 < result2;
        else if (expr->str() == "<=")
            *result = result1 <= result2;
        else if (expr->str() == ">")
            *result = result1 > result2;
        else if (expr->str() == ">=")
            *result = result1 >= result2;
        else if (expr->str() == "==")
            *result = result1 == result2;
        else if (expr->str() == "!=")
            *result = result1 != result2;
    }

    else if (expr->str() == "=") {
        execute(expr->astOperand2(), programMemory, result, error);
        if (!*error && expr->astOperand1() && expr->astOperand1()->varId())
            (*programMemory)[expr->astOperand1()->varId()] = *result;
        else
            *error = true;
    }

    else if (expr->str() == "++" || expr->str() == "--") {
        if (!expr->astOperand1() || expr->astOperand1()->varId() == 0U)
            *error = true;
        else {
            std::map<unsigned int, MathLib::bigint>::iterator var = programMemory->find(expr->astOperand1()->varId());
            if (var == programMemory->end())
                *error = true;
            else {
                if (var->second == 0 &&
                    expr->str() == "--" &&
                    expr->astOperand1()->variable() &&
                    expr->astOperand1()->variable()->typeStartToken()->isUnsigned())
                    *error = true; // overflow
                *result = var->second + (expr->str() == "++" ? 1 : -1);
                var->second = *result;
            }
        }
    }

    else if (expr->isArithmeticalOp() && expr->astOperand1() && expr->astOperand2()) {
        MathLib::bigint result1(0), result2(0);
        execute(expr->astOperand1(), programMemory, &result1, error);
        execute(expr->astOperand2(), programMemory, &result2, error);
        if (expr->str() == "+")
            *result = result1 + result2;
        else if (expr->str() == "-")
            *result = result1 - result2;
        else if (expr->str() == "*")
            *result = result1 * result2;
        else if (result2 == 0)
            *error = true;
        else if (expr->str() == "/")
            *result = result1 / result2;
        else if (expr->str() == "%")
            *result = result1 % result2;
    }

    else if (expr->str() == "&&") {
        bool error1 = false;
        execute(expr->astOperand1(), programMemory, result, &error1);
        if (!error1 && *result == 0)
            *result = 0;
        else {
            bool error2 = false;
            execute(expr->astOperand2(), programMemory, result, &error2);
            if (error1 && error2)
                *error = true;
            if (error2)
                *result = 1;
            else
                *result = !!*result;
        }
    }

    else if (expr->str() == "||") {
        execute(expr->astOperand1(), programMemory, result, error);
        if (*result == 0 && *error == false)
            execute(expr->astOperand2(), programMemory, result, error);
    }

    else
        *error = true;
}

static bool valueFlowForLoop1(const Token *tok, unsigned int * const varid, MathLib::bigint * const num1, MathLib::bigint * const num2)
{
    tok = tok->tokAt(2);
    if (!Token::Match(tok,"%type%| %var% ="))
        return false;
    const Token * const vartok = tok->tokAt(Token::Match(tok, "%var% =") ? 0 : 1);
    if (vartok->varId() == 0U)
        return false;
    *varid = vartok->varId();
    const Token * const num1tok = Token::Match(vartok->tokAt(2), "%num% ;") ? vartok->tokAt(2) : nullptr;
    if (num1tok)
        *num1 = MathLib::toLongNumber(num1tok->str());
    tok = vartok->tokAt(2);
    while (Token::Match(tok, "%var%|%num%|%or%|+|-|*|/|&|[|]|("))
        tok = (tok->str() == "(") ? tok->link()->next() : tok->next();
    if (!tok || tok->str() != ";")
        return false;
    tok = tok->next();
    const Token *num2tok = nullptr;
    if (Token::Match(tok, "%varid% <|<=|!=", vartok->varId())) {
        tok = tok->next();
        num2tok = tok->astOperand2();
        if (num2tok && num2tok->str() == "(" && !num2tok->astOperand2())
            num2tok = num2tok->astOperand1();
        if (!Token::Match(num2tok, "%num%"))
            num2tok = 0;
    }
    if (!num2tok)
        return false;
    *num2 = MathLib::toLongNumber(num2tok->str()) - ((tok->str()=="<=") ? 0 : 1);
    if (!num1tok)
        *num1 = *num2;
    while (tok && tok->str() != ";")
        tok = tok->next();
    if (!num2tok || !Token::Match(tok, "; %varid% ++ ) {", vartok->varId()))
        return false;
    return true;
}

static bool valueFlowForLoop2(const Token *tok,
                              std::map<unsigned int, MathLib::bigint> *memory1,
                              std::map<unsigned int, MathLib::bigint> *memory2)
{
    const Token *firstExpression  = tok->next()->astOperand2()->astOperand1();
    const Token *secondExpression = tok->next()->astOperand2()->astOperand2()->astOperand1();
    const Token *thirdExpression = tok->next()->astOperand2()->astOperand2()->astOperand2();

    std::map<unsigned int, MathLib::bigint> programMemory;
    MathLib::bigint result(0);
    bool error = false;
    execute(firstExpression, &programMemory, &result, &error);
    if (error)
        return false;
    execute(secondExpression, &programMemory, &result, &error);

    std::map<unsigned int, MathLib::bigint> startMemory(programMemory);
    std::map<unsigned int, MathLib::bigint> endMemory;

    unsigned int maxcount = 10000;
    while (result != 0 && !error && --maxcount) {
        endMemory = programMemory;
        execute(thirdExpression, &programMemory, &result, &error);
        if (!error)
            execute(secondExpression, &programMemory, &result, &error);
    }

    memory1->swap(startMemory);
    if (!error)
        memory2->swap(endMemory);

    return true;
}

static void valueFlowForLoopSimplify(Token * const bodyStart, const unsigned int varid, const MathLib::bigint value, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    const Token * const bodyEnd = bodyStart->link();

    // Is variable modified inside for loop
    if (isVariableChanged(bodyStart, bodyEnd, varid))
        return;

    for (Token *tok2 = bodyStart->next(); tok2 != bodyEnd; tok2 = tok2->next()) {
        if (tok2->varId() == varid) {
            const Token * parent = tok2->astParent();
            while (parent) {
                const Token * const p = parent;
                parent = parent->astParent();
                if (parent && parent->str() == ":")
                    break;
                if (parent && parent->str() == "?") {
                    if (parent->astOperand2() != p)
                        parent = NULL;
                    break;
                }
            }
            if (parent) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop variable " + tok2->str() + " stopping on ?");
                continue;
            }

            ValueFlow::Value value1(value);
            value1.varId = tok2->varId();
            setTokenValue(tok2, value1);
        }

        else if (Token::Match(tok2, ") {") && Token::findmatch(tok2->link(), "%varid%", tok2, varid)) {
            if (Token::findmatch(tok2, "continue|break|return", tok2->linkAt(1), varid)) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop variable bailout on conditional continue|break|return");
                break;
            }
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "For loop variable skipping conditional scope");
            tok2 = tok2->next()->link();
            if (Token::Match(tok2, "} else {")) {
                if (Token::findmatch(tok2, "continue|break|return", tok2->linkAt(2), varid)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "For loop variable bailout on conditional continue|break|return");
                    break;
                }

                tok2 = tok2->linkAt(2);
            }
        }
    }
}

static void valueFlowForLoop(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "for (") ||
            !Token::simpleMatch(tok->next()->astOperand2(), ";") ||
            !Token::simpleMatch(tok->next()->astOperand2()->astOperand2(), ";"))
            continue;

        Token * const bodyStart = tok->linkAt(1)->next();

        unsigned int varid(0);
        MathLib::bigint num1(0), num2(0);

        if (valueFlowForLoop1(tok, &varid, &num1, &num2)) {
            valueFlowForLoopSimplify(bodyStart, varid, num1, tokenlist, errorLogger, settings);
            valueFlowForLoopSimplify(bodyStart, varid, num2, tokenlist, errorLogger, settings);
        } else {
            std::map<unsigned int, MathLib::bigint> mem1, mem2;
            if (valueFlowForLoop2(tok, &mem1, &mem2)) {
                std::map<unsigned int, MathLib::bigint>::const_iterator it;
                for (it = mem1.begin(); it != mem1.end(); ++it)
                    valueFlowForLoopSimplify(bodyStart, it->first, it->second, tokenlist, errorLogger, settings);
                for (it = mem2.begin(); it != mem2.end(); ++it)
                    valueFlowForLoopSimplify(bodyStart, it->first, it->second, tokenlist, errorLogger, settings);
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
        const Variable * const arg = function ? function->getArgumentVar(argnr) : nullptr;
        if (!Token::Match(arg ? arg->typeStartToken() : nullptr, "%type% %var% ,|)"))
            continue;

        // Function scope..
        const Scope * const functionScope = function ? function->functionScope : nullptr;
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
    valueFlowBitAnd(tokenlist);
    valueFlowForLoop(tokenlist, errorLogger, settings);
    valueFlowBeforeCondition(tokenlist, errorLogger, settings);
    valueFlowAfterAssign(tokenlist, errorLogger, settings);
    valueFlowSubFunction(tokenlist, errorLogger, settings);
}

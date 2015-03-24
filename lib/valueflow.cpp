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

#include "valueflow.h"
#include "errorlogger.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include <stack>

static void execute(const Token *expr,
                    std::map<unsigned int, MathLib::bigint> * const programMemory,
                    MathLib::bigint *result,
                    bool *error);

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
    if (Token::Match(tok->tokAt(-2), ") & %name% [,)]") && Token::Match(tok->linkAt(-2)->previous(), "[,(] ("))
        ;
    else if (Token::Match(tok->tokAt(addressOf?-2:-1), "[(,] &| %name% [,)]"))
        ;
    else
        return false;

    // reinterpret_cast etc..
    if (Token::Match(tok->tokAt(-3), "> ( & %name% ) [,)]") &&
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
    if (!Token::Match(tok,"%name% ("))
        return false; // not a function => do not bailout

    if (!tok->function()) {
        // if value is 0 and the library says 0 is invalid => do not bailout
        if (value.intvalue==0 && settings->library.isnullargbad(tok, 1+argnr))
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
 * \param programMemory   program memory
 */
static bool conditionIsFalse(const Token *condition, const std::map<unsigned int, MathLib::bigint> &programMemory)
{
    if (!condition)
        return false;
    if (condition->str() == "&&") {
        const bool result1 = conditionIsFalse(condition->astOperand1(), programMemory);
        const bool result2 = result1 ? true : conditionIsFalse(condition->astOperand2(), programMemory);
        return result2;
    }
    std::map<unsigned int, MathLib::bigint> progmem(programMemory);
    MathLib::bigint result = 0;
    bool error = false;
    execute(condition, &progmem, &result, &error);
    return !error && result == 0;
}

/**
 * Is condition always true when variable has given value?
 * \param condition   top ast token in condition
 * \param programMemory   program memory
 */
static bool conditionIsTrue(const Token *condition, const std::map<unsigned int, MathLib::bigint> &programMemory)
{
    if (!condition)
        return false;
    if (condition->str() == "||") {
        const bool result1 = conditionIsTrue(condition->astOperand1(), programMemory);
        const bool result2 = result1 ? true : conditionIsTrue(condition->astOperand2(), programMemory);
        return result2;
    }
    std::map<unsigned int, MathLib::bigint> progmem(programMemory);
    bool error = false;
    MathLib::bigint result = 0;
    execute(condition, &progmem, &result, &error);
    return !error && result == 1;
}

/**
 * Get program memory by looking backwards from given token.
 */
static std::map<unsigned int, MathLib::bigint> getProgramMemory(const Token *tok, unsigned int varid, const ValueFlow::Value &value)
{
    std::map<unsigned int, MathLib::bigint> programMemory;
    programMemory[varid] = value.intvalue;
    if (value.varId)
        programMemory[value.varId] = value.varvalue;
    const std::map<unsigned int, MathLib::bigint> programMemory1(programMemory);
    int indentlevel = 0;
    for (const Token *tok2 = tok; tok2; tok2 = tok2->previous()) {
        if (Token::Match(tok2, "[;{}] %var% = %num% ;")) {
            const Token *vartok = tok2->next();
            const Token *numtok = tok2->tokAt(3);
            if (programMemory.find(vartok->varId()) == programMemory.end())
                programMemory[vartok->varId()] = MathLib::toLongNumber(numtok->str());
        }
        if (Token::Match(tok2, "[;{}] %varid% = %var% ;", varid)) {
            const Token *vartok = tok2->tokAt(3);
            programMemory[vartok->varId()] = value.intvalue;
        }
        if (tok2->str() == "{") {
            if (indentlevel <= 0)
                break;
            --indentlevel;
        }
        if (tok2->str() == "}") {
            const Token *cond = tok2->link();
            cond = Token::simpleMatch(cond->previous(), ") {") ? cond->linkAt(-1) : nullptr;
            if (cond && conditionIsFalse(cond->astOperand2(), programMemory1))
                tok2 = cond->previous();
            else if (cond && conditionIsTrue(cond->astOperand2(), programMemory1)) {
                ++indentlevel;
                continue;
            } else
                break;
        }
    }
    return programMemory;
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

static bool isReturn(const Token *tok)
{
    if (!tok)
        return false;

    const Token *prev = tok->previous();
    if (prev && Token::simpleMatch(prev->previous(), "} ;"))
        prev = prev->previous();

    if (Token::simpleMatch(prev, "}")) {
        if (Token::simpleMatch(prev->link()->tokAt(-2), "} else {"))
            return isReturn(prev) && isReturn(prev->link()->tokAt(-2));
        if (Token::simpleMatch(prev->link()->previous(), ") {") &&
            Token::simpleMatch(prev->link()->linkAt(-1)->previous(), "switch (") &&
            !Token::findsimplematch(prev->link(), "break", prev)) {
            return true;
        }
    } else if (Token::simpleMatch(prev, ";")) {
        // noreturn function
        if (Token::simpleMatch(prev->previous(), ") ;") && Token::Match(prev->linkAt(-1)->tokAt(-2), "[;{}] %name% ("))
            return true;
        // return/goto statement
        prev = prev->previous();
        while (prev && !Token::Match(prev, ";|{|}|return|goto|throw"))
            prev = prev->previous();
        return prev && prev->isName();
    }
    return false;
}

static bool isVariableChanged(const Token *start, const Token *end, const unsigned int varid)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() == varid) {
            if (Token::Match(tok, "%name% ="))
                return true;

            const Token *parent = tok->astParent();
            while (Token::Match(parent, ".|::"))
                parent = parent->astParent();
            if (parent && parent->type() == Token::eIncDecOp)
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
        // different intvalue => continue
        if (it->intvalue != value.intvalue)
            continue;

        // different tokvalue => continue
        if ((it->tokvalue == nullptr) != (value.tokvalue == nullptr))
            continue;
        if ((value.tokvalue != nullptr) && (it->tokvalue != value.tokvalue) && (it->tokvalue->str() != value.tokvalue->str()))
            continue;

        // same value, but old value is inconclusive so replace it
        if (it->inconclusive && !value.inconclusive) {
            *it = value;
            break;
        }

        // Same value already exists, don't  add new value
        return;
    }

    if (it == tok->values.end()) {
        tok->values.push_back(value);
        it = tok->values.end();
        --it;
        if (it->varId == 0)
            it->varId = tok->varId();
    }

    Token *parent = const_cast<Token*>(tok->astParent());
    if (!parent)
        return;

    // Cast..
    if (parent->str() == "(" && tok == parent->link()->next()) {
        setTokenValue(parent,value);
    }

    // Calculations..
    else if (parent->isArithmeticalOp() && parent->astOperand1() && parent->astOperand2()) {
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

    // Array element
    else if (parent->str() == "[" && parent->astOperand1() && parent->astOperand2()) {
        std::list<ValueFlow::Value>::const_iterator value1, value2;
        for (value1 = parent->astOperand1()->values.begin(); value1 != parent->astOperand1()->values.end(); ++value1) {
            if (!value1->tokvalue)
                continue;
            for (value2 = parent->astOperand2()->values.begin(); value2 != parent->astOperand2()->values.end(); ++value2) {
                if (value2->tokvalue)
                    continue;
                if (value1->varId == 0U || value2->varId == 0U ||
                    (value1->varId == value2->varId && value1->varvalue == value2->varvalue)) {
                    ValueFlow::Value result(0);
                    result.condition = value1->condition ? value1->condition : value2->condition;
                    result.inconclusive = value1->inconclusive | value2->inconclusive;
                    result.varId = (value1->varId != 0U) ? value1->varId : value2->varId;
                    result.varvalue = (result.varId == value1->varId) ? value1->intvalue : value2->intvalue;
                    if (value1->tokvalue->type() == Token::eString) {
                        const std::string s = value1->tokvalue->strValue();
                        const MathLib::bigint index = value2->intvalue;
                        if (index >= 0 && index < s.size()) {
                            result.intvalue = s[index];
                            setTokenValue(parent, result);
                        }
                    } else if (value1->tokvalue->str() == "{") {
                        MathLib::bigint index = value2->intvalue;
                        const Token *element = value1->tokvalue->next();
                        while (index > 0 && element->str() != "}") {
                            if (element->str() == ",")
                                --index;
                            if (Token::Match(element, "[{}()[]]"))
                                break;
                            element = element->next();
                        }
                        if (Token::Match(element, "%num% [,}]")) {
                            result.intvalue = MathLib::toLongNumber(element->str());
                            setTokenValue(parent, result);
                        }
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

static void valueFlowString(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->type() == Token::eString) {
            ValueFlow::Value strvalue;
            strvalue.tokvalue = tok;
            setTokenValue(tok, strvalue);
        }
    }
}

static void valueFlowArray(TokenList *tokenlist)
{
    std::map<unsigned int, const Token *> constantArrays;

    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "const %type% %var% [ %num%| ] = {")) {
            const Token *vartok = tok->tokAt(2);
            const Token *rhstok = vartok->next()->link()->tokAt(2);
            constantArrays[vartok->varId()] = rhstok;
            tok = rhstok->link();
        }

        if (Token::Match(tok, "const char %var% [ %num%| ] = %str% ;")) {
            const Token *vartok = tok->tokAt(2);
            const Token *strtok = vartok->next()->link()->tokAt(2);
            constantArrays[vartok->varId()] = strtok;
            tok = strtok->next();
        }

        if (tok->varId() > 0U) {
            const std::map<unsigned int, const Token *>::const_iterator it = constantArrays.find(tok->varId());
            if (it != constantArrays.end()) {
                ValueFlow::Value value;
                value.tokvalue = it->second;
                setTokenValue(tok, value);
            }
        }
    }
}

static void valueFlowPointerAlias(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        // not address of
        if (tok->str() != "&" || tok->astOperand2())
            continue;

        // parent should be a '='
        if (!Token::simpleMatch(tok->astParent(), "="))
            continue;

        // child should be some buffer or variable
        if (!Token::Match(tok->astOperand1(), "%name%|.|[|;"))
            continue;

        ValueFlow::Value value;
        value.tokvalue = tok;
        setTokenValue(tok, value);
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

static void valueFlowBeforeCondition(TokenList *tokenlist, SymbolDatabase *symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    const std::size_t functions = symboldatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symboldatabase->functionScopes[i];
        for (Token* tok = const_cast<Token*>(scope->classStart); tok != scope->classEnd; tok = tok->next()) {
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
            } else if (Token::Match(tok->previous(), "if|while ( %name% %oror%|&&|)") ||
                       Token::Match(tok, "%oror%|&& %name% %oror%|&&|)")) {
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
                    bailout(tokenlist, errorLogger, tok, "global variable " + var->name());
                continue;
            }

            // bailout: for/while-condition, variable is changed in while loop
            for (const Token *tok2 = tok; tok2; tok2 = tok2->astParent()) {
                if (tok2->astParent() || tok2->str() != "(" || !Token::simpleMatch(tok2->link(), ") {"))
                    continue;

                // Variable changed in 3rd for-expression
                if (Token::simpleMatch(tok2->previous(), "for (")) {
                    if (isVariableChanged(tok2->astOperand2()->astOperand2(), tok2->link(), varid)) {
                        varid = 0U;
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok, "variable " + var->name() + " used in loop");
                    }
                }

                // Variable changed in loop code
                if (Token::Match(tok2->previous(), "for|while (")) {
                    const Token * const start = tok2->link()->next();
                    const Token * const end   = start->link();

                    if (isVariableChanged(start,end,varid)) {
                        varid = 0U;
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok, "variable " + var->name() + " used in loop");
                    }
                }

                // if,macro => bailout
                else if (Token::simpleMatch(tok2->previous(), "if (") && tok2->previous()->isExpandedMacro()) {
                    varid = 0U;
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok, "variable " + var->name() + ", condition is defined in macro");
                }
            }
            if (varid == 0U)
                continue;

            // extra logic for unsigned variables 'i>=1' => possible value can also be 0
            ValueFlow::Value val(tok, num);
            val.varId = varid;
            if (Token::Match(tok, "<|>")) {
                if (num != 0)
                    continue;
                if (!var->typeStartToken()->isUnsigned())
                    continue;
            }
            ValueFlow::Value val2;
            if (num==1U && Token::Match(tok,"<=|>=")) {
                if (var->typeStartToken()->isUnsigned()) {
                    val2 = ValueFlow::Value(tok,0);
                    val2.varId = varid;
                }
            }
            for (Token *tok2 = tok->previous(); ; tok2 = tok2->previous()) {
                if (!tok2 || tok2->next() == scope->classStart) {
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
                    if (Token::Match(tok2->previous(), "!!* %name% =")) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "assignment of " + tok2->str());
                        break;
                    }

                    // increment/decrement
                    if (Token::Match(tok2->previous(), "[;{}] %name% ++|-- ;"))
                        val.intvalue += (tok2->strAt(1)=="++") ? -1 : 1;
                    else if (Token::Match(tok2->tokAt(-2), "[;{}] ++|-- %name% ;"))
                        val.intvalue += (tok2->strAt(-1)=="++") ? -1 : 1;
                    else if (Token::Match(tok2->previous(), "++|-- %name%") || Token::Match(tok2, "%name% ++|--")) {
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
                        while (Token::Match(prev,"%name%|.") && prev->str() != "sizeof")
                            prev = prev->previous();
                        if (prev && prev->str() == "sizeof")
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
                if (Token::Match(tok2, "[;{}] %name% :")) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2->next(), "variable " + var->name() + " stopping on goto label");
                    break;
                }

                if (tok2->str() == "}") {
                    const Token *vartok = Token::findmatch(tok2->link(), "%varid%", tok2, varid);
                    while (Token::Match(vartok, "%name% = %num% ;") && !vartok->tokAt(2)->getValue(num))
                        vartok = Token::findmatch(vartok->next(), "%varid%", tok2, varid);
                    if (vartok) {
                        if (settings->debugwarnings) {
                            std::string errmsg = "variable ";
                            if (var)
                                errmsg += var->name() + " ";
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
                                bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " is assigned in loop. so valueflow analysis bailout when start of loop is reached.");
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
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " stopping on " + parent->str());
                        break;
                    }
                }
            }
        }
    }
}

static void removeValues(std::list<ValueFlow::Value> &values, const std::list<ValueFlow::Value> &valuesToRemove)
{
    for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end();) {
        bool found = false;
        for (std::list<ValueFlow::Value>::const_iterator it2 = valuesToRemove.begin(); it2 != valuesToRemove.end(); ++it2) {
            if (it->intvalue == it2->intvalue) {
                found = true;
                break;
            }
        }
        if (found)
            values.erase(it++);
        else
            ++it;
    }
}

static bool valueFlowForward(Token * const               startToken,
                             const Token * const         endToken,
                             const Variable * const      var,
                             const unsigned int          varid,
                             std::list<ValueFlow::Value> values,
                             const bool                  constValue,
                             TokenList * const           tokenlist,
                             ErrorLogger * const         errorLogger,
                             const Settings * const      settings)
{
    int indentlevel = 0;
    unsigned int number_of_if = 0;
    int varusagelevel = -1;
    bool returnStatement = false;  // current statement is a return, stop analysis at the ";"
    bool read = false;  // is variable value read?

    for (Token *tok2 = startToken; tok2 && tok2 != endToken; tok2 = tok2->next()) {
        if (indentlevel >= 0 && tok2->str() == "{")
            ++indentlevel;
        else if (indentlevel >= 0 && tok2->str() == "}") {
            --indentlevel;
            if (indentlevel <= 0 && isReturn(tok2) && Token::Match(tok2->link()->previous(), "else|) {")) {
                const Token *condition = tok2->link();
                const bool iselse = Token::simpleMatch(condition->tokAt(-2), "} else {");
                if (iselse)
                    condition = condition->linkAt(-2);
                if (condition && Token::simpleMatch(condition->previous(), ") {"))
                    condition = condition->linkAt(-1)->astOperand2();
                else
                    condition = nullptr;
                if (!condition) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " valueFlowForward, bailing out since it's unknown if conditional return is executed");
                    return false;
                }

                bool bailoutflag = false;
                for (std::list<ValueFlow::Value>::const_iterator it = values.begin(); it != values.end(); ++it) {
                    if (!iselse && conditionIsTrue(condition, getProgramMemory(condition->astParent(), varid, *it))) {
                        bailoutflag = true;
                        break;
                    }
                    if (iselse && conditionIsFalse(condition, getProgramMemory(condition->astParent(), varid, *it))) {
                        bailoutflag = true;
                        break;
                    }
                }
                if (bailoutflag) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " valueFlowForward, conditional return is assumed to be executed");
                    return false;
                }
            }
        }

        if (Token::Match(tok2, "sizeof|typeof|typeid ("))
            tok2 = tok2->linkAt(1);

        else if (Token::simpleMatch(tok2, "else {")) {
            // Should scope be skipped because variable value is checked?
            bool skipelse = false;
            const Token *condition = tok2->linkAt(-1);
            condition = condition ? condition->linkAt(-1) : nullptr;
            condition = condition ? condition->astOperand2() : nullptr;
            for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end(); ++it) {
                if (conditionIsTrue(condition, getProgramMemory(tok2, varid, *it))) {
                    skipelse = true;
                    break;
                }
            }
            if (skipelse) {
                tok2 = tok2->linkAt(1);
                continue;
            }
        }

        // conditional block of code that assigns variable..
        else if (Token::Match(tok2, "%name% (") && Token::simpleMatch(tok2->linkAt(1), ") {")) {
            // is variable changed in condition?
            if (isVariableChanged(tok2->next(), tok2->next()->link(), varid)) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " valueFlowForward, assignment in condition");
                return false;
            }

            // Set values in condition
            for (Token* tok3 = tok2->tokAt(2); tok3 != tok2->next()->link(); tok3 = tok3->next()) {
                if (tok3->varId() == varid) {
                    for (std::list<ValueFlow::Value>::const_iterator it = values.begin(); it != values.end(); ++it)
                        setTokenValue(tok3, *it);
                    break;
                }
            }

            // Should scope be skipped because variable value is checked?
            std::list<ValueFlow::Value> truevalues;
            for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end(); ++it) {
                if (!conditionIsFalse(tok2->next()->astOperand2(), getProgramMemory(tok2, varid, *it)))
                    truevalues.push_back(*it);
            }
            if (truevalues.size() != values.size()) {
                // '{'
                Token * const startToken1 = tok2->linkAt(1)->next();

                valueFlowForward(startToken1->next(),
                                 startToken1->link(),
                                 var,
                                 varid,
                                 truevalues,
                                 constValue,
                                 tokenlist,
                                 errorLogger,
                                 settings);

                if (isVariableChanged(startToken1, startToken1->link(), varid))
                    removeValues(values, truevalues);

                // goto '}'
                tok2 = startToken1->link();
                continue;
            }

            Token * const start = tok2->linkAt(1)->next();
            Token * const end   = start->link();
            bool varusage = (indentlevel >= 0 && constValue && number_of_if == 0U) ?
                            isVariableChanged(start,end,varid) :
                            (nullptr != Token::findmatch(start, "%varid%", end, varid));
            if (!read) {
                read = bool(nullptr != Token::findmatch(tok2, "%varid% !!=", end, varid));
            }
            if (varusage) {
                varusagelevel = indentlevel;

                if (indentlevel < 0 && tok2->str() == "switch")
                    return false;

                // TODO: don't check noreturn scopes
                if (read && (number_of_if > 0U || Token::findmatch(tok2, "%varid%", start, varid))) {
                    // Set values in condition
                    const Token * const condend = tok2->linkAt(1);
                    for (Token *condtok = tok2; condtok != condend; condtok = condtok->next()) {
                        if (condtok->varId() == varid) {
                            std::list<ValueFlow::Value>::const_iterator it;
                            for (it = values.begin(); it != values.end(); ++it)
                                setTokenValue(condtok, *it);
                        }
                        if (Token::Match(condtok, "%oror%|&&"))
                            break;
                    }
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " is assigned in conditional code");
                    return false;
                }

                if (var->isStatic()) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " bailout when conditional code that contains var is seen");
                    return false;
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

            // stop after conditional noreturn scopes that are executed
            if (isReturn(end)) {
                std::list<ValueFlow::Value>::iterator it;
                for (it = values.begin(); it != values.end();) {
                    if (conditionIsTrue(tok2->next()->astOperand2(), getProgramMemory(tok2, varid, *it)))
                        values.erase(it++);
                    else
                        ++it;
                }
                if (values.empty())
                    return false;
            }

            // noreturn scopes..
            if ((number_of_if > 0 || Token::findmatch(tok2, "%varid%", start, varid)) &&
                (Token::findmatch(start, "return|continue|break|throw", end) ||
                 (Token::simpleMatch(end,"} else {") && Token::findmatch(end, "return|continue|break|throw", end->linkAt(2))))) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + ". noreturn conditional scope.");
                return false;
            }

            if (isVariableChanged(start, end, varid)) {
                if ((!read || number_of_if == 0) &&
                    Token::simpleMatch(tok2, "if (") &&
                    !(Token::simpleMatch(end, "} else {") &&
                      (Token::findmatch(end, "%varid%", end->linkAt(2), varid) ||
                       Token::findmatch(end, "return|continue|break|throw", end->linkAt(2))))) {
                    ++number_of_if;
                    tok2 = end;
                } else {
                    bool bail = true;

                    // loop that conditionally set variable and then break => either loop condition is
                    // redundant or the variable can be unchanged after the loop.
                    bool loopCondition = false;
                    if (Token::simpleMatch(tok2, "while (") && Token::Match(tok2->next()->astOperand2(), "%op%"))
                        loopCondition = true;
                    else if (Token::simpleMatch(tok2, "for (") &&
                             Token::simpleMatch(tok2->next()->astOperand2(), ";") &&
                             Token::simpleMatch(tok2->next()->astOperand2()->astOperand2(), ";") &&
                             Token::Match(tok2->next()->astOperand2()->astOperand2()->astOperand1(), "%op%"))
                        loopCondition = true;
                    if (loopCondition) {
                        const Token *tok3 = Token::findmatch(start, "%varid%", end, varid);
                        if (Token::Match(tok3, "%varid% =", varid) &&
                            tok3->scope()->classEnd                &&
                            Token::Match(tok3->scope()->classEnd->tokAt(-3), "[;}] break ;") &&
                            !Token::findmatch(tok3->next(), "%varid%", end, varid)) {
                            bail = false;
                            tok2 = end;
                        }
                    }

                    if (bail) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " is assigned in conditional code");
                        return false;
                    }
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
            if (tok2->str() == "break") {
                const Scope *scope = tok2->scope();
                if (scope && scope->type == Scope::eSwitch) {
                    tok2 = const_cast<Token *>(scope->classEnd);
                    --indentlevel;
                    continue;
                }
            }
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + ". noreturn conditional scope.");
            return false;
        }

        else if (indentlevel <= 0 && Token::Match(tok2, "return|throw"))
            returnStatement = true;

        else if (returnStatement && tok2->str() == ";")
            return false;

        if (tok2->varId() == varid) {
            // bailout: assignment
            if (Token::Match(tok2->previous(), "!!* %name% %op%") && tok2->next()->isAssignmentOp()) {
                // simplify rhs
                for (Token *tok3 = tok2->tokAt(2); tok3; tok3 = tok3->next()) {
                    if (tok3->varId() == varid) {
                        std::list<ValueFlow::Value>::const_iterator it;
                        for (it = values.begin(); it != values.end(); ++it)
                            setTokenValue(tok3, *it);
                    } else if (Token::Match(tok3, "++|--|?|:|;"))
                        break;
                }
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "assignment of " + tok2->str());
                return false;
            }

            // bailout increment/decrement for now..
            if (Token::Match(tok2->previous(), "++|-- %name%") || Token::Match(tok2, "%name% ++|--")) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "increment/decrement of " + tok2->str());
                return false;
            }

            // bailout: possible assignment using >>
            if (Token::Match(tok2->previous(), ">> %name% >>|;")) {
                const Token *parent = tok2->previous();
                while (Token::simpleMatch(parent,">>"))
                    parent = parent->astParent();
                if (!parent) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "Possible assignment of " + tok2->str() + " using >>");
                    return false;
                }
            }

            // skip if variable is conditionally used in ?: expression
            if (const Token *parent = skipValueInConditionalExpression(tok2)) {
                if (settings->debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "no simplification of " + tok2->str() + " within " + (Token::Match(parent,"[?:]") ? "?:" : parent->str()) + " expression");
                const Token *astTop = parent->astTop();
                if (Token::simpleMatch(astTop->astOperand1(), "for ("))
                    tok2 = const_cast<Token*>(astTop->link());
                continue;
            }

            {
                std::list<ValueFlow::Value>::const_iterator it;
                for (it = values.begin(); it != values.end(); ++it)
                    setTokenValue(tok2, *it);
            }

            // bailout if address of var is taken..
            if (tok2->astParent() && tok2->astParent()->str() == "&" && !tok2->astParent()->astOperand2()) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "Taking address of " + tok2->str());
                return false;
            }

            // bailout if reference is created..
            if (tok2->astParent() && Token::Match(tok2->astParent()->tokAt(-2), "& %name% =")) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "Reference of " + tok2->str());
                return false;
            }

            // assigned by subfunction?
            bool inconclusive = false;
            if (bailoutFunctionPar(tok2, ValueFlow::Value(), settings, &inconclusive)) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "possible assignment of " + tok2->str() + " by subfunction");
                return false;
            }
            if (inconclusive) {
                std::list<ValueFlow::Value>::iterator it;
                for (it = values.begin(); it != values.end(); ++it)
                    it->inconclusive = true;
            }
        }
    }
    return true;
}

static void valueFlowAfterAssign(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    const std::size_t functions = symboldatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symboldatabase->functionScopes[i];
        for (Token* tok = const_cast<Token*>(scope->classStart); tok != scope->classEnd; tok = tok->next()) {
            // Assignment
            if ((tok->str() != "=") || (tok->astParent()))
                continue;

            // Lhs should be a variable
            if (!tok->astOperand1() || !tok->astOperand1()->varId())
                continue;
            const unsigned int varid = tok->astOperand1()->varId();
            const Variable *var = tok->astOperand1()->variable();
            if (!var || (!var->isLocal() && !var->isArgument()))
                continue;

            const Token * const endOfVarScope = var->typeStartToken()->scope()->classEnd;

            // Rhs values..
            if (!tok->astOperand2() || tok->astOperand2()->values.empty())
                continue;

            const std::list<ValueFlow::Value>& values = tok->astOperand2()->values;
            const bool constValue = tok->astOperand2()->isNumber();
            valueFlowForward(tok, endOfVarScope, var, varid, values, constValue, tokenlist, errorLogger, settings);
        }
    }
}

static void valueFlowAfterCondition(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    const std::size_t functions = symboldatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symboldatabase->functionScopes[i];
        for (Token* tok = const_cast<Token*>(scope->classStart); tok != scope->classEnd; tok = tok->next()) {
            const Token *vartok, *numtok;

            // Comparison
            if (Token::Match(tok, "==|!=|>=|<=")) {
                if (!tok->astOperand1() || !tok->astOperand2())
                    continue;
                if (tok->astOperand1()->isNumber()) {
                    numtok = tok->astOperand1();
                    vartok = tok->astOperand2();
                } else {
                    numtok = tok->astOperand2();
                    vartok = tok->astOperand1();
                }
                if (vartok->str() == "=" && vartok->astOperand1() && vartok->astOperand2())
                    vartok = vartok->astOperand1();
                if (!vartok->isName() || !numtok->isNumber() || !MathLib::isInt(numtok->str()))
                    continue;
            } else if (tok->str() == "!") {
                vartok = tok->astOperand1();
                numtok = nullptr;
                if (!vartok || !vartok->isName())
                    continue;

            } else if (tok->isName() &&
                       (Token::Match(tok->astParent(), "%oror%|&&") ||
                        Token::Match(tok->tokAt(-2), "if|while ( %var% [)=]"))) {
                vartok = tok;
                numtok = nullptr;

            } else {
                continue;
            }

            const unsigned int varid = vartok->varId();
            if (varid == 0U)
                continue;
            const Variable *var = vartok->variable();
            if (!var || !(var->isLocal() || var->isArgument()))
                continue;
            std::list<ValueFlow::Value> values;
            values.push_back(ValueFlow::Value(tok, numtok ? MathLib::toLongNumber(numtok->str()) : 0LL));

            if (Token::Match(tok->astParent(), "%oror%|&&")) {
                Token *parent = const_cast<Token*>(tok->astParent());
                const std::string &op(parent->str());

                if (parent->astOperand1() == tok &&
                    ((op == "&&" && Token::Match(tok, "==|>=|<=|!")) ||
                     (op == "||" && Token::Match(tok, "%name%|!=")))) {
                    for (; parent && parent->str() == op; parent = const_cast<Token*>(parent->astParent())) {
                        std::stack<Token *> tokens;
                        tokens.push(const_cast<Token*>(parent->astOperand2()));
                        bool assign = false;
                        while (!tokens.empty()) {
                            Token *rhstok = tokens.top();
                            tokens.pop();
                            if (!rhstok)
                                continue;
                            tokens.push(const_cast<Token*>(rhstok->astOperand1()));
                            tokens.push(const_cast<Token*>(rhstok->astOperand2()));
                            if (rhstok->varId() == varid)
                                setTokenValue(rhstok, values.front());
                            if (Token::Match(rhstok, "++|--|=") && Token::Match(rhstok->astOperand1(), "%varid%", varid)) {
                                assign = true;
                                break;
                            }
                        }
                        if (assign)
                            break;
                        while (parent->astParent() && parent == parent->astParent()->astOperand2())
                            parent = const_cast<Token*>(parent->astParent());
                    }
                }
            }

            const Token *top = tok->astTop();
            if (top && Token::Match(top->previous(), "if|while (") && !top->previous()->isExpandedMacro()) {
                // does condition reassign variable?
                if (tok != top->astOperand2() &&
                    Token::Match(top->astOperand2(), "%oror%|&&") &&
                    isVariableChanged(top, top->link(), varid)) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok, "assignment in condition");
                    continue;
                }

                // start token of conditional code
                Token *startToken = nullptr;

                // based on the comparison, should we check the if or while?
                int codeblock = 0;
                if (Token::Match(tok, "==|>=|<=|!"))
                    codeblock = 1;
                else if (Token::Match(tok, "%name%|!="))
                    codeblock = 2;

                // determine startToken based on codeblock
                if (codeblock > 0) {
                    // if astParent is "!" we need to invert codeblock
                    const Token *parent = tok->astParent();
                    while (parent && parent->str() == "&&")
                        parent = parent->astParent();
                    if (parent && parent->str() == "!")
                        codeblock = (codeblock == 1) ? 2 : 1;

                    // convert codeblock to a startToken
                    if (codeblock == 1 && Token::simpleMatch(top->link(), ") {"))
                        startToken = top->link()->next();
                    else if (Token::simpleMatch(top->link()->linkAt(1), "} else {"))
                        startToken = top->link()->linkAt(1)->tokAt(2);
                }

                bool ok = true;
                if (startToken)
                    ok = valueFlowForward(startToken->next(), startToken->link(), var, varid, values, true, tokenlist, errorLogger, settings);

                // After conditional code..
                if (ok && Token::simpleMatch(top->link(), ") {")) {
                    Token *after = top->link()->linkAt(1);
                    std::string unknownFunction;
                    if (settings->library.isScopeNoReturn(after, &unknownFunction)) {
                        if (settings->debugwarnings && !unknownFunction.empty())
                            bailout(tokenlist, errorLogger, after, "possible noreturn scope");
                        continue;
                    }

                    bool isreturn = (codeblock == 1 && isReturn(after));

                    if (Token::simpleMatch(after, "} else {")) {
                        after = after->linkAt(2);
                        if (Token::simpleMatch(after->tokAt(-2), ") ; }")) {
                            if (settings->debugwarnings)
                                bailout(tokenlist, errorLogger, after, "possible noreturn scope");
                            continue;
                        }
                        isreturn |= (codeblock == 2 && isReturn(after));
                    }

                    if (!isreturn) {
                        // TODO: constValue could be true if there are no assignments in the conditional blocks and
                        //       perhaps if there are no && and no || in the condition
                        bool constValue = false;
                        valueFlowForward(after->next(), top->scope()->classEnd, var, varid, values, constValue, tokenlist, errorLogger, settings);
                    }
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

    else if (expr->isNumber()) {
        *result = MathLib::toLongNumber(expr->str());
        if (MathLib::isFloat(expr->str()))
            *error = true;
    }

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

    else if (Token::Match(expr, "++|--")) {
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

    else if (expr->str() == "!") {
        execute(expr->astOperand1(), programMemory, result, error);
        *result = !(*result);
    }

    else if (expr->str() == "," && expr->astOperand1() && expr->astOperand2()) {
        execute(expr->astOperand1(), programMemory, result, error);
        execute(expr->astOperand2(), programMemory, result, error);
    }

    else if (expr->str() == "[" && expr->astOperand1() && expr->astOperand2()) {
        if (expr->astOperand1()->values.size() != 1U) {
            *error = true;
            return;
        }
        const ValueFlow::Value val = expr->astOperand1()->values.front();
        if (!val.tokvalue || !val.tokvalue->isLiteral()) {
            *error = true;
            return;
        }
        const std::string strValue = val.tokvalue->strValue();
        MathLib::bigint index = 0;
        execute(expr->astOperand2(), programMemory, &index, error);
        if (index >= 0 && index < (int)strValue.size())
            *result = strValue[index];
        else if (index == (int)strValue.size())
            *result = 0;
        else
            *error = true;
    }

    else
        *error = true;
}

static bool valueFlowForLoop1(const Token *tok, unsigned int * const varid, MathLib::bigint * const num1, MathLib::bigint * const num2, MathLib::bigint * const numAfter)
{
    tok = tok->tokAt(2);
    if (!Token::Match(tok, "%type%| %var% ="))
        return false;
    const Token * const vartok = Token::Match(tok, "%var% =") ? tok : tok->next();
    *varid = vartok->varId();
    tok = vartok->tokAt(2);
    const Token * const num1tok = Token::Match(tok, "%num% ;") ? tok : nullptr;
    if (num1tok)
        *num1 = MathLib::toLongNumber(num1tok->str());
    while (Token::Match(tok, "%name%|%num%|%or%|+|-|*|/|&|[|]|("))
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
        if (!Token::Match(num2tok, "%num% ;|%oror%")) // TODO: || enlarges the scope of the condition, so it should not cause FP, but it should no lnger be part of this pattern as soon as valueFlowForLoop2 can handle an unknown RHS of || better
            num2tok = 0;
    }
    if (!num2tok)
        return false;
    *num2 = MathLib::toLongNumber(num2tok->str()) - ((tok->str()=="<=") ? 0 : 1);
    *numAfter = *num2 + 1;
    if (!num1tok)
        *num1 = *num2;
    while (tok && tok->str() != ";")
        tok = tok->next();
    if (!Token::Match(tok, "; %varid% ++ ) {", vartok->varId()) && !Token::Match(tok, "; ++ %varid% ) {", vartok->varId()))
        return false;
    return true;
}

static bool valueFlowForLoop2(const Token *tok,
                              std::map<unsigned int, MathLib::bigint> *memory1,
                              std::map<unsigned int, MathLib::bigint> *memory2,
                              std::map<unsigned int, MathLib::bigint> *memoryAfter)
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
    if (error) {
        // If a variable is reassigned in second expression, return false
        std::stack<const Token *> tokens;
        tokens.push(secondExpression);
        while (!tokens.empty()) {
            const Token *t = tokens.top();
            tokens.pop();
            if (!t)
                continue;
            if (t->str() == "=" && t->astOperand1() && programMemory.find(t->astOperand1()->varId()) != programMemory.end())
                // TODO: investigate what variable is assigned.
                return false;
            tokens.push(t->astOperand1());
            tokens.push(t->astOperand2());
        }
    }

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
    if (!error) {
        memory2->swap(endMemory);
        memoryAfter->swap(programMemory);
    }

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
                if (!parent || parent->str() == ":")
                    break;
                if (parent->str() == "?") {
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

        if (Token::Match(tok2, "%oror%|&&")) {
            const std::map<unsigned int, MathLib::bigint> programMemory(getProgramMemory(tok2->astTop(), varid, ValueFlow::Value(value)));
            if ((tok2->str() == "&&" && conditionIsFalse(tok2->astOperand1(), programMemory)) ||
                (tok2->str() == "||" && conditionIsTrue(tok2->astOperand1(), programMemory))) {
                // Skip second expression..
                const Token *parent = tok2;
                while (parent && parent->str() == tok2->str())
                    parent = parent->astParent();
                // Jump to end of condition
                if (parent && parent->str() == "(") {
                    tok2 = parent->link();
                    // cast
                    if (Token::simpleMatch(tok2, ") ("))
                        tok2 = tok2->linkAt(1);
                }
            }

        }
        if ((tok2->str() == "&&" && conditionIsFalse(tok2->astOperand1(), getProgramMemory(tok2->astTop(), varid, ValueFlow::Value(value)))) ||
            (tok2->str() == "||" && conditionIsTrue(tok2->astOperand1(), getProgramMemory(tok2->astTop(), varid, ValueFlow::Value(value)))))
            break;

        else if (Token::simpleMatch(tok2, ") {") && Token::findmatch(tok2->link(), "%varid%", tok2, varid)) {
            if (Token::findmatch(tok2, "continue|break|return", tok2->linkAt(1), varid)) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop variable bailout on conditional continue|break|return");
                break;
            }
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "For loop variable skipping conditional scope");
            tok2 = tok2->next()->link();
            if (Token::simpleMatch(tok2, "} else {")) {
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

static void valueFlowForLoopSimplifyAfter(Token *fortok, unsigned int varid, const MathLib::bigint num, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    const Token *vartok = nullptr;
    for (const Token *tok = fortok; tok; tok = tok->next()) {
        if (tok->varId() == varid) {
            vartok = tok;
            break;
        }
    }
    if (!vartok || !vartok->variable())
        return;

    const Variable *var = vartok->variable();
    const Token *endToken = nullptr;
    if (var->isLocal())
        endToken = var->typeStartToken()->scope()->classEnd;
    else
        endToken = fortok->scope()->classEnd;

    std::list<ValueFlow::Value> values;
    values.push_back(ValueFlow::Value(num));

    valueFlowForward(fortok->linkAt(1)->linkAt(1)->next(),
                     endToken,
                     var,
                     varid,
                     values,
                     false,
                     tokenlist,
                     errorLogger,
                     settings);
}

static void valueFlowForLoop(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (std::list<Scope>::const_iterator scope = symboldatabase->scopeList.begin(); scope != symboldatabase->scopeList.end(); ++scope) {
        if (scope->type != Scope::eFor)
            continue;

        Token* tok = const_cast<Token*>(scope->classDef);
        Token* const bodyStart = const_cast<Token*>(scope->classStart);

        if (!Token::simpleMatch(tok->next()->astOperand2(), ";") ||
            !Token::simpleMatch(tok->next()->astOperand2()->astOperand2(), ";"))
            continue;

        unsigned int varid(0);
        MathLib::bigint num1(0), num2(0), numAfter(0);

        if (valueFlowForLoop1(tok, &varid, &num1, &num2, &numAfter)) {
            if (num1 <= num2) {
                valueFlowForLoopSimplify(bodyStart, varid, num1, tokenlist, errorLogger, settings);
                valueFlowForLoopSimplify(bodyStart, varid, num2, tokenlist, errorLogger, settings);
                valueFlowForLoopSimplifyAfter(tok, varid, numAfter, tokenlist, errorLogger, settings);
            } else
                valueFlowForLoopSimplifyAfter(tok, varid, num1, tokenlist, errorLogger, settings);
        } else {
            std::map<unsigned int, MathLib::bigint> mem1, mem2, memAfter;
            if (valueFlowForLoop2(tok, &mem1, &mem2, &memAfter)) {
                std::map<unsigned int, MathLib::bigint>::const_iterator it;
                for (it = mem1.begin(); it != mem1.end(); ++it)
                    valueFlowForLoopSimplify(bodyStart, it->first, it->second, tokenlist, errorLogger, settings);
                for (it = mem2.begin(); it != mem2.end(); ++it)
                    valueFlowForLoopSimplify(bodyStart, it->first, it->second, tokenlist, errorLogger, settings);
                for (it = memAfter.begin(); it != memAfter.end(); ++it)
                    valueFlowForLoopSimplifyAfter(tok, it->first, it->second, tokenlist, errorLogger, settings);
            }
        }
    }
}

static void valueFlowInjectParameter(TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings, const Variable* arg, const Scope* functionScope, const std::list<ValueFlow::Value>& argvalues)
{
    // Is argument passed by value or const reference, and is it a known non-class type?
    if (arg->isReference() && !arg->isConst() && !arg->isClass())
        return;

    // Set value in function scope..
    const unsigned int varid2 = arg->declarationId();
    if (!varid2)
        return;

    valueFlowForward(const_cast<Token*>(functionScope->classStart->next()), functionScope->classEnd, arg, varid2, argvalues, true, tokenlist, errorLogger, settings);
}

static void valueFlowSubFunction(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;

        const Function * const currentFunction = tok->function();
        if (!currentFunction)
            continue;

        // Function scope..
        const Scope * const functionScope = currentFunction->functionScope;
        if (!functionScope)
            continue;

        unsigned int argnr = 0U;
        for (const Token *argtok = tok->tokAt(2); argtok; argtok = argtok->nextArgument()) {
            // Get function argument
            const Variable * const arg = currentFunction->getArgumentVar(argnr++);
            if (!arg)
                break;

            std::list<ValueFlow::Value> argvalues;

            // passing value(s) to function
            if (!argtok->values.empty() && Token::Match(argtok, "%name%|%num%|%str% [,)]"))
                argvalues = argtok->values;
            else {
                // bool operator => values 1/0 are passed to function..
                const Token *op = argtok;
                while (op && op->astParent() && !Token::Match(op->astParent(), "[(,]"))
                    op = op->astParent();
                if (Token::Match(op, "%comp%|%oror%|&&|!")) {
                    argvalues.clear();
                    argvalues.push_back(ValueFlow::Value(0));
                    argvalues.push_back(ValueFlow::Value(1));
                } else if (Token::Match(op, "%cop%") && !op->values.empty()) {
                    argvalues = op->values;
                } else {
                    // possible values are unknown..
                    continue;
                }
            }
            valueFlowInjectParameter(tokenlist, errorLogger, settings, arg, functionScope, argvalues);
        }
    }
}

static void valueFlowFunctionDefaultParameter(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    if (!tokenlist->isCPP())
        return;

    const std::size_t functions = symboldatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope* scope = symboldatabase->functionScopes[i];
        const Function* function = scope->function;
        if (!function)
            continue;
        for (std::size_t arg = function->minArgCount(); arg < function->argCount(); arg++) {
            const Variable* var = function->getArgumentVar(arg);
            if (var && var->hasDefault() && Token::Match(var->nameToken(), "%var% = %num%|%str% [,)]")) {
                const Token* valueTok = var->nameToken()->tokAt(2);
                if (valueTok->values.empty())
                    continue;
                const_cast<Token*>(valueTok)->values.front().defaultArg = true;
                valueFlowInjectParameter(tokenlist, errorLogger, settings, var, scope, valueTok->values);
            }
        }
    }
}

static bool constval(const Token * tok)
{
    return tok && tok->values.size() == 1U && tok->values.front().varId == 0U;
}

static void valueFlowFunctionReturn(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->str() != "(" || !tok->astOperand1() || !tok->astOperand1()->function())
            continue;

        // Arguments..
        std::vector<MathLib::bigint> parvalues;
        {
            const Token *partok = tok->astOperand2();
            while (partok && partok->str() == "," && constval(partok->astOperand2()))
                partok = partok->astOperand1();
            if (!constval(partok))
                continue;
            parvalues.push_back(partok->values.front().intvalue);
            partok = partok->astParent();
            while (partok && partok->str() == ",") {
                parvalues.push_back(partok->astOperand2()->values.front().intvalue);
                partok = partok->astParent();
            }
            if (partok != tok)
                continue;
        }

        // Get scope and args of function
        const Function * const function = tok->astOperand1()->function();
        const Scope * const functionScope = function->functionScope;
        if (!functionScope || !Token::simpleMatch(functionScope->classStart, "{ return")) {
            if (functionScope && settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok, "function return; nontrivial function body");
            continue;
        }

        std::map<unsigned int, MathLib::bigint> programMemory;
        for (std::size_t i = 0; i < parvalues.size(); ++i) {
            const Variable * const arg = function->getArgumentVar(i);
            if (!arg || !Token::Match(arg->typeStartToken(), "%type% %name% ,|)")) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok, "function return; unhandled argument type");
                programMemory.clear();
                break;
            }
            programMemory[arg->declarationId()] = parvalues[i];
        }
        if (programMemory.empty())
            continue;

        // Determine return value of subfunction..
        MathLib::bigint result = 0;
        bool error = false;
        execute(functionScope->classStart->next()->astOperand1(),
                &programMemory,
                &result,
                &error);
        if (!error)
            setTokenValue(tok, ValueFlow::Value(result));
    }
}

void ValueFlow::setValues(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next())
        tok->values.clear();

    valueFlowNumber(tokenlist);
    valueFlowString(tokenlist);
    valueFlowArray(tokenlist);
    valueFlowPointerAlias(tokenlist);
    valueFlowFunctionReturn(tokenlist, errorLogger, settings);
    valueFlowBitAnd(tokenlist);
    valueFlowForLoop(tokenlist, symboldatabase, errorLogger, settings);
    valueFlowBeforeCondition(tokenlist, symboldatabase, errorLogger, settings);
    valueFlowAfterAssign(tokenlist, symboldatabase, errorLogger, settings);
    valueFlowAfterCondition(tokenlist, symboldatabase, errorLogger, settings);
    valueFlowSubFunction(tokenlist, errorLogger, settings);
    valueFlowFunctionDefaultParameter(tokenlist, symboldatabase, errorLogger, settings);
}

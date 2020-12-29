/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

/**
 * @brief This is the ValueFlow component in Cppcheck.
 *
 * Each @sa Token in the token list has a list of values. These are
 * the "possible" values for the Token at runtime.
 *
 * In the --debug and --debug-normal output you can see the ValueFlow data. For example:
 *
 *     int f()
 *     {
 *         int x = 10;
 *         return 4 * x + 2;
 *     }
 *
 * The --debug-normal output says:
 *
 *     ##Value flow
 *     Line 3
 *       10 always 10
 *     Line 4
 *       4 always 4
 *       * always 40
 *       x always 10
 *       + always 42
 *       2 always 2
 *
 * All value flow analysis is executed in the ValueFlow::setValues() function. The ValueFlow analysis is executed after
 * the tokenizer/ast/symboldatabase/etc.. The ValueFlow analysis is done in a series of valueFlow* function calls, where
 * each such function call can only use results from previous function calls. The function calls should be arranged so
 * that valueFlow* that do not require previous ValueFlow information should be first.
 *
 * Type of analysis
 * ================
 *
 * This is "flow sensitive" value flow analysis. We _usually_ track the value for 1 variable at a time.
 *
 * How are calculations handled
 * ============================
 *
 * Here is an example code:
 *
 *   x = 3 + 4;
 *
 * The valueFlowNumber set the values for the "3" and "4" tokens by calling setTokenValue().
 * The setTokenValue() handle the calculations automatically. When both "3" and "4" have values, the "+" can be
 * calculated. setTokenValue() recursively calls itself when parents in calculations can be calculated.
 *
 * Forward / Reverse flow analysis
 * ===============================
 *
 * In forward value flow analysis we know a value and see what happens when we are stepping the program forward. Like
 * normal execution. The valueFlowForwardVariable is used in this analysis.
 *
 * In reverse value flow analysis we know the value of a variable at line X. And try to "execute backwards" to determine
 * possible values before line X. The valueFlowReverse is used in this analysis.
 *
 *
 */

#include "valueflow.h"

#include "analyzer.h"
#include "astutils.h"
#include "errorlogger.h"
#include "forwardanalyzer.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "programmemory.h"
#include "reverseanalyzer.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueptr.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <stack>
#include <tuple>
#include <vector>

static void bailoutInternal(const std::string& type, TokenList *tokenlist, ErrorLogger *errorLogger, const Token *tok, const std::string &what, const std::string &file, int line, const std::string &function)
{
    std::list<ErrorMessage::FileLocation> callstack(1, ErrorMessage::FileLocation(tok, tokenlist));
    ErrorMessage errmsg(callstack, tokenlist->getSourceFilePath(), Severity::debug,
                        Path::stripDirectoryPart(file) + ":" + MathLib::toString(line) + ":" + function + " bailout: " + what, type, false);
    errorLogger->reportErr(errmsg);
}

#if (defined __cplusplus) && __cplusplus >= 201103L
#define bailout2(type, tokenlist, errorLogger, tok, what) bailoutInternal(type, tokenlist, errorLogger, tok, what, __FILE__, __LINE__, __func__)
#elif (defined __GNUC__) || (defined __clang__) || (defined _MSC_VER)
#define bailout2(type, tokenlist, errorLogger, tok, what) bailoutInternal(type, tokenlist, errorLogger, tok, what, __FILE__, __LINE__, __FUNCTION__)
#else
#define bailout2(type, tokenlist, errorLogger, tok, what) bailoutInternal(type, tokenlist, errorLogger, tok, what, __FILE__, __LINE__, "(valueFlow)")
#endif

#define bailout(tokenlist, errorLogger, tok, what) bailout2("valueFlowBailout", tokenlist, errorLogger, tok, what)

#define bailoutIncompleteVar(tokenlist, errorLogger, tok, what) bailout2("valueFlowBailoutIncompleteVar", tokenlist, errorLogger, tok, what)

static void changeKnownToPossible(std::list<ValueFlow::Value> &values, int indirect=-1)
{
    for (ValueFlow::Value& v: values) {
        if (indirect >= 0 && v.indirect != indirect)
            continue;
        v.changeKnownToPossible();
    }
}

static void removeImpossible(std::list<ValueFlow::Value>& values, int indirect = -1)
{
    values.remove_if([&](const ValueFlow::Value& v) {
        if (indirect >= 0 && v.indirect != indirect)
            return false;
        return v.isImpossible();
    });
}

static void lowerToPossible(std::list<ValueFlow::Value>& values, int indirect = -1)
{
    changeKnownToPossible(values, indirect);
    removeImpossible(values, indirect);
}

static void changePossibleToKnown(std::list<ValueFlow::Value>& values, int indirect = -1)
{
    for (ValueFlow::Value& v : values) {
        if (indirect >= 0 && v.indirect != indirect)
            continue;
        if (!v.isPossible())
            continue;
        if (v.bound != ValueFlow::Value::Bound::Point)
            continue;
        v.setKnown();
    }
}

static void setValueUpperBound(ValueFlow::Value& value, bool upper)
{
    if (upper)
        value.bound = ValueFlow::Value::Bound::Upper;
    else
        value.bound = ValueFlow::Value::Bound::Lower;
}

static void setValueBound(ValueFlow::Value& value, const Token* tok, bool invert)
{
    if (Token::Match(tok, "<|<=")) {
        setValueUpperBound(value, !invert);
    } else if (Token::Match(tok, ">|>=")) {
        setValueUpperBound(value, invert);
    }
}

static void setConditionalValues(const Token *tok,
                                 bool invert,
                                 MathLib::bigint value,
                                 ValueFlow::Value &true_value,
                                 ValueFlow::Value &false_value)
{
    if (Token::Match(tok, "==|!=|>=|<=")) {
        true_value = ValueFlow::Value{tok, value};
        const char* greaterThan = ">=";
        const char* lessThan = "<=";
        if (invert)
            std::swap(greaterThan, lessThan);
        if (Token::simpleMatch(tok, greaterThan, strlen(greaterThan))) {
            false_value = ValueFlow::Value{tok, value - 1};
        } else if (Token::simpleMatch(tok, lessThan, strlen(lessThan))) {
            false_value = ValueFlow::Value{tok, value + 1};
        } else {
            false_value = ValueFlow::Value{tok, value};
        }
    } else {
        const char* greaterThan = ">";
        const char* lessThan = "<";
        if (invert)
            std::swap(greaterThan, lessThan);
        if (Token::simpleMatch(tok, greaterThan, strlen(greaterThan))) {
            true_value = ValueFlow::Value{tok, value + 1};
            false_value = ValueFlow::Value{tok, value};
        } else if (Token::simpleMatch(tok, lessThan, strlen(lessThan))) {
            true_value = ValueFlow::Value{tok, value - 1};
            false_value = ValueFlow::Value{tok, value};
        }
    }
    setValueBound(true_value, tok, invert);
    setValueBound(false_value, tok, !invert);
}

static bool isSaturated(MathLib::bigint value)
{
    return value == std::numeric_limits<MathLib::bigint>::max() || value == std::numeric_limits<MathLib::bigint>::min();
}

const Token *parseCompareInt(const Token *tok, ValueFlow::Value &true_value, ValueFlow::Value &false_value)
{
    if (!tok->astOperand1() || !tok->astOperand2())
        return nullptr;
    if (tok->isComparisonOp()) {
        if (tok->astOperand1()->hasKnownIntValue()) {
            MathLib::bigint value = tok->astOperand1()->values().front().intvalue;
            if (isSaturated(value))
                return nullptr;
            setConditionalValues(tok, true, value, true_value, false_value);
            return tok->astOperand2();
        } else if (tok->astOperand2()->hasKnownIntValue()) {
            MathLib::bigint value = tok->astOperand2()->values().front().intvalue;
            if (isSaturated(value))
                return nullptr;
            setConditionalValues(tok, false, value, true_value, false_value);
            return tok->astOperand1();
        }
    }
    return nullptr;
}


static bool isEscapeScope(const Token* tok, TokenList * tokenlist, bool unknown = false)
{
    if (!Token::simpleMatch(tok, "{"))
        return false;
    // TODO this search for termTok in all subscopes. It should check the end of the scope.
    const Token * termTok = Token::findmatch(tok, "return|continue|break|throw|goto", tok->link());
    if (termTok && termTok->scope() == tok->scope())
        return true;
    std::string unknownFunction;
    if (tokenlist && tokenlist->getSettings()->library.isScopeNoReturn(tok->link(), &unknownFunction))
        return unknownFunction.empty() || unknown;
    return false;
}

static ValueFlow::Value castValue(ValueFlow::Value value, const ValueType::Sign sign, nonneg int bit)
{
    if (value.isFloatValue()) {
        value.valueType = ValueFlow::Value::INT;
        if (value.floatValue >= std::numeric_limits<int>::min() && value.floatValue <= std::numeric_limits<int>::max()) {
            value.intvalue = value.floatValue;
        } else { // don't perform UB
            value.intvalue = 0;
        }
    }
    if (bit < MathLib::bigint_bits) {
        const MathLib::biguint one = 1;
        value.intvalue &= (one << bit) - 1;
        if (sign == ValueType::Sign::SIGNED && value.intvalue & (one << (bit - 1))) {
            value.intvalue |= ~((one << bit) - 1ULL);
        }
    }
    return value;
}

static void combineValueProperties(const ValueFlow::Value &value1, const ValueFlow::Value &value2, ValueFlow::Value *result)
{
    if (value1.isKnown() && value2.isKnown())
        result->setKnown();
    else if (value1.isImpossible() || value2.isImpossible())
        result->setImpossible();
    else if (value1.isInconclusive() || value2.isInconclusive())
        result->setInconclusive();
    else
        result->setPossible();
    if (value1.isIteratorValue())
        result->valueType = value1.valueType;
    if (value2.isIteratorValue())
        result->valueType = value2.valueType;
    result->condition = value1.condition ? value1.condition : value2.condition;
    result->varId = (value1.varId != 0U) ? value1.varId : value2.varId;
    result->varvalue = (result->varId == value1.varId) ? value1.varvalue : value2.varvalue;
    result->errorPath = (value1.errorPath.empty() ? value2 : value1).errorPath;
    result->safe = value1.safe || value2.safe;
    if (value1.bound == ValueFlow::Value::Bound::Point || value2.bound == ValueFlow::Value::Bound::Point) {
        if (value1.bound == ValueFlow::Value::Bound::Upper || value2.bound == ValueFlow::Value::Bound::Upper)
            result->bound = ValueFlow::Value::Bound::Upper;
        if (value1.bound == ValueFlow::Value::Bound::Lower || value2.bound == ValueFlow::Value::Bound::Lower)
            result->bound = ValueFlow::Value::Bound::Lower;
    }
    if (value1.path != value2.path)
        result->path = -1;
    else
        result->path = value1.path;
}

static const Token *getCastTypeStartToken(const Token *parent)
{
    // TODO: This might be a generic utility function?
    if (!parent || parent->str() != "(")
        return nullptr;
    if (!parent->astOperand2() && Token::Match(parent,"( %name%"))
        return parent->next();
    if (parent->astOperand2() && Token::Match(parent->astOperand1(), "const_cast|dynamic_cast|reinterpret_cast|static_cast <"))
        return parent->astOperand1()->tokAt(2);
    return nullptr;
}

static bool isComputableValue(const Token* parent, const ValueFlow::Value& value)
{
    const bool noninvertible = parent->isComparisonOp() || Token::Match(parent, "%|/|&|%or%");
    if (noninvertible && value.isImpossible())
        return false;
    if (!value.isIntValue() && !value.isFloatValue() && !value.isTokValue() && !value.isIteratorValue())
        return false;
    if (value.isIteratorValue() && !Token::Match(parent, "+|-"))
        return false;
    if (value.isTokValue() && (!parent->isComparisonOp() || value.tokvalue->tokType() != Token::eString))
        return false;
    return true;
}

/** Set token value for cast */
static void setTokenValueCast(Token *parent, const ValueType &valueType, const ValueFlow::Value &value, const Settings *settings);

/** set ValueFlow value and perform calculations if possible */
static void setTokenValue(Token* tok, const ValueFlow::Value &value, const Settings *settings)
{
    if (!tok->addValue(value))
        return;

    if (value.path < 0)
        return;

    Token *parent = tok->astParent();
    if (!parent)
        return;

    if (value.isContainerSizeValue()) {
        // .empty, .size, +"abc", +'a'
        if (parent->str() == "+" && parent->astOperand1() && parent->astOperand2()) {
            for (const ValueFlow::Value &value1 : parent->astOperand1()->values()) {
                for (const ValueFlow::Value &value2 : parent->astOperand2()->values()) {
                    if (value1.path != value2.path)
                        continue;
                    ValueFlow::Value result;
                    result.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    if (value1.isContainerSizeValue() && value2.isContainerSizeValue())
                        result.intvalue = value1.intvalue + value2.intvalue;
                    else if (value1.isContainerSizeValue() && value2.isTokValue() && value2.tokvalue->tokType() == Token::eString)
                        result.intvalue = value1.intvalue + Token::getStrLength(value2.tokvalue);
                    else if (value2.isContainerSizeValue() && value1.isTokValue() && value1.tokvalue->tokType() == Token::eString)
                        result.intvalue = Token::getStrLength(value1.tokvalue) + value2.intvalue;
                    else
                        continue;

                    combineValueProperties(value1, value2, &result);

                    setTokenValue(parent, result, settings);
                }
            }
        }


        else if (Token::Match(parent, ". %name% (") && parent->astParent() == parent->tokAt(2) && parent->astOperand1() && parent->astOperand1()->valueType()) {
            const Library::Container *c = parent->astOperand1()->valueType()->container;
            const Library::Container::Yield yields = c ? c->getYield(parent->strAt(1)) : Library::Container::Yield::NO_YIELD;
            if (yields == Library::Container::Yield::SIZE) {
                ValueFlow::Value v(value);
                v.valueType = ValueFlow::Value::ValueType::INT;
                setTokenValue(parent->astParent(), v, settings);
            } else if (yields == Library::Container::Yield::EMPTY) {
                ValueFlow::Value v(value);
                v.intvalue = !v.intvalue;
                v.valueType = ValueFlow::Value::ValueType::INT;
                setTokenValue(parent->astParent(), v, settings);
            }
        }

        return;
    }

    if (value.isLifetimeValue()) {
        if (!isLifetimeBorrowed(parent, settings))
            return;
        if (value.lifetimeKind == ValueFlow::Value::LifetimeKind::Iterator && astIsIterator(parent)) {
            setTokenValue(parent,value,settings);
        } else if (astIsPointer(tok) && astIsPointer(parent) &&
                   (parent->isArithmeticalOp() || Token::Match(parent, "( %type%"))) {
            setTokenValue(parent,value,settings);
        }
        return;
    }

    if (value.isUninitValue()) {
        ValueFlow::Value pvalue = value;
        if (parent->isUnaryOp("&")) {
            pvalue.indirect++;
            setTokenValue(parent, pvalue, settings);
        } else if (Token::Match(parent, ". %var%") && parent->astOperand1() == tok) {
            if (parent->originalName() == "->" && pvalue.indirect > 0)
                pvalue.indirect--;
            setTokenValue(parent->astOperand2(), pvalue, settings);
        } else if (Token::Match(parent->astParent(), ". %var%") && parent->astParent()->astOperand1() == parent) {
            if (parent->astParent()->originalName() == "->" && pvalue.indirect > 0)
                pvalue.indirect--;
            setTokenValue(parent->astParent()->astOperand2(), pvalue, settings);
        } else if (parent->isUnaryOp("*") && pvalue.indirect > 0) {
            pvalue.indirect--;
            setTokenValue(parent, pvalue, settings);
        }
        return;
    }

    // cast..
    if (const Token *castType = getCastTypeStartToken(parent)) {
        if (((tok->valueType() == nullptr && value.isImpossible()) || astIsPointer(tok)) && value.valueType == ValueFlow::Value::INT &&
            Token::simpleMatch(parent->astOperand1(), "dynamic_cast"))
            return;
        const ValueType &valueType = ValueType::parseDecl(castType, settings);
        setTokenValueCast(parent, valueType, value, settings);
    }

    else if (parent->str() == ":") {
        setTokenValue(parent,value,settings);
    }

    else if (parent->str() == "?" && tok->str() == ":" && tok == parent->astOperand2() && parent->astOperand1()) {
        // is condition always true/false?
        if (parent->astOperand1()->hasKnownValue()) {
            const ValueFlow::Value &condvalue = parent->astOperand1()->values().front();
            const bool cond(condvalue.isTokValue() || (condvalue.isIntValue() && condvalue.intvalue != 0));
            if (cond && !tok->astOperand1()) { // true condition, no second operator
                setTokenValue(parent, condvalue, settings);
            } else {
                const Token *op = cond ? tok->astOperand1() : tok->astOperand2();
                if (!op) // #7769 segmentation fault at setTokenValue()
                    return;
                const std::list<ValueFlow::Value> &values = op->values();
                if (std::find(values.begin(), values.end(), value) != values.end())
                    setTokenValue(parent, value, settings);
            }
        } else {
            // is condition only depending on 1 variable?
            int varId = 0;
            bool ret = false;
            visitAstNodes(parent->astOperand1(),
            [&](const Token *t) {
                if (t->varId()) {
                    if (varId > 0 || value.varId != 0U)
                        ret = true;
                    varId = t->varId();
                } else if (t->str() == "(" && Token::Match(t->previous(), "%name%"))
                    ret = true; // function call
                return ret ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
            });
            if (ret)
                return;

            ValueFlow::Value v(value);
            v.conditional = true;
            v.changeKnownToPossible();

            if (varId)
                v.varId = varId;

            setTokenValue(parent, v, settings);
        }
    }

    else if (parent->str() == "?" && value.isIntValue() && tok == parent->astOperand1() && value.isKnown() &&
             parent->astOperand2() && parent->astOperand2()->astOperand1() && parent->astOperand2()->astOperand2()) {
        const std::list<ValueFlow::Value> &values = (value.intvalue == 0
                ? parent->astOperand2()->astOperand2()->values()
                : parent->astOperand2()->astOperand1()->values());

        for (const ValueFlow::Value &v : values)
            setTokenValue(parent, v, settings);
    }

    // Calculations..
    else if ((parent->isArithmeticalOp() || parent->isComparisonOp() || (parent->tokType() == Token::eBitOp) || (parent->tokType() == Token::eLogicalOp)) &&
             parent->astOperand1() &&
             parent->astOperand2()) {

        const bool noninvertible = parent->isComparisonOp() || Token::Match(parent, "%|/|&|%or%");

        // Skip operators with impossible values that are not invertible
        if (noninvertible && value.isImpossible())
            return;

        // known result when a operand is 0.
        if (Token::Match(parent, "[&*]") && value.isKnown() && value.isIntValue() && value.intvalue==0) {
            setTokenValue(parent, value, settings);
            return;
        }

        // known result when a operand is true.
        if (Token::simpleMatch(parent, "&&") && value.isKnown() && value.isIntValue() && value.intvalue==0) {
            setTokenValue(parent, value, settings);
            return;
        }

        // known result when a operand is false.
        if (Token::simpleMatch(parent, "||") && value.isKnown() && value.isIntValue() && value.intvalue!=0) {
            setTokenValue(parent, value, settings);
            return;
        }

        for (const ValueFlow::Value &value1 : parent->astOperand1()->values()) {
            if (!isComputableValue(parent, value1))
                continue;
            for (const ValueFlow::Value &value2 : parent->astOperand2()->values()) {
                if (value1.path != value2.path)
                    continue;
                if (!isComputableValue(parent, value2))
                    continue;
                if (value1.isIteratorValue() && value2.isIteratorValue())
                    continue;
                if (value1.isKnown() || value2.isKnown() || value1.varId == 0U || value2.varId == 0U ||
                    (value1.varId == value2.varId && value1.varvalue == value2.varvalue && value1.isIntValue() &&
                     value2.isIntValue())) {
                    ValueFlow::Value result(0);
                    combineValueProperties(value1, value2, &result);
                    const double floatValue1 = value1.isIntValue() ? value1.intvalue : value1.floatValue;
                    const double floatValue2 = value2.isIntValue() ? value2.intvalue : value2.floatValue;
                    switch (parent->str()[0]) {
                    case '+':
                        if (value1.isTokValue() || value2.isTokValue())
                            break;
                        if (value1.isFloatValue() || value2.isFloatValue()) {
                            result.valueType = ValueFlow::Value::FLOAT;
                            result.floatValue = floatValue1 + floatValue2;
                        } else {
                            result.intvalue = value1.intvalue + value2.intvalue;
                        }
                        setTokenValue(parent, result, settings);
                        break;
                    case '-':
                        if (value1.isTokValue() || value2.isTokValue())
                            break;
                        if (value1.isFloatValue() || value2.isFloatValue()) {
                            result.valueType = ValueFlow::Value::FLOAT;
                            result.floatValue = floatValue1 - floatValue2;
                        } else {
                            // Avoid overflow
                            if (value1.intvalue < 0 && value2.intvalue > value1.intvalue - LLONG_MIN)
                                break;

                            result.intvalue = value1.intvalue - value2.intvalue;
                        }
                        // If the bound comes from the second value then invert the bound
                        if (value2.bound == result.bound && value2.bound != ValueFlow::Value::Bound::Point)
                            result.invertBound();
                        setTokenValue(parent, result, settings);
                        break;
                    case '*':
                        if (value1.isTokValue() || value2.isTokValue())
                            break;
                        if (value1.isFloatValue() || value2.isFloatValue()) {
                            result.valueType = ValueFlow::Value::FLOAT;
                            result.floatValue = floatValue1 * floatValue2;
                        } else {
                            result.intvalue = value1.intvalue * value2.intvalue;
                        }
                        setTokenValue(parent, result, settings);
                        break;
                    case '/':
                        if (value1.isTokValue() || value2.isTokValue() || value2.intvalue == 0)
                            break;
                        if (value1.isFloatValue() || value2.isFloatValue()) {
                            result.valueType = ValueFlow::Value::FLOAT;
                            result.floatValue = floatValue1 / floatValue2;
                        } else {
                            result.intvalue = value1.intvalue / value2.intvalue;
                        }
                        setTokenValue(parent, result, settings);
                        break;
                    case '%':
                        if (!value1.isIntValue() || !value2.isIntValue())
                            break;
                        if (value2.intvalue == 0)
                            break;
                        result.intvalue = value1.intvalue % value2.intvalue;
                        setTokenValue(parent, result, settings);
                        break;
                    case '=':
                        if (parent->str() == "==") {
                            if ((value1.isIntValue() && value2.isTokValue()) ||
                                (value1.isTokValue() && value2.isIntValue())) {
                                result.intvalue = 0;
                                setTokenValue(parent, result, settings);
                            } else if (value1.isIntValue() && value2.isIntValue()) {
                                result.intvalue = value1.intvalue == value2.intvalue;
                                setTokenValue(parent, result, settings);
                            }
                        }
                        break;
                    case '!':
                        if (parent->str() == "!=") {
                            if ((value1.isIntValue() && value2.isTokValue()) ||
                                (value1.isTokValue() && value2.isIntValue())) {
                                result.intvalue = 1;
                                setTokenValue(parent, result, settings);
                            } else if (value1.isIntValue() && value2.isIntValue()) {
                                result.intvalue = value1.intvalue != value2.intvalue;
                                setTokenValue(parent, result, settings);
                            }
                        }
                        break;
                    case '>': {
                        const bool f = value1.isFloatValue() || value2.isFloatValue();
                        if (!f && !value1.isIntValue() && !value2.isIntValue())
                            break;
                        if (parent->str() == ">")
                            result.intvalue = f ? (floatValue1 > floatValue2) : (value1.intvalue > value2.intvalue);
                        else if (parent->str() == ">=")
                            result.intvalue = f ? (floatValue1 >= floatValue2) : (value1.intvalue >= value2.intvalue);
                        else if (!f && parent->str() == ">>" && value1.intvalue >= 0 && value2.intvalue >= 0 && value2.intvalue < MathLib::bigint_bits)
                            result.intvalue = value1.intvalue >> value2.intvalue;
                        else
                            break;
                        setTokenValue(parent, result, settings);
                        break;
                    }
                    case '<': {
                        const bool f = value1.isFloatValue() || value2.isFloatValue();
                        if (!f && !value1.isIntValue() && !value2.isIntValue())
                            break;
                        if (parent->str() == "<")
                            result.intvalue = f ? (floatValue1 < floatValue2) : (value1.intvalue < value2.intvalue);
                        else if (parent->str() == "<=")
                            result.intvalue = f ? (floatValue1 <= floatValue2) : (value1.intvalue <= value2.intvalue);
                        else if (!f && parent->str() == "<<" && value1.intvalue >= 0 && value2.intvalue >= 0 && value2.intvalue < MathLib::bigint_bits)
                            result.intvalue = value1.intvalue << value2.intvalue;
                        else
                            break;
                        setTokenValue(parent, result, settings);
                        break;
                    }
                    case '&':
                        if (!value1.isIntValue() || !value2.isIntValue())
                            break;
                        if (parent->str() == "&")
                            result.intvalue = value1.intvalue & value2.intvalue;
                        else
                            result.intvalue = value1.intvalue && value2.intvalue;
                        setTokenValue(parent, result, settings);
                        break;
                    case '|':
                        if (!value1.isIntValue() || !value2.isIntValue())
                            break;
                        if (parent->str() == "|")
                            result.intvalue = value1.intvalue | value2.intvalue;
                        else
                            result.intvalue = value1.intvalue || value2.intvalue;
                        setTokenValue(parent, result, settings);
                        break;
                    case '^':
                        if (!value1.isIntValue() || !value2.isIntValue())
                            break;
                        result.intvalue = value1.intvalue ^ value2.intvalue;
                        setTokenValue(parent, result, settings);
                        break;
                    default:
                        // unhandled operator, do nothing
                        break;
                    }
                }
            }
        }
    }

    // !
    else if (parent->str() == "!") {
        for (const ValueFlow::Value &val : tok->values()) {
            if (!val.isIntValue())
                continue;
            ValueFlow::Value v(val);
            v.intvalue = !v.intvalue;
            setTokenValue(parent, v, settings);
        }
    }

    // ~
    else if (parent->str() == "~") {
        for (const ValueFlow::Value &val : tok->values()) {
            if (!val.isIntValue())
                continue;
            ValueFlow::Value v(val);
            v.intvalue = ~v.intvalue;
            int bits = 0;
            if (settings &&
                tok->valueType() &&
                tok->valueType()->sign == ValueType::Sign::UNSIGNED &&
                tok->valueType()->pointer == 0) {
                if (tok->valueType()->type == ValueType::Type::INT)
                    bits = settings->int_bit;
                else if (tok->valueType()->type == ValueType::Type::LONG)
                    bits = settings->long_bit;
            }
            if (bits > 0 && bits < MathLib::bigint_bits)
                v.intvalue &= (((MathLib::biguint)1)<<bits) - 1;
            setTokenValue(parent, v, settings);
        }
    }

    // unary minus
    else if (parent->isUnaryOp("-")) {
        for (const ValueFlow::Value &val : tok->values()) {
            if (!val.isIntValue() && !val.isFloatValue())
                continue;
            ValueFlow::Value v(val);
            if (v.isIntValue()) {
                if (v.intvalue == LLONG_MIN)
                    // Value can't be inverted
                    continue;
                v.intvalue = -v.intvalue;
            } else
                v.floatValue = -v.floatValue;
            v.invertBound();
            setTokenValue(parent, v, settings);
        }
    }

    // increment
    else if (parent->str() == "++") {
        for (const ValueFlow::Value &val : tok->values()) {
            if (!val.isIntValue() && !val.isFloatValue())
                continue;
            ValueFlow::Value v(val);
            if (parent == tok->previous()) {
                if (v.isIntValue())
                    v.intvalue = v.intvalue + 1;
                else
                    v.floatValue = v.floatValue + 1.0;
            }
            setTokenValue(parent, v, settings);
        }
    }

    // decrement
    else if (parent->str() == "--") {
        for (const ValueFlow::Value &val : tok->values()) {
            if (!val.isIntValue() && !val.isFloatValue())
                continue;
            ValueFlow::Value v(val);
            if (parent == tok->previous()) {
                if (v.isIntValue())
                    v.intvalue = v.intvalue - 1;
                else
                    v.floatValue = v.floatValue - 1.0;
            }
            setTokenValue(parent, v, settings);
        }
    }

    // Array element
    else if (parent->str() == "[" && parent->isBinaryOp()) {
        for (const ValueFlow::Value &value1 : parent->astOperand1()->values()) {
            if (!value1.isTokValue())
                continue;
            for (const ValueFlow::Value &value2 : parent->astOperand2()->values()) {
                if (!value2.isIntValue())
                    continue;
                if (value1.varId == 0U || value2.varId == 0U ||
                    (value1.varId == value2.varId && value1.varvalue == value2.varvalue)) {
                    ValueFlow::Value result(0);
                    result.condition = value1.condition ? value1.condition : value2.condition;
                    result.setInconclusive(value1.isInconclusive() | value2.isInconclusive());
                    result.varId = (value1.varId != 0U) ? value1.varId : value2.varId;
                    result.varvalue = (result.varId == value1.varId) ? value1.intvalue : value2.intvalue;
                    if (value1.valueKind == value2.valueKind)
                        result.valueKind = value1.valueKind;
                    if (value1.tokvalue->tokType() == Token::eString) {
                        const std::string s = value1.tokvalue->strValue();
                        const MathLib::bigint index = value2.intvalue;
                        if (index == s.size()) {
                            result.intvalue = 0;
                            setTokenValue(parent, result, settings);
                        } else if (index >= 0 && index < s.size()) {
                            result.intvalue = s[index];
                            setTokenValue(parent, result, settings);
                        }
                    } else if (value1.tokvalue->str() == "{") {
                        MathLib::bigint index = value2.intvalue;
                        const Token *element = value1.tokvalue->next();
                        while (index > 0 && element->str() != "}") {
                            if (element->str() == ",")
                                --index;
                            if (Token::Match(element, "[{}()[]]"))
                                break;
                            element = element->next();
                        }
                        if (Token::Match(element, "%num% [,}]")) {
                            result.intvalue = MathLib::toLongNumber(element->str());
                            setTokenValue(parent, result, settings);
                        }
                    }
                }
            }
        }
    }

    else if (Token::Match(parent, ":: %name%") && parent->astOperand2() == tok) {
        setTokenValue(parent, value, settings);
    }
}

static void setTokenValueCast(Token *parent, const ValueType &valueType, const ValueFlow::Value &value, const Settings *settings)
{
    if (valueType.pointer)
        setTokenValue(parent,value,settings);
    else if (valueType.type == ValueType::Type::CHAR)
        setTokenValue(parent, castValue(value, valueType.sign, settings->char_bit), settings);
    else if (valueType.type == ValueType::Type::SHORT)
        setTokenValue(parent, castValue(value, valueType.sign, settings->short_bit), settings);
    else if (valueType.type == ValueType::Type::INT)
        setTokenValue(parent, castValue(value, valueType.sign, settings->int_bit), settings);
    else if (valueType.type == ValueType::Type::LONG)
        setTokenValue(parent, castValue(value, valueType.sign, settings->long_bit), settings);
    else if (valueType.type == ValueType::Type::LONGLONG)
        setTokenValue(parent, castValue(value, valueType.sign, settings->long_long_bit), settings);
    else if (value.isIntValue()) {
        const long long charMax = settings->signedCharMax();
        const long long charMin = settings->signedCharMin();
        if (charMin <= value.intvalue && value.intvalue <= charMax) {
            // unknown type, but value is small so there should be no truncation etc
            setTokenValue(parent,value,settings);
        }
    }
}

static nonneg int getSizeOfType(const Token *typeTok, const Settings *settings)
{
    const ValueType &valueType = ValueType::parseDecl(typeTok, settings);
    if (valueType.pointer > 0)
        return settings->sizeof_pointer;
    if (valueType.type == ValueType::Type::BOOL || valueType.type == ValueType::Type::CHAR)
        return 1;
    if (valueType.type == ValueType::Type::SHORT)
        return settings->sizeof_short;
    if (valueType.type == ValueType::Type::INT)
        return settings->sizeof_int;
    if (valueType.type == ValueType::Type::LONG)
        return settings->sizeof_long;
    if (valueType.type == ValueType::Type::LONGLONG)
        return settings->sizeof_long_long;
    if (valueType.type == ValueType::Type::WCHAR_T)
        return settings->sizeof_wchar_t;

    return 0;
}

size_t ValueFlow::getSizeOf(const ValueType &vt, const Settings *settings)
{
    if (vt.pointer)
        return settings->sizeof_pointer;
    if (vt.type == ValueType::Type::CHAR)
        return 1;
    if (vt.type == ValueType::Type::SHORT)
        return settings->sizeof_short;
    if (vt.type == ValueType::Type::WCHAR_T)
        return settings->sizeof_wchar_t;
    if (vt.type == ValueType::Type::INT)
        return settings->sizeof_int;
    if (vt.type == ValueType::Type::LONG)
        return settings->sizeof_long;
    if (vt.type == ValueType::Type::LONGLONG)
        return settings->sizeof_long_long;
    if (vt.type == ValueType::Type::FLOAT)
        return settings->sizeof_float;
    if (vt.type == ValueType::Type::DOUBLE)
        return settings->sizeof_double;
    if (vt.type == ValueType::Type::LONGDOUBLE)
        return settings->sizeof_long_double;

    return 0;
}

// Handle various constants..
static Token * valueFlowSetConstantValue(Token *tok, const Settings *settings, bool cpp)
{
    if ((tok->isNumber() && MathLib::isInt(tok->str())) || (tok->tokType() == Token::eChar)) {
        ValueFlow::Value value(MathLib::toLongNumber(tok->str()));
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, value, settings);
    } else if (tok->isNumber() && MathLib::isFloat(tok->str())) {
        ValueFlow::Value value;
        value.valueType = ValueFlow::Value::FLOAT;
        value.floatValue = MathLib::toDoubleNumber(tok->str());
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, value, settings);
    } else if (tok->enumerator() && tok->enumerator()->value_known) {
        ValueFlow::Value value(tok->enumerator()->value);
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, value, settings);
    } else if (tok->str() == "NULL" || (cpp && tok->str() == "nullptr")) {
        ValueFlow::Value value(0);
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, value, settings);
    } else if (Token::simpleMatch(tok, "sizeof (")) {
        const Token *tok2 = tok->tokAt(2);
        // skip over tokens to find variable or type
        while (Token::Match(tok2, "%name% ::|.|[")) {
            if (tok2->next()->str() == "[")
                tok2 = tok2->linkAt(1)->next();
            else
                tok2 = tok2->tokAt(2);
        }
        if (Token::simpleMatch(tok, "sizeof ( *")) {
            const ValueType *vt = tok->tokAt(2)->valueType();
            const size_t sz = vt ? ValueFlow::getSizeOf(*vt, settings) : 0;
            if (sz > 0) {
                ValueFlow::Value value(sz);
                if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                    value.setKnown();
                setTokenValue(tok->next(), value, settings);
            }
        } else if (tok2->enumerator() && tok2->enumerator()->scope) {
            long long size = settings->sizeof_int;
            const Token * type = tok2->enumerator()->scope->enumType;
            if (type) {
                size = getSizeOfType(type, settings);
                if (size == 0)
                    tok->linkAt(1);
            }
            ValueFlow::Value value(size);
            if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                value.setKnown();
            setTokenValue(tok, value, settings);
            setTokenValue(tok->next(), value, settings);
        } else if (tok2->type() && tok2->type()->isEnumType()) {
            long long size = settings->sizeof_int;
            if (tok2->type()->classScope) {
                const Token * type = tok2->type()->classScope->enumType;
                if (type) {
                    size = getSizeOfType(type, settings);
                }
            }
            ValueFlow::Value value(size);
            if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                value.setKnown();
            setTokenValue(tok, value, settings);
            setTokenValue(tok->next(), value, settings);
        } else if (Token::Match(tok, "sizeof ( %var% ) / sizeof (") && tok->next()->astParent() == tok->tokAt(4)) {
            // Get number of elements in array
            const Token *sz1 = tok->tokAt(2);
            const Token *sz2 = tok->tokAt(7);
            const int varid1 = sz1->varId();
            if (varid1 &&
                sz1->variable() &&
                sz1->variable()->isArray() &&
                !sz1->variable()->dimensions().empty() &&
                sz1->variable()->dimensionKnown(0) &&
                (Token::Match(sz2, "* %varid% )", varid1) || Token::Match(sz2, "%varid% [ 0 ] )", varid1))) {
                ValueFlow::Value value(sz1->variable()->dimension(0));
                if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                    value.setKnown();
                setTokenValue(tok->tokAt(4), value, settings);
            }
        } else if (Token::Match(tok2, "%var% )")) {
            const Variable *var = tok2->variable();
            // only look for single token types (no pointers or references yet)
            if (var && var->typeStartToken() == var->typeEndToken()) {
                // find the size of the type
                size_t size = 0;
                if (var->isEnumType()) {
                    size = settings->sizeof_int;
                    if (var->type()->classScope && var->type()->classScope->enumType)
                        size = getSizeOfType(var->type()->classScope->enumType, settings);
                } else if (var->valueType()) {
                    size = ValueFlow::getSizeOf(*var->valueType(), settings);
                } else if (!var->type()) {
                    size = getSizeOfType(var->typeStartToken(), settings);
                }
                // find the number of elements
                size_t count = 1;
                for (size_t i = 0; i < var->dimensions().size(); ++i) {
                    if (var->dimensionKnown(i))
                        count *= var->dimension(i);
                    else
                        count = 0;
                }
                if (size && count > 0) {
                    ValueFlow::Value value(count * size);
                    if (settings->platformType != cppcheck::Platform::Unspecified)
                        value.setKnown();
                    setTokenValue(tok, value, settings);
                    setTokenValue(tok->next(), value, settings);
                }
            }
        } else if (tok2->tokType() == Token::eString) {
            size_t sz = Token::getStrSize(tok2, settings);
            if (sz > 0) {
                ValueFlow::Value value(sz);
                value.setKnown();
                setTokenValue(const_cast<Token *>(tok->next()), value, settings);
            }
        } else if (tok2->tokType() == Token::eChar) {
            nonneg int sz = 0;
            if (cpp && settings->standards.cpp >= Standards::CPP20 && tok2->isUtf8())
                sz = 1;
            else if (tok2->isUtf16())
                sz = 2;
            else if (tok2->isUtf32())
                sz = 4;
            else if (tok2->isLong())
                sz = settings->sizeof_wchar_t;
            else if ((tok2->isCChar() && !cpp) || (tok2->isCMultiChar()))
                sz = settings->sizeof_int;
            else
                sz = 1;

            if (sz > 0) {
                ValueFlow::Value value(sz);
                value.setKnown();
                setTokenValue(tok->next(), value, settings);
            }
        } else if (!tok2->type()) {
            const ValueType &vt = ValueType::parseDecl(tok2,settings);
            const size_t sz = ValueFlow::getSizeOf(vt, settings);
            if (sz > 0) {
                ValueFlow::Value value(sz);
                if (!tok2->isTemplateArg() && settings->platformType != cppcheck::Platform::Unspecified)
                    value.setKnown();
                setTokenValue(tok->next(), value, settings);
            }
        }
        // skip over enum
        tok = tok->linkAt(1);
    }
    return tok->next();
}


static void valueFlowNumber(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok;) {
        tok = valueFlowSetConstantValue(tok, tokenlist->getSettings(), tokenlist->isCPP());
    }

    if (tokenlist->isCPP()) {
        for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
            if (tok->isName() && !tok->varId() && Token::Match(tok, "false|true")) {
                ValueFlow::Value value(tok->str() == "true");
                if (!tok->isTemplateArg())
                    value.setKnown();
                setTokenValue(tok, value, tokenlist->getSettings());
            } else if (Token::Match(tok, "[(,] NULL [,)]")) {
                // NULL function parameters are not simplified in the
                // normal tokenlist
                ValueFlow::Value value(0);
                if (!tok->isTemplateArg())
                    value.setKnown();
                setTokenValue(tok->next(), value, tokenlist->getSettings());
            }
        }
    }
}

static void valueFlowString(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->tokType() == Token::eString) {
            ValueFlow::Value strvalue;
            strvalue.valueType = ValueFlow::Value::TOK;
            strvalue.tokvalue = tok;
            strvalue.setKnown();
            setTokenValue(tok, strvalue, tokenlist->getSettings());
        }
    }
}

static void valueFlowArray(TokenList *tokenlist)
{
    std::map<int, const Token *> constantArrays;

    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->varId() > 0U) {
            // array
            const std::map<int, const Token *>::const_iterator it = constantArrays.find(tok->varId());
            if (it != constantArrays.end()) {
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::TOK;
                value.tokvalue = it->second;
                value.setKnown();
                setTokenValue(tok, value, tokenlist->getSettings());
            }

            // pointer = array
            else if (tok->variable() &&
                     tok->variable()->isArray() &&
                     Token::simpleMatch(tok->astParent(), "=") &&
                     tok == tok->astParent()->astOperand2() &&
                     tok->astParent()->astOperand1() &&
                     tok->astParent()->astOperand1()->variable() &&
                     tok->astParent()->astOperand1()->variable()->isPointer()) {
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::TOK;
                value.tokvalue = tok;
                value.setKnown();
                setTokenValue(tok, value, tokenlist->getSettings());
            }
            continue;
        }

        if (Token::Match(tok, "const %type% %var% [ %num%| ] = {")) {
            const Token *vartok = tok->tokAt(2);
            const Token *rhstok = vartok->next()->link()->tokAt(2);
            constantArrays[vartok->varId()] = rhstok;
            tok = rhstok->link();
            continue;
        }

        else if (Token::Match(tok, "const char %var% [ %num%| ] = %str% ;")) {
            const Token *vartok = tok->tokAt(2);
            const Token *strtok = vartok->next()->link()->tokAt(2);
            constantArrays[vartok->varId()] = strtok;
            tok = strtok->next();
            continue;
        }
    }
}

static bool isNonZero(const Token *tok)
{
    return tok && (!tok->hasKnownIntValue() || tok->values().front().intvalue != 0);
}

static const Token *getOtherOperand(const Token *tok)
{
    if (!tok)
        return nullptr;
    if (!tok->astParent())
        return nullptr;
    if (tok->astParent()->astOperand1() != tok)
        return tok->astParent()->astOperand1();
    if (tok->astParent()->astOperand2() != tok)
        return tok->astParent()->astOperand2();
    return nullptr;
}

static void valueFlowArrayBool(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->hasKnownIntValue())
            continue;
        const Variable *var = nullptr;
        bool known = false;
        std::list<ValueFlow::Value>::const_iterator val =
            std::find_if(tok->values().begin(), tok->values().end(), std::mem_fn(&ValueFlow::Value::isTokValue));
        if (val == tok->values().end()) {
            var = tok->variable();
            known = true;
        } else {
            var = val->tokvalue->variable();
            known = val->isKnown();
        }
        if (!var)
            continue;
        if (!var->isArray() || var->isArgument() || var->isStlType())
            continue;
        if (isNonZero(getOtherOperand(tok)) && Token::Match(tok->astParent(), "%comp%"))
            continue;
        // TODO: Check for function argument
        if ((astIsBool(tok->astParent()) && !Token::Match(tok->astParent(), "(|%name%")) ||
            (tok->astParent() && Token::Match(tok->astParent()->previous(), "if|while|for ("))) {
            ValueFlow::Value value{1};
            if (known)
                value.setKnown();
            setTokenValue(tok, value, tokenlist->getSettings());
        }
    }
}

static void valueFlowPointerAlias(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        // not address of
        if (!tok->isUnaryOp("&"))
            continue;

        // parent should be a '='
        if (!Token::simpleMatch(tok->astParent(), "="))
            continue;

        // child should be some buffer or variable
        const Token *vartok = tok->astOperand1();
        while (vartok) {
            if (vartok->str() == "[")
                vartok = vartok->astOperand1();
            else if (vartok->str() == "." || vartok->str() == "::")
                vartok = vartok->astOperand2();
            else
                break;
        }
        if (!(vartok && vartok->variable() && !vartok->variable()->isPointer()))
            continue;

        ValueFlow::Value value;
        value.valueType = ValueFlow::Value::TOK;
        value.tokvalue = tok;
        setTokenValue(tok, value, tokenlist->getSettings());
    }
}

static void valueFlowPointerAliasDeref(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->isUnaryOp("*"))
            continue;
        if (!astIsPointer(tok->astOperand1()))
            continue;

        const Token* lifeTok = nullptr;
        ErrorPath errorPath;
        for (const ValueFlow::Value& v:tok->astOperand1()->values()) {
            if (!v.isLocalLifetimeValue())
                continue;
            lifeTok = v.tokvalue;
            errorPath = v.errorPath;
        }
        if (!lifeTok)
            continue;
        if (lifeTok->varId() == 0)
            continue;
        const Variable * var = lifeTok->variable();
        if (!var)
            continue;
        if (!var->isConst() && isVariableChanged(lifeTok->next(), tok, lifeTok->varId(), !var->isLocal(), tokenlist->getSettings(), tokenlist->isCPP()))
            continue;
        for (const ValueFlow::Value& v:lifeTok->values()) {
            // TODO: Move container size values to generic forward
            // Forward uninit values since not all values can be forwarded directly
            if (!(v.isContainerSizeValue() || v.isUninitValue()))
                continue;
            ValueFlow::Value value = v;
            value.errorPath.insert(value.errorPath.begin(), errorPath.begin(), errorPath.end());
            setTokenValue(tok, value, tokenlist->getSettings());
        }
    }
}

static void valueFlowBitAnd(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->str() != "&")
            continue;

        if (tok->hasKnownValue())
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
        while (bit <= (MathLib::bigint_bits - 2) && ((((MathLib::bigint)1) << bit) < number))
            ++bit;

        if ((((MathLib::bigint)1) << bit) == number) {
            setTokenValue(tok, ValueFlow::Value(0), tokenlist->getSettings());
            setTokenValue(tok, ValueFlow::Value(number), tokenlist->getSettings());
        }
    }
}

static void valueFlowSameExpressions(TokenList *tokenlist)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->hasKnownValue())
            continue;

        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        if (tok->astOperand1()->isLiteral() || tok->astOperand2()->isLiteral())
            continue;

        if (!astIsIntegral(tok->astOperand1(), false) && !astIsIntegral(tok->astOperand2(), false))
            continue;

        ValueFlow::Value val;

        if (Token::Match(tok, "==|>=|<=|/")) {
            val = ValueFlow::Value(1);
            val.setKnown();
        }

        if (Token::Match(tok, "!=|>|<|%|-")) {
            val = ValueFlow::Value(0);
            val.setKnown();
        }

        if (!val.isKnown())
            continue;

        if (isSameExpression(tokenlist->isCPP(), false, tok->astOperand1(), tok->astOperand2(), tokenlist->getSettings()->library, true, true, &val.errorPath)) {
            setTokenValue(tok, val, tokenlist->getSettings());
        }
    }
}

static void valueFlowTerminatingCondition(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    const bool cpp = symboldatabase->isCPP();
    typedef std::pair<const Token*, const Scope*> Condition;
    for (const Scope * scope : symboldatabase->functionScopes) {
        bool skipFunction = false;
        std::vector<Condition> conds;
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->isIncompleteVar()) {
                if (settings->debugwarnings)
                    bailoutIncompleteVar(tokenlist, errorLogger, tok, "Skipping function due to incomplete variable " + tok->str());
                skipFunction = true;
                break;
            }
            if (!Token::simpleMatch(tok, "if ("))
                continue;
            // Skip known values
            if (tok->next()->hasKnownValue())
                continue;
            const Token * condTok = tok->next();
            if (!Token::simpleMatch(condTok->link(), ") {"))
                continue;
            const Token * blockTok = condTok->link()->tokAt(1);
            // Check if the block terminates early
            if (!isEscapeScope(blockTok, tokenlist))
                continue;
            // Check if any variables are modified in scope
            if (isExpressionChanged(condTok->astOperand2(), blockTok->link(), scope->bodyEnd, settings, cpp))
                continue;
            // TODO: Handle multiple conditions
            if (Token::Match(condTok->astOperand2(), "%oror%|%or%|&|&&"))
                continue;
            const Scope * condScope = nullptr;
            for (const Scope * parent = condTok->scope(); parent; parent = parent->nestedIn) {
                if (parent->type == Scope::eIf ||
                    parent->type == Scope::eWhile ||
                    parent->type == Scope::eSwitch) {
                    condScope = parent;
                    break;
                }
            }
            conds.emplace_back(condTok->astOperand2(), condScope);
        }
        if (skipFunction)
            break;
        for (Condition cond:conds) {
            if (!cond.first)
                continue;
            Token *const startToken = cond.first->findExpressionStartEndTokens().second->next();
            for (Token* tok = startToken; tok != scope->bodyEnd; tok = tok->next()) {
                if (!Token::Match(tok, "%comp%"))
                    continue;
                // Skip known values
                if (tok->hasKnownValue())
                    continue;
                if (cond.second) {
                    bool bail = true;
                    for (const Scope * parent = tok->scope()->nestedIn; parent; parent = parent->nestedIn) {
                        if (parent == cond.second) {
                            bail = false;
                            break;
                        }
                    }
                    if (bail)
                        continue;
                }
                ErrorPath errorPath;
                if (isOppositeCond(true, cpp, tok, cond.first, settings->library, true, true, &errorPath)) {
                    ValueFlow::Value val(1);
                    val.setKnown();
                    val.condition = cond.first;
                    val.errorPath = errorPath;
                    val.errorPath.emplace_back(cond.first, "Assuming condition '" + cond.first->expressionString() + "' is false");
                    setTokenValue(tok, val, tokenlist->getSettings());
                } else if (isSameExpression(cpp, true, tok, cond.first, settings->library, true, true, &errorPath)) {
                    ValueFlow::Value val(0);
                    val.setKnown();
                    val.condition = cond.first;
                    val.errorPath = errorPath;
                    val.errorPath.emplace_back(cond.first, "Assuming condition '" + cond.first->expressionString() + "' is false");
                    setTokenValue(tok, val, tokenlist->getSettings());
                }
            }
        }
    }
}

static bool getExpressionRange(const Token *expr, MathLib::bigint *minvalue, MathLib::bigint *maxvalue)
{
    if (expr->hasKnownIntValue()) {
        if (minvalue)
            *minvalue = expr->values().front().intvalue;
        if (maxvalue)
            *maxvalue = expr->values().front().intvalue;
        return true;
    }

    if (expr->str() == "&" && expr->astOperand1() && expr->astOperand2()) {
        MathLib::bigint vals[4];
        bool lhsHasKnownRange = getExpressionRange(expr->astOperand1(), &vals[0], &vals[1]);
        bool rhsHasKnownRange = getExpressionRange(expr->astOperand2(), &vals[2], &vals[3]);
        if (!lhsHasKnownRange && !rhsHasKnownRange)
            return false;
        if (!lhsHasKnownRange || !rhsHasKnownRange) {
            if (minvalue)
                *minvalue = lhsHasKnownRange ? vals[0] : vals[2];
            if (maxvalue)
                *maxvalue = lhsHasKnownRange ? vals[1] : vals[3];
        } else {
            if (minvalue)
                *minvalue = vals[0] & vals[2];
            if (maxvalue)
                *maxvalue = vals[1] & vals[3];
        }
        return true;
    }

    if (expr->str() == "%" && expr->astOperand1() && expr->astOperand2()) {
        MathLib::bigint vals[4];
        if (!getExpressionRange(expr->astOperand2(), &vals[2], &vals[3]))
            return false;
        if (vals[2] <= 0)
            return false;
        bool lhsHasKnownRange = getExpressionRange(expr->astOperand1(), &vals[0], &vals[1]);
        if (lhsHasKnownRange && vals[0] < 0)
            return false;
        // If lhs has unknown value, it must be unsigned
        if (!lhsHasKnownRange && (!expr->astOperand1()->valueType() || expr->astOperand1()->valueType()->sign != ValueType::Sign::UNSIGNED))
            return false;
        if (minvalue)
            *minvalue = 0;
        if (maxvalue)
            *maxvalue = vals[3] - 1;
        return true;
    }

    return false;
}

static void valueFlowRightShift(TokenList *tokenList, const Settings* settings)
{
    for (Token *tok = tokenList->front(); tok; tok = tok->next()) {
        if (tok->str() != ">>")
            continue;

        if (tok->hasKnownValue())
            continue;

        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        if (!tok->astOperand2()->hasKnownValue())
            continue;

        const MathLib::bigint rhsvalue = tok->astOperand2()->values().front().intvalue;
        if (rhsvalue < 0)
            continue;

        if (!tok->astOperand1()->valueType() || !tok->astOperand1()->valueType()->isIntegral())
            continue;

        if (!tok->astOperand2()->valueType() || !tok->astOperand2()->valueType()->isIntegral())
            continue;

        MathLib::bigint lhsmax=0;
        if (!getExpressionRange(tok->astOperand1(), nullptr, &lhsmax))
            continue;
        if (lhsmax < 0)
            continue;
        int lhsbits;
        if ((tok->astOperand1()->valueType()->type == ValueType::Type::CHAR) ||
            (tok->astOperand1()->valueType()->type == ValueType::Type::SHORT) ||
            (tok->astOperand1()->valueType()->type == ValueType::Type::WCHAR_T) ||
            (tok->astOperand1()->valueType()->type == ValueType::Type::BOOL) ||
            (tok->astOperand1()->valueType()->type == ValueType::Type::INT))
            lhsbits = settings->int_bit;
        else if (tok->astOperand1()->valueType()->type == ValueType::Type::LONG)
            lhsbits = settings->long_bit;
        else if (tok->astOperand1()->valueType()->type == ValueType::Type::LONGLONG)
            lhsbits = settings->long_long_bit;
        else
            continue;
        if (rhsvalue >= lhsbits || rhsvalue >= MathLib::bigint_bits || (1ULL << rhsvalue) <= lhsmax)
            continue;

        ValueFlow::Value val(0);
        val.setKnown();
        setTokenValue(tok, val, tokenList->getSettings());
    }
}

static void valueFlowOppositeCondition(SymbolDatabase *symboldatabase, const Settings *settings)
{
    for (const Scope &scope : symboldatabase->scopeList) {
        if (scope.type != Scope::eIf)
            continue;
        Token *tok = const_cast<Token *>(scope.classDef);
        if (!Token::simpleMatch(tok, "if ("))
            continue;
        const Token *cond1 = tok->next()->astOperand2();
        if (!cond1 || !cond1->isComparisonOp())
            continue;
        const bool cpp = symboldatabase->isCPP();
        Token *tok2 = tok->linkAt(1);
        while (Token::simpleMatch(tok2, ") {")) {
            tok2 = tok2->linkAt(1);
            if (!Token::simpleMatch(tok2, "} else { if ("))
                break;
            Token *ifOpenBraceTok = tok2->tokAt(4);
            Token *cond2 = ifOpenBraceTok->astOperand2();
            if (!cond2 || !cond2->isComparisonOp())
                continue;
            if (isOppositeCond(true, cpp, cond1, cond2, settings->library, true, true)) {
                ValueFlow::Value value(1);
                value.setKnown();
                setTokenValue(cond2, value, settings);
            }
            tok2 = ifOpenBraceTok->link();
        }
    }
}

static void valueFlowEnumValue(SymbolDatabase * symboldatabase, const Settings * settings)
{

    for (Scope & scope : symboldatabase->scopeList) {
        if (scope.type != Scope::eEnum)
            continue;
        MathLib::bigint value = 0;
        bool prev_enum_is_known = true;

        for (Enumerator & enumerator : scope.enumeratorList) {
            if (enumerator.start) {
                Token *rhs = enumerator.start->previous()->astOperand2();
                ValueFlow::valueFlowConstantFoldAST(rhs, settings);
                if (rhs && rhs->hasKnownIntValue()) {
                    enumerator.value = rhs->values().front().intvalue;
                    enumerator.value_known = true;
                    value = enumerator.value + 1;
                    prev_enum_is_known = true;
                } else
                    prev_enum_is_known = false;
            } else if (prev_enum_is_known) {
                enumerator.value = value++;
                enumerator.value_known = true;
            }
        }
    }
}

static void valueFlowGlobalConstVar(TokenList* tokenList, const Settings *settings)
{
    // Get variable values...
    std::map<const Variable*, ValueFlow::Value> vars;
    for (const Token* tok = tokenList->front(); tok; tok = tok->next()) {
        if (!tok->variable())
            continue;
        // Initialization...
        if (tok == tok->variable()->nameToken() &&
            !tok->variable()->isVolatile() &&
            !tok->variable()->isArgument() &&
            tok->variable()->isConst() &&
            tok->valueType() &&
            tok->valueType()->isIntegral() &&
            tok->valueType()->pointer == 0 &&
            tok->valueType()->constness == 1 &&
            Token::Match(tok, "%name% =") &&
            tok->next()->astOperand2() &&
            tok->next()->astOperand2()->hasKnownIntValue()) {
            vars[tok->variable()] = tok->next()->astOperand2()->values().front();
        }
    }

    // Set values..
    for (Token* tok = tokenList->front(); tok; tok = tok->next()) {
        if (!tok->variable())
            continue;
        std::map<const Variable*, ValueFlow::Value>::const_iterator var = vars.find(tok->variable());
        if (var == vars.end())
            continue;
        setTokenValue(tok, var->second, settings);
    }
}

static void valueFlowGlobalStaticVar(TokenList *tokenList, const Settings *settings)
{
    // Get variable values...
    std::map<const Variable *, ValueFlow::Value> vars;
    for (const Token *tok = tokenList->front(); tok; tok = tok->next()) {
        if (!tok->variable())
            continue;
        // Initialization...
        if (tok == tok->variable()->nameToken() &&
            tok->variable()->isStatic() &&
            !tok->variable()->isConst() &&
            tok->valueType() &&
            tok->valueType()->isIntegral() &&
            tok->valueType()->pointer == 0 &&
            tok->valueType()->constness == 0 &&
            Token::Match(tok, "%name% =") &&
            tok->next()->astOperand2() &&
            tok->next()->astOperand2()->hasKnownIntValue()) {
            vars[tok->variable()] = tok->next()->astOperand2()->values().front();
        } else {
            // If variable is written anywhere in TU then remove it from vars
            if (!tok->astParent())
                continue;
            if (Token::Match(tok->astParent(), "++|--|&") && !tok->astParent()->astOperand2())
                vars.erase(tok->variable());
            else if (tok->astParent()->isAssignmentOp()) {
                if (tok == tok->astParent()->astOperand1())
                    vars.erase(tok->variable());
                else if (tokenList->isCPP() && Token::Match(tok->astParent()->tokAt(-2), "& %name% ="))
                    vars.erase(tok->variable());
            } else if (isLikelyStreamRead(tokenList->isCPP(), tok->astParent())) {
                vars.erase(tok->variable());
            } else if (Token::Match(tok->astParent(), "[(,]"))
                vars.erase(tok->variable());
        }
    }

    // Set values..
    for (Token *tok = tokenList->front(); tok; tok = tok->next()) {
        if (!tok->variable())
            continue;
        std::map<const Variable *, ValueFlow::Value>::const_iterator var = vars.find(tok->variable());
        if (var == vars.end())
            continue;
        setTokenValue(tok, var->second, settings);
    }
}

static Analyzer::Action valueFlowForwardVariable(Token* const startToken,
        const Token* const endToken,
        const Variable* const var,
        std::list<ValueFlow::Value> values,
        TokenList* const tokenlist,
        const Settings* const settings);

static void valueFlowReverse(TokenList* tokenlist,
                             Token* tok,
                             const Token* const varToken,
                             ValueFlow::Value val,
                             ValueFlow::Value val2,
                             ErrorLogger* errorLogger,
                             const Settings* settings);

static bool isConditionKnown(const Token* tok, bool then)
{
    const char* op = "||";
    if (then)
        op = "&&";
    const Token* parent = tok->astParent();
    while (parent && (parent->str() == op || parent->str() == "!"))
        parent = parent->astParent();
    return (parent && parent->str() == "(");
}

static void valueFlowBeforeCondition(TokenList *tokenlist, SymbolDatabase *symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope * scope : symboldatabase->functionScopes) {
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            MathLib::bigint num = 0;
            const Token *vartok = nullptr;
            if (tok->isComparisonOp() && tok->astOperand1() && tok->astOperand2()) {
                if (tok->astOperand1()->isName() && tok->astOperand2()->hasKnownIntValue()) {
                    vartok = tok->astOperand1();
                    num = tok->astOperand2()->values().front().intvalue;
                } else if (tok->astOperand1()->hasKnownIntValue() && tok->astOperand2()->isName()) {
                    vartok = tok->astOperand2();
                    num = tok->astOperand1()->values().front().intvalue;
                } else {
                    continue;
                }
            } else if (Token::Match(tok->previous(), "if|while ( %name% %oror%|&&|)") ||
                       Token::Match(tok, "%oror%|&& %name% %oror%|&&|)")) {
                vartok = tok->next();
                num = 0;
            } else if (Token::simpleMatch(tok, "!") && Token::Match(tok->astOperand1(), "%name%")) {
                vartok = tok->astOperand1();
                num = 0;
            } else if (Token::simpleMatch(tok->astParent(), "?") && Token::Match(tok, "%name%")) {
                vartok = tok;
                num = 0;
            } else {
                continue;
            }

            int varid = vartok->varId();
            const Variable * const var = vartok->variable();

            if (varid == 0U || !var)
                continue;

            if (Token::simpleMatch(tok->astParent(), "?") && tok->astParent()->isExpandedMacro()) {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok, "variable " + var->name() + ", condition is defined in macro");
                continue;
            }

            // bailout: for/while-condition, variable is changed in while loop
            for (const Token *tok2 = tok; tok2; tok2 = tok2->astParent()) {
                if (tok2->astParent() || tok2->str() != "(" || !Token::simpleMatch(tok2->link(), ") {"))
                    continue;

                // Variable changed in 3rd for-expression
                if (Token::simpleMatch(tok2->previous(), "for (")) {
                    if (tok2->astOperand2() && tok2->astOperand2()->astOperand2() && isVariableChanged(tok2->astOperand2()->astOperand2(), tok2->link(), varid, var->isGlobal(), settings, tokenlist->isCPP())) {
                        varid = 0U;
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok, "variable " + var->name() + " used in loop");
                    }
                }

                // Variable changed in loop code
                if (Token::Match(tok2->previous(), "for|while (")) {
                    const Token * const start = tok2->link()->next();
                    const Token * const end   = start->link();

                    if (isVariableChanged(start,end,varid,var->isGlobal(),settings, tokenlist->isCPP())) {
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
            if (Token::Match(tok, "<|>")) {
                if (num != 0)
                    continue;
                if (var->valueType() && var->valueType()->sign != ValueType::Sign::UNSIGNED)
                    continue;
            }
            ValueFlow::Value val(tok, num);
            val.varId = varid;
            ValueFlow::Value val2;
            if (num==1U && Token::Match(tok,"<=|>=")) {
                if (var->isUnsigned()) {
                    val2 = ValueFlow::Value(tok,0);
                    val2.varId = varid;
                }
            }
            Token* startTok = tok->astParent() ? tok->astParent() : tok->previous();
            valueFlowReverse(tokenlist, startTok, vartok, val, val2, errorLogger, settings);
        }
    }
}

static void valueFlowAST(Token *tok, nonneg int varid, const ValueFlow::Value &value, const Settings *settings)
{
    if (!tok)
        return;
    if (tok->varId() == varid)
        setTokenValue(tok, value, settings);
    valueFlowAST(tok->astOperand1(), varid, value, settings);
    if (tok->str() == "&&" && tok->astOperand1() && tok->astOperand1()->getValue(0)) {
        ProgramMemory pm;
        pm.setValue(varid,value);
        if (conditionIsFalse(tok->astOperand1(), pm))
            return;
    } else if (tok->str() == "||" && tok->astOperand1()) {
        const std::list<ValueFlow::Value> &values = tok->astOperand1()->values();
        const bool nonzero = std::any_of(values.cbegin(), values.cend(),
        [=](const ValueFlow::Value &v) {
            return v.intvalue != 0;
        });
        if (!nonzero)
            return;
        ProgramMemory pm;
        pm.setValue(varid,value);
        if (conditionIsTrue(tok->astOperand1(), pm))
            return;
    }
    valueFlowAST(tok->astOperand2(), varid, value, settings);
}

static const std::string& invertAssign(const std::string& assign)
{
    static std::unordered_map<std::string, std::string> lookup = {
        {"+=", "-="}, {"-=", "+="}, {"*=", "/="}, {"/=", "*="}, {"<<=", ">>="}, {">>=", "<<="}, {"^=", "^="}
    };
    static std::string empty = "";
    auto it = lookup.find(assign);
    if (it == lookup.end())
        return empty;
    else
        return it->second;
}

static bool evalAssignment(ValueFlow::Value &lhsValue, const std::string &assign, const ValueFlow::Value &rhsValue)
{
    if (lhsValue.isIntValue()) {
        if (assign == "=")
            lhsValue.intvalue = rhsValue.intvalue;
        else if (assign == "+=")
            lhsValue.intvalue += rhsValue.intvalue;
        else if (assign == "-=")
            lhsValue.intvalue -= rhsValue.intvalue;
        else if (assign == "*=")
            lhsValue.intvalue *= rhsValue.intvalue;
        else if (assign == "/=") {
            if (rhsValue.intvalue == 0)
                return false;
            lhsValue.intvalue /= rhsValue.intvalue;
        } else if (assign == "%=") {
            if (rhsValue.intvalue == 0)
                return false;
            lhsValue.intvalue %= rhsValue.intvalue;
        } else if (assign == "&=")
            lhsValue.intvalue &= rhsValue.intvalue;
        else if (assign == "|=")
            lhsValue.intvalue |= rhsValue.intvalue;
        else if (assign == "^=")
            lhsValue.intvalue ^= rhsValue.intvalue;
        else if (assign == "<<=") {
            if (rhsValue.intvalue < 0)
                return false;
            lhsValue.intvalue <<= rhsValue.intvalue;
        } else if (assign == ">>=") {
            if (rhsValue.intvalue < 0)
                return false;
            lhsValue.intvalue >>= rhsValue.intvalue;
        } else
            return false;
    } else if (lhsValue.isFloatValue()) {
        if (assign == "=")
            lhsValue.intvalue = rhsValue.intvalue;
        else if (assign == "+=")
            lhsValue.floatValue += rhsValue.intvalue;
        else if (assign == "-=")
            lhsValue.floatValue -= rhsValue.intvalue;
        else if (assign == "*=")
            lhsValue.floatValue *= rhsValue.intvalue;
        else if (assign == "/=")
            lhsValue.floatValue /= rhsValue.intvalue;
        else
            return false;
    } else {
        return false;
    }
    return true;
}

// Check if its an alias of the variable or is being aliased to this variable
static bool isAliasOf(const Variable * var, const Token *tok, nonneg int varid, const std::list<ValueFlow::Value>& values, bool* inconclusive = nullptr)
{
    if (tok->varId() == varid)
        return false;
    if (tok->varId() == 0)
        return false;
    if (isAliasOf(tok, varid, inconclusive))
        return true;
    if (var && !var->isPointer())
        return false;
    // Search through non value aliases
    for (const ValueFlow::Value &val : values) {
        if (!val.isNonValue())
            continue;
        if (val.isInconclusive())
            continue;
        if (val.isLifetimeValue() && !val.isLocalLifetimeValue())
            continue;
        if (val.isLifetimeValue() && val.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
            continue;
        if (!Token::Match(val.tokvalue, ".|&|*|%var%"))
            continue;
        if (astHasVar(val.tokvalue, tok->varId()))
            return true;
    }
    return false;
}

static const ValueFlow::Value* getKnownValue(const Token* tok, ValueFlow::Value::ValueType type)
{
    if (!tok)
        return nullptr;
    auto it = std::find_if(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& v) {
        return v.isKnown() && v.valueType == type;
    });
    if (it != tok->values().end())
        return &*it;
    return nullptr;
}

static bool bifurcate(const Token* tok, const std::set<nonneg int>& varids, const Settings* settings, int depth = 20)
{
    if (depth < 0)
        return false;
    if (!tok)
        return true;
    if (tok->hasKnownIntValue())
        return true;
    if (Token::Match(tok, "%cop%"))
        return bifurcate(tok->astOperand1(), varids, settings, depth) && bifurcate(tok->astOperand2(), varids, settings, depth);
    if (Token::Match(tok, "%var%")) {
        if (varids.count(tok->varId()) > 0)
            return true;
        const Variable* var = tok->variable();
        if (!var)
            return false;
        const Token* start = var->declEndToken();
        if (!start)
            return false;
        if (start->strAt(-1) == ")" || start->strAt(-1) == "}")
            return false;
        if (Token::Match(start, "; %varid% =", var->declarationId()))
            start = start->tokAt(2);
        if (var->isConst() ||
            !isVariableChanged(start->next(), tok, var->declarationId(), var->isGlobal(), settings, true))
            return var->isArgument() || bifurcate(start->astOperand2(), varids, settings, depth - 1);
        return false;
    }
    return false;
}

struct SelectMapKeys {
    template<class Pair>
    typename Pair::first_type operator()(const Pair& p) const {
        return p.first;
    }
};

struct SelectMapValues {
    template<class Pair>
    typename Pair::second_type operator()(const Pair& p) const {
        return p.second;
    }
};

struct ValueFlowAnalyzer : Analyzer {
    const TokenList* tokenlist;
    ProgramMemoryState pms;

    ValueFlowAnalyzer() : tokenlist(nullptr), pms() {}

    ValueFlowAnalyzer(const TokenList* t) : tokenlist(t), pms() {}

    virtual const ValueFlow::Value* getValue(const Token* tok) const = 0;
    virtual ValueFlow::Value* getValue(const Token* tok) = 0;

    virtual void makeConditional() = 0;

    virtual void addErrorPath(const Token* tok, const std::string& s) = 0;

    virtual bool match(const Token* tok) const = 0;

    virtual bool isAlias(const Token* tok, bool& inconclusive) const = 0;

    using ProgramState = std::unordered_map<nonneg int, ValueFlow::Value>;

    virtual ProgramState getProgramState() const = 0;

    virtual const ValueType* getValueType(const Token*) const {
        return nullptr;
    }
    virtual int getIndirect(const Token* tok) const {
        const ValueFlow::Value* value = getValue(tok);
        if (value)
            return value->indirect;
        return 0;
    }

    virtual bool isGlobal() const {
        return false;
    }

    virtual bool invalid() const {
        return false;
    }

    bool isCPP() const {
        return tokenlist->isCPP();
    }

    const Settings* getSettings() const {
        return tokenlist->getSettings();
    }

    virtual Action isModified(const Token* tok) const {
        Action read = Action::Read;
        bool inconclusive = false;
        if (isVariableChangedByFunctionCall(tok, getIndirect(tok), getSettings(), &inconclusive))
            return read | Action::Invalid;
        if (inconclusive)
            return read | Action::Inconclusive;
        if (isVariableChanged(tok, getIndirect(tok), getSettings(), isCPP())) {
            if (Token::Match(tok->astParent(), "*|[|.|++|--"))
                return read | Action::Invalid;
            const ValueFlow::Value* value = getValue(tok);
            // Check if its assigned to the same value
            if (value && !value->isImpossible() && Token::simpleMatch(tok->astParent(), "=") && astIsLHS(tok) &&
                astIsIntegral(tok->astParent()->astOperand2(), false)) {
                std::vector<int> result = evaluate(tok->astParent()->astOperand2());
                if (!result.empty() && value->equalTo(result.front()))
                    return Action::Idempotent;
            }
            return Action::Invalid;
        }
        return read;
    }

    virtual Action isAliasModified(const Token* tok) const {
        int indirect = 0;
        int baseIndirect = 0;
        const ValueType* vt = getValueType(tok);
        if (vt)
            baseIndirect = vt->pointer;
        if (tok->valueType())
            indirect = std::max<int>(0, tok->valueType()->pointer - baseIndirect);
        if (isVariableChanged(tok, indirect, getSettings(), isCPP()))
            return Action::Invalid;
        return Action::None;
    }

    virtual Action isWritable(const Token* tok, Direction d) const {
        const ValueFlow::Value* value = getValue(tok);
        if (!value)
            return Action::None;
        if (!(value->isIntValue() || value->isFloatValue()))
            return Action::None;
        const Token* parent = tok->astParent();

        if (parent && parent->isAssignmentOp() && astIsLHS(tok) &&
            parent->astOperand2()->hasKnownValue()) {
            // If the operator is invertible
            if (d == Direction::Reverse && (parent->str() == "&=" || parent->str() == "|=" || parent->str() == "%="))
                return Action::None;
            const Token* rhs = parent->astOperand2();
            const ValueFlow::Value* rhsValue = getKnownValue(rhs, ValueFlow::Value::ValueType::INT);
            Action a;
            if (!rhsValue)
                a = Action::Invalid;
            else
                a = Action::Write;
            if (parent->str() != "=") {
                a |= Action::Read;
            } else {
                if (rhsValue && !value->isImpossible() && value->equalValue(*rhsValue))
                    a = Action::Idempotent;
            }
            return a;
        }

        // increment/decrement
        if (Token::Match(tok->previous(), "++|-- %name%") || Token::Match(tok, "%name% ++|--")) {
            return Action::Read | Action::Write;
        }
        return Action::None;
    }

    static const std::string& getAssign(const Token* tok, Direction d) {
        if (d == Direction::Forward)
            return tok->str();
        else
            return invertAssign(tok->str());
    }

    virtual void writeValue(ValueFlow::Value* value, const Token* tok, Direction d) const {
        if (!value)
            return;
        if (!tok->astParent())
            return;
        if (tok->astParent()->isAssignmentOp()) {
            // TODO: Check result
            if (evalAssignment(*value,
                               getAssign(tok->astParent(), d),
                               *getKnownValue(tok->astParent()->astOperand2(), ValueFlow::Value::ValueType::INT))) {
                const std::string info("Compound assignment '" + tok->astParent()->str() + "', assigned value is " +
                                       value->infoString());
                if (tok->astParent()->str() == "=")
                    value->errorPath.clear();
                value->errorPath.emplace_back(tok, info);
            } else {
                // TODO: Don't set to zero
                value->intvalue = 0;
            }
        } else if (tok->astParent()->tokType() == Token::eIncDecOp) {
            bool inc = tok->astParent()->str() == "++";
            std::string opName(inc ? "incremented" : "decremented");
            if (d == Direction::Reverse)
                inc = !inc;
            value->intvalue += (inc ? 1 : -1);
            const std::string info(tok->str() + " is " + opName + "', new value is " + value->infoString());
            value->errorPath.emplace_back(tok, info);
        }
    }

    virtual Action analyze(const Token* tok, Direction d) const OVERRIDE {
        if (invalid())
            return Action::Invalid;
        bool inconclusive = false;
        if (match(tok)) {
            const Token* parent = tok->astParent();
            if (astIsPointer(tok) && (Token::Match(parent, "*|[") || (parent && parent->originalName() == "->")) && getIndirect(tok) <= 0)
                return Action::Read | Action::Match;

            // Action read = Action::Read;
            Action w = isWritable(tok, d);
            if (w != Action::None)
                return w | Action::Match;

            // Check for modifications by function calls
            return isModified(tok) | Action::Match;
        } else if (tok->isUnaryOp("*")) {
            const Token* lifeTok = nullptr;
            for (const ValueFlow::Value& v:tok->astOperand1()->values()) {
                if (!v.isLocalLifetimeValue())
                    continue;
                if (lifeTok)
                    return Action::None;
                lifeTok = v.tokvalue;
            }
            if (lifeTok && match(lifeTok)) {
                Action a = Action::Read;
                if (isModified(tok).isModified())
                    a = Action::Invalid;
                if (Token::Match(tok->astParent(), "%assign%") && astIsLHS(tok))
                    a |= Action::Read;
                return a;
            }
            return Action::None;

        } else if (isAlias(tok, inconclusive)) {
            Action a = isAliasModified(tok);
            if (inconclusive && a.isModified())
                return Action::Inconclusive;
            else
                return a;
        } else if (Token::Match(tok, "%name% (") && !Token::simpleMatch(tok->linkAt(1), ") {")) {
            // bailout: global non-const variables
            if (isGlobal()) {
                return Action::Invalid;
            }
        }
        return Action::None;
    }

    virtual std::vector<int> evaluate(const Token* tok) const OVERRIDE {
        if (tok->hasKnownIntValue())
            return {static_cast<int>(tok->values().front().intvalue)};
        std::vector<int> result;
        ProgramMemory pm = pms.get(tok, getProgramState());
        if (Token::Match(tok, "&&|%oror%")) {
            if (conditionIsTrue(tok, pm))
                result.push_back(1);
            if (conditionIsFalse(tok, pm))
                result.push_back(0);
        } else {
            MathLib::bigint out = 0;
            bool error = false;
            execute(tok, &pm, &out, &error);
            if (!error)
                result.push_back(out);
        }

        return result;
    }

    virtual void assume(const Token* tok, bool state, const Token* at) OVERRIDE {
        // Update program state
        pms.removeModifiedVars(tok);
        pms.addState(tok, getProgramState());
        pms.assume(tok, state);

        const bool isAssert = Token::Match(at, "assert|ASSERT");

        if (!isAssert && !Token::simpleMatch(at, "}")) {
            std::string s = state ? "true" : "false";
            addErrorPath(tok, "Assuming condition is " + s);
        }
        if (!isAssert)
            makeConditional();
    }

    virtual void update(Token* tok, Action a, Direction d) OVERRIDE {
        ValueFlow::Value* value = getValue(tok);
        if (!value)
            return;
        // Read first when moving forward
        if (d == Direction::Forward && a.isRead())
            setTokenValue(tok, *value, getSettings());
        if (a.isInconclusive())
            lowerToInconclusive();
        if (a.isWrite() && tok->astParent()) {
            writeValue(value, tok, d);
        }
        // Read last when moving in reverse
        if (d == Direction::Reverse && a.isRead())
            setTokenValue(tok, *value, getSettings());
    }

    virtual ValuePtr<Analyzer> reanalyze(Token*, const std::string&) const OVERRIDE {
        return {};
    }
};

ValuePtr<Analyzer> makeAnalyzer(Token* exprTok, const ValueFlow::Value& value, const TokenList* tokenlist);

struct SingleValueFlowAnalyzer : ValueFlowAnalyzer {
    std::unordered_map<nonneg int, const Variable*> varids;
    std::unordered_map<nonneg int, const Variable*> aliases;
    ValueFlow::Value value;

    SingleValueFlowAnalyzer() : ValueFlowAnalyzer() {}

    SingleValueFlowAnalyzer(const ValueFlow::Value& v, const TokenList* t) : ValueFlowAnalyzer(t), value(v) {}

    const std::unordered_map<nonneg int, const Variable*>& getVars() const {
        return varids;
    }

    const std::unordered_map<nonneg int, const Variable*>& getAliasedVars() const {
        return aliases;
    }

    virtual const ValueFlow::Value* getValue(const Token*) const OVERRIDE {
        return &value;
    }
    virtual ValueFlow::Value* getValue(const Token*) OVERRIDE {
        return &value;
    }

    virtual void makeConditional() OVERRIDE {
        value.conditional = true;
    }

    virtual void addErrorPath(const Token* tok, const std::string& s) OVERRIDE {
        value.errorPath.emplace_back(tok, s);
    }

    virtual bool isAlias(const Token* tok, bool& inconclusive) const OVERRIDE {
        if (value.isLifetimeValue())
            return false;
        for (const auto& m: {
        std::ref(getVars()), std::ref(getAliasedVars())
        }) {
            for (const auto& p:m.get()) {
                nonneg int varid = p.first;
                const Variable* var = p.second;
                if (tok->varId() == varid)
                    return true;
                if (isAliasOf(var, tok, varid, {value}, &inconclusive))
                    return true;
            }
        }
        return false;
    }

    virtual bool isGlobal() const OVERRIDE {
        for (const auto&p:getVars()) {
            const Variable* var = p.second;
            if (!var->isLocal() && !var->isArgument() && !var->isConst())
                return true;
        }
        return false;
    }

    virtual bool lowerToPossible() OVERRIDE {
        if (value.isImpossible())
            return false;
        value.changeKnownToPossible();
        return true;
    }
    virtual bool lowerToInconclusive() OVERRIDE {
        if (value.isImpossible())
            return false;
        value.setInconclusive();
        return true;
    }

    virtual bool isConditional() const OVERRIDE {
        if (value.conditional)
            return true;
        if (value.condition)
            return !value.isImpossible();
        return false;
    }

    virtual bool updateScope(const Token* endBlock, bool) const OVERRIDE {
        const Scope* scope = endBlock->scope();
        if (!scope)
            return false;
        if (scope->type == Scope::eLambda) {
            return value.isLifetimeValue();
        } else if (scope->type == Scope::eIf || scope->type == Scope::eElse || scope->type == Scope::eWhile ||
                   scope->type == Scope::eFor) {
            if (value.isKnown() || value.isImpossible())
                return true;
            if (value.isLifetimeValue())
                return true;
            if (isConditional())
                return false;
            const Token* condTok = getCondTokFromEnd(endBlock);
            std::set<nonneg int> varids2;
            std::transform(getVars().begin(), getVars().end(), std::inserter(varids2, varids2.begin()), SelectMapKeys{});
            return bifurcate(condTok, varids2, getSettings());
        }

        return false;
    }

    virtual ValuePtr<Analyzer> reanalyze(Token* tok, const std::string& msg) const OVERRIDE {
        ValueFlow::Value newValue = value;
        newValue.errorPath.emplace_back(tok, msg);
        return makeAnalyzer(tok, newValue, tokenlist);
    }
};

struct VariableAnalyzer : SingleValueFlowAnalyzer {
    const Variable* var;

    VariableAnalyzer() : SingleValueFlowAnalyzer(), var(nullptr) {}

    VariableAnalyzer(const Variable* v,
                     const ValueFlow::Value& val,
                     std::vector<const Variable*> paliases,
                     const TokenList* t)
        : SingleValueFlowAnalyzer(val, t), var(v) {
        varids[var->declarationId()] = var;
        for (const Variable* av:paliases) {
            if (!av)
                continue;
            aliases[av->declarationId()] = av;
        }
    }

    virtual const ValueType* getValueType(const Token*) const OVERRIDE {
        return var->valueType();
    }

    virtual bool match(const Token* tok) const OVERRIDE {
        return tok->varId() == var->declarationId();
    }

    virtual ProgramState getProgramState() const OVERRIDE {
        ProgramState ps;
        ps[var->declarationId()] = value;
        return ps;
    }
};

static std::vector<const Variable*> getAliasesFromValues(std::list<ValueFlow::Value> values, bool address=false)
{
    std::vector<const Variable*> aliases;
    for (const ValueFlow::Value& v : values) {
        if (!v.tokvalue)
            continue;
        const Token* lifeTok = nullptr;
        for (const ValueFlow::Value& lv:v.tokvalue->values()) {
            if (!lv.isLocalLifetimeValue())
                continue;
            if (address && lv.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
                continue;
            if (lifeTok) {
                lifeTok = nullptr;
                break;
            }
            lifeTok = lv.tokvalue;
        }
        if (lifeTok && lifeTok->variable()) {
            aliases.push_back(lifeTok->variable());
        }
    }
    return aliases;
}

static Analyzer::Action valueFlowForwardVariable(Token* const startToken,
        const Token* const endToken,
        const Variable* const var,
        std::list<ValueFlow::Value> values,
        std::vector<const Variable*> aliases,
        TokenList* const tokenlist,
        const Settings* const settings)
{
    Analyzer::Action actions;
    for (ValueFlow::Value& v : values) {
        VariableAnalyzer a(var, v, aliases, tokenlist);
        actions |= valueFlowGenericForward(startToken, endToken, a, settings);
    }
    return actions;
}

static Analyzer::Action valueFlowForwardVariable(Token* const startToken,
        const Token* const endToken,
        const Variable* const var,
        std::list<ValueFlow::Value> values,
        TokenList* const tokenlist,
        const Settings* const settings)
{
    return valueFlowForwardVariable(
               startToken, endToken, var, std::move(values), getAliasesFromValues(values), tokenlist, settings);
}

// Old deprecated version
static bool valueFlowForwardVariable(Token* const startToken,
                                     const Token* const endToken,
                                     const Variable* const var,
                                     const nonneg int,
                                     std::list<ValueFlow::Value> values,
                                     const bool,
                                     const bool,
                                     TokenList* const tokenlist,
                                     ErrorLogger* const,
                                     const Settings* const settings)
{
    valueFlowForwardVariable(startToken, endToken, var, std::move(values), tokenlist, settings);
    return true;
}

struct ExpressionAnalyzer : SingleValueFlowAnalyzer {
    const Token* expr;
    bool local;
    bool unknown;

    ExpressionAnalyzer() : SingleValueFlowAnalyzer(), expr(nullptr), local(true), unknown(false) {}

    ExpressionAnalyzer(const Token* e, const ValueFlow::Value& val, const TokenList* t)
        : SingleValueFlowAnalyzer(val, t), expr(e), local(true), unknown(false) {

        setupExprVarIds();
    }

    virtual const ValueType* getValueType(const Token*) const OVERRIDE {
        return expr->valueType();
    }

    static bool nonLocal(const Variable* var, bool deref) {
        return !var || (!var->isLocal() && !var->isArgument()) || (deref && var->isArgument() && var->isPointer()) || var->isStatic() || var->isReference() || var->isExtern();
    }

    void setupExprVarIds() {
        visitAstNodes(expr,
        [&](const Token *tok) {
            if (tok->varId() == 0 && tok->isName() && tok->previous()->str() != ".") {
                // unknown variable
                unknown = true;
                return ChildrenToVisit::none;
            }
            if (tok->varId() > 0) {
                varids[tok->varId()] = tok->variable();
                if (!Token::simpleMatch(tok->previous(), ".")) {
                    const Variable *var = tok->variable();
                    if (var && var->isReference() && var->isLocal() && Token::Match(var->nameToken(), "%var% [=(]") && !isGlobalData(var->nameToken()->next()->astOperand2(), isCPP()))
                        return ChildrenToVisit::none;
                    const bool deref = tok->astParent() && (tok->astParent()->isUnaryOp("*") || (tok->astParent()->str() == "[" && tok == tok->astParent()->astOperand1()));
                    local &= !nonLocal(tok->variable(), deref);
                }
            }
            return ChildrenToVisit::op1_and_op2;
        });
    }

    virtual bool invalid() const OVERRIDE {
        return unknown;
    }

    virtual std::vector<int> evaluate(const Token* tok) const OVERRIDE {
        if (tok->hasKnownIntValue())
            return {static_cast<int>(tok->values().front().intvalue)};
        return std::vector<int> {};
    }

    virtual bool match(const Token* tok) const OVERRIDE {
        return isSameExpression(isCPP(), true, expr, tok, getSettings()->library, true, true);
    }

    virtual ProgramState getProgramState() const OVERRIDE {
        return ProgramState{};
    }

    virtual bool isGlobal() const OVERRIDE {
        return !local;
    }
};

static Analyzer::Action valueFlowForwardExpression(Token* startToken,
        const Token* endToken,
        const Token* exprTok,
        const std::list<ValueFlow::Value>& values,
        const TokenList* const tokenlist,
        const Settings* settings)
{
    Analyzer::Action actions;
    for (const ValueFlow::Value& v : values) {
        ExpressionAnalyzer a(exprTok, v, tokenlist);
        actions |= valueFlowGenericForward(startToken, endToken, a, settings);
    }
    return actions;
}

static const Token* parseBinaryIntOp(const Token* expr, MathLib::bigint& known)
{
    if (!expr)
        return nullptr;
    if (!expr->astOperand1() || !expr->astOperand2())
        return nullptr;
    const Token* knownTok = nullptr;
    const Token* varTok = nullptr;
    if (expr->astOperand1()->hasKnownIntValue() && !expr->astOperand2()->hasKnownIntValue()) {
        varTok = expr->astOperand2();
        knownTok = expr->astOperand1();
    } else if (expr->astOperand2()->hasKnownIntValue() && !expr->astOperand1()->hasKnownIntValue()) {
        varTok = expr->astOperand1();
        knownTok = expr->astOperand2();
    }
    if (knownTok)
        known = knownTok->values().front().intvalue;
    return varTok;
}

template <class F>
void transformIntValues(std::list<ValueFlow::Value>& values, F f)
{
    std::transform(values.begin(), values.end(), values.begin(), [&](ValueFlow::Value x) {
        if (x.isIntValue() || x.isIteratorValue())
            x.intvalue = f(x.intvalue);
        return x;
    });
}

static const Token* solveExprValues(const Token* expr, std::list<ValueFlow::Value>& values)
{
    MathLib::bigint intval;
    const Token* binaryTok = parseBinaryIntOp(expr, intval);
    if (binaryTok && expr->str().size() == 1) {
        switch (expr->str()[0]) {
        case '+': {
            transformIntValues(values, [&](MathLib::bigint x) {
                return x - intval;
            });
            return solveExprValues(binaryTok, values);
        }
        case '*': {
            if (intval == 0)
                break;
            transformIntValues(values, [&](MathLib::bigint x) {
                return x / intval;
            });
            return solveExprValues(binaryTok, values);
        }
        case '^': {
            transformIntValues(values, [&](MathLib::bigint x) {
                return x ^ intval;
            });
            return solveExprValues(binaryTok, values);
        }
        }
    }
    return expr;
}

ValuePtr<Analyzer> makeAnalyzer(Token* exprTok, const ValueFlow::Value& value, const TokenList* tokenlist)
{
    std::list<ValueFlow::Value> values = {value};
    const Token* expr = solveExprValues(exprTok, values);
    if (expr->variable()) {
        return VariableAnalyzer(expr->variable(), value, getAliasesFromValues(values), tokenlist);
    } else {
        return ExpressionAnalyzer(expr, value, tokenlist);
    }
}

static Analyzer::Action valueFlowForward(Token* startToken,
        const Token* endToken,
        const Token* exprTok,
        std::list<ValueFlow::Value> values,
        TokenList* const tokenlist,
        const Settings* settings)
{
    const Token* expr = solveExprValues(exprTok, values);
    if (expr->variable()) {
        return valueFlowForwardVariable(startToken, endToken, expr->variable(), values, tokenlist, settings);
    } else {
        return valueFlowForwardExpression(startToken, endToken, expr, values, tokenlist, settings);
    }
}

static void valueFlowReverse(TokenList* tokenlist,
                             Token* tok,
                             const Token* const varToken,
                             ValueFlow::Value val,
                             ValueFlow::Value val2,
                             ErrorLogger* /*errorLogger*/,
                             const Settings* settings)
{
    std::list<ValueFlow::Value> values = {val};
    if (val2.varId != 0)
        values.push_back(val2);
    const Variable* var = varToken->variable();
    auto aliases = getAliasesFromValues(values);
    for (ValueFlow::Value& v : values) {
        VariableAnalyzer a(var, v, aliases, tokenlist);
        valueFlowGenericReverse(tok, a, settings);
    }
}

static int getArgumentPos(const Variable *var, const Function *f)
{
    auto arg_it = std::find_if(f->argumentList.begin(), f->argumentList.end(), [&](const Variable &v) {
        return v.nameToken() == var->nameToken();
    });
    if (arg_it == f->argumentList.end())
        return -1;
    return std::distance(f->argumentList.begin(), arg_it);
}

std::string lifetimeType(const Token *tok, const ValueFlow::Value *val)
{
    std::string result;
    if (!val)
        return "object";
    switch (val->lifetimeKind) {
    case ValueFlow::Value::LifetimeKind::Lambda:
        result = "lambda";
        break;
    case ValueFlow::Value::LifetimeKind::Iterator:
        result = "iterator";
        break;
    case ValueFlow::Value::LifetimeKind::Object:
    case ValueFlow::Value::LifetimeKind::SubObject:
    case ValueFlow::Value::LifetimeKind::Address:
        if (astIsPointer(tok))
            result = "pointer";
        else
            result = "object";
        break;
    }
    return result;
}

std::string lifetimeMessage(const Token *tok, const ValueFlow::Value *val, ErrorPath &errorPath)
{
    const Token *tokvalue = val ? val->tokvalue : nullptr;
    const Variable *tokvar = tokvalue ? tokvalue->variable() : nullptr;
    const Token *vartok = tokvar ? tokvar->nameToken() : nullptr;
    std::string type = lifetimeType(tok, val);
    std::string msg = type;
    if (vartok) {
        errorPath.emplace_back(vartok, "Variable created here.");
        const Variable * var = vartok->variable();
        if (var) {
            switch (val->lifetimeKind) {
            case ValueFlow::Value::LifetimeKind::SubObject:
            case ValueFlow::Value::LifetimeKind::Object:
            case ValueFlow::Value::LifetimeKind::Address:
                if (type == "pointer")
                    msg += " to local variable";
                else
                    msg += " that points to local variable";
                break;
            case ValueFlow::Value::LifetimeKind::Lambda:
                msg += " that captures local variable";
                break;
            case ValueFlow::Value::LifetimeKind::Iterator:
                msg += " to local container";
                break;
            }
            msg += " '" + var->name() + "'";
        }
    }
    return msg;
}

ValueFlow::Value getLifetimeObjValue(const Token *tok, bool inconclusive)
{
    ValueFlow::Value result;
    auto pred = [&](const ValueFlow::Value &v) {
        if (!v.isLocalLifetimeValue())
            return false;
        if (!inconclusive && v.isInconclusive())
            return false;
        if (!v.tokvalue->variable())
            return false;
        return true;
    };
    auto it = std::find_if(tok->values().begin(), tok->values().end(), pred);
    if (it == tok->values().end())
        return result;
    result = *it;
    // There should only be one lifetime
    if (std::find_if(std::next(it), tok->values().end(), pred) != tok->values().end())
        return ValueFlow::Value{};
    return result;
}

std::vector<LifetimeToken> getLifetimeTokens(const Token* tok, bool escape, ValueFlow::Value::ErrorPath errorPath, int depth)
{
    if (!tok)
        return std::vector<LifetimeToken> {};
    const Variable *var = tok->variable();
    if (depth < 0)
        return {{tok, std::move(errorPath)}};
    if (var && var->declarationId() == tok->varId()) {
        if (var->isReference() || var->isRValueReference()) {
            if (!var->declEndToken())
                return {{tok, true, std::move(errorPath)}};
            if (var->isArgument()) {
                errorPath.emplace_back(var->declEndToken(), "Passed to reference.");
                return {{tok, true, std::move(errorPath)}};
            } else if (Token::simpleMatch(var->declEndToken(), "=")) {
                errorPath.emplace_back(var->declEndToken(), "Assigned to reference.");
                const Token *vartok = var->declEndToken()->astOperand2();
                const bool temporaryDefault = false; //If we can't tell then assume the value is not temporary as this will result in fewer false positives.
                const bool temporary = isTemporary(true, vartok, nullptr, temporaryDefault);
                const bool nonlocal = var->isStatic() || var->isGlobal();
                if (vartok == tok || (nonlocal && temporary) || (!escape && (var->isConst() || var->isRValueReference()) && temporary))
                    return {{tok, true, std::move(errorPath)}};
                if (vartok)
                    return getLifetimeTokens(vartok, escape, std::move(errorPath), depth - 1);
            } else if (Token::simpleMatch(var->nameToken()->astParent(), ":") &&
                       var->nameToken()->astParent()->astParent() &&
                       Token::simpleMatch(var->nameToken()->astParent()->astParent()->previous(), "for (")) {
                errorPath.emplace_back(var->nameToken(), "Assigned to reference.");
                const Token* vartok = var->nameToken();
                if (vartok == tok)
                    return {{tok, true, std::move(errorPath)}};
                const Token* contok = var->nameToken()->astParent()->astOperand2();
                if (contok)
                    return getLifetimeTokens(contok, escape, std::move(errorPath), depth - 1);
            } else {
                return std::vector<LifetimeToken> {};
            }
        }
    } else if (Token::Match(tok->previous(), "%name% (")) {
        const Function *f = tok->previous()->function();
        if (f) {
            if (!Function::returnsReference(f))
                return {{tok, std::move(errorPath)}};
            std::vector<LifetimeToken> result;
            std::vector<const Token*> returns = Function::findReturns(f);
            for (const Token* returnTok : returns) {
                if (returnTok == tok)
                    continue;
                for (LifetimeToken& lt : getLifetimeTokens(returnTok, escape, std::move(errorPath), depth - 1)) {
                    const Token* argvarTok = lt.token;
                    const Variable* argvar = argvarTok->variable();
                    if (!argvar)
                        continue;
                    if (argvar->isArgument() && (argvar->isReference() || argvar->isRValueReference())) {
                        int n = getArgumentPos(argvar, f);
                        if (n < 0)
                            return std::vector<LifetimeToken> {};
                        std::vector<const Token*> args = getArguments(tok->previous());
                        // TODO: Track lifetimes of default parameters
                        if (n >= args.size())
                            return std::vector<LifetimeToken> {};
                        const Token* argTok = args[n];
                        lt.errorPath.emplace_back(returnTok, "Return reference.");
                        lt.errorPath.emplace_back(tok->previous(), "Called function passing '" + argTok->expressionString() + "'.");
                        std::vector<LifetimeToken> arglts = LifetimeToken::setInconclusive(
                                                                getLifetimeTokens(argTok, escape, std::move(lt.errorPath), depth - 1), returns.size() > 1);
                        result.insert(result.end(), arglts.begin(), arglts.end());
                    }
                }
            }
            return result;
        } else if (Token::Match(tok->tokAt(-2), ". %name% (") && tok->tokAt(-2)->originalName() != "->" && astIsContainer(tok->tokAt(-2)->astOperand1())) {
            const Library::Container* library = getLibraryContainer(tok->tokAt(-2)->astOperand1());
            Library::Container::Yield y = library->getYield(tok->previous()->str());
            if (y == Library::Container::Yield::AT_INDEX || y == Library::Container::Yield::ITEM) {
                errorPath.emplace_back(tok->previous(), "Accessing container.");
                return LifetimeToken::setAddressOf(
                           getLifetimeTokens(tok->tokAt(-2)->astOperand1(), escape, std::move(errorPath), depth - 1), false);
            }
        }
    } else if (Token::Match(tok, ".|::|[")) {
        const Token *vartok = tok;
        while (vartok) {
            if (vartok->str() == "[" || vartok->originalName() == "->")
                vartok = vartok->astOperand1();
            else if (vartok->str() == "." || vartok->str() == "::")
                vartok = vartok->astOperand2();
            else
                break;
        }

        if (!vartok)
            return {{tok, std::move(errorPath)}};
        const Variable *tokvar = vartok->variable();
        if (!astIsContainer(vartok) && !(tokvar && tokvar->isArray() && !tokvar->isArgument()) &&
            (Token::Match(vartok->astParent(), "[|*") || vartok->astParent()->originalName() == "->")) {
            for (const ValueFlow::Value &v : vartok->values()) {
                if (!v.isLocalLifetimeValue())
                    continue;
                if (v.tokvalue == tok)
                    continue;
                errorPath.insert(errorPath.end(), v.errorPath.begin(), v.errorPath.end());
                return getLifetimeTokens(v.tokvalue, escape, std::move(errorPath), depth - 1);
            }
        } else {
            return LifetimeToken::setAddressOf(getLifetimeTokens(vartok, escape, std::move(errorPath), depth - 1),
                                               !(astIsContainer(vartok) && Token::simpleMatch(vartok->astParent(), "[")));
        }
    }
    return {{tok, std::move(errorPath)}};
}

static const Token* getLifetimeToken(const Token* tok, ValueFlow::Value::ErrorPath& errorPath, bool* addressOf = nullptr)
{
    std::vector<LifetimeToken> lts = getLifetimeTokens(tok);
    if (lts.size() != 1)
        return nullptr;
    if (lts.front().inconclusive)
        return nullptr;
    if (addressOf)
        *addressOf = lts.front().addressOf;
    errorPath.insert(errorPath.end(), lts.front().errorPath.begin(), lts.front().errorPath.end());
    return lts.front().token;
}

const Variable* getLifetimeVariable(const Token* tok, ValueFlow::Value::ErrorPath& errorPath, bool* addressOf)
{
    const Token* tok2 = getLifetimeToken(tok, errorPath, addressOf);
    if (tok2 && tok2->variable())
        return tok2->variable();
    return nullptr;
}

const Variable* getLifetimeVariable(const Token* tok)
{
    ValueFlow::Value::ErrorPath errorPath;
    return getLifetimeVariable(tok, errorPath, nullptr);
}

static bool isNotLifetimeValue(const ValueFlow::Value& val)
{
    return !val.isLifetimeValue();
}

static bool isLifetimeOwned(const ValueType *vt, const ValueType *vtParent)
{
    if (!vtParent)
        return false;
    if (!vt) {
        if (vtParent->type == ValueType::CONTAINER)
            return true;
        return false;
    }
    if (vt->type != ValueType::UNKNOWN_TYPE && vtParent->type != ValueType::UNKNOWN_TYPE) {
        if (vt->pointer != vtParent->pointer)
            return true;
        if (vt->type != vtParent->type) {
            if (vtParent->type == ValueType::RECORD)
                return true;
            if (vtParent->type == ValueType::CONTAINER)
                return true;
        }
    }

    return false;
}

static bool isLifetimeBorrowed(const ValueType *vt, const ValueType *vtParent)
{
    if (!vtParent)
        return false;
    if (!vt)
        return false;
    if (vt->type != ValueType::UNKNOWN_TYPE && vtParent->type != ValueType::UNKNOWN_TYPE && vtParent->container == vt->container) {
        if (vtParent->pointer > vt->pointer)
            return true;
        if (vtParent->pointer < vt->pointer && vtParent->isIntegral())
            return true;
        if (vtParent->str() == vt->str())
            return true;
        if (vtParent->pointer == vt->pointer && vtParent->type == vt->type && vtParent->isIntegral())
            // sign conversion
            return true;
    }

    return false;
}

static const Token* skipCVRefs(const Token* tok, const Token* endTok)
{
    while (tok != endTok && Token::Match(tok, "const|volatile|auto|&|&&"))
        tok = tok->next();
    return tok;
}

static bool isNotEqual(std::pair<const Token*, const Token*> x, std::pair<const Token*, const Token*> y)
{
    const Token* start1 = x.first;
    const Token* start2 = y.first;
    if (start1 == nullptr || start2 == nullptr)
        return false;
    while (start1 != x.second && start2 != y.second) {
        const Token* tok1 = skipCVRefs(start1, x.second);
        if (tok1 != start1) {
            start1 = tok1;
            continue;
        }
        const Token* tok2 = skipCVRefs(start2, y.second);
        if (tok2 != start2) {
            start2 = tok2;
            continue;
        }
        if (start1->str() != start2->str())
            return true;
        start1 = start1->next();
        start2 = start2->next();
    }
    start1 = skipCVRefs(start1, x.second);
    start2 = skipCVRefs(start2, y.second);
    return !(start1 == x.second && start2 == y.second);
}
static bool isNotEqual(std::pair<const Token*, const Token*> x, const std::string& y)
{
    TokenList tokenList(nullptr);
    std::istringstream istr(y);
    tokenList.createTokens(istr);
    return isNotEqual(x, std::make_pair(tokenList.front(), tokenList.back()));
}
static bool isNotEqual(std::pair<const Token*, const Token*> x, const ValueType* y)
{
    if (y == nullptr)
        return false;
    if (y->originalTypeName.empty())
        return false;
    return isNotEqual(x, y->originalTypeName);
}

bool isLifetimeBorrowed(const Token *tok, const Settings *settings)
{
    if (!tok)
        return true;
    if (tok->str() == ",")
        return true;
    if (!tok->astParent())
        return true;
    if (!Token::Match(tok->astParent()->previous(), "%name% (") && !Token::simpleMatch(tok->astParent(), ",")) {
        if (!Token::simpleMatch(tok, "{")) {
            const ValueType *vt = tok->valueType();
            const ValueType *vtParent = tok->astParent()->valueType();
            if (isLifetimeBorrowed(vt, vtParent))
                return true;
            if (isLifetimeOwned(vt, vtParent))
                return false;
        }
        if (Token::Match(tok->astParent(), "return|(|{|%assign%")) {
            const Type *t = Token::typeOf(tok);
            const Type *parentT = Token::typeOf(tok->astParent());
            if (t && parentT) {
                if (t->classDef && parentT->classDef && t->classDef != parentT->classDef)
                    return false;
            } else {
                std::pair<const Token*, const Token*> decl = Token::typeDecl(tok);
                std::pair<const Token*, const Token*> parentdecl = Token::typeDecl(tok->astParent());
                if (isNotEqual(decl, parentdecl))
                    return false;
                if (isNotEqual(decl, tok->astParent()->valueType()))
                    return false;
                if (isNotEqual(parentdecl, tok->valueType()))
                    return false;
            }
        }
    } else if (Token::Match(tok->astParent()->tokAt(-3), "%var% . push_back|push_front|insert|push (") &&
               astIsContainer(tok->astParent()->tokAt(-3))) {
        const ValueType *vt = tok->valueType();
        const ValueType *vtCont = tok->astParent()->tokAt(-3)->valueType();
        if (!vtCont->containerTypeToken)
            return true;
        ValueType vtParent = ValueType::parseDecl(vtCont->containerTypeToken, settings);
        if (isLifetimeBorrowed(vt, &vtParent))
            return true;
        if (isLifetimeOwned(vt, &vtParent))
            return false;
    }

    return true;
}

static void valueFlowLifetimeFunction(Token *tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings);

static void valueFlowLifetimeConstructor(Token *tok,
        TokenList *tokenlist,
        ErrorLogger *errorLogger,
        const Settings *settings);

static const Token* getEndOfVarScope(const Token* tok, const std::vector<const Variable*>& vars)
{
    const Token* endOfVarScope = nullptr;
    for (const Variable* var : vars) {
        if (var && var->isLocal())
            endOfVarScope = var->typeStartToken()->scope()->bodyEnd;
        else if (!endOfVarScope)
            endOfVarScope = tok->scope()->bodyEnd;
    }
    return endOfVarScope;
}

static void valueFlowForwardLifetime(Token * tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    // Forward lifetimes to constructed variable
    if (Token::Match(tok->previous(), "%var% {")) {
        std::list<ValueFlow::Value> values = tok->values();
        values.remove_if(&isNotLifetimeValue);
        valueFlowForward(nextAfterAstRightmostLeaf(tok),
                         getEndOfVarScope(tok, {tok->variable()}),
                         tok->previous(),
                         values,
                         tokenlist,
                         settings);
        return;
    }
    Token *parent = tok->astParent();
    while (parent && (parent->isArithmeticalOp() || parent->str() == ","))
        parent = parent->astParent();
    if (!parent)
        return;
    // Assignment
    if (parent->str() == "=" && (!parent->astParent() || Token::simpleMatch(parent->astParent(), ";"))) {
        // Rhs values..
        if (!parent->astOperand2() || parent->astOperand2()->values().empty())
            return;

        if (!isLifetimeBorrowed(parent->astOperand2(), settings))
            return;

        std::vector<const Variable*> vars = getLHSVariables(parent);

        const Token* endOfVarScope = getEndOfVarScope(tok, vars);

        // Only forward lifetime values
        std::list<ValueFlow::Value> values = parent->astOperand2()->values();
        values.remove_if(&isNotLifetimeValue);

        // Skip RHS
        const Token *nextExpression = nextAfterAstRightmostLeaf(parent);

        if (Token::Match(parent->astOperand1(), ".|[|(")) {
            valueFlowForwardExpression(
                const_cast<Token*>(nextExpression), endOfVarScope, parent->astOperand1(), values, tokenlist, settings);

            for (ValueFlow::Value& val : values) {
                if (val.lifetimeKind == ValueFlow::Value::LifetimeKind::Address)
                    val.lifetimeKind = ValueFlow::Value::LifetimeKind::SubObject;
            }
        }
        for (const Variable* var : vars) {
            valueFlowForwardVariable(const_cast<Token*>(nextExpression),
                                     endOfVarScope,
                                     var,
                                     var->declarationId(),
                                     values,
                                     false,
                                     false,
                                     tokenlist,
                                     errorLogger,
                                     settings);

            if (tok->astTop() && Token::simpleMatch(tok->astTop()->previous(), "for (") &&
                Token::simpleMatch(tok->astTop()->link(), ") {")) {
                Token* start = tok->astTop()->link()->next();
                valueFlowForwardVariable(
                    start, start->link(), var, var->declarationId(), values, false, false, tokenlist, errorLogger, settings);
            }
        }
        // Constructor
    } else if (Token::simpleMatch(parent, "{") && !isScopeBracket(parent)) {
        valueFlowLifetimeConstructor(parent, tokenlist, errorLogger, settings);
        valueFlowForwardLifetime(parent, tokenlist, errorLogger, settings);
        // Function call
    } else if (Token::Match(parent->previous(), "%name% (")) {
        valueFlowLifetimeFunction(parent->previous(), tokenlist, errorLogger, settings);
        valueFlowForwardLifetime(parent, tokenlist, errorLogger, settings);
        // Variable
    } else if (tok->variable()) {
        const Variable *var = tok->variable();
        const Token *endOfVarScope = var->scope()->bodyEnd;

        std::list<ValueFlow::Value> values = tok->values();
        const Token *nextExpression = nextAfterAstRightmostLeaf(parent);
        // Only forward lifetime values
        values.remove_if(&isNotLifetimeValue);
        valueFlowForwardVariable(const_cast<Token*>(nextExpression),
                                 endOfVarScope,
                                 var,
                                 var->declarationId(),
                                 values,
                                 false,
                                 false,
                                 tokenlist,
                                 errorLogger,
                                 settings);
        // Cast
    } else if (parent->isCast()) {
        std::list<ValueFlow::Value> values = tok->values();
        // Only forward lifetime values
        values.remove_if(&isNotLifetimeValue);
        for (const ValueFlow::Value& value:values)
            setTokenValue(parent, value, tokenlist->getSettings());
        valueFlowForwardLifetime(parent, tokenlist, errorLogger, settings);
    }
}

struct LifetimeStore {
    const Token *argtok;
    std::string message;
    ValueFlow::Value::LifetimeKind type;
    ErrorPath errorPath;
    bool inconclusive;
    bool forward;

    struct Context {
        Token* tok;
        TokenList* tokenlist;
        ErrorLogger* errorLogger;
        const Settings* settings;
    };

    LifetimeStore()
        : argtok(nullptr), message(), type(), errorPath(), inconclusive(false), forward(true), mContext(nullptr)
    {}

    LifetimeStore(const Token* argtok,
                  const std::string& message,
                  ValueFlow::Value::LifetimeKind type = ValueFlow::Value::LifetimeKind::Object,
                  bool inconclusive = false)
        : argtok(argtok),
          message(message),
          type(type),
          errorPath(),
          inconclusive(inconclusive),
          forward(true),
          mContext(nullptr)
    {}

    template <class F>
    static void forEach(const std::vector<const Token*>& argtoks,
                        const std::string& message,
                        ValueFlow::Value::LifetimeKind type,
                        F f) {
        std::map<const Token*, Context> forwardToks;
        for (const Token* arg : argtoks) {
            LifetimeStore ls{arg, message, type};
            Context c{};
            ls.mContext = &c;
            ls.forward = false;
            f(ls);
            if (c.tok)
                forwardToks[c.tok] = c;
        }
        for (const auto& p : forwardToks) {
            const Context& c = p.second;
            valueFlowForwardLifetime(c.tok, c.tokenlist, c.errorLogger, c.settings);
        }
    }

    static LifetimeStore fromFunctionArg(const Function * f, Token *tok, const Variable *var, TokenList *tokenlist, ErrorLogger *errorLogger) {
        if (!var)
            return LifetimeStore{};
        if (!var->isArgument())
            return LifetimeStore{};
        int n = getArgumentPos(var, f);
        if (n < 0)
            return LifetimeStore{};
        std::vector<const Token *> args = getArguments(tok);
        if (n >= args.size()) {
            if (tokenlist->getSettings()->debugwarnings)
                bailout(tokenlist,
                        errorLogger,
                        tok,
                        "Argument mismatch: Function '" + tok->str() + "' returning lifetime from argument index " +
                        std::to_string(n) + " but only " + std::to_string(args.size()) +
                        " arguments are available.");
            return LifetimeStore{};
        }
        const Token *argtok2 = args[n];
        return LifetimeStore{argtok2, "Passed to '" + tok->expressionString() + "'.", ValueFlow::Value::LifetimeKind::Object};
    }

    template <class Predicate>
    bool byRef(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings, Predicate pred) const {
        if (!argtok)
            return false;
        bool update = false;
        for (const LifetimeToken& lt : getLifetimeTokens(argtok)) {
            if (!settings->inconclusive && lt.inconclusive)
                continue;
            ErrorPath er = errorPath;
            er.insert(er.end(), lt.errorPath.begin(), lt.errorPath.end());
            if (!lt.token)
                return false;
            if (!pred(lt.token))
                return false;
            er.emplace_back(argtok, message);

            ValueFlow::Value value;
            value.valueType = ValueFlow::Value::LIFETIME;
            value.lifetimeScope = ValueFlow::Value::LifetimeScope::Local;
            value.tokvalue = lt.token;
            value.errorPath = std::move(er);
            value.lifetimeKind = type;
            value.setInconclusive(lt.inconclusive || inconclusive);
            // Don't add the value a second time
            if (std::find(tok->values().begin(), tok->values().end(), value) != tok->values().end())
                return false;
            setTokenValue(tok, value, tokenlist->getSettings());
            update = true;
        }
        if (update && forward)
            forwardLifetime(tok, tokenlist, errorLogger, settings);
        return update;
    }

    bool byRef(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings) const {
        return byRef(tok, tokenlist, errorLogger, settings, [](const Token*) {
            return true;
        });
    }

    template <class Predicate>
    bool byVal(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings, Predicate pred) const {
        if (!argtok)
            return false;
        bool update = false;
        if (argtok->values().empty()) {
            ErrorPath er;
            er.emplace_back(argtok, message);
            const Variable *var = getLifetimeVariable(argtok, er);
            if (var && var->isArgument()) {
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::LIFETIME;
                value.lifetimeScope = ValueFlow::Value::LifetimeScope::Argument;
                value.tokvalue = var->nameToken();
                value.errorPath = er;
                value.lifetimeKind = type;
                value.setInconclusive(inconclusive);
                // Don't add the value a second time
                if (std::find(tok->values().begin(), tok->values().end(), value) != tok->values().end())
                    return false;
                setTokenValue(tok, value, tokenlist->getSettings());
                update = true;
            }
        }
        for (const ValueFlow::Value &v : argtok->values()) {
            if (!v.isLifetimeValue())
                continue;
            const Token *tok3 = v.tokvalue;
            for (const LifetimeToken& lt : getLifetimeTokens(tok3)) {
                if (!settings->inconclusive && lt.inconclusive)
                    continue;
                ErrorPath er = v.errorPath;
                er.insert(er.end(), lt.errorPath.begin(), lt.errorPath.end());
                if (!lt.token)
                    return false;
                if (!pred(lt.token))
                    return false;
                er.emplace_back(argtok, message);
                er.insert(er.end(), errorPath.begin(), errorPath.end());

                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::LIFETIME;
                value.lifetimeScope = v.lifetimeScope;
                value.path = v.path;
                value.tokvalue = lt.token;
                value.errorPath = std::move(er);
                value.lifetimeKind = type;
                value.setInconclusive(lt.inconclusive || v.isInconclusive() || inconclusive);
                // Don't add the value a second time
                if (std::find(tok->values().begin(), tok->values().end(), value) != tok->values().end())
                    continue;
                setTokenValue(tok, value, tokenlist->getSettings());
                update = true;
            }
        }
        if (update && forward)
            forwardLifetime(tok, tokenlist, errorLogger, settings);
        return update;
    }

    bool byVal(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings) const {
        return byVal(tok, tokenlist, errorLogger, settings, [](const Token*) {
            return true;
        });
    }

    template <class Predicate>
    void byDerefCopy(Token *tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings, Predicate pred) const {
        if (!settings->inconclusive && inconclusive)
            return;
        if (!argtok)
            return;
        for (const ValueFlow::Value &v : argtok->values()) {
            if (!v.isLifetimeValue())
                continue;
            const Token *tok2 = v.tokvalue;
            ErrorPath er = v.errorPath;
            const Variable *var = getLifetimeVariable(tok2, er);
            er.insert(er.end(), errorPath.begin(), errorPath.end());
            if (!var)
                continue;
            for (const Token *tok3 = tok; tok3 && tok3 != var->declEndToken(); tok3 = tok3->previous()) {
                if (tok3->varId() == var->declarationId()) {
                    LifetimeStore{tok3, message, type, inconclusive} .byVal(tok, tokenlist, errorLogger, settings, pred);
                    break;
                }
            }
        }
    }

    void byDerefCopy(Token *tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings) const {
        byDerefCopy(tok, tokenlist, errorLogger, settings, [](const Token *) {
            return true;
        });
    }

private:
    Context* mContext;
    void forwardLifetime(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings) const {
        if (mContext) {
            mContext->tok = tok;
            mContext->tokenlist = tokenlist;
            mContext->errorLogger = errorLogger;
            mContext->settings = settings;
        }
        valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
    }
};

static void valueFlowLifetimeFunction(Token *tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    if (!Token::Match(tok, "%name% ("))
        return;
    int returnContainer = settings->library.returnValueContainer(tok);
    if (returnContainer >= 0) {
        std::vector<const Token *> args = getArguments(tok);
        for (int argnr = 1; argnr <= args.size(); ++argnr) {
            const Library::ArgumentChecks::IteratorInfo *i = settings->library.getArgIteratorInfo(tok, argnr);
            if (!i)
                continue;
            if (i->container != returnContainer)
                continue;
            const Token * const argTok = args[argnr - 1];
            // Check if lifetime is available to avoid adding the lifetime twice
            ValueFlow::Value val = getLifetimeObjValue(argTok);
            if (val.tokvalue) {
                LifetimeStore{argTok, "Passed to '" + tok->str() + "'.", ValueFlow::Value::LifetimeKind::Iterator} .byVal(
                    tok->next(), tokenlist, errorLogger, settings);
                break;
            }
        }
    } else if (Token::Match(tok->tokAt(-2), "std :: ref|cref|tie|front_inserter|back_inserter")) {
        for (const Token *argtok : getArguments(tok)) {
            LifetimeStore{argtok, "Passed to '" + tok->str() + "'.", ValueFlow::Value::LifetimeKind::Object} .byRef(
                tok->next(), tokenlist, errorLogger, settings);
        }
    } else if (Token::Match(tok->tokAt(-2), "std :: make_tuple|tuple_cat|make_pair|make_reverse_iterator|next|prev|move|bind")) {
        for (const Token *argtok : getArguments(tok)) {
            LifetimeStore{argtok, "Passed to '" + tok->str() + "'.", ValueFlow::Value::LifetimeKind::Object} .byVal(
                tok->next(), tokenlist, errorLogger, settings);
        }
    } else if (Token::Match(tok->tokAt(-2), "%var% . push_back|push_front|insert|push|assign") &&
               astIsContainer(tok->tokAt(-2))) {
        Token *vartok = tok->tokAt(-2);
        std::vector<const Token *> args = getArguments(tok);
        std::size_t n = args.size();
        if (n > 1 && Token::typeStr(args[n - 2]) == Token::typeStr(args[n - 1]) &&
            (((astIsIterator(args[n - 2]) && astIsIterator(args[n - 1])) ||
              (astIsPointer(args[n - 2]) && astIsPointer(args[n - 1]))))) {
            LifetimeStore{args.back(), "Added to container '" + vartok->str() + "'.", ValueFlow::Value::LifetimeKind::Object} .byDerefCopy(
                vartok, tokenlist, errorLogger, settings);
        } else if (!args.empty() && isLifetimeBorrowed(args.back(), settings)) {
            LifetimeStore{args.back(), "Added to container '" + vartok->str() + "'.", ValueFlow::Value::LifetimeKind::Object} .byVal(
                vartok, tokenlist, errorLogger, settings);
        }
    } else if (tok->function()) {
        const Function *f = tok->function();
        if (Function::returnsReference(f))
            return;
        std::vector<const Token*> returns = Function::findReturns(f);
        const bool inconclusive = returns.size() > 1;
        bool update = false;
        for (const Token* returnTok : returns) {
            if (returnTok == tok)
                continue;
            const Variable *returnVar = getLifetimeVariable(returnTok);
            if (returnVar && returnVar->isArgument() && (returnVar->isConst() || !isVariableChanged(returnVar, settings, tokenlist->isCPP()))) {
                LifetimeStore ls = LifetimeStore::fromFunctionArg(f, tok, returnVar, tokenlist, errorLogger);
                ls.inconclusive = inconclusive;
                ls.forward = false;
                update |= ls.byVal(tok->next(), tokenlist, errorLogger, settings);
            }
            for (const ValueFlow::Value &v : returnTok->values()) {
                if (!v.isLifetimeValue())
                    continue;
                if (!v.tokvalue)
                    continue;
                const Variable *var = v.tokvalue->variable();
                LifetimeStore ls = LifetimeStore::fromFunctionArg(f, tok, var, tokenlist, errorLogger);
                if (!ls.argtok)
                    continue;
                ls.forward = false;
                ls.inconclusive = inconclusive;
                ls.errorPath = v.errorPath;
                ls.errorPath.emplace_front(returnTok, "Return " + lifetimeType(returnTok, &v) + ".");
                if (!v.isArgumentLifetimeValue() && (var->isReference() || var->isRValueReference())) {
                    update |= ls.byRef(tok->next(), tokenlist, errorLogger, settings);
                } else if (v.isArgumentLifetimeValue()) {
                    update |= ls.byVal(tok->next(), tokenlist, errorLogger, settings);
                }
            }
        }
        if (update)
            valueFlowForwardLifetime(tok->next(), tokenlist, errorLogger, settings);
    }
}

static void valueFlowLifetimeConstructor(Token* tok,
        const Type* t,
        TokenList* tokenlist,
        ErrorLogger* errorLogger,
        const Settings* settings)
{
    if (!Token::Match(tok, "(|{"))
        return;
    if (!t) {
        if (tok->valueType() && tok->valueType()->type != ValueType::RECORD)
            return;
        // If the type is unknown then assume it captures by value in the
        // constructor, but make each lifetime inconclusive
        std::vector<const Token*> args = getArguments(tok);
        LifetimeStore::forEach(
        args, "Passed to initializer list.", ValueFlow::Value::LifetimeKind::Object, [&](LifetimeStore& ls) {
            ls.inconclusive = true;
            ls.byVal(tok, tokenlist, errorLogger, settings);
        });
        return;
    }
    const Scope* scope = t->classScope;
    if (!scope)
        return;
    // Only support aggregate constructors for now
    if (scope->numConstructors == 0 && t->derivedFrom.empty() && (t->isClassType() || t->isStructType())) {
        std::vector<const Token*> args = getArguments(tok);
        auto it = scope->varlist.begin();
        LifetimeStore::forEach(args,
                               "Passed to constructor of '" + t->name() + "'.",
                               ValueFlow::Value::LifetimeKind::Object,
        [&](const LifetimeStore& ls) {
            if (it == scope->varlist.end())
                return;
            const Variable& var = *it;
            if (var.isReference() || var.isRValueReference()) {
                ls.byRef(tok, tokenlist, errorLogger, settings);
            } else {
                ls.byVal(tok, tokenlist, errorLogger, settings);
            }
            it++;
        });
    }
}

static bool hasInitList(const Token* tok)
{
    if (astIsPointer(tok))
        return true;
    if (astIsContainer(tok)) {
        const Library::Container * library = getLibraryContainer(tok);
        if (!library)
            return false;
        return library->hasInitializerListConstructor;
    }
    return false;
}

static void valueFlowLifetimeConstructor(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings)
{
    if (!Token::Match(tok, "(|{"))
        return;
    Token* parent = tok->astParent();
    while (Token::simpleMatch(parent, ","))
        parent = parent->astParent();
    if (Token::simpleMatch(parent, "{") && hasInitList(parent->astParent())) {
        valueFlowLifetimeConstructor(tok, Token::typeOf(parent->previous()), tokenlist, errorLogger, settings);
    } else if (Token::simpleMatch(tok, "{") && hasInitList(parent)) {
        std::vector<const Token *> args = getArguments(tok);
        // Assume range constructor if passed a pair of iterators
        if (astIsContainer(parent) && args.size() == 2 && astIsIterator(args[0]) && astIsIterator(args[1])) {
            LifetimeStore::forEach(
            args, "Passed to initializer list.", ValueFlow::Value::LifetimeKind::Object, [&](const LifetimeStore& ls) {
                ls.byDerefCopy(tok, tokenlist, errorLogger, settings);
            });
        } else {
            LifetimeStore::forEach(args,
                                   "Passed to initializer list.",
                                   ValueFlow::Value::LifetimeKind::Object,
            [&](const LifetimeStore& ls) {
                ls.byVal(tok, tokenlist, errorLogger, settings);
            });
        }
    } else {
        valueFlowLifetimeConstructor(tok, Token::typeOf(tok->previous()), tokenlist, errorLogger, settings);
    }
}

struct Lambda {
    enum class Capture {
        Undefined,
        ByValue,
        ByReference
    };
    explicit Lambda(const Token * tok)
        : capture(nullptr), arguments(nullptr), returnTok(nullptr), bodyTok(nullptr), explicitCaptures(), implicitCapture(Capture::Undefined) {
        if (!Token::simpleMatch(tok, "[") || !tok->link())
            return;
        capture = tok;

        if (Token::simpleMatch(capture->link(), "] (")) {
            arguments = capture->link()->next();
        }
        const Token * afterArguments = arguments ? arguments->link()->next() : capture->link()->next();
        if (afterArguments && afterArguments->originalName() == "->") {
            returnTok = afterArguments->next();
            bodyTok = Token::findsimplematch(returnTok, "{");
        } else if (Token::simpleMatch(afterArguments, "{")) {
            bodyTok = afterArguments;
        }
        for (const Token* c:getCaptures()) {
            if (c->variable()) {
                explicitCaptures[c->variable()] = std::make_pair(c, Capture::ByValue);
            } else if (c->isUnaryOp("&") && Token::Match(c->astOperand1(), "%var%")) {
                explicitCaptures[c->astOperand1()->variable()] = std::make_pair(c->astOperand1(), Capture::ByReference);
            } else {
                const std::string& s = c->expressionString();
                if (s == "=")
                    implicitCapture = Capture::ByValue;
                else if (s == "&")
                    implicitCapture = Capture::ByReference;
            }
        }
    }

    const Token * capture;
    const Token * arguments;
    const Token * returnTok;
    const Token * bodyTok;
    std::unordered_map<const Variable*, std::pair<const Token*, Capture>> explicitCaptures;
    Capture implicitCapture;

    std::vector<const Token*> getCaptures() {
        return getArguments(capture);
    }

    bool isLambda() const {
        return capture && bodyTok;
    }
};

static bool isDecayedPointer(const Token *tok)
{
    if (!tok)
        return false;
    if (!tok->astParent())
        return false;
    if (astIsPointer(tok->astParent()) && !Token::simpleMatch(tok->astParent(), "return"))
        return true;
    if (tok->astParent()->isConstOp())
        return true;
    if (!Token::simpleMatch(tok->astParent(), "return"))
        return false;
    return astIsPointer(tok->astParent());
}

static void valueFlowLifetime(TokenList *tokenlist, SymbolDatabase*, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->scope())
            continue;
        if (tok->scope()->type == Scope::eGlobal)
            continue;
        Lambda lam(tok);
        // Lamdas
        if (lam.isLambda()) {
            const Scope * bodyScope = lam.bodyTok->scope();

            std::set<const Scope *> scopes;
            // Avoid capturing a variable twice
            std::set<nonneg int> varids;

            auto isImplicitCapturingVariable = [&](const Token *varTok) {
                const Variable *var = varTok->variable();
                if (!var)
                    return false;
                if (varids.count(var->declarationId()) > 0)
                    return false;
                if (!var->isLocal() && !var->isArgument())
                    return false;
                const Scope *scope = var->scope();
                if (!scope)
                    return false;
                if (scopes.count(scope) > 0)
                    return false;
                if (scope->isNestedIn(bodyScope))
                    return false;
                scopes.insert(scope);
                varids.insert(var->declarationId());
                return true;
            };

            bool update = false;
            auto captureVariable = [&](const Token* tok2, Lambda::Capture c, std::function<bool(const Token*)> pred) {
                if (varids.count(tok->varId()) > 0)
                    return;
                ErrorPath errorPath;
                if (c == Lambda::Capture::ByReference) {
                    LifetimeStore ls{
                        tok2, "Lambda captures variable by reference here.", ValueFlow::Value::LifetimeKind::Lambda};
                    ls.forward = false;
                    update |= ls.byRef(tok, tokenlist, errorLogger, settings, pred);
                } else if (c == Lambda::Capture::ByValue) {
                    LifetimeStore ls{
                        tok2, "Lambda captures variable by value here.", ValueFlow::Value::LifetimeKind::Lambda};
                    ls.forward = false;
                    update |= ls.byVal(tok, tokenlist, errorLogger, settings, pred);
                }
            };

            // Handle explicit capture
            for (const auto& p:lam.explicitCaptures) {
                const Variable* var = p.first;
                if (!var)
                    continue;
                const Token* tok2 = p.second.first;
                Lambda::Capture c = p.second.second;
                captureVariable(tok2, c, [](const Token*) {
                    return true;
                });
                varids.insert(var->declarationId());
            }

            for (const Token * tok2 = lam.bodyTok; tok2 != lam.bodyTok->link(); tok2 = tok2->next()) {
                if (!tok2->variable())
                    continue;
                captureVariable(tok2, lam.implicitCapture, isImplicitCapturingVariable);
            }
            if (update)
                valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
        }
        // address of
        else if (tok->isUnaryOp("&")) {
            for (const LifetimeToken& lt : getLifetimeTokens(tok->astOperand1())) {
                if (!settings->inconclusive && lt.inconclusive)
                    continue;
                ErrorPath errorPath = lt.errorPath;
                errorPath.emplace_back(tok, "Address of variable taken here.");

                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::LIFETIME;
                value.lifetimeScope = ValueFlow::Value::LifetimeScope::Local;
                value.tokvalue = lt.token;
                value.errorPath = std::move(errorPath);
                if (lt.addressOf || astIsPointer(lt.token) || !Token::Match(lt.token->astParent(), ".|["))
                    value.lifetimeKind = ValueFlow::Value::LifetimeKind::Address;
                value.setInconclusive(lt.inconclusive);
                setTokenValue(tok, value, tokenlist->getSettings());

                valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
            }
        }
        // container lifetimes
        else if (astIsContainer(tok)) {
            Token * parent = astParentSkipParens(tok);
            if (!Token::Match(parent, ". %name% ("))
                continue;

            bool isContainerOfPointers = true;
            const Token* containerTypeToken = tok->valueType()->containerTypeToken;
            if (containerTypeToken) {
                ValueType vt = ValueType::parseDecl(containerTypeToken, settings);
                isContainerOfPointers = vt.pointer > 0;
            }

            LifetimeStore ls;

            if (astIsIterator(parent->tokAt(2)))
                ls = LifetimeStore{tok, "Iterator to container is created here.", ValueFlow::Value::LifetimeKind::Iterator};
            else if ((astIsPointer(parent->tokAt(2)) && !isContainerOfPointers) || Token::Match(parent->next(), "data|c_str"))
                ls = LifetimeStore{tok, "Pointer to container is created here.", ValueFlow::Value::LifetimeKind::Object};
            else
                continue;

            // Dereferencing
            if (tok->isUnaryOp("*") || parent->originalName() == "->")
                ls.byDerefCopy(parent->tokAt(2), tokenlist, errorLogger, settings);
            else
                ls.byRef(parent->tokAt(2), tokenlist, errorLogger, settings);

        }
        // Check constructors
        else if (Token::Match(tok, "=|return|%type%|%var% {")) {
            valueFlowLifetimeConstructor(tok->next(), tokenlist, errorLogger, settings);
        }
        // Check function calls
        else if (Token::Match(tok, "%name% (")) {
            valueFlowLifetimeFunction(tok, tokenlist, errorLogger, settings);
        }
        // Check variables
        else if (tok->variable()) {
            ErrorPath errorPath;
            const Variable * var = getLifetimeVariable(tok, errorPath);
            if (!var)
                continue;
            if (var->nameToken() == tok)
                continue;
            if (var->isArray() && !var->isStlType() && !var->isArgument() && isDecayedPointer(tok)) {
                errorPath.emplace_back(tok, "Array decayed to pointer here.");

                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::LIFETIME;
                value.lifetimeScope = ValueFlow::Value::LifetimeScope::Local;
                value.tokvalue = var->nameToken();
                value.errorPath = errorPath;
                setTokenValue(tok, value, tokenlist->getSettings());

                valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
            }
        }
        // Forward any lifetimes
        else if (std::any_of(tok->values().begin(), tok->values().end(), std::mem_fn(&ValueFlow::Value::isLifetimeValue))) {
            valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
        }
    }
}

static bool isStdMoveOrStdForwarded(Token * tok, ValueFlow::Value::MoveKind * moveKind, Token ** varTok = nullptr)
{
    if (tok->str() != "std")
        return false;
    ValueFlow::Value::MoveKind kind = ValueFlow::Value::MoveKind::NonMovedVariable;
    Token * variableToken = nullptr;
    if (Token::Match(tok, "std :: move ( %var% )")) {
        variableToken = tok->tokAt(4);
        kind = ValueFlow::Value::MoveKind::MovedVariable;
    } else if (Token::simpleMatch(tok, "std :: forward <")) {
        const Token * const leftAngle = tok->tokAt(3);
        Token * rightAngle = leftAngle->link();
        if (Token::Match(rightAngle, "> ( %var% )")) {
            variableToken = rightAngle->tokAt(2);
            kind = ValueFlow::Value::MoveKind::ForwardedVariable;
        }
    }
    if (!variableToken)
        return false;
    if (variableToken->strAt(2) == ".") // Only partially moved
        return false;
    if (variableToken->valueType() && variableToken->valueType()->type >= ValueType::Type::VOID)
        return false;
    if (moveKind != nullptr)
        *moveKind = kind;
    if (varTok != nullptr)
        *varTok = variableToken;
    return true;
}

static bool isOpenParenthesisMemberFunctionCallOfVarId(const Token * openParenthesisToken, nonneg int varId)
{
    const Token * varTok = openParenthesisToken->tokAt(-3);
    return Token::Match(varTok, "%varid% . %name% (", varId) &&
           varTok->next()->originalName() == emptyString;
}

static const Token * findOpenParentesisOfMove(const Token * moveVarTok)
{
    const Token * tok = moveVarTok;
    while (tok && tok->str() != "(")
        tok = tok->previous();
    return tok;
}

static const Token * findEndOfFunctionCallForParameter(const Token * parameterToken)
{
    if (!parameterToken)
        return nullptr;
    const Token * parent = parameterToken->astParent();
    while (parent && !parent->isOp() && parent->str() != "(")
        parent = parent->astParent();
    if (!parent)
        return nullptr;
    return nextAfterAstRightmostLeaf(parent);
}

static void valueFlowAfterMove(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    if (!tokenlist->isCPP() || settings->standards.cpp < Standards::CPP11)
        return;
    for (const Scope * scope : symboldatabase->functionScopes) {
        if (!scope)
            continue;
        const Token * start = scope->bodyStart;
        if (scope->function) {
            const Token * memberInitializationTok = scope->function->constructorMemberInitialization();
            if (memberInitializationTok)
                start = memberInitializationTok;
        }

        for (Token* tok = const_cast<Token*>(start); tok != scope->bodyEnd; tok = tok->next()) {
            Token * varTok;
            if (Token::Match(tok, "%var% . reset|clear (") && tok->next()->originalName() == emptyString) {
                varTok = tok;
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::MOVED;
                value.moveKind = ValueFlow::Value::MoveKind::NonMovedVariable;
                value.errorPath.emplace_back(tok, "Calling " + tok->next()->expressionString() + " makes " + tok->str() + " 'non-moved'");
                value.setKnown();
                std::list<ValueFlow::Value> values;
                values.push_back(value);

                const Variable *var = varTok->variable();
                if (!var || (!var->isLocal() && !var->isArgument()))
                    continue;
                const int varId = varTok->varId();
                const Token * const endOfVarScope = var->scope()->bodyEnd;
                setTokenValue(varTok, value, settings);
                valueFlowForwardVariable(
                    varTok->next(), endOfVarScope, var, varId, values, false, false, tokenlist, errorLogger, settings);
                continue;
            }
            ValueFlow::Value::MoveKind moveKind;
            if (!isStdMoveOrStdForwarded(tok, &moveKind, &varTok))
                continue;
            const int varId = varTok->varId();
            // x is not MOVED after assignment if code is:  x = ... std::move(x) .. ;
            const Token *parent = tok->astParent();
            while (parent && parent->str() != "=" && parent->str() != "return" &&
                   !(parent->str() == "(" && isOpenParenthesisMemberFunctionCallOfVarId(parent, varId)))
                parent = parent->astParent();
            if (parent &&
                (parent->str() == "return" || // MOVED in return statement
                 parent->str() == "(")) // MOVED in self assignment, isOpenParenthesisMemberFunctionCallOfVarId == true
                continue;
            if (parent && parent->astOperand1() && parent->astOperand1()->varId() == varId)
                continue;
            const Variable *var = varTok->variable();
            if (!var)
                continue;
            const Token * const endOfVarScope = var->scope()->bodyEnd;

            ValueFlow::Value value;
            value.valueType = ValueFlow::Value::ValueType::MOVED;
            value.moveKind = moveKind;
            if (moveKind == ValueFlow::Value::MoveKind::MovedVariable)
                value.errorPath.emplace_back(tok, "Calling std::move(" + varTok->str() + ")");
            else // if (moveKind == ValueFlow::Value::ForwardedVariable)
                value.errorPath.emplace_back(tok, "Calling std::forward(" + varTok->str() + ")");
            value.setKnown();
            std::list<ValueFlow::Value> values;
            values.push_back(value);
            const Token * openParentesisOfMove = findOpenParentesisOfMove(varTok);
            const Token * endOfFunctionCall = findEndOfFunctionCallForParameter(openParentesisOfMove);
            if (endOfFunctionCall)
                valueFlowForwardVariable(const_cast<Token*>(endOfFunctionCall),
                                         endOfVarScope,
                                         var,
                                         varId,
                                         values,
                                         false,
                                         false,
                                         tokenlist,
                                         errorLogger,
                                         settings);
        }
    }
}

static void valueFlowForwardAssign(Token * const               tok,
                                   const Variable * const      var,
                                   std::list<ValueFlow::Value> values,
                                   const bool                  constValue,
                                   const bool                  init,
                                   TokenList * const           tokenlist,
                                   ErrorLogger * const         errorLogger,
                                   const Settings * const      settings)
{
    const Token * endOfVarScope = nullptr;
    if (var->isLocal())
        endOfVarScope = var->scope()->bodyEnd;
    if (!endOfVarScope)
        endOfVarScope = tok->scope()->bodyEnd;
    if (std::any_of(values.begin(), values.end(), std::mem_fn(&ValueFlow::Value::isLifetimeValue))) {
        valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
        values.remove_if(std::mem_fn(&ValueFlow::Value::isLifetimeValue));
    }
    if (!var->isPointer() && !var->isSmartPointer())
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
    if (tok->astParent()) {
        for (ValueFlow::Value& value : values) {
            const std::string info = "Assignment '" + tok->astParent()->expressionString() + "', assigned value is " + value.infoString();
            value.errorPath.emplace_back(tok, info);
        }
    }

    if (tokenlist->isCPP() && Token::Match(var->typeStartToken(), "bool|_Bool")) {
        std::list<ValueFlow::Value>::iterator it;
        for (it = values.begin(); it != values.end(); ++it) {
            if (it->isIntValue())
                it->intvalue = (it->intvalue != 0);
            if (it->isTokValue())
                it ->intvalue = (it->tokvalue != nullptr);
        }
    }

    // Static variable initialisation?
    if (var->isStatic() && init)
        lowerToPossible(values);

    // Skip RHS
    const Token * nextExpression = tok->astParent() ? nextAfterAstRightmostLeaf(tok->astParent()) : tok->next();

    if (std::any_of(values.begin(), values.end(), std::mem_fn(&ValueFlow::Value::isTokValue))) {
        std::list<ValueFlow::Value> tokvalues;
        std::copy_if(values.begin(),
                     values.end(),
                     std::back_inserter(tokvalues),
                     std::mem_fn(&ValueFlow::Value::isTokValue));
        valueFlowForwardVariable(const_cast<Token*>(nextExpression),
                                 endOfVarScope,
                                 var,
                                 var->declarationId(),
                                 tokvalues,
                                 constValue,
                                 false,
                                 tokenlist,
                                 errorLogger,
                                 settings);
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
    }
    for (ValueFlow::Value& value:values)
        value.tokvalue = tok;
    valueFlowForwardVariable(const_cast<Token*>(nextExpression),
                             endOfVarScope,
                             var,
                             var->declarationId(),
                             values,
                             constValue,
                             false,
                             tokenlist,
                             errorLogger,
                             settings);
}

static std::list<ValueFlow::Value> truncateValues(std::list<ValueFlow::Value> values, const ValueType *valueType, const Settings *settings)
{
    if (!valueType || !valueType->isIntegral())
        return values;

    const size_t sz = ValueFlow::getSizeOf(*valueType, settings);

    for (ValueFlow::Value &value : values) {
        if (value.isFloatValue()) {
            value.intvalue = value.floatValue;
            value.valueType = ValueFlow::Value::INT;
        }

        if (value.isIntValue() && sz > 0 && sz < 8) {
            const MathLib::biguint unsignedMaxValue = (1ULL << (sz * 8)) - 1ULL;
            const MathLib::biguint signBit = 1ULL << (sz * 8 - 1);
            value.intvalue &= unsignedMaxValue;
            if (valueType->sign == ValueType::Sign::SIGNED && (value.intvalue & signBit))
                value.intvalue |= ~unsignedMaxValue;
        }
    }
    return values;
}

static bool isLiteralNumber(const Token *tok, bool cpp)
{
    return tok->isNumber() || tok->isEnumerator() || tok->str() == "NULL" || (cpp && Token::Match(tok, "false|true|nullptr"));
}

static bool isVariableInit(const Token *tok)
{
    return tok->str() == "(" &&
           tok->isBinaryOp() &&
           tok->astOperand1()->variable() &&
           tok->astOperand1()->variable()->nameToken() == tok->astOperand1() &&
           tok->astOperand1()->variable()->valueType() &&
           tok->astOperand1()->variable()->valueType()->type >= ValueType::Type::VOID &&
           !Token::simpleMatch(tok->astOperand2(), ",");
}

static void valueFlowAfterAssign(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope * scope : symboldatabase->functionScopes) {
        std::set<int> aliased;
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            // Alias
            if (tok->isUnaryOp("&")) {
                aliased.insert(tok->astOperand1()->varId());
                continue;
            }

            // Assignment
            if ((tok->str() != "=" && !isVariableInit(tok)) || (tok->astParent()))
                continue;

            // Lhs should be a variable
            if (!tok->astOperand1() || !tok->astOperand1()->varId())
                continue;
            const int varid = tok->astOperand1()->varId();
            if (aliased.find(varid) != aliased.end())
                continue;
            const Variable *var = tok->astOperand1()->variable();
            if (!var || (!var->isLocal() && !var->isGlobal() && !var->isArgument()))
                continue;

            // Rhs values..
            if (!tok->astOperand2() || tok->astOperand2()->values().empty())
                continue;

            std::list<ValueFlow::Value> values = truncateValues(tok->astOperand2()->values(), tok->astOperand1()->valueType(), settings);
            // Remove known values
            std::set<ValueFlow::Value::ValueType> types;
            if (tok->astOperand1()->hasKnownValue()) {
                for (const ValueFlow::Value& value:tok->astOperand1()->values()) {
                    if (value.isKnown())
                        types.insert(value.valueType);
                }
            }
            values.remove_if([&](const ValueFlow::Value& value) {
                return types.count(value.valueType) > 0;
            });
            // Remove container size if its not a container
            if (!astIsContainer(tok->astOperand2()))
                values.remove_if([&](const ValueFlow::Value& value) {
                return value.valueType == ValueFlow::Value::CONTAINER_SIZE;
            });
            if (values.empty())
                continue;
            const bool constValue = isLiteralNumber(tok->astOperand2(), tokenlist->isCPP());
            const bool init = var->nameToken() == tok->astOperand1();
            valueFlowForwardAssign(tok->astOperand2(), var, values, constValue, init, tokenlist, errorLogger, settings);
        }
    }
}

static void valueFlowSetConditionToKnown(const Token* tok, std::list<ValueFlow::Value>& values, bool then)
{
    if (values.empty())
        return;
    if (then && !Token::Match(tok, "==|!|("))
        return;
    if (!then && !Token::Match(tok, "!=|%var%|("))
        return;
    if (isConditionKnown(tok, then))
        changePossibleToKnown(values);
}

static bool isBreakScope(const Token* const endToken)
{
    if (!Token::simpleMatch(endToken, "}"))
        return false;
    if (!Token::simpleMatch(endToken->link(), "{"))
        return false;
    return Token::findmatch(endToken->link(), "break|goto", endToken);
}

static ValueFlow::Value asImpossible(ValueFlow::Value v)
{
    v.invertRange();
    v.setImpossible();
    return v;
}

static void insertImpossible(std::list<ValueFlow::Value>& values, const std::list<ValueFlow::Value>& input)
{
    std::transform(input.begin(), input.end(), std::back_inserter(values), &asImpossible);
}

static void insertNegateKnown(std::list<ValueFlow::Value>& values, const std::list<ValueFlow::Value>& input)
{
    for (ValueFlow::Value value:input) {
        if (!value.isIntValue() && !value.isContainerSizeValue())
            continue;
        value.intvalue = !value.intvalue;
        value.setKnown();
        values.push_back(value);
    }
}

static std::vector<const Variable*> getExprVariables(const Token* expr,
        const TokenList* tokenlist,
        const SymbolDatabase* symboldatabase,
        const Settings* settings)
{
    std::vector<const Variable*> result;
    FwdAnalysis fwdAnalysis(tokenlist->isCPP(), settings->library);
    std::set<int> varids = fwdAnalysis.getExprVarIds(expr);
    std::transform(varids.begin(), varids.end(), std::back_inserter(result), [&](int id) {
        return symboldatabase->getVariableFromVarId(id);
    });
    return result;
}

struct ConditionHandler {
    struct Condition {
        const Token *vartok;
        std::list<ValueFlow::Value> true_values;
        std::list<ValueFlow::Value> false_values;
        bool inverted = false;

        Condition() : vartok(nullptr), true_values(), false_values(), inverted(false) {}
    };

    virtual bool forward(Token* start,
                         const Token* stop,
                         const Token* exprTok,
                         const std::list<ValueFlow::Value>& values,
                         TokenList* tokenlist,
                         const Settings* settings) const = 0;

    virtual Condition parse(const Token* tok, const Settings* settings) const = 0;

    void traverseCondition(
        TokenList* tokenlist,
        SymbolDatabase* symboldatabase,
        ErrorLogger* errorLogger,
        const Settings* settings,
        const std::function<
        void(const Condition& cond, Token* tok, const Scope* scope, const std::vector<const Variable*>& vars)>& f) const {
        for (const Scope *scope : symboldatabase->functionScopes) {
            std::set<unsigned> aliased;
            for (Token *tok = const_cast<Token *>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
                if (Token::Match(tok, "if|while|for ("))
                    continue;

                if (Token::Match(tok, "= & %var% ;"))
                    aliased.insert(tok->tokAt(2)->varId());
                const Token* top = tok->astTop();
                if (!top)
                    continue;

                if (!Token::Match(top->previous(), "if|while|for (") && !Token::Match(tok->astParent(), "&&|%oror%"))
                    continue;

                Condition cond = parse(tok, settings);
                if (!cond.vartok)
                    continue;
                if (cond.vartok->variable() && cond.vartok->variable()->isVolatile())
                    continue;
                if (cond.true_values.empty() || cond.false_values.empty())
                    continue;

                if (exprDependsOnThis(cond.vartok))
                    continue;
                std::vector<const Variable*> vars = getExprVariables(cond.vartok, tokenlist, symboldatabase, settings);
                if (std::any_of(vars.begin(), vars.end(), [](const Variable* var) {
                return !var;
            }))
                continue;
                if (!vars.empty() && (vars.front()))
                    if (std::any_of(vars.begin(), vars.end(), [&](const Variable* var) {
                    return var && aliased.find(var->declarationId()) != aliased.end();
                    })) {
                    if (settings->debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                cond.vartok,
                                "variable is aliased so we just skip all valueflow after condition");
                    continue;
                }
                f(cond, tok, scope, vars);
            }
        }
    }

    void afterCondition(TokenList* tokenlist,
                        SymbolDatabase* symboldatabase,
                        ErrorLogger* errorLogger,
                        const Settings* settings) const {
        traverseCondition(
            tokenlist,
            symboldatabase,
            errorLogger,
            settings,
        [&](const Condition& cond, Token* tok, const Scope* scope, const std::vector<const Variable*>& vars) {
            const Token* top = tok->astTop();

            std::list<ValueFlow::Value> thenValues;
            std::list<ValueFlow::Value> elseValues;

            if (!Token::Match(tok, "!=|=|(|.") && tok != cond.vartok) {
                thenValues.insert(thenValues.end(), cond.true_values.begin(), cond.true_values.end());
                if (isConditionKnown(tok, false))
                    insertImpossible(elseValues, cond.false_values);
            }
            if (!Token::Match(tok, "==|!")) {
                elseValues.insert(elseValues.end(), cond.false_values.begin(), cond.false_values.end());
                if (isConditionKnown(tok, true)) {
                    insertImpossible(thenValues, cond.true_values);
                    if (Token::Match(tok, "(|.|%var%") && astIsBool(tok))
                        insertNegateKnown(thenValues, cond.true_values);
                }
            }

            if (cond.inverted)
                std::swap(thenValues, elseValues);

            if (Token::Match(tok->astParent(), "%oror%|&&")) {
                Token *parent = tok->astParent();
                if (astIsRHS(tok) && parent->astParent() && parent->str() == parent->astParent()->str())
                    parent = parent->astParent();
                else if (!astIsLHS(tok)) {
                    parent = nullptr;
                }
                if (parent) {
                    const std::string &op(parent->str());
                    std::list<ValueFlow::Value> values;
                    if (op == "&&")
                        values = thenValues;
                    else if (op == "||")
                        values = elseValues;
                    if (Token::Match(tok, "==|!="))
                        changePossibleToKnown(values);
                    if (!values.empty()) {
                        bool assign = false;
                        visitAstNodes(parent->astOperand2(), [&](Token* tok2) {
                            if (tok2 == tok)
                                return ChildrenToVisit::done;
                            if (isSameExpression(tokenlist->isCPP(), false, cond.vartok, tok2, settings->library, true, false))
                                setTokenValue(tok2, values.front(), settings);
                            else if (Token::Match(tok2, "++|--|=") && isSameExpression(tokenlist->isCPP(),
                                     false,
                                     cond.vartok,
                                     tok2->astOperand1(),
                                     settings->library,
                                     true,
                                     false)) {
                                assign = true;
                                return ChildrenToVisit::done;
                            }
                            return ChildrenToVisit::op1_and_op2;
                        });
                        if (assign)
                            return;
                    }
                }
            }

            {
                const Token *tok2 = tok;
                std::string op;
                bool mixedOperators = false;
                while (tok2->astParent()) {
                    const Token *parent = tok2->astParent();
                    if (Token::Match(parent, "%oror%|&&")) {
                        if (op.empty()) {
                            op = parent->str() == "&&" ? "&&" : "||";
                        } else if (op != parent->str()) {
                            mixedOperators = true;
                            break;
                        }
                    }
                    if (parent->str()=="!") {
                        op = (op == "&&" ? "||" : "&&");
                    }
                    tok2 = parent;
                }

                if (mixedOperators) {
                    return;
                }
            }

            if (top && Token::Match(top->previous(), "if|while (") && !top->previous()->isExpandedMacro()) {
                // does condition reassign variable?
                if (tok != top->astOperand2() && Token::Match(top->astOperand2(), "%oror%|&&") &&
                    isVariablesChanged(top, top->link(), 0, vars, settings, tokenlist->isCPP())) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok, "assignment in condition");
                    return;
                }

                // start token of conditional code
                Token* startTokens[] = {nullptr, nullptr};

                // if astParent is "!" we need to invert codeblock
                {
                    const Token *tok2 = tok;
                    while (tok2->astParent()) {
                        const Token *parent = tok2->astParent();
                        while (parent && parent->str() == "&&")
                            parent = parent->astParent();
                        if (parent && (parent->str() == "!" || Token::simpleMatch(parent, "== false"))) {
                            std::swap(thenValues, elseValues);
                        }
                        tok2 = parent;
                    }
                }

                // determine startToken(s)
                if (Token::simpleMatch(top->link(), ") {"))
                    startTokens[0] = top->link()->next();
                if (Token::simpleMatch(top->link()->linkAt(1), "} else {"))
                    startTokens[1] = top->link()->linkAt(1)->tokAt(2);

                int changeBlock = -1;

                for (int i = 0; i < 2; i++) {
                    const Token *const startToken = startTokens[i];
                    if (!startToken)
                        continue;
                    std::list<ValueFlow::Value>& values = (i == 0 ? thenValues : elseValues);
                    valueFlowSetConditionToKnown(tok, values, i == 0);

                    // TODO: The endToken should not be startTokens[i]->link() in the valueFlowForwardVariable call
                    if (forward(startTokens[i], startTokens[i]->link(), cond.vartok, values, tokenlist, settings))
                        changeBlock = i;
                    changeKnownToPossible(values);
                }
                // TODO: Values changed in noreturn blocks should not bail
                if (changeBlock >= 0 && !Token::simpleMatch(top->previous(), "while (")) {
                    if (settings->debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                startTokens[changeBlock]->link(),
                                "valueFlowAfterCondition: " + cond.vartok->expressionString() +
                                " is changed in conditional block");
                    return;
                }

                // After conditional code..
                if (Token::simpleMatch(top->link(), ") {")) {
                    Token *after = top->link()->linkAt(1);
                    const Token* unknownFunction = nullptr;
                    const bool isWhile =
                        tok->astParent() && Token::simpleMatch(tok->astParent()->previous(), "while (");
                    bool dead_if = (!isBreakScope(after) && isWhile) ||
                                   (isReturnScope(after, &settings->library, &unknownFunction) && !isWhile);
                    bool dead_else = false;

                    if (!dead_if && unknownFunction) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, unknownFunction, "possible noreturn scope");
                        return;
                    }

                    if (Token::simpleMatch(after, "} else {")) {
                        after = after->linkAt(2);
                        unknownFunction = nullptr;
                        dead_else = isReturnScope(after, &settings->library, &unknownFunction);
                        if (!dead_else && unknownFunction) {
                            if (settings->debugwarnings)
                                bailout(tokenlist, errorLogger, unknownFunction, "possible noreturn scope");
                            return;
                        }
                    }

                    if (dead_if && dead_else)
                        return;

                    std::list<ValueFlow::Value> values;
                    if (dead_if) {
                        values = elseValues;
                    } else if (dead_else) {
                        values = thenValues;
                    } else {
                        std::copy_if(thenValues.begin(),
                                     thenValues.end(),
                                     std::back_inserter(values),
                                     std::mem_fn(&ValueFlow::Value::isPossible));
                        std::copy_if(elseValues.begin(),
                                     elseValues.end(),
                                     std::back_inserter(values),
                                     std::mem_fn(&ValueFlow::Value::isPossible));
                    }

                    if (!values.empty()) {
                        if ((dead_if || dead_else) && !Token::Match(tok->astParent(), "&&|&")) {
                            valueFlowSetConditionToKnown(tok, values, true);
                            valueFlowSetConditionToKnown(tok, values, false);
                        }
                        forward(after, scope->bodyEnd, cond.vartok, values, tokenlist, settings);
                    }
                }
            }
        });
    }
    virtual ~ConditionHandler() {}
};

static void valueFlowCondition(const ValuePtr<ConditionHandler>& handler,
                               TokenList* tokenlist,
                               SymbolDatabase* symboldatabase,
                               ErrorLogger* errorLogger,
                               const Settings* settings)
{
    handler->afterCondition(tokenlist, symboldatabase, errorLogger, settings);
}

struct SimpleConditionHandler : ConditionHandler {
    virtual bool forward(Token* start,
                         const Token* stop,
                         const Token* exprTok,
                         const std::list<ValueFlow::Value>& values,
                         TokenList* tokenlist,
                         const Settings* settings) const OVERRIDE {
        return valueFlowForward(start->next(), stop, exprTok, values, tokenlist, settings).isModified();
    }

    virtual Condition parse(const Token* tok, const Settings*) const OVERRIDE {
        Condition cond;
        ValueFlow::Value true_value;
        ValueFlow::Value false_value;
        const Token *vartok = parseCompareInt(tok, true_value, false_value);
        if (vartok) {
            if (vartok->hasKnownValue())
                return cond;
            if (vartok->str() == "=" && vartok->astOperand1() && vartok->astOperand2())
                vartok = vartok->astOperand1();
            cond.true_values.push_back(true_value);
            cond.false_values.push_back(false_value);
            cond.vartok = vartok;
            return cond;
        }

        if (tok->str() == "!") {
            vartok = tok->astOperand1();

        } else if (tok->astParent() && (Token::Match(tok->astParent(), "%oror%|&&") ||
                                        Token::Match(tok->astParent()->previous(), "if|while ("))) {
            if (Token::simpleMatch(tok, "="))
                vartok = tok->astOperand1();
            else if (!Token::Match(tok, "%comp%|%assign%"))
                vartok = tok;
        }

        if (!vartok)
            return cond;
        cond.true_values.emplace_back(tok, 0LL);
        cond.false_values.emplace_back(tok, 0LL);
        cond.vartok = vartok;

        return cond;
    }
};

static bool isInBounds(const ValueFlow::Value& value, MathLib::bigint x)
{
    if (value.intvalue == x)
        return true;
    if (value.bound == ValueFlow::Value::Bound::Lower && value.intvalue > x)
        return false;
    if (value.bound == ValueFlow::Value::Bound::Upper && value.intvalue < x)
        return false;
    // Checking for equality is not necessary since we already know the value is not equal
    if (value.bound == ValueFlow::Value::Bound::Point)
        return false;
    return true;
}

static const ValueFlow::Value* getCompareIntValue(const std::list<ValueFlow::Value>& values, std::function<bool(MathLib::bigint, MathLib::bigint)> compare)
{
    const ValueFlow::Value* result = nullptr;
    for (const ValueFlow::Value& value : values) {
        if (!value.isIntValue())
            continue;
        if (result)
            result = &std::min(value, *result, [compare](const ValueFlow::Value& x, const ValueFlow::Value& y) {
            return compare(x.intvalue, y.intvalue);
        });
        else
            result = &value;
    }
    return result;
}

static const ValueFlow::Value* proveLessThan(const std::list<ValueFlow::Value>& values, MathLib::bigint x)
{
    const ValueFlow::Value* result = nullptr;
    const ValueFlow::Value* maxValue = getCompareIntValue(values, std::greater<MathLib::bigint> {});
    if (maxValue && maxValue->isImpossible() && maxValue->bound == ValueFlow::Value::Bound::Lower) {
        if (maxValue->intvalue <= x)
            result = maxValue;
    }
    return result;
}

static const ValueFlow::Value* proveGreaterThan(const std::list<ValueFlow::Value>& values, MathLib::bigint x)
{
    const ValueFlow::Value* result = nullptr;
    const ValueFlow::Value* minValue = getCompareIntValue(values, std::less<MathLib::bigint> {});
    if (minValue && minValue->isImpossible() && minValue->bound == ValueFlow::Value::Bound::Upper) {
        if (minValue->intvalue >= x)
            result = minValue;
    }
    return result;
}

static const ValueFlow::Value* proveNotEqual(const std::list<ValueFlow::Value>& values, MathLib::bigint x)
{
    const ValueFlow::Value* result = nullptr;
    for (const ValueFlow::Value& value : values) {
        if (value.valueType != ValueFlow::Value::INT)
            continue;
        if (result && !isInBounds(value, result->intvalue))
            continue;
        if (value.isImpossible()) {
            if (value.intvalue == x)
                return &value;
            if (!isInBounds(value, x))
                continue;
            result = &value;
        } else {
            if (value.intvalue == x)
                return nullptr;
            if (!isInBounds(value, x))
                continue;
            result = nullptr;
        }
    }
    return result;
}

static void valueFlowInferCondition(TokenList* tokenlist,
                                    const Settings* settings)
{
    for (Token* tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->astParent())
            continue;
        if (tok->hasKnownValue())
            continue;
        if (tok->variable() && (Token::Match(tok->astParent(), "?|&&|!|%oror%") ||
                                Token::Match(tok->astParent()->previous(), "if|while ("))) {
            const ValueFlow::Value* result = proveNotEqual(tok->values(), 0);
            if (!result)
                continue;
            ValueFlow::Value value = *result;
            value.intvalue = 1;
            value.bound = ValueFlow::Value::Bound::Point;
            value.setKnown();
            setTokenValue(tok, value, settings);
        } else if (tok->isComparisonOp()) {
            MathLib::bigint val = 0;
            const Token* varTok = nullptr;
            std::string op = tok->str();
            if (tok->astOperand1()->hasKnownIntValue()) {
                val = tok->astOperand1()->values().front().intvalue;
                varTok = tok->astOperand2();
                // Flip the operator
                if (op == ">")
                    op = "<";
                else if (op == "<")
                    op = ">";
                else if (op == ">=")
                    op = "<=";
                else if (op == "<=")
                    op = ">=";
            } else if (tok->astOperand2()->hasKnownIntValue()) {
                val = tok->astOperand2()->values().front().intvalue;
                varTok = tok->astOperand1();
            }
            if (!varTok)
                continue;
            if (varTok->hasKnownIntValue())
                continue;
            if (varTok->values().empty())
                continue;
            const ValueFlow::Value* result = nullptr;
            bool known = false;
            if (op == "==" || op == "!=") {
                result = proveNotEqual(varTok->values(), val);
                known = op == "!=";
            } else if (op == "<" || op == ">=") {
                result = proveLessThan(varTok->values(), val);
                known = op == "<";
                if (!result && !isSaturated(val)) {
                    result = proveGreaterThan(varTok->values(), val - 1);
                    known = op == ">=";
                }
            } else if (op == ">" || op == "<=") {
                result = proveGreaterThan(varTok->values(), val);
                known = op == ">";
                if (!result && !isSaturated(val)) {
                    result = proveLessThan(varTok->values(), val + 1);
                    known = op == "<=";
                }
            }
            if (!result)
                continue;
            ValueFlow::Value value = *result;
            value.intvalue = known;
            value.bound = ValueFlow::Value::Bound::Point;
            value.setKnown();
            setTokenValue(tok, value, settings);
        }
    }
}

static bool valueFlowForLoop2(const Token *tok,
                              ProgramMemory *memory1,
                              ProgramMemory *memory2,
                              ProgramMemory *memoryAfter)
{
    // for ( firstExpression ; secondExpression ; thirdExpression )
    const Token *firstExpression  = tok->next()->astOperand2()->astOperand1();
    const Token *secondExpression = tok->next()->astOperand2()->astOperand2()->astOperand1();
    const Token *thirdExpression = tok->next()->astOperand2()->astOperand2()->astOperand2();

    ProgramMemory programMemory;
    MathLib::bigint result(0);
    bool error = false;
    execute(firstExpression, &programMemory, &result, &error);
    if (error)
        return false;
    execute(secondExpression, &programMemory, &result, &error);
    if (result == 0) // 2nd expression is false => no looping
        return false;
    if (error) {
        // If a variable is reassigned in second expression, return false
        bool reassign = false;
        visitAstNodes(secondExpression,
        [&](const Token *t) {
            if (t->str() == "=" && t->astOperand1() && programMemory.hasValue(t->astOperand1()->varId()))
                // TODO: investigate what variable is assigned.
                reassign = true;
            return reassign ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
        });
        if (reassign)
            return false;
    }

    ProgramMemory startMemory(programMemory);
    ProgramMemory endMemory;

    int maxcount = 10000;
    while (result != 0 && !error && --maxcount > 0) {
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

static void valueFlowForLoopSimplify(Token * const bodyStart, const nonneg int varid, bool globalvar, const MathLib::bigint value, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    const Token * const bodyEnd = bodyStart->link();

    // Is variable modified inside for loop
    if (isVariableChanged(bodyStart, bodyEnd, varid, globalvar, settings, tokenlist->isCPP()))
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
                        parent = nullptr;
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
            setTokenValue(tok2, value1, settings);
        }

        if (Token::Match(tok2, "%oror%|&&")) {
            const ProgramMemory programMemory(getProgramMemory(tok2->astTop(), varid, ValueFlow::Value(value)));
            if ((tok2->str() == "&&" && !conditionIsTrue(tok2->astOperand1(), programMemory)) ||
                (tok2->str() == "||" && !conditionIsFalse(tok2->astOperand1(), programMemory))) {
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

        else if (Token::simpleMatch(tok2, ") {")) {
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "For loop skipping {} code");
            tok2 = tok2->linkAt(1);
            if (Token::simpleMatch(tok2, "} else {"))
                tok2 = tok2->linkAt(2);
        }
    }
}

static void valueFlowForLoopSimplifyAfter(Token *fortok, nonneg int varid, const MathLib::bigint num, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
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
        endToken = var->scope()->bodyEnd;
    else
        endToken = fortok->scope()->bodyEnd;

    Token* blockTok = fortok->linkAt(1)->linkAt(1);
    std::list<ValueFlow::Value> values;
    values.emplace_back(num);
    values.back().errorPath.emplace_back(fortok,"After for loop, " + var->name() + " has value " + values.back().infoString());

    if (blockTok != endToken) {
        valueFlowForwardVariable(
            blockTok->next(), endToken, var, varid, values, false, false, tokenlist, errorLogger, settings);
    }
}

static void valueFlowForLoop(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope &scope : symboldatabase->scopeList) {
        if (scope.type != Scope::eFor)
            continue;

        Token* tok = const_cast<Token*>(scope.classDef);
        Token* const bodyStart = const_cast<Token*>(scope.bodyStart);

        if (!Token::simpleMatch(tok->next()->astOperand2(), ";") ||
            !Token::simpleMatch(tok->next()->astOperand2()->astOperand2(), ";"))
            continue;

        nonneg int varid;
        bool knownInitValue, partialCond;
        MathLib::bigint initValue, stepValue, lastValue;

        if (extractForLoopValues(tok, &varid, &knownInitValue, &initValue, &partialCond, &stepValue, &lastValue)) {
            const bool executeBody = !knownInitValue || initValue <= lastValue;
            if (executeBody) {
                valueFlowForLoopSimplify(bodyStart, varid, false, initValue, tokenlist, errorLogger, settings);
                if (stepValue == 1)
                    valueFlowForLoopSimplify(bodyStart, varid, false, lastValue, tokenlist, errorLogger, settings);
            }
            const MathLib::bigint afterValue = executeBody ? lastValue + stepValue : initValue;
            valueFlowForLoopSimplifyAfter(tok, varid, afterValue, tokenlist, errorLogger, settings);
        } else {
            ProgramMemory mem1, mem2, memAfter;
            if (valueFlowForLoop2(tok, &mem1, &mem2, &memAfter)) {
                ProgramMemory::Map::const_iterator it;
                for (it = mem1.values.begin(); it != mem1.values.end(); ++it) {
                    if (!it->second.isIntValue())
                        continue;
                    valueFlowForLoopSimplify(bodyStart, it->first, false, it->second.intvalue, tokenlist, errorLogger, settings);
                }
                for (it = mem2.values.begin(); it != mem2.values.end(); ++it) {
                    if (!it->second.isIntValue())
                        continue;
                    valueFlowForLoopSimplify(bodyStart, it->first, false, it->second.intvalue, tokenlist, errorLogger, settings);
                }
                for (it = memAfter.values.begin(); it != memAfter.values.end(); ++it) {
                    if (!it->second.isIntValue())
                        continue;
                    valueFlowForLoopSimplifyAfter(tok, it->first, it->second.intvalue, tokenlist, errorLogger, settings);
                }
            }
        }
    }
}

struct MultiValueFlowAnalyzer : ValueFlowAnalyzer {
    std::unordered_map<nonneg int, ValueFlow::Value> values;
    std::unordered_map<nonneg int, const Variable*> vars;

    MultiValueFlowAnalyzer() : ValueFlowAnalyzer(), values(), vars() {}

    MultiValueFlowAnalyzer(const std::unordered_map<const Variable*, ValueFlow::Value>& args, const TokenList* t)
        : ValueFlowAnalyzer(t), values(), vars() {
        for (const auto& p:args) {
            values[p.first->declarationId()] = p.second;
            vars[p.first->declarationId()] = p.first;
        }
    }

    virtual const std::unordered_map<nonneg int, const Variable*>& getVars() const {
        return vars;
    }

    virtual const ValueFlow::Value* getValue(const Token* tok) const OVERRIDE {
        if (tok->varId() == 0)
            return nullptr;
        auto it = values.find(tok->varId());
        if (it == values.end())
            return nullptr;
        return &it->second;
    }
    virtual ValueFlow::Value* getValue(const Token* tok) OVERRIDE {
        if (tok->varId() == 0)
            return nullptr;
        auto it = values.find(tok->varId());
        if (it == values.end())
            return nullptr;
        return &it->second;
    }

    virtual void makeConditional() OVERRIDE {
        for (auto&& p:values) {
            p.second.conditional = true;
        }
    }

    virtual void addErrorPath(const Token* tok, const std::string& s) OVERRIDE {
        for (auto&& p:values) {
            p.second.errorPath.emplace_back(tok, "Assuming condition is " + s);
        }
    }

    virtual bool isAlias(const Token* tok, bool& inconclusive) const OVERRIDE {
        std::list<ValueFlow::Value> vals;
        std::transform(values.begin(), values.end(), std::back_inserter(vals), SelectMapValues{});

        for (const auto& p:getVars()) {
            nonneg int varid = p.first;
            const Variable* var = p.second;
            if (tok->varId() == varid)
                return true;
            if (isAliasOf(var, tok, varid, vals, &inconclusive))
                return true;
        }
        return false;
    }

    virtual bool isGlobal() const OVERRIDE {
        return false;
    }

    virtual bool lowerToPossible() OVERRIDE {
        for (auto&& p:values) {
            if (p.second.isImpossible())
                return false;
            p.second.changeKnownToPossible();
        }
        return true;
    }
    virtual bool lowerToInconclusive() OVERRIDE {
        for (auto&& p:values) {
            if (p.second.isImpossible())
                return false;
            p.second.setInconclusive();
        }
        return true;
    }

    virtual bool isConditional() const OVERRIDE {
        for (auto&& p:values) {
            if (p.second.conditional)
                return true;
            if (p.second.condition)
                return !p.second.isImpossible();
        }
        return false;
    }

    virtual bool updateScope(const Token* endBlock, bool) const OVERRIDE {
        const Scope* scope = endBlock->scope();
        if (!scope)
            return false;
        if (scope->type == Scope::eLambda) {
            for (const auto& p:values) {
                if (!p.second.isLifetimeValue())
                    return false;
            }
            return true;
        } else if (scope->type == Scope::eIf || scope->type == Scope::eElse || scope->type == Scope::eWhile ||
                   scope->type == Scope::eFor) {
            auto pred = [](const ValueFlow::Value& value) {
                if (value.isKnown())
                    return true;
                if (value.isImpossible())
                    return true;
                if (value.isLifetimeValue())
                    return true;
                return false;
            };
            if (std::all_of(values.begin(), values.end(), std::bind(pred, std::bind(SelectMapValues{}, std::placeholders::_1))))
                return true;
            if (isConditional())
                return false;
            const Token* condTok = getCondTokFromEnd(endBlock);
            std::set<nonneg int> varids;
            std::transform(getVars().begin(), getVars().end(), std::inserter(varids, varids.begin()), SelectMapKeys{});
            return bifurcate(condTok, varids, getSettings());
        }

        return false;
    }

    virtual bool match(const Token* tok) const OVERRIDE {
        return values.count(tok->varId()) > 0;
    }

    virtual ProgramState getProgramState() const OVERRIDE {
        ProgramState ps;
        for (const auto& p:values)
            ps[p.first] = p.second;
        return ps;
    }
};

static void valueFlowInjectParameter(TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings, const Scope* functionScope, const std::unordered_map<const Variable*, std::list<ValueFlow::Value>>& vars)
{
    using Args = std::vector<std::unordered_map<const Variable*, ValueFlow::Value>>;
    Args args(1);
    // Compute cartesian product of all arguments
    for (const auto& p:vars) {
        if (p.second.empty())
            continue;
        args.back()[p.first] = p.second.front();
    }
    for (const auto& p:vars) {
        if (args.size() > 256) {
            std::string fname = "<unknown>";
            Function* f = functionScope->function;
            if (f)
                fname = f->name();
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, functionScope->bodyStart, "Too many argument passed to " + fname);
            break;
        }
        std::for_each(std::next(p.second.begin()), p.second.end(), [&](const ValueFlow::Value& value) {
            Args new_args;
            for (auto arg:args) {
                if (value.path != 0) {
                    for (const auto& q:arg) {
                        if (q.second.path == 0)
                            continue;
                        if (q.second.path != value.path)
                            return;
                    }
                }
                arg[p.first] = value;
                new_args.push_back(arg);
            }
            std::copy(new_args.begin(), new_args.end(), std::back_inserter(args));
        });
    }

    for (const auto& arg:args) {
        if (arg.empty())
            continue;
        bool skip = false;
        // Make sure all arguments are the same path
        MathLib::bigint path = arg.begin()->second.path;
        for (const auto& p:arg) {
            if (p.second.path != path) {
                skip = true;
                break;
            }
        }
        if (skip)
            continue;
        MultiValueFlowAnalyzer a(arg, tokenlist);
        valueFlowGenericForward(const_cast<Token*>(functionScope->bodyStart), functionScope->bodyEnd, a, settings);
    }
}

static void valueFlowInjectParameter(TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings, const Variable* arg, const Scope* functionScope, const std::list<ValueFlow::Value>& argvalues)
{
    // Is argument passed by value or const reference, and is it a known non-class type?
    if (arg->isReference() && !arg->isConst() && !arg->isClass())
        return;

    // Set value in function scope..
    const int varid2 = arg->declarationId();
    if (!varid2)
        return;

    valueFlowForwardVariable(const_cast<Token*>(functionScope->bodyStart->next()),
                             functionScope->bodyEnd,
                             arg,
                             varid2,
                             argvalues,
                             false,
                             true,
                             tokenlist,
                             errorLogger,
                             settings);
}

static void valueFlowSwitchVariable(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope &scope : symboldatabase->scopeList) {
        if (scope.type != Scope::ScopeType::eSwitch)
            continue;
        if (!Token::Match(scope.classDef, "switch ( %var% ) {"))
            continue;
        const Token *vartok = scope.classDef->tokAt(2);
        const Variable *var = vartok->variable();
        if (!var)
            continue;

        // bailout: global non-const variables
        if (!(var->isLocal() || var->isArgument()) && !var->isConst()) {
            if (settings->debugwarnings)
                bailout(tokenlist, errorLogger, vartok, "switch variable " + var->name() + " is global");
            continue;
        }

        for (Token *tok = scope.bodyStart->next(); tok != scope.bodyEnd; tok = tok->next()) {
            if (tok->str() == "{") {
                tok = tok->link();
                continue;
            }
            if (Token::Match(tok, "case %num% :")) {
                std::list<ValueFlow::Value> values;
                values.emplace_back(MathLib::toLongNumber(tok->next()->str()));
                values.back().condition = tok;
                const std::string info("case " + tok->next()->str() + ": " + vartok->str() + " is " + tok->next()->str() + " here.");
                values.back().errorPath.emplace_back(tok, info);
                bool known = false;
                if ((Token::simpleMatch(tok->previous(), "{") || Token::simpleMatch(tok->tokAt(-2), "break ;")) && !Token::Match(tok->tokAt(3), ";| case"))
                    known = true;
                while (Token::Match(tok->tokAt(3), ";| case %num% :")) {
                    known = false;
                    tok = tok->tokAt(3);
                    if (!tok->isName())
                        tok = tok->next();
                    values.emplace_back(MathLib::toLongNumber(tok->next()->str()));
                    values.back().condition = tok;
                    const std::string info2("case " + tok->next()->str() + ": " + vartok->str() + " is " + tok->next()->str() + " here.");
                    values.back().errorPath.emplace_back(tok, info2);
                }
                for (std::list<ValueFlow::Value>::const_iterator val = values.begin(); val != values.end(); ++val) {
                    valueFlowReverse(tokenlist,
                                     const_cast<Token*>(scope.classDef),
                                     vartok,
                                     *val,
                                     ValueFlow::Value(),
                                     errorLogger,
                                     settings);
                }
                if (vartok->variable()->scope()) {
                    if (known)
                        values.back().setKnown();

                    // FIXME We must check if there is a return. See #9276
                    /*
                    valueFlowForwardVariable(tok->tokAt(3),
                                             vartok->variable()->scope()->bodyEnd,
                                             vartok->variable(),
                                             vartok->varId(),
                                             values,
                                             values.back().isKnown(),
                                             false,
                                             tokenlist,
                                             errorLogger,
                                             settings);
                    */
                }
            }
        }
    }
}

static void setTokenValues(Token *tok, const std::list<ValueFlow::Value> &values, const Settings *settings)
{
    for (const ValueFlow::Value &value : values) {
        if (value.isIntValue())
            setTokenValue(tok, value, settings);
    }
}

static bool evaluate(const Token *expr, const std::vector<std::list<ValueFlow::Value>> &values, std::list<ValueFlow::Value> *result)
{
    if (!expr)
        return false;

    // strlen(arg)..
    if (expr->str() == "(" && Token::Match(expr->previous(), "strlen ( %name% )")) {
        const Token *arg = expr->next();
        if (arg->str().compare(0,3,"arg") != 0 || arg->str().size() != 4)
            return false;
        const char n = arg->str()[3];
        if (n < '1' || n - '1' >= values.size())
            return false;
        for (const ValueFlow::Value &argvalue : values[n - '1']) {
            if (argvalue.isTokValue() && argvalue.tokvalue->tokType() == Token::eString) {
                ValueFlow::Value res(argvalue); // copy all "inconclusive", "condition", etc attributes
                // set return value..
                res.valueType = ValueFlow::Value::INT;
                res.tokvalue = nullptr;
                res.intvalue = Token::getStrLength(argvalue.tokvalue);
                result->emplace_back(std::move(res));
            }
        }
        return !result->empty();
    }

    // unary operands
    if (expr->astOperand1() && !expr->astOperand2()) {
        std::list<ValueFlow::Value> opvalues;
        if (!evaluate(expr->astOperand1(), values, &opvalues))
            return false;
        if (expr->str() == "+") {
            result->swap(opvalues);
            return true;
        }
        if (expr->str() == "-") {
            for (ValueFlow::Value v: opvalues) {
                if (v.isIntValue()) {
                    v.intvalue = -v.intvalue;
                    result->emplace_back(std::move(v));
                }
            }
            return true;
        }
        return false;
    }
    // binary/ternary operands
    if (expr->astOperand1() && expr->astOperand2()) {
        std::list<ValueFlow::Value> lhsValues, rhsValues;
        if (!evaluate(expr->astOperand1(), values, &lhsValues))
            return false;
        if (expr->str() != "?" && !evaluate(expr->astOperand2(), values, &rhsValues))
            return false;

        for (const ValueFlow::Value &val1 : lhsValues) {
            if (!val1.isIntValue())
                continue;
            if (expr->str() == "?") {
                rhsValues.clear();
                const Token *expr2 = val1.intvalue ? expr->astOperand2()->astOperand1() : expr->astOperand2()->astOperand2();
                if (!evaluate(expr2, values, &rhsValues))
                    continue;
                result->insert(result->end(), rhsValues.begin(), rhsValues.end());
                continue;
            }

            for (const ValueFlow::Value &val2 : rhsValues) {
                if (!val2.isIntValue())
                    continue;

                if (val1.varId != 0 && val2.varId != 0) {
                    if (val1.varId != val2.varId || val1.varvalue != val2.varvalue)
                        continue;
                }

                if (expr->str() == "+")
                    result->emplace_back(ValueFlow::Value(val1.intvalue + val2.intvalue));
                else if (expr->str() == "-")
                    result->emplace_back(ValueFlow::Value(val1.intvalue - val2.intvalue));
                else if (expr->str() == "*")
                    result->emplace_back(ValueFlow::Value(val1.intvalue * val2.intvalue));
                else if (expr->str() == "/" && val2.intvalue != 0)
                    result->emplace_back(ValueFlow::Value(val1.intvalue / val2.intvalue));
                else if (expr->str() == "%" && val2.intvalue != 0)
                    result->emplace_back(ValueFlow::Value(val1.intvalue % val2.intvalue));
                else if (expr->str() == "&")
                    result->emplace_back(ValueFlow::Value(val1.intvalue & val2.intvalue));
                else if (expr->str() == "|")
                    result->emplace_back(ValueFlow::Value(val1.intvalue | val2.intvalue));
                else if (expr->str() == "^")
                    result->emplace_back(ValueFlow::Value(val1.intvalue ^ val2.intvalue));
                else if (expr->str() == "==")
                    result->emplace_back(ValueFlow::Value(val1.intvalue == val2.intvalue));
                else if (expr->str() == "!=")
                    result->emplace_back(ValueFlow::Value(val1.intvalue != val2.intvalue));
                else if (expr->str() == "<")
                    result->emplace_back(ValueFlow::Value(val1.intvalue < val2.intvalue));
                else if (expr->str() == ">")
                    result->emplace_back(ValueFlow::Value(val1.intvalue > val2.intvalue));
                else if (expr->str() == ">=")
                    result->emplace_back(ValueFlow::Value(val1.intvalue >= val2.intvalue));
                else if (expr->str() == "<=")
                    result->emplace_back(ValueFlow::Value(val1.intvalue <= val2.intvalue));
                else if (expr->str() == "&&")
                    result->emplace_back(ValueFlow::Value(val1.intvalue && val2.intvalue));
                else if (expr->str() == "||")
                    result->emplace_back(ValueFlow::Value(val1.intvalue || val2.intvalue));
                else if (expr->str() == "<<")
                    result->emplace_back(ValueFlow::Value(val1.intvalue << val2.intvalue));
                else if (expr->str() == ">>")
                    result->emplace_back(ValueFlow::Value(val1.intvalue >> val2.intvalue));
                else
                    return false;
                combineValueProperties(val1, val2, &result->back());
            }
        }
        return !result->empty();
    }
    if (expr->str().compare(0,3,"arg")==0) {
        *result = values[expr->str()[3] - '1'];
        return true;
    }
    if (expr->isNumber()) {
        result->emplace_back(ValueFlow::Value(MathLib::toLongNumber(expr->str())));
        result->back().setKnown();
        return true;
    } else if (expr->tokType() == Token::eChar) {
        result->emplace_back(ValueFlow::Value(MathLib::toLongNumber(expr->str())));
        result->back().setKnown();
        return true;
    }
    return false;
}

static std::list<ValueFlow::Value> getFunctionArgumentValues(const Token *argtok)
{
    std::list<ValueFlow::Value> argvalues(argtok->values());
    removeImpossible(argvalues);
    if (argvalues.empty() && Token::Match(argtok, "%comp%|%oror%|&&|!")) {
        argvalues.emplace_back(0);
        argvalues.emplace_back(1);
    }
    return argvalues;
}

static void valueFlowLibraryFunction(Token *tok, const std::string &returnValue, const Settings *settings)
{
    std::vector<std::list<ValueFlow::Value>> argValues;
    for (const Token *argtok : getArguments(tok->previous())) {
        argValues.emplace_back(getFunctionArgumentValues(argtok));
        if (argValues.back().empty())
            return;
    }
    if (returnValue.find("arg") != std::string::npos && argValues.empty())
        return;

    TokenList tokenList(settings);
    {
        const std::string code = "return " + returnValue + ";";
        std::istringstream istr(code);
        if (!tokenList.createTokens(istr))
            return;
    }

    // combine operators, set links, etc..
    std::stack<Token *> lpar;
    for (Token *tok2 = tokenList.front(); tok2; tok2 = tok2->next()) {
        if (Token::Match(tok2, "[!<>=] =")) {
            tok2->str(tok2->str() + "=");
            tok2->deleteNext();
        } else if (tok2->str() == "(")
            lpar.push(tok2);
        else if (tok2->str() == ")") {
            if (lpar.empty())
                return;
            Token::createMutualLinks(lpar.top(), tok2);
            lpar.pop();
        }
    }
    if (!lpar.empty())
        return;

    // Evaluate expression
    tokenList.createAst();
    std::list<ValueFlow::Value> results;
    if (evaluate(tokenList.front()->astOperand1(), argValues, &results))
        setTokenValues(tok, results, settings);
}

static void valueFlowSubFunction(TokenList* tokenlist, SymbolDatabase* symboldatabase,  ErrorLogger* errorLogger, const Settings* settings)
{
    for (const Scope* scope : symboldatabase->functionScopes) {
        const Function* function = scope->function;
        if (!function)
            continue;
        int id = 0;
        for (const Token *tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "%name% ("))
                continue;

            const Function * const calledFunction = tok->function();
            if (!calledFunction) {
                // library function?
                const std::string& returnValue(settings->library.returnValue(tok));
                if (!returnValue.empty())
                    valueFlowLibraryFunction(tok->next(), returnValue, settings);
                continue;
            }

            const Scope * const calledFunctionScope = calledFunction->functionScope;
            if (!calledFunctionScope)
                continue;

            id++;
            std::unordered_map<const Variable*, std::list<ValueFlow::Value>> argvars;
            // TODO: Rewrite this. It does not work well to inject 1 argument at a time.
            const std::vector<const Token *> &callArguments = getArguments(tok);
            for (int argnr = 0U; argnr < callArguments.size(); ++argnr) {
                const Token *argtok = callArguments[argnr];
                // Get function argument
                const Variable * const argvar = calledFunction->getArgumentVar(argnr);
                if (!argvar)
                    break;

                // passing value(s) to function
                std::list<ValueFlow::Value> argvalues(getFunctionArgumentValues(argtok));

                // Remove non-local lifetimes
                argvalues.remove_if([](const ValueFlow::Value& v) {
                    if (v.isLifetimeValue())
                        return !v.isLocalLifetimeValue() && !v.isSubFunctionLifetimeValue();
                    return false;
                });
                // Don't forward container sizes for now since programmemory can't evaluate conditions
                argvalues.remove_if(std::mem_fn(&ValueFlow::Value::isContainerSizeValue));

                if (argvalues.empty())
                    continue;

                // Error path..
                for (ValueFlow::Value &v : argvalues) {
                    const std::string nr = MathLib::toString(argnr + 1) + getOrdinalText(argnr + 1);

                    v.errorPath.emplace_back(argtok,
                                             "Calling function '" +
                                             calledFunction->name() +
                                             "', " +
                                             nr +
                                             " argument '" +
                                             argtok->expressionString() +
                                             "' value is " +
                                             v.infoString());
                    v.path = 256 * v.path + id;
                    // Change scope of lifetime values
                    if (v.isLifetimeValue())
                        v.lifetimeScope = ValueFlow::Value::LifetimeScope::SubFunction;
                }

                // passed values are not "known"..
                lowerToPossible(argvalues);

                argvars[argvar] = argvalues;
            }
            valueFlowInjectParameter(tokenlist, errorLogger, settings, calledFunctionScope, argvars);
        }
    }
}

static void valueFlowFunctionDefaultParameter(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    if (!tokenlist->isCPP())
        return;

    for (const Scope* scope : symboldatabase->functionScopes) {
        const Function* function = scope->function;
        if (!function)
            continue;
        for (std::size_t arg = function->minArgCount(); arg < function->argCount(); arg++) {
            const Variable* var = function->getArgumentVar(arg);
            if (var && var->hasDefault() && Token::Match(var->nameToken(), "%var% = %num%|%str% [,)]")) {
                const std::list<ValueFlow::Value> &values = var->nameToken()->tokAt(2)->values();
                std::list<ValueFlow::Value> argvalues;
                for (const ValueFlow::Value &value : values) {
                    ValueFlow::Value v(value);
                    v.defaultArg = true;
                    v.changeKnownToPossible();
                    if (v.isPossible())
                        argvalues.push_back(v);
                }
                if (!argvalues.empty())
                    valueFlowInjectParameter(tokenlist, errorLogger, settings, var, scope, argvalues);
            }
        }
    }
}

static bool isKnown(const Token * tok)
{
    return tok && tok->hasKnownIntValue();
}

static void valueFlowFunctionReturn(TokenList *tokenlist, ErrorLogger *errorLogger)
{
    for (Token *tok = tokenlist->back(); tok; tok = tok->previous()) {
        if (tok->str() != "(" || !tok->astOperand1() || !tok->astOperand1()->function())
            continue;

        if (tok->hasKnownValue())
            continue;

        // Arguments..
        std::vector<MathLib::bigint> parvalues;
        if (tok->astOperand2()) {
            const Token *partok = tok->astOperand2();
            while (partok && partok->str() == "," && isKnown(partok->astOperand2()))
                partok = partok->astOperand1();
            if (!isKnown(partok))
                continue;
            parvalues.push_back(partok->values().front().intvalue);
            partok = partok->astParent();
            while (partok && partok->str() == ",") {
                parvalues.push_back(partok->astOperand2()->values().front().intvalue);
                partok = partok->astParent();
            }
            if (partok != tok)
                continue;
        }

        // Get scope and args of function
        const Function * const function = tok->astOperand1()->function();
        const Scope * const functionScope = function->functionScope;
        if (!functionScope || !Token::simpleMatch(functionScope->bodyStart, "{ return")) {
            if (functionScope && tokenlist->getSettings()->debugwarnings && Token::findsimplematch(functionScope->bodyStart, "return", functionScope->bodyEnd))
                bailout(tokenlist, errorLogger, tok, "function return; nontrivial function body");
            continue;
        }

        ProgramMemory programMemory;
        for (std::size_t i = 0; i < parvalues.size(); ++i) {
            const Variable * const arg = function->getArgumentVar(i);
            if (!arg || !Token::Match(arg->typeStartToken(), "%type% %name% ,|)")) {
                if (tokenlist->getSettings()->debugwarnings)
                    bailout(tokenlist, errorLogger, tok, "function return; unhandled argument type");
                programMemory.clear();
                break;
            }
            programMemory.setIntValue(arg->declarationId(), parvalues[i]);
        }
        if (programMemory.empty() && !parvalues.empty())
            continue;

        // Determine return value of subfunction..
        MathLib::bigint result = 0;
        bool error = false;
        execute(functionScope->bodyStart->next()->astOperand1(),
                &programMemory,
                &result,
                &error);
        if (!error) {
            ValueFlow::Value v(result);
            if (function->hasVirtualSpecifier())
                v.setPossible();
            else
                v.setKnown();
            setTokenValue(tok, v, tokenlist->getSettings());
        }
    }
}

static void valueFlowUninit(TokenList *tokenlist, SymbolDatabase * /*symbolDatabase*/, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!Token::Match(tok,"[;{}] %type%"))
            continue;
        if (!tok->scope()->isExecutable())
            continue;
        const Token *vardecl = tok->next();
        bool stdtype = false;
        bool pointer = false;
        while (Token::Match(vardecl, "%name%|::|*") && vardecl->varId() == 0) {
            stdtype |= vardecl->isStandardType();
            pointer |= vardecl->str() == "*";
            vardecl = vardecl->next();
        }
        // if (!stdtype && !pointer)
        // continue;
        if (!Token::Match(vardecl, "%var% ;"))
            continue;
        const Variable *var = vardecl->variable();
        if (!var || var->nameToken() != vardecl || var->isInit())
            continue;
        if ((!var->isPointer() && var->type() && var->type()->needInitialization != Type::NeedInitialization::True) ||
            !var->isLocal() || var->isStatic() || var->isExtern() || var->isReference() || var->isThrow())
            continue;
        if (!var->type() && !stdtype && !pointer)
            continue;

        ValueFlow::Value uninitValue;
        uninitValue.setKnown();
        uninitValue.valueType = ValueFlow::Value::UNINIT;
        uninitValue.tokvalue = vardecl;
        std::list<ValueFlow::Value> values;
        values.push_back(uninitValue);

        const bool constValue = true;
        const bool subFunction = false;

        valueFlowForwardVariable(vardecl->next(),
                                 vardecl->scope()->bodyEnd,
                                 var,
                                 vardecl->varId(),
                                 values,
                                 constValue,
                                 subFunction,
                                 tokenlist,
                                 errorLogger,
                                 settings);
    }
}

static bool hasContainerSizeGuard(const Token *tok, nonneg int containerId)
{
    for (; tok && tok->astParent(); tok = tok->astParent()) {
        const Token *parent = tok->astParent();
        if (tok != parent->astOperand2())
            continue;
        if (!Token::Match(parent, "%oror%|&&|?"))
            continue;
        // is container found in lhs?
        bool found = false;
        visitAstNodes(parent->astOperand1(),
        [&](const Token *t) {
            if (t->varId() == containerId)
                found = true;
            return found ? ChildrenToVisit::done : ChildrenToVisit::op1_and_op2;
        });
        if (found)
            return true;
    }
    return false;
}

static bool isContainerSize(const Token* tok)
{
    if (!Token::Match(tok, "%var% . %name% ("))
        return false;
    if (!astIsContainer(tok))
        return false;
    if (tok->valueType()->container && tok->valueType()->container->getYield(tok->strAt(2)) == Library::Container::Yield::SIZE)
        return true;
    if (Token::Match(tok->tokAt(2), "size|length ( )"))
        return true;
    return false;
}

static bool isContainerEmpty(const Token* tok)
{
    if (!Token::Match(tok, "%var% . %name% ("))
        return false;
    if (!astIsContainer(tok))
        return false;
    if (tok->valueType()->container && tok->valueType()->container->getYield(tok->strAt(2)) == Library::Container::Yield::EMPTY)
        return true;
    if (Token::simpleMatch(tok->tokAt(2), "empty ( )"))
        return true;
    return false;
}
static bool isContainerSizeChanged(const Token *tok, int depth=20);

static bool isContainerSizeChanged(nonneg int varId, const Token *start, const Token *end, int depth = 20);

static bool isContainerSizeChangedByFunction(const Token *tok, int depth = 20)
{
    if (!tok->valueType() || !tok->valueType()->container)
        return false;
    // If we are accessing an element then we are not changing the container size
    if (Token::Match(tok, "%name% . %name% (")) {
        Library::Container::Yield yield = tok->valueType()->container->getYield(tok->strAt(2));
        if (yield != Library::Container::Yield::NO_YIELD)
            return false;
    }
    if (Token::simpleMatch(tok->astParent(), "["))
        return false;

    // address of variable
    const bool addressOf = tok->valueType()->pointer || (tok->astParent() && tok->astParent()->isUnaryOp("&"));

    int narg;
    const Token * ftok = getTokenArgumentFunction(tok, narg);
    if (!ftok)
        return false; // not a function => variable not changed
    const Function * fun = ftok->function();
    if (fun && !fun->hasVirtualSpecifier()) {
        const Variable *arg = fun->getArgumentVar(narg);
        if (arg) {
            if (!arg->isReference() && !addressOf)
                return false;
            if (!addressOf && arg->isConst())
                return false;
            if (arg->valueType() && arg->valueType()->constness == 1)
                return false;
            const Scope * scope = fun->functionScope;
            if (scope) {
                // Argument not used
                if (!arg->nameToken())
                    return false;
                if (depth > 0)
                    return isContainerSizeChanged(arg->declarationId(), scope->bodyStart, scope->bodyEnd, depth - 1);
            }
            // Don't know => Safe guess
            return true;
        }
    }

    bool inconclusive = false;
    const bool isChanged = isVariableChangedByFunctionCall(tok, 0, nullptr, &inconclusive);
    return (isChanged || inconclusive);
}

static void valueFlowContainerReverse(Token *tok, nonneg int containerId, const ValueFlow::Value &value, const Settings *settings)
{
    while (nullptr != (tok = tok->previous())) {
        if (Token::Match(tok, "[{}]"))
            break;
        if (Token::Match(tok, "return|break|continue"))
            break;
        if (tok->varId() != containerId)
            continue;
        if (Token::Match(tok, "%name% ="))
            break;
        if (isContainerSizeChangedByFunction(tok))
            break;
        if (!tok->valueType() || !tok->valueType()->container)
            break;
        if (Token::Match(tok, "%name% . %name% (") && tok->valueType()->container->getAction(tok->strAt(2)) != Library::Container::Action::NO_ACTION)
            break;
        if (!hasContainerSizeGuard(tok, containerId))
            setTokenValue(tok, value, settings);
    }
}

struct ContainerVariableAnalyzer : VariableAnalyzer {
    ContainerVariableAnalyzer() : VariableAnalyzer() {}

    ContainerVariableAnalyzer(const Variable* v,
                              const ValueFlow::Value& val,
                              std::vector<const Variable*> paliases,
                              const TokenList* t)
        : VariableAnalyzer(v, val, std::move(paliases), t)
    {}

    virtual bool match(const Token* tok) const OVERRIDE {
        return tok->varId() == var->declarationId() || (astIsIterator(tok) && isAliasOf(tok, var->declarationId()));
    }

    virtual Action isWritable(const Token* tok, Direction d) const OVERRIDE {
        if (astIsIterator(tok))
            return Action::None;
        if (d == Direction::Reverse)
            return Action::None;
        const ValueFlow::Value* value = getValue(tok);
        if (!value)
            return Action::None;
        if (!tok->valueType() || !tok->valueType()->container)
            return Action::None;
        const Token* parent = tok->astParent();

        if (tok->valueType()->container->stdStringLike && Token::simpleMatch(parent, "+=") && astIsLHS(tok) && parent->astOperand2()) {
            const Token* rhs = parent->astOperand2();
            if (rhs->tokType() == Token::eString)
                return Action::Read | Action::Write;
            if (rhs->valueType() && rhs->valueType()->container && rhs->valueType()->container->stdStringLike) {
                if (std::any_of(rhs->values().begin(), rhs->values().end(), [&](const ValueFlow::Value &rhsval) {
                return rhsval.isKnown() && rhsval.isContainerSizeValue();
                }))
                return Action::Read | Action::Write;
            }
        } else if (Token::Match(tok, "%name% . %name% (")) {
            Library::Container::Action action = tok->valueType()->container->getAction(tok->strAt(2));
            if (action == Library::Container::Action::PUSH || action == Library::Container::Action::POP)
                return Action::Read | Action::Write;
        }
        return Action::None;
    }

    virtual void writeValue(ValueFlow::Value* value, const Token* tok, Direction d) const OVERRIDE {
        if (d == Direction::Reverse)
            return;
        if (!value)
            return;
        if (!tok->astParent())
            return;
        const Token* parent = tok->astParent();
        if (!tok->valueType() || !tok->valueType()->container)
            return;

        if (tok->valueType()->container->stdStringLike && Token::simpleMatch(parent, "+=") && parent->astOperand2()) {
            const Token* rhs = parent->astOperand2();
            if (rhs->tokType() == Token::eString)
                value->intvalue += Token::getStrLength(rhs);
            else if (rhs->valueType() && rhs->valueType()->container && rhs->valueType()->container->stdStringLike) {
                for (const ValueFlow::Value &rhsval : rhs->values()) {
                    if (rhsval.isKnown() && rhsval.isContainerSizeValue()) {
                        value->intvalue += rhsval.intvalue;
                    }
                }
            }
        } else if (Token::Match(tok, "%name% . %name% (")) {
            Library::Container::Action action = tok->valueType()->container->getAction(tok->strAt(2));
            if (action == Library::Container::Action::PUSH)
                value->intvalue++;
            if (action == Library::Container::Action::POP)
                value->intvalue--;
        }
    }

    virtual Action isModified(const Token* tok) const OVERRIDE {
        Action read = Action::Read;
        // An iterator won't change the container size
        if (astIsIterator(tok))
            return read;
        if (Token::Match(tok->astParent(), "%assign%") && astIsLHS(tok))
            return Action::Invalid;
        if (isLikelyStreamRead(isCPP(), tok->astParent()))
            return Action::Invalid;
        if (isContainerSizeChanged(tok))
            return Action::Invalid;
        return read;
    }
};

static Analyzer::Action valueFlowContainerForward(Token* tok,
        const Token* endToken,
        const Variable* var,
        ValueFlow::Value value,
        TokenList* tokenlist)
{
    ContainerVariableAnalyzer a(var, value, getAliasesFromValues({value}), tokenlist);
    return valueFlowGenericForward(tok, endToken, a, tokenlist->getSettings());
}
static Analyzer::Action valueFlowContainerForward(Token* tok,
        const Variable* var,
        ValueFlow::Value value,
        TokenList* tokenlist)
{
    const Token * endOfVarScope = nullptr;
    if (var->isLocal() || var->isArgument())
        endOfVarScope = var->scope()->bodyEnd;
    if (!endOfVarScope)
        endOfVarScope = tok->scope()->bodyEnd;
    return valueFlowContainerForward(tok, endOfVarScope, var, std::move(value), tokenlist);
}

static bool isContainerSizeChanged(const Token *tok, int depth)
{
    if (!tok)
        return false;
    if (!tok->valueType() || !tok->valueType()->container)
        return true;
    if (Token::Match(tok, "%name% %assign%|<<"))
        return true;
    if (Token::Match(tok, "%var% [") && tok->valueType()->container->stdAssociativeLike)
        return true;
    if (Token::Match(tok, "%name% . %name% (")) {
        Library::Container::Action action = tok->valueType()->container->getAction(tok->strAt(2));
        Library::Container::Yield yield = tok->valueType()->container->getYield(tok->strAt(2));
        switch (action) {
        case Library::Container::Action::RESIZE:
        case Library::Container::Action::CLEAR:
        case Library::Container::Action::PUSH:
        case Library::Container::Action::POP:
        case Library::Container::Action::CHANGE:
        case Library::Container::Action::INSERT:
        case Library::Container::Action::ERASE:
        case Library::Container::Action::CHANGE_INTERNAL:
            return true;
        case Library::Container::Action::NO_ACTION: // might be unknown action
            return yield == Library::Container::Yield::NO_YIELD;
        case Library::Container::Action::FIND:
        case Library::Container::Action::CHANGE_CONTENT:
            break;
        }
    }
    if (isContainerSizeChangedByFunction(tok, depth))
        return true;
    return false;
}

static bool isContainerSizeChanged(nonneg int varId, const Token *start, const Token *end, int depth)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() != varId)
            continue;
        if (isContainerSizeChanged(tok, depth))
            return true;
    }
    return false;
}

static void valueFlowSmartPointer(TokenList *tokenlist, ErrorLogger * errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->scope())
            continue;
        if (!tok->scope()->isExecutable())
            continue;
        if (tok->variable()) {
            const Variable* var = tok->variable();
            if (!var->isSmartPointer())
                continue;
            if (var->nameToken() == tok) {
                if (Token::Match(tok, "%var% (|{") && tok->next()->astOperand2() &&
                    tok->next()->astOperand2()->str() != ",") {
                    Token* inTok = tok->next()->astOperand2();
                    std::list<ValueFlow::Value> values = inTok->values();
                    const bool constValue = inTok->isNumber();
                    valueFlowForwardAssign(inTok, var, values, constValue, true, tokenlist, errorLogger, settings);

                } else if (Token::Match(tok, "%var% ;")) {
                    std::list<ValueFlow::Value> values;
                    ValueFlow::Value v(0);
                    v.setKnown();
                    values.push_back(v);
                    valueFlowForwardAssign(tok, var, values, false, true, tokenlist, errorLogger, settings);
                }
            } else if (Token::Match(tok, "%var% . reset (") && tok->next()->originalName() != "->") {
                if (Token::simpleMatch(tok->tokAt(3), "( )")) {
                    std::list<ValueFlow::Value> values;
                    ValueFlow::Value v(0);
                    v.setKnown();
                    values.push_back(v);
                    valueFlowForwardAssign(tok->tokAt(4), var, values, false, false, tokenlist, errorLogger, settings);
                } else {
                    tok->removeValues(std::mem_fn(&ValueFlow::Value::isIntValue));
                    Token* inTok = tok->tokAt(3)->astOperand2();
                    if (!inTok)
                        continue;
                    std::list<ValueFlow::Value> values = inTok->values();
                    const bool constValue = inTok->isNumber();
                    valueFlowForwardAssign(inTok, var, values, constValue, false, tokenlist, errorLogger, settings);
                }
            } else if (Token::Match(tok, "%var% . release ( )") && tok->next()->originalName() != "->") {
                std::list<ValueFlow::Value> values;
                ValueFlow::Value v(0);
                v.setKnown();
                values.push_back(v);
                valueFlowForwardAssign(tok->tokAt(4), var, values, false, false, tokenlist, errorLogger, settings);
            }
        } else if (Token::Match(tok->previous(), "%name%|> (|{") && astIsSmartPointer(tok) &&
                   astIsSmartPointer(tok->astOperand1())) {
            std::vector<const Token*> args = getArguments(tok);
            if (args.empty())
                continue;
            for (const ValueFlow::Value& v : args.front()->values())
                setTokenValue(tok, v, settings);
        }
    }
}

static void valueFlowIterators(TokenList *tokenlist, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->scope())
            continue;
        if (!tok->scope()->isExecutable())
            continue;
        if (!astIsContainer(tok))
            continue;
        if (Token::Match(tok->astParent(), ". %name% (")) {
            Library::Container::Yield yield = tok->valueType()->container->getYield(tok->astParent()->strAt(1));
            ValueFlow::Value v(0);
            v.setKnown();
            if (yield == Library::Container::Yield::START_ITERATOR) {
                v.valueType = ValueFlow::Value::ITERATOR_START;
                setTokenValue(tok->astParent()->tokAt(2), v, settings);
            } else if (yield == Library::Container::Yield::END_ITERATOR) {
                v.valueType = ValueFlow::Value::ITERATOR_END;
                setTokenValue(tok->astParent()->tokAt(2), v, settings);
            }
        }
    }
}

static std::list<ValueFlow::Value> getIteratorValues(std::list<ValueFlow::Value> values, ValueFlow::Value::ValueKind* kind = nullptr)
{
    values.remove_if([&](const ValueFlow::Value& v) {
        if (kind && v.valueKind != *kind)
            return true;
        return !v.isIteratorValue();
    });
    return values;
}

struct IteratorConditionHandler : SimpleConditionHandler {
    virtual Condition parse(const Token* tok, const Settings*) const OVERRIDE {
        Condition cond;

        ValueFlow::Value true_value;
        ValueFlow::Value false_value;

        if (Token::Match(tok, "==|!=")) {
            if (!tok->astOperand1() || !tok->astOperand2())
                return cond;

            ValueFlow::Value::ValueKind kind = ValueFlow::Value::ValueKind::Known;
            std::list<ValueFlow::Value> values = getIteratorValues(tok->astOperand1()->values(), &kind);
            if (!values.empty()) {
                cond.vartok = tok->astOperand2();
            } else {
                values = getIteratorValues(tok->astOperand2()->values());
                if (!values.empty())
                    cond.vartok = tok->astOperand1();
            }
            for (ValueFlow::Value& v:values) {
                v.setPossible();
                v.assumeCondition(tok);
            }
            cond.true_values = values;
            cond.false_values = values;
        }

        return cond;
    }
};

static void valueFlowIteratorInfer(TokenList *tokenlist, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->scope())
            continue;
        if (!tok->scope()->isExecutable())
            continue;
        std::list<ValueFlow::Value> values = getIteratorValues(tok->values());
        values.remove_if([&](const ValueFlow::Value& v) {
            if (!v.isImpossible())
                return true;
            if (v.isIteratorEndValue() && v.intvalue <= 0)
                return true;
            if (v.isIteratorStartValue() && v.intvalue >= 0)
                return true;
            return false;
        });
        for (ValueFlow::Value& v:values) {
            v.setPossible();
            if (v.isIteratorStartValue())
                v.intvalue++;
            if (v.isIteratorEndValue())
                v.intvalue--;
            setTokenValue(tok, v, settings);
        }
    }
}

static std::vector<ValueFlow::Value> getInitListSize(const Token* tok, const Library::Container *container)
{
    std::vector<const Token*> args = getArguments(tok);
    if ((args.size() == 1 && astIsContainer(args[0]) && args[0]->valueType()->container == container) ||
        (args.size() == 2 && astIsIterator(args[0]) && astIsIterator(args[1]))) {
        std::vector<ValueFlow::Value> values;
        std::copy_if(args[0]->values().begin(),
                     args[0]->values().end(),
                     std::back_inserter(values),
                     std::mem_fn(&ValueFlow::Value::isContainerSizeValue));
        return values;
    }
    ValueFlow::Value value(args.size());
    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
    value.setKnown();
    return {value};
}

static void valueFlowContainerSize(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger * /*errorLogger*/, const Settings *settings)
{
    // declaration
    for (const Variable *var : symboldatabase->variableList()) {
        if (!var || !var->isLocal() || var->isPointer() || var->isReference() || var->isStatic())
            continue;
        if (!var->valueType() || !var->valueType()->container)
            continue;
        if (!astIsContainer(var->nameToken()))
            continue;
        if (var->nameToken()->hasKnownValue())
            continue;
        if (!Token::Match(var->nameToken(), "%name% ;") &&
            !(Token::Match(var->nameToken(), "%name% {") && Token::simpleMatch(var->nameToken()->next()->link(), "} ;")))
            continue;
        std::vector<ValueFlow::Value> values{ValueFlow::Value{0}};
        values.back().valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
        values.back().setKnown();
        if (var->valueType()->container->size_templateArgNo >= 0) {
            if (var->dimensions().size() == 1 && var->dimensions().front().known)
                values.back().intvalue = var->dimensions().front().num;
            else
                continue;
        } else if (Token::simpleMatch(var->nameToken()->next(), "{")) {
            const Token* initList = var->nameToken()->next();
            values = getInitListSize(initList, var->valueType()->container);
        }
        for (const ValueFlow::Value& value : values)
            valueFlowContainerForward(var->nameToken()->next(), var, value, tokenlist);
    }

    // after assignment
    for (const Scope *functionScope : symboldatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%name%|;|{|} %var% = %str% ;")) {
                const Token *containerTok = tok->next();
                if (containerTok && containerTok->valueType() && containerTok->valueType()->container && containerTok->valueType()->container->stdStringLike) {
                    ValueFlow::Value value(Token::getStrLength(containerTok->tokAt(2)));
                    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    value.setKnown();
                    valueFlowContainerForward(containerTok->next(), containerTok->variable(), value, tokenlist);
                }
            } else if (Token::Match(tok, "%name%|;|{|}|> %var% = {") && Token::simpleMatch(tok->linkAt(3), "} ;")) {
                const Token* containerTok = tok->next();
                if (astIsContainer(containerTok)) {
                    std::vector<ValueFlow::Value> values = getInitListSize(tok->tokAt(3), containerTok->valueType()->container);
                    for (const ValueFlow::Value& value : values)
                        valueFlowContainerForward(containerTok->next(), containerTok->variable(), value, tokenlist);
                }
            } else if (Token::Match(tok, "%var% . %name% (") && tok->valueType() && tok->valueType()->container) {
                Library::Container::Action action = tok->valueType()->container->getAction(tok->strAt(2));
                if (action == Library::Container::Action::CLEAR) {
                    ValueFlow::Value value(0);
                    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    value.setKnown();
                    valueFlowContainerForward(tok->next(), tok->variable(), value, tokenlist);
                } else if (action == Library::Container::Action::RESIZE && tok->tokAt(4)->hasKnownIntValue()) {
                    ValueFlow::Value value(tok->tokAt(4)->values().front());
                    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    value.setKnown();
                    valueFlowContainerForward(tok->next(), tok->variable(), value, tokenlist);
                }
            }
        }
    }

    // conditional conditionSize
    for (const Scope &scope : symboldatabase->scopeList) {
        if (scope.type != Scope::ScopeType::eIf) // TODO: while
            continue;
        for (const Token *tok = scope.classDef; tok && tok->str() != "{"; tok = tok->next()) {
            if (!tok->isName() || !tok->valueType() || tok->valueType()->type != ValueType::CONTAINER || !tok->valueType()->container)
                continue;

            const Token *conditionToken;
            MathLib::bigint intval;

            if (Token::Match(tok, "%name% . %name% (")) {
                if (tok->valueType()->container->getYield(tok->strAt(2)) == Library::Container::Yield::SIZE) {
                    const Token *parent = tok->tokAt(3)->astParent();
                    if (!parent || !parent->isComparisonOp() || !parent->astOperand2())
                        continue;
                    if (parent->astOperand1()->hasKnownIntValue())
                        intval = parent->astOperand1()->values().front().intvalue;
                    else if (parent->astOperand2()->hasKnownIntValue())
                        intval = parent->astOperand2()->values().front().intvalue;
                    else
                        continue;
                    conditionToken = parent;
                } else if (tok->valueType()->container->getYield(tok->strAt(2)) == Library::Container::Yield::EMPTY) {
                    conditionToken = tok->tokAt(3);
                    intval = 0;
                } else {
                    continue;
                }
            } else if (tok->valueType()->container->stdStringLike && Token::Match(tok, "%name% ==|!= %str%") && tok->next()->astOperand2() == tok->tokAt(2)) {
                intval = Token::getStrLength(tok->tokAt(2));
                conditionToken = tok->next();
            } else {
                continue;
            }

            ValueFlow::Value value(conditionToken, intval);
            value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;

            // possible value before condition
            valueFlowContainerReverse(const_cast<Token *>(scope.classDef), tok->varId(), value, settings);
        }
    }
}

struct ContainerConditionHandler : ConditionHandler {
    virtual bool forward(Token* start,
                         const Token* stop,
                         const Token* exprTok,
                         const std::list<ValueFlow::Value>& values,
                         TokenList* tokenlist,
                         const Settings*) const OVERRIDE {
        // TODO: Forward multiple values
        if (values.empty())
            return false;
        const Variable* var = exprTok->variable();
        if (!var)
            return false;
        return valueFlowContainerForward(start->next(), stop, var, values.front(), tokenlist).isModified();
    }

    virtual Condition parse(const Token* tok, const Settings*) const OVERRIDE {
        Condition cond;
        ValueFlow::Value true_value;
        ValueFlow::Value false_value;
        const Token *vartok = parseCompareInt(tok, true_value, false_value);
        if (vartok) {
            vartok = vartok->tokAt(-3);
            if (!isContainerSize(vartok))
                return cond;
            true_value.valueType = ValueFlow::Value::CONTAINER_SIZE;
            false_value.valueType = ValueFlow::Value::CONTAINER_SIZE;
            cond.true_values.push_back(true_value);
            cond.false_values.push_back(false_value);
            cond.vartok = vartok;
            return cond;
        }

        // Empty check
        if (tok->str() == "(") {
            vartok = tok->tokAt(-3);
            // TODO: Handle .size()
            if (!isContainerEmpty(vartok))
                return cond;
            const Token *parent = tok->astParent();
            while (parent) {
                if (Token::Match(parent, "%comp%"))
                    return cond;
                parent = parent->astParent();
            }
            ValueFlow::Value value(tok, 0LL);
            value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            cond.true_values.emplace_back(value);
            cond.false_values.emplace_back(std::move(value));
            cond.vartok = vartok;
            cond.inverted = true;
            return cond;
        }
        // String compare
        if (Token::Match(tok, "==|!=")) {
            const Token *strtok = nullptr;
            if (Token::Match(tok->astOperand1(), "%str%")) {
                strtok = tok->astOperand1();
                vartok = tok->astOperand2();
            } else if (Token::Match(tok->astOperand2(), "%str%")) {
                strtok = tok->astOperand2();
                vartok = tok->astOperand1();
            }
            if (!strtok)
                return cond;
            if (!astIsContainer(vartok))
                return cond;
            ValueFlow::Value value(tok, Token::getStrLength(strtok));
            value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            cond.false_values.emplace_back(value);
            cond.true_values.emplace_back(std::move(value));
            cond.vartok = vartok;
            return cond;
        }
        return cond;
    }
};

static void valueFlowFwdAnalysis(const TokenList *tokenlist, const Settings *settings)
{
    for (const Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "for ("))
            tok = tok->linkAt(1);
        if (tok->str() != "=" || !tok->astOperand1() || !tok->astOperand2())
            continue;
        // Skip variables
        if (tok->astOperand1()->variable())
            continue;
        if (!tok->scope()->isExecutable())
            continue;
        if (!tok->astOperand2()->hasKnownIntValue())
            continue;
        ValueFlow::Value v(tok->astOperand2()->values().front());
        v.errorPath.emplace_back(tok, tok->astOperand1()->expressionString() + " is assigned value " + MathLib::toString(v.intvalue));
        const Token *startToken = tok->findExpressionStartEndTokens().second->next();
        const Scope *functionScope = tok->scope();
        while (functionScope->nestedIn && functionScope->nestedIn->isExecutable())
            functionScope = functionScope->nestedIn;
        const Token *endToken = functionScope->bodyEnd;
        valueFlowForwardExpression(const_cast<Token*>(startToken), endToken, tok->astOperand1(), {v}, tokenlist, settings);
    }
}

static void valueFlowDynamicBufferSize(TokenList *tokenlist, SymbolDatabase *symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope *functionScope : symboldatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "[;{}] %var% ="))
                continue;

            if (!tok->next()->variable())
                continue;

            const Token *rhs = tok->tokAt(2)->astOperand2();
            while (rhs && rhs->isCast())
                rhs = rhs->astOperand2() ? rhs->astOperand2() : rhs->astOperand1();
            if (!rhs)
                continue;

            if (!Token::Match(rhs->previous(), "%name% ("))
                continue;

            const Library::AllocFunc *allocFunc = settings->library.getAllocFuncInfo(rhs->previous());
            if (!allocFunc)
                allocFunc = settings->library.getReallocFuncInfo(rhs->previous());
            if (!allocFunc || allocFunc->bufferSize == Library::AllocFunc::BufferSize::none)
                continue;

            const std::vector<const Token *> args = getArguments(rhs->previous());

            const Token * const arg1 = (args.size() >= allocFunc->bufferSizeArg1) ? args[allocFunc->bufferSizeArg1 - 1] : nullptr;
            const Token * const arg2 = (args.size() >= allocFunc->bufferSizeArg2) ? args[allocFunc->bufferSizeArg2 - 1] : nullptr;

            MathLib::bigint sizeValue = -1;
            switch (allocFunc->bufferSize) {
            case Library::AllocFunc::BufferSize::none:
                break;
            case Library::AllocFunc::BufferSize::malloc:
                if (arg1 && arg1->hasKnownIntValue())
                    sizeValue = arg1->getKnownIntValue();
                break;
            case Library::AllocFunc::BufferSize::calloc:
                if (arg1 && arg2 && arg1->hasKnownIntValue() && arg2->hasKnownIntValue())
                    sizeValue = arg1->getKnownIntValue() * arg2->getKnownIntValue();
                break;
            case Library::AllocFunc::BufferSize::strdup:
                if (arg1 && arg1->hasKnownValue()) {
                    const ValueFlow::Value &value = arg1->values().back();
                    if (value.isTokValue() && value.tokvalue->tokType() == Token::eString)
                        sizeValue = Token::getStrLength(value.tokvalue) + 1; // Add one for the null terminator
                }
                break;
            }
            if (sizeValue < 0)
                continue;

            ValueFlow::Value value(sizeValue);
            value.errorPath.emplace_back(tok->tokAt(2), "Assign " + tok->strAt(1) + ", buffer with size " + MathLib::toString(sizeValue));
            value.valueType = ValueFlow::Value::ValueType::BUFFER_SIZE;
            value.setKnown();
            const std::list<ValueFlow::Value> values{value};
            valueFlowForwardVariable(const_cast<Token*>(rhs),
                                     functionScope->bodyEnd,
                                     tok->next()->variable(),
                                     tok->next()->varId(),
                                     values,
                                     true,
                                     false,
                                     tokenlist,
                                     errorLogger,
                                     settings);
        }
    }
}

static bool getMinMaxValues(const ValueType *vt, const cppcheck::Platform &platform, MathLib::bigint *minValue, MathLib::bigint *maxValue)
{
    if (!vt || !vt->isIntegral() || vt->pointer)
        return false;

    int bits;
    switch (vt->type) {
    case ValueType::Type::BOOL:
        bits = 1;
        break;
    case ValueType::Type::CHAR:
        bits = platform.char_bit;
        break;
    case ValueType::Type::SHORT:
        bits = platform.short_bit;
        break;
    case ValueType::Type::INT:
        bits = platform.int_bit;
        break;
    case ValueType::Type::LONG:
        bits = platform.long_bit;
        break;
    case ValueType::Type::LONGLONG:
        bits = platform.long_long_bit;
        break;
    default:
        return false;
    }

    if (bits == 1) {
        *minValue = 0;
        *maxValue = 1;
    } else if (bits < 62) {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            *minValue = 0;
            *maxValue = (1LL << bits) - 1;
        } else {
            *minValue = -(1LL << (bits - 1));
            *maxValue = (1LL << (bits - 1)) - 1;
        }
    } else if (bits == 64) {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            *minValue = 0;
            *maxValue = LLONG_MAX; // todo max unsigned value
        } else {
            *minValue = LLONG_MIN;
            *maxValue = LLONG_MAX;
        }
    } else {
        return false;
    }

    return true;
}

static bool getMinMaxValues(const std::string &typestr, const Settings *settings, MathLib::bigint *minvalue, MathLib::bigint *maxvalue)
{
    TokenList typeTokens(settings);
    std::istringstream istr(typestr+";");
    if (!typeTokens.createTokens(istr))
        return false;
    typeTokens.simplifyPlatformTypes();
    typeTokens.simplifyStdType();
    const ValueType &vt = ValueType::parseDecl(typeTokens.front(), settings);
    return getMinMaxValues(&vt, *settings, minvalue, maxvalue);
}

static void valueFlowSafeFunctions(TokenList *tokenlist, SymbolDatabase *symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope *functionScope : symboldatabase->functionScopes) {
        if (!functionScope->bodyStart)
            continue;
        const Function *function = functionScope->function;
        if (!function)
            continue;

        const bool safe = function->isSafe(settings);
        const bool all = safe && settings->platformType != cppcheck::Platform::PlatformType::Unspecified;

        for (const Variable &arg : function->argumentList) {
            if (!arg.nameToken() || !arg.valueType())
                continue;

            if (arg.valueType()->type == ValueType::Type::CONTAINER) {
                if (!safe)
                    continue;
                std::list<ValueFlow::Value> argValues;
                argValues.emplace_back(0);
                argValues.back().valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                argValues.back().errorPath.emplace_back(arg.nameToken(), "Assuming " + arg.name() + " is empty");
                argValues.back().safe = true;
                argValues.emplace_back(1000000);
                argValues.back().valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                argValues.back().errorPath.emplace_back(arg.nameToken(), "Assuming " + arg.name() + " size is 1000000");
                argValues.back().safe = true;
                for (const ValueFlow::Value &value : argValues)
                    valueFlowContainerForward(const_cast<Token*>(functionScope->bodyStart), &arg, value, tokenlist);
                continue;
            }

            MathLib::bigint low, high;
            bool isLow = arg.nameToken()->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::LOW, &low);
            bool isHigh = arg.nameToken()->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::HIGH, &high);

            if (!isLow && !isHigh && !all)
                continue;

            const bool safeLow = !isLow;
            const bool safeHigh = !isHigh;

            if ((!isLow || !isHigh) && all) {
                MathLib::bigint minValue, maxValue;
                if (getMinMaxValues(arg.valueType(), *settings, &minValue, &maxValue)) {
                    if (!isLow)
                        low = minValue;
                    if (!isHigh)
                        high = maxValue;
                    isLow = isHigh = true;
                } else if (arg.valueType()->type == ValueType::Type::FLOAT || arg.valueType()->type == ValueType::Type::DOUBLE || arg.valueType()->type == ValueType::Type::LONGDOUBLE) {
                    std::list<ValueFlow::Value> argValues;
                    argValues.emplace_back(0);
                    argValues.back().valueType = ValueFlow::Value::ValueType::FLOAT;
                    argValues.back().floatValue = isLow ? low : -1E25f;
                    argValues.back().errorPath.emplace_back(arg.nameToken(), "Safe checks: Assuming argument has value " + MathLib::toString(argValues.back().floatValue));
                    argValues.back().safe = true;
                    argValues.emplace_back(0);
                    argValues.back().valueType = ValueFlow::Value::ValueType::FLOAT;
                    argValues.back().floatValue = isHigh ? high : 1E25f;
                    argValues.back().errorPath.emplace_back(arg.nameToken(), "Safe checks: Assuming argument has value " + MathLib::toString(argValues.back().floatValue));
                    argValues.back().safe = true;
                    valueFlowForwardVariable(const_cast<Token*>(functionScope->bodyStart->next()),
                                             functionScope->bodyEnd,
                                             &arg,
                                             arg.declarationId(),
                                             argValues,
                                             false,
                                             false,
                                             tokenlist,
                                             errorLogger,
                                             settings);
                    continue;
                }
            }

            std::list<ValueFlow::Value> argValues;
            if (isLow) {
                argValues.emplace_back(low);
                argValues.back().errorPath.emplace_back(arg.nameToken(), std::string(safeLow ? "Safe checks: " : "") + "Assuming argument has value " + MathLib::toString(low));
                argValues.back().safe = safeLow;
            }
            if (isHigh) {
                argValues.emplace_back(high);
                argValues.back().errorPath.emplace_back(arg.nameToken(), std::string(safeHigh ? "Safe checks: " : "") + "Assuming argument has value " + MathLib::toString(high));
                argValues.back().safe = safeHigh;
            }

            if (!argValues.empty())
                valueFlowForwardVariable(const_cast<Token*>(functionScope->bodyStart->next()),
                                         functionScope->bodyEnd,
                                         &arg,
                                         arg.declarationId(),
                                         argValues,
                                         false,
                                         false,
                                         tokenlist,
                                         errorLogger,
                                         settings);
        }
    }
}

static void valueFlowUnknownFunctionReturn(TokenList *tokenlist, const Settings *settings)
{
    if (settings->checkUnknownFunctionReturn.empty())
        return;
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->astParent() || tok->str() != "(" || !tok->previous()->isName())
            continue;
        if (settings->checkUnknownFunctionReturn.find(tok->previous()->str()) == settings->checkUnknownFunctionReturn.end())
            continue;
        std::vector<MathLib::bigint> unknownValues = settings->library.unknownReturnValues(tok->astOperand1());
        if (unknownValues.empty())
            continue;

        // Get min/max values for return type
        const std::string &typestr = settings->library.returnValueType(tok->previous());
        MathLib::bigint minvalue, maxvalue;
        if (!getMinMaxValues(typestr, settings, &minvalue, &maxvalue))
            continue;

        for (MathLib::bigint value : unknownValues) {
            if (value < minvalue)
                value = minvalue;
            else if (value > maxvalue)
                value = maxvalue;
            setTokenValue(const_cast<Token *>(tok), ValueFlow::Value(value), settings);
        }
    }
}

ValueFlow::Value::Value(const Token* c, long long val)
    : valueType(INT),
      bound(Bound::Point),
      intvalue(val),
      tokvalue(nullptr),
      floatValue(0.0),
      moveKind(MoveKind::NonMovedVariable),
      varvalue(val),
      condition(c),
      varId(0U),
      safe(false),
      conditional(false),
      defaultArg(false),
      indirect(0),
      path(0),
      lifetimeKind(LifetimeKind::Object),
      lifetimeScope(LifetimeScope::Local),
      valueKind(ValueKind::Possible)
{
    errorPath.emplace_back(c, "Assuming that condition '" + c->expressionString() + "' is not redundant");
}

void ValueFlow::Value::assumeCondition(const Token* tok)
{
    condition = tok;
    errorPath.emplace_back(tok, "Assuming that condition '" + tok->expressionString() + "' is not redundant");
}

std::string ValueFlow::Value::infoString() const
{
    switch (valueType) {
    case INT:
        return MathLib::toString(intvalue);
    case TOK:
        return tokvalue->str();
    case FLOAT:
        return MathLib::toString(floatValue);
    case MOVED:
        return "<Moved>";
    case UNINIT:
        return "<Uninit>";
    case BUFFER_SIZE:
    case CONTAINER_SIZE:
        return "size=" + MathLib::toString(intvalue);
    case ITERATOR_START:
        return "start=" + MathLib::toString(intvalue);
    case ITERATOR_END:
        return "end=" + MathLib::toString(intvalue);
    case LIFETIME:
        return "lifetime=" + tokvalue->str();
    }
    throw InternalError(nullptr, "Invalid ValueFlow Value type");
}

const char* ValueFlow::Value::toString(MoveKind moveKind)
{
    switch (moveKind) {
    case MoveKind::NonMovedVariable:
        return "NonMovedVariable";
    case MoveKind::MovedVariable:
        return "MovedVariable";
    case MoveKind::ForwardedVariable:
        return "ForwardedVariable";
    }
    return "";
}


const ValueFlow::Value *ValueFlow::valueFlowConstantFoldAST(Token *expr, const Settings *settings)
{
    if (expr && expr->values().empty()) {
        valueFlowConstantFoldAST(expr->astOperand1(), settings);
        valueFlowConstantFoldAST(expr->astOperand2(), settings);
        valueFlowSetConstantValue(expr, settings, true /* TODO: this is a guess */);
    }
    return expr && expr->hasKnownValue() ? &expr->values().front() : nullptr;
}

static std::size_t getTotalValues(TokenList *tokenlist)
{
    std::size_t n = 1;
    for (Token *tok = tokenlist->front(); tok; tok = tok->next())
        n += tok->values().size();
    return n;
}

void ValueFlow::setValues(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next())
        tok->clearValueFlow();

    valueFlowEnumValue(symboldatabase, settings);
    valueFlowNumber(tokenlist);
    valueFlowString(tokenlist);
    valueFlowArray(tokenlist);
    valueFlowUnknownFunctionReturn(tokenlist, settings);
    valueFlowGlobalConstVar(tokenlist, settings);
    valueFlowEnumValue(symboldatabase, settings);
    valueFlowNumber(tokenlist);
    valueFlowGlobalStaticVar(tokenlist, settings);
    valueFlowPointerAlias(tokenlist);
    valueFlowLifetime(tokenlist, symboldatabase, errorLogger, settings);
    valueFlowBitAnd(tokenlist);
    valueFlowSameExpressions(tokenlist);
    valueFlowFwdAnalysis(tokenlist, settings);

    std::size_t values = 0;
    std::size_t n = 4;
    while (n > 0 && values < getTotalValues(tokenlist)) {
        values = getTotalValues(tokenlist);
        valueFlowPointerAliasDeref(tokenlist);
        valueFlowArrayBool(tokenlist);
        valueFlowRightShift(tokenlist, settings);
        valueFlowOppositeCondition(symboldatabase, settings);
        valueFlowTerminatingCondition(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowBeforeCondition(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowAfterMove(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowCondition(SimpleConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings);
        valueFlowInferCondition(tokenlist, settings);
        valueFlowAfterAssign(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowSwitchVariable(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowForLoop(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowSubFunction(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowFunctionReturn(tokenlist, errorLogger);
        valueFlowLifetime(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowFunctionDefaultParameter(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowUninit(tokenlist, symboldatabase, errorLogger, settings);
        if (tokenlist->isCPP()) {
            valueFlowSmartPointer(tokenlist, errorLogger, settings);
            valueFlowIterators(tokenlist, settings);
            valueFlowCondition(IteratorConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings);
            valueFlowIteratorInfer(tokenlist, settings);
            valueFlowContainerSize(tokenlist, symboldatabase, errorLogger, settings);
            valueFlowCondition(ContainerConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings);
        }
        valueFlowSafeFunctions(tokenlist, symboldatabase, errorLogger, settings);
        n--;
    }

    valueFlowDynamicBufferSize(tokenlist, symboldatabase, errorLogger, settings);
}


std::string ValueFlow::eitherTheConditionIsRedundant(const Token *condition)
{
    if (!condition)
        return "Either the condition is redundant";
    if (condition->str() == "case") {
        std::string expr;
        for (const Token *tok = condition; tok && tok->str() != ":"; tok = tok->next()) {
            expr += tok->str();
            if (Token::Match(tok, "%name%|%num% %name%|%num%"))
                expr += ' ';
        }
        return "Either the switch case '" + expr + "' is redundant";
    }
    return "Either the condition '" + condition->expressionString() + "' is redundant";
}

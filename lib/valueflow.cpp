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
 * normal execution. The valueFlowForward is used in this analysis.
 *
 * In reverse value flow analysis we know the value of a variable at line X. And try to "execute backwards" to determine
 * possible values before line X. The valueFlowReverse is used in this analysis.
 *
 *
 */

#include "valueflow.h"

#include "analyzer.h"
#include "astutils.h"
#include "calculate.h"
#include "checkuninitvar.h"
#include "config.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "forwardanalyzer.h"
#include "infer.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "programmemory.h"
#include "reverseanalyzer.h"
#include "settings.h"
#include "sourcelocation.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueptr.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static void bailoutInternal(const std::string& type, TokenList *tokenlist, ErrorLogger *errorLogger, const Token *tok, const std::string &what, const std::string &file, int line, std::string function)
{
    if (function.find("operator") != std::string::npos)
        function = "(valueFlow)";
    std::list<ErrorMessage::FileLocation> callstack(1, ErrorMessage::FileLocation(tok, tokenlist));
    ErrorMessage errmsg(callstack, tokenlist->getSourceFilePath(), Severity::debug,
                        Path::stripDirectoryPart(file) + ":" + MathLib::toString(line) + ":" + function + " bailout: " + what, type, Certainty::normal);
    errorLogger->reportErr(errmsg);
}

#define bailout2(type, tokenlist, errorLogger, tok, what) bailoutInternal(type, tokenlist, errorLogger, tok, what, __FILE__, __LINE__, __func__)

#define bailout(tokenlist, errorLogger, tok, what) bailout2("valueFlowBailout", tokenlist, errorLogger, tok, what)

#define bailoutIncompleteVar(tokenlist, errorLogger, tok, what) bailout2("valueFlowBailoutIncompleteVar", tokenlist, errorLogger, tok, what)

static std::string debugString(const ValueFlow::Value& v)
{
    std::string kind;
    switch (v.valueKind) {

    case ValueFlow::Value::ValueKind::Impossible:
    case ValueFlow::Value::ValueKind::Known:
        kind = "always";
        break;
    case ValueFlow::Value::ValueKind::Inconclusive:
        kind = "inconclusive";
        break;
    case ValueFlow::Value::ValueKind::Possible:
        kind = "possible";
        break;
    }
    return kind + " " + v.toString();
}

static void setSourceLocation(ValueFlow::Value& v,
                              SourceLocation ctx,
                              const Token* tok,
                              SourceLocation local = SourceLocation::current())
{
    std::string file = ctx.file_name();
    if (file.empty())
        return;
    std::string s = Path::stripDirectoryPart(file) + ":" + MathLib::toString(ctx.line()) + ": " + ctx.function_name() +
                    " => " + local.function_name() + ": " + debugString(v);
    v.debugPath.emplace_back(tok, s);
}

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

static bool isNonConditionalPossibleIntValue(const ValueFlow::Value& v)
{
    if (v.conditional)
        return false;
    if (v.condition)
        return false;
    if (!v.isPossible())
        return false;
    if (!v.isIntValue())
        return false;
    return true;
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

static void setConditionalValue(ValueFlow::Value& value, const Token* tok, MathLib::bigint i)
{
    assert(value.isIntValue());
    value.intvalue = i;
    value.assumeCondition(tok);
    value.setPossible();
}

static void setConditionalValues(const Token* tok,
                                 bool lhs,
                                 MathLib::bigint value,
                                 ValueFlow::Value& true_value,
                                 ValueFlow::Value& false_value)
{
    if (Token::Match(tok, "==|!=|>=|<=")) {
        setConditionalValue(true_value, tok, value);
        const char* greaterThan = ">=";
        const char* lessThan = "<=";
        if (lhs)
            std::swap(greaterThan, lessThan);
        if (Token::simpleMatch(tok, greaterThan, strlen(greaterThan))) {
            setConditionalValue(false_value, tok, value - 1);
        } else if (Token::simpleMatch(tok, lessThan, strlen(lessThan))) {
            setConditionalValue(false_value, tok, value + 1);
        } else {
            setConditionalValue(false_value, tok, value);
        }
    } else {
        const char* greaterThan = ">";
        const char* lessThan = "<";
        if (lhs)
            std::swap(greaterThan, lessThan);
        if (Token::simpleMatch(tok, greaterThan, strlen(greaterThan))) {
            setConditionalValue(true_value, tok, value + 1);
            setConditionalValue(false_value, tok, value);
        } else if (Token::simpleMatch(tok, lessThan, strlen(lessThan))) {
            setConditionalValue(true_value, tok, value - 1);
            setConditionalValue(false_value, tok, value);
        }
    }
    setValueBound(true_value, tok, lhs);
    setValueBound(false_value, tok, !lhs);
}

static bool isSaturated(MathLib::bigint value)
{
    return value == std::numeric_limits<MathLib::bigint>::max() || value == std::numeric_limits<MathLib::bigint>::min();
}

static void parseCompareEachInt(
    const Token* tok,
    const std::function<void(const Token* varTok, ValueFlow::Value true_value, ValueFlow::Value false_value)>& each,
    const std::function<std::vector<ValueFlow::Value>(const Token*)>& evaluate)
{
    if (!tok->astOperand1() || !tok->astOperand2())
        return;
    if (tok->isComparisonOp()) {
        std::vector<ValueFlow::Value> value1 = evaluate(tok->astOperand1());
        std::vector<ValueFlow::Value> value2 = evaluate(tok->astOperand2());
        if (!value1.empty() && !value2.empty()) {
            if (tok->astOperand1()->hasKnownIntValue())
                value2.clear();
            if (tok->astOperand2()->hasKnownIntValue())
                value1.clear();
        }
        for (const ValueFlow::Value& v1 : value1) {
            ValueFlow::Value true_value = v1;
            ValueFlow::Value false_value = v1;
            if (isSaturated(v1.intvalue) || astIsFloat(tok->astOperand2(), /*unknown*/ false))
                continue;
            setConditionalValues(tok, true, v1.intvalue, true_value, false_value);
            each(tok->astOperand2(), std::move(true_value), std::move(false_value));
        }
        for (const ValueFlow::Value& v2 : value2) {
            ValueFlow::Value true_value = v2;
            ValueFlow::Value false_value = v2;
            if (isSaturated(v2.intvalue) || astIsFloat(tok->astOperand1(), /*unknown*/ false))
                continue;
            setConditionalValues(tok, false, v2.intvalue, true_value, false_value);
            each(tok->astOperand1(), std::move(true_value), std::move(false_value));
        }
    }
}

static void parseCompareEachInt(
    const Token* tok,
    const std::function<void(const Token* varTok, ValueFlow::Value true_value, ValueFlow::Value false_value)>& each)
{
    parseCompareEachInt(tok, each, [](const Token* t) -> std::vector<ValueFlow::Value> {
        if (t->hasKnownIntValue())
            return {t->values().front()};
        std::vector<ValueFlow::Value> result;
        std::copy_if(t->values().begin(), t->values().end(), std::back_inserter(result), [&](const ValueFlow::Value& v) {
            if (v.path < 1)
                return false;
            if (!isNonConditionalPossibleIntValue(v))
                return false;
            return true;
        });
        return result;
    });
}

const Token* parseCompareInt(const Token* tok,
                             ValueFlow::Value& true_value,
                             ValueFlow::Value& false_value,
                             const std::function<std::vector<MathLib::bigint>(const Token*)>& evaluate)
{
    const Token* result = nullptr;
    parseCompareEachInt(
        tok,
        [&](const Token* vartok, ValueFlow::Value true_value2, ValueFlow::Value false_value2) {
        if (result)
            return;
        result = vartok;
        true_value = std::move(true_value2);
        false_value = std::move(false_value2);
    },
        [&](const Token* t) -> std::vector<ValueFlow::Value> {
        std::vector<ValueFlow::Value> r;
        std::vector<MathLib::bigint> v = evaluate(t);

        std::transform(v.begin(), v.end(), std::back_inserter(r), [&](MathLib::bigint i) {
            return ValueFlow::Value{i};
        });
        return r;
    });
    return result;
}

const Token *parseCompareInt(const Token *tok, ValueFlow::Value &true_value, ValueFlow::Value &false_value)
{
    return parseCompareInt(tok, true_value, false_value, [](const Token* t) -> std::vector<MathLib::bigint> {
        if (t->hasKnownIntValue())
            return {t->values().front().intvalue};
        return std::vector<MathLib::bigint>{};
    });
}

static bool isInConstructorList(const Token* tok)
{
    if (!tok)
        return false;
    if (!astIsRHS(tok))
        return false;
    const Token* parent = tok->astParent();
    if (!Token::Match(parent, "{|("))
        return false;
    if (!Token::Match(parent->previous(), "%var% {|("))
        return false;
    if (!parent->astOperand1() || !parent->astOperand2())
        return false;
    do {
        parent = parent->astParent();
    } while (Token::simpleMatch(parent, ","));
    return Token::simpleMatch(parent, ":") && !Token::simpleMatch(parent->astParent(), "?");
}

static std::vector<ValueType> getParentValueTypes(const Token* tok,
                                                  const Settings* settings = nullptr,
                                                  const Token** parent = nullptr)
{
    if (!tok)
        return {};
    if (!tok->astParent())
        return {};
    if (isInConstructorList(tok)) {
        if (parent)
            *parent = tok->astParent()->astOperand1();
        if (tok->astParent()->astOperand1()->valueType())
            return {*tok->astParent()->astOperand1()->valueType()};
        return {};
    } else if (Token::Match(tok->astParent(), "(|{|,")) {
        int argn = -1;
        const Token* ftok = getTokenArgumentFunction(tok, argn);
        const Token* typeTok = nullptr;
        if (ftok && argn >= 0) {
            if (ftok->function()) {
                std::vector<ValueType> result;
                std::vector<const Variable*> argsVars = getArgumentVars(ftok, argn);
                const Token* nameTok = nullptr;
                for (const Variable* var : getArgumentVars(ftok, argn)) {
                    if (!var)
                        continue;
                    if (!var->valueType())
                        continue;
                    nameTok = var->nameToken();
                    result.push_back(*var->valueType());
                }
                if (result.size() == 1 && nameTok && parent) {
                    *parent = nameTok;
                }
                return result;
            } else if (const Type* t = Token::typeOf(ftok, &typeTok)) {
                if (astIsPointer(typeTok))
                    return {*typeTok->valueType()};
                const Scope* scope = t->classScope;
                // Check for aggregate constructors
                if (scope && scope->numConstructors == 0 && t->derivedFrom.empty() &&
                    (t->isClassType() || t->isStructType()) && numberOfArguments(ftok) < scope->varlist.size()) {
                    assert(argn < scope->varlist.size());
                    auto it = std::next(scope->varlist.begin(), argn);
                    if (it->valueType())
                        return {*it->valueType()};
                }
            }
        }
    }
    if (settings && Token::Match(tok->astParent()->tokAt(-2), ". push_back|push_front|insert|push (") &&
        astIsContainer(tok->astParent()->tokAt(-2)->astOperand1())) {
        const Token* contTok = tok->astParent()->tokAt(-2)->astOperand1();
        const ValueType* vtCont = contTok->valueType();
        if (!vtCont->containerTypeToken)
            return {};
        ValueType vtParent = ValueType::parseDecl(vtCont->containerTypeToken, settings);
        return {std::move(vtParent)};
    }
    if (Token::Match(tok->astParent(), "return|(|{|%assign%") && parent) {
        *parent = tok->astParent();
    }
    if (tok->astParent()->valueType())
        return {*tok->astParent()->valueType()};
    return {};
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
        value.valueType = ValueFlow::Value::ValueType::INT;
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

static bool isNumeric(const ValueFlow::Value& value) {
    return value.isIntValue() || value.isFloatValue();
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
    if (value1.isSymbolicValue()) {
        result->valueType = value1.valueType;
        result->tokvalue = value1.tokvalue;
    }
    if (value2.isSymbolicValue()) {
        result->valueType = value2.valueType;
        result->tokvalue = value2.tokvalue;
    }
    if (value1.isIteratorValue())
        result->valueType = value1.valueType;
    if (value2.isIteratorValue())
        result->valueType = value2.valueType;
    result->condition = value1.condition ? value1.condition : value2.condition;
    result->varId = (value1.varId != 0) ? value1.varId : value2.varId;
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
    if (!Token::Match(parent, "{|("))
        return nullptr;
    // Functional cast
    if (parent->isBinaryOp() && Token::Match(parent->astOperand1(), "%type% (|{") &&
        parent->astOperand1()->tokType() == Token::eType && astIsPrimitive(parent))
        return parent->astOperand1();
    if (parent->str() != "(")
        return nullptr;
    if (!parent->astOperand2() && Token::Match(parent,"( %name%"))
        return parent->next();
    if (parent->astOperand2() && Token::Match(parent->astOperand1(), "const_cast|dynamic_cast|reinterpret_cast|static_cast <"))
        return parent->astOperand1()->tokAt(2);
    return nullptr;
}

// does the operation cause a loss of information?
static bool isNonInvertibleOperation(const Token* tok)
{
    return !Token::Match(tok, "+|-");
}

static bool isComputableValue(const Token* parent, const ValueFlow::Value& value)
{
    const bool noninvertible = isNonInvertibleOperation(parent);
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

static bool isCompatibleValueTypes(ValueFlow::Value::ValueType x, ValueFlow::Value::ValueType y)
{
    static const std::unordered_map<ValueFlow::Value::ValueType,
                                    std::unordered_set<ValueFlow::Value::ValueType, EnumClassHash>,
                                    EnumClassHash>
    compatibleTypes = {
        {ValueFlow::Value::ValueType::INT,
         {ValueFlow::Value::ValueType::FLOAT,
          ValueFlow::Value::ValueType::SYMBOLIC,
          ValueFlow::Value::ValueType::TOK}},
        {ValueFlow::Value::ValueType::FLOAT, {ValueFlow::Value::ValueType::INT}},
        {ValueFlow::Value::ValueType::TOK, {ValueFlow::Value::ValueType::INT}},
        {ValueFlow::Value::ValueType::ITERATOR_START, {ValueFlow::Value::ValueType::INT}},
        {ValueFlow::Value::ValueType::ITERATOR_END, {ValueFlow::Value::ValueType::INT}},
    };
    if (x == y)
        return true;
    auto it = compatibleTypes.find(x);
    if (it == compatibleTypes.end())
        return false;
    return it->second.count(y) > 0;
}

static bool isCompatibleValues(const ValueFlow::Value& value1, const ValueFlow::Value& value2)
{
    if (value1.isSymbolicValue() && value2.isSymbolicValue() && value1.tokvalue->exprId() != value2.tokvalue->exprId())
        return false;
    if (!isCompatibleValueTypes(value1.valueType, value2.valueType))
        return false;
    if (value1.isKnown() || value2.isKnown())
        return true;
    if (value1.isImpossible() || value2.isImpossible())
        return false;
    if (value1.varId == 0 || value2.varId == 0)
        return true;
    if (value1.varId == value2.varId && value1.varvalue == value2.varvalue && value1.isIntValue() && value2.isIntValue())
        return true;
    return false;
}

static ValueFlow::Value truncateImplicitConversion(Token* parent, const ValueFlow::Value& value, const Settings* settings)
{
    if (!value.isIntValue() && !value.isFloatValue())
        return value;
    if (!parent)
        return value;
    if (!parent->isBinaryOp())
        return value;
    if (!parent->isConstOp())
        return value;
    if (!astIsIntegral(parent->astOperand1(), false))
        return value;
    if (!astIsIntegral(parent->astOperand2(), false))
        return value;
    const ValueType* vt1 = parent->astOperand1()->valueType();
    const ValueType* vt2 = parent->astOperand2()->valueType();
    // If the sign is the same there is no truncation
    if (vt1->sign == vt2->sign)
        return value;
    size_t n1 = ValueFlow::getSizeOf(*vt1, settings);
    size_t n2 = ValueFlow::getSizeOf(*vt2, settings);
    ValueType::Sign sign = ValueType::Sign::UNSIGNED;
    if (n1 < n2)
        sign = vt2->sign;
    else if (n1 > n2)
        sign = vt1->sign;
    ValueFlow::Value v = castValue(value, sign, std::max(n1, n2) * 8);
    v.wideintvalue = value.intvalue;
    return v;
}

/** set ValueFlow value and perform calculations if possible */
static void setTokenValue(Token* tok,
                          ValueFlow::Value value,
                          const Settings* settings,
                          SourceLocation loc = SourceLocation::current())
{
    // Skip setting values that are too big since its ambiguous
    if (!value.isImpossible() && value.isIntValue() && value.intvalue < 0 && astIsUnsigned(tok) &&
        ValueFlow::getSizeOf(*tok->valueType(), settings) >= sizeof(MathLib::bigint))
        return;

    if (!value.isImpossible() && value.isIntValue())
        value = truncateImplicitConversion(tok->astParent(), value, settings);

    if (settings->debugnormal)
        setSourceLocation(value, loc, tok);

    if (!tok->addValue(value))
        return;

    if (value.path < 0)
        return;

    Token *parent = tok->astParent();
    if (!parent)
        return;

    if (Token::simpleMatch(parent, "=") && astIsRHS(tok) && !value.isLifetimeValue()) {
        setTokenValue(parent, value, settings);
        return;
    }

    if (value.isContainerSizeValue()) {
        // .empty, .size, +"abc", +'a'
        if (Token::Match(parent, "+|==|!=") && parent->astOperand1() && parent->astOperand2()) {
            for (const ValueFlow::Value &value1 : parent->astOperand1()->values()) {
                if (value1.isImpossible())
                    continue;
                for (const ValueFlow::Value &value2 : parent->astOperand2()->values()) {
                    if (value2.isImpossible())
                        continue;
                    if (value1.path != value2.path)
                        continue;
                    ValueFlow::Value result;
                    if (Token::Match(parent, "%comp%"))
                        result.valueType = ValueFlow::Value::ValueType::INT;
                    else
                        result.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;

                    if (value1.isContainerSizeValue() && value2.isContainerSizeValue())
                        result.intvalue = calculate(parent->str(), value1.intvalue, value2.intvalue);
                    else if (value1.isContainerSizeValue() && value2.isTokValue() && value2.tokvalue->tokType() == Token::eString)
                        result.intvalue = calculate(parent->str(), value1.intvalue, MathLib::bigint(Token::getStrLength(value2.tokvalue)));
                    else if (value2.isContainerSizeValue() && value1.isTokValue() && value1.tokvalue->tokType() == Token::eString)
                        result.intvalue = calculate(parent->str(), MathLib::bigint(Token::getStrLength(value1.tokvalue)), value2.intvalue);
                    else
                        continue;

                    combineValueProperties(value1, value2, &result);

                    if (Token::simpleMatch(parent, "==") && result.intvalue)
                        continue;
                    if (Token::simpleMatch(parent, "!=") && !result.intvalue)
                        continue;

                    setTokenValue(parent, result, settings);
                }
            }
        }

        else if (Token::Match(parent, ". %name% (") && parent->astParent() == parent->tokAt(2) &&
                 parent->astOperand1() && parent->astOperand1()->valueType()) {
            const Library::Container* c = getLibraryContainer(parent->astOperand1());
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
        } else if (Token::Match(parent->previous(), "%name% (")) {
            if (const Library::Function* f = settings->library.getFunction(parent->previous())) {
                if (f->containerYield == Library::Container::Yield::SIZE) {
                    ValueFlow::Value v(value);
                    v.valueType = ValueFlow::Value::ValueType::INT;
                    setTokenValue(parent, v, settings);
                } else if (f->containerYield == Library::Container::Yield::EMPTY) {
                    ValueFlow::Value v(value);
                    v.intvalue = !v.intvalue;
                    v.valueType = ValueFlow::Value::ValueType::INT;
                    setTokenValue(parent, v, settings);
                }
            }
        }

        return;
    }

    if (value.isLifetimeValue()) {
        if (!isLifetimeBorrowed(parent, settings))
            return;
        if (value.lifetimeKind == ValueFlow::Value::LifetimeKind::Iterator && astIsIterator(parent)) {
            setTokenValue(parent,value,settings);
        } else if (astIsPointer(tok) && astIsPointer(parent) && !parent->isUnaryOp("*") &&
                   (parent->isArithmeticalOp() || parent->isCast())) {
            setTokenValue(parent,value,settings);
        }
        return;
    }

    if (value.isUninitValue()) {
        if (Token::Match(tok, ". %var%"))
            setTokenValue(tok->next(), value, settings);
        if (parent->isCast()) {
            setTokenValue(parent, value, settings);
            return;
        }
        ValueFlow::Value pvalue = value;
        if (!value.subexpressions.empty() && Token::Match(parent, ". %var%")) {
            if (contains(value.subexpressions, parent->next()->str()))
                pvalue.subexpressions.clear();
            else
                return;
        }
        if (parent->isUnaryOp("&")) {
            pvalue.indirect++;
            setTokenValue(parent, pvalue, settings);
        } else if (Token::Match(parent, ". %var%") && parent->astOperand1() == tok && parent->astOperand2()) {
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
        if (((tok->valueType() == nullptr && value.isImpossible()) || astIsPointer(tok)) &&
            contains({ValueFlow::Value::ValueType::INT, ValueFlow::Value::ValueType::SYMBOLIC}, value.valueType) &&
            Token::simpleMatch(parent->astOperand1(), "dynamic_cast"))
            return;
        const ValueType &valueType = ValueType::parseDecl(castType, settings);
        if (value.isImpossible() && value.isIntValue() && value.intvalue < 0 && astIsUnsigned(tok) &&
            valueType.sign == ValueType::SIGNED && tok->valueType() &&
            ValueFlow::getSizeOf(*tok->valueType(), settings) >= ValueFlow::getSizeOf(valueType, settings))
            return;
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
        } else if (!value.isImpossible()) {
            // is condition only depending on 1 variable?
            // cppcheck-suppress[variableScope] #8541
            nonneg int varId = 0;
            bool ret = false;
            visitAstNodes(parent->astOperand1(),
                          [&](const Token *t) {
                if (t->varId()) {
                    if (varId > 0 || value.varId != 0)
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

        const bool noninvertible = isNonInvertibleOperation(parent);

        // Skip operators with impossible values that are not invertible
        if (noninvertible && value.isImpossible())
            return;

        // known result when a operand is 0.
        if (Token::Match(parent, "[&*]") && astIsIntegral(parent, true) && value.isKnown() && value.isIntValue() &&
            value.intvalue == 0) {
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
                if (!isCompatibleValues(value1, value2))
                    continue;
                ValueFlow::Value result(0);
                combineValueProperties(value1, value2, &result);
                if (astIsFloat(parent, false)) {
                    if (!result.isIntValue() && !result.isFloatValue())
                        continue;
                    result.valueType = ValueFlow::Value::ValueType::FLOAT;
                }
                const double floatValue1 = value1.isFloatValue() ? value1.floatValue : value1.intvalue;
                const double floatValue2 = value2.isFloatValue() ? value2.floatValue : value2.intvalue;
                const MathLib::bigint intValue1 =
                    value1.isFloatValue() ? std::llround(value1.floatValue) : value1.intvalue;
                const MathLib::bigint intValue2 =
                    value2.isFloatValue() ? std::llround(value2.floatValue) : value2.intvalue;
                if ((value1.isFloatValue() || value2.isFloatValue()) && Token::Match(parent, "&|^|%|<<|>>|==|!=|%or%"))
                    continue;
                if (Token::Match(parent, "==|!=")) {
                    if ((value1.isIntValue() && value2.isTokValue()) || (value1.isTokValue() && value2.isIntValue())) {
                        if (parent->str() == "==")
                            result.intvalue = 0;
                        else if (parent->str() == "!=")
                            result.intvalue = 1;
                    } else if (value1.isIntValue() && value2.isIntValue()) {
                        bool error = false;
                        result.intvalue = calculate(parent->str(), intValue1, intValue2, &error);
                        if (error)
                            continue;
                    } else {
                        continue;
                    }
                    setTokenValue(parent, result, settings);
                } else if (Token::Match(parent, "%op%")) {
                    if (Token::Match(parent, "%comp%")) {
                        if (!result.isFloatValue() && !value1.isIntValue() && !value2.isIntValue())
                            continue;
                    } else {
                        if (value1.isTokValue() || value2.isTokValue())
                            break;
                    }
                    bool error = false;
                    if (result.isFloatValue()) {
                        result.floatValue = calculate(parent->str(), floatValue1, floatValue2, &error);
                    } else {
                        result.intvalue = calculate(parent->str(), intValue1, intValue2, &error);
                    }
                    if (error)
                        continue;
                    // If the bound comes from the second value then invert the bound when subtracting
                    if (Token::simpleMatch(parent, "-") && value2.bound == result.bound &&
                        value2.bound != ValueFlow::Value::Bound::Point)
                        result.invertBound();
                    setTokenValue(parent, result, settings);
                }
            }
        }
    }

    // !
    else if (parent->str() == "!") {
        for (const ValueFlow::Value &val : tok->values()) {
            if (!val.isIntValue())
                continue;
            if (val.isImpossible() && val.intvalue != 0)
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
            if (tok->valueType() &&
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
            if (!val.isIntValue() && !val.isFloatValue() && !val.isSymbolicValue())
                continue;
            ValueFlow::Value v(val);
            if (parent == tok->previous()) {
                if (v.isIntValue() || v.isSymbolicValue())
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
            if (!val.isIntValue() && !val.isFloatValue() && !val.isSymbolicValue())
                continue;
            ValueFlow::Value v(val);
            if (parent == tok->previous()) {
                if (v.isIntValue() || v.isSymbolicValue())
                    v.intvalue = v.intvalue - 1;
                else
                    v.floatValue = v.floatValue - 1.0;
            }
            setTokenValue(parent, v, settings);
        }
    }

    else if (Token::Match(parent, ":: %name%") && parent->astOperand2() == tok) {
        setTokenValue(parent, value, settings);
    }
    // Calling std::size or std::empty on an array
    else if (value.isTokValue() && Token::simpleMatch(value.tokvalue, "{") && tok->variable() &&
             tok->variable()->isArray() && Token::Match(parent->previous(), "%name% (") && astIsRHS(tok)) {
        std::vector<const Token*> args = getArguments(value.tokvalue);
        if (const Library::Function* f = settings->library.getFunction(parent->previous())) {
            if (f->containerYield == Library::Container::Yield::SIZE) {
                ValueFlow::Value v(value);
                v.valueType = ValueFlow::Value::ValueType::INT;
                v.intvalue = args.size();
                setTokenValue(parent, v, settings);
            } else if (f->containerYield == Library::Container::Yield::EMPTY) {
                ValueFlow::Value v(value);
                v.intvalue = args.empty();
                v.valueType = ValueFlow::Value::ValueType::INT;
                setTokenValue(parent, v, settings);
            }
        }
    }
}

static void setTokenValueCast(Token *parent, const ValueType &valueType, const ValueFlow::Value &value, const Settings *settings)
{
    if (valueType.pointer || value.isImpossible())
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
    else if (valueType.isFloat() && isNumeric(value)) {
        ValueFlow::Value floatValue = value;
        floatValue.valueType = ValueFlow::Value::ValueType::FLOAT;
        if (value.isIntValue())
            floatValue.floatValue = value.intvalue;
        setTokenValue(parent, floatValue, settings);
    } else if (value.isIntValue()) {
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
        try {
            ValueFlow::Value value(MathLib::toLongNumber(tok->str()));
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok, value, settings);
        } catch (const std::exception & /*e*/) {
            // Bad character literal
        }
    } else if (tok->isNumber() && MathLib::isFloat(tok->str())) {
        ValueFlow::Value value;
        value.valueType = ValueFlow::Value::ValueType::FLOAT;
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
        if (tok->next()->astOperand2() && !tok->next()->astOperand2()->isLiteral() && tok->next()->astOperand2()->valueType() &&
            tok->next()->astOperand2()->valueType()->pointer == 0 && // <- TODO this is a bailout, abort when there are array->pointer conversions
            !tok->next()->astOperand2()->valueType()->isEnum()) { // <- TODO this is a bailout, handle enum with non-int types
            const size_t sz = ValueFlow::getSizeOf(*tok->next()->astOperand2()->valueType(), settings);
            if (sz) {
                ValueFlow::Value value(sz);
                value.setKnown();
                setTokenValue(tok->next(), value, settings);
                return tok->linkAt(1);
            }
        }

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
            const Token *sz2 = tok->tokAt(6); // left parenthesis
            const nonneg int varid1 = sz1->varId();
            if (varid1 &&
                sz1->variable() &&
                sz1->variable()->isArray() &&
                !sz1->variable()->dimensions().empty() &&
                sz1->variable()->dimensionKnown(0) &&
                Token::Match(sz2->astOperand2(), "*|[") && Token::Match(sz2->astOperand2()->astOperand1(), "%varid%", varid1)) {
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
            strvalue.valueType = ValueFlow::Value::ValueType::TOK;
            strvalue.tokvalue = tok;
            strvalue.setKnown();
            setTokenValue(tok, strvalue, tokenlist->getSettings());
        }
    }
}

static void valueFlowArray(TokenList *tokenlist)
{
    std::map<nonneg int, const Token *> constantArrays;

    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->varId() > 0) {
            // array
            const std::map<nonneg int, const Token *>::const_iterator it = constantArrays.find(tok->varId());
            if (it != constantArrays.end()) {
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::TOK;
                value.tokvalue = it->second;
                value.setKnown();
                setTokenValue(tok, value, tokenlist->getSettings());
            }

            // const array decl
            else if (tok->variable() && tok->variable()->isArray() && tok->variable()->isConst() &&
                     tok->variable()->nameToken() == tok && Token::Match(tok, "%var% [ %num%| ] = {")) {
                const Token* rhstok = tok->next()->link()->tokAt(2);
                constantArrays[tok->varId()] = rhstok;
                tok = rhstok->link();
            }

            // pointer = array
            else if (tok->variable() && tok->variable()->isArray() && Token::simpleMatch(tok->astParent(), "=") &&
                     astIsRHS(tok) && tok->astParent()->astOperand1() &&
                     tok->astParent()->astOperand1()->variable() &&
                     tok->astParent()->astOperand1()->variable()->isPointer()) {
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::TOK;
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

static void valueFlowArrayElement(TokenList* tokenlist, const Settings* settings)
{
    for (Token* tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!Token::simpleMatch(tok, "["))
            continue;
        if (!tok->isBinaryOp())
            continue;
        if (tok->hasKnownIntValue())
            continue;
        const Token* indexTok = tok->astOperand2();
        const Token* arrayTok = tok->astOperand1();

        for (const ValueFlow::Value& arrayValue : arrayTok->values()) {
            if (!arrayValue.isTokValue())
                continue;
            for (const ValueFlow::Value& indexValue : indexTok->values()) {
                if (!indexValue.isIntValue())
                    continue;
                if (arrayValue.varId != 0 && indexValue.varId != 0 &&
                    !(arrayValue.varId == indexValue.varId && arrayValue.varvalue == indexValue.varvalue))
                    continue;

                ValueFlow::Value result(0);
                result.condition = arrayValue.condition ? arrayValue.condition : indexValue.condition;
                result.setInconclusive(arrayValue.isInconclusive() | indexValue.isInconclusive());
                result.varId = (arrayValue.varId != 0) ? arrayValue.varId : indexValue.varId;
                result.varvalue = (result.varId == arrayValue.varId) ? arrayValue.intvalue : indexValue.intvalue;
                if (arrayValue.valueKind == indexValue.valueKind)
                    result.valueKind = arrayValue.valueKind;

                result.errorPath.insert(result.errorPath.end(), arrayValue.errorPath.begin(), arrayValue.errorPath.end());
                result.errorPath.insert(result.errorPath.end(), indexValue.errorPath.begin(), indexValue.errorPath.end());

                const MathLib::bigint index = indexValue.intvalue;

                if (arrayValue.tokvalue->tokType() == Token::eString) {
                    const std::string s = arrayValue.tokvalue->strValue();
                    if (index == s.size()) {
                        result.intvalue = 0;
                        setTokenValue(tok, result, settings);
                    } else if (index >= 0 && index < s.size()) {
                        result.intvalue = s[index];
                        setTokenValue(tok, result, settings);
                    }
                } else if (Token::simpleMatch(arrayValue.tokvalue, "{")) {
                    std::vector<const Token*> args = getArguments(arrayValue.tokvalue);
                    if (index < 0 || index >= args.size())
                        continue;
                    const Token* arg = args[index];
                    if (!arg->hasKnownIntValue())
                        continue;
                    const ValueFlow::Value& v = arg->values().front();
                    result.intvalue = v.intvalue;
                    result.errorPath.insert(result.errorPath.end(), v.errorPath.begin(), v.errorPath.end());
                    setTokenValue(tok, result, settings);
                }
            }
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
        value.valueType = ValueFlow::Value::ValueType::TOK;
        value.tokvalue = tok;
        setTokenValue(tok, value, tokenlist->getSettings());
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
        if (tok->hasKnownIntValue())
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

static std::vector<MathLib::bigint> minUnsignedValue(const Token* tok, int depth = 8)
{
    std::vector<MathLib::bigint> result = {};
    if (!tok)
        return result;
    if (depth < 0)
        return result;
    if (tok->hasKnownIntValue()) {
        result = {tok->values().front().intvalue};
    } else if (!Token::Match(tok, "-|%|&|^") && tok->isConstOp() && tok->astOperand1() && tok->astOperand2()) {
        std::vector<MathLib::bigint> op1 = minUnsignedValue(tok->astOperand1(), depth - 1);
        std::vector<MathLib::bigint> op2 = minUnsignedValue(tok->astOperand2(), depth - 1);
        if (!op1.empty() && !op2.empty()) {
            result = calculate<std::vector<MathLib::bigint>>(tok->str(), op1.front(), op2.front());
        }
    }
    if (result.empty() && astIsUnsigned(tok))
        result = {0};
    return result;
}

static bool isConvertedToIntegral(const Token* tok, const Settings* settings)
{
    if (!tok)
        return false;
    std::vector<ValueType> parentTypes = getParentValueTypes(tok, settings);
    if (parentTypes.empty())
        return false;
    const ValueType& vt = parentTypes.front();
    return vt.type != ValueType::UNKNOWN_INT && vt.isIntegral();
}

static void valueFlowImpossibleValues(TokenList* tokenList, const Settings* settings)
{
    for (Token* tok = tokenList->front(); tok; tok = tok->next()) {
        if (tok->hasKnownIntValue())
            continue;
        if (Token::Match(tok, "true|false"))
            continue;
        if (astIsBool(tok) || Token::Match(tok, "%comp%")) {
            ValueFlow::Value lower{-1};
            lower.bound = ValueFlow::Value::Bound::Upper;
            lower.setImpossible();
            setTokenValue(tok, lower, settings);

            ValueFlow::Value upper{2};
            upper.bound = ValueFlow::Value::Bound::Lower;
            upper.setImpossible();
            setTokenValue(tok, upper, settings);
        } else if (astIsUnsigned(tok) && !astIsPointer(tok)) {
            std::vector<MathLib::bigint> minvalue = minUnsignedValue(tok);
            if (minvalue.empty())
                continue;
            ValueFlow::Value value{std::max<MathLib::bigint>(0, minvalue.front()) - 1};
            value.bound = ValueFlow::Value::Bound::Upper;
            value.setImpossible();
            setTokenValue(tok, value, settings);
        }
        if (Token::simpleMatch(tok, "%") && tok->astOperand2() && tok->astOperand2()->hasKnownIntValue()) {
            ValueFlow::Value value{tok->astOperand2()->values().front()};
            value.bound = ValueFlow::Value::Bound::Lower;
            value.setImpossible();
            setTokenValue(tok, value, settings);
        } else if (Token::Match(tok, "abs|labs|llabs|fabs|fabsf|fabsl (")) {
            ValueFlow::Value value{-1};
            value.bound = ValueFlow::Value::Bound::Upper;
            value.setImpossible();
            setTokenValue(tok->next(), value, settings);
        } else if (Token::Match(tok, ". data|c_str (") && astIsContainerOwned(tok->astOperand1())) {
            const Library::Container* container = getLibraryContainer(tok->astOperand1());
            if (!container)
                continue;
            if (!container->stdStringLike)
                continue;
            if (container->view)
                continue;
            ValueFlow::Value value{0};
            value.setImpossible();
            setTokenValue(tok->tokAt(2), value, settings);
        } else if (Token::Match(tok, "make_shared|make_unique <") && Token::simpleMatch(tok->linkAt(1), "> (")) {
            ValueFlow::Value value{0};
            value.setImpossible();
            setTokenValue(tok->linkAt(1)->next(), value, settings);
        } else if (tokenList->isCPP() && Token::simpleMatch(tok, "this")) {
            ValueFlow::Value value{0};
            value.setImpossible();
            setTokenValue(tok, value, settings);
        } else if (tok->isIncompleteVar() && tok->astParent() && tok->astParent()->isUnaryOp("-") &&
                   isConvertedToIntegral(tok->astParent(), settings)) {
            ValueFlow::Value value{0};
            value.setImpossible();
            setTokenValue(tok, value, settings);
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

ValuePtr<Analyzer> makeAnalyzer(const Token* exprTok, ValueFlow::Value value, const TokenList* tokenlist);
ValuePtr<Analyzer> makeReverseAnalyzer(const Token* exprTok, const ValueFlow::Value& value, const TokenList* tokenlist);

static Analyzer::Result valueFlowForward(Token* startToken,
                                         const Token* endToken,
                                         const Token* exprTok,
                                         ValueFlow::Value value,
                                         TokenList* const tokenlist,
                                         SourceLocation loc = SourceLocation::current())
{
    if (tokenlist->getSettings()->debugnormal)
        setSourceLocation(value, loc, startToken);
    return valueFlowGenericForward(startToken,
                                   endToken,
                                   makeAnalyzer(exprTok, std::move(value), tokenlist),
                                   tokenlist->getSettings());
}

static Analyzer::Result valueFlowForward(Token* startToken,
                                         const Token* endToken,
                                         const Token* exprTok,
                                         std::list<ValueFlow::Value> values,
                                         TokenList* const tokenlist,
                                         SourceLocation loc = SourceLocation::current())
{
    Analyzer::Result result{};
    for (ValueFlow::Value& v : values) {
        result.update(valueFlowForward(startToken, endToken, exprTok, std::move(v), tokenlist, loc));
    }
    return result;
}

template<class ValueOrValues>
static Analyzer::Result valueFlowForward(Token* startToken,
                                         const Token* exprTok,
                                         ValueOrValues v,
                                         TokenList* tokenlist,
                                         SourceLocation loc = SourceLocation::current())
{
    const Token* endToken = nullptr;
    const Function* f = Scope::nestedInFunction(startToken->scope());
    if (f && f->functionScope)
        endToken = f->functionScope->bodyEnd;
    return valueFlowForward(startToken, endToken, exprTok, std::move(v), tokenlist, loc);
}

static Analyzer::Result valueFlowForwardRecursive(Token* top,
                                                  const Token* exprTok,
                                                  std::list<ValueFlow::Value> values,
                                                  TokenList* const tokenlist,
                                                  SourceLocation loc = SourceLocation::current())
{
    Analyzer::Result result{};
    for (ValueFlow::Value& v : values) {
        if (tokenlist->getSettings()->debugnormal)
            setSourceLocation(v, loc, top);
        result.update(
            valueFlowGenericForward(top, makeAnalyzer(exprTok, std::move(v), tokenlist), tokenlist->getSettings()));
    }
    return result;
}

static void valueFlowReverse(Token* tok,
                             const Token* const endToken,
                             const Token* const varToken,
                             std::list<ValueFlow::Value> values,
                             TokenList* tokenlist,
                             SourceLocation loc = SourceLocation::current())
{
    for (ValueFlow::Value& v : values) {
        if (tokenlist->getSettings()->debugnormal)
            setSourceLocation(v, loc, tok);
        valueFlowGenericReverse(tok, endToken, makeReverseAnalyzer(varToken, v, tokenlist), tokenlist->getSettings());
    }
}

// Deprecated
static void valueFlowReverse(TokenList* tokenlist,
                             Token* tok,
                             const Token* const varToken,
                             ValueFlow::Value val,
                             const ValueFlow::Value& val2,
                             ErrorLogger* /*errorLogger*/,
                             const Settings* = nullptr,
                             SourceLocation loc = SourceLocation::current())
{
    std::list<ValueFlow::Value> values = {val};
    if (val2.varId != 0)
        values.push_back(val2);
    valueFlowReverse(tok, nullptr, varToken, values, tokenlist, loc);
}

static bool isConditionKnown(const Token* tok, bool then)
{
    const char* op = "||";
    if (then)
        op = "&&";
    const Token* parent = tok->astParent();
    while (parent && (parent->str() == op || parent->str() == "!"))
        parent = parent->astParent();
    return Token::Match(parent, "(|;");
}

static const std::string& invertAssign(const std::string& assign)
{
    static std::unordered_map<std::string, std::string> lookup = {{"=", "="},
        {"+=", "-="},
        {"-=", "+="},
        {"*=", "/="},
        {"/=", "*="},
        {"<<=", ">>="},
        {">>=", "<<="},
        {"^=", "^="}};
    auto it = lookup.find(assign);
    if (it == lookup.end()) {
        static std::string empty;
        return empty;
    }
    else
        return it->second;
}

static std::string removeAssign(const std::string& assign) {
    return std::string{assign.begin(), assign.end() - 1};
}

template<class T, class U>
static T calculateAssign(const std::string& assign, const T& x, const U& y, bool* error = nullptr)
{
    if (assign.empty() || assign.back() != '=') {
        if (error)
            *error = true;
        return T{};
    }
    if (assign == "=")
        return y;
    return calculate<T, T>(removeAssign(assign), x, y, error);
}

template<class T, class U>
static void assignValueIfMutable(T& x, const U& y)
{
    x = y;
}

template<class T, class U>
static void assignValueIfMutable(const T&, const U&)
{}

template<class Value, REQUIRES("Value must ValueFlow::Value", std::is_convertible<Value&, const ValueFlow::Value&> )>
static bool evalAssignment(Value& lhsValue, const std::string& assign, const ValueFlow::Value& rhsValue)
{
    bool error = false;
    if (lhsValue.isSymbolicValue() && rhsValue.isIntValue()) {
        if (assign != "+=" && assign != "-=")
            return false;
        assignValueIfMutable(lhsValue.intvalue, calculateAssign(assign, lhsValue.intvalue, rhsValue.intvalue, &error));
    } else if (lhsValue.isIntValue() && rhsValue.isIntValue()) {
        assignValueIfMutable(lhsValue.intvalue, calculateAssign(assign, lhsValue.intvalue, rhsValue.intvalue, &error));
    } else if (lhsValue.isFloatValue() && rhsValue.isIntValue()) {
        assignValueIfMutable(lhsValue.floatValue,
                             calculateAssign(assign, lhsValue.floatValue, rhsValue.intvalue, &error));
    } else {
        return false;
    }
    return !error;
}

static ValueFlow::Value::MoveKind isMoveOrForward(const Token* tok)
{
    if (!tok)
        return ValueFlow::Value::MoveKind::NonMovedVariable;
    const Token* parent = tok->astParent();
    if (!Token::simpleMatch(parent, "("))
        return ValueFlow::Value::MoveKind::NonMovedVariable;
    const Token* ftok = parent->astOperand1();
    if (!ftok)
        return ValueFlow::Value::MoveKind::NonMovedVariable;
    if (Token::simpleMatch(ftok->astOperand1(), "std :: move"))
        return ValueFlow::Value::MoveKind::MovedVariable;
    if (Token::simpleMatch(ftok->astOperand1(), "std :: forward"))
        return ValueFlow::Value::MoveKind::ForwardedVariable;
    // TODO: Check for cast
    return ValueFlow::Value::MoveKind::NonMovedVariable;
}

template<class T>
struct SingleRange {
    T* x;
    T* begin() const {
        return x;
    }
    T* end() const {
        return x+1;
    }
};

template<class T>
SingleRange<T> MakeSingleRange(T& x)
{
    return {&x};
}

class SelectValueFromVarIdMapRange {
    using M = std::unordered_map<nonneg int, ValueFlow::Value>;

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = const ValueFlow::Value;
        using pointer = value_type *;
        using reference = value_type &;

        explicit Iterator(const M::const_iterator &it)
            : mIt(it) {}

        reference operator*() const {
            return mIt->second;
        }

        pointer operator->() const {
            return &mIt->second;
        }

        Iterator &operator++() {
            // cppcheck-suppress postfixOperator - forward iterator needs to perform post-increment
            mIt++;
            return *this;
        }

        friend bool operator==(const Iterator &a, const Iterator &b) {
            return a.mIt == b.mIt;
        }

        friend bool operator!=(const Iterator &a, const Iterator &b) {
            return a.mIt != b.mIt;
        }

    private:
        M::const_iterator mIt;
    };

public:
    explicit SelectValueFromVarIdMapRange(const M *m)
        : mMap(m) {}

    Iterator begin() const {
        return Iterator(mMap->begin());
    }
    Iterator end() const {
        return Iterator(mMap->end());
    }

private:
    const M *mMap;
};

// Check if its an alias of the variable or is being aliased to this variable
template<typename V>
static bool isAliasOf(const Variable * var, const Token *tok, nonneg int varid, const V& values, bool* inconclusive = nullptr)
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

static bool bifurcate(const Token* tok, const std::set<nonneg int>& varids, const Settings* settings, int depth = 20);

static bool bifurcateVariableChanged(const Variable* var,
                                     const std::set<nonneg int>& varids,
                                     const Token* start,
                                     const Token* end,
                                     const Settings* settings,
                                     int depth = 20)
{
    bool result = false;
    const Token* tok = start;
    while ((tok = findVariableChanged(
                tok->next(), end, var->isPointer(), var->declarationId(), var->isGlobal(), settings, true))) {
        if (Token::Match(tok->astParent(), "%assign%")) {
            if (!bifurcate(tok->astParent()->astOperand2(), varids, settings, depth - 1))
                return true;
        } else {
            result = true;
        }
    }
    return result;
}

static bool bifurcate(const Token* tok, const std::set<nonneg int>& varids, const Settings* settings, int depth)
{
    if (depth < 0)
        return false;
    if (!tok)
        return true;
    if (tok->hasKnownIntValue())
        return true;
    if (tok->isConstOp())
        return bifurcate(tok->astOperand1(), varids, settings, depth) && bifurcate(tok->astOperand2(), varids, settings, depth);
    if (tok->varId() != 0) {
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
        if (var->isConst() || !bifurcateVariableChanged(var, varids, start, tok, settings, depth))
            return var->isArgument() || bifurcate(start->astOperand2(), varids, settings, depth - 1);
        return false;
    }
    return false;
}

struct ValueFlowAnalyzer : Analyzer {
    const TokenList* tokenlist;
    ProgramMemoryState pms;

    ValueFlowAnalyzer() : tokenlist(nullptr), pms(nullptr) {}

    explicit ValueFlowAnalyzer(const TokenList* t) : tokenlist(t), pms(tokenlist->getSettings()) {}

    virtual const ValueFlow::Value* getValue(const Token* tok) const = 0;
    virtual ValueFlow::Value* getValue(const Token* tok) = 0;

    virtual void makeConditional() = 0;

    virtual void addErrorPath(const Token* tok, const std::string& s) = 0;

    virtual bool match(const Token* tok) const = 0;

    virtual bool internalMatch(const Token*) const {
        return false;
    }

    virtual bool isAlias(const Token* tok, bool& inconclusive) const = 0;

    using ProgramState = ProgramMemory::Map;

    virtual ProgramState getProgramState() const = 0;

    virtual int getIndirect(const Token* tok) const {
        const ValueFlow::Value* value = getValue(tok);
        if (value)
            return value->indirect;
        return 0;
    }

    virtual bool isGlobal() const {
        return false;
    }
    virtual bool dependsOnThis() const {
        return false;
    }
    virtual bool isVariable() const {
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

    struct ConditionState {
        bool dependent = true;
        bool unknown = true;

        bool isUnknownDependent() const {
            return unknown && dependent;
        }
    };

    std::unordered_map<nonneg int, const Token*> getSymbols(const Token* tok) const
    {
        std::unordered_map<nonneg int, const Token*> result;
        if (!tok)
            return result;
        for (const ValueFlow::Value& v : tok->values()) {
            if (!v.isSymbolicValue())
                continue;
            if (v.isImpossible())
                continue;
            if (!v.tokvalue)
                continue;
            if (v.tokvalue->exprId() == 0)
                continue;
            if (match(v.tokvalue))
                continue;
            result[v.tokvalue->exprId()] = v.tokvalue;
        }
        return result;
    }

    ConditionState analyzeCondition(const Token* tok, int depth = 20) const
    {
        ConditionState result;
        if (!tok)
            return result;
        if (depth < 0)
            return result;
        depth--;
        if (analyze(tok, Direction::Forward).isRead()) {
            result.dependent = true;
            result.unknown = false;
            return result;
        } else if (tok->hasKnownIntValue() || tok->isLiteral()) {
            result.dependent = false;
            result.unknown = false;
            return result;
        } else if (Token::Match(tok, "%cop%")) {
            if (isLikelyStream(isCPP(), tok->astOperand1())) {
                result.dependent = false;
                return result;
            }
            ConditionState lhs = analyzeCondition(tok->astOperand1(), depth - 1);
            if (lhs.isUnknownDependent())
                return lhs;
            ConditionState rhs = analyzeCondition(tok->astOperand2(), depth - 1);
            if (rhs.isUnknownDependent())
                return rhs;
            if (Token::Match(tok, "%comp%"))
                result.dependent = lhs.dependent && rhs.dependent;
            else
                result.dependent = lhs.dependent || rhs.dependent;
            result.unknown = lhs.unknown || rhs.unknown;
            return result;
        } else if (Token::Match(tok->previous(), "%name% (")) {
            std::vector<const Token*> args = getArguments(tok->previous());
            if (Token::Match(tok->tokAt(-2), ". %name% (")) {
                args.push_back(tok->tokAt(-2)->astOperand1());
            }
            result.dependent = std::any_of(args.begin(), args.end(), [&](const Token* arg) {
                ConditionState cs = analyzeCondition(arg, depth - 1);
                return cs.dependent;
            });
            if (result.dependent) {
                // Check if we can evaluate the function
                if (!evaluate(Evaluate::Integral, tok).empty())
                    result.unknown = false;
            }
            return result;
        } else {
            std::unordered_map<nonneg int, const Token*> symbols = getSymbols(tok);
            result.dependent = false;
            for (auto&& p : symbols) {
                const Token* arg = p.second;
                ConditionState cs = analyzeCondition(arg, depth - 1);
                result.dependent = cs.dependent;
                if (result.dependent)
                    break;
            }
            if (result.dependent) {
                // Check if we can evaluate the token
                if (!evaluate(Evaluate::Integral, tok).empty())
                    result.unknown = false;
            }
            return result;
        }
    }

    virtual Action isModified(const Token* tok) const {
        Action read = Action::Read;
        const ValueFlow::Value* value = getValue(tok);
        if (value) {
            // Moving a moved value wont change the moved value
            if (value->isMovedValue() && isMoveOrForward(tok) != ValueFlow::Value::MoveKind::NonMovedVariable)
                return read;
            // Inserting elements to container wont change the lifetime
            if (astIsContainer(tok) && value->isLifetimeValue() &&
                contains({Library::Container::Action::PUSH,
                          Library::Container::Action::INSERT,
                          Library::Container::Action::CHANGE_INTERNAL},
                         astContainerAction(tok)))
                return read;
        }
        bool inconclusive = false;
        if (isVariableChangedByFunctionCall(tok, getIndirect(tok), getSettings(), &inconclusive))
            return read | Action::Invalid;
        if (inconclusive)
            return read | Action::Inconclusive;
        if (isVariableChanged(tok, getIndirect(tok), getSettings(), isCPP())) {
            if (Token::Match(tok->astParent(), "*|[|.|++|--"))
                return read | Action::Invalid;
            // Check if its assigned to the same value
            if (value && !value->isImpossible() && Token::simpleMatch(tok->astParent(), "=") && astIsLHS(tok) &&
                astIsIntegral(tok->astParent()->astOperand2(), false)) {
                std::vector<MathLib::bigint> result =
                    evaluateInt(tok->astParent()->astOperand2(), [&] {
                    return ProgramMemory{getProgramState()};
                });
                if (!result.empty() && value->equalTo(result.front()))
                    return Action::Idempotent;
            }
            return Action::Invalid;
        }
        return read;
    }

    virtual Action isAliasModified(const Token* tok) const {
        // Lambda function call
        if (Token::Match(tok, "%var% ("))
            // TODO: Check if modified in the lambda function
            return Action::Invalid;
        int indirect = 0;
        if (tok->valueType())
            indirect = tok->valueType()->pointer;
        if (isVariableChanged(tok, indirect, getSettings(), isCPP()))
            return Action::Invalid;
        return Action::None;
    }

    virtual Action isThisModified(const Token* tok) const {
        if (isThisChanged(tok, 0, getSettings(), isCPP()))
            return Action::Invalid;
        return Action::None;
    }

    Action isGlobalModified(const Token* tok) const
    {
        if (tok->function()) {
            if (!tok->function()->isConstexpr() && !isConstFunctionCall(tok, getSettings()->library))
                return Action::Invalid;
        } else if (getSettings()->library.getFunction(tok)) {
            // Assume library function doesn't modify user-global variables
            return Action::None;
        } else if (Token::simpleMatch(tok->astParent(), ".") && astIsContainer(tok->astParent()->astOperand1())) {
            // Assume container member function doesn't modify user-global variables
            return Action::None;
        } else if (tok->tokType() == Token::eType && astIsPrimitive(tok->next())) {
            // Function cast does not modify global variables
            return Action::None;
        } else if (!tok->isKeyword() && Token::Match(tok, "%name% (")) {
            return Action::Invalid;
        }
        return Action::None;
    }

    static const std::string& getAssign(const Token* tok, Direction d)
    {
        if (d == Direction::Forward)
            return tok->str();
        else
            return invertAssign(tok->str());
    }

    virtual Action isWritable(const Token* tok, Direction d) const {
        const ValueFlow::Value* value = getValue(tok);
        if (!value)
            return Action::None;
        if (!(value->isIntValue() || value->isFloatValue() || value->isSymbolicValue() || value->isLifetimeValue()))
            return Action::None;
        const Token* parent = tok->astParent();
        // Only if its invertible
        if (value->isImpossible() && !Token::Match(parent, "+=|-=|*=|++|--"))
            return Action::None;
        if (value->isLifetimeValue()) {
            if (value->lifetimeKind != ValueFlow::Value::LifetimeKind::Iterator)
                return Action::None;
            if (!Token::Match(parent, "++|--|+="))
                return Action::None;
            return Action::Read | Action::Write;
        }
        if (parent && parent->isAssignmentOp() && astIsLHS(tok) &&
            parent->astOperand2()->hasKnownValue()) {
            const Token* rhs = parent->astOperand2();
            const ValueFlow::Value* rhsValue = rhs->getKnownValue(ValueFlow::Value::ValueType::INT);
            Action a;
            if (!rhsValue || !evalAssignment(*value, getAssign(parent, d), *rhsValue))
                a = Action::Invalid;
            else
                a = Action::Write;
            if (parent->str() != "=") {
                a |= Action::Read;
            } else {
                if (rhsValue && !value->isImpossible() && value->equalValue(*rhsValue))
                    a = Action::Idempotent;
                a |= Action::Incremental;
            }
            return a;
        }

        // increment/decrement
        if (Token::Match(tok->astParent(), "++|--")) {
            return Action::Read | Action::Write | Action::Incremental;
        }
        return Action::None;
    }

    virtual void writeValue(ValueFlow::Value* value, const Token* tok, Direction d) const {
        if (!value)
            return;
        if (!tok->astParent())
            return;
        // Lifetime value doesn't change
        if (value->isLifetimeValue())
            return;
        if (tok->astParent()->isAssignmentOp()) {
            const ValueFlow::Value* rhsValue =
                tok->astParent()->astOperand2()->getKnownValue(ValueFlow::Value::ValueType::INT);
            assert(rhsValue);
            if (evalAssignment(*value, getAssign(tok->astParent(), d), *rhsValue)) {
                const std::string info("Compound assignment '" + tok->astParent()->str() + "', assigned value is " +
                                       value->infoString());
                if (tok->astParent()->str() == "=")
                    value->errorPath.clear();
                value->errorPath.emplace_back(tok, info);
            } else {
                assert(false && "Writable value cannot be evaluated");
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

    virtual bool useSymbolicValues() const {
        return true;
    }

    const Token* findMatch(const Token* tok) const
    {
        return findAstNode(tok, [&](const Token* child) {
            return match(child);
        });
    }

    bool isSameSymbolicValue(const Token* tok, ValueFlow::Value* value = nullptr) const
    {
        if (!useSymbolicValues())
            return false;
        if (Token::Match(tok, "%assign%"))
            return false;
        const ValueFlow::Value* currValue = getValue(tok);
        if (!currValue)
            return false;
        // If the same symbolic value is already there then skip
        if (currValue->isSymbolicValue() &&
            std::any_of(tok->values().begin(), tok->values().end(), [&](const ValueFlow::Value& v) {
            return v.isSymbolicValue() && currValue->equalValue(v);
        }))
            return false;
        const bool isPoint = currValue->bound == ValueFlow::Value::Bound::Point && currValue->isIntValue();
        const bool exact = !currValue->isIntValue() || currValue->isImpossible();
        for (const ValueFlow::Value& v : tok->values()) {
            if (!v.isSymbolicValue())
                continue;
            if (currValue->equalValue(v))
                continue;
            const bool toImpossible = v.isImpossible() && currValue->isKnown();
            if (!v.isKnown() && !toImpossible)
                continue;
            if (exact && v.intvalue != 0 && !isPoint)
                continue;
            std::vector<MathLib::bigint> r;
            ValueFlow::Value::Bound bound = currValue->bound;
            if (match(v.tokvalue)) {
                r = {currValue->intvalue};
            } else if (!exact && findMatch(v.tokvalue)) {
                r = evaluate(Evaluate::Integral, v.tokvalue, tok);
                if (bound == ValueFlow::Value::Bound::Point)
                    bound = v.bound;
            }
            if (!r.empty()) {
                if (value) {
                    value->errorPath.insert(value->errorPath.end(), v.errorPath.begin(), v.errorPath.end());
                    value->intvalue = r.front() + v.intvalue;
                    if (toImpossible)
                        value->setImpossible();
                    value->bound = bound;
                }
                return true;
            }
        }
        return false;
    }

    Action analyzeMatch(const Token* tok, Direction d) const {
        const Token* parent = tok->astParent();
        if (d == Direction::Reverse && isGlobal() && !dependsOnThis() && Token::Match(parent, ". %name% (")) {
            Action a = isGlobalModified(parent->next());
            if (a != Action::None)
                return a;
        }
        if ((astIsPointer(tok) || astIsSmartPointer(tok)) &&
            (Token::Match(parent, "*|[") || (parent && parent->originalName() == "->")) && getIndirect(tok) <= 0)
            return Action::Read;

        Action w = isWritable(tok, d);
        if (w != Action::None)
            return w;

        // Check for modifications by function calls
        return isModified(tok);
    }

    Action analyzeToken(const Token* ref, const Token* tok, Direction d, bool inconclusiveRef) const {
        if (!ref)
            return Action::None;
        // If its an inconclusiveRef then ref != tok
        assert(!inconclusiveRef || ref != tok);
        bool inconclusive = false;
        if (match(ref)) {
            if (inconclusiveRef) {
                Action a = isModified(tok);
                if (a.isModified() || a.isInconclusive())
                    return Action::Inconclusive;
            } else {
                return analyzeMatch(tok, d) | Action::Match;
            }
        } else if (ref->isUnaryOp("*") && !match(ref->astOperand1())) {
            const Token* lifeTok = nullptr;
            for (const ValueFlow::Value& v:ref->astOperand1()->values()) {
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
                    a |= Action::Invalid;
                if (inconclusiveRef && a.isModified())
                    return Action::Inconclusive;
                return a;
            }
            return Action::None;

        } else if (isAlias(ref, inconclusive)) {
            inconclusive |= inconclusiveRef;
            Action a = isAliasModified(tok);
            if (inconclusive && a.isModified())
                return Action::Inconclusive;
            else
                return a;
        } else if (isSameSymbolicValue(ref)) {
            return Action::Read | Action::SymbolicMatch;
        }
        return Action::None;
    }

    Action analyze(const Token* tok, Direction d) const override {
        if (invalid())
            return Action::Invalid;
        // Follow references
        std::vector<ReferenceToken> refs = followAllReferences(tok);
        const bool inconclusiveRefs = refs.size() != 1;
        if (std::none_of(refs.begin(), refs.end(), [&](const ReferenceToken& ref) {
            return tok == ref.token;
        }))
            refs.push_back(ReferenceToken{tok, {}});
        for (const ReferenceToken& ref:refs) {
            Action a = analyzeToken(ref.token, tok, d, inconclusiveRefs && ref.token != tok);
            if (internalMatch(ref.token))
                a |= Action::Internal;
            if (a != Action::None)
                return a;
        }
        if (dependsOnThis() && exprDependsOnThis(tok, !isVariable()))
            return isThisModified(tok);

        // bailout: global non-const variables
        if (isGlobal() && !dependsOnThis() && Token::Match(tok, "%name% (") &&
            !Token::simpleMatch(tok->linkAt(1), ") {")) {
            return isGlobalModified(tok);
        }
        return Action::None;
    }

    template<class F>
    std::vector<MathLib::bigint> evaluateInt(const Token* tok, F getProgramMemory) const
    {
        if (tok->hasKnownIntValue())
            return {static_cast<int>(tok->values().front().intvalue)};
        std::vector<MathLib::bigint> result;
        ProgramMemory pm = getProgramMemory();
        if (Token::Match(tok, "&&|%oror%")) {
            if (conditionIsTrue(tok, pm, getSettings()))
                result.push_back(1);
            if (conditionIsFalse(tok, pm, getSettings()))
                result.push_back(0);
        } else {
            MathLib::bigint out = 0;
            bool error = false;
            execute(tok, &pm, &out, &error, getSettings());
            if (!error)
                result.push_back(out);
        }
        return result;
    }

    std::vector<MathLib::bigint> evaluate(Evaluate e, const Token* tok, const Token* ctx = nullptr) const override
    {
        if (e == Evaluate::Integral) {
            return evaluateInt(tok, [&] {
                return pms.get(tok, ctx, getProgramState());
            });
        } else if (e == Evaluate::ContainerEmpty) {
            const ValueFlow::Value* value = ValueFlow::findValue(tok->values(), nullptr, [](const ValueFlow::Value& v) {
                return v.isKnown() && v.isContainerSizeValue();
            });
            if (value)
                return {value->intvalue == 0};
            ProgramMemory pm = pms.get(tok, ctx, getProgramState());
            MathLib::bigint out = 0;
            if (pm.getContainerEmptyValue(tok->exprId(), &out))
                return {static_cast<int>(out)};
            return {};
        } else {
            return {};
        }
    }

    void assume(const Token* tok, bool state, unsigned int flags) override {
        // Update program state
        pms.removeModifiedVars(tok);
        pms.addState(tok, getProgramState());
        pms.assume(tok, state, flags & Assume::ContainerEmpty);

        bool isCondBlock = false;
        const Token* parent = tok->astParent();
        if (parent) {
            isCondBlock = Token::Match(parent->previous(), "if|while (");
        }

        if (isCondBlock) {
            const Token* startBlock = parent->link()->next();
            if (Token::simpleMatch(startBlock, ";") && Token::simpleMatch(parent->tokAt(-2), "} while ("))
                startBlock = parent->linkAt(-2);
            const Token* endBlock = startBlock->link();
            pms.removeModifiedVars(endBlock);
            if (state)
                pms.addState(endBlock->previous(), getProgramState());
            else if (Token::simpleMatch(endBlock, "} else {"))
                pms.addState(endBlock->linkAt(2)->previous(), getProgramState());
        }

        if (!(flags & Assume::Quiet)) {
            if (flags & Assume::ContainerEmpty) {
                std::string s = state ? "empty" : "not empty";
                addErrorPath(tok, "Assuming container is " + s);
            } else {
                std::string s = state ? "true" : "false";
                addErrorPath(tok, "Assuming condition is " + s);
            }
        }
        if (!(flags & Assume::Absolute))
            makeConditional();
    }

    virtual void internalUpdate(Token*, const ValueFlow::Value&, Direction)
    {
        assert(false && "Internal update unimplemented.");
    }

    void update(Token* tok, Action a, Direction d) override {
        ValueFlow::Value* value = getValue(tok);
        if (!value)
            return;
        ValueFlow::Value localValue;
        if (a.isSymbolicMatch()) {
            // Make a copy of the value to modify it
            localValue = *value;
            value = &localValue;
            isSameSymbolicValue(tok, &localValue);
        }
        if (a.isInternal())
            internalUpdate(tok, *value, d);
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

    ValuePtr<Analyzer> reanalyze(Token*, const std::string&) const override {
        return {};
    }
};

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

    const ValueFlow::Value* getValue(const Token*) const override {
        return &value;
    }
    ValueFlow::Value* getValue(const Token*) override {
        return &value;
    }

    void makeConditional() override {
        value.conditional = true;
    }

    bool useSymbolicValues() const override
    {
        if (value.isUninitValue())
            return false;
        if (value.isLifetimeValue())
            return false;
        return true;
    }

    void addErrorPath(const Token* tok, const std::string& s) override {
        value.errorPath.emplace_back(tok, s);
    }

    bool isAlias(const Token* tok, bool& inconclusive) const override {
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
                if (isAliasOf(var, tok, varid, MakeSingleRange(value), &inconclusive))
                    return true;
            }
        }
        return false;
    }

    bool isGlobal() const override {
        for (const auto&p:getVars()) {
            const Variable* var = p.second;
            if (!var->isLocal() && !var->isArgument() && !var->isConst())
                return true;
        }
        return false;
    }

    bool lowerToPossible() override {
        if (value.isImpossible())
            return false;
        value.changeKnownToPossible();
        return true;
    }
    bool lowerToInconclusive() override {
        if (value.isImpossible())
            return false;
        value.setInconclusive();
        return true;
    }

    bool isConditional() const override {
        if (value.conditional)
            return true;
        if (value.condition)
            return !value.isKnown() && !value.isImpossible();
        return false;
    }

    bool stopOnCondition(const Token* condTok) const override
    {
        if (value.isNonValue())
            return false;
        if (value.isImpossible())
            return false;
        if (isConditional() && !value.isKnown() && !value.isImpossible())
            return true;
        if (value.isSymbolicValue())
            return false;
        ConditionState cs = analyzeCondition(condTok);
        return cs.isUnknownDependent();
    }

    bool updateScope(const Token* endBlock, bool) const override {
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

    ValuePtr<Analyzer> reanalyze(Token* tok, const std::string& msg) const override {
        ValueFlow::Value newValue = value;
        newValue.errorPath.emplace_back(tok, msg);
        return makeAnalyzer(tok, newValue, tokenlist);
    }
};

struct ExpressionAnalyzer : SingleValueFlowAnalyzer {
    const Token* expr;
    bool local;
    bool unknown;
    bool dependOnThis;

    ExpressionAnalyzer() : SingleValueFlowAnalyzer(), expr(nullptr), local(true), unknown(false), dependOnThis(false) {}

    ExpressionAnalyzer(const Token* e, const ValueFlow::Value& val, const TokenList* t)
        : SingleValueFlowAnalyzer(val, t), expr(e), local(true), unknown(false), dependOnThis(false) {

        assert(e && e->exprId() != 0 && "Not a valid expression");
        dependOnThis = exprDependsOnThis(expr);
        setupExprVarIds(expr);
        if (val.isSymbolicValue()) {
            dependOnThis |= exprDependsOnThis(val.tokvalue);
            setupExprVarIds(val.tokvalue);
        }
    }

    static bool nonLocal(const Variable* var, bool deref) {
        return !var || (!var->isLocal() && !var->isArgument()) || (deref && var->isArgument() && var->isPointer()) ||
               var->isStatic() || var->isReference() || var->isExtern();
    }

    void setupExprVarIds(const Token* start, int depth = 0) {
        const int maxDepth = 4;
        if (depth > maxDepth)
            return;
        visitAstNodes(start, [&](const Token* tok) {
            const bool top = depth == 0 && tok == start;
            const bool ispointer = astIsPointer(tok) || astIsSmartPointer(tok) || astIsIterator(tok);
            if (!top || !ispointer || value.indirect != 0) {
                for (const ValueFlow::Value& v : tok->values()) {
                    if (!(v.isLocalLifetimeValue() || (ispointer && v.isSymbolicValue() && v.isKnown())))
                        continue;
                    if (!v.tokvalue)
                        continue;
                    if (v.tokvalue == tok)
                        continue;
                    setupExprVarIds(v.tokvalue, depth + 1);
                }
            }
            if (depth == 0 && tok->isIncompleteVar()) {
                // TODO: Treat incomplete var as global, but we need to update
                // the alias variables to just expr ids instead of requiring
                // Variable
                unknown = true;
                return ChildrenToVisit::none;
            }
            if (tok->varId() > 0) {
                varids[tok->varId()] = tok->variable();
                if (!Token::simpleMatch(tok->previous(), ".")) {
                    const Variable* var = tok->variable();
                    if (var && var->isReference() && var->isLocal() && Token::Match(var->nameToken(), "%var% [=(]") &&
                        !isGlobalData(var->nameToken()->next()->astOperand2(), isCPP()))
                        return ChildrenToVisit::none;
                    const bool deref = tok->astParent() &&
                                       (tok->astParent()->isUnaryOp("*") ||
                                        (tok->astParent()->str() == "[" && tok == tok->astParent()->astOperand1()));
                    local &= !nonLocal(tok->variable(), deref);
                }
            }
            return ChildrenToVisit::op1_and_op2;
        });
    }

    bool invalid() const override {
        return unknown;
    }

    ProgramState getProgramState() const override {
        ProgramState ps;
        ps[expr] = value;
        return ps;
    }

    bool match(const Token* tok) const override {
        return tok->exprId() == expr->exprId();
    }

    bool dependsOnThis() const override {
        return dependOnThis;
    }

    bool isGlobal() const override {
        return !local;
    }

    bool isVariable() const override {
        return expr->varId() > 0;
    }
};

struct OppositeExpressionAnalyzer : ExpressionAnalyzer {
    bool isNot;

    OppositeExpressionAnalyzer() : ExpressionAnalyzer(), isNot(false) {}

    OppositeExpressionAnalyzer(bool pIsNot, const Token* e, const ValueFlow::Value& val, const TokenList* t)
        : ExpressionAnalyzer(e, val, t), isNot(pIsNot)
    {}

    bool match(const Token* tok) const override {
        return isOppositeCond(isNot, isCPP(), expr, tok, getSettings()->library, true, true);
    }
};

struct SubExpressionAnalyzer : ExpressionAnalyzer {
    using PartialReadContainer = std::vector<std::pair<Token *, ValueFlow::Value>>;
    // A shared_ptr is used so partial reads can be captured even after forking
    std::shared_ptr<PartialReadContainer> partialReads;
    SubExpressionAnalyzer() : ExpressionAnalyzer(), partialReads(nullptr) {}

    SubExpressionAnalyzer(const Token* e, const ValueFlow::Value& val, const TokenList* t)
        : ExpressionAnalyzer(e, val, t), partialReads(std::make_shared<PartialReadContainer>())
    {}

    virtual bool submatch(const Token* tok, bool exact = true) const = 0;

    bool isAlias(const Token* tok, bool& inconclusive) const override
    {
        if (tok->exprId() == expr->exprId() && tok->astParent() && submatch(tok->astParent(), false))
            return false;
        return ExpressionAnalyzer::isAlias(tok, inconclusive);
    }

    bool match(const Token* tok) const override
    {
        return tok->astOperand1() && tok->astOperand1()->exprId() == expr->exprId() && submatch(tok);
    }
    bool internalMatch(const Token* tok) const override
    {
        return tok->exprId() == expr->exprId() && !(astIsLHS(tok) && submatch(tok->astParent(), false));
    }
    void internalUpdate(Token* tok, const ValueFlow::Value& v, Direction) override
    {
        partialReads->push_back(std::make_pair(tok, v));
    }

    // No reanalysis for subexression
    ValuePtr<Analyzer> reanalyze(Token*, const std::string&) const override {
        return {};
    }
};

struct MemberExpressionAnalyzer : SubExpressionAnalyzer {
    std::string varname;
    MemberExpressionAnalyzer() : SubExpressionAnalyzer(), varname() {}

    MemberExpressionAnalyzer(std::string varname, const Token* e, const ValueFlow::Value& val, const TokenList* t)
        : SubExpressionAnalyzer(e, val, t), varname(std::move(varname))
    {}

    bool submatch(const Token* tok, bool exact) const override
    {
        if (!Token::Match(tok, ". %var%"))
            return false;
        if (!exact)
            return true;
        return tok->next()->str() == varname;
    }
};

enum class LifetimeCapture { Undefined, ByValue, ByReference };

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
    const bool classVar = tokvar ? (!tokvar->isLocal() && !tokvar->isArgument() && !tokvar->isGlobal()) : false;
    std::string type = lifetimeType(tok, val);
    std::string msg = type;
    if (vartok) {
        if (!classVar)
            errorPath.emplace_back(vartok, "Variable created here.");
        const Variable * var = vartok->variable();
        if (var) {
            std::string submessage;
            switch (val->lifetimeKind) {
            case ValueFlow::Value::LifetimeKind::SubObject:
            case ValueFlow::Value::LifetimeKind::Object:
            case ValueFlow::Value::LifetimeKind::Address:
                if (type == "pointer")
                    submessage = " to local variable";
                else
                    submessage = " that points to local variable";
                break;
            case ValueFlow::Value::LifetimeKind::Lambda:
                submessage = " that captures local variable";
                break;
            case ValueFlow::Value::LifetimeKind::Iterator:
                submessage = " to local container";
                break;
            }
            if (classVar)
                submessage.replace(submessage.find("local"), 5, "member");
            msg += submessage + " '" + var->name() + "'";
        }
    }
    return msg;
}

std::vector<ValueFlow::Value> getLifetimeObjValues(const Token* tok, bool inconclusive, MathLib::bigint path)
{
    std::vector<ValueFlow::Value> result;
    auto pred = [&](const ValueFlow::Value& v) {
        if (!v.isLocalLifetimeValue() && !(path != 0 && v.isSubFunctionLifetimeValue()))
            return false;
        if (!inconclusive && v.isInconclusive())
            return false;
        if (!v.tokvalue)
            return false;
        if (path >= 0 && v.path != 0 && v.path != path)
            return false;
        return true;
    };
    std::copy_if(tok->values().begin(), tok->values().end(), std::back_inserter(result), pred);
    return result;
}

static bool hasUniqueOwnership(const Token* tok)
{
    if (!tok)
        return false;
    const Variable* var = tok->variable();
    if (var && var->isArray() && !var->isArgument())
        return true;
    if (astIsPointer(tok))
        return false;
    if (astIsUniqueSmartPointer(tok))
        return true;
    if (astIsContainerOwned(tok))
        return true;
    return false;
}

// Check if dereferencing an object that doesn't have unique ownership
static bool derefShared(const Token* tok)
{
    if (!tok)
        return false;
    if (!tok->isUnaryOp("*") && tok->str() != "[" && tok->str() != ".")
        return false;
    if (tok->str() == "." && tok->originalName() != "->")
        return false;
    const Token* ptrTok = tok->astOperand1();
    return !hasUniqueOwnership(ptrTok);
}

ValueFlow::Value getLifetimeObjValue(const Token *tok, bool inconclusive)
{
    std::vector<ValueFlow::Value> values = getLifetimeObjValues(tok, inconclusive);
    // There should only be one lifetime
    if (values.size() != 1)
        return ValueFlow::Value{};
    return values.front();
}

template<class Predicate>
static std::vector<LifetimeToken> getLifetimeTokens(const Token* tok,
                                                    bool escape,
                                                    ValueFlow::Value::ErrorPath errorPath,
                                                    Predicate pred,
                                                    int depth = 20)
{
    if (!tok)
        return std::vector<LifetimeToken> {};
    if (Token::simpleMatch(tok, "..."))
        return std::vector<LifetimeToken>{};
    const Variable *var = tok->variable();
    if (pred(tok))
        return {{tok, std::move(errorPath)}};
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
                const bool temporary = isTemporary(true, vartok, nullptr, true);
                const bool nonlocal = var->isStatic() || var->isGlobal();
                if (vartok == tok || (nonlocal && temporary) ||
                    (!escape && (var->isConst() || var->isRValueReference()) && temporary))
                    return {{tok, true, std::move(errorPath)}};
                if (vartok)
                    return getLifetimeTokens(vartok, escape, std::move(errorPath), pred, depth - 1);
            } else if (Token::simpleMatch(var->nameToken()->astParent(), ":") &&
                       var->nameToken()->astParent()->astParent() &&
                       Token::simpleMatch(var->nameToken()->astParent()->astParent()->previous(), "for (")) {
                errorPath.emplace_back(var->nameToken(), "Assigned to reference.");
                const Token* vartok = var->nameToken();
                if (vartok == tok)
                    return {{tok, true, std::move(errorPath)}};
                const Token* contok = var->nameToken()->astParent()->astOperand2();
                if (astIsContainer(contok))
                    return getLifetimeTokens(contok, escape, std::move(errorPath), pred, depth - 1);
                else
                    return std::vector<LifetimeToken>{};
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
                for (LifetimeToken& lt : getLifetimeTokens(returnTok, escape, errorPath, pred, depth - returns.size())) {
                    const Token* argvarTok = lt.token;
                    const Variable* argvar = argvarTok->variable();
                    if (!argvar)
                        continue;
                    const Token* argTok = nullptr;
                    if (argvar->isArgument() && (argvar->isReference() || argvar->isRValueReference())) {
                        int n = getArgumentPos(argvar, f);
                        if (n < 0)
                            return std::vector<LifetimeToken> {};
                        std::vector<const Token*> args = getArguments(tok->previous());
                        // TODO: Track lifetimes of default parameters
                        if (n >= args.size())
                            return std::vector<LifetimeToken> {};
                        argTok = args[n];
                        lt.errorPath.emplace_back(returnTok, "Return reference.");
                        lt.errorPath.emplace_back(tok->previous(), "Called function passing '" + argTok->expressionString() + "'.");
                    } else if (Token::Match(tok->tokAt(-2), ". %name% (") && !derefShared(tok->tokAt(-2)) &&
                               exprDependsOnThis(argvarTok)) {
                        argTok = tok->tokAt(-2)->astOperand1();
                        lt.errorPath.emplace_back(returnTok, "Return reference that depends on 'this'.");
                        lt.errorPath.emplace_back(tok->previous(),
                                                  "Calling member function on '" + argTok->expressionString() + "'.");
                    }
                    if (argTok) {
                        std::vector<LifetimeToken> arglts = LifetimeToken::setInconclusive(
                            getLifetimeTokens(argTok, escape, std::move(lt.errorPath), pred, depth - returns.size()),
                            returns.size() > 1);
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
                    getLifetimeTokens(tok->tokAt(-2)->astOperand1(), escape, std::move(errorPath), pred, depth - 1),
                    false);
            }
        }
    } else if (Token::Match(tok, ".|::|[") || tok->isUnaryOp("*")) {

        const Token *vartok = tok;
        while (vartok) {
            if (vartok->str() == "[" || vartok->originalName() == "->" || vartok->isUnaryOp("*"))
                vartok = vartok->astOperand1();
            else if (vartok->str() == "." || vartok->str() == "::")
                vartok = vartok->astOperand2();
            else
                break;
        }

        if (!vartok)
            return {{tok, std::move(errorPath)}};
        if (derefShared(vartok->astParent())) {
            for (const ValueFlow::Value &v : vartok->values()) {
                if (!v.isLocalLifetimeValue())
                    continue;
                if (v.tokvalue == tok)
                    continue;
                errorPath.insert(errorPath.end(), v.errorPath.begin(), v.errorPath.end());
                return getLifetimeTokens(v.tokvalue, escape, std::move(errorPath), pred, depth - 1);
            }
        } else {
            return LifetimeToken::setAddressOf(getLifetimeTokens(vartok, escape, std::move(errorPath), pred, depth - 1),
                                               !(astIsContainer(vartok) && Token::simpleMatch(vartok->astParent(), "[")));
        }
    } else if (Token::simpleMatch(tok, "{") && getArgumentStart(tok) &&
               !Token::simpleMatch(getArgumentStart(tok), ",") && getArgumentStart(tok)->valueType()) {
        const Token* vartok = getArgumentStart(tok);
        auto vts = getParentValueTypes(tok);
        for (const ValueType& vt : vts) {
            if (vt.isTypeEqual(vartok->valueType()))
                return getLifetimeTokens(vartok, escape, std::move(errorPath), pred, depth - 1);
        }
    }
    return {{tok, std::move(errorPath)}};
}

std::vector<LifetimeToken> getLifetimeTokens(const Token* tok, bool escape, ValueFlow::Value::ErrorPath errorPath)
{
    return getLifetimeTokens(tok, escape, std::move(errorPath), [](const Token*) {
        return false;
    });
}

bool hasLifetimeToken(const Token* tok, const Token* lifetime)
{
    bool result = false;
    getLifetimeTokens(tok, false, ValueFlow::Value::ErrorPath{}, [&](const Token* tok2) {
        result = tok2->exprId() == lifetime->exprId();
        return result;
    });
    return result;
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

static bool isLifetimeOwned(const ValueType* vtParent)
{
    if (vtParent->container)
        return !vtParent->container->view;
    return vtParent->type == ValueType::CONTAINER;
}

static bool isLifetimeOwned(const ValueType *vt, const ValueType *vtParent)
{
    if (!vtParent)
        return false;
    if (isLifetimeOwned(vtParent))
        return true;
    if (!vt)
        return false;
    // If converted from iterator to pointer then the iterator is most likely a pointer
    if (vtParent->pointer == 1 && vt->pointer == 0 && vt->type == ValueType::ITERATOR)
        return false;
    if (vt->type != ValueType::UNKNOWN_TYPE && vtParent->type != ValueType::UNKNOWN_TYPE) {
        if (vt->pointer != vtParent->pointer)
            return true;
        if (vt->type != vtParent->type) {
            if (vtParent->type == ValueType::RECORD)
                return true;
            if (isLifetimeOwned(vtParent))
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
    if (vt->pointer > 0 && vt->pointer == vtParent->pointer)
        return true;
    if (vtParent->container && vtParent->container->view)
        return true;
    if (vt->type != ValueType::UNKNOWN_TYPE && vtParent->type != ValueType::UNKNOWN_TYPE && vtParent->container == vt->container) {
        if (vtParent->pointer > vt->pointer)
            return true;
        if (vtParent->pointer < vt->pointer && vtParent->isIntegral())
            return true;
        if (vtParent->str() == vt->str())
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

static bool isDifferentType(const Token* src, const Token* dst)
{
    const Type* t = Token::typeOf(src);
    const Type* parentT = Token::typeOf(dst);
    if (t && parentT) {
        if (t->classDef && parentT->classDef && t->classDef != parentT->classDef)
            return true;
    } else {
        std::pair<const Token*, const Token*> decl = Token::typeDecl(src);
        std::pair<const Token*, const Token*> parentdecl = Token::typeDecl(dst);
        if (isNotEqual(decl, parentdecl))
            return true;
        if (isNotEqual(decl, dst->valueType()))
            return true;
        if (isNotEqual(parentdecl, src->valueType()))
            return true;
    }
    return false;
}

bool isLifetimeBorrowed(const Token *tok, const Settings *settings)
{
    if (!tok)
        return true;
    if (tok->str() == ",")
        return true;
    if (!tok->astParent())
        return true;
    const Token* parent = nullptr;
    const ValueType* vt = tok->valueType();
    std::vector<ValueType> vtParents = getParentValueTypes(tok, settings, &parent);
    for (const ValueType& vtParent : vtParents) {
        if (isLifetimeBorrowed(vt, &vtParent))
            return true;
        if (isLifetimeOwned(vt, &vtParent))
            return false;
    }
    if (parent) {
        if (isDifferentType(tok, parent))
            return false;
    }
    return true;
}

static void valueFlowLifetimeFunction(Token *tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings);

static void valueFlowLifetimeConstructor(Token *tok,
                                         TokenList *tokenlist,
                                         ErrorLogger *errorLogger,
                                         const Settings *settings);

static const Token* getEndOfVarScope(const Variable* var)
{
    if (!var)
        return nullptr;
    const Scope* innerScope = var->scope();
    const Scope* outerScope = innerScope;
    if (var->typeStartToken() && var->typeStartToken()->scope())
        outerScope = var->typeStartToken()->scope();
    if (!innerScope && outerScope)
        innerScope = outerScope;
    if (!innerScope || !outerScope)
        return nullptr;
    if (!innerScope->isExecutable())
        return nullptr;
    // If the variable is defined in a for/while initializer then we want to
    // pick one token after the end so forward analysis can analyze the exit
    // conditions
    if (innerScope != outerScope && outerScope->isExecutable() && innerScope->isLocal())
        return innerScope->bodyEnd->next();
    return innerScope->bodyEnd;
}

const Token* getEndOfExprScope(const Token* tok, const Scope* defaultScope, bool smallest)
{
    const Token* end = nullptr;
    bool local = false;
    visitAstNodes(tok, [&](const Token* child) {
        if (const Variable* var = child->variable()) {
            local |= var->isLocal();
            if (var->isLocal() || var->isArgument()) {
                const Token* varEnd = getEndOfVarScope(var);
                if (!end || (smallest ? precedes(varEnd, end) : succeeds(varEnd, end)))
                    end = varEnd;
            }
        }
        return ChildrenToVisit::op1_and_op2;
    });
    if (!end && defaultScope)
        end = defaultScope->bodyEnd;
    if (!end) {
        const Scope* scope = tok->scope();
        if (scope)
            end = scope->bodyEnd;
        // If there is no local variables then pick the function scope
        if (!local) {
            while (scope && scope->isLocal())
                scope = scope->nestedIn;
            if (scope && scope->isExecutable())
                end = scope->bodyEnd;
        }
    }
    return end;
}

static void valueFlowForwardLifetime(Token * tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    // Forward lifetimes to constructed variable
    if (Token::Match(tok->previous(), "%var% {|(") && isVariableDecl(tok->previous())) {
        std::list<ValueFlow::Value> values = tok->values();
        values.remove_if(&isNotLifetimeValue);
        valueFlowForward(nextAfterAstRightmostLeaf(tok), getEndOfExprScope(tok), tok->previous(), values, tokenlist);
        return;
    }
    Token *parent = tok->astParent();
    while (parent && parent->str() == ",")
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

        const Token* expr = getLHSVariableToken(parent);
        if (!expr)
            return;

        if (expr->exprId() == 0)
            return;

        const Token* endOfVarScope = getEndOfExprScope(expr);

        // Only forward lifetime values
        std::list<ValueFlow::Value> values = parent->astOperand2()->values();
        values.remove_if(&isNotLifetimeValue);
        // Dont forward lifetimes that overlap
        values.remove_if([&](const ValueFlow::Value& value) {
            return findAstNode(value.tokvalue, [&](const Token* child) {
                return child->exprId() == expr->exprId();
            });
        });

        // Skip RHS
        const Token *nextExpression = nextAfterAstRightmostLeaf(parent);

        if (expr->exprId() > 0) {
            valueFlowForward(const_cast<Token*>(nextExpression), endOfVarScope->next(), expr, values, tokenlist);

            for (ValueFlow::Value& val : values) {
                if (val.lifetimeKind == ValueFlow::Value::LifetimeKind::Address)
                    val.lifetimeKind = ValueFlow::Value::LifetimeKind::SubObject;
            }
            // TODO: handle `[`
            if (Token::simpleMatch(parent->astOperand1(), ".")) {
                const Token* parentLifetime =
                    getParentLifetime(tokenlist->isCPP(), parent->astOperand1()->astOperand2(), &settings->library);
                if (parentLifetime && parentLifetime->exprId() > 0) {
                    valueFlowForward(const_cast<Token*>(nextExpression), endOfVarScope, parentLifetime, values, tokenlist);
                }
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
        valueFlowForward(const_cast<Token*>(nextExpression), endOfVarScope, tok, values, tokenlist);
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

    template<class F>
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

    template<class Predicate>
    bool byRef(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings, Predicate pred) const {
        if (!argtok)
            return false;
        bool update = false;
        for (const LifetimeToken& lt : getLifetimeTokens(argtok)) {
            if (!settings->certainty.isEnabled(Certainty::inconclusive) && lt.inconclusive)
                continue;
            ErrorPath er = errorPath;
            er.insert(er.end(), lt.errorPath.begin(), lt.errorPath.end());
            if (!lt.token)
                return false;
            if (!pred(lt.token))
                return false;
            er.emplace_back(argtok, message);

            ValueFlow::Value value;
            value.valueType = ValueFlow::Value::ValueType::LIFETIME;
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

    template<class Predicate>
    bool byVal(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings, Predicate pred) const {
        if (!argtok)
            return false;
        bool update = false;
        if (argtok->values().empty()) {
            ErrorPath er;
            er.emplace_back(argtok, message);
            for (const LifetimeToken& lt : getLifetimeTokens(argtok)) {
                if (!settings->certainty.isEnabled(Certainty::inconclusive) && lt.inconclusive)
                    continue;
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::LIFETIME;
                value.tokvalue = lt.token;
                value.capturetok = argtok;
                value.errorPath = er;
                value.lifetimeKind = type;
                value.setInconclusive(inconclusive || lt.inconclusive);
                const Variable* var = lt.token->variable();
                if (var && var->isArgument()) {
                    value.lifetimeScope = ValueFlow::Value::LifetimeScope::Argument;
                } else {
                    continue;
                }
                // Don't add the value a second time
                if (std::find(tok->values().begin(), tok->values().end(), value) != tok->values().end())
                    continue;

                setTokenValue(tok, value, tokenlist->getSettings());
                update = true;
            }
        }
        for (const ValueFlow::Value &v : argtok->values()) {
            if (!v.isLifetimeValue())
                continue;
            const Token *tok3 = v.tokvalue;
            for (const LifetimeToken& lt : getLifetimeTokens(tok3)) {
                if (!settings->certainty.isEnabled(Certainty::inconclusive) && lt.inconclusive)
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
                value.valueType = ValueFlow::Value::ValueType::LIFETIME;
                value.lifetimeScope = v.lifetimeScope;
                value.path = v.path;
                value.tokvalue = lt.token;
                value.capturetok = argtok;
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

    template<class Predicate>
    void byDerefCopy(Token *tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings, Predicate pred) const {
        if (!settings->certainty.isEnabled(Certainty::inconclusive) && inconclusive)
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
                    LifetimeStore{tok3, message, type, inconclusive}.byVal(tok, tokenlist, errorLogger, settings, pred);
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

static void valueFlowLifetimeUserConstructor(Token* tok,
                                             const Function* constructor,
                                             const std::string& name,
                                             std::vector<const Token*> args,
                                             TokenList* tokenlist,
                                             ErrorLogger* errorLogger,
                                             const Settings* settings)
{
    if (!constructor)
        return;
    std::unordered_map<const Token*, const Variable*> argToParam;
    for (std::size_t i = 0; i < args.size(); i++)
        argToParam[args[i]] = constructor->getArgumentVar(i);
    if (const Token* initList = constructor->constructorMemberInitialization()) {
        std::unordered_map<const Variable*, LifetimeCapture> paramCapture;
        for (const Token* tok2 : astFlatten(initList->astOperand2(), ",")) {
            if (!Token::simpleMatch(tok2, "("))
                continue;
            if (!tok2->astOperand1())
                continue;
            if (!tok2->astOperand2())
                continue;
            const Variable* var = tok2->astOperand1()->variable();
            const Token* expr = tok2->astOperand2();
            if (!var)
                continue;
            if (!isLifetimeBorrowed(expr, settings))
                continue;
            const Variable* argvar = getLifetimeVariable(expr);
            if (var->isReference() || var->isRValueReference()) {
                if (argvar && argvar->isArgument() && (argvar->isReference() || argvar->isRValueReference())) {
                    paramCapture[argvar] = LifetimeCapture::ByReference;
                }
            } else {
                bool found = false;
                for (const ValueFlow::Value& v : expr->values()) {
                    if (!v.isLifetimeValue())
                        continue;
                    if (v.path > 0)
                        continue;
                    if (!v.tokvalue)
                        continue;
                    const Variable* lifeVar = v.tokvalue->variable();
                    if (!lifeVar)
                        continue;
                    LifetimeCapture c = LifetimeCapture::Undefined;
                    if (!v.isArgumentLifetimeValue() && (lifeVar->isReference() || lifeVar->isRValueReference()))
                        c = LifetimeCapture::ByReference;
                    else if (v.isArgumentLifetimeValue())
                        c = LifetimeCapture::ByValue;
                    if (c != LifetimeCapture::Undefined) {
                        paramCapture[lifeVar] = c;
                        found = true;
                    }
                }
                if (!found && argvar && argvar->isArgument())
                    paramCapture[argvar] = LifetimeCapture::ByValue;
            }
        }
        // TODO: Use SubExpressionAnalyzer for members
        LifetimeStore::forEach(args,
                               "Passed to constructor of '" + name + "'.",
                               ValueFlow::Value::LifetimeKind::SubObject,
                               [&](const LifetimeStore& ls) {
            const Variable* paramVar = argToParam.at(ls.argtok);
            if (paramCapture.count(paramVar) == 0)
                return;
            LifetimeCapture c = paramCapture.at(paramVar);
            if (c == LifetimeCapture::ByReference)
                ls.byRef(tok, tokenlist, errorLogger, settings);
            else
                ls.byVal(tok, tokenlist, errorLogger, settings);
        });
    } else if (!constructor->nestedIn->varlist.empty()) {
        LifetimeStore::forEach(args,
                               "Passed to constructor of '" + name + "'.",
                               ValueFlow::Value::LifetimeKind::SubObject,
                               [&](LifetimeStore& ls) {
            ls.inconclusive = true;
            const Variable* var = argToParam.at(ls.argtok);
            if (var && !var->isConst() && var->isReference())
                ls.byRef(tok, tokenlist, errorLogger, settings);
            else
                ls.byVal(tok, tokenlist, errorLogger, settings);
        });
    }
}

static void valueFlowLifetimeFunction(Token *tok, TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings)
{
    if (!Token::Match(tok, "%name% ("))
        return;
    Token* memtok = nullptr;
    if (Token::Match(tok->astParent(), ". %name% (") && astIsRHS(tok))
        memtok = tok->astParent()->astOperand1();
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
            bool forward = false;
            for (ValueFlow::Value val : argTok->values()) {
                if (!val.isLifetimeValue())
                    continue;
                val.errorPath.emplace_back(argTok, "Passed to '" + tok->str() + "'.");
                setTokenValue(tok->next(), val, settings);
                forward = true;
            }
            // Check if lifetime is available to avoid adding the lifetime twice
            if (forward) {
                valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
                break;
            }
        }
    } else if (Token::Match(tok->tokAt(-2), "std :: ref|cref|tie|front_inserter|back_inserter")) {
        for (const Token *argtok : getArguments(tok)) {
            LifetimeStore{argtok, "Passed to '" + tok->str() + "'.", ValueFlow::Value::LifetimeKind::Object}.byRef(
                tok->next(), tokenlist, errorLogger, settings);
        }
    } else if (Token::Match(tok->tokAt(-2), "std :: make_tuple|tuple_cat|make_pair|make_reverse_iterator|next|prev|move|bind")) {
        for (const Token *argtok : getArguments(tok)) {
            LifetimeStore{argtok, "Passed to '" + tok->str() + "'.", ValueFlow::Value::LifetimeKind::Object}.byVal(
                tok->next(), tokenlist, errorLogger, settings);
        }
    } else if (memtok && Token::Match(tok->astParent(), ". push_back|push_front|insert|push|assign") &&
               astIsContainer(memtok)) {
        std::vector<const Token *> args = getArguments(tok);
        std::size_t n = args.size();
        if (n > 1 && Token::typeStr(args[n - 2]) == Token::typeStr(args[n - 1]) &&
            (((astIsIterator(args[n - 2]) && astIsIterator(args[n - 1])) ||
              (astIsPointer(args[n - 2]) && astIsPointer(args[n - 1]))))) {
            LifetimeStore{
                args.back(), "Added to container '" + memtok->str() + "'.", ValueFlow::Value::LifetimeKind::Object}
            .byDerefCopy(memtok, tokenlist, errorLogger, settings);
        } else if (!args.empty() && isLifetimeBorrowed(args.back(), settings)) {
            LifetimeStore{
                args.back(), "Added to container '" + memtok->str() + "'.", ValueFlow::Value::LifetimeKind::Object}
            .byVal(memtok, tokenlist, errorLogger, settings);
        }
    } else if (tok->function()) {
        const Function *f = tok->function();
        if (f->isConstructor()) {
            valueFlowLifetimeUserConstructor(tok->next(), f, tok->str(), getArguments(tok), tokenlist, errorLogger, settings);
            return;
        }
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
                if (memtok &&
                    (contains({ValueFlow::Value::LifetimeScope::ThisPointer, ValueFlow::Value::LifetimeScope::ThisValue},
                              v.lifetimeScope) ||
                     exprDependsOnThis(v.tokvalue))) {
                    LifetimeStore ls = LifetimeStore{memtok,
                                                     "Passed to member function '" + tok->expressionString() + "'.",
                                                     ValueFlow::Value::LifetimeKind::Object};
                    ls.inconclusive = inconclusive;
                    ls.forward = false;
                    ls.errorPath = v.errorPath;
                    ls.errorPath.emplace_front(returnTok, "Return " + lifetimeType(returnTok, &v) + ".");
                    if (v.lifetimeScope == ValueFlow::Value::LifetimeScope::ThisValue)
                        update |= ls.byVal(tok->next(), tokenlist, errorLogger, settings);
                    else
                        update |= ls.byRef(tok->next(), tokenlist, errorLogger, settings);
                    continue;
                }
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
    } else if (tok->valueType()) {
        // TODO: Propagate lifetimes with library functions
        if (settings->library.getFunction(tok->previous()))
            return;
        // Assume constructing the valueType
        valueFlowLifetimeConstructor(tok->next(), tokenlist, errorLogger, settings);
        valueFlowForwardLifetime(tok->next(), tokenlist, errorLogger, settings);
    }
}

static bool isScope(const Token* tok)
{
    if (!tok)
        return false;
    if (!Token::simpleMatch(tok, "{"))
        return false;
    const Scope* scope = tok->scope();
    if (!scope)
        return false;
    if (!scope->bodyStart)
        return false;
    return scope->bodyStart == tok;
}

static const Function* findConstructor(const Scope* scope, const Token* tok, const std::vector<const Token*>& args)
{
    if (!tok)
        return nullptr;
    const Function* f = tok->function();
    if (!f && tok->astOperand1())
        f = tok->astOperand1()->function();
    // Search for a constructor
    if (!f || !f->isConstructor()) {
        f = nullptr;
        std::vector<const Function*> candidates;
        for (const Function& function : scope->functionList) {
            if (function.argCount() != args.size())
                continue;
            if (!function.isConstructor())
                continue;
            candidates.push_back(&function);
        }
        // TODO: Narrow the candidates
        if (candidates.size() == 1)
            f = candidates.front();
    }
    if (!f)
        return nullptr;
    return f;
}

static void valueFlowLifetimeClassConstructor(Token* tok,
                                              const Type* t,
                                              TokenList* tokenlist,
                                              ErrorLogger* errorLogger,
                                              const Settings* settings)
{
    if (!Token::Match(tok, "(|{"))
        return;
    if (isScope(tok))
        return;
    if (!t) {
        if (tok->valueType() && tok->valueType()->type != ValueType::RECORD)
            return;
        if (tok->str() != "{" && !Token::Match(tok->previous(), "%var% (") && !isVariableDecl(tok->previous()))
            return;
        // If the type is unknown then assume it captures by value in the
        // constructor, but make each lifetime inconclusive
        std::vector<const Token*> args = getArguments(tok);
        LifetimeStore::forEach(args,
                               "Passed to initializer list.",
                               ValueFlow::Value::LifetimeKind::SubObject,
                               [&](LifetimeStore& ls) {
            ls.inconclusive = true;
            ls.byVal(tok, tokenlist, errorLogger, settings);
        });
        return;
    }
    const Scope* scope = t->classScope;
    if (!scope)
        return;
    // Only support aggregate constructors for now
    if (t->derivedFrom.empty() && (t->isClassType() || t->isStructType())) {
        std::vector<const Token*> args = getArguments(tok);
        if (scope->numConstructors == 0) {
            auto it = scope->varlist.begin();
            LifetimeStore::forEach(args,
                                   "Passed to constructor of '" + t->name() + "'.",
                                   ValueFlow::Value::LifetimeKind::SubObject,
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
        } else {
            const Function* constructor = findConstructor(scope, tok, args);
            valueFlowLifetimeUserConstructor(tok, constructor, t->name(), args, tokenlist, errorLogger, settings);
        }
    }
}

static void valueFlowLifetimeConstructor(Token* tok, TokenList* tokenlist, ErrorLogger* errorLogger, const Settings* settings)
{
    if (!Token::Match(tok, "(|{"))
        return;
    if (isScope(tok))
        return;
    std::vector<ValueType> vts;
    if (tok->valueType()) {
        vts = {*tok->valueType()};
    } else if (Token::Match(tok->previous(), "%var% {|(") && isVariableDecl(tok->previous()) &&
               tok->previous()->valueType()) {
        vts = {*tok->previous()->valueType()};
    } else if (Token::simpleMatch(tok, "{") && !Token::Match(tok->previous(), "%name%")) {
        vts = getParentValueTypes(tok, settings);
    }

    for (const ValueType& vt : vts) {
        if (vt.pointer > 0) {
            std::vector<const Token*> args = getArguments(tok);
            LifetimeStore::forEach(args,
                                   "Passed to initializer list.",
                                   ValueFlow::Value::LifetimeKind::SubObject,
                                   [&](const LifetimeStore& ls) {
                ls.byVal(tok, tokenlist, errorLogger, settings);
            });
        } else if (vt.container && vt.type == ValueType::CONTAINER) {
            std::vector<const Token*> args = getArguments(tok);
            if (args.size() == 1 && vt.container->view && astIsContainerOwned(args.front())) {
                LifetimeStore{args.front(), "Passed to container view.", ValueFlow::Value::LifetimeKind::SubObject}
                .byRef(tok, tokenlist, errorLogger, settings);
            } else if (args.size() == 2 && astIsIterator(args[0]) && astIsIterator(args[1])) {
                LifetimeStore::forEach(
                    args,
                    "Passed to initializer list.",
                    ValueFlow::Value::LifetimeKind::SubObject,
                    [&](const LifetimeStore& ls) {
                    ls.byDerefCopy(tok, tokenlist, errorLogger, settings);
                });
            } else if (vt.container->hasInitializerListConstructor) {
                LifetimeStore::forEach(args,
                                       "Passed to initializer list.",
                                       ValueFlow::Value::LifetimeKind::SubObject,
                                       [&](const LifetimeStore& ls) {
                    ls.byVal(tok, tokenlist, errorLogger, settings);
                });
            }
        } else {
            const Type* t = nullptr;
            if (vt.typeScope && vt.typeScope->definedType)
                t = vt.typeScope->definedType;
            else
                t = Token::typeOf(tok->previous());
            valueFlowLifetimeClassConstructor(tok, t, tokenlist, errorLogger, settings);
        }
    }
}

struct Lambda {
    explicit Lambda(const Token* tok)
        : capture(nullptr),
        arguments(nullptr),
        returnTok(nullptr),
        bodyTok(nullptr),
        explicitCaptures(),
        implicitCapture(LifetimeCapture::Undefined)
    {
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
            if (Token::Match(c, "this !!.")) {
                explicitCaptures[c->variable()] = std::make_pair(c, LifetimeCapture::ByReference);
            } else if (Token::simpleMatch(c, "* this")) {
                explicitCaptures[c->next()->variable()] = std::make_pair(c->next(), LifetimeCapture::ByValue);
            } else if (c->variable()) {
                explicitCaptures[c->variable()] = std::make_pair(c, LifetimeCapture::ByValue);
            } else if (c->isUnaryOp("&") && Token::Match(c->astOperand1(), "%var%")) {
                explicitCaptures[c->astOperand1()->variable()] =
                    std::make_pair(c->astOperand1(), LifetimeCapture::ByReference);
            } else {
                const std::string& s = c->expressionString();
                if (s == "=")
                    implicitCapture = LifetimeCapture::ByValue;
                else if (s == "&")
                    implicitCapture = LifetimeCapture::ByReference;
            }
        }
    }

    const Token * capture;
    const Token * arguments;
    const Token * returnTok;
    const Token * bodyTok;
    std::unordered_map<const Variable*, std::pair<const Token*, LifetimeCapture>> explicitCaptures;
    LifetimeCapture implicitCapture;

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

static bool isConvertedToView(const Token* tok, const Settings* settings)
{
    std::vector<ValueType> vtParents = getParentValueTypes(tok, settings);
    return std::any_of(vtParents.begin(), vtParents.end(), [&](const ValueType& vt) {
        if (!vt.container)
            return false;
        return vt.container->view;
    });
}

static void valueFlowLifetime(TokenList *tokenlist, SymbolDatabase*, ErrorLogger *errorLogger, const Settings *settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->scope())
            continue;
        if (tok->scope()->type == Scope::eGlobal)
            continue;
        Lambda lam(tok);
        // Lambdas
        if (lam.isLambda()) {
            const Scope * bodyScope = lam.bodyTok->scope();

            std::set<const Scope *> scopes;
            // Avoid capturing a variable twice
            std::set<nonneg int> varids;
            bool capturedThis = false;

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
            auto captureVariable = [&](const Token* tok2, LifetimeCapture c, std::function<bool(const Token*)> pred) {
                if (varids.count(tok->varId()) > 0)
                    return;
                ErrorPath errorPath;
                if (c == LifetimeCapture::ByReference) {
                    LifetimeStore ls{
                        tok2, "Lambda captures variable by reference here.", ValueFlow::Value::LifetimeKind::Lambda};
                    ls.forward = false;
                    update |= ls.byRef(tok, tokenlist, errorLogger, settings, pred);
                } else if (c == LifetimeCapture::ByValue) {
                    LifetimeStore ls{
                        tok2, "Lambda captures variable by value here.", ValueFlow::Value::LifetimeKind::Lambda};
                    ls.forward = false;
                    update |= ls.byVal(tok, tokenlist, errorLogger, settings, pred);
                    pred(tok2);
                }
            };

            auto captureThisVariable = [&](const Token* tok2, LifetimeCapture c) {
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::LIFETIME;
                if (c == LifetimeCapture::ByReference)
                    value.lifetimeScope = ValueFlow::Value::LifetimeScope::ThisPointer;
                else if (c == LifetimeCapture::ByValue)
                    value.lifetimeScope = ValueFlow::Value::LifetimeScope::ThisValue;
                value.tokvalue = tok2;
                value.errorPath.push_back({tok2, "Lambda captures the 'this' variable here."});
                value.lifetimeKind = ValueFlow::Value::LifetimeKind::Lambda;
                capturedThis = true;
                // Don't add the value a second time
                if (std::find(tok->values().begin(), tok->values().end(), value) != tok->values().end())
                    return;
                setTokenValue(tok, value, tokenlist->getSettings());
                update |= true;
            };

            // Handle explicit capture
            for (const auto& p:lam.explicitCaptures) {
                const Variable* var = p.first;
                const Token* tok2 = p.second.first;
                LifetimeCapture c = p.second.second;
                if (Token::Match(tok2, "this !!.")) {
                    captureThisVariable(tok2, c);
                } else if (var) {
                    captureVariable(tok2, c, [](const Token*) {
                        return true;
                    });
                    varids.insert(var->declarationId());
                }
            }

            auto isImplicitCapturingThis = [&](const Token* tok2) {
                if (capturedThis)
                    return false;
                if (Token::simpleMatch(tok2, "this")) {
                    return true;
                } else if (tok2->variable()) {
                    if (Token::simpleMatch(tok2->previous(), "."))
                        return false;
                    const Variable* var = tok2->variable();
                    if (var->isLocal())
                        return false;
                    if (var->isArgument())
                        return false;
                    return exprDependsOnThis(tok2);
                } else if (Token::simpleMatch(tok2, "(")) {
                    return exprDependsOnThis(tok2);
                }
                return false;
            };

            for (const Token * tok2 = lam.bodyTok; tok2 != lam.bodyTok->link(); tok2 = tok2->next()) {
                if (isImplicitCapturingThis(tok2)) {
                    captureThisVariable(tok2, LifetimeCapture::ByReference);
                } else if (tok2->variable()) {
                    captureVariable(tok2, lam.implicitCapture, isImplicitCapturingVariable);
                }
            }
            if (update)
                valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
        }
        // address of
        else if (tok->isUnaryOp("&")) {
            for (const LifetimeToken& lt : getLifetimeTokens(tok->astOperand1())) {
                if (!settings->certainty.isEnabled(Certainty::inconclusive) && lt.inconclusive)
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
        // Converting to container view
        else if (astIsContainerOwned(tok) && isConvertedToView(tok, settings)) {
            LifetimeStore ls =
                LifetimeStore{tok, "Converted to container view", ValueFlow::Value::LifetimeKind::SubObject};
            ls.byRef(tok, tokenlist, errorLogger, settings);
            valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
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

            ValueFlow::Value master;
            master.valueType = ValueFlow::Value::ValueType::LIFETIME;
            master.lifetimeScope = ValueFlow::Value::LifetimeScope::Local;

            if (astIsIterator(parent->tokAt(2))) {
                master.errorPath.emplace_back(parent->tokAt(2), "Iterator to container is created here.");
                master.lifetimeKind = ValueFlow::Value::LifetimeKind::Iterator;
            } else if ((astIsPointer(parent->tokAt(2)) && !isContainerOfPointers) ||
                       Token::Match(parent->next(), "data|c_str")) {
                master.errorPath.emplace_back(parent->tokAt(2), "Pointer to container is created here.");
                master.lifetimeKind = ValueFlow::Value::LifetimeKind::Object;
            } else {
                continue;
            }

            std::vector<const Token*> toks = {};
            if (tok->isUnaryOp("*") || parent->originalName() == "->") {
                for (const ValueFlow::Value& v : tok->values()) {
                    if (!v.isLocalLifetimeValue())
                        continue;
                    if (v.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
                        continue;
                    if (!v.tokvalue)
                        continue;
                    toks.push_back(v.tokvalue);
                }
            } else if (astIsContainerView(tok)) {
                for (const ValueFlow::Value& v : tok->values()) {
                    if (!v.isLifetimeValue())
                        continue;
                    if (!v.tokvalue)
                        continue;
                    if (!astIsContainerOwned(v.tokvalue))
                        continue;
                    toks.push_back(v.tokvalue);
                }
            } else {
                toks = {tok};
            }

            for (const Token* tok2 : toks) {
                for (const ReferenceToken& rt : followAllReferences(tok2, false)) {
                    ValueFlow::Value value = master;
                    value.tokvalue = rt.token;
                    value.errorPath.insert(value.errorPath.begin(), rt.errors.begin(), rt.errors.end());
                    setTokenValue(parent->tokAt(2), value, tokenlist->getSettings());

                    if (!rt.token->variable()) {
                        LifetimeStore ls = LifetimeStore{
                            rt.token, master.errorPath.back().second, ValueFlow::Value::LifetimeKind::Object};
                        ls.byRef(parent->tokAt(2), tokenlist, errorLogger, settings);
                    }
                }
            }
            valueFlowForwardLifetime(parent->tokAt(2), tokenlist, errorLogger, settings);
        }
        // Check constructors
        else if (Token::Match(tok, "=|return|%name%|{|,|> {") && !isScope(tok->next())) {
            valueFlowLifetimeConstructor(tok->next(), tokenlist, errorLogger, settings);
        }
        // Check function calls
        else if (Token::Match(tok, "%name% (") && !Token::simpleMatch(tok->next()->link(), ") {")) {
            valueFlowLifetimeFunction(tok, tokenlist, errorLogger, settings);
        }
        // Unique pointer lifetimes
        else if (astIsUniqueSmartPointer(tok) && astIsLHS(tok) && Token::simpleMatch(tok->astParent(), ". get ( )")) {
            Token* ptok = tok->astParent()->tokAt(2);
            ErrorPath errorPath = {{ptok, "Raw pointer to smart pointer created here."}};
            ValueFlow::Value value;
            value.valueType = ValueFlow::Value::ValueType::LIFETIME;
            value.lifetimeScope = ValueFlow::Value::LifetimeScope::Local;
            value.lifetimeKind = ValueFlow::Value::LifetimeKind::SubObject;
            value.tokvalue = tok;
            value.errorPath = errorPath;
            setTokenValue(ptok, value, tokenlist->getSettings());
            valueFlowForwardLifetime(ptok, tokenlist, errorLogger, settings);
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
           varTok->next()->originalName().empty();
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

static void valueFlowAfterMove(TokenList* tokenlist, SymbolDatabase* symboldatabase, const Settings* settings)
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
            if (Token::Match(tok, "%var% . reset|clear (") && tok->next()->originalName().empty()) {
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
                const Token * const endOfVarScope = var->scope()->bodyEnd;
                setTokenValue(varTok, value, settings);
                valueFlowForward(varTok->next(), endOfVarScope, varTok, values, tokenlist);
                continue;
            }
            ValueFlow::Value::MoveKind moveKind;
            if (!isStdMoveOrStdForwarded(tok, &moveKind, &varTok))
                continue;
            const nonneg int varId = varTok->varId();
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
                valueFlowForward(const_cast<Token*>(endOfFunctionCall), endOfVarScope, varTok, values, tokenlist);
        }
    }
}

static const Token* findIncompleteVar(const Token* start, const Token* end)
{
    for (const Token* tok = start; tok != end; tok = tok->next()) {
        if (tok->isIncompleteVar())
            return tok;
    }
    return nullptr;
}

static ValueFlow::Value makeConditionValue(long long val,
                                           const Token* condTok,
                                           bool assume,
                                           const Settings* settings = nullptr,
                                           SourceLocation loc = SourceLocation::current())
{
    ValueFlow::Value v(val);
    v.setKnown();
    v.condition = condTok;
    if (assume)
        v.errorPath.emplace_back(condTok, "Assuming condition '" + condTok->expressionString() + "' is true");
    else
        v.errorPath.emplace_back(condTok, "Assuming condition '" + condTok->expressionString() + "' is false");
    if (settings && settings->debugnormal)
        setSourceLocation(v, loc, condTok);
    return v;
}

static std::vector<const Token*> getConditions(const Token* tok, const char* op)
{
    std::vector<const Token*> conds = {tok};
    if (tok->str() == op) {
        std::vector<const Token*> args = astFlatten(tok, op);
        std::copy_if(args.begin(), args.end(), std::back_inserter(conds), [&](const Token* tok2) {
            if (tok2->exprId() == 0)
                return false;
            if (tok2->hasKnownIntValue())
                return false;
            if (Token::Match(tok2, "%var%|.") && !astIsBool(tok2))
                return false;
            return true;
        });
    }
    return conds;
}

static bool isBreakOrContinueScope(const Token* endToken)
{
    if (!Token::simpleMatch(endToken, "}"))
        return false;
    return Token::Match(endToken->tokAt(-2), "break|continue ;");
}

static const Scope* getLoopScope(const Token* tok)
{
    if (!tok)
        return nullptr;
    const Scope* scope = tok->scope();
    while (scope && scope->type != Scope::eWhile && scope->type != Scope::eFor && scope->type != Scope::eDo)
        scope = scope->nestedIn;
    return scope;
}

//
static void valueFlowConditionExpressions(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope * scope : symboldatabase->functionScopes) {
        if (const Token* incompleteTok = findIncompleteVar(scope->bodyStart, scope->bodyEnd)) {
            if (incompleteTok->isIncompleteVar()) {
                if (settings->debugwarnings)
                    bailoutIncompleteVar(tokenlist, errorLogger, incompleteTok, "Skipping function due to incomplete variable " + incompleteTok->str());
                break;
            }
        }

        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "if ("))
                continue;
            Token * parenTok = tok->next();
            if (!Token::simpleMatch(parenTok->link(), ") {"))
                continue;
            Token * blockTok = parenTok->link()->tokAt(1);
            const Token* condTok = parenTok->astOperand2();
            if (condTok->exprId() == 0)
                continue;
            if (condTok->hasKnownIntValue())
                continue;
            if (!isConstExpression(condTok, settings->library, true, tokenlist->isCPP()))
                continue;
            const bool is1 = (condTok->isComparisonOp() || condTok->tokType() == Token::eLogicalOp || astIsBool(condTok));

            Token* startTok = blockTok;
            // Inner condition
            {
                for (const Token* condTok2 : getConditions(condTok, "&&")) {
                    if (is1) {
                        ExpressionAnalyzer a1(condTok2, makeConditionValue(1, condTok2, true), tokenlist);
                        valueFlowGenericForward(startTok, startTok->link(), a1, settings);
                    }

                    OppositeExpressionAnalyzer a2(true, condTok2, makeConditionValue(0, condTok2, true), tokenlist);
                    valueFlowGenericForward(startTok, startTok->link(), a2, settings);
                }
            }

            std::vector<const Token*> conds = getConditions(condTok, "||");

            // Check else block
            if (Token::simpleMatch(startTok->link(), "} else {")) {
                startTok = startTok->link()->tokAt(2);
                for (const Token* condTok2:conds) {
                    ExpressionAnalyzer a1(condTok2, makeConditionValue(0, condTok2, false), tokenlist);
                    valueFlowGenericForward(startTok, startTok->link(), a1, settings);

                    if (is1) {
                        OppositeExpressionAnalyzer a2(true, condTok2, makeConditionValue(1, condTok2, false), tokenlist);
                        valueFlowGenericForward(startTok, startTok->link(), a2, settings);
                    }
                }
            }

            // Check if the block terminates early
            if (isEscapeScope(blockTok, tokenlist)) {
                const Scope* scope2 = scope;
                // If escaping a loop then only use the loop scope
                if (isBreakOrContinueScope(blockTok->link())) {
                    scope2 = getLoopScope(blockTok->link());
                    if (!scope2)
                        continue;
                }
                for (const Token* condTok2:conds) {
                    ExpressionAnalyzer a1(condTok2, makeConditionValue(0, condTok2, false), tokenlist);
                    valueFlowGenericForward(startTok->link()->next(), scope2->bodyEnd, a1, settings);

                    if (is1) {
                        OppositeExpressionAnalyzer a2(true, condTok2, makeConditionValue(1, condTok2, false), tokenlist);
                        valueFlowGenericForward(startTok->link()->next(), scope2->bodyEnd, a2, settings);
                    }
                }
            }

        }
    }
}

static bool isTruncated(const ValueType* src, const ValueType* dst, const Settings* settings)
{
    if (src->pointer > 0 || dst->pointer > 0)
        return src->pointer != dst->pointer;
    if (src->smartPointer && dst->smartPointer)
        return false;
    if ((src->isIntegral() && dst->isIntegral()) || (src->isFloat() && dst->isFloat())) {
        size_t srcSize = ValueFlow::getSizeOf(*src, settings);
        size_t dstSize = ValueFlow::getSizeOf(*dst, settings);
        if (srcSize > dstSize)
            return true;
        if (srcSize == dstSize && src->sign != dst->sign)
            return true;
    } else if (src->type == dst->type) {
        if (src->type == ValueType::Type::RECORD)
            return src->typeScope != dst->typeScope;
    } else {
        return true;
    }
    return false;
}

static void setSymbolic(ValueFlow::Value& value, const Token* tok)
{
    assert(tok && tok->exprId() > 0 && "Missing expr id for symbolic value");
    value.valueType = ValueFlow::Value::ValueType::SYMBOLIC;
    value.tokvalue = tok;
}

static ValueFlow::Value makeSymbolic(const Token* tok, MathLib::bigint delta = 0)
{
    ValueFlow::Value value;
    value.setKnown();
    setSymbolic(value, tok);
    value.intvalue = delta;
    return value;
}

static std::set<nonneg int> getVarIds(const Token* tok)
{
    std::set<nonneg int> result;
    visitAstNodes(tok, [&](const Token* child) {
        if (child->varId() > 0)
            result.insert(child->varId());
        return ChildrenToVisit::op1_and_op2;
    });
    return result;
}

static void valueFlowSymbolic(TokenList* tokenlist, SymbolDatabase* symboldatabase)
{
    for (const Scope* scope : symboldatabase->functionScopes) {
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "="))
                continue;
            if (tok->astParent())
                continue;
            if (!tok->astOperand1())
                continue;
            if (!tok->astOperand2())
                continue;
            if (tok->astOperand1()->hasKnownIntValue())
                continue;
            if (tok->astOperand2()->hasKnownIntValue())
                continue;
            if (tok->astOperand1()->exprId() == 0)
                continue;
            if (tok->astOperand2()->exprId() == 0)
                continue;
            if (!isConstExpression(tok->astOperand2(), tokenlist->getSettings()->library, true, tokenlist->isCPP()))
                continue;
            if (tok->astOperand1()->valueType() && tok->astOperand2()->valueType()) {
                if (isTruncated(
                        tok->astOperand2()->valueType(), tok->astOperand1()->valueType(), tokenlist->getSettings()))
                    continue;
            } else if (isDifferentType(tok->astOperand2(), tok->astOperand1())) {
                continue;
            }
            const std::set<nonneg int> rhsVarIds = getVarIds(tok->astOperand2());
            const std::vector<const Variable*> vars = getLHSVariables(tok);
            if (std::any_of(vars.begin(), vars.end(), [&](const Variable* var) {
                if (rhsVarIds.count(var->declarationId()) > 0)
                    return true;
                if (var->isLocal())
                    return var->isStatic();
                return !var->isArgument();
            }))
                continue;

            if (findAstNode(tok, [](const Token* child) {
                return child->isIncompleteVar();
            }))
                continue;

            Token* start = nextAfterAstRightmostLeaf(tok);
            const Token* end = getEndOfExprScope(tok->astOperand1(), scope);

            ValueFlow::Value rhs = makeSymbolic(tok->astOperand2());
            rhs.errorPath.emplace_back(tok,
                                       tok->astOperand1()->expressionString() + " is assigned '" +
                                       tok->astOperand2()->expressionString() + "' here.");
            valueFlowForward(start, end, tok->astOperand1(), rhs, tokenlist);

            ValueFlow::Value lhs = makeSymbolic(tok->astOperand1());
            lhs.errorPath.emplace_back(tok,
                                       tok->astOperand1()->expressionString() + " is assigned '" +
                                       tok->astOperand2()->expressionString() + "' here.");
            valueFlowForward(start, end, tok->astOperand2(), lhs, tokenlist);
        }
    }
}

static void valueFlowSymbolicIdentity(TokenList* tokenlist)
{
    for (Token* tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->hasKnownIntValue())
            continue;
        if (!Token::Match(tok, "*|/|<<|>>|^|+|-|%or%"))
            continue;
        if (!tok->astOperand1())
            continue;
        if (!tok->astOperand2())
            continue;
        if (!astIsIntegral(tok->astOperand1(), false) && !astIsIntegral(tok->astOperand2(), false))
            continue;
        const ValueFlow::Value* constant = nullptr;
        const Token* vartok = nullptr;
        if (tok->astOperand1()->hasKnownIntValue()) {
            constant = &tok->astOperand1()->values().front();
            vartok = tok->astOperand2();
        }
        if (tok->astOperand2()->hasKnownIntValue()) {
            constant = &tok->astOperand2()->values().front();
            vartok = tok->astOperand1();
        }
        if (!constant)
            continue;
        if (!vartok)
            continue;
        if (vartok->exprId() == 0)
            continue;
        if (Token::Match(tok, "<<|>>|/") && !astIsLHS(vartok))
            continue;
        if (Token::Match(tok, "<<|>>|^|+|-|%or%") && constant->intvalue != 0)
            continue;
        if (Token::Match(tok, "*|/") && constant->intvalue != 1)
            continue;
        std::vector<ValueFlow::Value> values = {makeSymbolic(vartok)};
        std::unordered_set<nonneg int> ids = {vartok->exprId()};
        std::copy_if(
            vartok->values().begin(), vartok->values().end(), std::back_inserter(values), [&](const ValueFlow::Value& v) {
            if (!v.isSymbolicValue())
                return false;
            if (!v.tokvalue)
                return false;
            return ids.insert(v.tokvalue->exprId()).second;
        });
        for (const ValueFlow::Value& v : values)
            setTokenValue(tok, v, tokenlist->getSettings());
    }
}

static void valueFlowSymbolicAbs(TokenList* tokenlist, SymbolDatabase* symboldatabase)
{
    for (const Scope* scope : symboldatabase->functionScopes) {
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "abs|labs|llabs|fabs|fabsf|fabsl ("))
                continue;
            if (tok->hasKnownIntValue())
                continue;

            const Token* arg = tok->next()->astOperand2();
            if (!arg)
                continue;
            ValueFlow::Value c = inferCondition(">=", arg, 0);
            if (!c.isKnown())
                continue;

            ValueFlow::Value v = makeSymbolic(arg);
            v.errorPath = c.errorPath;
            v.errorPath.emplace_back(tok, "Passed to " + tok->str());
            if (c.intvalue == 0)
                v.setImpossible();
            else
                v.setKnown();
            setTokenValue(tok->next(), v, tokenlist->getSettings());
        }
    }
}

struct SymbolicInferModel : InferModel {
    const Token* expr;
    explicit SymbolicInferModel(const Token* tok) : expr(tok) {
        assert(expr->exprId() != 0);
    }
    bool match(const ValueFlow::Value& value) const override
    {
        return value.isSymbolicValue() && value.tokvalue && value.tokvalue->exprId() == expr->exprId();
    }
    ValueFlow::Value yield(MathLib::bigint value) const override
    {
        ValueFlow::Value result(value);
        result.valueType = ValueFlow::Value::ValueType::SYMBOLIC;
        result.tokvalue = expr;
        result.setKnown();
        return result;
    }
};

static void valueFlowSymbolicInfer(TokenList* tokenlist, SymbolDatabase* symboldatabase)
{
    for (const Scope* scope : symboldatabase->functionScopes) {
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::Match(tok, "-|%comp%"))
                continue;
            if (tok->hasKnownIntValue())
                continue;
            if (!tok->astOperand1())
                continue;
            if (!tok->astOperand2())
                continue;
            if (tok->astOperand1()->exprId() == 0)
                continue;
            if (tok->astOperand2()->exprId() == 0)
                continue;
            if (tok->astOperand1()->hasKnownIntValue())
                continue;
            if (tok->astOperand2()->hasKnownIntValue())
                continue;
            if (astIsFloat(tok->astOperand1(), false))
                continue;
            if (astIsFloat(tok->astOperand2(), false))
                continue;

            SymbolicInferModel leftModel{tok->astOperand1()};
            std::vector<ValueFlow::Value> values = infer(leftModel, tok->str(), 0, tok->astOperand2()->values());
            if (values.empty()) {
                SymbolicInferModel rightModel{tok->astOperand2()};
                values = infer(rightModel, tok->str(), tok->astOperand1()->values(), 0);
            }
            for (const ValueFlow::Value& value : values) {
                setTokenValue(tok, value, tokenlist->getSettings());
            }
        }
    }
}

template<class ContainerOfValue>
static void valueFlowForwardConst(Token* start,
                                  const Token* end,
                                  const Variable* var,
                                  const ContainerOfValue& values,
                                  const Settings* const settings)
{
    for (Token* tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() == var->declarationId()) {
            for (const ValueFlow::Value& value : values)
                setTokenValue(tok, value, settings);
        } else {
            [&] {
                // Follow references
                std::vector<ReferenceToken> refs = followAllReferences(tok);
                for (const ReferenceToken& ref : refs) {
                    if (ref.token->varId() == var->declarationId()) {
                        for (ValueFlow::Value value : values) {
                            if (refs.size() > 1)
                                value.setInconclusive();
                            value.errorPath.insert(value.errorPath.end(), ref.errors.begin(), ref.errors.end());
                            setTokenValue(tok, value, settings);
                        }
                        return;
                    }
                }
                // Follow symbolic vaues
                for (const ValueFlow::Value& v : tok->values()) {
                    if (!v.isSymbolicValue())
                        continue;
                    if (!v.tokvalue)
                        continue;
                    if (v.tokvalue->varId() != var->declarationId())
                        continue;
                    for (ValueFlow::Value value : values) {
                        if (v.intvalue != 0) {
                            if (!value.isIntValue())
                                continue;
                            value.intvalue += v.intvalue;
                        }
                        value.valueKind = v.valueKind;
                        value.bound = v.bound;
                        value.errorPath.insert(value.errorPath.end(), v.errorPath.begin(), v.errorPath.end());
                        setTokenValue(tok, value, settings);
                    }
                }
            }();
        }
    }
}

static void valueFlowForwardAssign(Token* const tok,
                                   const Token* expr,
                                   std::vector<const Variable*> vars,
                                   std::list<ValueFlow::Value> values,
                                   const bool init,
                                   TokenList* const tokenlist,
                                   ErrorLogger* const errorLogger,
                                   const Settings* const settings)
{
    if (Token::simpleMatch(tok->astParent(), "return"))
        return;
    const Token* endOfVarScope = getEndOfExprScope(expr);
    if (std::any_of(values.begin(), values.end(), std::mem_fn(&ValueFlow::Value::isLifetimeValue))) {
        valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
        values.remove_if(std::mem_fn(&ValueFlow::Value::isLifetimeValue));
    }
    if (std::all_of(
            vars.begin(), vars.end(), [&](const Variable* var) {
        return !var->isPointer() && !var->isSmartPointer();
    }))
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
    if (tok->astParent()) {
        for (ValueFlow::Value& value : values) {
            std::string valueKind;
            if (value.valueKind == ValueFlow::Value::ValueKind::Impossible) {
                if (value.bound == ValueFlow::Value::Bound::Point)
                    valueKind = "never ";
                else if (value.bound == ValueFlow::Value::Bound::Lower)
                    valueKind = "less than ";
                else if (value.bound == ValueFlow::Value::Bound::Upper)
                    valueKind = "greater than ";
            }
            const std::string info = "Assignment '" + tok->astParent()->expressionString() + "', assigned value is " + valueKind + value.infoString();
            value.errorPath.emplace_back(tok, info);
        }
    }

    if (tokenlist->isCPP() && vars.size() == 1 && Token::Match(vars.front()->typeStartToken(), "bool|_Bool")) {
        for (ValueFlow::Value& value : values) {
            if (value.isImpossible())
                continue;
            if (value.isIntValue())
                value.intvalue = (value.intvalue != 0);
            if (value.isTokValue())
                value.intvalue = (value.tokvalue != nullptr);
        }
    }

    // Static variable initialisation?
    if (vars.size() == 1 && vars.front()->isStatic() && init)
        lowerToPossible(values);

    // is volatile
    if (std::any_of(vars.begin(), vars.end(), [&](const Variable* var) {
        return var->isVolatile();
    }))
        lowerToPossible(values);

    // Skip RHS
    const Token * nextExpression = tok->astParent() ? nextAfterAstRightmostLeaf(tok->astParent()) : tok->next();
    if (!nextExpression)
        return;

    for (ValueFlow::Value& value : values) {
        if (value.isSymbolicValue())
            continue;
        if (value.isTokValue())
            continue;
        value.tokvalue = tok;
    }
    // Const variable
    if (expr->variable() && expr->variable()->isConst() && !expr->variable()->isReference()) {
        auto it = std::remove_if(values.begin(), values.end(), [](const ValueFlow::Value& value) {
            if (!value.isKnown())
                return false;
            if (value.isIntValue())
                return true;
            if (value.isFloatValue())
                return true;
            if (value.isContainerSizeValue())
                return true;
            if (value.isIteratorValue())
                return true;
            return false;
        });
        std::list<ValueFlow::Value> constValues;
        constValues.splice(constValues.end(), values, it, values.end());
        valueFlowForwardConst(const_cast<Token*>(nextExpression), endOfVarScope, expr->variable(), constValues, settings);
    }
    valueFlowForward(const_cast<Token*>(nextExpression), endOfVarScope, expr, values, tokenlist);
}

static void valueFlowForwardAssign(Token* const tok,
                                   const Variable* const var,
                                   const std::list<ValueFlow::Value>& values,
                                   const bool,
                                   const bool init,
                                   TokenList* const tokenlist,
                                   ErrorLogger* const errorLogger,
                                   const Settings* const settings)
{
    valueFlowForwardAssign(tok, var->nameToken(), {var}, values, init, tokenlist, errorLogger, settings);
}

static std::list<ValueFlow::Value> truncateValues(std::list<ValueFlow::Value> values,
                                                  const ValueType* dst,
                                                  const ValueType* src,
                                                  const Settings* settings)
{
    if (!dst || !dst->isIntegral())
        return values;

    const size_t sz = ValueFlow::getSizeOf(*dst, settings);

    if (src) {
        const size_t osz = ValueFlow::getSizeOf(*src, settings);
        if (osz >= sz && dst->sign == ValueType::Sign::SIGNED && src->sign == ValueType::Sign::UNSIGNED) {
            values.remove_if([&](const ValueFlow::Value& value) {
                if (!value.isIntValue())
                    return false;
                if (!value.isImpossible())
                    return false;
                if (value.bound != ValueFlow::Value::Bound::Upper)
                    return false;
                if (osz == sz && value.intvalue < 0)
                    return true;
                if (osz > sz)
                    return true;
                return false;
            });
        }
    }

    for (ValueFlow::Value &value : values) {
        // Don't truncate impossible values since those can be outside of the valid range
        if (value.isImpossible())
            continue;
        if (value.isFloatValue()) {
            value.intvalue = value.floatValue;
            value.valueType = ValueFlow::Value::ValueType::INT;
        }

        if (value.isIntValue() && sz > 0 && sz < 8) {
            const MathLib::biguint unsignedMaxValue = (1ULL << (sz * 8)) - 1ULL;
            const MathLib::biguint signBit = 1ULL << (sz * 8 - 1);
            value.intvalue &= unsignedMaxValue;
            if (dst->sign == ValueType::Sign::SIGNED && (value.intvalue & signBit))
                value.intvalue |= ~unsignedMaxValue;
        }
    }
    return values;
}

static bool isVariableInit(const Token *tok)
{
    return (tok->str() == "(" || tok->str() == "{") &&
           tok->isBinaryOp() &&
           tok->astOperand1()->variable() &&
           tok->astOperand1()->variable()->nameToken() == tok->astOperand1() &&
           tok->astOperand1()->variable()->valueType() &&
           tok->astOperand1()->variable()->valueType()->type >= ValueType::Type::VOID &&
           !Token::simpleMatch(tok->astOperand2(), ",");
}

// Return true if two associative containers intersect
template<class C1, class C2>
static bool intersects(const C1& c1, const C2& c2)
{
    if (c1.size() > c2.size())
        return intersects(c2, c1);
    for (auto&& x : c1) {
        if (c2.find(x) != c2.end())
            return true;
    }
    return false;
}

static void valueFlowAfterAssign(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings)
{
    for (const Scope * scope : symboldatabase->functionScopes) {
        std::unordered_map<nonneg int, std::unordered_set<nonneg int>> backAssigns;
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            // Assignment
            if (tok->str() != "=" && !isVariableInit(tok))
                continue;

            if (tok->astParent() && !(tok->astParent()->str() == ";" && astIsLHS(tok)))
                continue;

            // Lhs should be a variable
            if (!tok->astOperand1() || !tok->astOperand1()->exprId())
                continue;
            std::vector<const Variable*> vars = getLHSVariables(tok);

            // Rhs values..
            if (!tok->astOperand2() || tok->astOperand2()->values().empty())
                continue;

            std::list<ValueFlow::Value> values = truncateValues(
                tok->astOperand2()->values(), tok->astOperand1()->valueType(), tok->astOperand2()->valueType(), settings);
            // Remove known values
            std::set<ValueFlow::Value::ValueType> types;
            if (tok->astOperand1()->hasKnownValue()) {
                for (const ValueFlow::Value& value:tok->astOperand1()->values()) {
                    if (value.isKnown() && !value.isSymbolicValue())
                        types.insert(value.valueType);
                }
            }
            values.remove_if([&](const ValueFlow::Value& value) {
                return types.count(value.valueType) > 0;
            });
            // Remove container size if its not a container
            if (!astIsContainer(tok->astOperand2()))
                values.remove_if([&](const ValueFlow::Value& value) {
                    return value.valueType == ValueFlow::Value::ValueType::CONTAINER_SIZE;
                });
            // Remove symbolic values that are the same as the LHS
            values.remove_if([&](const ValueFlow::Value& value) {
                if (value.isSymbolicValue() && value.tokvalue)
                    return value.tokvalue->exprId() == tok->astOperand1()->exprId();
                return false;
            });
            // If assignment copy by value, remove Uninit values..
            if ((tok->astOperand1()->valueType() && tok->astOperand1()->valueType()->pointer == 0) ||
                (tok->astOperand1()->variable() && tok->astOperand1()->variable()->isReference() && tok->astOperand1()->variable()->nameToken() == tok->astOperand1()))
                values.remove_if([&](const ValueFlow::Value& value) {
                    return value.isUninitValue();
                });
            if (values.empty())
                continue;
            const bool init = vars.size() == 1 && (vars.front()->nameToken() == tok->astOperand1() || tok->isSplittedVarDeclEq());
            valueFlowForwardAssign(
                tok->astOperand2(), tok->astOperand1(), vars, values, init, tokenlist, errorLogger, settings);
            // Back propagate symbolic values
            if (tok->astOperand1()->exprId() > 0) {
                Token* start = nextAfterAstRightmostLeaf(tok);
                const Token* end = scope->bodyEnd;
                // Collect symbolic ids
                std::unordered_set<nonneg int> ids;
                for (const ValueFlow::Value& value : values) {
                    if (!value.isSymbolicValue())
                        continue;
                    if (!value.tokvalue)
                        continue;
                    if (value.tokvalue->exprId() == 0)
                        continue;
                    ids.insert(value.tokvalue->exprId());
                }
                for (ValueFlow::Value value : values) {
                    if (!value.isSymbolicValue())
                        continue;
                    const Token* expr = value.tokvalue;
                    value.intvalue = -value.intvalue;
                    value.tokvalue = tok->astOperand1();

                    // Skip if it intersects with an already assigned symbol
                    auto& s = backAssigns[value.tokvalue->exprId()];
                    if (intersects(s, ids))
                        continue;
                    s.insert(expr->exprId());

                    value.errorPath.emplace_back(tok,
                                                 tok->astOperand1()->expressionString() + " is assigned '" +
                                                 tok->astOperand2()->expressionString() + "' here.");
                    valueFlowForward(start, end, expr, value, tokenlist);
                }
            }
        }
    }
}

static std::vector<const Variable*> getVariables(const Token* tok)
{
    std::vector<const Variable*> result;
    visitAstNodes(tok, [&](const Token* child) {
        if (child->variable())
            result.push_back(child->variable());
        return ChildrenToVisit::op1_and_op2;
    });
    return result;
}

static void valueFlowAfterSwap(TokenList* tokenlist,
                               SymbolDatabase* symboldatabase,
                               ErrorLogger* errorLogger,
                               const Settings* settings)
{
    for (const Scope* scope : symboldatabase->functionScopes) {
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            if (!Token::simpleMatch(tok, "swap ("))
                continue;
            if (!Token::simpleMatch(tok->next()->astOperand2(), ","))
                continue;
            std::vector<Token*> args = astFlatten(tok->next()->astOperand2(), ",");
            if (args.size() != 2)
                continue;
            if (args[0]->exprId() == 0)
                continue;
            if (args[1]->exprId() == 0)
                continue;
            for (int i = 0; i < 2; i++) {
                std::vector<const Variable*> vars = getVariables(args[0]);
                std::list<ValueFlow::Value> values = args[0]->values();
                valueFlowForwardAssign(args[0], args[1], vars, values, false, tokenlist, errorLogger, settings);
                std::swap(args[0], args[1]);
            }
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

ValueFlow::Value asImpossible(ValueFlow::Value v)
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

struct ConditionHandler {
    struct Condition {
        const Token *vartok;
        std::list<ValueFlow::Value> true_values;
        std::list<ValueFlow::Value> false_values;
        bool inverted = false;
        // Whether to insert impossible values for the condition or only use possible values
        bool impossible = true;

        bool isBool() const {
            return astIsBool(vartok);
        }

        static MathLib::bigint findPath(const std::list<ValueFlow::Value>& values)
        {
            auto it = std::find_if(values.begin(), values.end(), [](const ValueFlow::Value& v) {
                return v.path > 0;
            });
            if (it == values.end())
                return 0;
            assert(std::all_of(it, values.end(), [&](const ValueFlow::Value& v) {
                return v.path == 0 || v.path == it->path;
            }));
            return it->path;
        }

        MathLib::bigint getPath() const
        {
            assert(std::abs(findPath(true_values) - findPath(false_values)) == 0);
            return findPath(true_values) | findPath(false_values);
        }

        Condition() : vartok(nullptr), true_values(), false_values(), inverted(false), impossible(true) {}
    };

    virtual std::vector<Condition> parse(const Token* tok, const Settings* settings) const = 0;

    virtual Analyzer::Result forward(Token* start,
                                     const Token* stop,
                                     const Token* exprTok,
                                     const std::list<ValueFlow::Value>& values,
                                     TokenList* tokenlist,
                                     SourceLocation loc = SourceLocation::current()) const
    {
        return valueFlowForward(start->next(), stop, exprTok, values, tokenlist, loc);
    }

    virtual Analyzer::Result forward(Token* top,
                                     const Token* exprTok,
                                     const std::list<ValueFlow::Value>& values,
                                     TokenList* tokenlist,
                                     SourceLocation loc = SourceLocation::current()) const
    {
        return valueFlowForwardRecursive(top, exprTok, values, tokenlist, loc);
    }

    virtual void reverse(Token* start,
                         const Token* endToken,
                         const Token* exprTok,
                         const std::list<ValueFlow::Value>& values,
                         TokenList* tokenlist,
                         SourceLocation loc = SourceLocation::current()) const
    {
        return valueFlowReverse(start, endToken, exprTok, values, tokenlist, loc);
    }

    void traverseCondition(TokenList* tokenlist,
                           SymbolDatabase* symboldatabase,
                           const std::function<void(const Condition& cond, Token* tok, const Scope* scope)>& f) const
    {
        for (const Scope *scope : symboldatabase->functionScopes) {
            for (Token *tok = const_cast<Token *>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
                if (Token::Match(tok, "if|while|for ("))
                    continue;
                if (Token::Match(tok, ":|;|,"))
                    continue;

                const Token* top = tok->astTop();
                if (!top)
                    continue;

                if (!Token::Match(top->previous(), "if|while|for (") && !Token::Match(tok->astParent(), "&&|%oror%|?|!"))
                    continue;
                for (const Condition& cond : parse(tok, tokenlist->getSettings())) {
                    if (!cond.vartok)
                        continue;
                    if (cond.vartok->exprId() == 0)
                        continue;
                    if (cond.vartok->hasKnownIntValue())
                        continue;
                    if (cond.true_values.empty() || cond.false_values.empty())
                        continue;
                    if (!isConstExpression(cond.vartok, tokenlist->getSettings()->library, true, tokenlist->isCPP()))
                        continue;
                    f(cond, tok, scope);
                }
            }
        }
    }

    void beforeCondition(TokenList* tokenlist,
                         SymbolDatabase* symboldatabase,
                         ErrorLogger* errorLogger,
                         const Settings* settings) const {
        traverseCondition(tokenlist, symboldatabase, [&](const Condition& cond, Token* tok, const Scope*) {
            if (cond.vartok->exprId() == 0)
                return;

            // If condition is known then don't propagate value
            if (tok->hasKnownIntValue())
                return;

            const Token* top = tok->astTop();

            if (Token::Match(top, "%assign%"))
                return;
            if (Token::Match(cond.vartok->astParent(), "%assign%|++|--"))
                return;

            if (Token::simpleMatch(tok->astParent(), "?") && tok->astParent()->isExpandedMacro()) {
                if (settings->debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok,
                            "variable '" + cond.vartok->expressionString() + "', condition is defined in macro");
                return;
            }

            // if,macro => bailout
            if (Token::simpleMatch(top->previous(), "if (") && top->previous()->isExpandedMacro()) {
                if (settings->debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok,
                            "variable '" + cond.vartok->expressionString() + "', condition is defined in macro");
                return;
            }

            std::list<ValueFlow::Value> values = cond.true_values;
            if (cond.true_values != cond.false_values)
                values.insert(values.end(), cond.false_values.begin(), cond.false_values.end());

            // extra logic for unsigned variables 'i>=1' => possible value can also be 0
            if (Token::Match(tok, "<|>")) {
                values.remove_if([](const ValueFlow::Value& v) {
                    if (v.isIntValue())
                        return v.intvalue != 0;
                    return false;
                });
                if (cond.vartok->valueType() && cond.vartok->valueType()->sign != ValueType::Sign::UNSIGNED)
                    return;
            }
            if (values.empty())
                return;

            // bailout: for/while-condition, variable is changed in while loop
            if (Token::Match(top->previous(), "for|while (") && Token::simpleMatch(top->link(), ") {")) {

                // Variable changed in 3rd for-expression
                if (Token::simpleMatch(top->previous(), "for (")) {
                    if (top->astOperand2() && top->astOperand2()->astOperand2() &&
                        isExpressionChanged(
                            cond.vartok, top->astOperand2()->astOperand2(), top->link(), settings, tokenlist->isCPP())) {
                        if (settings->debugwarnings)
                            bailout(tokenlist,
                                    errorLogger,
                                    tok,
                                    "variable '" + cond.vartok->expressionString() + "' used in loop");
                        return;
                    }
                }

                // Variable changed in loop code
                const Token* const start = top;
                const Token* const block = top->link()->next();
                const Token* const end = block->link();

                if (isExpressionChanged(cond.vartok, start, end, settings, tokenlist->isCPP())) {
                    // If its reassigned in loop then analyze from the end
                    if (!Token::Match(tok, "%assign%|++|--") &&
                        findExpression(cond.vartok->exprId(), start, end, [&](const Token* tok2) {
                        return Token::Match(tok2->astParent(), "%assign%") && astIsLHS(tok2);
                    })) {
                        // Start at the end of the loop body
                        Token* bodyTok = top->link()->next();
                        reverse(bodyTok->link(), bodyTok, cond.vartok, values, tokenlist);
                    }
                    if (settings->debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                tok,
                                "variable '" + cond.vartok->expressionString() + "' used in loop");
                    return;
                }
            }

            Token* startTok = nullptr;
            if (astIsRHS(tok))
                startTok = tok->astParent();
            else if (astIsLHS(tok))
                startTok = previousBeforeAstLeftmostLeaf(tok->astParent());
            if (!startTok)
                startTok = tok->previous();

            reverse(startTok, nullptr, cond.vartok, values, tokenlist);
        });
    }

    static Token* skipNotAndCasts(Token* tok, bool* inverted = nullptr)
    {
        for (; tok->astParent(); tok = tok->astParent()) {
            if (Token::simpleMatch(tok->astParent(), "!")) {
                if (inverted)
                    *inverted ^= true;
                continue;
            }
            if (Token::Match(tok->astParent(), "==|!=")) {
                Token* sibling = tok->astSibling();
                if (sibling->hasKnownIntValue() && (astIsBool(tok) || astIsBool(sibling))) {
                    bool value = sibling->values().front().intvalue;
                    if (inverted)
                        *inverted ^= value == Token::simpleMatch(tok->astParent(), "!=");
                    continue;
                }
            }
            if (tok->astParent()->isCast() && astIsBool(tok->astParent()))
                continue;
            return tok;
        }
        return tok;
    }

    static void fillFromPath(ProgramMemory& pm, const Token* top, MathLib::bigint path)
    {
        if (path < 1)
            return;
        visitAstNodes(top, [&](const Token* tok) {
            const ValueFlow::Value* v = ValueFlow::findValue(tok->values(), nullptr, [&](const ValueFlow::Value& v) {
                return v.path == path && isNonConditionalPossibleIntValue(v);
            });
            if (v == nullptr)
                return ChildrenToVisit::op1_and_op2;
            pm.setValue(tok, *v);
            return ChildrenToVisit::op1_and_op2;
        });
    }

    void afterCondition(TokenList* tokenlist,
                        SymbolDatabase* symboldatabase,
                        ErrorLogger* errorLogger,
                        const Settings* settings) const {
        traverseCondition(tokenlist, symboldatabase, [&](const Condition& cond, Token* condTok, const Scope* scope) {
            const Token* top = condTok->astTop();

            const MathLib::bigint path = cond.getPath();
            const bool allowKnown = path == 0;
            const bool allowImpossible = cond.impossible && allowKnown;

            std::list<ValueFlow::Value> thenValues;
            std::list<ValueFlow::Value> elseValues;

            if (!Token::Match(condTok, "!=|=|(|.") && condTok != cond.vartok) {
                thenValues.insert(thenValues.end(), cond.true_values.begin(), cond.true_values.end());
                if (allowImpossible && isConditionKnown(condTok, false))
                    insertImpossible(elseValues, cond.false_values);
            }
            if (!Token::Match(condTok, "==|!")) {
                elseValues.insert(elseValues.end(), cond.false_values.begin(), cond.false_values.end());
                if (allowImpossible && isConditionKnown(condTok, true)) {
                    insertImpossible(thenValues, cond.true_values);
                    if (cond.isBool())
                        insertNegateKnown(thenValues, cond.true_values);
                }
            }

            bool inverted = cond.inverted;
            Token* ctx = skipNotAndCasts(condTok, &inverted);
            if (inverted)
                std::swap(thenValues, elseValues);

            if (Token::Match(ctx->astParent(), "%oror%|&&")) {
                Token* parent = ctx->astParent();
                if (astIsRHS(ctx) && astIsLHS(parent) && parent->astParent() &&
                    parent->str() == parent->astParent()->str())
                    parent = parent->astParent();
                else if (!astIsLHS(ctx)) {
                    parent = nullptr;
                }
                if (parent) {
                    std::vector<Token*> nextExprs = {parent->astOperand2()};
                    if (astIsLHS(parent) && parent->astParent() && parent->astParent()->str() == parent->str()) {
                        nextExprs.push_back(parent->astParent()->astOperand2());
                    }
                    const std::string& op(parent->str());
                    std::list<ValueFlow::Value> values;
                    if (op == "&&")
                        values = thenValues;
                    else if (op == "||")
                        values = elseValues;
                    if (allowKnown && (Token::Match(condTok, "==|!=") || cond.isBool()))
                        changePossibleToKnown(values);
                    if (astIsFloat(cond.vartok, false) ||
                        (!cond.vartok->valueType() &&
                         std::all_of(values.begin(), values.end(), [](const ValueFlow::Value& v) {
                        return v.isIntValue() || v.isFloatValue();
                    })))
                        values.remove_if([&](const ValueFlow::Value& v) {
                            return v.isImpossible();
                        });
                    for (Token* start:nextExprs) {
                        Analyzer::Result r = forward(start, cond.vartok, values, tokenlist);
                        if (r.terminate != Analyzer::Terminate::None)
                            return;
                    }
                }
            }

            {
                const Token* tok2 = condTok;
                std::string op;
                bool mixedOperators = false;
                while (tok2->astParent()) {
                    const Token* parent = tok2->astParent();
                    if (Token::Match(parent, "%oror%|&&")) {
                        if (op.empty()) {
                            op = parent->str();
                        } else if (op != parent->str()) {
                            mixedOperators = true;
                            break;
                        }
                    }
                    if (parent->str() == "!") {
                        op = (op == "&&" ? "||" : "&&");
                    }
                    tok2 = parent;
                }

                if (mixedOperators) {
                    return;
                }
            }

            if (!top)
                return;

            if (top->previous()->isExpandedMacro()) {
                for (std::list<ValueFlow::Value>* values : {&thenValues, &elseValues}) {
                    for (ValueFlow::Value& v : *values)
                        v.macro = true;
                }
            }

            Token* condTop = ctx->astParent();
            {
                bool inverted2 = false;
                while (Token::Match(condTop, "%oror%|&&")) {
                    Token* parent = skipNotAndCasts(condTop, &inverted2)->astParent();
                    if (!parent)
                        break;
                    condTop = parent;
                }
                if (inverted2)
                    std::swap(thenValues, elseValues);
            }

            if (!condTop)
                return;

            if (Token::simpleMatch(condTop, "?")) {
                Token* colon = condTop->astOperand2();
                forward(colon->astOperand1(), cond.vartok, thenValues, tokenlist);
                forward(colon->astOperand2(), cond.vartok, elseValues, tokenlist);
                // TODO: Handle after condition
                return;
            }

            if (condTop != top && condTop->str() != ";")
                return;

            if (!Token::Match(top->previous(), "if|while|for ("))
                return;

            if (top->previous()->str() == "for") {
                if (!Token::Match(condTok, "%comp%"))
                    return;
                if (!Token::simpleMatch(condTok->astParent(), ";"))
                    return;
                const Token* stepTok = getStepTok(top);
                if (cond.vartok->varId() == 0)
                    return;
                if (!cond.vartok->variable())
                    return;
                if (!Token::Match(stepTok, "++|--"))
                    return;
                std::set<ValueFlow::Value::Bound> bounds;
                for (const ValueFlow::Value& v : thenValues) {
                    if (v.bound != ValueFlow::Value::Bound::Point && v.isImpossible())
                        continue;
                    bounds.insert(v.bound);
                }
                if (Token::simpleMatch(stepTok, "++") && bounds.count(ValueFlow::Value::Bound::Lower) > 0)
                    return;
                if (Token::simpleMatch(stepTok, "--") && bounds.count(ValueFlow::Value::Bound::Upper) > 0)
                    return;
                const Token* childTok = condTok->astOperand1();
                if (!childTok)
                    childTok = condTok->astOperand2();
                if (!childTok)
                    return;
                if (childTok->varId() != cond.vartok->varId())
                    return;
                const Token* startBlock = top->link()->next();
                if (isVariableChanged(startBlock,
                                      startBlock->link(),
                                      cond.vartok->varId(),
                                      cond.vartok->variable()->isGlobal(),
                                      settings,
                                      tokenlist->isCPP()))
                    return;
                // Check if condition in for loop is always false
                const Token* initTok = getInitTok(top);
                ProgramMemory pm;
                fillFromPath(pm, initTok, path);
                fillFromPath(pm, condTok, path);
                execute(initTok, &pm, nullptr, nullptr);
                MathLib::bigint result = 1;
                execute(condTok, &pm, &result, nullptr);
                if (result == 0)
                    return;
                // Remove condition since for condition is not redundant
                for (std::list<ValueFlow::Value>* values : {&thenValues, &elseValues}) {
                    for (ValueFlow::Value& v : *values) {
                        v.condition = nullptr;
                        v.conditional = true;
                    }
                }
            }

            bool deadBranch[] = {false, false};
            // start token of conditional code
            Token* startTokens[] = {nullptr, nullptr};
            // determine startToken(s)
            if (Token::simpleMatch(top->link(), ") {"))
                startTokens[0] = top->link()->next();
            if (Token::simpleMatch(top->link()->linkAt(1), "} else {"))
                startTokens[1] = top->link()->linkAt(1)->tokAt(2);

            int changeBlock = -1;
            int bailBlock = -1;

            for (int i = 0; i < 2; i++) {
                const Token* const startToken = startTokens[i];
                if (!startToken)
                    continue;
                std::list<ValueFlow::Value>& values = (i == 0 ? thenValues : elseValues);
                if (allowKnown)
                    valueFlowSetConditionToKnown(condTok, values, i == 0);

                Analyzer::Result r = forward(startTokens[i], startTokens[i]->link(), cond.vartok, values, tokenlist);
                deadBranch[i] = r.terminate == Analyzer::Terminate::Escape;
                if (r.action.isModified() && !deadBranch[i])
                    changeBlock = i;
                if (r.terminate != Analyzer::Terminate::None && r.terminate != Analyzer::Terminate::Escape &&
                    r.terminate != Analyzer::Terminate::Modified)
                    bailBlock = i;
                changeKnownToPossible(values);
            }
            if (changeBlock >= 0 && !Token::simpleMatch(top->previous(), "while (")) {
                if (settings->debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            startTokens[changeBlock]->link(),
                            "valueFlowAfterCondition: " + cond.vartok->expressionString() +
                            " is changed in conditional block");
                return;
            } else if (bailBlock >= 0) {
                if (settings->debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            startTokens[bailBlock]->link(),
                            "valueFlowAfterCondition: bailing in conditional block");
                return;
            }

            // After conditional code..
            if (Token::simpleMatch(top->link(), ") {")) {
                Token* after = top->link()->linkAt(1);
                bool dead_if = deadBranch[0];
                bool dead_else = deadBranch[1];
                const Token* unknownFunction = nullptr;
                if (condTok->astParent() && Token::Match(top->previous(), "while|for ("))
                    dead_if = !isBreakScope(after);
                else if (!dead_if)
                    dead_if = isReturnScope(after, &settings->library, &unknownFunction);

                if (!dead_if && unknownFunction) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, unknownFunction, "possible noreturn scope");
                    return;
                }

                if (Token::simpleMatch(after, "} else {")) {
                    after = after->linkAt(2);
                    unknownFunction = nullptr;
                    if (!dead_else)
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

                if (values.empty())
                    return;

                if (dead_if || dead_else) {
                    const Token* parent = condTok->astParent();
                    // Skip the not operator
                    while (Token::simpleMatch(parent, "!"))
                        parent = parent->astParent();
                    bool possible = false;
                    if (Token::Match(parent, "&&|%oror%")) {
                        std::string op = parent->str();
                        while (parent && parent->str() == op)
                            parent = parent->astParent();
                        if (Token::simpleMatch(parent, "!") || Token::simpleMatch(parent, "== false"))
                            possible = op == "||";
                        else
                            possible = op == "&&";
                    }
                    if (possible) {
                        values.remove_if(std::mem_fn(&ValueFlow::Value::isImpossible));
                        changeKnownToPossible(values);
                    } else if (allowKnown) {
                        valueFlowSetConditionToKnown(condTok, values, true);
                        valueFlowSetConditionToKnown(condTok, values, false);
                    }
                }
                if (values.empty())
                    return;
                bool isKnown = std::any_of(values.begin(), values.end(), [&](const ValueFlow::Value& v) {
                    return v.isKnown() || v.isImpossible();
                });
                if (isKnown && isBreakOrContinueScope(after)) {
                    const Scope* loopScope = getLoopScope(cond.vartok);
                    if (loopScope) {
                        Analyzer::Result r = forward(after, loopScope->bodyEnd, cond.vartok, values, tokenlist);
                        if (r.terminate != Analyzer::Terminate::None)
                            return;
                        if (r.action.isModified())
                            return;
                        Token* start = const_cast<Token*>(loopScope->bodyEnd);
                        if (Token::simpleMatch(start, "} while (")) {
                            start = start->tokAt(2);
                            forward(start, start->link(), cond.vartok, values, tokenlist);
                            start = start->link();
                        }
                        values.remove_if(std::mem_fn(&ValueFlow::Value::isImpossible));
                        changeKnownToPossible(values);
                    }
                }
                forward(after, getEndOfExprScope(cond.vartok, scope), cond.vartok, values, tokenlist);
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
    handler->beforeCondition(tokenlist, symboldatabase, errorLogger, settings);
    handler->afterCondition(tokenlist, symboldatabase, errorLogger, settings);
}

struct SimpleConditionHandler : ConditionHandler {
    std::vector<Condition> parse(const Token* tok, const Settings*) const override {

        std::vector<Condition> conds;
        parseCompareEachInt(tok, [&](const Token* vartok, ValueFlow::Value true_value, ValueFlow::Value false_value) {
            Condition cond;
            if (vartok->hasKnownIntValue())
                return;
            if (vartok->str() == "=" && vartok->astOperand1() && vartok->astOperand2())
                vartok = vartok->astOperand1();
            cond.true_values.push_back(true_value);
            cond.false_values.push_back(false_value);
            cond.vartok = vartok;
            conds.push_back(cond);
        });
        if (!conds.empty())
            return conds;

        Condition cond;
        ValueFlow::Value true_value;
        ValueFlow::Value false_value;
        const Token* vartok = nullptr;

        if (tok->str() == "!") {
            vartok = tok->astOperand1();

        } else if (tok->astParent() && (Token::Match(tok->astParent(), "%oror%|&&|?") ||
                                        Token::Match(tok->astParent()->previous(), "if|while ("))) {
            if (Token::simpleMatch(tok, "="))
                vartok = tok->astOperand1();
            else if (!Token::Match(tok, "%comp%|%assign%"))
                vartok = tok;
        }

        if (!vartok)
            return {};
        cond.true_values.emplace_back(tok, 0LL);
        cond.false_values.emplace_back(tok, 0LL);
        cond.vartok = vartok;

        return {cond};
    }
};

struct IntegralInferModel : InferModel {
    bool match(const ValueFlow::Value& value) const override {
        return value.isIntValue();
    }
    ValueFlow::Value yield(MathLib::bigint value) const override
    {
        ValueFlow::Value result(value);
        result.valueType = ValueFlow::Value::ValueType::INT;
        result.setKnown();
        return result;
    }
};

ValuePtr<InferModel> makeIntegralInferModel() {
    return IntegralInferModel{};
}

ValueFlow::Value inferCondition(const std::string& op, const Token* varTok, MathLib::bigint val)
{
    if (!varTok)
        return ValueFlow::Value{};
    if (varTok->hasKnownIntValue())
        return ValueFlow::Value{};
    std::vector<ValueFlow::Value> r = infer(IntegralInferModel{}, op, varTok->values(), val);
    if (r.size() == 1 && r.front().isKnown())
        return r.front();
    return ValueFlow::Value{};
}

ValueFlow::Value inferCondition(std::string op, MathLib::bigint val, const Token* varTok)
{
    if (!varTok)
        return ValueFlow::Value{};
    if (varTok->hasKnownIntValue())
        return ValueFlow::Value{};
    std::vector<ValueFlow::Value> r = infer(IntegralInferModel{}, op, val, varTok->values());
    if (r.size() == 1 && r.front().isKnown())
        return r.front();
    return ValueFlow::Value{};
}

struct IteratorInferModel : InferModel {
    virtual ValueFlow::Value::ValueType getType() const = 0;
    bool match(const ValueFlow::Value& value) const override {
        return value.valueType == getType();
    }
    ValueFlow::Value yield(MathLib::bigint value) const override
    {
        ValueFlow::Value result(value);
        result.valueType = getType();
        result.setKnown();
        return result;
    }
};

struct EndIteratorInferModel : IteratorInferModel {
    ValueFlow::Value::ValueType getType() const override {
        return ValueFlow::Value::ValueType::ITERATOR_END;
    }
};

struct StartIteratorInferModel : IteratorInferModel {
    ValueFlow::Value::ValueType getType() const override {
        return ValueFlow::Value::ValueType::ITERATOR_END;
    }
};

static void valueFlowInferCondition(TokenList* tokenlist,
                                    const Settings* settings)
{
    for (Token* tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->astParent())
            continue;
        if (tok->hasKnownIntValue())
            continue;
        if (tok->variable() && (Token::Match(tok->astParent(), "?|&&|!|%oror%") ||
                                Token::Match(tok->astParent()->previous(), "if|while ("))) {
            std::vector<ValueFlow::Value> result = infer(IntegralInferModel{}, "!=", tok->values(), 0);
            if (result.size() != 1)
                continue;
            ValueFlow::Value value = result.front();
            value.intvalue = 1;
            value.bound = ValueFlow::Value::Bound::Point;
            setTokenValue(tok, value, settings);
        } else if (Token::Match(tok, "%comp%|-") && tok->astOperand1() && tok->astOperand2()) {
            if (astIsIterator(tok->astOperand1()) || astIsIterator(tok->astOperand2())) {
                static const std::array<ValuePtr<InferModel>, 2> iteratorModels = {EndIteratorInferModel{},
                                                                                   StartIteratorInferModel{}};
                for (const ValuePtr<InferModel>& model : iteratorModels) {
                    std::vector<ValueFlow::Value> result =
                        infer(model, tok->str(), tok->astOperand1()->values(), tok->astOperand2()->values());
                    for (ValueFlow::Value value : result) {
                        value.valueType = ValueFlow::Value::ValueType::INT;
                        setTokenValue(tok, value, settings);
                    }
                }
            } else {
                std::vector<ValueFlow::Value> result =
                    infer(IntegralInferModel{}, tok->str(), tok->astOperand1()->values(), tok->astOperand2()->values());
                for (const ValueFlow::Value& value : result) {
                    setTokenValue(tok, value, settings);
                }
            }
        }
    }
}

struct SymbolicConditionHandler : SimpleConditionHandler {

    static bool isNegatedBool(const Token* tok)
    {
        if (!Token::simpleMatch(tok, "!"))
            return false;
        return (astIsBool(tok->astOperand1()));
    }

    static const Token* skipNot(const Token* tok)
    {
        if (!Token::simpleMatch(tok, "!"))
            return tok;
        return tok->astOperand1();
    }

    std::vector<Condition> parse(const Token* tok, const Settings* settings) const override
    {
        if (!Token::Match(tok, "%comp%"))
            return {};
        if (tok->hasKnownIntValue())
            return {};
        if (!tok->astOperand1() || tok->astOperand1()->hasKnownIntValue() || tok->astOperand1()->isLiteral())
            return {};
        if (!tok->astOperand2() || tok->astOperand2()->hasKnownIntValue() || tok->astOperand2()->isLiteral())
            return {};
        if (!isConstExpression(tok, settings->library, true, true))
            return {};

        std::vector<Condition> result;
        auto addCond = [&](const Token* lhsTok, const Token* rhsTok, bool inverted) {
            for (int i = 0; i < 2; i++) {
                const bool lhs = i == 0;
                const Token* vartok = lhs ? lhsTok : rhsTok;
                const Token* valuetok = lhs ? rhsTok : lhsTok;
                if (valuetok->exprId() == 0)
                    continue;
                if (valuetok->hasKnownSymbolicValue(vartok))
                    continue;
                if (vartok->hasKnownSymbolicValue(valuetok))
                    continue;
                ValueFlow::Value true_value;
                ValueFlow::Value false_value;
                setConditionalValues(tok, !lhs, 0, true_value, false_value);
                setSymbolic(true_value, valuetok);
                setSymbolic(false_value, valuetok);

                Condition cond;
                cond.true_values = {true_value};
                cond.false_values = {false_value};
                cond.vartok = vartok;
                cond.inverted = inverted;
                result.push_back(cond);
            }
        };
        addCond(tok->astOperand1(), tok->astOperand2(), false);
        if (Token::Match(tok, "==|!=") && (isNegatedBool(tok->astOperand1()) || isNegatedBool(tok->astOperand2()))) {
            const Token* lhsTok = skipNot(tok->astOperand1());
            const Token* rhsTok = skipNot(tok->astOperand2());
            addCond(lhsTok, rhsTok, !(isNegatedBool(tok->astOperand1()) && isNegatedBool(tok->astOperand2())));
        }
        return result;
    }
};

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

    if (memory1)
        memory1->swap(startMemory);
    if (!error) {
        if (memory2)
            memory2->swap(endMemory);
        if (memoryAfter)
            memoryAfter->swap(programMemory);
    }

    return true;
}

static void valueFlowForLoopSimplify(Token* const bodyStart,
                                     const Token* expr,
                                     bool globalvar,
                                     const MathLib::bigint value,
                                     TokenList* tokenlist,
                                     ErrorLogger* errorLogger,
                                     const Settings* settings)
{
    // TODO: Refactor this to use arbitary expressions
    assert(expr->varId() > 0);
    const Token * const bodyEnd = bodyStart->link();

    // Is variable modified inside for loop
    if (isVariableChanged(bodyStart, bodyEnd, expr->varId(), globalvar, settings, tokenlist->isCPP()))
        return;

    for (Token *tok2 = bodyStart->next(); tok2 != bodyEnd; tok2 = tok2->next()) {
        if (tok2->varId() == expr->varId()) {
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
            const ProgramMemory programMemory(getProgramMemory(tok2->astTop(), expr, ValueFlow::Value(value), settings));
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
        const Token* vartok = expr;
        const Token* rml = nextAfterAstRightmostLeaf(vartok);
        if (rml)
            vartok = rml->str() == "]" ? rml : rml->previous();
        if (vartok->str() == "]" && vartok->link()->previous())
            vartok = vartok->link()->previous();

        if ((tok2->str() == "&&" &&
             conditionIsFalse(tok2->astOperand1(),
                              getProgramMemory(tok2->astTop(), expr, ValueFlow::Value(value), settings))) ||
            (tok2->str() == "||" &&
             conditionIsTrue(tok2->astOperand1(),
                             getProgramMemory(tok2->astTop(), expr, ValueFlow::Value(value), settings))))
            break;

        else if (Token::simpleMatch(tok2, ") {")) {
            if (vartok->varId() && Token::findmatch(tok2->link(), "%varid%", tok2, vartok->varId())) {
                if (Token::findmatch(tok2, "continue|break|return", tok2->linkAt(1), vartok->varId())) {
                    if (settings->debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "For loop variable bailout on conditional continue|break|return");
                    break;
                }
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop variable skipping conditional scope");
                tok2 = tok2->next()->link();
                if (Token::simpleMatch(tok2, "} else {")) {
                    if (Token::findmatch(tok2, "continue|break|return", tok2->linkAt(2), vartok->varId())) {
                        if (settings->debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "For loop variable bailout on conditional continue|break|return");
                        break;
                    }
                    tok2 = tok2->linkAt(2);
                }
            }
            else {
                if (settings->debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop skipping {} code");
                tok2 = tok2->linkAt(1);
                if (Token::simpleMatch(tok2, "} else {"))
                    tok2 = tok2->linkAt(2);
            }
        }
    }
}

static void valueFlowForLoopSimplifyAfter(Token* fortok, nonneg int varid, const MathLib::bigint num, TokenList* tokenlist)
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
        valueFlowForward(blockTok->next(), endToken, vartok, values, tokenlist);
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
            const Token* vartok = Token::findmatch(tok, "%varid%", bodyStart, varid);
            if (executeBody && vartok) {
                std::list<ValueFlow::Value> initValues;
                initValues.emplace_back(initValue, ValueFlow::Value::Bound::Lower);
                initValues.push_back(asImpossible(initValues.back()));
                Analyzer::Result result = valueFlowForward(bodyStart, bodyStart->link(), vartok, initValues, tokenlist);

                if (!result.action.isModified()) {
                    std::list<ValueFlow::Value> lastValues;
                    lastValues.emplace_back(lastValue, ValueFlow::Value::Bound::Upper);
                    lastValues.back().conditional = true;
                    lastValues.push_back(asImpossible(lastValues.back()));
                    if (stepValue != 1)
                        lastValues.pop_front();
                    valueFlowForward(bodyStart, bodyStart->link(), vartok, lastValues, tokenlist);
                }
            }
            const MathLib::bigint afterValue = executeBody ? lastValue + stepValue : initValue;
            valueFlowForLoopSimplifyAfter(tok, varid, afterValue, tokenlist);
        } else {
            ProgramMemory mem1, mem2, memAfter;
            if (valueFlowForLoop2(tok, &mem1, &mem2, &memAfter)) {
                for (const auto& p : mem1) {
                    if (!p.second.isIntValue())
                        continue;
                    if (p.first.tok->varId() == 0)
                        continue;
                    valueFlowForLoopSimplify(bodyStart, p.first.tok, false, p.second.intvalue, tokenlist, errorLogger, settings);
                }
                for (const auto& p : mem2) {
                    if (!p.second.isIntValue())
                        continue;
                    if (p.first.tok->varId() == 0)
                        continue;
                    valueFlowForLoopSimplify(bodyStart, p.first.tok, false, p.second.intvalue, tokenlist, errorLogger, settings);
                }
                for (const auto& p : memAfter) {
                    if (!p.second.isIntValue())
                        continue;
                    if (p.first.tok->varId() == 0)
                        continue;
                    valueFlowForLoopSimplifyAfter(tok, p.first.getExpressionId(), p.second.intvalue, tokenlist);
                }
            }
        }
    }
}

struct MultiValueFlowAnalyzer : ValueFlowAnalyzer {
    std::unordered_map<nonneg int, ValueFlow::Value> values;
    std::unordered_map<nonneg int, const Variable*> vars;
    SymbolDatabase* symboldatabase;

    MultiValueFlowAnalyzer() : ValueFlowAnalyzer(), values(), vars(), symboldatabase(nullptr) {}

    MultiValueFlowAnalyzer(const std::unordered_map<const Variable*, ValueFlow::Value>& args, const TokenList* t, SymbolDatabase* s)
        : ValueFlowAnalyzer(t), values(), vars(), symboldatabase(s) {
        for (const auto& p:args) {
            values[p.first->declarationId()] = p.second;
            vars[p.first->declarationId()] = p.first;
        }
    }

    virtual const std::unordered_map<nonneg int, const Variable*>& getVars() const {
        return vars;
    }

    const ValueFlow::Value* getValue(const Token* tok) const override {
        if (tok->varId() == 0)
            return nullptr;
        auto it = values.find(tok->varId());
        if (it == values.end())
            return nullptr;
        return &it->second;
    }
    ValueFlow::Value* getValue(const Token* tok) override {
        if (tok->varId() == 0)
            return nullptr;
        auto it = values.find(tok->varId());
        if (it == values.end())
            return nullptr;
        return &it->second;
    }

    void makeConditional() override {
        for (auto&& p:values) {
            p.second.conditional = true;
        }
    }

    void addErrorPath(const Token* tok, const std::string& s) override {
        for (auto&& p:values) {
            p.second.errorPath.emplace_back(tok, "Assuming condition is " + s);
        }
    }

    bool isAlias(const Token* tok, bool& inconclusive) const override {
        const auto range = SelectValueFromVarIdMapRange(&values);

        for (const auto& p:getVars()) {
            nonneg int varid = p.first;
            const Variable* var = p.second;
            if (tok->varId() == varid)
                return true;
            if (isAliasOf(var, tok, varid, range, &inconclusive))
                return true;
        }
        return false;
    }

    bool isGlobal() const override {
        return false;
    }

    bool lowerToPossible() override {
        for (auto&& p:values) {
            if (p.second.isImpossible())
                return false;
            p.second.changeKnownToPossible();
        }
        return true;
    }
    bool lowerToInconclusive() override {
        for (auto&& p:values) {
            if (p.second.isImpossible())
                return false;
            p.second.setInconclusive();
        }
        return true;
    }

    bool isConditional() const override {
        for (auto&& p:values) {
            if (p.second.conditional)
                return true;
            if (p.second.condition)
                return !p.second.isImpossible();
        }
        return false;
    }

    bool stopOnCondition(const Token*) const override {
        return isConditional();
    }

    bool updateScope(const Token* endBlock, bool) const override {
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

    bool match(const Token* tok) const override {
        return values.count(tok->varId()) > 0;
    }

    ProgramState getProgramState() const override {
        ProgramState ps;
        for (const auto& p : values) {
            const Variable* var = vars.at(p.first);
            if (!var)
                continue;
            ps[var->nameToken()] = p.second;
        }
        return ps;
    }
};

template<class Key, class F>
bool productParams(const std::unordered_map<Key, std::list<ValueFlow::Value>>& vars, F f)
{
    using Args = std::vector<std::unordered_map<Key, ValueFlow::Value>>;
    Args args(1);
    // Compute cartesian product of all arguments
    for (const auto& p:vars) {
        if (p.second.empty())
            continue;
        args.back()[p.first] = p.second.front();
    }
    for (const auto& p:vars) {
        if (args.size() > 256)
            return false;
        if (p.second.empty())
            continue;
        std::for_each(std::next(p.second.begin()), p.second.end(), [&](const ValueFlow::Value& value) {
            Args new_args;
            for (auto arg:args) {
                if (value.path != 0) {
                    for (const auto& q:arg) {
                        if (q.first == p.first)
                            continue;
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
        f(arg);
    }
    return true;
}

static void valueFlowInjectParameter(TokenList* tokenlist,
                                     SymbolDatabase* symboldatabase,
                                     ErrorLogger* errorLogger,
                                     const Settings* settings,
                                     const Scope* functionScope,
                                     const std::unordered_map<const Variable*, std::list<ValueFlow::Value>>& vars)
{
    bool r = productParams(vars, [&](const std::unordered_map<const Variable*, ValueFlow::Value>& arg) {
        MultiValueFlowAnalyzer a(arg, tokenlist, symboldatabase);
        valueFlowGenericForward(const_cast<Token*>(functionScope->bodyStart), functionScope->bodyEnd, a, settings);
    });
    if (!r) {
        std::string fname = "<unknown>";
        Function* f = functionScope->function;
        if (f)
            fname = f->name();
        if (settings->debugwarnings)
            bailout(tokenlist, errorLogger, functionScope->bodyStart, "Too many argument passed to " + fname);
    }
}

static void valueFlowInjectParameter(TokenList* tokenlist,
                                     const Variable* arg,
                                     const Scope* functionScope,
                                     const std::list<ValueFlow::Value>& argvalues)
{
    // Is argument passed by value or const reference, and is it a known non-class type?
    if (arg->isReference() && !arg->isConst() && !arg->isClass())
        return;

    // Set value in function scope..
    const nonneg int varid2 = arg->declarationId();
    if (!varid2)
        return;

    valueFlowForward(const_cast<Token*>(functionScope->bodyStart->next()),
                     functionScope->bodyEnd,
                     arg->nameToken(),
                     argvalues,
                     tokenlist);
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
    std::unordered_map<nonneg int, std::list<ValueFlow::Value>> argValues;
    int argn = 1;
    for (const Token *argtok : getArguments(tok->previous())) {
        argValues[argn] = getFunctionArgumentValues(argtok);
        argn++;
    }
    if (returnValue.find("arg") != std::string::npos && argValues.empty())
        return;
    productParams(argValues, [&](const std::unordered_map<nonneg int, ValueFlow::Value>& arg) {
        ValueFlow::Value value = evaluateLibraryFunction(arg, returnValue, settings);
        if (value.isUninitValue())
            return;
        ValueFlow::Value::ValueKind kind = ValueFlow::Value::ValueKind::Known;
        for (auto&& p : arg) {
            if (p.second.isPossible())
                kind = p.second.valueKind;
            if (p.second.isInconclusive()) {
                kind = p.second.valueKind;
                break;
            }
        }
        if (value.isImpossible() && kind != ValueFlow::Value::ValueKind::Known)
            return;
        if (!value.isImpossible())
            value.valueKind = kind;
        setTokenValue(tok, value, settings);
    });
}

template<class Iterator>
struct IteratorRange
{
    Iterator mBegin;
    Iterator mEnd;

    Iterator begin() const {
        return mBegin;
    }

    Iterator end() const {
        return mEnd;
    }
};

template<class Iterator>
IteratorRange<Iterator> MakeIteratorRange(Iterator start, Iterator last)
{
    return {start, last};
}

static void valueFlowSubFunction(TokenList* tokenlist, SymbolDatabase* symboldatabase,  ErrorLogger* errorLogger, const Settings* settings)
{
    int id = 0;
    for (const Scope* scope : MakeIteratorRange(symboldatabase->functionScopes.rbegin(), symboldatabase->functionScopes.rend())) {
        const Function* function = scope->function;
        if (!function)
            continue;
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
                // Remove uninit values if argument is passed by value
                if (argtok->variable() && !argtok->variable()->isPointer() && argvalues.size() == 1 && argvalues.front().isUninitValue()) {
                    if (CheckUninitVar::isVariableUsage(tokenlist->isCPP(), argtok, settings->library, false, CheckUninitVar::Alloc::NO_ALLOC, 0))
                        continue;
                }

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
                    v.path = 256 * v.path + id % 256;
                    // Change scope of lifetime values
                    if (v.isLifetimeValue())
                        v.lifetimeScope = ValueFlow::Value::LifetimeScope::SubFunction;
                }

                // passed values are not "known"..
                lowerToPossible(argvalues);

                argvars[argvar] = argvalues;
            }
            valueFlowInjectParameter(tokenlist, symboldatabase, errorLogger, settings, calledFunctionScope, argvars);
        }
    }
}

static void valueFlowFunctionDefaultParameter(TokenList* tokenlist, SymbolDatabase* symboldatabase)
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
                    valueFlowInjectParameter(tokenlist, var, scope, argvalues);
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
        if (tok->str() != "(" || !tok->astOperand1())
            continue;

        const Function* function = nullptr;
        if (Token::Match(tok->previous(), "%name% ("))
            function = tok->previous()->function();
        else
            function = tok->astOperand1()->function();
        if (!function)
            continue;
        // TODO: Check if member variable is a pointer or reference
        if (function->isImplicitlyVirtual() && !function->hasFinalSpecifier())
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
            programMemory.setIntValue(arg->nameToken(), parvalues[i]);
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

static bool needsInitialization(const Variable* var, bool cpp)
{
    if (!var)
        return false;
    if (var->isPointer())
        return true;
    if (var->type() && var->type()->isUnionType())
        return false;
    if (!cpp)
        return true;
    if (var->type() && var->type()->needInitialization == Type::NeedInitialization::True)
        return true;
    if (var->valueType() && (var->valueType()->isPrimitive() || var->valueType()->type == ValueType::Type::POD))
        return true;
    return false;
}

static void addToErrorPath(ValueFlow::Value& value, const ValueFlow::Value& from)
{
    std::unordered_set<const Token*> locations;
    std::transform(value.errorPath.begin(),
                   value.errorPath.end(),
                   std::inserter(locations, locations.begin()),
                   [](const ErrorPathItem& e) {
        return e.first;
    });
    if (from.condition && !value.condition)
        value.condition = from.condition;
    std::copy_if(from.errorPath.begin(),
                 from.errorPath.end(),
                 std::back_inserter(value.errorPath),
                 [&](const ErrorPathItem& e) {
        return locations.insert(e.first).second;
    });
}

static void valueFlowUninit(TokenList* tokenlist, SymbolDatabase* /*symbolDatabase*/, const Settings* settings)
{
    for (Token *tok = tokenlist->front(); tok; tok = tok->next()) {
        if (!tok->scope()->isExecutable())
            continue;
        if (!Token::Match(tok, "%var% ;|["))
            continue;
        const Variable* var = tok->variable();
        if (!var)
            continue;
        if (var->nameToken() != tok || var->isInit())
            continue;
        if (!needsInitialization(var, tokenlist->isCPP()))
            continue;
        if (!var->isLocal() || var->isStatic() || var->isExtern() || var->isReference() || var->isThrow())
            continue;

        ValueFlow::Value uninitValue;
        uninitValue.setKnown();
        uninitValue.valueType = ValueFlow::Value::ValueType::UNINIT;
        uninitValue.tokvalue = tok;
        if (var->isArray())
            uninitValue.indirect = var->dimensions().size();

        bool partial = false;

        std::map<Token*, ValueFlow::Value> partialReads;
        if (const Scope* scope = var->typeScope()) {
            if (Token::findsimplematch(scope->bodyStart, "union", scope->bodyEnd))
                continue;
            for (const Variable& memVar : scope->varlist) {
                if (!memVar.isPublic())
                    continue;
                // Skip array since we can't track partial initialization from nested subexpressions
                if (memVar.isArray())
                    continue;
                if (!needsInitialization(&memVar, tokenlist->isCPP())) {
                    partial = true;
                    continue;
                }
                MemberExpressionAnalyzer analyzer(memVar.nameToken()->str(), tok, uninitValue, tokenlist);
                valueFlowGenericForward(tok->next(), tok->scope()->bodyEnd, analyzer, settings);

                for (auto&& p : *analyzer.partialReads) {
                    Token* tok2 = p.first;
                    const ValueFlow::Value& v = p.second;
                    // Try to insert into map
                    auto pp = partialReads.insert(std::make_pair(tok2, v));
                    ValueFlow::Value& v2 = pp.first->second;
                    bool inserted = pp.second;
                    // Merge the two values if it is already in map
                    if (!inserted) {
                        if (v.valueType != v2.valueType)
                            continue;
                        addToErrorPath(v2, v);
                    }
                    v2.subexpressions.push_back(memVar.nameToken()->str());
                }
            }
        }

        for (auto&& p : partialReads) {
            Token* tok2 = p.first;
            const ValueFlow::Value& v = p.second;

            setTokenValue(tok2, v, settings);
        }

        if (partial)
            continue;

        valueFlowForward(tok->next(), tok->scope()->bodyEnd, var->nameToken(), uninitValue, tokenlist);
    }
}

static bool isContainerSizeChanged(nonneg int varId,
                                   const Token* start,
                                   const Token* end,
                                   const Settings* settings = nullptr,
                                   int depth = 20);

static bool isContainerSizeChangedByFunction(const Token* tok, const Settings* settings = nullptr, int depth = 20)
{
    if (!tok->valueType())
        return false;
    if (!astIsContainer(tok))
        return false;
    // If we are accessing an element then we are not changing the container size
    if (Token::Match(tok, "%name% . %name% (")) {
        Library::Container::Yield yield = getLibraryContainer(tok)->getYield(tok->strAt(2));
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
                    return isContainerSizeChanged(
                        arg->declarationId(), scope->bodyStart, scope->bodyEnd, settings, depth - 1);
            }
            // Don't know => Safe guess
            return true;
        }
    }

    bool inconclusive = false;
    const bool isChanged = isVariableChangedByFunctionCall(tok, 0, settings, &inconclusive);
    return (isChanged || inconclusive);
}

struct ContainerExpressionAnalyzer : ExpressionAnalyzer {
    ContainerExpressionAnalyzer() : ExpressionAnalyzer() {}

    ContainerExpressionAnalyzer(const Token* expr, const ValueFlow::Value& val, const TokenList* t)
        : ExpressionAnalyzer(expr, val, t)
    {}

    bool match(const Token* tok) const override {
        return tok->exprId() == expr->exprId() || (astIsIterator(tok) && isAliasOf(tok, expr->exprId()));
    }

    Action isWritable(const Token* tok, Direction d) const override {
        if (astIsIterator(tok))
            return Action::None;
        if (d == Direction::Reverse)
            return Action::None;
        if (!getValue(tok))
            return Action::None;
        if (!tok->valueType())
            return Action::None;
        if (!astIsContainer(tok))
            return Action::None;
        const Token* parent = tok->astParent();
        const Library::Container* container = getLibraryContainer(tok);

        if (container->stdStringLike && Token::simpleMatch(parent, "+=") && astIsLHS(tok) && parent->astOperand2()) {
            const Token* rhs = parent->astOperand2();
            if (rhs->tokType() == Token::eString)
                return Action::Read | Action::Write | Action::Incremental;
            const Library::Container* rhsContainer = getLibraryContainer(rhs);
            if (rhsContainer && rhsContainer->stdStringLike) {
                if (std::any_of(rhs->values().begin(), rhs->values().end(), [&](const ValueFlow::Value &rhsval) {
                    return rhsval.isKnown() && rhsval.isContainerSizeValue();
                }))
                    return Action::Read | Action::Write | Action::Incremental;
            }
        } else if (astIsLHS(tok) && Token::Match(tok->astParent(), ". %name% (")) {
            Library::Container::Action action = container->getAction(tok->astParent()->strAt(1));
            if (action == Library::Container::Action::PUSH || action == Library::Container::Action::POP) {
                std::vector<const Token*> args = getArguments(tok->tokAt(3));
                if (args.size() < 2)
                    return Action::Read | Action::Write | Action::Incremental;
            }
        }
        return Action::None;
    }

    void writeValue(ValueFlow::Value* val, const Token* tok, Direction d) const override {
        if (d == Direction::Reverse)
            return;
        if (!val)
            return;
        if (!tok->astParent())
            return;
        if (!tok->valueType())
            return;
        if (!astIsContainer(tok))
            return;
        const Token* parent = tok->astParent();
        const Library::Container* container = getLibraryContainer(tok);

        if (container->stdStringLike && Token::simpleMatch(parent, "+=") && parent->astOperand2()) {
            const Token* rhs = parent->astOperand2();
            const Library::Container* rhsContainer = getLibraryContainer(rhs);
            if (rhs->tokType() == Token::eString)
                val->intvalue += Token::getStrLength(rhs);
            else if (rhsContainer && rhsContainer->stdStringLike) {
                for (const ValueFlow::Value &rhsval : rhs->values()) {
                    if (rhsval.isKnown() && rhsval.isContainerSizeValue()) {
                        val->intvalue += rhsval.intvalue;
                    }
                }
            }
        } else if (astIsLHS(tok) && Token::Match(tok->astParent(), ". %name% (")) {
            Library::Container::Action action = container->getAction(tok->astParent()->strAt(1));
            if (action == Library::Container::Action::PUSH)
                val->intvalue++;
            if (action == Library::Container::Action::POP)
                val->intvalue--;
        }
    }

    Action isModified(const Token* tok) const override {
        Action read = Action::Read;
        // An iterator won't change the container size
        if (astIsIterator(tok))
            return read;
        if (Token::Match(tok->astParent(), "%assign%") && astIsLHS(tok))
            return Action::Invalid;
        if (isLikelyStreamRead(isCPP(), tok->astParent()))
            return Action::Invalid;
        if (astIsContainer(tok) && isContainerSizeChanged(tok, getSettings()))
            return Action::Invalid;
        return read;
    }
};

static const Token* parseBinaryIntOp(const Token* expr,
                                     const std::function<std::vector<MathLib::bigint>(const Token*)>& eval,
                                     MathLib::bigint& known)
{
    if (!expr)
        return nullptr;
    if (!expr->astOperand1() || !expr->astOperand2())
        return nullptr;
    if (expr->astOperand1()->exprId() == 0 && expr->astOperand2()->exprId() == 0)
        return nullptr;
    std::vector<MathLib::bigint> x1 = eval(expr->astOperand1());
    std::vector<MathLib::bigint> x2 = eval(expr->astOperand2());
    if (expr->astOperand1()->exprId() == 0 && x1.empty())
        return nullptr;
    if (expr->astOperand2()->exprId() == 0 && x2.empty())
        return nullptr;
    const Token* varTok = nullptr;
    if (!x1.empty() && x2.empty()) {
        varTok = expr->astOperand2();
        known = x1.front();
    } else if (x1.empty() && !x2.empty()) {
        varTok = expr->astOperand1();
        known = x2.front();
    }
    return varTok;
}

const Token* solveExprValue(const Token* expr,
                            const std::function<std::vector<MathLib::bigint>(const Token*)>& eval,
                            ValueFlow::Value& value)
{
    if (!value.isIntValue() && !value.isIteratorValue() && !value.isSymbolicValue())
        return expr;
    if (value.isSymbolicValue() && !Token::Match(expr, "+|-"))
        return expr;
    MathLib::bigint intval;
    const Token* binaryTok = parseBinaryIntOp(expr, eval, intval);
    bool rhs = astIsRHS(binaryTok);
    // If its on the rhs, then -1 multiplication is needed, which is not possible with simple delta analysis used currently for symbolic values
    if (value.isSymbolicValue() && rhs && Token::simpleMatch(expr, "-"))
        return expr;
    if (binaryTok && expr->str().size() == 1) {
        switch (expr->str()[0]) {
        case '+': {
            value.intvalue -= intval;
            return solveExprValue(binaryTok, eval, value);
        }
        case '-': {
            if (rhs)
                value.intvalue = intval - value.intvalue;
            else
                value.intvalue += intval;
            return solveExprValue(binaryTok, eval, value);
        }
        case '*': {
            if (intval == 0)
                break;
            value.intvalue /= intval;
            return solveExprValue(binaryTok, eval, value);
        }
        case '^': {
            value.intvalue ^= intval;
            return solveExprValue(binaryTok, eval, value);
        }
        }
    }
    return expr;
}

static const Token* solveExprValue(const Token* expr, ValueFlow::Value& value)
{
    return solveExprValue(
        expr,
        [](const Token* tok) -> std::vector<MathLib::bigint> {
        if (tok->hasKnownIntValue())
            return {tok->values().front().intvalue};
        return {};
    },
        value);
}

ValuePtr<Analyzer> makeAnalyzer(const Token* exprTok, ValueFlow::Value value, const TokenList* tokenlist)
{
    if (value.isContainerSizeValue())
        return ContainerExpressionAnalyzer(exprTok, value, tokenlist);
    const Token* expr = solveExprValue(exprTok, value);
    return ExpressionAnalyzer(expr, value, tokenlist);
}

ValuePtr<Analyzer> makeReverseAnalyzer(const Token* exprTok, const ValueFlow::Value& value, const TokenList* tokenlist)
{
    if (value.isContainerSizeValue())
        return ContainerExpressionAnalyzer(exprTok, value, tokenlist);
    return ExpressionAnalyzer(exprTok, value, tokenlist);
}

bool isContainerSizeChanged(const Token* tok, const Settings* settings, int depth)
{
    if (!tok)
        return false;
    if (!tok->valueType() || !tok->valueType()->container)
        return true;
    if (astIsLHS(tok) && Token::Match(tok->astParent(), "%assign%|<<"))
        return true;
    const Library::Container* container = tok->valueType()->container;
    // Views cannot change container size
    if (container->view)
        return false;
    if (astIsLHS(tok) && Token::simpleMatch(tok->astParent(), "["))
        return container->stdAssociativeLike;
    Library::Container::Action action = astContainerAction(tok);
    Library::Container::Yield yield = astContainerYield(tok);
    switch (action) {
    case Library::Container::Action::RESIZE:
    case Library::Container::Action::CLEAR:
    case Library::Container::Action::PUSH:
    case Library::Container::Action::POP:
    case Library::Container::Action::CHANGE:
    case Library::Container::Action::INSERT:
    case Library::Container::Action::ERASE:
        return true;
    case Library::Container::Action::NO_ACTION:
        // Is this an unknown member function call?
        if (astIsLHS(tok) && Token::Match(tok->astParent(), ". %name% ("))
            return yield == Library::Container::Yield::NO_YIELD;
        break;
    case Library::Container::Action::FIND:
    case Library::Container::Action::CHANGE_CONTENT:
    case Library::Container::Action::CHANGE_INTERNAL:
        break;
    }
    if (isContainerSizeChangedByFunction(tok, settings, depth))
        return true;
    return false;
}

static bool isContainerSizeChanged(nonneg int varId,
                                   const Token* start,
                                   const Token* end,
                                   const Settings* settings,
                                   int depth)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() != varId)
            continue;
        if (isContainerSizeChanged(tok, settings, depth))
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
        if (!astIsSmartPointer(tok))
            continue;
        if (tok->variable() && Token::Match(tok, "%var% (|{|;")) {
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
            }
        } else if (astIsLHS(tok) && Token::Match(tok->astParent(), ". %name% (") &&
                   tok->astParent()->originalName() != "->") {
            std::vector<const Variable*> vars = getVariables(tok);
            Token* ftok = tok->astParent()->tokAt(2);
            if (Token::simpleMatch(tok->astParent(), ". reset (")) {
                if (Token::simpleMatch(ftok, "( )")) {
                    std::list<ValueFlow::Value> values;
                    ValueFlow::Value v(0);
                    v.setKnown();
                    values.push_back(v);
                    valueFlowForwardAssign(ftok, tok, vars, values, false, tokenlist, errorLogger, settings);
                } else {
                    tok->removeValues(std::mem_fn(&ValueFlow::Value::isIntValue));
                    Token* inTok = ftok->astOperand2();
                    if (!inTok)
                        continue;
                    std::list<ValueFlow::Value> values = inTok->values();
                    valueFlowForwardAssign(inTok, tok, vars, values, false, tokenlist, errorLogger, settings);
                }
            } else if (Token::simpleMatch(tok->astParent(), ". release ( )")) {
                const Token* parent = ftok->astParent();
                bool hasParentReset = false;
                while (parent) {
                    if (Token::Match(parent->tokAt(-2), ". release|reset (") &&
                        parent->tokAt(-2)->astOperand1()->exprId() == tok->exprId()) {
                        hasParentReset = true;
                        break;
                    }
                    parent = parent->astParent();
                }
                if (hasParentReset)
                    continue;
                std::list<ValueFlow::Value> values;
                ValueFlow::Value v(0);
                v.setKnown();
                values.push_back(v);
                valueFlowForwardAssign(ftok, tok, vars, values, false, tokenlist, errorLogger, settings);
            } else if (Token::simpleMatch(tok->astParent(), ". get ( )")) {
                ValueFlow::Value v = makeSymbolic(tok);
                setTokenValue(tok->astParent()->tokAt(2), v, settings);
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
        const Token* ftok = nullptr;
        Library::Container::Yield yield = astContainerYield(tok, &ftok);
        if (ftok) {
            ValueFlow::Value v(0);
            v.setKnown();
            if (yield == Library::Container::Yield::START_ITERATOR) {
                v.valueType = ValueFlow::Value::ValueType::ITERATOR_START;
                setTokenValue(ftok->next(), v, settings);
            } else if (yield == Library::Container::Yield::END_ITERATOR) {
                v.valueType = ValueFlow::Value::ValueType::ITERATOR_END;
                setTokenValue(ftok->next(), v, settings);
            }
        }
    }
}

static std::list<ValueFlow::Value> getIteratorValues(std::list<ValueFlow::Value> values, const ValueFlow::Value::ValueKind* kind = nullptr)
{
    values.remove_if([&](const ValueFlow::Value& v) {
        if (kind && v.valueKind != *kind)
            return true;
        return !v.isIteratorValue();
    });
    return values;
}

struct IteratorConditionHandler : SimpleConditionHandler {
    std::vector<Condition> parse(const Token* tok, const Settings*) const override {
        Condition cond;

        ValueFlow::Value true_value;
        ValueFlow::Value false_value;

        if (Token::Match(tok, "==|!=")) {
            if (!tok->astOperand1() || !tok->astOperand2())
                return {};

            ValueFlow::Value::ValueKind kind = ValueFlow::Value::ValueKind::Known;
            std::list<ValueFlow::Value> values = getIteratorValues(tok->astOperand1()->values(), &kind);
            if (!values.empty()) {
                cond.vartok = tok->astOperand2();
            } else {
                values = getIteratorValues(tok->astOperand2()->values(), &kind);
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

        return {cond};
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
            if (!v.condition)
                return true;
            if (v.bound != ValueFlow::Value::Bound::Point)
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

static std::vector<ValueFlow::Value> getContainerValues(const Token* tok)
{
    std::vector<ValueFlow::Value> values;
    if (tok) {
        std::copy_if(tok->values().begin(),
                     tok->values().end(),
                     std::back_inserter(values),
                     std::mem_fn(&ValueFlow::Value::isContainerSizeValue));
    }
    return values;
}

static ValueFlow::Value makeContainerSizeValue(std::size_t s, bool known = true)
{
    ValueFlow::Value value(s);
    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
    if (known)
        value.setKnown();
    return value;
}

static std::vector<ValueFlow::Value> makeContainerSizeValue(const Token* tok, bool known = true)
{
    if (tok->hasKnownIntValue())
        return {makeContainerSizeValue(tok->values().front().intvalue, known)};
    return {};
}

static std::vector<ValueFlow::Value> getContainerSizeFromConstructorArgs(const std::vector<const Token*>& args,
                                                                         const Library::Container* container,
                                                                         bool known)
{
    if (astIsIntegral(args[0], false)) { // { count, i } or { count }
        if (args.size() == 1 || (args.size() > 1 && !astIsIntegral(args[1], false)))
            return {makeContainerSizeValue(args[0], known)};
    } else if (astIsContainer(args[0]) && args.size() == 1) { // copy constructor
        return getContainerValues(args[0]);
    } else if (isIteratorPair(args)) {
        std::vector<ValueFlow::Value> result = getContainerValues(args[0]);
        if (!result.empty())
            return result;
        // (ptr, ptr + size)
        if (astIsPointer(args[0]) && args[0]->exprId() != 0) {
            // (ptr, ptr) is empty
            // TODO: Use lifetime values to check if it points to the same address
            if (args[0]->exprId() == args[1]->exprId())
                return {makeContainerSizeValue(std::size_t{0}, known)};
            // TODO: Insert iterator positions for pointers
            if (Token::simpleMatch(args[1], "+")) {
                nonneg int eid = args[0]->exprId();
                const Token* vartok = args[1]->astOperand1();
                const Token* sizetok = args[1]->astOperand2();
                if (sizetok->exprId() == eid)
                    std::swap(vartok, sizetok);
                if (vartok->exprId() == eid && sizetok->hasKnownIntValue())
                    return {makeContainerSizeValue(sizetok, known)};
            }
        }
    } else if (container->stdStringLike) {
        if (astIsPointer(args[0])) {
            // TODO: Try to read size of string literal { "abc" }
            if (args.size() == 2 && astIsIntegral(args[1], false)) // { char*, count }
                return {makeContainerSizeValue(args[1], known)};
        } else if (astIsContainer(args[0])) {
            if (args.size() == 1) // copy constructor { str }
                return getContainerValues(args[0]);
            if (args.size() == 3) // { str, pos, count }
                return {makeContainerSizeValue(args[2], known)};
            // TODO: { str, pos }, { ..., alloc }
        }
    }
    return {};
}

static std::vector<ValueFlow::Value> getInitListSize(const Token* tok,
                                                     const ValueType* valueType,
                                                     const Settings* settings,
                                                     bool known = true)
{
    std::vector<const Token*> args = getArguments(tok);
    if (args.empty())
        return {makeContainerSizeValue(std::size_t{0}, known)};
    bool initList = true;
    // Try to disambiguate init list from constructor
    if (args.size() < 4) {
        initList = !isIteratorPair(args) && !(args.size() < 3 && astIsIntegral(args[0], false));
        const Token* containerTypeToken = valueType->containerTypeToken;
        if (valueType->container->stdStringLike) {
            initList = astIsGenericChar(args[0]) && !astIsPointer(args[0]);
        } else if (containerTypeToken && settings) {
            ValueType vt = ValueType::parseDecl(containerTypeToken, settings);
            if (vt.pointer > 0 && astIsPointer(args[0]))
                initList = true;
            else if (vt.type == ValueType::ITERATOR && astIsIterator(args[0]))
                initList = true;
            else if (vt.isIntegral() && astIsIntegral(args[0], false))
                initList = true;
        }
    }
    if (!initList)
        return getContainerSizeFromConstructorArgs(args, valueType->container, known);
    return {makeContainerSizeValue(args.size(), known)};
}

static std::vector<ValueFlow::Value> getContainerSizeFromConstructor(const Token* tok,
                                                                     const ValueType* valueType,
                                                                     const Settings* settings,
                                                                     bool known = true)
{
    std::vector<const Token*> args = getArguments(tok);
    if (args.empty())
        return {makeContainerSizeValue(std::size_t{0}, known)};
    // Init list in constructor
    if (args.size() == 1 && Token::simpleMatch(args[0], "{"))
        return getInitListSize(args[0], valueType, settings, known);
    return getContainerSizeFromConstructorArgs(args, valueType->container, known);
}

static void valueFlowContainerSize(TokenList* tokenlist,
                                   SymbolDatabase* symboldatabase,
                                   ErrorLogger* /*errorLogger*/,
                                   const Settings* settings)
{
    // declaration
    for (const Variable *var : symboldatabase->variableList()) {
        if (!var)
            continue;
        if (!var->scope() || !var->scope()->bodyEnd || !var->scope()->bodyStart)
            continue;
        if (!var->valueType() || !var->valueType()->container)
            continue;
        if (!astIsContainer(var->nameToken()))
            continue;

        bool known = true;
        int size = 0;
        bool nonLocal = !var->isLocal() || var->isPointer() || var->isReference() || var->isStatic();
        bool constSize = var->isConst() && !nonLocal;
        bool staticSize = false;
        if (var->valueType()->container->size_templateArgNo >= 0) {
            staticSize = true;
            constSize = true;
            size = -1;
            if (var->dimensions().size() == 1) {
                const Dimension& dim = var->dimensions().front();
                if (dim.known) {
                    size = dim.num;
                } else if (dim.tok && dim.tok->hasKnownIntValue()) {
                    size = dim.tok->values().front().intvalue;
                }
            }
            if (size < 0)
                continue;
        }
        if (!staticSize && nonLocal)
            continue;
        if (var->nameToken()->hasKnownValue(ValueFlow::Value::ValueType::CONTAINER_SIZE))
            continue;
        if (!staticSize) {
            if (!Token::Match(var->nameToken(), "%name% ;") &&
                !(Token::Match(var->nameToken(), "%name% {") &&
                  Token::simpleMatch(var->nameToken()->next()->link(), "} ;")) &&
                !Token::Match(var->nameToken(), "%name% ("))
                continue;
        }
        if (var->nameToken()->astTop() && Token::Match(var->nameToken()->astTop()->previous(), "for|while"))
            known = !isVariableChanged(var, settings, true);
        std::vector<ValueFlow::Value> values{ValueFlow::Value{size}};
        values.back().valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
        if (known)
            values.back().setKnown();
        if (!staticSize) {
            if (Token::simpleMatch(var->nameToken()->next(), "{")) {
                const Token* initList = var->nameToken()->next();
                values = getInitListSize(initList, var->valueType(), settings, known);
            } else if (Token::simpleMatch(var->nameToken()->next(), "(")) {
                const Token* constructorArgs = var->nameToken()->next();
                values = getContainerSizeFromConstructor(constructorArgs, var->valueType(), settings, known);
            }
        }
        for (const ValueFlow::Value& value : values) {
            if (constSize)
                valueFlowForwardConst(var->nameToken()->next(), var->scope()->bodyEnd, var, values, settings);
            else
                valueFlowForward(var->nameToken()->next(), var->nameToken(), value, tokenlist);
        }
    }

    // after assignment
    for (const Scope *functionScope : symboldatabase->functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%name%|;|{|} %var% = %str% ;")) {
                const Token *containerTok = tok->next();
                if (containerTok->exprId() == 0)
                    continue;
                if (containerTok->valueType() && containerTok->valueType()->container && containerTok->valueType()->container->stdStringLike) {
                    ValueFlow::Value value(Token::getStrLength(containerTok->tokAt(2)));
                    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    value.setKnown();
                    valueFlowForward(containerTok->next(), containerTok, value, tokenlist);
                }
            } else if (Token::Match(tok, "%name%|;|{|}|> %var% = {") && Token::simpleMatch(tok->linkAt(3), "} ;")) {
                const Token* containerTok = tok->next();
                if (containerTok->exprId() == 0)
                    continue;
                if (astIsContainer(containerTok) && containerTok->valueType()->container->size_templateArgNo < 0) {
                    std::vector<ValueFlow::Value> values =
                        getInitListSize(tok->tokAt(3), containerTok->valueType(), settings);
                    for (const ValueFlow::Value& value : values)
                        valueFlowForward(containerTok->next(), containerTok, value, tokenlist);
                }
            } else if (Token::Match(tok, ". %name% (") && tok->astOperand1() && tok->astOperand1()->valueType() &&
                       tok->astOperand1()->valueType()->container) {
                const Token* containerTok = tok->astOperand1();
                if (containerTok->exprId() == 0)
                    continue;
                Library::Container::Action action = containerTok->valueType()->container->getAction(tok->strAt(1));
                if (action == Library::Container::Action::CLEAR) {
                    ValueFlow::Value value(0);
                    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    value.setKnown();
                    valueFlowForward(tok->next(), containerTok, value, tokenlist);
                } else if (action == Library::Container::Action::RESIZE && tok->tokAt(2)->astOperand2() &&
                           tok->tokAt(2)->astOperand2()->hasKnownIntValue()) {
                    ValueFlow::Value value(tok->tokAt(2)->astOperand2()->values().front());
                    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    value.setKnown();
                    valueFlowForward(tok->next(), containerTok, value, tokenlist);
                }
            }
        }
    }
}

struct ContainerConditionHandler : ConditionHandler {
    std::vector<Condition> parse(const Token* tok, const Settings* settings) const override
    {
        std::vector<Condition> conds;
        parseCompareEachInt(tok, [&](const Token* vartok, ValueFlow::Value true_value, ValueFlow::Value false_value) {
            Condition cond;
            vartok = settings->library.getContainerFromYield(vartok, Library::Container::Yield::SIZE);
            if (!vartok)
                return;
            true_value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            false_value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            cond.true_values.push_back(true_value);
            cond.false_values.push_back(false_value);
            cond.vartok = vartok;
            conds.push_back(cond);
        });
        if (!conds.empty())
            return conds;

        Condition cond;
        const Token* vartok = nullptr;

        // Empty check
        if (tok->str() == "(") {
            vartok = settings->library.getContainerFromYield(tok, Library::Container::Yield::EMPTY);
            // TODO: Handle .size()
            if (!vartok)
                return {};
            const Token *parent = tok->astParent();
            while (parent) {
                if (Token::Match(parent, "%comp%"))
                    return {};
                parent = parent->astParent();
            }
            ValueFlow::Value value(tok, 0LL);
            value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            cond.true_values.emplace_back(value);
            cond.false_values.emplace_back(std::move(value));
            cond.vartok = vartok;
            cond.inverted = true;
            return {cond};
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
                return {};
            if (!astIsContainer(vartok))
                return {};
            ValueFlow::Value value(tok, Token::getStrLength(strtok));
            value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            cond.false_values.emplace_back(value);
            cond.true_values.emplace_back(std::move(value));
            cond.vartok = vartok;
            cond.impossible = false;
            return {cond};
        }
        return {};
    }
};

static void valueFlowDynamicBufferSize(TokenList* tokenlist, SymbolDatabase* symboldatabase, const Settings* settings)
{
    auto getBufferSizeFromAllocFunc = [&](const Token* funcTok) -> MathLib::bigint {
        MathLib::bigint sizeValue = -1;
        const Library::AllocFunc* allocFunc = settings->library.getAllocFuncInfo(funcTok);
        if (!allocFunc)
            allocFunc = settings->library.getReallocFuncInfo(funcTok);
        if (!allocFunc || allocFunc->bufferSize == Library::AllocFunc::BufferSize::none)
            return sizeValue;

        const std::vector<const Token*> args = getArguments(funcTok);

        const Token* const arg1 = (args.size() >= allocFunc->bufferSizeArg1) ? args[allocFunc->bufferSizeArg1 - 1] : nullptr;
        const Token* const arg2 = (args.size() >= allocFunc->bufferSizeArg2) ? args[allocFunc->bufferSizeArg2 - 1] : nullptr;

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
                const ValueFlow::Value& value = arg1->values().back();
                if (value.isTokValue() && value.tokvalue->tokType() == Token::eString)
                    sizeValue = Token::getStrLength(value.tokvalue) + 1; // Add one for the null terminator
            }
            break;
        }
        return sizeValue;
    };

    auto getBufferSizeFromNew = [&](const Token* newTok) -> MathLib::bigint {
        MathLib::bigint sizeValue = -1, numElem = -1;

        if (newTok && newTok->astOperand1()) { // number of elements
            const Token* bracTok = nullptr, *typeTok = nullptr;
            if (newTok->astOperand1()->str() == "[")
                bracTok = newTok->astOperand1();
            else if (newTok->astOperand1()->str() == "(") {
                if (newTok->astOperand1()->astOperand1() && newTok->astOperand1()->astOperand1()->str() == "[")
                    bracTok = newTok->astOperand1()->astOperand1();
                else
                    typeTok = newTok->astOperand1()->astOperand1();
            }
            else {
                typeTok = newTok->astOperand1();
                if (typeTok->str() == "{")
                    typeTok = typeTok->astOperand1();
            }
            if (bracTok && bracTok->astOperand2() && bracTok->astOperand2()->hasKnownIntValue())
                numElem = bracTok->astOperand2()->getKnownIntValue();
            else if (Token::Match(typeTok, "%type%"))
                numElem = 1;
        }

        if (numElem >= 0 && newTok->astParent() && newTok->astParent()->isAssignmentOp()) { // size of the allocated type
            const Token* typeTok = newTok->astParent()->astOperand1(); // TODO: implement fallback for e.g. "auto p = new Type;"
            if (!typeTok || !typeTok->varId())
                typeTok = newTok->astParent()->previous(); // hack for "int** z = ..."
            if (typeTok && typeTok->valueType()) {
                const MathLib::bigint typeSize = typeTok->valueType()->typeSize(*settings, typeTok->valueType()->pointer > 1);
                if (typeSize >= 0)
                    sizeValue = numElem * typeSize;
            }
        }
        return sizeValue;
    };

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

            const bool isNew = symboldatabase->isCPP() && rhs->str() == "new";
            if (!isNew && !Token::Match(rhs->previous(), "%name% ("))
                continue;

            const MathLib::bigint sizeValue = isNew ? getBufferSizeFromNew(rhs) : getBufferSizeFromAllocFunc(rhs->previous());
            if (sizeValue < 0)
                continue;

            ValueFlow::Value value(sizeValue);
            value.errorPath.emplace_back(tok->tokAt(2), "Assign " + tok->strAt(1) + ", buffer with size " + MathLib::toString(sizeValue));
            value.valueType = ValueFlow::Value::ValueType::BUFFER_SIZE;
            value.setKnown();
            const std::list<ValueFlow::Value> values{value};
            valueFlowForward(const_cast<Token*>(rhs), functionScope->bodyEnd, tok->next(), values, tokenlist);
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

static void valueFlowSafeFunctions(TokenList* tokenlist, SymbolDatabase* symboldatabase, const Settings* settings)
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
                    valueFlowForward(const_cast<Token*>(functionScope->bodyStart), arg.nameToken(), value, tokenlist);
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
                    valueFlowForward(const_cast<Token*>(functionScope->bodyStart->next()),
                                     functionScope->bodyEnd,
                                     arg.nameToken(),
                                     argValues,
                                     tokenlist);
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
                valueFlowForward(const_cast<Token*>(functionScope->bodyStart->next()),
                                 functionScope->bodyEnd,
                                 arg.nameToken(),
                                 argValues,
                                 tokenlist);
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

static void valueFlowDebug(TokenList* tokenlist, ErrorLogger* errorLogger)
{
    if (!tokenlist->getSettings()->debugnormal && !tokenlist->getSettings()->debugwarnings)
        return;
    for (Token* tok = tokenlist->front(); tok; tok = tok->next()) {
        if (tok->getTokenDebug() != TokenDebug::ValueFlow)
            continue;
        if (tok->astParent() && tok->astParent()->getTokenDebug() == TokenDebug::ValueFlow)
            continue;
        for (const ValueFlow::Value& v : tok->values()) {
            std::string msg = "The value is " + debugString(v);
            ErrorPath errorPath = v.errorPath;
            errorPath.insert(errorPath.end(), v.debugPath.begin(), v.debugPath.end());
            errorPath.emplace_back(tok, "");
            errorLogger->reportErr({errorPath, tokenlist, Severity::debug, "valueFlow", msg, CWE{0}, Certainty::normal});
        }
    }
}

ValueFlow::Value::Value(const Token* c, long long val, Bound b)
    : valueType(ValueType::INT),
    bound(b),
    intvalue(val),
    tokvalue(nullptr),
    floatValue(0.0),
    moveKind(MoveKind::NonMovedVariable),
    varvalue(val),
    condition(c),
    varId(0),
    safe(false),
    conditional(false),
    macro(false),
    defaultArg(false),
    indirect(0),
    path(0),
    wideintvalue(0),
    subexpressions(),
    capturetok(nullptr),
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

std::string ValueFlow::Value::toString() const
{
    std::stringstream ss;
    if (this->isImpossible())
        ss << "!";
    if (this->bound == ValueFlow::Value::Bound::Lower)
        ss << ">=";
    if (this->bound == ValueFlow::Value::Bound::Upper)
        ss << "<=";
    switch (this->valueType) {
    case ValueFlow::Value::ValueType::INT:
        ss << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::TOK:
        ss << this->tokvalue->str();
        break;
    case ValueFlow::Value::ValueType::FLOAT:
        ss << this->floatValue;
        break;
    case ValueFlow::Value::ValueType::MOVED:
        ss << ValueFlow::Value::toString(this->moveKind);
        break;
    case ValueFlow::Value::ValueType::UNINIT:
        ss << "Uninit";
        break;
    case ValueFlow::Value::ValueType::BUFFER_SIZE:
    case ValueFlow::Value::ValueType::CONTAINER_SIZE:
        ss << "size=" << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::ITERATOR_START:
        ss << "start=" << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::ITERATOR_END:
        ss << "end=" << this->intvalue;
        break;
    case ValueFlow::Value::ValueType::LIFETIME:
        ss << "lifetime[" << ValueFlow::Value::toString(this->lifetimeKind) << "]=("
           << this->tokvalue->expressionString() << ")";
        break;
    case ValueFlow::Value::ValueType::SYMBOLIC:
        ss << "symbolic=(" << this->tokvalue->expressionString();
        if (this->intvalue > 0)
            ss << "+" << this->intvalue;
        else if (this->intvalue < 0)
            ss << "-" << -this->intvalue;
        ss << ")";
        break;
    }
    if (this->indirect > 0)
        for (int i = 0; i < this->indirect; i++)
            ss << "*";
    if (this->path > 0)
        ss << "@" << this->path;
    return ss.str();
}

std::string ValueFlow::Value::infoString() const
{
    switch (valueType) {
    case ValueType::INT:
        return MathLib::toString(intvalue);
    case ValueType::TOK:
        return tokvalue->str();
    case ValueType::FLOAT:
        return MathLib::toString(floatValue);
    case ValueType::MOVED:
        return "<Moved>";
    case ValueType::UNINIT:
        return "<Uninit>";
    case ValueType::BUFFER_SIZE:
    case ValueType::CONTAINER_SIZE:
        return "size=" + MathLib::toString(intvalue);
    case ValueType::ITERATOR_START:
        return "start=" + MathLib::toString(intvalue);
    case ValueType::ITERATOR_END:
        return "end=" + MathLib::toString(intvalue);
    case ValueType::LIFETIME:
        return "lifetime=" + tokvalue->str();
    case ValueType::SYMBOLIC:
        std::string result = "symbolic=" + tokvalue->expressionString();
        if (intvalue > 0)
            result += "+" + MathLib::toString(intvalue);
        else if (intvalue < 0)
            result += "-" + MathLib::toString(-intvalue);
        return result;
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

const char* ValueFlow::Value::toString(LifetimeKind lifetimeKind)
{
    switch (lifetimeKind) {
    case LifetimeKind::Object:
        return "Object";
    case LifetimeKind::SubObject:
        return "SubObject";
    case LifetimeKind::Lambda:
        return "Lambda";
    case LifetimeKind::Iterator:
        return "Iterator";
    case LifetimeKind::Address:
        return "Address";
    }
    return "";
}

bool ValueFlow::Value::sameToken(const Token* tok1, const Token* tok2)
{
    if (tok1 == tok2)
        return true;
    if (!tok1)
        return false;
    if (tok1->exprId() == 0 || tok2->exprId() == 0)
        return false;
    return tok1->exprId() == tok2->exprId();
}
const char* ValueFlow::Value::toString(LifetimeScope lifetimeScope)
{
    switch (lifetimeScope) {
    case ValueFlow::Value::LifetimeScope::Local:
        return "Local";
    case ValueFlow::Value::LifetimeScope::Argument:
        return "Argument";
    case ValueFlow::Value::LifetimeScope::SubFunction:
        return "SubFunction";
    case ValueFlow::Value::LifetimeScope::ThisPointer:
        return "ThisPointer";
    case ValueFlow::Value::LifetimeScope::ThisValue:
        return "ThisValue";
    }
    return "";
}
const char* ValueFlow::Value::toString(Bound bound)
{
    switch (bound) {
    case ValueFlow::Value::Bound::Point:
        return "Point";
    case ValueFlow::Value::Bound::Upper:
        return "Upper";
    case ValueFlow::Value::Bound::Lower:
        return "Lower";
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
    valueFlowSymbolic(tokenlist, symboldatabase);
    valueFlowBitAnd(tokenlist);
    valueFlowSameExpressions(tokenlist);
    valueFlowConditionExpressions(tokenlist, symboldatabase, errorLogger, settings);

    std::size_t values = 0;
    std::size_t n = 4;
    while (n > 0 && values < getTotalValues(tokenlist)) {
        values = getTotalValues(tokenlist);
        valueFlowImpossibleValues(tokenlist, settings);
        valueFlowSymbolicIdentity(tokenlist);
        valueFlowSymbolicAbs(tokenlist, symboldatabase);
        valueFlowCondition(SymbolicConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings);
        valueFlowSymbolicInfer(tokenlist, symboldatabase);
        valueFlowArrayBool(tokenlist);
        valueFlowArrayElement(tokenlist, settings);
        valueFlowRightShift(tokenlist, settings);
        valueFlowAfterAssign(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowAfterSwap(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowCondition(SimpleConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings);
        valueFlowInferCondition(tokenlist, settings);
        valueFlowSwitchVariable(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowForLoop(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowSubFunction(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowFunctionReturn(tokenlist, errorLogger);
        valueFlowLifetime(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowFunctionDefaultParameter(tokenlist, symboldatabase);
        valueFlowUninit(tokenlist, symboldatabase, settings);
        if (tokenlist->isCPP()) {
            valueFlowAfterMove(tokenlist, symboldatabase, settings);
            valueFlowSmartPointer(tokenlist, errorLogger, settings);
            valueFlowIterators(tokenlist, settings);
            valueFlowCondition(IteratorConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings);
            valueFlowIteratorInfer(tokenlist, settings);
            valueFlowContainerSize(tokenlist, symboldatabase, errorLogger, settings);
            valueFlowCondition(ContainerConditionHandler{}, tokenlist, symboldatabase, errorLogger, settings);
        }
        valueFlowSafeFunctions(tokenlist, symboldatabase, settings);
        n--;
    }

    valueFlowDynamicBufferSize(tokenlist, symboldatabase, settings);

    valueFlowDebug(tokenlist, errorLogger);
}

ValueFlow::Value ValueFlow::Value::unknown()
{
    Value v;
    v.valueType = Value::ValueType::UNINIT;
    return v;
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

const ValueFlow::Value* ValueFlow::findValue(const std::list<ValueFlow::Value>& values,
                                             const Settings* settings,
                                             std::function<bool(const ValueFlow::Value&)> pred)
{
    const ValueFlow::Value* ret = nullptr;
    for (const ValueFlow::Value& v : values) {
        if (pred(v)) {
            if (!ret || ret->isInconclusive() || (ret->condition && !v.isInconclusive()))
                ret = &v;
            if (!ret->isInconclusive() && !ret->condition)
                break;
        }
    }
    if (settings && ret) {
        if (ret->isInconclusive() && !settings->certainty.isEnabled(Certainty::inconclusive))
            return nullptr;
        if (ret->condition && !settings->severity.isEnabled(Severity::warning))
            return nullptr;
    }
    return ret;
}

static std::vector<ValueFlow::Value> isOutOfBoundsImpl(const ValueFlow::Value& size,
                                                       const Token* indexTok,
                                                       bool condition)
{
    if (!indexTok)
        return {};
    const ValueFlow::Value* indexValue = indexTok->getMaxValue(condition, size.path);
    if (!indexValue)
        return {};
    if (indexValue->intvalue >= size.intvalue)
        return {*indexValue};
    if (!condition)
        return {};
    // TODO: Use a better way to decide if the variable in unconstrained
    if (!indexTok->variable() || !indexTok->variable()->isArgument())
        return {};
    if (std::any_of(indexTok->values().begin(), indexTok->values().end(), [&](const ValueFlow::Value& v) {
        return v.isSymbolicValue() && v.isPossible() && v.bound == ValueFlow::Value::Bound::Upper;
    }))
        return {};
    if (indexValue->bound != ValueFlow::Value::Bound::Lower)
        return {};
    if (size.bound == ValueFlow::Value::Bound::Lower)
        return {};
    ValueFlow::Value value = inferCondition(">=", indexTok, indexValue->intvalue);
    if (!value.isKnown())
        return {};
    if (value.intvalue == 0)
        return {};
    value.intvalue = size.intvalue;
    value.bound = ValueFlow::Value::Bound::Lower;
    return {value};
}

std::vector<ValueFlow::Value> ValueFlow::isOutOfBounds(const Value& size, const Token* indexTok, bool possible)
{
    ValueFlow::Value inBoundsValue = inferCondition("<", indexTok, size.intvalue);
    if (inBoundsValue.isKnown() && inBoundsValue.intvalue != 0)
        return {};
    std::vector<ValueFlow::Value> result = isOutOfBoundsImpl(size, indexTok, false);
    if (!result.empty())
        return result;
    if (!possible)
        return result;
    return isOutOfBoundsImpl(size, indexTok, true);
}

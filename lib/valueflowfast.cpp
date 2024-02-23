/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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
 * All value flow analysis is executed in the ValueFlowFast::setValues() function. The ValueFlow analysis is executed after
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

#include "valueflowfast.h"

#include "astutils.h"
#include "calculate.h"
#include "errorlogger.h"
#include "fwdanalysis.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "programmemoryfast.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "valueflow.h" // <- ValueFlow::getSizeOf

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <stack>
#include <sstream>
#include <vector>

static const int TIMEOUT = 10; // Do not repeat ValueFlow analysis more than 10 seconds

static void bailoutInternal(const std::string& type, const TokenList &tokenlist, ErrorLogger *errorLogger, const Token *tok, const std::string &what, const std::string &file, int line, std::string function)
{
    if (function.find("operator") != std::string::npos)
        function = "(valueFlow)";
    std::list<ErrorMessage::FileLocation> callstack(1, ErrorMessage::FileLocation(tok, &tokenlist));
    const std::string location = Path::stripDirectoryPart(file) + ":" + std::to_string(line) + ":";
    ErrorMessage errmsg(std::move(callstack), tokenlist.getSourceFilePath(), Severity::debug,
                        (file.empty() ? "" : location) + function + " bailout: " + what, type, Certainty::normal);
    errorLogger->reportErr(errmsg);
}

#define bailout2(type, tokenlist, errorLogger, tok, what) bailoutInternal((type), (tokenlist), (errorLogger), (tok), (what), __FILE__, __LINE__, __func__)

#define bailout(tokenlist, errorLogger, tok, what) bailout2("valueFlowBailout", (tokenlist), (errorLogger), (tok), (what))

#define bailoutIncompleteVar(tokenlist, errorLogger, tok, what) bailoutInternal("valueFlowBailoutIncompleteVar", (tokenlist), (errorLogger), (tok), (what), "", 0, __func__)

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
    std::string s = Path::stripDirectoryPart(file) + ":" + std::to_string(ctx.line()) + ": " + ctx.function_name() +
                    " => " + local.function_name() + ": " + debugString(v);
    v.debugPath.emplace_back(tok, std::move(s));
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

static void lowerToInconclusive(std::list<ValueFlow::Value>& values, const Settings& settings, int indirect = -1)
{
    if (settings.certainty.isEnabled(Certainty::inconclusive)) {
        removeImpossible(values, indirect);
        for (ValueFlow::Value& v : values) {
            if (indirect >= 0 && v.indirect != indirect)
                continue;
            v.setInconclusive();
        }
    } else {
        // Remove all values if the inconclusive flags is not set
        values.remove_if([&](const ValueFlow::Value& v) {
            if (indirect >= 0 && v.indirect != indirect)
                return false;
            return true;
        });
    }
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

        if (tok->hasKnownIntValue())
            return tok;

        // Is variable protected in LHS..
        bool bailout = false;
        visitAstNodes(tok->astOperand1(), [&](const Token *tok2) {
            if (tok2->str() == ".")
                return ChildrenToVisit::none;
            // A variable is seen..
            if (tok2 != valuetok && tok2->variable() &&
                (tok2->varId() == valuetok->varId() || (!tok2->variable()->isArgument() && !tok2->hasKnownIntValue()))) {
                // TODO: limit this bailout
                bailout = true;
                return ChildrenToVisit::done;
            }
            return ChildrenToVisit::op1_and_op2;
        });
        if (bailout)
            return tok;
    }
    return nullptr;
}

static bool isEscapeScope(const Token* tok, const Settings& settings, bool unknown = false)
{
    if (!Token::simpleMatch(tok, "{"))
        return false;
    // TODO this search for termTok in all subscopes. It should check the end of the scope.
    const Token * termTok = Token::findmatch(tok, "return|continue|break|throw|goto", tok->link());
    if (termTok && termTok->scope() == tok->scope())
        return true;
    std::string unknownFunction;
    if (settings.library.isScopeNoReturn(tok->link(), &unknownFunction))
        return unknownFunction.empty() || unknown;
    return false;
}

static bool bailoutSelfAssignment(const Token * const tok)
{
    const Token *parent = tok;
    while (parent) {
        const Token *op = parent;
        parent = parent->astParent();

        // Assignment where lhs variable exists in rhs => return true
        if (parent                         != nullptr &&
            parent->astOperand2()          == op &&
            parent->astOperand1()          != nullptr &&
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
        constexpr MathLib::biguint one = 1;
        value.intvalue &= (one << bit) - 1;
        if (sign == ValueType::Sign::SIGNED && value.intvalue & (one << (bit - 1))) {
            value.intvalue |= ~((one << bit) - 1ULL);
        }
    }
    return value;
}

static const Token *getCastTypeStartToken(const Token *parent, const Settings& settings)
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
    if (!parent->astOperand2() && Token::Match(parent, "( %name%|::")) {
        const Token* ftok = parent->next();
        if (ftok->isStandardType())
            return ftok;
        if (Token::simpleMatch(ftok, "::"))
            ftok = ftok->next();
        while (Token::Match(ftok, "%name% ::"))
            ftok = ftok->tokAt(2);
        if (settings.library.isNotLibraryFunction(ftok))
            return parent->next();
    }
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
    if (value.isTokValue() && (!parent->isComparisonOp() || !Token::Match(value.tokvalue, "{|%str%")))
        return false;
    return true;
}
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

static bool isNumeric(const ValueFlow::Value& value) {
    return value.isIntValue() || value.isFloatValue();
}

static ValueFlow::Value truncateImplicitConversion(Token* parent, const ValueFlow::Value& value, const Settings& settings)
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
    const size_t n1 = ValueFlow::getSizeOf(*vt1, settings);
    const size_t n2 = ValueFlow::getSizeOf(*vt2, settings);
    ValueType::Sign sign = ValueType::Sign::UNSIGNED;
    if (n1 < n2)
        sign = vt2->sign;
    else if (n1 > n2)
        sign = vt1->sign;
    ValueFlow::Value v = castValue(value, sign, std::max(n1, n2) * 8);
    v.wideintvalue = value.intvalue;
    return v;
}

static Library::Container::Yield getContainerYield(Token* tok, const Settings& settings, Token** parent = nullptr)
{
    if (Token::Match(tok, ". %name% (") && tok->astParent() == tok->tokAt(2) && tok->astOperand1() &&
        tok->astOperand1()->valueType()) {
        const Library::Container* c = getLibraryContainer(tok->astOperand1());
        if (parent)
            *parent = tok->astParent();
        return c ? c->getYield(tok->strAt(1)) : Library::Container::Yield::NO_YIELD;
    }
    if (Token::Match(tok->previous(), "%name% (")) {
        if (parent)
            *parent = tok;
        if (const Library::Function* f = settings.library.getFunction(tok->previous())) {
            return f->containerYield;
        }
    }
    return Library::Container::Yield::NO_YIELD;
}

/** Set token value for cast */
static void setTokenValueCast(Token *parent, const ValueType &valueType, const ValueFlow::Value &value, const Settings &settings);

/** set ValueFlow value and perform calculations if possible */
static void setTokenValue(Token* tok,
                          ValueFlow::Value value,
                          const Settings& settings,
                          SourceLocation loc = SourceLocation::current())
{
    // Skip setting values that are too big since its ambiguous
    if (!value.isImpossible() && value.isIntValue() && value.intvalue < 0 && astIsUnsigned(tok) &&
        ValueFlow::getSizeOf(*tok->valueType(), settings) >= sizeof(MathLib::bigint))
        return;

    if (!value.isImpossible() && value.isIntValue())
        value = truncateImplicitConversion(tok->astParent(), value, settings);

    if (settings.debugnormal)
        setSourceLocation(value, loc, tok);

    if (!tok->addValue(value))
        return;

    if (value.path < 0)
        return;

    Token *parent = tok->astParent();
    if (!parent)
        return;

    if (Token::simpleMatch(parent, ",") && !parent->isInitComma() && astIsRHS(tok)) {
        const Token* callParent = findParent(parent, [](const Token* p) {
            return !Token::simpleMatch(p, ",");
        });
        // Ensure that the comma isn't a function call
        if (!callParent || (!Token::Match(callParent->previous(), "%name%|> (") && !Token::simpleMatch(callParent, "{") &&
                            (!Token::Match(callParent, "( %name%") || settings.library.isNotLibraryFunction(callParent->next())) &&
                            !(callParent->str() == "(" && (Token::simpleMatch(callParent->astOperand1(), "*") || Token::Match(callParent->astOperand1(), "%name%|("))))) {
            setTokenValue(parent, std::move(value), settings);
            return;
        }
    }

    if (Token::simpleMatch(parent, "=") && astIsRHS(tok)) {
        setTokenValue(parent, value, settings);
        if (!value.isUninitValue())
            return;
    }

    if (value.isContainerSizeValue() && astIsContainer(tok)) {
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

                    combineValueProperties(value1, value2, result);

                    if (Token::simpleMatch(parent, "==") && result.intvalue)
                        continue;
                    if (Token::simpleMatch(parent, "!=") && !result.intvalue)
                        continue;

                    setTokenValue(parent, std::move(result), settings);
                }
            }
        }
        Token* next = nullptr;
        const Library::Container::Yield yields = getContainerYield(parent, settings, &next);
        if (yields == Library::Container::Yield::SIZE) {
            ValueFlow::Value v(value);
            v.valueType = ValueFlow::Value::ValueType::INT;
            setTokenValue(next, std::move(v), settings);
        } else if (yields == Library::Container::Yield::EMPTY) {
            ValueFlow::Value v(value);
            v.valueType = ValueFlow::Value::ValueType::INT;
            v.bound = ValueFlow::Value::Bound::Point;
            if (value.isImpossible()) {
                if (value.intvalue == 0)
                    v.setKnown();
                else if ((value.bound == ValueFlow::Value::Bound::Upper && value.intvalue > 0) ||
                         (value.bound == ValueFlow::Value::Bound::Lower && value.intvalue < 0)) {
                    v.intvalue = 0;
                    v.setKnown();
                } else
                    v.setPossible();
            } else {
                v.intvalue = !v.intvalue;
            }
            setTokenValue(next, std::move(v), settings);
        }
        return;
    }

    if (value.isLifetimeValue()) {
        if (!ValueFlow::isLifetimeBorrowed(parent, settings))
            return;
        if (value.lifetimeKind == ValueFlow::Value::LifetimeKind::Iterator && astIsIterator(parent)) {
            setTokenValue(parent,std::move(value),settings);
        } else if (astIsPointer(tok) && astIsPointer(parent) && !parent->isUnaryOp("*") &&
                   (parent->isArithmeticalOp() || parent->isCast())) {
            setTokenValue(parent,std::move(value),settings);
        }
        return;
    }

    if (value.isUninitValue()) {
        if (Token::Match(tok, ". %var%"))
            setTokenValue(tok->next(), value, settings);
        if (parent->isCast()) {
            setTokenValue(parent, std::move(value), settings);
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
            setTokenValue(parent, std::move(pvalue), settings);
        } else if (Token::Match(parent, ". %var%") && parent->astOperand1() == tok && parent->astOperand2()) {
            if (parent->originalName() == "->" && pvalue.indirect > 0)
                pvalue.indirect--;
            setTokenValue(parent->astOperand2(), std::move(pvalue), settings);
        } else if (Token::Match(parent->astParent(), ". %var%") && parent->astParent()->astOperand1() == parent) {
            if (parent->astParent()->originalName() == "->" && pvalue.indirect > 0)
                pvalue.indirect--;
            setTokenValue(parent->astParent()->astOperand2(), std::move(pvalue), settings);
        } else if (parent->isUnaryOp("*") && pvalue.indirect > 0) {
            pvalue.indirect--;
            setTokenValue(parent, std::move(pvalue), settings);
        }
        return;
    }

    // cast..
    if (const Token *castType = getCastTypeStartToken(parent, settings)) {
        if (contains({ValueFlow::Value::ValueType::INT, ValueFlow::Value::ValueType::SYMBOLIC}, value.valueType) &&
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
        setTokenValue(parent,std::move(value),settings);
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
                if (std::find(values.cbegin(), values.cend(), value) != values.cend())
                    setTokenValue(parent, std::move(value), settings);
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

            ValueFlow::Value v(std::move(value));
            v.conditional = true;
            v.changeKnownToPossible();

            setTokenValue(parent, std::move(v), settings);
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
            setTokenValue(parent, std::move(value), settings);
            return;
        }

        // known result when a operand is true.
        if (Token::simpleMatch(parent, "&&") && value.isKnown() && value.isIntValue() && value.intvalue==0) {
            setTokenValue(parent, std::move(value), settings);
            return;
        }

        // known result when a operand is false.
        if (Token::simpleMatch(parent, "||") && value.isKnown() && value.isIntValue() && value.intvalue!=0) {
            setTokenValue(parent, std::move(value), settings);
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
                combineValueProperties(value1, value2, result);
                if (astIsFloat(parent, false)) {
                    if (!result.isIntValue() && !result.isFloatValue())
                        continue;
                    result.valueType = ValueFlow::Value::ValueType::FLOAT;
                }
                const double floatValue1 = value1.isFloatValue() ? value1.floatValue : value1.intvalue;
                const double floatValue2 = value2.isFloatValue() ? value2.floatValue : value2.intvalue;
                const auto intValue1 = [&]() -> MathLib::bigint {
                    return value1.isFloatValue() ? static_cast<MathLib::bigint>(value1.floatValue) : value1.intvalue;
                };
                const auto intValue2 = [&]() -> MathLib::bigint {
                    return value2.isFloatValue() ? static_cast<MathLib::bigint>(value2.floatValue) : value2.intvalue;
                };
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
                        result.intvalue = calculate(parent->str(), intValue1(), intValue2(), &error);
                        if (error)
                            continue;
                    } else if (value1.isTokValue() && value2.isTokValue() &&
                               (astIsContainer(parent->astOperand1()) || astIsContainer(parent->astOperand2()))) {
                        const Token* tok1 = value1.tokvalue;
                        const Token* tok2 = value2.tokvalue;
                        bool equal = false;
                        if (Token::Match(tok1, "%str%") && Token::Match(tok2, "%str%")) {
                            equal = tok1->str() == tok2->str();
                        } else if (Token::simpleMatch(tok1, "{") && Token::simpleMatch(tok2, "{")) {
                            std::vector<const Token*> args1 = getArguments(tok1);
                            std::vector<const Token*> args2 = getArguments(tok2);
                            if (args1.size() == args2.size()) {
                                if (!std::all_of(args1.begin(), args1.end(), std::mem_fn(&Token::hasKnownIntValue)))
                                    continue;
                                if (!std::all_of(args2.begin(), args2.end(), std::mem_fn(&Token::hasKnownIntValue)))
                                    continue;
                                equal = std::equal(args1.begin(),
                                                   args1.end(),
                                                   args2.begin(),
                                                   [&](const Token* atok, const Token* btok) {
                                    return atok->values().front().intvalue ==
                                    btok->values().front().intvalue;
                                });
                            } else {
                                equal = false;
                            }
                        } else {
                            continue;
                        }
                        result.intvalue = parent->str() == "==" ? equal : !equal;
                    } else {
                        continue;
                    }
                    setTokenValue(parent, std::move(result), settings);
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
                        result.intvalue = calculate(parent->str(), intValue1(), intValue2(), &error);
                    }
                    if (error)
                        continue;
                    // If the bound comes from the second value then invert the bound when subtracting
                    if (Token::simpleMatch(parent, "-") && value2.bound == result.bound &&
                        value2.bound != ValueFlow::Value::Bound::Point)
                        result.invertBound();
                    setTokenValue(parent, std::move(result), settings);
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
            if (val.isImpossible())
                v.setKnown();
            else
                v.intvalue = !v.intvalue;
            setTokenValue(parent, std::move(v), settings);
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
                    bits = settings.platform.int_bit;
                else if (tok->valueType()->type == ValueType::Type::LONG)
                    bits = settings.platform.long_bit;
            }
            if (bits > 0 && bits < MathLib::bigint_bits)
                v.intvalue &= (((MathLib::biguint)1)<<bits) - 1;
            setTokenValue(parent, std::move(v), settings);
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
            setTokenValue(parent, std::move(v), settings);
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
            setTokenValue(parent, std::move(v), settings);
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
            setTokenValue(parent, std::move(v), settings);
        }
    }

    // C++ init
    else if (parent->str() == "{" && Token::simpleMatch(parent->previous(), "= {") && Token::simpleMatch(parent->link(), "} ;")) {
        const Token* lhs = parent->previous()->astOperand1();
        if (lhs && lhs->valueType()) {
            if (lhs->valueType()->isIntegral() || lhs->valueType()->isFloat() || (lhs->valueType()->pointer > 0 && value.isIntValue())) {
                setTokenValue(parent, std::move(value), settings);
            }
        }
    }

    else if (Token::Match(parent, ":: %name%") && parent->astOperand2() == tok) {
        setTokenValue(parent, std::move(value), settings);
    }
    // Calling std::size or std::empty on an array
    else if (value.isTokValue() && Token::simpleMatch(value.tokvalue, "{") && tok->variable() &&
             tok->variable()->isArray() && Token::Match(parent->previous(), "%name% (") && astIsRHS(tok)) {
        std::vector<const Token*> args = getArguments(value.tokvalue);
        if (const Library::Function* f = settings.library.getFunction(parent->previous())) {
            if (f->containerYield == Library::Container::Yield::SIZE) {
                ValueFlow::Value v(std::move(value));
                v.valueType = ValueFlow::Value::ValueType::INT;
                v.intvalue = args.size();
                setTokenValue(parent, std::move(v), settings);
            } else if (f->containerYield == Library::Container::Yield::EMPTY) {
                ValueFlow::Value v(std::move(value));
                v.intvalue = args.empty();
                v.valueType = ValueFlow::Value::ValueType::INT;
                setTokenValue(parent, std::move(v), settings);
            }
        }
    }
}

static void setTokenValueCast(Token *parent, const ValueType &valueType, const ValueFlow::Value &value, const Settings &settings)
{
    if (valueType.pointer || value.isImpossible())
        setTokenValue(parent,value,settings);
    else if (valueType.type == ValueType::Type::CHAR)
        setTokenValue(parent, castValue(value, valueType.sign, settings.platform.char_bit), settings);
    else if (valueType.type == ValueType::Type::SHORT)
        setTokenValue(parent, castValue(value, valueType.sign, settings.platform.short_bit), settings);
    else if (valueType.type == ValueType::Type::INT)
        setTokenValue(parent, castValue(value, valueType.sign, settings.platform.int_bit), settings);
    else if (valueType.type == ValueType::Type::LONG)
        setTokenValue(parent, castValue(value, valueType.sign, settings.platform.long_bit), settings);
    else if (valueType.type == ValueType::Type::LONGLONG)
        setTokenValue(parent, castValue(value, valueType.sign, settings.platform.long_long_bit), settings);
    else if (valueType.isFloat() && isNumeric(value)) {
        ValueFlow::Value floatValue = value;
        floatValue.valueType = ValueFlow::Value::ValueType::FLOAT;
        if (value.isIntValue())
            floatValue.floatValue = value.intvalue;
        setTokenValue(parent, std::move(floatValue), settings);
    } else if (value.isIntValue()) {
        const long long charMax = settings.platform.signedCharMax();
        const long long charMin = settings.platform.signedCharMin();
        if (charMin <= value.intvalue && value.intvalue <= charMax) {
            // unknown type, but value is small so there should be no truncation etc
            setTokenValue(parent,value,settings);
        }
    }
}

static nonneg int getSizeOfType(const Token *typeTok, const Settings &settings)
{
    const ValueType &valueType = ValueType::parseDecl(typeTok, settings);

    return ValueFlow::getSizeOf(valueType, settings);
}

static bool getMinMaxValues(const ValueType *vt, const Platform &platform, MathLib::bigint &minValue, MathLib::bigint &maxValue)
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
        minValue = 0;
        maxValue = 1;
    } else if (bits < 62) {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            minValue = 0;
            maxValue = (1LL << bits) - 1;
        } else {
            minValue = -(1LL << (bits - 1));
            maxValue = (1LL << (bits - 1)) - 1;
        }
    } else if (bits == 64) {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            minValue = 0;
            maxValue = LLONG_MAX; // todo max unsigned value
        } else {
            minValue = LLONG_MIN;
            maxValue = LLONG_MAX;
        }
    } else {
        return false;
    }

    return true;
}

/*
   static bool getMinMaxValues(const std::string &typestr, const Settings &settings, MathLib::bigint &minvalue, MathLib::bigint &maxvalue)
   {
    TokenList typeTokens(&settings);
    std::istringstream istr(typestr+";");
    if (!typeTokens.createTokens(istr, Standards::Language::CPP))
        return false;
    typeTokens.simplifyPlatformTypes();
    typeTokens.simplifyStdType();
    const ValueType &vt = ValueType::parseDecl(typeTokens.front(), settings);
    return getMinMaxValues(&vt, settings.platform, minvalue, maxvalue);
   }
 */

static void valueFlowEnumValue(SymbolDatabase & symboldatabase, const Settings & settings)
{
    for (Scope & scope : symboldatabase.scopeList) {
        if (scope.type != Scope::eEnum)
            continue;
        MathLib::bigint value = 0;
        bool prev_enum_is_known = true;

        for (Enumerator & enumerator : scope.enumeratorList) {
            if (enumerator.start) {
                Token* rhs = const_cast<Token*>(enumerator.start->previous()->astOperand2());
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

// Handle various constants..
static Token * valueFlowSetConstantValue(Token *tok, const Settings &settings, bool cpp)
{
    if ((tok->isNumber() && MathLib::isInt(tok->str())) || (tok->tokType() == Token::eChar)) {
        try {
            MathLib::bigint signedValue = MathLib::toBigNumber(tok->str());
            const ValueType* vt = tok->valueType();
            if (vt && vt->sign == ValueType::UNSIGNED && signedValue < 0 && ValueFlow::getSizeOf(*vt, settings) < sizeof(MathLib::bigint)) {
                MathLib::bigint minValue{}, maxValue{};
                if (getMinMaxValues(tok->valueType(), settings.platform, minValue, maxValue))
                    signedValue += maxValue + 1;
            }
            ValueFlow::Value value(signedValue);
            if (!tok->isTemplateArg())
                value.setKnown();
            setTokenValue(tok, std::move(value), settings);
        } catch (const std::exception & /*e*/) {
            // Bad character literal
        }
    } else if (tok->isNumber() && MathLib::isFloat(tok->str())) {
        ValueFlow::Value value;
        value.valueType = ValueFlow::Value::ValueType::FLOAT;
        value.floatValue = MathLib::toDoubleNumber(tok->str());
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, std::move(value), settings);
    } else if (tok->enumerator() && tok->enumerator()->value_known) {
        ValueFlow::Value value(tok->enumerator()->value);
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, std::move(value), settings);
    } else if (tok->str() == "NULL" || (cpp && tok->str() == "nullptr")) {
        ValueFlow::Value value(0);
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok, std::move(value), settings);
    } else if (Token::simpleMatch(tok, "sizeof (")) {
        if (tok->next()->astOperand2() && !tok->next()->astOperand2()->isLiteral() && tok->next()->astOperand2()->valueType() &&
            (tok->next()->astOperand2()->valueType()->pointer == 0 || // <- TODO this is a bailout, abort when there are array->pointer conversions
             (tok->next()->astOperand2()->variable() && !tok->next()->astOperand2()->variable()->isArray())) &&
            !tok->next()->astOperand2()->valueType()->isEnum()) { // <- TODO this is a bailout, handle enum with non-int types
            const size_t sz = ValueFlow::getSizeOf(*tok->next()->astOperand2()->valueType(), settings);
            if (sz) {
                ValueFlow::Value value(sz);
                value.setKnown();
                setTokenValue(tok->next(), std::move(value), settings);
                return tok->linkAt(1);
            }
        }

        const Token *tok2 = tok->tokAt(2);
        // skip over tokens to find variable or type
        while (tok2 && !tok2->isStandardType() && Token::Match(tok2, "%name% ::|.|[")) {
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
                if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                    value.setKnown();
                setTokenValue(tok->next(), std::move(value), settings);
            }
        } else if (tok2->enumerator() && tok2->enumerator()->scope) {
            long long size = settings.platform.sizeof_int;
            const Token * type = tok2->enumerator()->scope->enumType;
            if (type) {
                size = getSizeOfType(type, settings);
                if (size == 0)
                    tok->linkAt(1);
            }
            ValueFlow::Value value(size);
            if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                value.setKnown();
            setTokenValue(tok, value, settings);
            setTokenValue(tok->next(), std::move(value), settings);
        } else if (tok2->type() && tok2->type()->isEnumType()) {
            long long size = settings.platform.sizeof_int;
            if (tok2->type()->classScope) {
                const Token * type = tok2->type()->classScope->enumType;
                if (type) {
                    size = getSizeOfType(type, settings);
                }
            }
            ValueFlow::Value value(size);
            if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                value.setKnown();
            setTokenValue(tok, value, settings);
            setTokenValue(tok->next(), std::move(value), settings);
        } else if (Token::Match(tok, "sizeof ( %var% ) /") && tok->next()->astParent() == tok->tokAt(4) &&
                   tok->tokAt(4)->astOperand2() && Token::simpleMatch(tok->tokAt(4)->astOperand2()->previous(), "sizeof (")) {
            // Get number of elements in array
            const Token *sz1 = tok->tokAt(2);
            const Token *sz2 = tok->tokAt(4)->astOperand2(); // left parenthesis of sizeof on rhs
            const nonneg int varid1 = sz1->varId();
            if (varid1 &&
                sz1->variable() &&
                sz1->variable()->isArray() &&
                !sz1->variable()->dimensions().empty() &&
                sz1->variable()->dimensionKnown(0) &&
                Token::Match(sz2->astOperand2(), "*|[") && Token::Match(sz2->astOperand2()->astOperand1(), "%varid%", varid1)) {
                ValueFlow::Value value(sz1->variable()->dimension(0));
                if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                    value.setKnown();
                setTokenValue(tok->tokAt(4), std::move(value), settings);
            }
        } else if (Token::Match(tok2, "%var% )")) {
            const Variable *var = tok2->variable();
            // only look for single token types (no pointers or references yet)
            if (var && var->typeStartToken() == var->typeEndToken()) {
                // find the size of the type
                size_t size = 0;
                if (var->isEnumType()) {
                    size = settings.platform.sizeof_int;
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
                    if (settings.platform.type != Platform::Type::Unspecified)
                        value.setKnown();
                    setTokenValue(tok, value, settings);
                    setTokenValue(tok->next(), std::move(value), settings);
                }
            }
        } else if (tok2->tokType() == Token::eString) {
            const size_t sz = Token::getStrSize(tok2, settings);
            if (sz > 0) {
                ValueFlow::Value value(sz);
                value.setKnown();
                setTokenValue(tok->next(), std::move(value), settings);
            }
        } else if (tok2->tokType() == Token::eChar) {
            nonneg int sz = 0;
            if (cpp && settings.standards.cpp >= Standards::CPP20 && tok2->isUtf8())
                sz = 1;
            else if (tok2->isUtf16())
                sz = 2;
            else if (tok2->isUtf32())
                sz = 4;
            else if (tok2->isLong())
                sz = settings.platform.sizeof_wchar_t;
            else if ((tok2->isCChar() && !cpp) || (tok2->isCMultiChar()))
                sz = settings.platform.sizeof_int;
            else
                sz = 1;

            if (sz > 0) {
                ValueFlow::Value value(sz);
                value.setKnown();
                setTokenValue(tok->next(), std::move(value), settings);
            }
        } else if (!tok2->type()) {
            const ValueType& vt = ValueType::parseDecl(tok2, settings);
            size_t sz = ValueFlow::getSizeOf(vt, settings);
            const Token* brac = tok2->astParent();
            while (Token::simpleMatch(brac, "[")) {
                const Token* num = brac->astOperand2();
                if (num && ((num->isNumber() && MathLib::isInt(num->str())) || num->tokType() == Token::eChar)) {
                    try {
                        const MathLib::biguint dim = MathLib::toBigUNumber(num->str());
                        sz *= dim;
                        brac = brac->astParent();
                        continue;
                    }
                    catch (const std::exception& /*e*/) {
                        // Bad integer literal
                    }
                }
                sz = 0;
                break;
            }
            if (sz > 0) {
                ValueFlow::Value value(sz);
                if (!tok2->isTemplateArg() && settings.platform.type != Platform::Type::Unspecified)
                    value.setKnown();
                setTokenValue(tok->next(), std::move(value), settings);
            }
        }
        // skip over enum
        tok = tok->linkAt(1);
    } else if (Token::Match(tok, "%name% [{(] [)}]") && (tok->isStandardType() ||
                                                         (tok->variable() && tok->variable()->nameToken() == tok &&
                                                          (tok->variable()->isPointer() || (tok->variable()->valueType() && tok->variable()->valueType()->isIntegral()))))) {
        ValueFlow::Value value(0);
        if (!tok->isTemplateArg())
            value.setKnown();
        setTokenValue(tok->next(), std::move(value), settings);
    } else if (Token::simpleMatch(tok, "= { } ;")) {
        const Token* lhs = tok->astOperand1();
        if (lhs && lhs->valueType() && (lhs->valueType()->isIntegral() || lhs->valueType()->pointer > 0)) {
            ValueFlow::Value value(0);
            value.setKnown();
            setTokenValue(tok->next(), std::move(value), settings);
        }
    }
    return tok->next();
}

static void valueFlowNumber(TokenList &tokenlist, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok;) {
        tok = valueFlowSetConstantValue(tok, settings, tokenlist.isCPP());
    }

    if (tokenlist.isCPP()) {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->isName() && !tok->varId() && Token::Match(tok, "false|true")) {
                ValueFlow::Value value(tok->str() == "true");
                if (!tok->isTemplateArg())
                    value.setKnown();
                setTokenValue(tok, std::move(value), settings);
            } else if (Token::Match(tok, "[(,] NULL [,)]")) {
                // NULL function parameters are not simplified in the
                // normal tokenlist
                ValueFlow::Value value(0);
                if (!tok->isTemplateArg())
                    value.setKnown();
                setTokenValue(tok->next(), std::move(value), settings);
            }
        }
    }
}

static void valueFlowString(TokenList &tokenlist, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (tok->tokType() == Token::eString) {
            ValueFlow::Value strvalue;
            strvalue.valueType = ValueFlow::Value::ValueType::TOK;
            strvalue.tokvalue = tok;
            strvalue.setKnown();
            setTokenValue(tok, std::move(strvalue), settings);
        }
    }
}

static void valueFlowArray(TokenList &tokenlist, const Settings &settings)
{
    std::map<nonneg int, const Token *> constantArrays;

    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (tok->varId() > 0) {
            // array
            const std::map<nonneg int, const Token *>::const_iterator it = constantArrays.find(tok->varId());
            if (it != constantArrays.end()) {
                ValueFlow::Value value;
                value.valueType = ValueFlow::Value::ValueType::TOK;
                value.tokvalue = it->second;
                value.setKnown();
                setTokenValue(tok, std::move(value), settings);
            }

            // const array decl
            else if (tok->variable() && tok->variable()->isArray() && tok->variable()->isConst() &&
                     tok->variable()->nameToken() == tok && Token::Match(tok, "%var% [ %num%| ] = {")) {
                Token* rhstok = tok->next()->link()->tokAt(2);
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
                setTokenValue(tok, std::move(value), settings);
            }
            continue;
        }

        if (Token::Match(tok, "const %type% %var% [ %num%| ] = {")) {
            Token *vartok = tok->tokAt(2);
            Token *rhstok = vartok->next()->link()->tokAt(2);
            constantArrays[vartok->varId()] = rhstok;
            tok = rhstok->link();
            continue;
        }

        if (Token::Match(tok, "const char %var% [ %num%| ] = %str% ;")) {
            Token *vartok = tok->tokAt(2);
            Token *strtok = vartok->next()->link()->tokAt(2);
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

static void valueFlowArrayBool(TokenList &tokenlist, const Settings &settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (tok->hasKnownIntValue())
            continue;
        const Variable *var = nullptr;
        bool known = false;
        const std::list<ValueFlow::Value>::const_iterator val =
            std::find_if(tok->values().cbegin(), tok->values().cend(), std::mem_fn(&ValueFlow::Value::isTokValue));
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
            setTokenValue(tok, std::move(value), settings);
        }
    }
}

static void valueFlowArrayElement(TokenList& tokenlist, const Settings& settings)
{
    for (Token* tok = tokenlist.front(); tok; tok = tok->next()) {
        if (tok->hasKnownIntValue())
            continue;
        const Token* indexTok = nullptr;
        const Token* arrayTok = nullptr;
        if (Token::simpleMatch(tok, "[") && tok->isBinaryOp()) {
            indexTok = tok->astOperand2();
            arrayTok = tok->astOperand1();
        } else if (Token::Match(tok->tokAt(-2), ". %name% (") && astIsContainer(tok->tokAt(-2)->astOperand1())) {
            arrayTok = tok->tokAt(-2)->astOperand1();
            const Library::Container* container = getLibraryContainer(arrayTok);
            if (!container || container->stdAssociativeLike)
                continue;
            const Library::Container::Yield yield = container->getYield(tok->strAt(-1));
            if (yield != Library::Container::Yield::AT_INDEX)
                continue;
            indexTok = tok->astOperand2();
        }

        if (!indexTok || !arrayTok)
            continue;

        for (const ValueFlow::Value& arrayValue : arrayTok->values()) {
            if (!arrayValue.isTokValue())
                continue;
            if (arrayValue.isImpossible())
                continue;
            for (const ValueFlow::Value& indexValue : indexTok->values()) {
                if (!indexValue.isIntValue())
                    continue;
                if (indexValue.isImpossible())
                    continue;
                if (!arrayValue.isKnown() && !indexValue.isKnown() && arrayValue.varId != 0 && indexValue.varId != 0 &&
                    !(arrayValue.varId == indexValue.varId && arrayValue.varvalue == indexValue.varvalue))
                    continue;

                ValueFlow::Value result(0);
                result.condition = arrayValue.condition ? arrayValue.condition : indexValue.condition;
                result.setInconclusive(arrayValue.isInconclusive() || indexValue.isInconclusive());
                result.varId = (arrayValue.varId != 0) ? arrayValue.varId : indexValue.varId;
                result.varvalue = (result.varId == arrayValue.varId) ? arrayValue.intvalue : indexValue.intvalue;
                if (arrayValue.valueKind == indexValue.valueKind)
                    result.valueKind = arrayValue.valueKind;

                result.errorPath.insert(result.errorPath.end(), arrayValue.errorPath.cbegin(), arrayValue.errorPath.cend());
                result.errorPath.insert(result.errorPath.end(), indexValue.errorPath.cbegin(), indexValue.errorPath.cend());

                const MathLib::bigint index = indexValue.intvalue;

                if (arrayValue.tokvalue->tokType() == Token::eString) {
                    const std::string s = arrayValue.tokvalue->strValue();
                    if (index == s.size()) {
                        result.intvalue = 0;
                        setTokenValue(tok, std::move(result), settings);
                    } else if (index >= 0 && index < s.size()) {
                        result.intvalue = s[index];
                        setTokenValue(tok, std::move(result), settings);
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
                    result.errorPath.insert(result.errorPath.end(), v.errorPath.cbegin(), v.errorPath.cend());
                    setTokenValue(tok, std::move(result), settings);
                }
            }
        }
    }
}

static void valueFlowPointerAlias(TokenList &tokenlist, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
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
        setTokenValue(tok, std::move(value), settings);
    }
}

static void valueFlowBitAnd(TokenList &tokenlist, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (tok->str() != "&")
            continue;

        if (tok->hasKnownValue())
            continue;

        if (!tok->astOperand1() || !tok->astOperand2())
            continue;

        MathLib::bigint number;
        if (MathLib::isInt(tok->astOperand1()->str()))
            number = MathLib::toBigNumber(tok->astOperand1()->str());
        else if (MathLib::isInt(tok->astOperand2()->str()))
            number = MathLib::toBigNumber(tok->astOperand2()->str());
        else
            continue;

        int bit = 0;
        while (bit <= (MathLib::bigint_bits - 2) && ((((MathLib::bigint)1) << bit) < number))
            ++bit;

        if ((((MathLib::bigint)1) << bit) == number) {
            setTokenValue(tok, ValueFlow::Value(0), settings);
            setTokenValue(tok, ValueFlow::Value(number), settings);
        }
    }
}

static void valueFlowSameExpressions(TokenList &tokenlist, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
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

        if (isSameExpression(false, tok->astOperand1(), tok->astOperand2(), settings.library, true, true, &val.errorPath)) {
            setTokenValue(tok, std::move(val), settings);
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
        const bool lhsHasKnownRange = getExpressionRange(expr->astOperand1(), &vals[0], &vals[1]);
        const bool rhsHasKnownRange = getExpressionRange(expr->astOperand2(), &vals[2], &vals[3]);
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
        const bool lhsHasKnownRange = getExpressionRange(expr->astOperand1(), &vals[0], &vals[1]);
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

static void valueFlowRightShift(TokenList &tokenList, const Settings& settings)
{
    for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
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
            lhsbits = settings.platform.int_bit;
        else if (tok->astOperand1()->valueType()->type == ValueType::Type::LONG)
            lhsbits = settings.platform.long_bit;
        else if (tok->astOperand1()->valueType()->type == ValueType::Type::LONGLONG)
            lhsbits = settings.platform.long_long_bit;
        else
            continue;
        if (rhsvalue >= lhsbits || rhsvalue >= MathLib::bigint_bits || (1ULL << rhsvalue) <= lhsmax)
            continue;

        ValueFlow::Value val(0);
        val.setKnown();
        setTokenValue(tok, std::move(val), settings);
    }
}

static void valueFlowGlobalConstVar(TokenList& tokenList, const Settings &settings)
{
    // Get variable values...
    std::map<const Variable*, ValueFlow::Value> vars;
    for (const Token* tok = tokenList.front(); tok; tok = tok->next()) {
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
    for (Token* tok = tokenList.front(); tok; tok = tok->next()) {
        if (!tok->variable())
            continue;
        const std::map<const Variable*, ValueFlow::Value>::const_iterator var = vars.find(tok->variable());
        if (var == vars.end())
            continue;
        setTokenValue(tok, var->second, settings);
    }
}

static void valueFlowGlobalStaticVar(TokenList &tokenList, const Settings &settings)
{
    // Get variable values...
    std::map<const Variable *, ValueFlow::Value> vars;
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
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
                else if (tokenList.isCPP() && Token::Match(tok->astParent()->tokAt(-2), "& %name% ="))
                    vars.erase(tok->variable());
            } else if (isLikelyStreamRead(tok->astParent())) {
                vars.erase(tok->variable());
            } else if (Token::Match(tok->astParent(), "[(,]"))
                vars.erase(tok->variable());
        }
    }

    // Set values..
    for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (!tok->variable())
            continue;
        const std::map<const Variable *, ValueFlow::Value>::const_iterator var = vars.find(tok->variable());
        if (var == vars.end())
            continue;
        setTokenValue(tok, var->second, settings);
    }
}

static void valueFlowForward(Token* startToken,
                             const Token* endToken,
                             const Token* exprTok,
                             std::list<ValueFlow::Value> values,
                             const bool constValue,
                             const bool subFunction,
                             TokenList& tokenlist,
                             ErrorLogger* errorLogger,
                             const Settings& settings);

static void valueFlowReverse(TokenList& tokenlist,
                             Token *tok,
                             const Token * const varToken,
                             ValueFlow::Value val,
                             ValueFlow::Value val2,
                             ErrorLogger* errorLogger,
                             const Settings& settings)
{
    const MathLib::bigint num        = val.intvalue;
    const Variable * const var        = varToken->variable();
    if (!var)
        return;

    const int varid      = varToken->varId();
    const Token * const startToken = var->nameToken();

    for (Token *tok2 = tok->previous(); ; tok2 = tok2->previous()) {
        if (!tok2 || tok2 == startToken ||
            (tok2->str() == "{" &&
             (tok2->scope()->type == Scope::ScopeType::eFunction || tok2->scope()->type == Scope::ScopeType::eLambda))) {
            break;
        }

        if (tok2->varId() == varid) {
            if (tok2->hasKnownValue())
                break;
            // bailout: assignment
            if (Token::Match(tok2->previous(), "!!* %name% =")) {
                Token* assignTok = const_cast<Token*>(tok2->next()->astOperand2());
                if (!assignTok->hasKnownValue()) {
                    setTokenValue(assignTok, val, settings);
                    const std::string info = "Assignment from '" + assignTok->expressionString() + "'";
                    val.errorPath.emplace_back(assignTok, info);
                    std::list<ValueFlow::Value> values = {val};
                    if (val2.condition) {
                        val2.errorPath.emplace_back(assignTok, info);
                        setTokenValue(assignTok, val2, settings);
                        values.push_back(val2);
                    }
                    const Token* startForwardToken = nextAfterAstRightmostLeaf(tok2->next());
                    const Token* endForwardToken = tok->scope() ? tok->scope()->bodyEnd : tok;
                    valueFlowForward(const_cast<Token*>(startForwardToken),
                                     endForwardToken,
                                     assignTok,
                                     values,
                                     false,
                                     false,
                                     tokenlist,
                                     errorLogger,
                                     settings);
                    // Only reverse analysis supported with variables
                    if (assignTok->varId() > 0)
                        valueFlowReverse(tokenlist, tok2->previous(), assignTok, val, val2, errorLogger, settings);
                }
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "assignment of " + tok2->str());
                break;
            }

            // increment/decrement
            int inc = 0;
            if (Token::Match(tok2->previous(), "[;{}] %name% ++|-- ;"))
                inc = (tok2->strAt(1)=="++") ? -1 : 1;
            else if (Token::Match(tok2->tokAt(-2), "[;{}] ++|-- %name% ;"))
                inc = (tok2->strAt(-1)=="++") ? -1 : 1;
            else if (Token::Match(tok2->previous(), "++|-- %name%") || Token::Match(tok2, "%name% ++|--")) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "increment/decrement of " + tok2->str());
                break;
            }
            if (inc != 0) {
                val.intvalue += inc;
                const std::string info(tok2->str() + " is " + std::string(inc==1 ? "decremented" : "incremented") + ", before this " + (inc==1?"decrement":"increment") + " the value is " + val.infoString());
                val.errorPath.emplace_back(tok2, info);
            }

            // compound assignment
            if (Token::Match(tok2->previous(), "[;{}] %var% %assign%") && tok2->next()->str() != "=") {
                const Token * const assignToken = tok2->next();
                const Token * const rhsToken = assignToken->astOperand2();
                if (!rhsToken || !rhsToken->hasKnownIntValue()) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "compound assignment, rhs value is not known");
                    break;
                }
                const MathLib::bigint rhsValue =  rhsToken->values().front().intvalue;
                if (assignToken->str() == "+=")
                    val.intvalue -= rhsValue;
                else if (assignToken->str() == "-=")
                    val.intvalue += rhsValue;
                else if (assignToken->str() == "*=" && rhsValue != 0)
                    val.intvalue /= rhsValue;
                else {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "compound assignment " + tok2->str());
                    break;
                }

                const std::string info("Compound assignment '" + assignToken->str() + "', before assignment value is " + val.infoString());
                val.errorPath.emplace_back(tok2, info);
            }

            // bailout: variable is used in rhs in assignment to itself
            if (bailoutSelfAssignment(tok2)) {
                if (settings.debugwarnings)
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
            if (isVariableChangedByFunctionCall(tok2, std::max(val.indirect, val2.indirect), &settings, &inconclusive)) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "possible assignment of " + tok2->str() + " by subfunction");
                break;
            }
            // Impossible values can't be inconclusive
            if (val.isImpossible() || val2.isImpossible())
                break;
            val.setInconclusive(inconclusive);
            val2.setInconclusive(inconclusive);

            // skip if variable is conditionally used in ?: expression
            if (const Token *parent = skipValueInConditionalExpression(tok2)) {
                if (settings.debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "no simplification of " + tok2->str() + " within " + (Token::Match(parent,"[?:]") ? "?:" : parent->str()) + " expression");
                continue;
            }

            // do-while condition, break in the loop body
            {
                const Token *parent = tok2->astParent();
                while (parent && !Token::simpleMatch(parent->previous(), "while ("))
                    parent = parent->astParent();
                if (parent && Token::simpleMatch(parent->tokAt(-2), "} while (") && Token::simpleMatch(parent->linkAt(-2)->previous(), "do {")) {
                    bool breakBailout = false;
                    for (const Token *iftok = parent->linkAt(-2); iftok != parent; iftok = iftok->next()) {
                        if (!Token::simpleMatch(iftok, "if ("))
                            continue;
                        if (!Token::simpleMatch(iftok->linkAt(1), ") { break"))
                            continue;
                        ProgramMemoryFast programMemory;
                        programMemory.setIntValue(varid, num);
                        if (conditionIsTrue(iftok->next()->astOperand2(), programMemory)) {
                            breakBailout = true;
                            break;
                        }
                    }
                    if (breakBailout) {
                        if (settings.debugwarnings)
                            bailout(tokenlist,
                                    errorLogger,
                                    tok2,
                                    "no simplification of " + tok2->str() + " in do-while condition since there is a break in the loop body");
                        break;
                    }
                }
            }

            setTokenValue(tok2, val, settings);
            if (val2.condition)
                setTokenValue(tok2,val2, settings);
            if (tok2 == var->nameToken())
                break;
        }

        // skip sizeof etc..
        if (tok2->str() == ")" && Token::Match(tok2->link()->previous(), "sizeof|typeof|typeid ("))
            tok2 = tok2->link();

        // goto label
        if (Token::Match(tok2, "[;{}] %name% :")) {
            if (settings.debugwarnings)
                bailout(tokenlist, errorLogger, tok2->next(), "variable " + var->name() + " stopping on goto label");
            break;
        }

        if (tok2->str() == "}") {
            const Token *vartok = Token::findmatch(tok2->link(), "%varid%", tok2, varid);
            while (Token::Match(vartok, "%name% = %num% ;") && !vartok->tokAt(2)->getValue(num))
                vartok = Token::findmatch(vartok->next(), "%varid%", tok2, varid);
            if (vartok) {
                if (settings.debugwarnings) {
                    std::string errmsg = "variable ";
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
                if (isVariableChanged(start, end, varid, var->isGlobal(), &settings, tokenlist.isCPP())) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " is assigned in loop. so valueflow analysis bailout when start of loop is reached.");
                    break;
                }
            }

            // Global variable : stop when leaving the function scope
            if (!var->isLocal()) {
                if (!Token::Match(tok2->previous(), ")|else|do {"))
                    break;
                if ((tok2->previous()->str() == ")") &&
                    !Token::Match(tok2->linkAt(-1)->previous(), "if|for|while ("))
                    break;
            }
        } else if (tok2->str() == ";") {
            const Token *parent = tok2->previous();
            while (parent && !Token::Match(parent, "return|break|continue|goto"))
                parent = parent->astParent();
            // reaching a break/continue/return
            if (parent) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " stopping on " + parent->str());
                break;
            }
        }

        if (Token::Match(tok2, "%name% (") && !Token::simpleMatch(tok2->linkAt(1), ") {")) {
            // bailout: global non-const variables
            if (!(var->isLocal() || var->isArgument()) && !var->isConst()) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok, "global variable " + var->name());
                return;
            }
        }
    }
}

static bool isConditionKnown(const Token* tok, bool then)
{
    const char* op = "||";
    if (then)
        op = "&&";
    const Token* parent = tok->astParent();
    while (parent && parent->str() == op)
        parent = parent->astParent();
    return (parent && parent->str() == "(");
}

static void valueFlowBeforeCondition(TokenList& tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings& settings)
{
    for (const Scope * scope : symboldatabase.functionScopes) {
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
            } else if (Token::Match(tok, "[!?]") && Token::Match(tok->astOperand1(), "%name%")) {
                vartok = tok->astOperand1();
                num = 0;
            } else {
                continue;
            }

            int varid = vartok->varId();
            const Variable * const var = vartok->variable();

            if (varid == 0U || !var)
                continue;

            if (tok->str() == "?" && tok->isExpandedMacro()) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok, "variable " + var->name() + ", condition is defined in macro");
                continue;
            }

            // bailout: for/while-condition, variable is changed in while loop
            for (const Token *tok2 = tok; tok2; tok2 = tok2->astParent()) {
                if (tok2->astParent() || tok2->str() != "(" || !Token::simpleMatch(tok2->link(), ") {"))
                    continue;

                // Variable changed in 3rd for-expression
                if (Token::simpleMatch(tok2->previous(), "for (")) {
                    if (tok2->astOperand2() && tok2->astOperand2()->astOperand2() && isVariableChanged(tok2->astOperand2()->astOperand2(), tok2->link(), varid, var->isGlobal(), &settings, tokenlist.isCPP())) {
                        varid = 0U;
                        if (settings.debugwarnings)
                            bailout(tokenlist, errorLogger, tok, "variable " + var->name() + " used in loop");
                    }
                }

                // Variable changed in loop code
                if (Token::Match(tok2->previous(), "for|while (")) {
                    const Token * const start = tok2->link()->next();
                    const Token * const end   = start->link();

                    if (isVariableChanged(start,end,varid,var->isGlobal(),&settings,tokenlist.isCPP())) {
                        varid = 0U;
                        if (settings.debugwarnings)
                            bailout(tokenlist, errorLogger, tok, "variable " + var->name() + " used in loop");
                    }
                }

                // if,macro => bailout
                else if (Token::simpleMatch(tok2->previous(), "if (") && tok2->previous()->isExpandedMacro()) {
                    varid = 0U;
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok, "variable " + var->name() + ", condition is defined in macro");
                }
            }
            if (varid == 0U)
                continue;

            // extra logic for unsigned variables 'i>=1' => possible value can also be 0
            if (Token::Match(tok, "<|>")) {
                if (num != 0)
                    continue;
                if (!var->typeStartToken()->isUnsigned())
                    continue;
            }
            ValueFlow::Value val(tok, num);
            val.varId = varid;
            ValueFlow::Value val2;
            if (num==1U && Token::Match(tok,"<=|>=")) {
                if (var->typeStartToken()->isUnsigned()) {
                    val2 = ValueFlow::Value(tok,0);
                    val2.varId = varid;
                }
            }
            Token* startTok = tok->astParent() ? tok->astParent() : tok->previous();
            valueFlowReverse(tokenlist, startTok, vartok, val, val2, errorLogger, settings);
        }
    }
}

static void removeValues(std::list<ValueFlow::Value> &values, const std::list<ValueFlow::Value> &valuesToRemove)
{
    for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end();) {
        const bool found = std::any_of(valuesToRemove.cbegin(), valuesToRemove.cend(),
                                       [=](const ValueFlow::Value &v2) {
            return it->intvalue == v2.intvalue;
        });
        if (found)
            values.erase(it++);
        else
            ++it;
    }
}

static void valueFlowAST(Token *tok, const Variable* var, const nonneg int varid, const ValueFlow::Value &value, const Settings& settings)
{
    if (!tok)
        return;
    if (tok->varId() == var->declarationId())
        setTokenValue(tok, value, settings);
    valueFlowAST(tok->astOperand1(), var, varid, value, settings);
    if (tok->str() == "&&" && tok->astOperand1() && tok->astOperand1()->getValue(0)) {
        ProgramMemoryFast pm;
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
        ProgramMemoryFast pm;
        pm.setValue(varid,value);
        if (conditionIsTrue(tok->astOperand1(), pm))
            return;
    }
    valueFlowAST(tok->astOperand2(), var, varid, value, settings);
}

/** if known variable is changed in loop body, change it to a possible value */
static bool handleKnownValuesInLoop(const Token                 *startToken,
                                    const Token                 *endToken,
                                    std::list<ValueFlow::Value> *values,
                                    nonneg int varid,
                                    bool globalvar,
                                    const Settings              *settings)
{
    const bool isChanged = isVariableChanged(startToken, endToken, varid, globalvar, settings, true);
    if (!isChanged)
        return false;
    lowerToPossible(*values);
    return isChanged;
}

static bool evalAssignment(ValueFlow::Value &lhsValue, const std::string &assign, const ValueFlow::Value &rhsValue)
{
    if (lhsValue.isIntValue()) {
        if (assign == "+=")
            lhsValue.intvalue += rhsValue.intvalue;
        else if (assign == "-=")
            lhsValue.intvalue -= rhsValue.intvalue;
        else if (assign == "*=")
            lhsValue.intvalue *= rhsValue.intvalue;
        else if (assign == "/=") {
            if (rhsValue.intvalue == 0)
                return false;
            else
                lhsValue.intvalue /= rhsValue.intvalue;
        } else if (assign == "%=") {
            if (rhsValue.intvalue == 0)
                return false;
            else
                lhsValue.intvalue %= rhsValue.intvalue;
        } else if (assign == "&=")
            lhsValue.intvalue &= rhsValue.intvalue;
        else if (assign == "|=")
            lhsValue.intvalue |= rhsValue.intvalue;
        else if (assign == "^=")
            lhsValue.intvalue ^= rhsValue.intvalue;
        else
            return false;
    } else if (lhsValue.isFloatValue()) {
        if (assign == "+=")
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
static bool isAliasOf(const Variable * var, const Token *tok, nonneg int varid, const std::list<ValueFlow::Value>& values)
{
    if (tok->varId() == varid)
        return false;
    if (tok->varId() == 0)
        return false;
    if (isAliasOf(tok, varid))
        return true;
    if (!var->isPointer())
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

static std::set<int> getIndirections(const std::list<ValueFlow::Value>& values)
{
    std::set<int> result;
    std::transform(values.begin(), values.end(), std::inserter(result, result.end()), [](const ValueFlow::Value& v) {
        return std::max(0, v.indirect);
    });
    return result;
}

static void valueFlowForwardExpression(Token* startToken,
                                       const Token* endToken,
                                       const Token* exprTok,
                                       const std::list<ValueFlow::Value>& values,
                                       const TokenList& tokenlist,
                                       const Settings& settings)
{
    (void)startToken;
    (void)endToken;
    (void)exprTok;
    (void)values;
    (void)tokenlist;
    (void)settings;
    // FIXME!!!
    /*
       FwdAnalysis fwdAnalysis(tokenlist.isCPP(), settings.library);
       for (const FwdAnalysis::KnownAndToken read : fwdAnalysis.valueFlow(exprTok, startToken, endToken)) {
        for (const ValueFlow::Value& value : values) {
            // Don't set inconclusive values
            if (value.isInconclusive())
                continue;
            ValueFlow::Value v = value;
            if (v.isImpossible()) {
                if (read.known)
                    continue;
            } else if (!read.known) {
                v.valueKind = ValueFlow::Value::ValueKind::Possible;
            }
            setTokenValue(const_cast<Token*>(read.token), v, settings);
        }
       }
     */
}

static bool valueFlowForwardVariable(Token* const startToken,
                                     const Token* const endToken,
                                     const Variable* const var,
                                     const nonneg int varid,
                                     std::list<ValueFlow::Value> values,
                                     const bool constValue,
                                     const bool subFunction,
                                     TokenList& tokenlist,
                                     ErrorLogger* const errorLogger,
                                     const Settings& settings)
{
    int indentlevel = 0;
    int number_of_if = 0;
    int varusagelevel = -1;
    bool returnStatement = false;  // current statement is a return, stop analysis at the ";"
    bool read = false;  // is variable value read?

    if (values.empty())
        return true;

    for (Token *tok2 = startToken; tok2 && tok2 != endToken; tok2 = tok2->next()) {
        if (values.empty())
            return true;
        if (indentlevel >= 0 && tok2->str() == "{")
            ++indentlevel;
        else if (indentlevel >= 0 && tok2->str() == "}") {
            --indentlevel;
            if (indentlevel <= 0 && isReturnScope(tok2, &settings.library) && Token::Match(tok2->link()->previous(), "else|) {")) {
                const Token *condition = tok2->link();
                const bool iselse = Token::simpleMatch(condition->tokAt(-2), "} else {");
                if (iselse)
                    condition = condition->linkAt(-2);
                if (condition && Token::simpleMatch(condition->previous(), ") {"))
                    condition = condition->linkAt(-1)->astOperand2();
                else
                    condition = nullptr;
                if (!condition) {
                    if (settings.debugwarnings)
                        bailout(
                            tokenlist,
                            errorLogger,
                            tok2,
                            "variable " + var->name() +
                            " valueFlowForwardVariable, bailing out since it's unknown if conditional return is executed");
                    return false;
                }

                bool bailoutflag = false;
                const Token * const start1 = iselse ? tok2->link()->linkAt(-2) : nullptr;
                for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end();) {
                    if (!iselse && conditionIsTrue(condition, getProgramMemoryFast(condition->astParent(), varid, *it))) {
                        bailoutflag = true;
                        break;
                    }
                    if (iselse && conditionIsFalse(condition, getProgramMemoryFast(condition->astParent(), varid, *it))) {
                        bailoutflag = true;
                        break;
                    }
                    if (iselse && it->isPossible() && isVariableChanged(start1, start1->link(), varid, var->isGlobal(), &settings, tokenlist.isCPP()))
                        values.erase(it++);
                    else
                        ++it;
                }
                if (bailoutflag) {
                    if (settings.debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                tok2,
                                "variable " + var->name() +
                                " valueFlowForwardVariable, conditional return is assumed to be executed");
                    return false;
                }

                if (values.empty())
                    return true;
            } else if (indentlevel <= 0 &&
                       Token::simpleMatch(tok2->link()->previous(), "else {") &&
                       !isReturnScope(tok2->link()->tokAt(-2), &settings.library) &&
                       isVariableChanged(tok2->link(), tok2, varid, var->isGlobal(), &settings, tokenlist.isCPP())) {
                lowerToPossible(values);
            }
        }

        // skip lambda functions
        // TODO: handle lambda functions
        if (tok2->str() == "[" && findLambdaEndToken(tok2)) {
            Token *lambdaEndToken = const_cast<Token *>(findLambdaEndToken(tok2));
            if (isVariableChanged(lambdaEndToken->link(), lambdaEndToken, varid, var->isGlobal(), &settings, tokenlist.isCPP()))
                return false;
            // Don't skip lambdas for lifetime values
            if (!std::all_of(values.begin(), values.end(), std::mem_fn(&ValueFlow::Value::isLifetimeValue))) {
                tok2 = lambdaEndToken;
                continue;
            }
        }

        if (Token::Match(tok2, "[;{}] %name% :") || tok2->str() == "case") {
            lowerToPossible(values);
            tok2 = tok2->tokAt(2);
            continue;
        }

        else if ((var->isGlobal() || tok2->str() == "asm") && Token::Match(tok2, "%name% (") && Token::Match(tok2->linkAt(1), ") !!{")) {
            return false;
        }

        // Skip sizeof etc
        else if (Token::Match(tok2, "sizeof|typeof|typeid ("))
            tok2 = tok2->linkAt(1);

        else if (Token::simpleMatch(tok2, "else {")) {
            // Should scope be skipped because variable value is checked?
            const Token *condition = tok2->linkAt(-1);
            condition = condition ? condition->linkAt(-1) : nullptr;
            condition = condition ? condition->astOperand2() : nullptr;

            const bool skipelse = std::any_of(values.cbegin(), values.cend(),
                                              [=](const ValueFlow::Value &v) {
                return conditionIsTrue(condition, getProgramMemoryFast(tok2, varid, v));
            });
            if (skipelse) {
                tok2 = tok2->linkAt(1);
                continue;
            }
        }

        else if (Token::simpleMatch(tok2, "do {")) {
            const Token *start = tok2->next();
            const Token *end   = start->link();
            if (Token::simpleMatch(end, "} while ("))
                end = end->linkAt(2);

            if (isVariableChanged(start, end, var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP())) {
                if (settings.debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "variable " + var->name() + " valueFlowForwardVariable, assignment in do-while");
                return false;
            }

            handleKnownValuesInLoop(start, end, &values, var->declarationId(), var->isGlobal(), &settings);
        }

        // conditional block of code that assigns variable..
        else if (!tok2->varId() && Token::Match(tok2, "%name% (") && Token::simpleMatch(tok2->linkAt(1), ") {")) {
            // is variable changed in condition?
            for (int i:getIndirections(values)) {
                Token* tokChanged = findVariableChanged(tok2->next(), tok2->next()->link(), i, var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP());
                if (tokChanged != nullptr) {
                    // Set the value before bailing
                    if (tokChanged->varId() == var->declarationId()) {
                        for (const ValueFlow::Value &v : values) {
                            if (!v.isNonValue())
                                continue;
                            setTokenValue(tokChanged, v, settings);
                        }
                    }
                    values.remove_if([&](const ValueFlow::Value& v) {
                        return v.indirect == i;
                    });
                }
            }
            if (values.empty()) {
                if (settings.debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "variable " + var->name() + " valueFlowForwardVariable, assignment in condition");
                return false;
            }

            // if known variable is changed in loop body, change it to a possible value..
            if (Token::Match(tok2, "for|while")) {
                if (handleKnownValuesInLoop(tok2, tok2->linkAt(1)->linkAt(1), &values, var->declarationId(), var->isGlobal(), &settings))
                    number_of_if++;
            }

            // Set values in condition
            for (Token* tok3 = tok2->tokAt(2); tok3 != tok2->next()->link(); tok3 = tok3->next()) {
                if (tok3->varId() == var->declarationId()) {
                    for (const ValueFlow::Value &v : values)
                        setTokenValue(tok3, v, settings);
                } else if (Token::Match(tok3, "%oror%|&&|?|;")) {
                    break;
                }
            }

            const Token * const condTok = tok2->next()->astOperand2();
            const bool condAlwaysTrue = (condTok && condTok->hasKnownIntValue() && condTok->values().front().intvalue != 0);
            const bool condAlwaysFalse = (condTok && condTok->hasKnownIntValue() && condTok->values().front().intvalue == 0);

            // Should scope be skipped because variable value is checked?
            std::list<ValueFlow::Value> truevalues;
            std::list<ValueFlow::Value> falsevalues;
            for (const ValueFlow::Value &v : values) {
                if (condAlwaysTrue) {
                    truevalues.push_back(v);
                    continue;
                }
                if (condAlwaysFalse) {
                    falsevalues.push_back(v);
                    continue;
                }
                // TODO: Compute program from tokvalue first
                ProgramMemoryFast programMemory = getProgramMemoryFast(tok2, varid, v);
                const bool isTrue = conditionIsTrue(condTok, programMemory);
                const bool isFalse = conditionIsFalse(condTok, programMemory);

                if (isTrue)
                    truevalues.push_back(v);
                if (isFalse)
                    falsevalues.push_back(v);

            }
            if (!truevalues.empty() || !falsevalues.empty()) {
                // '{'
                Token * const startToken1 = tok2->linkAt(1)->next();

                bool vfresult = valueFlowForwardVariable(startToken1->next(),
                                                         startToken1->link(),
                                                         var,
                                                         varid,
                                                         truevalues,
                                                         constValue,
                                                         subFunction,
                                                         tokenlist,
                                                         errorLogger,
                                                         settings);

                if (!condAlwaysFalse && isVariableChanged(startToken1, startToken1->link(), var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP())) {
                    removeValues(values, truevalues);
                    lowerToPossible(values);
                }

                // goto '}'
                tok2 = startToken1->link();

                if (isEscapeScope(startToken1, settings, true) || !vfresult) {
                    if (condAlwaysTrue)
                        return false;
                    removeValues(values, truevalues);
                }

                if (Token::simpleMatch(tok2, "} else {")) {
                    Token * const startTokenElse = tok2->tokAt(2);

                    vfresult = valueFlowForwardVariable(startTokenElse->next(),
                                                        startTokenElse->link(),
                                                        var,
                                                        varid,
                                                        falsevalues,
                                                        constValue,
                                                        subFunction,
                                                        tokenlist,
                                                        errorLogger,
                                                        settings);

                    if (!condAlwaysTrue && isVariableChanged(startTokenElse, startTokenElse->link(), var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP())) {
                        removeValues(values, falsevalues);
                        lowerToPossible(values);
                    }

                    // goto '}'
                    tok2 = startTokenElse->link();

                    if (isEscapeScope(startTokenElse, settings, true) || !vfresult) {
                        if (condAlwaysFalse)
                            return false;
                        removeValues(values, falsevalues);
                    }
                }
                if (values.empty())
                    return false;
                continue;
            }

            Token * const start = tok2->linkAt(1)->next();
            Token * const end   = start->link();
            const bool varusage = (indentlevel >= 0 && constValue && number_of_if == 0U) ?
                                  isVariableChanged(start,end,var->declarationId(),var->isGlobal(),&settings,tokenlist.isCPP()) :
                                  (nullptr != Token::findmatch(start, "%varid%", end, var->declarationId()));
            if (!read) {
                read = bool(nullptr != Token::findmatch(tok2, "%varid% !!=", end, var->declarationId()));
            }
            if (varusage) {
                varusagelevel = indentlevel;

                if (indentlevel < 0 && tok2->str() == "switch")
                    return false;

                // TODO: don't check noreturn scopes
                if (read && (number_of_if > 0U || Token::findmatch(tok2, "%varid%", start, var->declarationId()))) {
                    // Set values in condition
                    const Token * const condend = tok2->linkAt(1);
                    for (Token *condtok = tok2; condtok != condend; condtok = condtok->next()) {
                        if (condtok->varId() == var->declarationId()) {
                            for (const ValueFlow::Value &v : values)
                                setTokenValue(condtok, v, settings);
                        }
                        if (Token::Match(condtok, "%oror%|&&|?|;"))
                            break;
                    }
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " is assigned in conditional code");
                    return false;
                }

                if (var->isStatic()) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " bailout when conditional code that contains var is seen");
                    return false;
                }

                // Forward known values in the else branch
                if (Token::simpleMatch(end, "} else {")) {
                    std::list<ValueFlow::Value> knownValues;
                    std::copy_if(values.begin(), values.end(), std::back_inserter(knownValues), std::mem_fn(&ValueFlow::Value::isKnown));
                    valueFlowForwardVariable(end->tokAt(2),
                                             end->linkAt(2),
                                             var,
                                             varid,
                                             knownValues,
                                             constValue,
                                             subFunction,
                                             tokenlist,
                                             errorLogger,
                                             settings);
                }

                // Remove conditional values
                std::list<ValueFlow::Value>::iterator it;
                for (it = values.begin(); it != values.end();) {
                    if (it->condition || it->conditional || it->isImpossible())
                        values.erase(it++);
                    else {
                        it->changeKnownToPossible();
                        ++it;
                    }
                }
            }

            // stop after conditional return scopes that are executed
            if (isReturnScope(end, &settings.library)) {
                std::list<ValueFlow::Value>::iterator it;
                for (it = values.begin(); it != values.end();) {
                    if (conditionIsTrue(tok2->next()->astOperand2(), getProgramMemoryFast(tok2, varid, *it)))
                        values.erase(it++);
                    else
                        ++it;
                }
                if (values.empty())
                    return false;
            }

            // noreturn scopes..
            if ((number_of_if > 0 || Token::findmatch(tok2, "%varid%", start, var->declarationId())) &&
                (isEscapeScope(start, settings) ||
                 (Token::simpleMatch(end,"} else {") && isEscapeScope(end->tokAt(2), settings)))) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + ". noreturn conditional scope.");
                return false;
            }

            if (isVariableChanged(start, end, var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP())) {
                if ((!read || number_of_if == 0) &&
                    Token::simpleMatch(tok2, "if (") &&
                    !(Token::simpleMatch(end, "} else {") &&
                      isEscapeScope(end->tokAt(2), settings))) {
                    ++number_of_if;
                    tok2 = end;
                    if (number_of_if >= 2)
                        return false; // FIXME is this correct?
                } else {
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

                    bool bail = true;
                    if (loopCondition) {
                        const Token *tok3 = Token::findmatch(start, "%varid%", end, var->declarationId());
                        if (Token::Match(tok3, "%varid% =", var->declarationId()) &&
                            tok3->scope()->bodyEnd &&
                            Token::Match(tok3->scope()->bodyEnd->tokAt(-3), "[;}] break ;") &&
                            !Token::findmatch(tok3->next(), "%varid%", end, var->declarationId())) {
                            bail = false;
                            tok2 = end;
                        }
                    }

                    if (bail) {
                        if (settings.debugwarnings)
                            bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + " is assigned in conditional code");
                        return false;
                    }
                }
            }

            else if (Token::simpleMatch(tok2, "if (") && Token::findmatch(tok2, "%varid% !!=", tok2->linkAt(1), var->declarationId()))
                return false; // FIXME tok2 = end; ?
        }

        else if (Token::Match(tok2, "assert|ASSERT (") && Token::simpleMatch(tok2->linkAt(1), ") ;")) {
            const Token * const arg = tok2->next()->astOperand2();
            if (arg != nullptr && arg->str() != ",") {
                // Should scope be skipped because variable value is checked?
                values.clear();  // <- FIXME remove this line
                for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end();) {
                    if (conditionIsFalse(arg, getProgramMemoryFast(tok2, varid, *it)))
                        values.erase(it++);
                    else
                        ++it;
                }
            }
        }

        // TODO: Check for eFunction
        else if (tok2->str() == "}" && indentlevel <= 0 && tok2->scope() && tok2->scope()->type == Scope::eLambda) {
            return true;
        }

        else if (tok2->str() == "}" && indentlevel == varusagelevel) {
            ++number_of_if;

            // Set "conditional" flag for all values
            removeImpossible(values);
            std::list<ValueFlow::Value>::iterator it;
            for (it = values.begin(); it != values.end(); ++it) {
                it->conditional = true;
                it->changeKnownToPossible();
            }

            if (Token::simpleMatch(tok2,"} else {"))
                tok2 = tok2->linkAt(2);
        }

        else if (Token::Match(tok2, "break|continue|goto")) {
            const Scope *scope = tok2->scope();
            if (indentlevel > 0) {
                const Token *tok3 = tok2->tokAt(2);
                int indentlevel2 = indentlevel;
                while (indentlevel2 > 0 &&
                       tok3->str() == "}" &&
                       Token::Match(tok3->link()->previous(), "!!)")) {
                    indentlevel2--;
                    tok3 = tok3->next();
                    if (tok3 && tok3->str() == ";")
                        tok3 = tok3->next();
                }
                if (indentlevel2 > 0)
                    continue;
                scope = tok3->scope();
                indentlevel = 0;
            }
            if (tok2->str() == "break") {
                if (scope && scope->type == Scope::eSwitch) {
                    tok2 = const_cast<Token *>(scope->bodyEnd);
                    if (tok2 == endToken)
                        break;
                    --indentlevel;
                    lowerToPossible(values);
                    continue;
                }
            }
            if (settings.debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "variable " + var->name() + ". noreturn conditional scope.");
            return false;
        }

        else if (indentlevel <= 0 && Token::Match(tok2, "return|throw|setjmp|longjmp"))
            returnStatement = true;

        else if (returnStatement && tok2->str() == ";")
            return false;

        // If a ? is seen and it's known that the condition is true/false..
        else if (tok2->str() == "?") {
            if (subFunction && (astIsPointer(tok2->astOperand1()) || astIsIntegral(tok2->astOperand1(), false))) {
                tok2 = const_cast<Token*>(nextAfterAstRightmostLeaf(tok2));
                if (settings.debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "variable " + var->name() + " valueFlowForwardVariable, skip ternary in subfunctions");
                continue;
            }
            const Token *condition = tok2->astOperand1();
            Token *op2 = tok2->astOperand2();
            if (!condition || !op2) // Ticket #6713
                continue;

            if (condition->hasKnownIntValue()) {
                const ValueFlow::Value &condValue = condition->values().front();
                Token *expr2 = (condValue.intvalue != 0) ? op2->astOperand1() : op2->astOperand2();
                for (const ValueFlow::Value &v : values)
                    valueFlowAST(expr2, var, varid, v, settings);
                if (isVariableChangedByFunctionCall(expr2, 0, var->declarationId(), &settings, nullptr))
                    lowerToPossible(values, 0);
                if (isVariableChangedByFunctionCall(expr2, 1, var->declarationId(), &settings, nullptr))
                    lowerToPossible(values, 1);
            } else {
                if (number_of_if >= 1) {
                    // is variable used in conditional code? the value is not known
                    if (settings.debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                tok2,
                                "variable " + var->name() + " valueFlowForwardVariable, number_of_if");
                    return false;
                }

                for (const ValueFlow::Value &v : values) {
                    const ProgramMemoryFast programMemory(getProgramMemoryFast(tok2, varid, v));
                    if (conditionIsTrue(condition, programMemory))
                        valueFlowAST(op2->astOperand1(), var, varid, v, settings);
                    else if (conditionIsFalse(condition, programMemory))
                        valueFlowAST(op2->astOperand2(), var, varid, v, settings);
                    else
                        valueFlowAST(op2, var, varid, v, settings);
                }

                const Token * const expr0 = op2->astOperand1() ? op2->astOperand1() : tok2->astOperand1();
                const Token * const expr1 = op2->astOperand2();

                const std::pair<const Token *, const Token *> startEnd0 = expr0->findExpressionStartEndTokens();
                const std::pair<const Token *, const Token *> startEnd1 = expr1->findExpressionStartEndTokens();
                const bool changed0 = isVariableChanged(startEnd0.first, startEnd0.second->next(), var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP());
                const bool changed1 = isVariableChanged(startEnd1.first, startEnd1.second->next(), var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP());

                if (changed0 && changed1) {
                    if (settings.debugwarnings)
                        bailout(tokenlist,
                                errorLogger,
                                tok2,
                                "variable " + var->name() + " valueFlowForwardVariable, changed in both : expressions");
                    return false;
                }

                if (changed0 || changed1)
                    lowerToPossible(values);
            }

            // Skip conditional expressions..
            const Token * const questionToken = tok2;
            while (tok2->astOperand1() || tok2->astOperand2()) {
                if (tok2->astOperand2())
                    tok2 = tok2->astOperand2();
                else if (tok2->isUnaryPreOp())
                    tok2 = tok2->astOperand1();
                else
                    break;
            }
            tok2 = tok2->next();

            if (isVariableChanged(questionToken, questionToken->astOperand2(), var->declarationId(), false, &settings, tokenlist.isCPP()) &&
                isVariableChanged(questionToken->astOperand2(), tok2, var->declarationId(), false, &settings, tokenlist.isCPP())) {
                if (settings.debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "variable " + var->name() + " valueFlowForwardVariable, assignment in condition");
                return false;

            }
        }

        else if (tok2->varId() == var->declarationId()) {
            // compound assignment, known value in rhs
            if (Token::Match(tok2->previous(), "!!* %name% %assign%") &&
                tok2->next()->str() != "=" &&
                tok2->next()->astOperand2() &&
                tok2->next()->astOperand2()->hasKnownIntValue()) {

                const ValueFlow::Value &rhsValue = tok2->next()->astOperand2()->values().front();
                const std::string &assign = tok2->next()->str();
                std::list<ValueFlow::Value>::iterator it;
                // Erase values that are not int values..
                for (it = values.begin(); it != values.end();) {
                    if (!evalAssignment(*it, assign, rhsValue)) {
                        it = values.erase(it);
                    } else {
                        const std::string info("Compound assignment '" + assign + "', assigned value is " + it->infoString());
                        it->errorPath.emplace_back(tok2, info);

                        ++it;
                    }

                }
                if (values.empty()) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "compound assignment of " + tok2->str());
                    return false;
                }
            }

            // bailout: assignment
            else if (Token::Match(tok2->previous(), "!!* %name% %assign%")) {
                // simplify rhs
                std::stack<Token *> rhs;
                rhs.push(tok2->next()->astOperand2());
                while (!rhs.empty()) {
                    Token *rtok = rhs.top();
                    rhs.pop();
                    if (!rtok)
                        continue;
                    if (rtok->str() == "(" && Token::Match(rtok->astOperand1(), "sizeof|typeof|typeid"))
                        continue;
                    if (Token::Match(rtok, "++|--|?|:|;|,"))
                        continue;
                    if (rtok->varId() == var->declarationId()) {
                        for (const ValueFlow::Value &v : values)
                            setTokenValue(rtok, v, settings);
                    }
                    rhs.push(rtok->astOperand1());
                    rhs.push(rtok->astOperand2());
                }
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "assignment of " + tok2->str());
                return false;
            }

            // bailout: possible assignment using >>
            if (isLikelyStreamRead(tok2->previous())) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "Possible assignment of " + tok2->str() + " using " + tok2->strAt(-1));
                return false;
            }

            // skip if variable is conditionally used in ?: expression
            if (const Token *parent = skipValueInConditionalExpression(tok2)) {
                if (settings.debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "no simplification of " + tok2->str() + " within " + (Token::Match(parent,"[?:]") ? "?:" : parent->str()) + " expression");
                const Token * const astTop = parent->astTop();
                if (Token::simpleMatch(astTop->astOperand1(), "for ("))
                    tok2 = const_cast<Token*>(astTop->link());

                // bailout if address of var is taken..
                if (tok2->astParent() && tok2->astParent()->isUnaryOp("&")) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "Taking address of " + tok2->str());
                    return false;
                }

                continue;
            }

            {
                // Is variable usage protected by && || ?:
                const Token *tok3 = tok2;
                const Token *parent = tok3->astParent();
                while (parent && !Token::Match(parent, "%oror%|&&|:")) {
                    tok3 = parent;
                    parent = parent->astParent();
                }
                const bool conditional = parent && (parent->str() == ":" || parent->astOperand2() == tok3);

                for (const ValueFlow::Value &v : values) {
                    if (!conditional || !v.conditional)
                        setTokenValue(tok2, v, settings);
                }
            }

            // increment/decrement
            if (Token::Match(tok2->previous(), "++|-- %name%") || Token::Match(tok2, "%name% ++|--")) {
                std::list<ValueFlow::Value>::iterator it;
                // Erase values that are not int values..
                for (it = values.begin(); it != values.end();) {
                    if (!it->isIntValue())
                        it = values.erase(it);
                    else
                        ++it;
                }
                if (values.empty()) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "increment/decrement of " + tok2->str());
                    return false;
                }
                const bool pre   = Token::Match(tok2->previous(), "++|--");
                Token * const op = pre ? tok2->previous() : tok2->next();
                const bool inc   = (op->str() == "++");
                // Perform increment/decrement..
                for (it = values.begin(); it != values.end(); ++it) {
                    if (!pre)
                        setTokenValue(op, *it, settings);
                    it->intvalue += (inc ? 1 : -1);
                    if (pre)
                        setTokenValue(op, *it, settings);
                    const std::string info(tok2->str() + " is " + std::string(inc ? "incremented" : "decremented") + "', new value is " + it->infoString());
                    it->errorPath.emplace_back(tok2, info);
                }
            }

            // bailout if address of var is taken..
            if (tok2->astParent() && tok2->astParent()->isUnaryOp("&")) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "Taking address of " + tok2->str());
                return false;
            }

            // bailout if reference is created..
            if (tok2->astParent() && Token::Match(tok2->astParent()->tokAt(-2), "& %name% =")) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "Reference of " + tok2->str());
                return false;
            }

            // bailout if its stream..
            if (isLikelyStream(tok2)) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "Stream used: " + tok2->str());
                return false;
            }

            // assigned by subfunction?
            for (int i:getIndirections(values)) {
                bool inconclusive = false;
                if (isVariableChangedByFunctionCall(tok2, i, &settings, &inconclusive)) {
                    values.remove_if([&](const ValueFlow::Value& v) {
                        return v.indirect <= i;
                    });
                }
                if (inconclusive)
                    lowerToInconclusive(values, settings, i);
            }
            if (values.empty()) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "possible assignment of " + tok2->str() + " by subfunction");
                return false;
            }
            if (tok2->strAt(1) == "." && tok2->next()->originalName() != "->") {
                lowerToInconclusive(values, settings);
                if (!settings.certainty.isEnabled(Certainty::inconclusive)) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "possible assignment of " + tok2->str() + " by member function");
                    return false;
                }
            }
            // Variable changed
            for (int i:getIndirections(values)) {
                // Remove uninitialized values if modified
                if (isVariableChanged(tok2, i, &settings, tokenlist.isCPP()))
                    values.remove_if([&](const ValueFlow::Value& v) {
                        return v.isUninitValue() && v.indirect <= i;
                    });
            }
        } else if (isAliasOf(var, tok2, var->declarationId(), values) && isVariableChanged(tok2, 0, &settings, tokenlist.isCPP())) {
            if (settings.debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "Alias variable was modified.");
            // Bail at the end of the statement if its in an assignment
            const Token * top = tok2->astTop();
            if (Token::Match(top, "%assign%") && astHasToken(top->astOperand1(), tok2))
                returnStatement = true;
            else
                return false;
        }

        // Lambda function
        if (Token::simpleMatch(tok2, "= [") &&
            Token::simpleMatch(tok2->linkAt(1), "] (") &&
            Token::simpleMatch(tok2->linkAt(1)->linkAt(1), ") {")) {
            const Token *bodyStart = tok2->linkAt(1)->linkAt(1)->next();
            if (isVariableChanged(bodyStart, bodyStart->link(), var->declarationId(), var->isGlobal(), &settings, tokenlist.isCPP())) {
                if (settings.debugwarnings)
                    bailout(tokenlist,
                            errorLogger,
                            tok2,
                            "valueFlowForwardVariable, " + var->name() + " is changed in lambda function");
                return false;
            }
        }

    }
    return true;
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

template<class F>
void transformIntValues(std::list<ValueFlow::Value>& values, F f)
{
    std::transform(values.begin(), values.end(), values.begin(), [&](ValueFlow::Value x) {
        if (x.isIntValue())
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

static void valueFlowForward(Token* startToken,
                             const Token* endToken,
                             const Token* exprTok,
                             std::list<ValueFlow::Value> values,
                             const bool constValue,
                             const bool subFunction,
                             TokenList& tokenlist,
                             ErrorLogger* const errorLogger,
                             const Settings& settings)
{
    const Token* expr = solveExprValues(exprTok, values);
    if (Token::Match(expr, "%var%") && expr->variable()) {
        valueFlowForwardVariable(startToken,
                                 endToken,
                                 expr->variable(),
                                 expr->varId(),
                                 values,
                                 constValue,
                                 subFunction,
                                 tokenlist,
                                 errorLogger,
                                 settings);
    } else {
        valueFlowForwardExpression(startToken, endToken, expr, values, tokenlist, settings);
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
        Token * rightAngle = const_cast<Token*>(leftAngle->link());
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

static void valueFlowAfterMove(TokenList& tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings& settings)
{
    if (!tokenlist.isCPP() || settings.standards.cpp < Standards::CPP11)
        return;
    for (const Scope * scope : symboldatabase.functionScopes) {
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
                const Token * const endOfVarScope = var->typeStartToken()->scope()->bodyEnd;
                setTokenValue(varTok, value, settings);
                valueFlowForwardVariable(
                    varTok->next(), endOfVarScope, var, varTok->varId(), values, false, false, tokenlist, errorLogger, settings);
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
            const Token * const endOfVarScope = var->typeStartToken()->scope()->bodyEnd;

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
                                         varTok->varId(),
                                         values,
                                         false,
                                         false,
                                         tokenlist,
                                         errorLogger,
                                         settings);
        }
    }
}

static void valueFlowForwardAssign(Token * const tok,
                                   const Variable * const var,
                                   std::list<ValueFlow::Value> values,
                                   const bool constValue,
                                   const bool init,
                                   TokenList&                  tokenlist,
                                   ErrorLogger * const errorLogger,
                                   const Settings&             settings)
{
    const Token * const endOfVarScope = var->typeStartToken()->scope()->bodyEnd;
    if (std::any_of(values.begin(), values.end(), std::mem_fn(&ValueFlow::Value::isLifetimeValue))) {
        //valueFlowForwardLifetime(tok, tokenlist, errorLogger, settings);
        values.remove_if(std::mem_fn(&ValueFlow::Value::isLifetimeValue));
    }
    if (!var->isPointer() && !var->isSmartPointer())
        values.remove_if(std::mem_fn(&ValueFlow::Value::isTokValue));
    if (tok->astParent()) {
        for (std::list<ValueFlow::Value>::iterator it = values.begin(); it != values.end(); ++it) {
            const std::string info = "Assignment '" + tok->astParent()->expressionString() + "', assigned value is " + it->infoString();
            it->errorPath.emplace_back(tok, info);
        }
    }

    if (tokenlist.isCPP() && Token::Match(var->typeStartToken(), "bool|_Bool")) {
        std::list<ValueFlow::Value>::iterator it;
        for (it = values.begin(); it != values.end(); ++it) {
            if (it->isIntValue())
                it->intvalue = (it->intvalue != 0);
            if (it->isTokValue())
                it->intvalue = (it->tokvalue != nullptr);
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

static std::list<ValueFlow::Value> truncateValues(std::list<ValueFlow::Value> values, const ValueType *valueType, const Settings&settings)
{
    if (!valueType || !valueType->isIntegral())
        return values;

    const size_t sz = ValueFlow::getSizeOf(*valueType, settings);

    for (ValueFlow::Value &value : values) {
        if (value.isFloatValue()) {
            value.intvalue = value.floatValue;
            value.valueType = ValueFlow::Value::ValueType::INT;
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
    return tok->isNumber() || tok->str() == "NULL" || (cpp && Token::Match(tok, "false|true|nullptr"));
}

static void valueFlowAfterAssign(TokenList&tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings& settings)
{
    for (const Scope * scope : symboldatabase.functionScopes) {
        std::set<int> aliased;
        for (Token* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
            // Alias
            if (tok->isUnaryOp("&")) {
                aliased.insert(tok->astOperand1()->varId());
                continue;
            }

            // Assignment
            if ((tok->str() != "=") || (tok->astParent()))
                continue;

            // Lhs should be a variable
            if (!tok->astOperand1() || !tok->astOperand1()->varId() || tok->astOperand1()->hasKnownValue())
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
            const bool constValue = isLiteralNumber(tok->astOperand2(), tokenlist.isCPP());
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

static void insertImpossible(std::list<ValueFlow::Value>& values, const std::list<ValueFlow::Value>& input)
{
    std::transform(input.begin(), input.end(), std::back_inserter(values), &ValueFlow::asImpossible);
}

static std::vector<const Variable*> getExprVariables(const Token* expr,
                                                     const SymbolDatabase& symboldatabase,
                                                     const Settings& settings)
{
    std::vector<const Variable*> result;
    FwdAnalysis fwdAnalysis(settings.library);
    std::set<int> varids = fwdAnalysis.getExprVarIds(expr);
    std::transform(varids.begin(), varids.end(), std::back_inserter(result), [&](int id) {
        return symboldatabase.getVariableFromVarId(id);
    });
    return result;
}

namespace {
    struct ValueFlowConditionHandler {
        struct Condition {
            const Token *vartok;
            std::list<ValueFlow::Value> true_values;
            std::list<ValueFlow::Value> false_values;

            Condition() : vartok(nullptr), true_values(), false_values() {}
        };
        std::function<bool(Token* start, const Token* stop, const Token* exprTok, const std::list<ValueFlow::Value>& values, bool constValue)>
        forward;
        std::function<Condition(Token *tok)> parse;

        void afterCondition(TokenList& tokenlist,
                            const SymbolDatabase& symboldatabase,
                            ErrorLogger *errorLogger,
                            const Settings& settings) const {
            for (const Scope *scope : symboldatabase.functionScopes) {
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

                    Condition cond = parse(tok);
                    if (!cond.vartok)
                        continue;
                    if (cond.true_values.empty() || cond.false_values.empty())
                        continue;

                    if (exprDependsOnThis(cond.vartok))
                        continue;
                    std::vector<const Variable*> vars = getExprVariables(cond.vartok, symboldatabase, settings);
                    if (std::any_of(vars.begin(), vars.end(), [](const Variable* var) {
                        return !var;
                    }))
                        continue;
                    if (!vars.empty() && (vars.front()))
                        if (std::any_of(vars.begin(), vars.end(), [&](const Variable* var) {
                            return var && aliased.find(var->declarationId()) != aliased.end();
                        })) {
                            if (settings.debugwarnings)
                                bailout(tokenlist,
                                        errorLogger,
                                        cond.vartok,
                                        "variable is aliased so we just skip all valueflow after condition");
                            continue;
                        }

                    if (Token::Match(tok->astParent(), "%oror%|&&")) {
                        Token *parent = tok->astParent();
                        const std::string &op(parent->str());

                        if (parent->astOperand1() == tok && ((op == "&&" && Token::Match(tok, "==|>=|<=|!")) ||
                                                             (op == "||" && Token::Match(tok, "%name%|!=")))) {
                            for (; parent && parent->str() == op; parent = parent->astParent()) {
                                std::stack<Token *> tokens;
                                tokens.push(parent->astOperand2());
                                bool assign = false;
                                while (!tokens.empty()) {
                                    Token *rhstok = tokens.top();
                                    tokens.pop();
                                    if (!rhstok)
                                        continue;
                                    tokens.push(rhstok->astOperand1());
                                    tokens.push(rhstok->astOperand2());
                                    if (isSameExpression(false, cond.vartok, rhstok, settings.library, true, false))
                                        setTokenValue(rhstok, cond.true_values.front(), settings);
                                    else if (Token::Match(rhstok, "++|--|=") && isSameExpression(false,
                                                                                                 cond.vartok,
                                                                                                 rhstok->astOperand1(),
                                                                                                 settings.library,
                                                                                                 true,
                                                                                                 false)) {
                                        assign = true;
                                        break;
                                    }
                                }
                                if (assign)
                                    break;
                                while (parent->astParent() && parent == parent->astParent()->astOperand2())
                                    parent = parent->astParent();
                            }
                        }
                    }

                    if (top && Token::Match(top->previous(), "if|while (") && !top->previous()->isExpandedMacro()) {
                        // does condition reassign variable?
                        if (tok != top->astOperand2() && Token::Match(top->astOperand2(), "%oror%|&&") &&
                            isVariablesChanged(top, top->link(), 0, vars, &settings)) {
                            if (settings.debugwarnings)
                                bailout(tokenlist, errorLogger, tok, "assignment in condition");
                            continue;
                        }

                        std::list<ValueFlow::Value> thenValues;
                        std::list<ValueFlow::Value> elseValues;

                        if (!Token::Match(tok, "!=|=") && tok != cond.vartok) {
                            thenValues.insert(thenValues.end(), cond.true_values.begin(), cond.true_values.end());
                            if (isConditionKnown(tok, false))
                                insertImpossible(elseValues, cond.false_values);
                        }
                        if (!Token::Match(tok, "==|!")) {
                            elseValues.insert(elseValues.end(), cond.false_values.begin(), cond.false_values.end());
                            if (isConditionKnown(tok, true))
                                insertImpossible(thenValues, cond.true_values);
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
                            startTokens[0] = const_cast<Token*>(top->link()->next());
                        if (Token::simpleMatch(top->link()->linkAt(1), "} else {"))
                            startTokens[1] = const_cast<Token*>(top->link()->linkAt(1)->tokAt(2));

                        int changeBlock = -1;

                        for (int i = 0; i < 2; i++) {
                            const Token *const startToken = startTokens[i];
                            if (!startToken)
                                continue;
                            std::list<ValueFlow::Value>& values = (i == 0 ? thenValues : elseValues);
                            valueFlowSetConditionToKnown(tok, values, i == 0);

                            // TODO: The endToken should not be startTokens[i]->link() in the valueFlowForwardVariable call
                            if (forward(startTokens[i], startTokens[i]->link(), cond.vartok, values, true))
                                changeBlock = i;
                            changeKnownToPossible(values);
                        }
                        // TODO: Values changed in noreturn blocks should not bail
                        if (changeBlock >= 0 && !Token::simpleMatch(top->previous(), "while (")) {
                            if (settings.debugwarnings)
                                bailout(tokenlist,
                                        errorLogger,
                                        startTokens[changeBlock]->link(),
                                        "valueFlowAfterCondition: " + cond.vartok->expressionString() +
                                        " is changed in conditional block");
                            continue;
                        }

                        // After conditional code..
                        if (Token::simpleMatch(top->link(), ") {")) {
                            Token *after = const_cast<Token*>(top->link()->linkAt(1));
                            std::string unknownFunction;
                            if (settings.library.isScopeNoReturn(after, &unknownFunction)) {
                                if (settings.debugwarnings && !unknownFunction.empty())
                                    bailout(tokenlist, errorLogger, after, "possible noreturn scope");
                                continue;
                            }

                            bool dead_if = isReturnScope(after, &settings.library) ||
                                           (tok->astParent() && Token::simpleMatch(tok->astParent()->previous(), "while (") &&
                                            !isBreakScope(after));
                            bool dead_else = false;

                            if (Token::simpleMatch(after, "} else {")) {
                                after = after->linkAt(2);
                                if (Token::simpleMatch(after->tokAt(-2), ") ; }")) {
                                    if (settings.debugwarnings)
                                        bailout(tokenlist, errorLogger, after, "possible noreturn scope");
                                    continue;
                                }
                                dead_else = isReturnScope(after, &settings.library);
                            }

                            if (dead_if && dead_else)
                                continue;

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
                                // TODO: constValue could be true if there are no assignments in the conditional blocks and
                                //       perhaps if there are no && and no || in the condition
                                bool constValue = false;
                                forward(after, top->scope()->bodyEnd, cond.vartok, values, constValue);
                            }
                        }
                    }
                }
            }
        }
    };
}

static void valueFlowAfterCondition(TokenList& tokenlist,
                                    const SymbolDatabase& symboldatabase,
                                    ErrorLogger* errorLogger,
                                    const Settings& settings)
{
    ValueFlowConditionHandler handler;
    handler.forward = [&](Token* start,
                          const Token* stop,
                          const Token* vartok,
                          const std::list<ValueFlow::Value>& values,
                          bool constValue) {
        valueFlowForward(start->next(), stop, vartok, values, constValue, false, tokenlist, errorLogger, settings);
        std::vector<const Variable*> vars = getExprVariables(vartok, symboldatabase, settings);
        return isVariablesChanged(start, stop, 0, vars, &settings);
    };
    handler.parse = [&](const Token *tok) {
        ValueFlowConditionHandler::Condition cond;
        ValueFlow::Value true_value;
        ValueFlow::Value false_value;
        const Token *vartok = ValueFlow::parseCompareInt(tok, true_value, false_value);
        if (vartok) {
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
    };
    handler.afterCondition(tokenlist, symboldatabase, errorLogger, settings);
}

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

static const ValueFlow::Value* proveNotEqual(const std::list<ValueFlow::Value>& values, MathLib::bigint x)
{
    const ValueFlow::Value* result = nullptr;
    for (const ValueFlow::Value& value : values) {
        if (value.valueType != ValueFlow::Value::ValueType::INT)
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

static void valueFlowInferCondition(TokenList& tokenlist,
                                    const Settings& settings)
{
    for (Token* tok = tokenlist.front(); tok; tok = tok->next()) {
        if (!tok->astParent())
            continue;
        if (tok->hasKnownValue())
            continue;
        if (Token::Match(tok, "%var%") && (Token::Match(tok->astParent(), "?|&&|!|%oror%") ||
                                           Token::Match(tok->astParent()->previous(), "if|while ("))) {
            const ValueFlow::Value* result = proveNotEqual(tok->values(), 0);
            if (!result)
                continue;
            ValueFlow::Value value = *result;
            value.intvalue = 1;
            value.setKnown();
            setTokenValue(tok, value, settings);
        } else if (Token::Match(tok, "==|!=")) {
            MathLib::bigint val = 0;
            const Token* varTok = nullptr;
            if (tok->astOperand1()->hasKnownIntValue()) {
                val = tok->astOperand1()->values().front().intvalue;
                varTok = tok->astOperand2();
            } else if (tok->astOperand2()->hasKnownIntValue()) {
                val = tok->astOperand2()->values().front().intvalue;
                varTok = tok->astOperand1();
            }
            if (!varTok)
                continue;
            if (varTok->hasKnownIntValue())
                continue;
            const ValueFlow::Value* result = proveNotEqual(varTok->values(), val);
            if (!result)
                continue;
            ValueFlow::Value value = *result;
            value.intvalue = tok->str() == "!=";
            value.setKnown();
            setTokenValue(tok, value, settings);
        }
    }
}

// FIXME
/*

   static bool valueFlowForLoop1(const Token *tok, int * const varid, MathLib::bigint * const num1, MathLib::bigint * const num2, MathLib::bigint * const numAfter)
   {
    tok = tok->tokAt(2);
    if (!Token::Match(tok, "%type%| %var% ="))
        return false;
    const Token * const vartok = Token::Match(tok, "%var% =") ? tok : tok->next();
 * varid = vartok->varId();
    tok = vartok->tokAt(2);
    const Token * const num1tok = Token::Match(tok, "%num% ;") ? tok : nullptr;
    if (num1tok)
 * num1 = num1tok->getKnownIntValue();
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
            num2tok = nullptr;
    }
    if (!num2tok)
        return false;
 * num2 = num2tok->getKnownIntValue() - ((tok->str()=="<=") ? 0 : 1);
 * numAfter = *num2 + 1;
    if (!num1tok)
 * num1 = *num2;
    while (tok && tok->str() != ";")
        tok = tok->next();
    if (!Token::Match(tok, "; %varid% ++ ) {", vartok->varId()) && !Token::Match(tok, "; ++ %varid% ) {", vartok->varId()))
        return false;
    return true;
   }

   static bool valueFlowForLoop2(const Token *tok,
                              ProgramMemoryFast *memory1,
                              ProgramMemoryFast *memory2,
                              ProgramMemoryFast *memoryAfter)
   {
    // for ( firstExpression ; secondExpression ; thirdExpression )
    const Token *firstExpression  = tok->next()->astOperand2()->astOperand1();
    const Token *secondExpression = tok->next()->astOperand2()->astOperand2()->astOperand1();
    const Token *thirdExpression = tok->next()->astOperand2()->astOperand2()->astOperand2();

    ProgramMemoryFast programMemory;
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

    ProgramMemoryFast startMemory(programMemory);
    ProgramMemoryFast endMemory;

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

   static void valueFlowForLoopSimplify(Token * const bodyStart, const nonneg int varid, bool globalvar, const MathLib::bigint value, TokenList&tokenlist, ErrorLogger *errorLogger, const Settings&settings)
   {
    const Token * const bodyEnd = bodyStart->link();

    // Is variable modified inside for loop
    if (isVariableChanged(bodyStart, bodyEnd, varid, globalvar, settings, tokenlist.isCPP()))
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
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop variable " + tok2->str() + " stopping on ?");
                continue;
            }

            ValueFlow::Value value1(value);
            value1.varId = tok2->varId();
            setTokenValue(tok2, value1, settings);
        }

        if (Token::Match(tok2, "%oror%|&&")) {
            const ProgramMemoryFast programMemory(getProgramMemoryFast(tok2->astTop(), varid, ValueFlow::Value(value)));
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
        if ((tok2->str() == "&&" && conditionIsFalse(tok2->astOperand1(), getProgramMemoryFast(tok2->astTop(), varid, ValueFlow::Value(value)))) ||
            (tok2->str() == "||" && conditionIsTrue(tok2->astOperand1(), getProgramMemoryFast(tok2->astTop(), varid, ValueFlow::Value(value)))))
            break;

        else if (Token::simpleMatch(tok2, ") {") && Token::findmatch(tok2->link(), "%varid%", tok2, varid)) {
            if (Token::findmatch(tok2, "continue|break|return", tok2->linkAt(1), varid)) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, tok2, "For loop variable bailout on conditional continue|break|return");
                break;
            }
            if (settings.debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "For loop variable skipping conditional scope");
            tok2 = tok2->next()->link();
            if (Token::simpleMatch(tok2, "} else {")) {
                if (Token::findmatch(tok2, "continue|break|return", tok2->linkAt(2), varid)) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok2, "For loop variable bailout on conditional continue|break|return");
                    break;
                }

                tok2 = tok2->linkAt(2);
            }
        }

        else if (Token::simpleMatch(tok2, ") {")) {
            if (settings.debugwarnings)
                bailout(tokenlist, errorLogger, tok2, "For loop skipping {} code");
            tok2 = tok2->linkAt(1);
            if (Token::simpleMatch(tok2, "} else {"))
                tok2 = tok2->linkAt(2);
        }
    }
   }

   static void valueFlowForLoopSimplifyAfter(Token *fortok, nonneg int varid, const MathLib::bigint num, TokenList&tokenlist, ErrorLogger *errorLogger, const Settings&settings)
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
        endToken = var->typeStartToken()->scope()->bodyEnd;
    else
        endToken = fortok->scope()->bodyEnd;

    std::list<ValueFlow::Value> values;
    values.emplace_back(num);
    values.back().errorPath.emplace_back(fortok,"After for loop, " + var->name() + " has value " + values.back().infoString());

    valueFlowForwardVariable(
        fortok->linkAt(1)->linkAt(1)->next(), endToken, var, varid, values, false, false, tokenlist, errorLogger, settings);
   }

   static void valueFlowForLoop(TokenList&tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings&settings)
   {
    for (const Scope &scope : symboldatabase->scopeList) {
        if (scope.type != Scope::eFor)
            continue;

        Token* tok = const_cast<Token*>(scope.classDef);
        Token* const bodyStart = const_cast<Token*>(scope.bodyStart);

        if (!Token::simpleMatch(tok->next()->astOperand2(), ";") ||
            !Token::simpleMatch(tok->next()->astOperand2()->astOperand2(), ";"))
            continue;

        int varid(0);
        MathLib::bigint num1(0), num2(0), numAfter(0);

        if (valueFlowForLoop1(tok, &varid, &num1, &num2, &numAfter)) {
            if (num1 <= num2) {
                valueFlowForLoopSimplify(bodyStart, varid, false, num1, tokenlist, errorLogger, settings);
                valueFlowForLoopSimplify(bodyStart, varid, false, num2, tokenlist, errorLogger, settings);
                valueFlowForLoopSimplifyAfter(tok, varid, numAfter, tokenlist, errorLogger, settings);
            } else
                valueFlowForLoopSimplifyAfter(tok, varid, num1, tokenlist, errorLogger, settings);
        } else {
            ProgramMemoryFast mem1, mem2, memAfter;
            if (valueFlowForLoop2(tok, &mem1, &mem2, &memAfter)) {
                std::map<int, ValueFlow::Value>::const_iterator it;
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
 */

static void valueFlowInjectParameter(TokenList& tokenlist, ErrorLogger* errorLogger, const Settings& settings, const Variable* arg, const Scope* functionScope, const std::list<ValueFlow::Value>& argvalues)
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

static void valueFlowSwitchVariable(TokenList&tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings&settings)
{
    for (const Scope &scope : symboldatabase.scopeList) {
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
            if (settings.debugwarnings)
                bailout(tokenlist, errorLogger, vartok, "switch variable " + var->name() + " is global");
            continue;
        }

        for (Token *tok = const_cast<Token*>(scope.bodyStart->next()); tok != scope.bodyEnd; tok = tok->next()) {
            if (tok->str() == "{") {
                tok = tok->link();
                continue;
            }
            if (Token::Match(tok, "case %num% :")) {
                std::list<ValueFlow::Value> values;
                values.emplace_back(tok->next()->getKnownIntValue());
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
                    values.emplace_back(tok->next()->getKnownIntValue());
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

static void setTokenValues(Token *tok, const std::list<ValueFlow::Value> &values, const Settings&settings)
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
                res.valueType = ValueFlow::Value::ValueType::INT;
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
                ValueFlow::combineValueProperties(val1, val2, result->back());
            }
        }
        return !result->empty();
    }
    if (expr->str().compare(0,3,"arg")==0) {
        *result = values[expr->str()[3] - '1'];
        return true;
    }
    if (expr->isNumber()) {
        result->emplace_back(ValueFlow::Value(MathLib::toBigNumber(expr->str())));
        result->back().setKnown();
        return true;
    } else if (expr->tokType() == Token::eChar) {
        result->emplace_back(ValueFlow::Value(MathLib::toBigNumber(expr->str())));
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

static void valueFlowLibraryFunction(Token *tok, const std::string &returnValue, const Settings&settings)
{
    std::vector<std::list<ValueFlow::Value>> argValues;
    for (const Token *argtok : getArguments(tok->previous())) {
        argValues.emplace_back(getFunctionArgumentValues(argtok));
        if (argValues.back().empty())
            return;
    }
    if (returnValue.find("arg") != std::string::npos && argValues.empty())
        return;

    TokenList tokenList(&settings);
    {
        const std::string code = "return " + returnValue + ";";
        std::istringstream istr(code);
        if (!tokenList.createTokens(istr, Standards::Language::CPP))
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

static void valueFlowSubFunction(TokenList& tokenlist, ErrorLogger* errorLogger, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (!Token::Match(tok, "%name% ("))
            continue;

        const Function * const calledFunction = tok->function();
        if (!calledFunction) {
            // library function?
            const std::string& returnValue(settings.library.returnValue(tok));
            if (!returnValue.empty())
                valueFlowLibraryFunction(tok->next(), returnValue, settings);
            continue;
        }

        const Scope * const calledFunctionScope = calledFunction->functionScope;
        if (!calledFunctionScope)
            continue;

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

            // Don't forward lifetime values
            argvalues.remove_if(std::mem_fn(&ValueFlow::Value::isLifetimeValue));

            if (argvalues.empty())
                continue;

            // Error path..
            for (ValueFlow::Value &v : argvalues) {
                const std::string nr = std::to_string(argnr + 1) + getOrdinalText(argnr + 1);

                v.errorPath.emplace_back(argtok,
                                         "Calling function '" +
                                         calledFunction->name() +
                                         "', " +
                                         nr +
                                         " argument '" +
                                         argtok->expressionString() +
                                         "' value is " +
                                         v.infoString());
            }

            // passed values are not "known"..
            lowerToPossible(argvalues);

            valueFlowInjectParameter(tokenlist, errorLogger, settings, argvar, calledFunctionScope, argvalues);
            // FIXME: We need to rewrite the valueflow analysis to better handle multiple arguments
            if (!argvalues.empty())
                break;
        }
    }
}

static void valueFlowFunctionDefaultParameter(TokenList& tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings& settings)
{
    if (!tokenlist.isCPP())
        return;

    for (const Scope* scope : symboldatabase.functionScopes) {
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

// FIXME: execute
/*
   static bool isKnown(const Token * tok)
   {
    return tok && tok->hasKnownIntValue();
   }

   static void valueFlowFunctionReturn(TokenList& tokenlist, ErrorLogger *errorLogger, const Settings& settings)
   {
    for (Token *tok = tokenlist.back(); tok; tok = tok->previous()) {
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
            if (functionScope && settings.debugwarnings && Token::findsimplematch(functionScope->bodyStart, "return", functionScope->bodyEnd))
                bailout(tokenlist, errorLogger, tok, "function return; nontrivial function body");
            continue;
        }

        ProgramMemoryFast programMemory;
        for (std::size_t i = 0; i < parvalues.size(); ++i) {
            const Variable * const arg = function->getArgumentVar(i);
            if (!arg || !Token::Match(arg->typeStartToken(), "%type% %name% ,|)")) {
                if (settings.debugwarnings)
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
            setTokenValue(tok, v, settings);
        }
    }
   }
 */

static void valueFlowUninit(TokenList& tokenlist, ErrorLogger *errorLogger, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
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
        if (Token::Match(vardecl, "%varid% ; %varid% =", vardecl->varId()))
            continue;
        const Variable *var = vardecl->variable();
        if (!var || var->nameToken() != vardecl)
            continue;
        if ((!var->isPointer() && var->type() && var->type()->needInitialization != Type::NeedInitialization::True) ||
            !var->isLocal() || var->isStatic() || var->isExtern() || var->isReference() || var->isThrow())
            continue;
        if (!var->type() && !stdtype && !pointer)
            continue;

        ValueFlow::Value uninitValue;
        uninitValue.setKnown();
        uninitValue.valueType = ValueFlow::Value::ValueType::UNINIT;
        uninitValue.tokvalue = vardecl;
        std::list<ValueFlow::Value> values;
        values.push_back(uninitValue);

        const bool constValue = true;
        const bool subFunction = false;

        valueFlowForwardVariable(const_cast<Token*>(vardecl->next()),
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
    if (fun) {
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

static void valueFlowContainerReverse(Token *tok, nonneg int containerId, const ValueFlow::Value &value, const Settings&settings)
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

static void valueFlowContainerForward(Token *tok, nonneg int containerId, ValueFlow::Value value, const Settings& settings)
{
    while (nullptr != (tok = tok->next())) {
        if (Token::Match(tok, "[{}]"))
            break;
        if (Token::Match(tok, "while|for (")) {
            const Token *start = tok->linkAt(1)->next();
            if (!Token::simpleMatch(start->link(), "{"))
                break;
            if (isContainerSizeChanged(containerId, start, start->link()))
                break;
        }
        if (Token::simpleMatch(tok, ") {") && Token::Match(tok->link()->previous(), "while|for|if (")) {
            const Token *start = tok->next();
            if (isContainerSizeChanged(containerId, start, start->link()) || isEscapeScope(start, settings))
                break;
            tok = const_cast<Token*>(start->link());
            if (Token::simpleMatch(tok, "} else {")) {
                start = tok->tokAt(2);
                if (isContainerSizeChanged(containerId, start, start->link()))
                    break;
                tok = const_cast<Token*>(start->link());
            }
        }
        if (tok->varId() != containerId)
            continue;
        if (Token::Match(tok, "%name% ="))
            break;
        if (Token::Match(tok, "%name% +=")) {
            if (!tok->valueType() || !tok->valueType()->container || !tok->valueType()->container->stdStringLike)
                break;
            const Token *rhs = tok->next()->astOperand2();
            if (rhs->tokType() == Token::eString)
                value.intvalue += Token::getStrLength(rhs);
            else if (rhs->valueType() && rhs->valueType()->container && rhs->valueType()->container->stdStringLike) {
                bool found = false;
                for (const ValueFlow::Value &rhsval : rhs->values()) {
                    if (rhsval.isKnown() && rhsval.isContainerSizeValue()) {
                        value.intvalue += rhsval.intvalue;
                        found = true;
                    }
                }
                if (!found)
                    break;
            } else
                break;
        }
        if (isLikelyStreamRead(tok->astParent()))
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

static bool isContainerSizeChanged(nonneg int varId, const Token *start, const Token *end, int depth)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->varId() != varId)
            continue;
        if (!tok->valueType() || !tok->valueType()->container)
            return true;
        if (Token::Match(tok, "%name% %assign%|<<"))
            return true;
        if (Token::Match(tok, "%name% . %name% (")) {
            Library::Container::Action action = tok->valueType()->container->getAction(tok->strAt(2));
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
                return true;
            case Library::Container::Action::FIND:
            case Library::Container::Action::FIND_CONST:
            case Library::Container::Action::CHANGE_CONTENT:
                break;
            };
        }
        if (isContainerSizeChangedByFunction(tok, depth))
            return true;
    }
    return false;
}

static void valueFlowSmartPointer(TokenList&tokenlist, ErrorLogger * errorLogger, const Settings&settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (!tok->scope())
            continue;
        if (!tok->scope()->isExecutable())
            continue;
        if (!tok->variable())
            continue;
        const Variable * var = tok->variable();
        if (!var->isSmartPointer())
            continue;
        if (var->nameToken() == tok) {
            if (Token::Match(tok, "%var% (|{") && tok->next()->astOperand2() && tok->next()->astOperand2()->str() != ",") {
                Token * inTok = tok->next()->astOperand2();
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
                Token * inTok = tok->tokAt(3)->astOperand2();
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
    }
}

static void valueFlowContainerSize(const SymbolDatabase& symboldatabase, ErrorLogger * /*errorLogger*/, const Settings& settings)
{
    // declaration
    for (const Variable *var : symboldatabase.variableList()) {
        if (!var || !var->isLocal() || var->isPointer() || var->isReference())
            continue;
        if (!var->valueType() || !var->valueType()->container)
            continue;
        if (!Token::Match(var->nameToken(), "%name% ;"))
            continue;
        if (var->nameToken()->hasKnownValue())
            continue;
        ValueFlow::Value value(0);
        if (var->valueType()->container->size_templateArgNo >= 0) {
            if (var->dimensions().size() == 1 && var->dimensions().front().tok && var->dimensions().front().tok->hasKnownIntValue())
                value.intvalue = var->dimensions().front().tok->getKnownIntValue();
            else
                continue;
        }
        value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
        value.setKnown();
        valueFlowContainerForward(const_cast<Token*>(var->nameToken()->next()), var->declarationId(), value, settings);
    }

    // after assignment
    for (const Scope *functionScope : symboldatabase.functionScopes) {
        for (const Token *tok = functionScope->bodyStart; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "%name%|;|{|} %var% = %str% ;")) {
                const Token *containerTok = tok->next();
                if (containerTok && containerTok->valueType() && containerTok->valueType()->container && containerTok->valueType()->container->stdStringLike) {
                    ValueFlow::Value value(Token::getStrLength(containerTok->tokAt(2)));
                    value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
                    value.setKnown();
                    valueFlowContainerForward(const_cast<Token*>(containerTok->next()), containerTok->varId(), value, settings);
                }
            }
        }
    }

    // conditional conditionSize
    for (const Scope &scope : symboldatabase.scopeList) {
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

static void valueFlowContainerAfterCondition(TokenList& tokenlist,
                                             const SymbolDatabase& symboldatabase,
                                             ErrorLogger *errorLogger,
                                             const Settings& settings)
{
    ValueFlowConditionHandler handler;
    handler.forward =
        [&](Token* start, const Token* stop, const Token* vartok, const std::list<ValueFlow::Value>& values, bool) {
        // TODO: Forward multiple values
        if (values.empty())
            return false;
        const Variable* var = vartok->variable();
        if (!var)
            return false;
        valueFlowContainerForward(start, var->declarationId(), values.front(), settings);
        return isContainerSizeChanged(var->declarationId(), start, stop);
    };
    handler.parse = [&](const Token *tok) {
        ValueFlowConditionHandler::Condition cond;
        ValueFlow::Value true_value;
        ValueFlow::Value false_value;
        const Token *vartok = ValueFlow::parseCompareInt(tok, true_value, false_value);
        if (vartok) {
            vartok = vartok->tokAt(-3);
            if (!isContainerSize(vartok))
                return cond;
            true_value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            false_value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
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
                if (Token::Match(parent, "%comp%|!"))
                    return cond;
                parent = parent->astParent();
            }
            ValueFlow::Value value(tok, 0LL);
            value.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
            cond.true_values.emplace_back(value);
            cond.false_values.emplace_back(std::move(value));
            cond.vartok = vartok;
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
    };
    handler.afterCondition(tokenlist, symboldatabase, errorLogger, settings);
}

static void valueFlowFwdAnalysis(const TokenList&tokenlist, const Settings&settings)
{
    for (const Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (Token::simpleMatch(tok, "for ("))
            tok = tok->linkAt(1);
        if (tok->str() != "=" || !tok->astOperand1() || !tok->astOperand2())
            continue;
        if (!tok->scope()->isExecutable())
            continue;
        if (!tok->astOperand2()->hasKnownIntValue())
            continue;
        ValueFlow::Value v(tok->astOperand2()->values().front());
        v.errorPath.emplace_back(tok, tok->astOperand1()->expressionString() + " is assigned value " + std::to_string(v.intvalue));
        const Token *startToken = tok->findExpressionStartEndTokens().second->next();
        const Scope *functionScope = tok->scope();
        while (functionScope->nestedIn && functionScope->nestedIn->isExecutable())
            functionScope = functionScope->nestedIn;
        const Token *endToken = functionScope->bodyEnd;
        valueFlowForwardExpression(const_cast<Token*>(startToken), endToken, tok->astOperand1(), {v}, tokenlist, settings);
    }
}

static void valueFlowDynamicBufferSize(TokenList& tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings& settings)
{
    for (const Scope *functionScope : symboldatabase.functionScopes) {
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

            const Library::AllocFunc *allocFunc = settings.library.getAllocFuncInfo(rhs->previous());
            if (!allocFunc)
                allocFunc = settings.library.getReallocFuncInfo(rhs->previous());
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
            };
            if (sizeValue < 0)
                continue;

            ValueFlow::Value value(sizeValue);
            value.errorPath.emplace_back(tok->tokAt(2), "Assign " + tok->strAt(1) + ", buffer with size " + std::to_string(sizeValue));
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

static bool getMinMaxValues(const ValueType *vt, const Platform &platform, MathLib::bigint *minValue, MathLib::bigint *maxValue)
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
    };

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

static bool getMinMaxValues(const std::string &typestr, const Settings& settings, MathLib::bigint *minvalue, MathLib::bigint *maxvalue)
{
    TokenList typeTokens(&settings);
    std::istringstream istr(typestr+";");
    if (!typeTokens.createTokens(istr, Standards::Language::CPP))
        return false;
    typeTokens.simplifyPlatformTypes();
    typeTokens.simplifyStdType();
    const ValueType &vt = ValueType::parseDecl(typeTokens.front(), settings);
    return getMinMaxValues(&vt, settings.platform, minvalue, maxvalue);
}

// FIXME
/*
   static void valueFlowSafeFunctions(TokenList& tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings& settings)
   {
    for (const Scope *functionScope : symboldatabase.functionScopes) {
        if (!functionScope->bodyStart)
            continue;
        const Function *function = functionScope->function;
        if (!function)
            continue;

        const bool safe = function->isSafe(settings);
        const bool all = safe && settings.platform.type != Platform::Type::Unspecified;

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
                    valueFlowContainerForward(const_cast<Token*>(functionScope->bodyStart), arg.declarationId(), value, settings);
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
 */

static void valueFlowUnknownFunctionReturn(TokenList& tokenlist, const Settings& settings)
{
    if (settings.checkUnknownFunctionReturn.empty())
        return;
    for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
        if (!tok->astParent() || tok->str() != "(")
            continue;
        if (!Token::Match(tok->previous(), "%name%"))
            continue;
        if (settings.checkUnknownFunctionReturn.find(tok->previous()->str()) == settings.checkUnknownFunctionReturn.end())
            continue;
        std::vector<MathLib::bigint> unknownValues = settings.library.unknownReturnValues(tok->astOperand1());
        if (unknownValues.empty())
            continue;

        // Get min/max values for return type
        const std::string &typestr = settings.library.returnValueType(tok->previous());
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

const ValueFlow::Value *ValueFlowFast::valueFlowConstantFoldAST(Token *expr, const Settings& settings)
{
    if (expr && expr->values().empty()) {
        valueFlowConstantFoldAST(expr->astOperand1(), settings);
        valueFlowConstantFoldAST(expr->astOperand2(), settings);
        valueFlowSetConstantValue(expr, settings, true /* TODO: this is a guess */);
    }
    return expr && expr->hasKnownValue() ? &expr->values().front() : nullptr;
}

static std::size_t getTotalValues(TokenList&tokenlist)
{
    std::size_t n = 1;
    for (Token *tok = tokenlist.front(); tok; tok = tok->next())
        n += tok->values().size();
    return n;
}

void ValueFlowFast::setValues(TokenList& tokenlist, SymbolDatabase& symboldatabase, ErrorLogger *errorLogger, const Settings& settings)
{
    for (Token *tok = tokenlist.front(); tok; tok = tok->next())
        tok->clearValueFlow();

    valueFlowEnumValue(symboldatabase, settings);
    valueFlowNumber(tokenlist, settings);
    valueFlowString(tokenlist, settings);
    valueFlowArray(tokenlist, settings);
    valueFlowUnknownFunctionReturn(tokenlist, settings);
    valueFlowGlobalConstVar(tokenlist, settings);
    valueFlowEnumValue(symboldatabase, settings);
    valueFlowGlobalStaticVar(tokenlist, settings);
    valueFlowPointerAlias(tokenlist, settings);
    ValueFlow::valueFlowLifetime(tokenlist, *errorLogger, settings);
    // ? valueFlowFunctionReturn(tokenlist, errorLogger);
    valueFlowBitAnd(tokenlist, settings);
    valueFlowSameExpressions(tokenlist, settings);
    valueFlowFwdAnalysis(tokenlist, settings);

    std::size_t values = 0;
    std::size_t n = 4;
    while (n > 0 && values < getTotalValues(tokenlist)) {
        values = getTotalValues(tokenlist);
        // ? valueFlowPointerAliasDeref(tokenlist);
        valueFlowArrayBool(tokenlist, settings);
        valueFlowArrayElement(tokenlist, settings);
        valueFlowRightShift(tokenlist, settings);
        // ? valueFlowOppositeCondition(symboldatabase, settings);
        // ? valueFlowTerminatingCondition(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowBeforeCondition(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowAfterMove(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowAfterAssign(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowAfterCondition(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowInferCondition(tokenlist, settings);
        valueFlowSwitchVariable(tokenlist, symboldatabase, errorLogger, settings);
        // FIXME valueFlowForLoop(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowSubFunction(tokenlist, errorLogger, settings);
        valueFlowFunctionDefaultParameter(tokenlist, symboldatabase, errorLogger, settings);
        valueFlowUninit(tokenlist, errorLogger, settings);
        if (tokenlist.isCPP()) {
            valueFlowSmartPointer(tokenlist, errorLogger, settings);
            valueFlowContainerSize(symboldatabase, errorLogger, settings);
            valueFlowContainerAfterCondition(tokenlist, symboldatabase, errorLogger, settings);
        }
        // FIXME valueFlowSafeFunctions(tokenlist, symboldatabase, errorLogger, settings);
        n--;
    }

    valueFlowDynamicBufferSize(tokenlist, symboldatabase, errorLogger, settings);
}


std::string ValueFlowFast::eitherTheConditionIsRedundant(const Token *condition)
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

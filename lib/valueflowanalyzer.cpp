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

#include "valueflowanalyzer.h"

#include "astutils.h"
#include "calculate.h"
#include "library.h"
#include "settings.h"
#include "smallvector.h"
#include "symboldatabase.h"
#include "utils.h"
#include "valueflow.h"
#include "vfvalue.h"

#include "vf_common.h"
#include "vf_settokenvalue.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <list>
#include <type_traits>
#include <utility>

int ValueFlowAnalyzer::getIndirect(const Token* tok) const
{
    const ValueFlow::Value* value = getValue(tok);
    if (value)
        return value->indirect;
    return 0;
}

ValueFlowAnalyzer::ConditionState ValueFlowAnalyzer::analyzeCondition(const Token* tok, int depth) const
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
    }
    if (tok->hasKnownIntValue() || tok->isLiteral()) {
        result.dependent = false;
        result.unknown = false;
        return result;
    }
    if (Token::Match(tok, "%cop%")) {
        if (isLikelyStream(tok->astOperand1())) {
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
    }
    if (Token::Match(tok->previous(), "%name% (")) {
        std::vector<const Token*> args = getArguments(tok->previous());
        if (Token::Match(tok->tokAt(-2), ". %name% (")) {
            args.push_back(tok->tokAt(-2)->astOperand1());
        }
        result.dependent = std::any_of(args.cbegin(), args.cend(), [&](const Token* arg) {
            ConditionState cs = analyzeCondition(arg, depth - 1);
            return cs.dependent;
        });
        if (result.dependent) {
            // Check if we can evaluate the function
            if (!evaluate(Evaluate::Integral, tok).empty())
                result.unknown = false;
        }
        return result;
    }

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

Analyzer::Action ValueFlowAnalyzer::isModified(const Token* tok) const
{
    const Action read = Action::Read;
    const ValueFlow::Value* value = getValue(tok);
    if (value) {
        // Moving a moved value won't change the moved value
        if (value->isMovedValue() && isMoveOrForward(tok) != ValueFlow::Value::MoveKind::NonMovedVariable)
            return read;
        // Inserting elements to container won't change the lifetime
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
    if (isVariableChanged(tok, getIndirect(tok), getSettings())) {
        if (Token::Match(tok->astParent(), "*|[|.|++|--"))
            return read | Action::Invalid;
        // Check if its assigned to the same value
        if (value && !value->isImpossible() && Token::simpleMatch(tok->astParent(), "=") && astIsLHS(tok) &&
            astIsIntegral(tok->astParent()->astOperand2(), false)) {
            std::vector<MathLib::bigint> result = evaluateInt(tok->astParent()->astOperand2());
            if (!result.empty() && value->equalTo(result.front()))
                return Action::Idempotent;
        }
        return Action::Invalid;
    }
    return read;
}

Analyzer::Action ValueFlowAnalyzer::isAliasModified(const Token* tok, int indirect) const
{
    // Lambda function call
    if (Token::Match(tok, "%var% ("))
        // TODO: Check if modified in the lambda function
        return Action::Invalid;
    if (indirect == -1) {
        indirect = 0;
        if (const ValueType* vt = tok->valueType()) {
            indirect = vt->pointer;
            if (vt->type == ValueType::ITERATOR)
                ++indirect;
            const Token* tok2 = tok;
            while (Token::simpleMatch(tok2->astParent(), "[")) {
                tok2 = tok2->astParent();
                --indirect;
            }
            indirect = std::max(indirect, 0);
        }
    }
    for (int i = 0; i <= indirect; ++i)
        if (isVariableChanged(tok, i, getSettings()))
            return Action::Invalid;
    return Action::None;
}

Analyzer::Action ValueFlowAnalyzer::isThisModified(const Token* tok) const
{
    if (isThisChanged(tok, 0, getSettings()))
        return Action::Invalid;
    return Action::None;
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
        return emptyString;
    }
    return it->second;
}

static const std::string& getAssign(const Token* tok, Analyzer::Direction d)
{
    if (d == Analyzer::Direction::Forward)
        return tok->str();
    return invertAssign(tok->str());
}

template<class T, class U>
static void assignValueIfMutable(T& x, const U& y)
{
    x = y;
}

template<class T, class U>
static void assignValueIfMutable(const T& /*unused*/, const U& /*unused*/)
{}

static std::string removeAssign(const std::string& assign)
{
    return std::string{assign.cbegin(), assign.cend() - 1};
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

Analyzer::Action ValueFlowAnalyzer::isWritable(const Token* tok, Direction d) const
{
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
    if (parent && parent->isAssignmentOp() && astIsLHS(tok)) {
        const Token* rhs = parent->astOperand2();
        std::vector<MathLib::bigint> result = evaluateInt(rhs);
        if (!result.empty()) {
            ValueFlow::Value rhsValue{result.front()};
            Action a;
            if (!evalAssignment(*value, getAssign(parent, d), rhsValue))
                a = Action::Invalid;
            else
                a = Action::Write;
            if (parent->str() != "=") {
                a |= Action::Read | Action::Incremental;
            } else {
                if (!value->isImpossible() && value->equalValue(rhsValue))
                    a = Action::Idempotent;
                if (tok->exprId() != 0 &&
                    findAstNode(rhs, [&](const Token* child) {
                    return tok->exprId() == child->exprId();
                }))
                    a |= Action::Incremental;
            }
            return a;
        }
    }

    // increment/decrement
    if (Token::Match(tok->astParent(), "++|--")) {
        return Action::Read | Action::Write | Action::Incremental;
    }
    return Action::None;
}

void ValueFlowAnalyzer::writeValue(ValueFlow::Value* value, const Token* tok, Direction d) const
{
    if (!value)
        return;
    if (!tok->astParent())
        return;
    // Lifetime value doesn't change
    if (value->isLifetimeValue())
        return;
    if (tok->astParent()->isAssignmentOp()) {
        const Token* rhs = tok->astParent()->astOperand2();
        std::vector<MathLib::bigint> result = evaluateInt(rhs);
        assert(!result.empty());
        ValueFlow::Value rhsValue{result.front()};
        if (evalAssignment(*value, getAssign(tok->astParent(), d), rhsValue)) {
            std::string info("Compound assignment '" + tok->astParent()->str() + "', assigned value is " +
                             value->infoString());
            if (tok->astParent()->str() == "=")
                value->errorPath.clear();
            value->errorPath.emplace_back(tok, std::move(info));
        } else {
            assert(false && "Writable value cannot be evaluated");
            // TODO: Don't set to zero
            value->intvalue = 0;
        }
    } else if (tok->astParent()->tokType() == Token::eIncDecOp) {
        bool inc = tok->astParent()->str() == "++";
        const std::string opName(inc ? "incremented" : "decremented");
        if (d == Direction::Reverse)
            inc = !inc;
        value->intvalue += (inc ? 1 : -1);

        /* Truncate value */
        const ValueType *dst = tok->valueType();
        if (dst) {
            const size_t sz = ValueFlow::getSizeOf(*dst, settings);
            if (sz > 0 && sz < sizeof(MathLib::biguint)) {
                long long newvalue = ValueFlow::truncateIntValue(value->intvalue, sz, dst->sign);

                /* Handle overflow/underflow for value bounds */
                if (value->bound != ValueFlow::Value::Bound::Point) {
                    if ((newvalue > value->intvalue && !inc) || (newvalue < value->intvalue && inc))
                        value->invertBound();
                }

                value->intvalue = newvalue;
            }

            value->errorPath.emplace_back(tok, tok->str() + " is " + opName + "', new value is " + value->infoString());
        }
    }
}

void ValueFlowAnalyzer::internalUpdate(Token* /*tok*/, const ValueFlow::Value& /*v*/, Direction /*d*/)
{
    assert(false && "Internal update unimplemented.");
}

Analyzer::Action ValueFlowAnalyzer::analyzeLifetime(const Token* tok) const
{
    if (!tok)
        return Action::None;
    if (match(tok))
        return Action::Match;
    if (Token::simpleMatch(tok, ".") && analyzeLifetime(tok->astOperand1()) != Action::None)
        return Action::Read;
    if (astIsRHS(tok) && Token::simpleMatch(tok->astParent(), "."))
        return analyzeLifetime(tok->astParent());
    return Action::None;
}

std::unordered_map<nonneg int, const Token*> ValueFlowAnalyzer::getSymbols(const Token* tok) const
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

Analyzer::Action ValueFlowAnalyzer::isGlobalModified(const Token* tok) const
{
    if (tok->function()) {
        if (!tok->function()->isConstexpr() && !isConstFunctionCall(tok, getSettings().library))
            return Action::Invalid;
    } else if (getSettings().library.getFunction(tok)) {
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

const Token* ValueFlowAnalyzer::findMatch(const Token* tok) const
{
    return findAstNode(tok, [&](const Token* child) {
        return match(child);
    });
}

bool ValueFlowAnalyzer::isSameSymbolicValue(const Token* tok, ValueFlow::Value* value) const
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
        std::any_of(tok->values().cbegin(), tok->values().cend(), [&](const ValueFlow::Value& v) {
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
                value->errorPath.insert(value->errorPath.end(), v.errorPath.cbegin(), v.errorPath.cend());
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

Analyzer::Action ValueFlowAnalyzer::analyzeMatch(const Token* tok, Direction d) const
{
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

Analyzer::Action ValueFlowAnalyzer::analyzeToken(const Token* ref, const Token* tok, Direction d, bool inconclusiveRef) const
{
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
        if (!lifeTok)
            return Action::None;
        Action la = analyzeLifetime(lifeTok);
        if (la.matches()) {
            Action a = Action::Read;
            if (isModified(tok).isModified())
                a = Action::Invalid;
            if (Token::Match(tok->astParent(), "%assign%") && astIsLHS(tok))
                a |= Action::Invalid;
            if (inconclusiveRef && a.isModified())
                return Action::Inconclusive;
            return a;
        }
        if (la.isRead()) {
            return isAliasModified(tok);
        }
        return Action::None;

    } else if (isAlias(ref, inconclusive)) {
        inconclusive |= inconclusiveRef;
        Action a = isAliasModified(tok);
        if (inconclusive && a.isModified())
            return Action::Inconclusive;
        return a;
    }
    if (isSameSymbolicValue(ref))
        return Action::Read | Action::SymbolicMatch;

    return Action::None;
}

Analyzer::Action ValueFlowAnalyzer::analyze(const Token* tok, Direction d) const
{
    if (invalid())
        return Action::Invalid;
    // Follow references
    auto refs = followAllReferences(tok);
    const bool inconclusiveRefs = refs.size() != 1;
    if (std::none_of(refs.cbegin(), refs.cend(), [&](const ReferenceToken& ref) {
        return tok == ref.token;
    }))
        refs.emplace_back(ReferenceToken{tok, {}});
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
    if (isGlobal() && !dependsOnThis() && Token::Match(tok, "%name% (") && !tok->variable() &&
        !Token::simpleMatch(tok->linkAt(1), ") {")) {
        return isGlobalModified(tok);
    }
    return Action::None;
}

template<class F>
static std::vector<MathLib::bigint> evaluateInt(const Token* tok, const Settings& settings, F getProgramMemory)
{
    if (tok->hasKnownIntValue())
        return {static_cast<int>(tok->values().front().intvalue)};
    std::vector<MathLib::bigint> result;
    ProgramMemory pm = getProgramMemory();
    if (Token::Match(tok, "&&|%oror%")) {
        if (conditionIsTrue(tok, pm, settings))
            result.push_back(1);
        if (conditionIsFalse(tok, std::move(pm), settings))
            result.push_back(0);
    } else {
        MathLib::bigint out = 0;
        bool error = false;
        execute(tok, pm, &out, &error, settings);
        if (!error)
            result.push_back(out);
    }
    return result;
}

std::vector<MathLib::bigint> ValueFlowAnalyzer::evaluateInt(const Token* tok) const
{
    return ::evaluateInt(tok, settings, [&] {
        return ProgramMemory{getProgramState()};
    });
}

std::vector<MathLib::bigint> ValueFlowAnalyzer::evaluate(Evaluate e, const Token* tok, const Token* ctx) const
{
    if (e == Evaluate::Integral) {
        return ::evaluateInt(tok, settings, [&] {
            return pms.get(tok, ctx, getProgramState());
        });
    }
    if (e == Evaluate::ContainerEmpty) {
        const ValueFlow::Value* value = ValueFlow::findValue(tok->values(), settings, [](const ValueFlow::Value& v) {
            return v.isKnown() && v.isContainerSizeValue();
        });
        if (value)
            return {value->intvalue == 0};
        ProgramMemory pm = pms.get(tok, ctx, getProgramState());
        MathLib::bigint out = 0;
        if (pm.getContainerEmptyValue(tok->exprId(), out))
            return {static_cast<int>(out)};
        return {};
    }
    return {};
}

void ValueFlowAnalyzer::assume(const Token* tok, bool state, unsigned int flags)
{
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
        if (state) {
            pms.removeModifiedVars(endBlock);
            pms.addState(endBlock->previous(), getProgramState());
        } else {
            if (Token::simpleMatch(endBlock, "} else {"))
                pms.addState(endBlock->linkAt(2)->previous(), getProgramState());
        }
    }

    if (!(flags & Assume::Quiet)) {
        if (flags & Assume::ContainerEmpty) {
            std::string s = state ? "empty" : "not empty";
            addErrorPath(tok, "Assuming container is " + s);
        } else {
            std::string s = bool_to_string(state);
            addErrorPath(tok, "Assuming condition is " + s);
        }
    }
    if (!(flags & Assume::Absolute))
        makeConditional();
}

void ValueFlowAnalyzer::updateState(const Token* tok)
{
    // Update program state
    pms.removeModifiedVars(tok);
    pms.addState(tok, getProgramState());
}

void ValueFlowAnalyzer::update(Token* tok, Action a, Direction d)
{
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
        (void)lowerToInconclusive();
    if (a.isWrite() && tok->astParent()) {
        writeValue(value, tok, d);
    }
    // Read last when moving in reverse
    if (d == Direction::Reverse && a.isRead())
        setTokenValue(tok, *value, getSettings());
}

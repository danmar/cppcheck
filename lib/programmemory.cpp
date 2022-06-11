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

#include "programmemory.h"

#include "astutils.h"
#include "calculate.h"
#include "infer.h"
#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "utils.h"
#include "valueflow.h"
#include "valueptr.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

nonneg int ExprIdToken::getExpressionId() const {
    return tok ? tok->exprId() : exprid;
}

std::size_t ExprIdToken::Hash::operator()(ExprIdToken etok) const
{
    return std::hash<nonneg int>()(etok.getExpressionId());
}

void ProgramMemory::setValue(const Token* expr, const ValueFlow::Value& value) {
    mValues[expr] = value;
    ValueFlow::Value subvalue = value;
    const Token* subexpr = solveExprValue(
        expr,
        [&](const Token* tok) -> std::vector<MathLib::bigint> {
        if (tok->hasKnownIntValue())
            return {tok->values().front().intvalue};
        MathLib::bigint result = 0;
        if (getIntValue(tok->exprId(), &result))
            return {result};
        return {};
    },
        subvalue);
    if (subexpr)
        mValues[subexpr] = subvalue;
}
const ValueFlow::Value* ProgramMemory::getValue(nonneg int exprid, bool impossible) const
{
    const ProgramMemory::Map::const_iterator it = mValues.find(exprid);
    const bool found = it != mValues.end() && (impossible || !it->second.isImpossible());
    if (found)
        return &it->second;
    else
        return nullptr;
}

// cppcheck-suppress unusedFunction
bool ProgramMemory::getIntValue(nonneg int exprid, MathLib::bigint* result) const
{
    const ValueFlow::Value* value = getValue(exprid);
    if (value && value->isIntValue()) {
        *result = value->intvalue;
        return true;
    }
    return false;
}

void ProgramMemory::setIntValue(const Token* expr, MathLib::bigint value, bool impossible)
{
    ValueFlow::Value v(value);
    if (impossible)
        v.setImpossible();
    setValue(expr, v);
}

bool ProgramMemory::getTokValue(nonneg int exprid, const Token** result) const
{
    const ValueFlow::Value* value = getValue(exprid);
    if (value && value->isTokValue()) {
        *result = value->tokvalue;
        return true;
    }
    return false;
}

// cppcheck-suppress unusedFunction
bool ProgramMemory::getContainerSizeValue(nonneg int exprid, MathLib::bigint* result) const
{
    const ValueFlow::Value* value = getValue(exprid);
    if (value && value->isContainerSizeValue()) {
        *result = value->intvalue;
        return true;
    }
    return false;
}
bool ProgramMemory::getContainerEmptyValue(nonneg int exprid, MathLib::bigint* result) const
{
    const ValueFlow::Value* value = getValue(exprid, true);
    if (value && value->isContainerSizeValue()) {
        if (value->isImpossible() && value->intvalue == 0) {
            *result = false;
            return true;
        }
        if (!value->isImpossible()) {
            *result = (value->intvalue == 0);
            return true;
        }
    }
    return false;
}

void ProgramMemory::setContainerSizeValue(const Token* expr, MathLib::bigint value, bool isEqual)
{
    ValueFlow::Value v(value);
    v.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
    if (!isEqual)
        v.valueKind = ValueFlow::Value::ValueKind::Impossible;
    setValue(expr, v);
}

void ProgramMemory::setUnknown(const Token* expr) {
    mValues[expr].valueType = ValueFlow::Value::ValueType::UNINIT;
}

bool ProgramMemory::hasValue(nonneg int exprid)
{
    return mValues.find(exprid) != mValues.end();
}

const ValueFlow::Value& ProgramMemory::at(nonneg int exprid) const {
    return mValues.at(exprid);
}
ValueFlow::Value& ProgramMemory::at(nonneg int exprid) {
    return mValues.at(exprid);
}

void ProgramMemory::erase_if(const std::function<bool(const ExprIdToken&)>& pred)
{
    for (auto it = mValues.begin(); it != mValues.end();) {
        if (pred(it->first))
            it = mValues.erase(it);
        else
            ++it;
    }
}

void ProgramMemory::swap(ProgramMemory &pm)
{
    mValues.swap(pm.mValues);
}

void ProgramMemory::clear()
{
    mValues.clear();
}

bool ProgramMemory::empty() const
{
    return mValues.empty();
}

void ProgramMemory::replace(const ProgramMemory &pm)
{
    for (auto&& p : pm.mValues) {
        mValues[p.first] = p.second;
    }
}

void ProgramMemory::insert(const ProgramMemory &pm)
{
    for (auto&& p : pm)
        mValues.insert(p);
}

static ValueFlow::Value execute(const Token* expr, ProgramMemory& pm, const Settings* settings = nullptr);

static bool evaluateCondition(const std::string& op,
                              MathLib::bigint r,
                              const Token* condition,
                              ProgramMemory& pm,
                              const Settings* settings)
{
    if (!condition)
        return false;
    if (condition->str() == op) {
        return evaluateCondition(op, r, condition->astOperand1(), pm, settings) ||
               evaluateCondition(op, r, condition->astOperand2(), pm, settings);
    }
    MathLib::bigint result = 0;
    bool error = false;
    execute(condition, &pm, &result, &error, settings);
    return !error && result == r;
}

bool conditionIsFalse(const Token* condition, ProgramMemory pm, const Settings* settings)
{
    return evaluateCondition("&&", 0, condition, pm, settings);
}

bool conditionIsTrue(const Token* condition, ProgramMemory pm, const Settings* settings)
{
    return evaluateCondition("||", 1, condition, pm, settings);
}

static bool frontIs(const std::vector<MathLib::bigint>& v, bool i)
{
    if (v.empty())
        return false;
    if (v.front())
        return i;
    return !i;
}

// If the scope is a non-range for loop
static bool isBasicForLoop(const Token* tok)
{
    if (!tok)
        return false;
    if (Token::simpleMatch(tok, "}"))
        return isBasicForLoop(tok->link());
    if (!Token::simpleMatch(tok->previous(), ") {"))
        return false;
    const Token* start = tok->linkAt(-1);
    if (!start)
        return false;
    if (!Token::simpleMatch(start->previous(), "for ("))
        return false;
    if (!Token::simpleMatch(start->astOperand2(), ";"))
        return false;
    return true;
}

void programMemoryParseCondition(ProgramMemory& pm, const Token* tok, const Token* endTok, const Settings* settings, bool then)
{
    auto eval = [&](const Token* t) -> std::vector<MathLib::bigint> {
        if (t->hasKnownIntValue())
            return {t->values().front().intvalue};
        MathLib::bigint result = 0;
        bool error = false;
        execute(t, &pm, &result, &error);
        if (!error)
            return {result};
        return std::vector<MathLib::bigint>{};
    };
    if (Token::Match(tok, "==|>=|<=|<|>|!=")) {
        ValueFlow::Value truevalue;
        ValueFlow::Value falsevalue;
        const Token* vartok = parseCompareInt(tok, truevalue, falsevalue, eval);
        if (!vartok)
            return;
        if (vartok->exprId() == 0)
            return;
        if (!truevalue.isIntValue())
            return;
        if (endTok && isExpressionChanged(vartok, tok->next(), endTok, settings, true))
            return;
        const bool impossible = (tok->str() == "==" && !then) || (tok->str() == "!=" && then);
        const ValueFlow::Value& v = then ? truevalue : falsevalue;
        pm.setValue(vartok, impossible ? asImpossible(v) : v);
        const Token* containerTok = settings->library.getContainerFromYield(vartok, Library::Container::Yield::SIZE);
        if (containerTok)
            pm.setContainerSizeValue(containerTok, v.intvalue, !impossible);
    } else if (Token::simpleMatch(tok, "!")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, !then);
    } else if (then && Token::simpleMatch(tok, "&&")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
    } else if (!then && Token::simpleMatch(tok, "||")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
    } else if (Token::Match(tok, "&&|%oror%")) {
        std::vector<MathLib::bigint> lhs = eval(tok->astOperand1());
        std::vector<MathLib::bigint> rhs = eval(tok->astOperand2());
        if (lhs.empty() || rhs.empty()) {
            if (frontIs(lhs, !then))
                programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
            if (frontIs(rhs, !then))
                programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
        }
    } else if (tok->exprId() > 0) {
        if (endTok && isExpressionChanged(tok, tok->next(), endTok, settings, true))
            return;
        pm.setIntValue(tok, 0, then);
        const Token* containerTok = settings->library.getContainerFromYield(tok, Library::Container::Yield::EMPTY);
        if (containerTok)
            pm.setContainerSizeValue(containerTok, 0, then);
    }
}

static void fillProgramMemoryFromConditions(ProgramMemory& pm, const Scope* scope, const Token* endTok, const Settings* settings)
{
    if (!scope)
        return;
    if (!scope->isLocal())
        return;
    assert(scope != scope->nestedIn);
    fillProgramMemoryFromConditions(pm, scope->nestedIn, endTok, settings);
    if (scope->type == Scope::eIf || scope->type == Scope::eWhile || scope->type == Scope::eElse || scope->type == Scope::eFor) {
        const Token* condTok = getCondTokFromEnd(scope->bodyEnd);
        if (!condTok)
            return;
        MathLib::bigint result = 0;
        bool error = false;
        execute(condTok, &pm, &result, &error);
        if (error)
            programMemoryParseCondition(pm, condTok, endTok, settings, scope->type != Scope::eElse);
    }
}

static void fillProgramMemoryFromConditions(ProgramMemory& pm, const Token* tok, const Settings* settings)
{
    fillProgramMemoryFromConditions(pm, tok->scope(), tok, settings);
}

static void fillProgramMemoryFromAssignments(ProgramMemory& pm, const Token* tok, const ProgramMemory& state, const ProgramMemory::Map& vars)
{
    int indentlevel = 0;
    for (const Token *tok2 = tok; tok2; tok2 = tok2->previous()) {
        if ((Token::simpleMatch(tok2, "=") || Token::Match(tok2->previous(), "%var% (|{")) && tok2->astOperand1() &&
            tok2->astOperand2()) {
            bool setvar = false;
            const Token* vartok = tok2->astOperand1();
            const Token* valuetok = tok2->astOperand2();
            for (const auto& p:vars) {
                if (p.first != vartok->exprId())
                    continue;
                if (vartok == tok)
                    continue;
                pm.setValue(vartok, p.second);
                setvar = true;
            }
            if (!setvar) {
                if (!pm.hasValue(vartok->exprId())) {
                    pm.setValue(vartok, execute(valuetok, pm));
                }
            }
        } else if (tok2->exprId() > 0 && Token::Match(tok2, ".|(|[|*|%var%") && !pm.hasValue(tok2->exprId()) &&
                   isVariableChanged(tok2, 0, nullptr, true)) {
            pm.setUnknown(tok2);
        }

        if (tok2->str() == "{") {
            if (indentlevel <= 0) {
                const Token* cond = getCondTokFromEnd(tok2->link());
                // Keep progressing with anonymous/do scopes and always true branches
                if (!Token::Match(tok2->previous(), "do|; {") && !conditionIsTrue(cond, state) &&
                    (cond || !isBasicForLoop(tok2)))
                    break;
            } else
                --indentlevel;
            if (Token::simpleMatch(tok2->previous(), "else {"))
                tok2 = tok2->linkAt(-2)->previous();
        }
        if (tok2->str() == "}") {
            const Token *cond = getCondTokFromEnd(tok2);
            const bool inElse = Token::simpleMatch(tok2->link()->previous(), "else {");
            if (cond) {
                if (conditionIsFalse(cond, state)) {
                    if (inElse) {
                        ++indentlevel;
                        continue;
                    }
                } else if (conditionIsTrue(cond, state)) {
                    if (inElse)
                        tok2 = tok2->link()->tokAt(-2);
                    ++indentlevel;
                    continue;
                }
            }
            break;
        }
    }
}

static void removeModifiedVars(ProgramMemory& pm, const Token* tok, const Token* origin)
{
    pm.erase_if([&](const ExprIdToken& e) {
        return isVariableChanged(origin, tok, e.getExpressionId(), false, nullptr, true);
    });
}

static ProgramMemory getInitialProgramState(const Token* tok,
                                            const Token* origin,
                                            const ProgramMemory::Map& vars = ProgramMemory::Map {})
{
    ProgramMemory pm;
    if (origin) {
        fillProgramMemoryFromConditions(pm, origin, nullptr);
        const ProgramMemory state = pm;
        fillProgramMemoryFromAssignments(pm, tok, state, vars);
        removeModifiedVars(pm, tok, origin);
    }
    return pm;
}

ProgramMemoryState::ProgramMemoryState(const Settings* s) : state(), origins(), settings(s) {}

void ProgramMemoryState::insert(const ProgramMemory &pm, const Token* origin)
{
    if (origin)
        for (auto&& p : pm)
            origins.insert(std::make_pair(p.first.getExpressionId(), origin));
    state.insert(pm);
}

void ProgramMemoryState::replace(const ProgramMemory &pm, const Token* origin)
{
    if (origin)
        for (auto&& p : pm)
            origins[p.first.getExpressionId()] = origin;
    state.replace(pm);
}

static void addVars(ProgramMemory& pm, const ProgramMemory::Map& vars)
{
    for (const auto& p:vars) {
        const ValueFlow::Value &value = p.second;
        pm.setValue(p.first.tok, value);
    }
}

void ProgramMemoryState::addState(const Token* tok, const ProgramMemory::Map& vars)
{
    ProgramMemory pm = state;
    addVars(pm, vars);
    fillProgramMemoryFromConditions(pm, tok, settings);
    ProgramMemory local = pm;
    fillProgramMemoryFromAssignments(pm, tok, local, vars);
    addVars(pm, vars);
    replace(pm, tok);
}

void ProgramMemoryState::assume(const Token* tok, bool b, bool isEmpty)
{
    ProgramMemory pm = state;
    if (isEmpty)
        pm.setContainerSizeValue(tok, 0, b);
    else
        programMemoryParseCondition(pm, tok, nullptr, settings, b);
    const Token* origin = tok;
    const Token* top = tok->astTop();
    if (top && Token::Match(top->previous(), "for|while ("))
        origin = top->link();
    replace(pm, origin);
}

void ProgramMemoryState::removeModifiedVars(const Token* tok)
{
    state.erase_if([&](const ExprIdToken& e) {
        const Token* start = origins[e.getExpressionId()];
        const Token* expr = e.tok;
        if (!expr || isExpressionChanged(expr, start, tok, settings, true)) {
            origins.erase(e.getExpressionId());
            return true;
        }
        return false;
    });
}

ProgramMemory ProgramMemoryState::get(const Token* tok, const Token* ctx, const ProgramMemory::Map& vars) const
{
    ProgramMemoryState local = *this;
    if (ctx)
        local.addState(ctx, vars);
    const Token* start = previousBeforeAstLeftmostLeaf(tok);
    if (!start)
        start = tok;

    if (!ctx || precedes(start, ctx)) {
        local.removeModifiedVars(start);
        local.addState(start, vars);
    } else {
        local.removeModifiedVars(ctx);
    }
    return local.state;
}

ProgramMemory getProgramMemory(const Token *tok, const ProgramMemory::Map& vars)
{
    ProgramMemory programMemory;
    for (const auto& p:vars) {
        const ValueFlow::Value &value = p.second;
        programMemory.replace(getInitialProgramState(tok, value.tokvalue));
        programMemory.replace(getInitialProgramState(tok, value.condition));
    }
    fillProgramMemoryFromConditions(programMemory, tok, nullptr);
    ProgramMemory state;
    for (const auto& p:vars) {
        const ValueFlow::Value &value = p.second;
        programMemory.setValue(p.first.tok, value);
    }
    state = programMemory;
    fillProgramMemoryFromAssignments(programMemory, tok, state, vars);
    return programMemory;
}

ProgramMemory getProgramMemory(const Token* tok, const Token* expr, const ValueFlow::Value& value, const Settings* settings)
{
    ProgramMemory programMemory;
    programMemory.replace(getInitialProgramState(tok, value.tokvalue));
    programMemory.replace(getInitialProgramState(tok, value.condition));
    fillProgramMemoryFromConditions(programMemory, tok, settings);
    programMemory.setValue(expr, value);
    const ProgramMemory state = programMemory;
    fillProgramMemoryFromAssignments(programMemory, tok, state, {{expr, value}});
    return programMemory;
}

static bool isNumericValue(const ValueFlow::Value& value) {
    return value.isIntValue() || value.isFloatValue();
}

static double asFloat(const ValueFlow::Value& value)
{
    return value.isFloatValue() ? value.floatValue : value.intvalue;
}

static std::string removeAssign(const std::string& assign) {
    return std::string{assign.begin(), assign.end() - 1};
}

struct assign {
    template<class T, class U>
    void operator()(T& x, const U& y) const
    {
        x = y;
    }
};

static bool isIntegralValue(const ValueFlow::Value& value)
{
    return value.isIntValue() || value.isIteratorValue() || value.isSymbolicValue();
}

static ValueFlow::Value evaluate(const std::string& op, const ValueFlow::Value& lhs, const ValueFlow::Value& rhs)
{
    ValueFlow::Value result;
    if (lhs.isImpossible() && rhs.isImpossible())
        return ValueFlow::Value::unknown();
    if (lhs.isImpossible() || rhs.isImpossible()) {
        // noninvertible
        if (contains({"%", "/", "&", "|"}, op))
            return ValueFlow::Value::unknown();
        result.setImpossible();
    }
    if (isNumericValue(lhs) && isNumericValue(rhs)) {
        if (lhs.isFloatValue() || rhs.isFloatValue()) {
            result.valueType = ValueFlow::Value::ValueType::FLOAT;
            bool error = false;
            result.floatValue = calculate(op, asFloat(lhs), asFloat(rhs), &error);
            if (error)
                return ValueFlow::Value::unknown();
            return result;
        }
    }
    // Must be integral types
    if (!isIntegralValue(lhs) && !isIntegralValue(rhs))
        return ValueFlow::Value::unknown();
    // If not the same type then one must be int
    if (lhs.valueType != rhs.valueType && !lhs.isIntValue() && !rhs.isIntValue())
        return ValueFlow::Value::unknown();
    bool compareOp = contains({"==", "!=", "<", ">", ">=", "<="}, op);
    // Comparison must be the same type
    if (compareOp && lhs.valueType != rhs.valueType)
        return ValueFlow::Value::unknown();
    // Only add, subtract, and compare for non-integers
    if (!compareOp && !contains({"+", "-"}, op) && !lhs.isIntValue() && !rhs.isIntValue())
        return ValueFlow::Value::unknown();
    // Both cant be iterators for non-compare
    if (!compareOp && lhs.isIteratorValue() && rhs.isIteratorValue())
        return ValueFlow::Value::unknown();
    // Symbolic values must be in the same ring
    if (lhs.isSymbolicValue() && rhs.isSymbolicValue() && lhs.tokvalue != rhs.tokvalue)
        return ValueFlow::Value::unknown();
    if (!lhs.isIntValue() && !compareOp) {
        result.valueType = lhs.valueType;
        result.tokvalue = lhs.tokvalue;
    } else if (!rhs.isIntValue() && !compareOp) {
        result.valueType = rhs.valueType;
        result.tokvalue = rhs.tokvalue;
    } else {
        result.valueType = ValueFlow::Value::ValueType::INT;
    }
    bool error = false;
    result.intvalue = calculate(op, lhs.intvalue, rhs.intvalue, &error);
    if (error)
        return ValueFlow::Value::unknown();
    if (result.isImpossible()) {
        if ((result.intvalue == 0 && op == "!=") || (result.intvalue != 0 && op == "==")) {
            result.setPossible();
            result.intvalue = !result.intvalue;
        }
    }
    return result;
}

static ValueFlow::Value executeImpl(const Token* expr, ProgramMemory& pm, const Settings* settings)
{
    ValueFlow::Value unknown = ValueFlow::Value::unknown();
    const ValueFlow::Value* value = nullptr;
    if (!expr)
        return unknown;
    else if (expr->hasKnownIntValue() && !expr->isAssignmentOp()) {
        return expr->values().front();
    } else if ((value = expr->getKnownValue(ValueFlow::Value::ValueType::FLOAT)) ||
               (value = expr->getKnownValue(ValueFlow::Value::ValueType::ITERATOR_START)) ||
               (value = expr->getKnownValue(ValueFlow::Value::ValueType::ITERATOR_END)) ||
               (value = expr->getKnownValue(ValueFlow::Value::ValueType::CONTAINER_SIZE))) {
        return *value;
    } else if (expr->isNumber()) {
        if (MathLib::isFloat(expr->str()))
            return unknown;
        return ValueFlow::Value{MathLib::toLongNumber(expr->str())};
    } else if (Token::Match(expr->tokAt(-2), ". %name% (") && astIsContainer(expr->tokAt(-2)->astOperand1())) {
        const Token* containerTok = expr->tokAt(-2)->astOperand1();
        Library::Container::Yield yield = containerTok->valueType()->container->getYield(expr->strAt(-1));
        if (yield == Library::Container::Yield::SIZE) {
            ValueFlow::Value v = execute(containerTok, pm);
            if (!v.isContainerSizeValue())
                return unknown;
            v.valueType = ValueFlow::Value::ValueType::INT;
            return v;
        } else if (yield == Library::Container::Yield::EMPTY) {
            ValueFlow::Value v = execute(containerTok, pm);
            if (!v.isContainerSizeValue())
                return unknown;
            if (v.isImpossible() && v.intvalue == 0)
                return ValueFlow::Value{0};
            else if (!v.isImpossible())
                return ValueFlow::Value{v.intvalue == 0};
        }
    } else if (expr->isAssignmentOp() && expr->astOperand1() && expr->astOperand2() && expr->astOperand1()->exprId() > 0) {
        ValueFlow::Value rhs = execute(expr->astOperand2(), pm);
        if (rhs.isUninitValue())
            return unknown;
        if (expr->str() != "=") {
            if (!pm.hasValue(expr->astOperand1()->exprId()))
                return unknown;
            ValueFlow::Value& lhs = pm.at(expr->astOperand1()->exprId());
            rhs = evaluate(removeAssign(expr->str()), lhs, rhs);
            if (lhs.isIntValue())
                ValueFlow::Value::visitValue(rhs, std::bind(assign{}, std::ref(lhs.intvalue), std::placeholders::_1));
            else if (lhs.isFloatValue())
                ValueFlow::Value::visitValue(rhs, std::bind(assign{}, std::ref(lhs.floatValue), std::placeholders::_1));
            else
                return unknown;
            return lhs;
        } else {
            pm.setValue(expr->astOperand1(), rhs);
            return rhs;
        }
    } else if (expr->str() == "&&" && expr->astOperand1() && expr->astOperand2()) {
        ValueFlow::Value lhs = execute(expr->astOperand1(), pm);
        if (!lhs.isIntValue())
            return unknown;
        if (lhs.intvalue == 0)
            return lhs;
        return execute(expr->astOperand2(), pm);
    } else if (expr->str() == "||" && expr->astOperand1() && expr->astOperand2()) {
        ValueFlow::Value lhs = execute(expr->astOperand1(), pm);
        if (!lhs.isIntValue())
            return unknown;
        if (lhs.intvalue != 0)
            return lhs;
        return execute(expr->astOperand2(), pm);
    } else if (expr->str() == "," && expr->astOperand1() && expr->astOperand2()) {
        execute(expr->astOperand1(), pm);
        return execute(expr->astOperand2(), pm);
    } else if (expr->tokType() == Token::eIncDecOp && expr->astOperand1() && expr->astOperand1()->exprId() != 0) {
        if (!pm.hasValue(expr->astOperand1()->exprId()))
            return unknown;
        ValueFlow::Value& lhs = pm.at(expr->astOperand1()->exprId());
        if (!lhs.isIntValue())
            return unknown;
        // overflow
        if (!lhs.isImpossible() && lhs.intvalue == 0 && expr->str() == "--" && astIsUnsigned(expr->astOperand1()))
            return unknown;

        if (expr->str() == "++")
            lhs.intvalue++;
        else
            lhs.intvalue--;
        return lhs;
    } else if (expr->str() == "[" && expr->astOperand1() && expr->astOperand2()) {
        const Token *tokvalue = nullptr;
        if (!pm.getTokValue(expr->astOperand1()->exprId(), &tokvalue)) {
            auto tokvalue_it = std::find_if(expr->astOperand1()->values().begin(),
                                            expr->astOperand1()->values().end(),
                                            std::mem_fn(&ValueFlow::Value::isTokValue));
            if (tokvalue_it == expr->astOperand1()->values().end()) {
                return unknown;
            }
            tokvalue = tokvalue_it->tokvalue;
        }
        if (!tokvalue || !tokvalue->isLiteral()) {
            return unknown;
        }
        const std::string strValue = tokvalue->strValue();
        ValueFlow::Value rhs = execute(expr->astOperand2(), pm);
        if (!rhs.isIntValue())
            return unknown;
        MathLib::bigint index = rhs.intvalue;
        if (index >= 0 && index < strValue.size())
            return ValueFlow::Value{strValue[index]};
        else if (index == strValue.size())
            return ValueFlow::Value{};
    } else if (Token::Match(expr, "%cop%") && expr->astOperand1() && expr->astOperand2()) {
        ValueFlow::Value lhs = execute(expr->astOperand1(), pm);
        ValueFlow::Value rhs = execute(expr->astOperand2(), pm);
        if (!lhs.isUninitValue() && !rhs.isUninitValue())
            return evaluate(expr->str(), lhs, rhs);
        if (expr->isComparisonOp()) {
            if (rhs.isIntValue()) {
                std::vector<ValueFlow::Value> result =
                    infer(makeIntegralInferModel(), expr->str(), expr->astOperand1()->values(), {rhs});
                if (result.empty() || !result.front().isKnown())
                    return unknown;
                return result.front();
            } else if (lhs.isIntValue()) {
                std::vector<ValueFlow::Value> result =
                    infer(makeIntegralInferModel(), expr->str(), {lhs}, expr->astOperand2()->values());
                if (result.empty() || !result.front().isKnown())
                    return unknown;
                return result.front();
            }
        }
    }
    // Unary ops
    else if (Token::Match(expr, "!|+|-") && expr->astOperand1() && !expr->astOperand2()) {
        ValueFlow::Value lhs = execute(expr->astOperand1(), pm);
        if (!lhs.isIntValue())
            return unknown;
        if (expr->str() == "!")
            lhs.intvalue = !lhs.intvalue;
        if (expr->str() == "-")
            lhs.intvalue = -lhs.intvalue;
        return lhs;
    } else if (expr->str() == "?" && expr->astOperand1() && expr->astOperand2()) {
        ValueFlow::Value cond = execute(expr->astOperand1(), pm);
        if (!cond.isIntValue())
            return unknown;
        const Token* child = expr->astOperand2();
        if (cond.intvalue == 0)
            return execute(child->astOperand2(), pm);
        else
            return execute(child->astOperand1(), pm);
    } else if (expr->str() == "(" && expr->isCast()) {
        if (Token::simpleMatch(expr->previous(), ">") && expr->previous()->link())
            return execute(expr->astOperand2(), pm);
        else
            return execute(expr->astOperand1(), pm);
    }
    if (expr->exprId() > 0 && pm.hasValue(expr->exprId())) {
        ValueFlow::Value result = pm.at(expr->exprId());
        if (result.isImpossible() && result.isIntValue() && result.intvalue == 0 && isUsedAsBool(expr)) {
            result.intvalue = !result.intvalue;
            result.setKnown();
        }
        return result;
    }

    if (Token::Match(expr->previous(), ">|%name% {|(")) {
        const Token* ftok = expr->previous();
        const Function* f = ftok->function();
        // TODO: Evaluate inline functions as well
        if (!f && settings && expr->str() == "(") {
            std::unordered_map<nonneg int, ValueFlow::Value> args;
            int argn = 0;
            for (const Token* tok : getArguments(expr)) {
                ValueFlow::Value result = execute(tok, pm, settings);
                if (!result.isUninitValue())
                    args[argn] = result;
                argn++;
            }
            // strlen is a special builtin
            if (Token::simpleMatch(ftok, "strlen")) {
                if (args.count(0) > 0) {
                    ValueFlow::Value v = args.at(0);
                    if (v.isTokValue() && v.tokvalue->tokType() == Token::eString) {
                        v.valueType = ValueFlow::Value::ValueType::INT;
                        v.intvalue = Token::getStrLength(v.tokvalue);
                        v.tokvalue = nullptr;
                        return v;
                    }
                }
            } else {
                const std::string& returnValue = settings->library.returnValue(ftok);
                if (!returnValue.empty())
                    return evaluateLibraryFunction(args, returnValue, settings);
            }
        }
        // Check if functon modifies argument
        visitAstNodes(expr->astOperand2(), [&](const Token* child) {
            if (child->exprId() > 0 && pm.hasValue(child->exprId())) {
                ValueFlow::Value& v = pm.at(child->exprId());
                if (v.valueType == ValueFlow::Value::ValueType::CONTAINER_SIZE) {
                    if (isContainerSizeChanged(child, settings))
                        v = unknown;
                } else if (v.valueType != ValueFlow::Value::ValueType::UNINIT) {
                    if (isVariableChanged(child, v.indirect, settings, true))
                        v = unknown;
                }
            }
            return ChildrenToVisit::op1_and_op2;
        });
    }

    return unknown;
}

static ValueFlow::Value execute(const Token* expr, ProgramMemory& pm, const Settings* settings)
{
    ValueFlow::Value v = executeImpl(expr, pm, settings);
    if (!v.isUninitValue())
        return v;
    if (!expr)
        return v;
    if (pm.hasValue(expr->exprId()))
        return pm.at(expr->exprId());
    return v;
}

ValueFlow::Value evaluateLibraryFunction(const std::unordered_map<nonneg int, ValueFlow::Value>& args,
                                         const std::string& returnValue,
                                         const Settings* settings)
{
    static std::unordered_map<std::string,
                              std::function<ValueFlow::Value(const std::unordered_map<nonneg int, ValueFlow::Value>& arg)>>
    functions = {};
    if (functions.count(returnValue) == 0) {

        std::unordered_map<nonneg int, const Token*> lookupVarId;
        std::shared_ptr<Token> expr = createTokenFromExpression(returnValue, settings, &lookupVarId);

        functions[returnValue] =
            [lookupVarId, expr, settings](const std::unordered_map<nonneg int, ValueFlow::Value>& xargs) {
            if (!expr)
                return ValueFlow::Value::unknown();
            ProgramMemory pm{};
            for (const auto& p : xargs) {
                auto it = lookupVarId.find(p.first);
                if (it != lookupVarId.end())
                    pm.setValue(it->second, p.second);
            }
            return execute(expr.get(), pm, settings);
        };
    }
    return functions.at(returnValue)(args);
}

void execute(const Token* expr,
             ProgramMemory* const programMemory,
             MathLib::bigint* result,
             bool* error,
             const Settings* settings)
{
    ValueFlow::Value v = execute(expr, *programMemory, settings);
    if (!v.isIntValue() || v.isImpossible()) {
        if (error)
            *error = true;
    } else if (result)
        *result = v.intvalue;
}

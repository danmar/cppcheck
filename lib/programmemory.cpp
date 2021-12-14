
#include "programmemory.h"
#include "astutils.h"
#include "calculate.h"
#include "errortypes.h"
#include "infer.h"
#include "mathlib.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueflow.h"
#include "valueptr.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>

void ProgramMemory::setValue(nonneg int exprid, const ValueFlow::Value& value)
{
    values[exprid] = value;
}
const ValueFlow::Value* ProgramMemory::getValue(nonneg int exprid, bool impossible) const
{
    const ProgramMemory::Map::const_iterator it = values.find(exprid);
    const bool found = it != values.end() && (impossible || !it->second.isImpossible());
    if (found)
        return &it->second;
    else
        return nullptr;
}

bool ProgramMemory::getIntValue(nonneg int exprid, MathLib::bigint* result) const
{
    const ValueFlow::Value* value = getValue(exprid);
    if (value && value->isIntValue()) {
        *result = value->intvalue;
        return true;
    }
    return false;
}

void ProgramMemory::setIntValue(nonneg int exprid, MathLib::bigint value, bool impossible)
{
    ValueFlow::Value v(value);
    if (impossible)
        v.setImpossible();
    values[exprid] = v;
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

void ProgramMemory::setContainerSizeValue(nonneg int exprid, MathLib::bigint value, bool isEqual)
{
    ValueFlow::Value v(value);
    v.valueType = ValueFlow::Value::ValueType::CONTAINER_SIZE;
    if (!isEqual)
        v.valueKind = ValueFlow::Value::ValueKind::Impossible;
    values[exprid] = v;
}

void ProgramMemory::setUnknown(nonneg int exprid)
{
    values[exprid].valueType = ValueFlow::Value::ValueType::UNINIT;
}

bool ProgramMemory::hasValue(nonneg int exprid)
{
    return values.find(exprid) != values.end();
}

void ProgramMemory::swap(ProgramMemory &pm)
{
    values.swap(pm.values);
}

void ProgramMemory::clear()
{
    values.clear();
}

bool ProgramMemory::empty() const
{
    return values.empty();
}

void ProgramMemory::replace(const ProgramMemory &pm)
{
    for (auto&& p : pm.values) {
        values[p.first] = p.second;
    }
}

void ProgramMemory::insert(const ProgramMemory &pm)
{
    for (auto&& p:pm.values)
        values.insert(p);
}

bool conditionIsFalse(const Token *condition, const ProgramMemory &programMemory)
{
    if (!condition)
        return false;
    if (condition->str() == "&&") {
        return conditionIsFalse(condition->astOperand1(), programMemory) ||
               conditionIsFalse(condition->astOperand2(), programMemory);
    }
    ProgramMemory progmem(programMemory);
    MathLib::bigint result = 0;
    bool error = false;
    execute(condition, &progmem, &result, &error);
    return !error && result == 0;
}

bool conditionIsTrue(const Token *condition, const ProgramMemory &programMemory)
{
    if (!condition)
        return false;
    if (condition->str() == "||") {
        return conditionIsTrue(condition->astOperand1(), programMemory) ||
               conditionIsTrue(condition->astOperand2(), programMemory);
    }
    ProgramMemory progmem(programMemory);
    bool error = false;
    MathLib::bigint result = 0;
    execute(condition, &progmem, &result, &error);
    return !error && result == 1;
}

static bool frontIs(const std::vector<MathLib::bigint>& v, bool i)
{
    if (v.empty())
        return false;
    if (v.front())
        return i;
    return !i;
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
        bool impossible = (tok->str() == "==" && !then) || (tok->str() == "!=" && then);
        pm.setIntValue(vartok->exprId(), then ? truevalue.intvalue : falsevalue.intvalue, impossible);
        const Token* containerTok = settings->library.getContainerFromYield(vartok, Library::Container::Yield::SIZE);
        if (containerTok)
            pm.setContainerSizeValue(containerTok->exprId(), then ? truevalue.intvalue : falsevalue.intvalue, !impossible);
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
        pm.setIntValue(tok->exprId(), 0, then);
        const Token* containerTok = settings->library.getContainerFromYield(tok, Library::Container::Yield::EMPTY);
        if (containerTok)
            pm.setContainerSizeValue(containerTok->exprId(), 0, then);
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
                pm.setValue(vartok->exprId(), p.second);
                setvar = true;
            }
            if (!setvar) {
                if (!pm.hasValue(vartok->exprId())) {
                    MathLib::bigint result = 0;
                    bool error = false;
                    execute(valuetok, &pm, &result, &error);
                    if (!error)
                        pm.setIntValue(vartok->exprId(), result);
                    else
                        pm.setUnknown(vartok->exprId());
                }
            }
        } else if (tok2->exprId() > 0 && Token::Match(tok2, ".|(|[|*|%var%") && !pm.hasValue(tok2->exprId()) &&
                   isVariableChanged(tok2, 0, nullptr, true)) {
            pm.setUnknown(tok2->exprId());
        }

        if (tok2->str() == "{") {
            if (indentlevel <= 0) {
                // Keep progressing with anonymous/do scopes
                if (!Token::Match(tok2->previous(), "do|; {"))
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
                    tok2 = cond->astParent()->previous();
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
    for (auto i = pm.values.begin(), last = pm.values.end(); i != last;) {
        if (isVariableChanged(origin, tok, i->first, false, nullptr, true)) {
            i = pm.values.erase(i);
        } else {
            ++i;
        }
    }
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
        for (auto&& p:pm.values)
            origins.insert(std::make_pair(p.first, origin));
    state.insert(pm);
}

void ProgramMemoryState::replace(const ProgramMemory &pm, const Token* origin)
{
    if (origin)
        for (auto&& p:pm.values)
            origins[p.first] = origin;
    state.replace(pm);
}

void ProgramMemoryState::addState(const Token* tok, const ProgramMemory::Map& vars)
{
    ProgramMemory pm = state;
    for (const auto& p:vars) {
        nonneg int exprid = p.first;
        const ValueFlow::Value &value = p.second;
        pm.setValue(exprid, value);
        if (value.varId)
            pm.setIntValue(value.varId, value.varvalue);
    }
    fillProgramMemoryFromConditions(pm, tok, settings);
    ProgramMemory local = pm;
    fillProgramMemoryFromAssignments(pm, tok, local, vars);
    replace(pm, tok);
}

void ProgramMemoryState::assume(const Token* tok, bool b, bool isEmpty)
{
    ProgramMemory pm = state;
    if (isEmpty)
        pm.setContainerSizeValue(tok->exprId(), 0, b);
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
    for (auto i = state.values.begin(), last = state.values.end(); i != last;) {
        const Token* start = origins[i->first];
        const Token* expr = findExpression(start ? start : tok, i->first);
        if (!expr || isExpressionChanged(expr, start, tok, settings, true)) {
            origins.erase(i->first);
            i = state.values.erase(i);
        } else {
            ++i;
        }
    }
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
        nonneg int exprid = p.first;
        const ValueFlow::Value &value = p.second;
        programMemory.setValue(exprid, value);
        if (value.varId)
            programMemory.setIntValue(value.varId, value.varvalue);
    }
    state = programMemory;
    fillProgramMemoryFromAssignments(programMemory, tok, state, vars);
    return programMemory;
}

ProgramMemory getProgramMemory(const Token* tok, nonneg int exprid, const ValueFlow::Value& value, const Settings *settings)
{
    ProgramMemory programMemory;
    programMemory.replace(getInitialProgramState(tok, value.tokvalue));
    programMemory.replace(getInitialProgramState(tok, value.condition));
    fillProgramMemoryFromConditions(programMemory, tok, settings);
    programMemory.setValue(exprid, value);
    if (value.varId)
        programMemory.setIntValue(value.varId, value.varvalue);
    const ProgramMemory state = programMemory;
    fillProgramMemoryFromAssignments(programMemory, tok, state, {{exprid, value}});
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
    result.valueType = ValueFlow::Value::ValueType::INT;
    if (op == "+") {
        if (lhs.isIteratorValue())
            result.valueType = lhs.valueType;
        else if (rhs.isIteratorValue())
            result.valueType = rhs.valueType;
    } else if (lhs.valueType != rhs.valueType) {
        return ValueFlow::Value::unknown();
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

static ValueFlow::Value execute(const Token* expr, ProgramMemory& pm)
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
            ValueFlow::Value& lhs = pm.values.at(expr->astOperand1()->exprId());
            rhs = evaluate(removeAssign(expr->str()), lhs, rhs);
            if (lhs.isIntValue())
                ValueFlow::Value::visitValue(rhs, std::bind(assign{}, std::ref(lhs.intvalue), std::placeholders::_1));
            else if (lhs.isFloatValue())
                ValueFlow::Value::visitValue(rhs, std::bind(assign{}, std::ref(lhs.floatValue), std::placeholders::_1));
            else
                return unknown;
            return lhs;
        } else {
            pm.values[expr->astOperand1()->exprId()] = rhs;
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
    } else if (Token::Match(expr, "++|--") && expr->astOperand1() && expr->astOperand1()->exprId() != 0) {
        if (!pm.hasValue(expr->astOperand1()->exprId()))
            return unknown;
        ValueFlow::Value& lhs = pm.values.at(expr->astOperand1()->exprId());
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
        ValueFlow::Value result = pm.values.at(expr->exprId());
        if (result.isImpossible() && result.isIntValue() && result.intvalue == 0 && isUsedAsBool(expr)) {
            result.intvalue = !result.intvalue;
            result.setKnown();
        }
        return result;
    }

    return unknown;
}

void execute(const Token* expr, ProgramMemory* const programMemory, MathLib::bigint* result, bool* error)
{
    ValueFlow::Value v = execute(expr, *programMemory);
    if (!v.isIntValue() || v.isImpossible()) {
        if (error)
            *error = true;
    } else if (result)
        *result = v.intvalue;
}


#include "programmemory.h"
#include "astutils.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "settings.h"
#include "token.h"
#include "valueflow.h"
#include <algorithm>
#include <cassert>
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

void ProgramMemory::setIntValue(nonneg int exprid, MathLib::bigint value)
{
    values[exprid] = ValueFlow::Value(value);
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
        if (then && !Token::Match(tok, "==|>=|<=|<|>"))
            return;
        if (!then && !Token::Match(tok, "<|>|!="))
            return;
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
        if (!impossible)
            pm.setIntValue(vartok->exprId(), then ? truevalue.intvalue : falsevalue.intvalue);
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
        if (then && !astIsPointer(tok) && !astIsBool(tok))
            return;
        if (endTok && isExpressionChanged(tok, tok->next(), endTok, settings, true))
            return;
        pm.setIntValue(tok->exprId(), then);
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

static PMEvaluateFunction evaluateAsInt(PMEvaluateFunction f, ValueFlow::Value::ValueType t)
{
    return [=](const Token* expr, ProgramMemory* const programMemory, MathLib::bigint* result) -> bool {
        const ValueFlow::Value* value = expr->getKnownValue(t);
        if (!value)
            value = programMemory->getValue(expr->exprId());
        if (value && value->valueType == t)
        {
            *result = value->intvalue;
            return true;
        }
        return f && f(expr, programMemory, result);
    };
}

static std::set<ValueFlow::Value::ValueType> findIteratorTypes(const ProgramMemory& pm)
{
    std::set<ValueFlow::Value::ValueType> result;
    for (auto&& p : pm.values) {
        if (p.second.isIteratorValue())
            result.insert(p.second.valueType);
        if (result.size() == 2)
            break;
    }
    return result;
}

static bool isIterator(const Token* expr)
{
    if (!expr)
        return false;
    if (astIsIterator(expr))
        return true;
    return std::any_of(expr->values().begin(), expr->values().end(), std::mem_fn(&ValueFlow::Value::isIteratorValue));
}

void execute(const Token* expr,
             ProgramMemory* const programMemory,
             MathLib::bigint* result,
             bool* error,
             const PMEvaluateFunction& f)
{
    if (!expr)
        *error = true;

    else if (f && f(expr, programMemory, result))
        *error = false;

    else if (expr->hasKnownIntValue() && !expr->isAssignmentOp()) {
        *result = expr->values().front().intvalue;
    }

    else if (expr->isNumber()) {
        *result = MathLib::toLongNumber(expr->str());
        if (MathLib::isFloat(expr->str()))
            *error = true;
    }

    else if (expr->varId() > 0) {
        if (!programMemory->getIntValue(expr->varId(), result))
            *error = true;
    }

    else if (Token::Match(expr->tokAt(-3), "%var% . %name% (") && astIsContainer(expr->tokAt(-3))) {
        const Token* containerTok = expr->tokAt(-3);
        Library::Container::Yield yield = containerTok->valueType()->container->getYield(expr->strAt(-1));
        if (yield == Library::Container::Yield::SIZE) {
            if (!programMemory->getContainerSizeValue(containerTok->exprId(), result))
                *error = true;
        } else if (yield == Library::Container::Yield::EMPTY) {
            if (!programMemory->getContainerEmptyValue(containerTok->exprId(), result))
                *error = true;
        } else {
            *error = true;
        }
    }

    else if (expr->exprId() > 0 && programMemory->hasValue(expr->exprId())) {
        if (!programMemory->getIntValue(expr->exprId(), result))
            *error = true;
    }

    else if (expr->isComparisonOp()) {
        MathLib::bigint result1(0), result2(0);
        bool error1 = false;
        bool error2 = false;
        execute(expr->astOperand1(), programMemory, &result1, &error1, f);
        execute(expr->astOperand2(), programMemory, &result2, &error2, f);
        if (error1 && error2 && (isIterator(expr->astOperand1()) || isIterator(expr->astOperand2()))) {
            for (ValueFlow::Value::ValueType t : findIteratorTypes(*programMemory)) {
                execute(expr->astOperand1(), programMemory, &result1, &error1, evaluateAsInt(f, t));
                execute(expr->astOperand2(), programMemory, &result2, &error2, evaluateAsInt(f, t));
                if (!error1 && !error2)
                    break;
            }
        }
        if (error1 && error2) {
            *error = true;
        } else if (error1 && !error2) {
            ValueFlow::Value v = inferCondition(expr->str(), expr->astOperand1(), result2);
            *error = !v.isKnown();
            *result = v.intvalue;
        } else if (!error1 && error2) {
            ValueFlow::Value v = inferCondition(expr->str(), result1, expr->astOperand2());
            *error = !v.isKnown();
            *result = v.intvalue;
        } else {
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
    }

    else if (expr->isAssignmentOp()) {
        execute(expr->astOperand2(), programMemory, result, error, f);
        if (!expr->astOperand1() || !expr->astOperand1()->exprId())
            *error = true;
        if (*error)
            return;

        if (expr->str() == "=") {
            programMemory->setIntValue(expr->astOperand1()->exprId(), *result);
            return;
        }

        long long intValue;
        if (!programMemory->getIntValue(expr->astOperand1()->exprId(), &intValue)) {
            *error = true;
            return;
        }
        if (expr->str() == "+=")
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue + *result);
        else if (expr->str() == "-=")
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue - *result);
        else if (expr->str() == "*=")
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue * *result);
        else if (expr->str() == "/=" && *result != 0)
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue / *result);
        else if (expr->str() == "%=" && *result != 0)
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue % *result);
        else if (expr->str() == "&=")
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue & *result);
        else if (expr->str() == "|=")
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue | *result);
        else if (expr->str() == "^=")
            programMemory->setIntValue(expr->astOperand1()->exprId(), intValue ^ *result);
    }

    else if (Token::Match(expr, "++|--")) {
        if (!expr->astOperand1() || expr->astOperand1()->exprId() == 0)
            *error = true;
        else {
            long long intValue;
            if (!programMemory->getIntValue(expr->astOperand1()->exprId(), &intValue))
                *error = true;
            else {
                if (intValue == 0 &&
                    expr->str() == "--" &&
                    expr->astOperand1()->variable() &&
                    expr->astOperand1()->variable()->isUnsigned())
                    *error = true; // overflow
                *result = intValue + (expr->str() == "++" ? 1 : -1);
                programMemory->setIntValue(expr->astOperand1()->exprId(), *result);
            }
        }
    }

    else if (expr->str() == "&&") {
        bool error1 = false;
        execute(expr->astOperand1(), programMemory, result, &error1, f);
        if (!error1 && *result == 0)
            *result = 0;
        else {
            bool error2 = false;
            execute(expr->astOperand2(), programMemory, result, &error2, f);
            if (error1 || error2)
                *error = true;
        }
    }

    else if (expr->str() == "||") {
        execute(expr->astOperand1(), programMemory, result, error, f);
        if (*result == 1 && *error == false)
            *result = 1;
        else if (*result == 0 && *error == false)
            execute(expr->astOperand2(), programMemory, result, error, f);
    } else if (expr->isConstOp() && expr->astOperand1() && expr->astOperand2()) {
        MathLib::bigint result1(0), result2(0);
        execute(expr->astOperand1(), programMemory, &result1, error, f);
        execute(expr->astOperand2(), programMemory, &result2, error, f);
        if (expr->str() == "+")
            *result = result1 + result2;
        else if (expr->str() == "-")
            *result = result1 - result2;
        else if (expr->str() == "*") {
            if (result2 && (result1 > std::numeric_limits<MathLib::bigint>::max()/result2))
                *error = true;
            else
                *result = result1 * result2;
        } else if (result2 == 0)
            *error = true;
        else if (expr->str() == "/")
            *result = result1 / result2;
        else if (expr->str() == "%")
            *result = result1 % result2;
        else if (expr->str() == "<<") {
            if (result2 < 0 || result1 < 0 || result2 >= MathLib::bigint_bits) {  // don't perform UB
                *error= true;
            } else {
                *result = result1 << result2;
            }
        } else if (expr->str() == ">>") {
            if (result2 < 0 || result2 >= MathLib::bigint_bits) { // don't perform UB
                *error=true;
            } else {
                *result = result1 >> result2;
            }
        } else if (expr->str() == "&") {
            *result = result1 & result2;
        } else if (expr->str() == "|") {
            *result = result1 | result2;
        } else {
            *error = true;
        }
    }

    else if (expr->str() == "!") {
        execute(expr->astOperand1(), programMemory, result, error, f);
        *result = !(*result);
    } else if (expr->isUnaryOp("-")) {
        execute(expr->astOperand1(), programMemory, result, error, f);
        *result = -(*result);
    } else if (expr->isUnaryOp("+")) {
        execute(expr->astOperand1(), programMemory, result, error, f);
    } else if (expr->str() == "," && expr->astOperand1() && expr->astOperand2()) {
        execute(expr->astOperand1(), programMemory, result, error, f);
        execute(expr->astOperand2(), programMemory, result, error, f);
    }

    else if (expr->str() == "[" && expr->astOperand1() && expr->astOperand2()) {
        const Token *tokvalue = nullptr;
        if (!programMemory->getTokValue(expr->astOperand1()->exprId(), &tokvalue)) {
            auto tokvalue_it = std::find_if(expr->astOperand1()->values().begin(),
                                            expr->astOperand1()->values().end(),
                                            std::mem_fn(&ValueFlow::Value::isTokValue));
            if (tokvalue_it == expr->astOperand1()->values().end()) {
                *error = true;
                return;
            }
            tokvalue = tokvalue_it->tokvalue;
        }
        if (!tokvalue || !tokvalue->isLiteral()) {
            *error = true;
            return;
        }
        const std::string strValue = tokvalue->strValue();
        MathLib::bigint index = 0;
        execute(expr->astOperand2(), programMemory, &index, error, f);
        if (index >= 0 && index < strValue.size())
            *result = strValue[index];
        else if (index == strValue.size())
            *result = 0;
        else
            *error = true;
    } else if (expr->str() == "?" && expr->astOperand1() && expr->astOperand2()) {
        execute(expr->astOperand1(), programMemory, result, error, f);
        if (*error)
            return;
        const Token* childTok = expr->astOperand2();
        if (*result == 0)
            execute(childTok->astOperand2(), programMemory, result, error, f);
        else
            execute(childTok->astOperand1(), programMemory, result, error, f);

    } else if (expr->str() == "(" && expr->isCast()) {
        if (Token::simpleMatch(expr->previous(), ">") && expr->previous()->link())
            execute(expr->astOperand2(), programMemory, result, error);
        else
            execute(expr->astOperand1(), programMemory, result, error);
    } else
        *error = true;
}

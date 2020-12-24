
#include "programmemory.h"
#include "token.h"
#include "astutils.h"
#include "symboldatabase.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>

void ProgramMemory::setValue(nonneg int varid, const ValueFlow::Value &value)
{
    values[varid] = value;
}
const ValueFlow::Value* ProgramMemory::getValue(nonneg int varid) const
{
    const ProgramMemory::Map::const_iterator it = values.find(varid);
    const bool found = it != values.end() && !it->second.isImpossible();
    if (found)
        return &it->second;
    else
        return nullptr;
}

bool ProgramMemory::getIntValue(nonneg int varid, MathLib::bigint* result) const
{
    const ValueFlow::Value* value = getValue(varid);
    if (value && value->isIntValue()) {
        *result = value->intvalue;
        return true;
    }
    return false;
}

void ProgramMemory::setIntValue(nonneg int varid, MathLib::bigint value)
{
    values[varid] = ValueFlow::Value(value);
}

bool ProgramMemory::getTokValue(nonneg int varid, const Token** result) const
{
    const ValueFlow::Value* value = getValue(varid);
    if (value && value->isTokValue()) {
        *result = value->tokvalue;
        return true;
    }
    return false;
}

bool ProgramMemory::getContainerSizeValue(nonneg int varid, MathLib::bigint* result) const
{
    const ValueFlow::Value* value = getValue(varid);
    if (value && value->isContainerSizeValue()) {
        *result = value->intvalue;
        return true;
    }
    return false;
}

void ProgramMemory::setUnknown(nonneg int varid)
{
    values[varid].valueType = ValueFlow::Value::ValueType::UNINIT;
}

bool ProgramMemory::hasValue(nonneg int varid)
{
    return values.find(varid) != values.end();
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
    for (auto&& p:pm.values)
        values[p.first] = p.second;
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

static void programMemoryParseCondition(ProgramMemory& pm, const Token* tok, const Token* endTok, const Settings* settings, bool then)
{
    if (Token::Match(tok, "==|>=|<=|<|>|!=")) {
        if (then && !Token::Match(tok, "==|>=|<="))
            return;
        if (!then && !Token::Match(tok, "<|>|!="))
            return;
        ValueFlow::Value truevalue;
        ValueFlow::Value falsevalue;
        const Token* vartok = parseCompareInt(tok, truevalue, falsevalue);
        if (!vartok)
            return;
        if (vartok->varId() == 0)
            return;
        if (!truevalue.isIntValue())
            return;
        if (endTok && isVariableChanged(tok->next(), endTok, vartok->varId(), false, settings, true))
            return;
        pm.setIntValue(vartok->varId(),  then ? truevalue.intvalue : falsevalue.intvalue);
    } else if (Token::Match(tok, "%var%")) {
        if (tok->varId() == 0)
            return;
        if (then && !astIsPointer(tok) && !astIsBool(tok))
            return;
        if (endTok && isVariableChanged(tok->next(), endTok, tok->varId(), false, settings, true))
            return;
        pm.setIntValue(tok->varId(), then);
    } else if (Token::simpleMatch(tok, "!")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, !then);
    } else if (then && Token::simpleMatch(tok, "&&")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
    } else if (!then && Token::simpleMatch(tok, "||")) {
        programMemoryParseCondition(pm, tok->astOperand1(), endTok, settings, then);
        programMemoryParseCondition(pm, tok->astOperand2(), endTok, settings, then);
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
    if (scope->type == Scope::eIf || scope->type == Scope::eWhile || scope->type == Scope::eElse) {
        const Token * bodyStart = scope->bodyStart;
        if (scope->type == Scope::eElse) {
            if (!Token::simpleMatch(bodyStart->tokAt(-2), "} else {"))
                return;
            bodyStart = bodyStart->linkAt(-2);
        }
        const Token * condEndTok = bodyStart->previous();
        if (!Token::simpleMatch(condEndTok, ") {"))
            return;
        const Token * condStartTok = condEndTok->link();
        if (!condStartTok)
            return;
        if (!Token::Match(condStartTok->previous(), "if|while ("))
            return;
        const Token * condTok = condStartTok->astOperand2();
        programMemoryParseCondition(pm, condTok, endTok, settings, scope->type != Scope::eElse);
    }
}

static void fillProgramMemoryFromConditions(ProgramMemory& pm, const Token* tok, const Settings* settings)
{
    fillProgramMemoryFromConditions(pm, tok->scope(), tok, settings);
}

static void fillProgramMemoryFromAssignments(ProgramMemory& pm, const Token* tok, const ProgramMemory& state, ProgramMemory::Map vars)
{
    int indentlevel = 0;
    for (const Token *tok2 = tok; tok2; tok2 = tok2->previous()) {
        bool setvar = false;
        if (Token::Match(tok2, "[;{}] %var% = %var% ;")) {
            for (auto&& p:vars) {
                if (p.first != tok2->next()->varId())
                    continue;
                const Token *vartok = tok2->tokAt(3);
                if (vartok == tok)
                    continue;
                pm.setValue(vartok->varId(), p.second);
                setvar = true;
            }
        }
        if (!setvar && (Token::Match(tok2, ";|{|}|%type% %var% =") ||
                        Token::Match(tok2, "[;{}] const| %type% %var% ("))) {
            const Token *vartok = tok2->next();
            while (vartok->next()->isName())
                vartok = vartok->next();
            if (!pm.hasValue(vartok->varId())) {
                MathLib::bigint result = 0;
                bool error = false;
                execute(vartok->next()->astOperand2(), &pm, &result, &error);
                if (!error)
                    pm.setIntValue(vartok->varId(), result);
                else
                    pm.setUnknown(vartok->varId());
            }
        } else if (!setvar && Token::Match(tok2, "%var% !!=") && isVariableChanged(tok2, 0, nullptr, true)) {
            const Token *vartok = tok2;
            if (!pm.hasValue(vartok->varId()))
                pm.setUnknown(vartok->varId());
        }

        if (tok2->str() == "{") {
            if (indentlevel <= 0)
                break;
            --indentlevel;
        }
        if (tok2->str() == "}") {
            const Token *cond = tok2->link();
            cond = Token::simpleMatch(cond->previous(), ") {") ? cond->linkAt(-1) : nullptr;
            if (cond && conditionIsFalse(cond->astOperand2(), state))
                tok2 = cond->previous();
            else if (cond && conditionIsTrue(cond->astOperand2(), state)) {
                ++indentlevel;
                continue;
            } else
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
    ProgramMemory pm;
    fillProgramMemoryFromConditions(pm, tok, nullptr);
    ProgramMemory local;
    for (const auto& p:vars) {
        nonneg int varid = p.first;
        const ValueFlow::Value &value = p.second;
        pm.setValue(varid, value);
        if (value.varId)
            pm.setIntValue(value.varId, value.varvalue);
    }
    local = pm;
    fillProgramMemoryFromAssignments(pm, tok, local, vars);
    replace(pm, tok);
}

void ProgramMemoryState::assume(const Token* tok, bool b)
{
    ProgramMemory pm = state;
    programMemoryParseCondition(pm, tok, nullptr, nullptr, b);
    insert(pm, tok);
}

void ProgramMemoryState::removeModifiedVars(const Token* tok)
{
    for (auto i = state.values.begin(), last = state.values.end(); i != last;) {
        if (isVariableChanged(origins[i->first], tok, i->first, false, nullptr, true)) {
            origins.erase(i->first);
            i = state.values.erase(i);
        } else {
            ++i;
        }
    }
}

ProgramMemory ProgramMemoryState::get(const Token *tok, const ProgramMemory::Map& vars) const
{
    ProgramMemoryState local = *this;
    local.addState(tok, vars);
    local.removeModifiedVars(tok);
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
        nonneg int varid = p.first;
        const ValueFlow::Value &value = p.second;
        programMemory.setValue(varid, value);
        if (value.varId)
            programMemory.setIntValue(value.varId, value.varvalue);
    }
    state = programMemory;
    fillProgramMemoryFromAssignments(programMemory, tok, state, vars);
    return programMemory;
}

ProgramMemory getProgramMemory(const Token *tok, nonneg int varid, const ValueFlow::Value &value)
{
    ProgramMemory programMemory;
    programMemory.replace(getInitialProgramState(tok, value.tokvalue));
    programMemory.replace(getInitialProgramState(tok, value.condition));
    fillProgramMemoryFromConditions(programMemory, tok, nullptr);
    programMemory.setValue(varid, value);
    if (value.varId)
        programMemory.setIntValue(value.varId, value.varvalue);
    const ProgramMemory state = programMemory;
    fillProgramMemoryFromAssignments(programMemory, tok, state, {{varid, value}});
    return programMemory;
}

void execute(const Token *expr,
             ProgramMemory * const programMemory,
             MathLib::bigint *result,
             bool *error)
{
    if (!expr)
        *error = true;

    else if (expr->hasKnownIntValue()) {
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

    else if (expr->isAssignmentOp()) {
        execute(expr->astOperand2(), programMemory, result, error);
        if (!expr->astOperand1() || !expr->astOperand1()->varId())
            *error = true;
        if (*error)
            return;

        if (expr->str() == "=") {
            programMemory->setIntValue(expr->astOperand1()->varId(), *result);
            return;
        }

        long long intValue;
        if (!programMemory->getIntValue(expr->astOperand1()->varId(), &intValue)) {
            *error = true;
            return;
        }
        if (expr->str() == "+=")
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue + *result);
        else if (expr->str() == "-=")
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue - *result);
        else if (expr->str() == "*=")
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue * *result);
        else if (expr->str() == "/=" && *result != 0)
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue / *result);
        else if (expr->str() == "%=" && *result != 0)
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue % *result);
        else if (expr->str() == "&=")
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue & *result);
        else if (expr->str() == "|=")
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue | *result);
        else if (expr->str() == "^=")
            programMemory->setIntValue(expr->astOperand1()->varId(), intValue ^ *result);
    }

    else if (Token::Match(expr, "++|--")) {
        if (!expr->astOperand1() || expr->astOperand1()->varId() == 0U)
            *error = true;
        else {
            long long intValue;
            if (!programMemory->getIntValue(expr->astOperand1()->varId(), &intValue))
                *error = true;
            else {
                if (intValue == 0 &&
                    expr->str() == "--" &&
                    expr->astOperand1()->variable() &&
                    expr->astOperand1()->variable()->isUnsigned())
                    *error = true; // overflow
                *result = intValue + (expr->str() == "++" ? 1 : -1);
                programMemory->setIntValue(expr->astOperand1()->varId(), *result);
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
        else if (expr->str() == "<<")  {
            if (result2 < 0 || result1 < 0 || result2 >= MathLib::bigint_bits)  { // don't perform UB
                *error= true;
            } else {
                *result = result1 << result2;
            }
        } else if (expr->str() == ">>") {
            if (result2 < 0) { // don't perform UB
                *error=true;
            } else {
                *result = result1 >> result2;
            }
        }
    }

    else if (expr->str() == "&&") {
        bool error1 = false;
        execute(expr->astOperand1(), programMemory, result, &error1);
        if (!error1 && *result == 0)
            *result = 0;
        else {
            bool error2 = false;
            execute(expr->astOperand2(), programMemory, result, &error2);
            if (error1 || error2)
                *error = true;
        }
    }

    else if (expr->str() == "||") {
        execute(expr->astOperand1(), programMemory, result, error);
        if (*result == 1 && *error == false)
            *result = 1;
        else if (*result == 0 && *error == false)
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
        const Token *tokvalue = nullptr;
        if (!programMemory->getTokValue(expr->astOperand1()->varId(), &tokvalue)) {
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
        execute(expr->astOperand2(), programMemory, &index, error);
        if (index >= 0 && index < strValue.size())
            *result = strValue[index];
        else if (index == strValue.size())
            *result = 0;
        else
            *error = true;
    } else if (Token::Match(expr->tokAt(-3), "%var% . %name% (")) {
        const Token* containerTok = expr->tokAt(-3);
        if (astIsContainer(containerTok)) {
            Library::Container::Yield yield = containerTok->valueType()->container->getYield(expr->strAt(-1));
            if (yield == Library::Container::Yield::SIZE) {
                if (!programMemory->getContainerSizeValue(containerTok->varId(), result))
                    *error = true;
            } else if (yield == Library::Container::Yield::EMPTY) {
                MathLib::bigint size = 0;
                if (!programMemory->getContainerSizeValue(containerTok->varId(), &size))
                    *error = true;
                *result = (size == 0);
            } else {
                *error = true;
            }
        } else {
            *error = true;
        }
    }

    else
        *error = true;
}

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

#include "bughuntingchecks.h"
#include "astutils.h"
#include "errorlogger.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include <cstring>


static float getKnownFloatValue(const Token *tok, float def)
{
    for (const auto &value: tok->values()) {
        if (value.isKnown() && value.valueType == ValueFlow::Value::ValueType::FLOAT)
            return value.floatValue;
    }
    return def;
}


static void bufferOverflow(const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase)
{
    if (!Token::simpleMatch(tok->astParent(), ","))
        return;

    if (!tok->valueType() || tok->valueType()->pointer != 1 || tok->valueType()->type != ::ValueType::Type::CHAR)
        return;

    int argnr = (tok == tok->astParent()->astOperand1()) ? 0 : 1;
    const Token *ftok = tok->astParent();
    while (Token::simpleMatch(ftok, ",")) {
        ++argnr;
        ftok = ftok->astParent();
    }
    ftok = ftok ? ftok->previous() : nullptr;
    if (!Token::Match(ftok, "%name% ("))
        return;

    int overflowArgument = 0;

    if (const Library::Function *func = dataBase->settings->library.getFunction(ftok)) {
        for (auto argNrChecks: func->argumentChecks) {
            int nr = argNrChecks.first;
            const Library::ArgumentChecks &checks = argNrChecks.second;
            for (const Library::ArgumentChecks::MinSize &minsize: checks.minsizes) {
                if (minsize.type == Library::ArgumentChecks::MinSize::STRLEN && minsize.arg == argnr)
                    overflowArgument = nr;
            }
        }
    }

    if (!overflowArgument)
        return;

    const bool bailout = (value.type == ExprEngine::ValueType::BailoutValue);
    dataBase->reportError(tok,
                          Severity::SeverityType::error,
                          "bughuntingBufferOverflow",
                          "Buffer read/write, when calling '" + ftok->str() +  "' it cannot be determined that " + std::to_string(overflowArgument) + getOrdinalText(overflowArgument) + " argument is not overflowed",
                          CWE(120),
                          false,
                          bailout);
}

static void divByZero(const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase)
{
    if (!tok->astParent() || !std::strchr("/%", tok->astParent()->str()[0]))
        return;
    if (tok->hasKnownIntValue() && tok->getKnownIntValue() != 0)
        return;
    if (tok->isImpossibleIntValue(0))
        return;
    if (value.isUninit() && value.type != ExprEngine::ValueType::BailoutValue)
        return;
    float f = getKnownFloatValue(tok, 0.0f);
    if (f > 0.0f || f < 0.0f)
        return;
    if (value.type == ExprEngine::ValueType::BailoutValue) {
        if (Token::simpleMatch(tok->previous(), "sizeof ("))
            return;
    }
    if (tok->astParent()->astOperand2() == tok && value.isEqual(dataBase, 0)) {
        const char * const id = (tok->valueType() && tok->valueType()->isFloat()) ? "bughuntingDivByZeroFloat" : "bughuntingDivByZero";
        const bool bailout = (value.type == ExprEngine::ValueType::BailoutValue);
        dataBase->reportError(dataBase->settings->clang ? tok : tok->astParent(),
                              Severity::SeverityType::error,
                              id,
                              "There is division, cannot determine that there can't be a division by zero.",
                              CWE(369),
                              false,
                              bailout);
    }
}

#ifdef BUG_HUNTING_INTEGEROVERFLOW
static void integerOverflow(const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase)
{
    if (!tok->isArithmeticalOp() || !tok->valueType() || !tok->valueType()->isIntegral() || tok->valueType()->pointer > 0)
        return;

    const ExprEngine::BinOpResult *b = dynamic_cast<const ExprEngine::BinOpResult *>(&value);
    if (!b)
        return;

    int bits = getIntBitsFromValueType(tok->valueType(), *dataBase->settings);
    if (bits == 0 || bits >= 60)
        return;

    std::string errorMessage;
    if (tok->valueType()->sign == ::ValueType::Sign::SIGNED) {
        MathLib::bigint v = 1LL << (bits - 1);
        if (b->isGreaterThan(dataBase, v-1))
            errorMessage = "greater than " + std::to_string(v - 1);
        if (b->isLessThan(dataBase, -v)) {
            if (!errorMessage.empty())
                errorMessage += " or ";
            errorMessage += "less than " + std::to_string(-v);
        }
    } else {
        MathLib::bigint maxValue = (1LL << bits) - 1;
        if (b->isGreaterThan(dataBase, maxValue))
            errorMessage = "greater than " + std::to_string(maxValue);
        if (b->isLessThan(dataBase, 0)) {
            if (!errorMessage.empty())
                errorMessage += " or ";
            errorMessage += "less than 0";
        }
    }

    if (errorMessage.empty())
        return;


    errorMessage = "There is integer arithmetic, cannot determine that there can't be overflow (if result is " + errorMessage + ").";

    if (tok->valueType()->sign == ::ValueType::Sign::UNSIGNED)
        errorMessage += " Note that unsigned integer overflow is defined and will wrap around.";

    dataBase->reportError(tok, Severity::SeverityType::error, "bughuntingIntegerOverflow", errorMessage, false, value.type == ExprEngine::ValueType::BailoutValue);
}
#endif

/** check if variable is unconditionally assigned */
static bool isVariableAssigned(const Variable *var, const Token *tok, const Token *scopeStart=nullptr)
{
    const Token * const start = scopeStart && precedes(var->nameToken(), scopeStart) ? scopeStart : var->nameToken();

    for (const Token *prev = tok->previous(); prev; prev = prev->previous()) {
        if (!precedes(start, prev))
            break;

        if (prev->str() == "}") {
            if (Token::simpleMatch(prev->link()->tokAt(-2), "} else {")) {
                const Token *elseEnd = prev;
                const Token *elseStart = prev->link();
                const Token *ifEnd = elseStart->tokAt(-2);
                const Token *ifStart = ifEnd->link();
                if (isVariableAssigned(var, ifEnd, ifStart) && isVariableAssigned(var, elseEnd, elseStart)) {
                    return true;
                }
            }
            prev = prev->link();
        }
        if (scopeStart && Token::Match(prev, "return|throw|continue|break"))
            return true;
        if (Token::Match(prev, "%varid% =", var->declarationId())) {
            bool usedInRhs = false;
            visitAstNodes(prev->next()->astOperand2(), [&usedInRhs, var](const Token *tok) {
                if (tok->varId() == var->declarationId()) {
                    usedInRhs = true;
                    return ChildrenToVisit::done;
                }
                return ChildrenToVisit::op1_and_op2;
            });
            if (!usedInRhs)
                return true;
        }
    }
    return false;
}

static void uninit(const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase)
{
    if (!tok->astParent())
        return;

    std::string uninitStructMember;
    if (const auto* structValue = dynamic_cast<const ExprEngine::StructValue*>(&value)) {
        uninitStructMember = structValue->getUninitStructMember();

        // uninitialized struct member => is there data copy of struct..
        if (!uninitStructMember.empty()) {
            if (!Token::Match(tok->astParent(), "[=,(]"))
                return;
        }
    }

    bool uninitData = false;
    if (!value.isUninit() && uninitStructMember.empty()) {
        if (Token::Match(tok->astParent(), "[(,]")) {
            if (const auto* arrayValue = dynamic_cast<const ExprEngine::ArrayValue*>(&value)) {
                uninitData = arrayValue->data.size() >= 1 && arrayValue->data[0].value->isUninit();
            }
        }

        if (!uninitData)
            return;
    }

    // container is not uninitialized
    if (tok->valueType() && tok->valueType()->pointer==0 && tok->valueType()->container)
        return;

    // container element is not uninitialized
    if (tok->str() == "[" &&
        tok->astOperand1() &&
        tok->astOperand1()->valueType() &&
        tok->astOperand1()->valueType()->pointer==0 &&
        tok->astOperand1()->valueType()->container) {
        if (tok->astOperand1()->valueType()->container->stdStringLike)
            return;
        bool pointerType = false;
        for (const Token *typeTok = tok->astOperand1()->valueType()->containerTypeToken; Token::Match(typeTok, "%name%|*|::|<"); typeTok = typeTok->next()) {
            if (typeTok->str() == "<" && typeTok->link())
                typeTok = typeTok->link();
            if (typeTok->str() == "*")
                pointerType = true;
        }
        if (!pointerType)
            return;
    }

    // lhs in assignment
    if (tok->astParent()->str() == "=" && tok == tok->astParent()->astOperand1())
        return;

    // Avoid FP when there is bailout..
    if (value.type == ExprEngine::ValueType::BailoutValue) {
        if (tok->hasKnownValue())
            return;
        if (!tok->variable())
            // FIXME
            return;

        // lhs for scope operator
        if (Token::Match(tok, "%name% ::"))
            return;
        if (tok->astParent()->str() == "::" && tok == tok->astParent()->astOperand1())
            return;

        // Object allocated on the stack
        if (Token::Match(tok, "%var% .") && tok->next()->originalName() != "->")
            return;

        // Assume that stream object is initialized
        if (Token::Match(tok->previous(), "[;{}] %var% <<|>>") && !tok->next()->astParent())
            return;

        // Containers are not uninitialized
        std::vector<const Token *> tokens{tok, tok->astOperand1(), tok->astOperand2()};
        if (Token::Match(tok->previous(), ". %name%"))
            tokens.push_back(tok->previous()->astOperand1());
        for (const Token *t: tokens) {
            if (t && t->valueType() && t->valueType()->pointer == 0 && t->valueType()->container)
                return;
        }

        const Variable *var = tok->variable();
        if (var && var->nameToken() == tok)
            return;
        if (var && !var->isLocal())
            return; // FIXME
        if (var && !var->isPointer()) {
            if (!var->isLocal() || var->isStatic())
                return;
        }
        if (var && (Token::Match(var->nameToken(), "%name% [=:({)]") || Token::Match(var->nameToken(), "%varid% ; %varid% =", var->declarationId())))
            return;
        if (var && var->nameToken() == tok)
            return;

        // Are there unconditional assignment?
        if (var && Token::Match(var->nameToken(), "%varid% ;| %varid%| =", tok->varId()))
            return;

        // Arrays are allocated on the stack
        if (var && Token::Match(tok, "%var% [") && var->isArray())
            return;

        if (tok->variable() && isVariableAssigned(tok->variable(), tok))
            return;
    }

    // Uninitialized function argument
    bool inconclusive = false;
    if (Token::Match(tok->astParent(), "[,(]")) {
        const Token *parent = tok->astParent();
        int count = 0;
        if (Token::simpleMatch(parent, ",")) {
            if (tok == parent->astOperand2())
                count = 1;
            parent = parent->astParent();
            while (Token::simpleMatch(parent, ",")) {
                count++;
                parent = parent->astParent();
            }
        }
        if (Token::simpleMatch(parent, "(") && parent->astOperand1() != tok) {
            if (parent->astOperand1()->function()) {
                const Variable *argvar = parent->astOperand1()->function()->getArgumentVar(count);
                if (argvar && argvar->isReference() && !argvar->isConst())
                    return;
                if (uninitData && argvar && !argvar->isConst()) {
                    if (parent->astOperand1()->function()->hasBody())
                        return;
                    inconclusive = true;
                }
                if (!uninitStructMember.empty() && dataBase->isC() && argvar && !argvar->isConst()) {
                    if (parent->astOperand1()->function()->hasBody())
                        return;
                    inconclusive = true;
                }
            } else if (uninitData) {
                if (dataBase->settings->library.getFunction(parent->astOperand1()))
                    return;
                if (parent->astOperand1()->isKeyword())
                    return;
            }
        } else if (uninitData)
            return;
    }

    if (inconclusive && !dataBase->settings->inconclusive)
        return;

    // Avoid FP for array declaration
    const Token *parent = tok->astParent();
    while (parent && parent->str() == "[")
        parent = parent->astParent();
    if (!parent)
        return;

    const std::string inconclusiveMessage(inconclusive ? ". It is inconclusive if there would be a problem in the function call." : "");

    if (!uninitStructMember.empty()) {
        const std::string symbol = tok->expressionString() + "." + uninitStructMember;
        dataBase->reportError(tok,
                              Severity::SeverityType::error,
                              "bughuntingUninitStructMember",
                              "$symbol:" + symbol + "\nCannot determine that '$symbol' is initialized" + inconclusiveMessage,
                              CWE_USE_OF_UNINITIALIZED_VARIABLE,
                              inconclusive,
                              value.type == ExprEngine::ValueType::BailoutValue);
        return;
    }

    std::string uninitexpr = tok->expressionString();
    if (uninitData)
        uninitexpr += "[0]";

    const std::string symbol = (tok->varId() > 0) ? ("$symbol:" + tok->str() + "\n") : std::string();

    dataBase->reportError(tok,
                          Severity::SeverityType::error,
                          "bughuntingUninit",
                          symbol + "Cannot determine that '" + uninitexpr + "' is initialized" + inconclusiveMessage,
                          CWE_USE_OF_UNINITIALIZED_VARIABLE,
                          inconclusive,
                          value.type == ExprEngine::ValueType::BailoutValue);
}

static void checkFunctionCall(const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase)
{
    if (!Token::Match(tok->astParent(), "[(,]"))
        return;
    const Token *parent = tok->astParent();
    while (Token::simpleMatch(parent, ","))
        parent = parent->astParent();
    if (!parent || parent->str() != "(")
        return;

    int num = 0;
    for (const Token *argTok: getArguments(parent->astOperand1())) {
        --num;
        if (argTok == tok) {
            num = -num;
            break;
        }
    }
    if (num <= 0)
        return;

    if (parent->astOperand1()->function()) {
        const Variable *arg = parent->astOperand1()->function()->getArgumentVar(num - 1);
        if (arg && arg->nameToken()) {
            std::string bad;

            MathLib::bigint low;
            if (arg->nameToken()->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::LOW, &low)) {
                if (!(tok->hasKnownIntValue() && tok->getKnownIntValue() >= low) && value.isLessThan(dataBase, low))
                    bad = "__cppcheck_low__(" + std::to_string(low) + ")";
            }

            MathLib::bigint high;
            if (arg->nameToken()->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::HIGH, &high)) {
                if (!(tok->hasKnownIntValue() && tok->getKnownIntValue() <= high) && value.isGreaterThan(dataBase, high))
                    bad = "__cppcheck_high__(" + std::to_string(high) + ")";
            }

            if (!bad.empty()) {
                dataBase->reportError(tok,
                                      Severity::SeverityType::error,
                                      "bughuntingInvalidArgValue",
                                      "There is function call, cannot determine that " + std::to_string(num) + getOrdinalText(num) + " argument value meets the attribute " + bad,
                                      CWE(0),
                                      false);
                return;
            }
        }
    }

    // Check invalid function argument values..
    for (const Library::InvalidArgValue &invalidArgValue : Library::getInvalidArgValues(dataBase->settings->library.validarg(parent->astOperand1(), num))) {
        bool err = false;
        std::string bad;
        switch (invalidArgValue.type) {
        case Library::InvalidArgValue::eq:
            if (!tok->hasKnownIntValue() || tok->getKnownIntValue() == MathLib::toLongNumber(invalidArgValue.op1))
                err = value.isEqual(dataBase, MathLib::toLongNumber(invalidArgValue.op1));
            bad = "equals " + invalidArgValue.op1;
            break;
        case Library::InvalidArgValue::le:
            if (!tok->hasKnownIntValue() || tok->getKnownIntValue() <= MathLib::toLongNumber(invalidArgValue.op1))
                err = value.isLessThan(dataBase, MathLib::toLongNumber(invalidArgValue.op1) + 1);
            bad = "less equal " + invalidArgValue.op1;
            break;
        case Library::InvalidArgValue::lt:
            if (!tok->hasKnownIntValue() || tok->getKnownIntValue() < MathLib::toLongNumber(invalidArgValue.op1))
                err = value.isLessThan(dataBase, MathLib::toLongNumber(invalidArgValue.op1));
            bad = "less than " + invalidArgValue.op1;
            break;
        case Library::InvalidArgValue::ge:
            if (!tok->hasKnownIntValue() || tok->getKnownIntValue() >= MathLib::toLongNumber(invalidArgValue.op1))
                err = value.isGreaterThan(dataBase, MathLib::toLongNumber(invalidArgValue.op1) - 1);
            bad = "greater equal " + invalidArgValue.op1;
            break;
        case Library::InvalidArgValue::gt:
            if (!tok->hasKnownIntValue() || tok->getKnownIntValue() > MathLib::toLongNumber(invalidArgValue.op1))
                err = value.isGreaterThan(dataBase, MathLib::toLongNumber(invalidArgValue.op1));
            bad = "greater than " + invalidArgValue.op1;
            break;
        case Library::InvalidArgValue::range:
            // TODO
            err = value.isEqual(dataBase, MathLib::toLongNumber(invalidArgValue.op1));
            err |= value.isEqual(dataBase, MathLib::toLongNumber(invalidArgValue.op2));
            bad = "range " + invalidArgValue.op1 + "-" + invalidArgValue.op2;
            break;
        }

        if (err) {
            dataBase->reportError(tok, Severity::SeverityType::error, "bughuntingInvalidArgValue", "There is function call, cannot determine that " + std::to_string(num) + getOrdinalText(num) + " argument value is valid. Bad value: " + bad, CWE(0), false);
            break;
        }
    }

    // Uninitialized function argument..
    if (dataBase->settings->library.isuninitargbad(parent->astOperand1(), num) && dataBase->settings->library.isnullargbad(parent->astOperand1(), num) && value.type == ExprEngine::ValueType::ArrayValue) {
        const ExprEngine::ArrayValue &arrayValue = static_cast<const ExprEngine::ArrayValue &>(value);
        auto index0 = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
        for (const auto &v: arrayValue.read(index0)) {
            if (v.second->isUninit()) {
                dataBase->reportError(tok, Severity::SeverityType::error, "bughuntingUninitArg", "There is function call, cannot determine that " + std::to_string(num) + getOrdinalText(num) + " argument is initialized.", CWE_USE_OF_UNINITIALIZED_VARIABLE, false);
                break;
            }
        }
    }
}

static void checkAssignment(const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase)
{
    if (!Token::simpleMatch(tok->astParent(), "="))
        return;
    const Token *lhs = tok->astParent()->astOperand1();
    while (Token::simpleMatch(lhs, "."))
        lhs = lhs->astOperand2();
    if (!lhs || !lhs->variable() || !lhs->variable()->nameToken())
        return;

    const Token *vartok = lhs->variable()->nameToken();

    MathLib::bigint low;
    if (vartok->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::LOW, &low)) {
        if (value.isLessThan(dataBase, low))
            dataBase->reportError(tok, Severity::SeverityType::error, "bughuntingAssign", "There is assignment, cannot determine that value is greater or equal with " + std::to_string(low), CWE_INCORRECT_CALCULATION, false);
    }

    MathLib::bigint high;
    if (vartok->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::HIGH, &high)) {
        if (value.isGreaterThan(dataBase, high))
            dataBase->reportError(tok, Severity::SeverityType::error, "bughuntingAssign", "There is assignment, cannot determine that value is lower or equal with " + std::to_string(high), CWE_INCORRECT_CALCULATION, false);
    }
}

void addBughuntingChecks(std::vector<ExprEngine::Callback> *callbacks)
{
    callbacks->push_back(bufferOverflow);
    callbacks->push_back(divByZero);
    callbacks->push_back(checkFunctionCall);
    callbacks->push_back(checkAssignment);
#ifdef BUG_HUNTING_INTEGEROVERFLOW
    callbacks->push_back(integerOverflow);
#endif
    callbacks->push_back(uninit);

}


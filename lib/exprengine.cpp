/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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

#include "exprengine.h"
#include "astutils.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenize.h"

#include <memory>
#include <iostream>

std::string ExprEngine::str(int128_t value)
{
    std::ostringstream ostr;
#ifdef __GNUC__
    if (value == (int)value) {
        ostr << (int) value;
        return ostr.str();
    }
    if (value < 0) {
        ostr << "-";
        value = -value;
    }

    uint64_t high = value >> 64;
    uint64_t low = value;
    if (high > 0)
        ostr << "h" << std::hex << high << "l";
    ostr << std::hex << low;
#else
    ostr << value;
#endif
    return ostr.str();
}

static ExprEngine::ValuePtr getValueRangeFromValueType(const std::string &name, const ValueType *vt, const cppcheck::Platform &platform);

namespace {
    class TrackExecution {
    public:
        TrackExecution() : mDataIndex(0) {}
        std::map<const Token *, std::vector<std::string>> map;
        int getNewDataIndex() {
            return mDataIndex++;
        }

        void newValue(const Token *tok, const ExprEngine::ValuePtr& value) {
            if (!tok)
                return;
            if (!value)
                map[tok].push_back(tok->expressionString() + "=TODO_NO_VALUE");
            /*
                        else if (value->name[0] == '$')
                            map[tok].push_back(tok->expressionString() + "=(" + value->name + "," + value->getRange() + ")");
                        else
                            map[tok].push_back(tok->expressionString() + "=" + value->name);
            */
        }

        void state(const Token *tok, const std::string &s) {
            map[tok].push_back(s);
        }

        void print() {
            std::set<std::pair<int,int>> locations;
            for (auto it : map) {
                locations.insert(std::pair<int,int>(it.first->linenr(), it.first->column()));
            }
            for (const std::pair<int,int> &loc : locations) {
                int lineNumber = loc.first;
                int column = loc.second;
                for (auto &it : map) {
                    const Token *tok = it.first;
                    if (lineNumber != tok->linenr())
                        continue;
                    const std::vector<std::string> &dumps = it.second;
                    for (const std::string &dump : dumps)
                        std::cout << lineNumber << ":" << column << ": " << dump << "\n";
                }
            }
        }
    private:
        int mDataIndex;
    };

    class Data {
    public:
        Data(int *symbolValueIndex, const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks, TrackExecution *trackExecution)
            : symbolValueIndex(symbolValueIndex)
            , tokenizer(tokenizer)
            , settings(settings)
            , callbacks(callbacks)
            , mTrackExecution(trackExecution)
            , mDataIndex(trackExecution->getNewDataIndex()) {}
        typedef std::map<nonneg int, std::shared_ptr<ExprEngine::Value>> Memory;
        Memory memory;
        int * const symbolValueIndex;
        const Tokenizer * const tokenizer;
        const Settings * const settings;
        const std::vector<ExprEngine::Callback> &callbacks;

        Data getData(const Token *cond, bool trueData) {
            Data ret(symbolValueIndex, tokenizer, settings, callbacks, mTrackExecution);
            for (Memory::const_iterator mem = memory.cbegin(); mem != memory.cend(); ++mem) {
                ret.memory[mem->first] = mem->second;

                if (cond->isComparisonOp() && cond->astOperand1()->varId() == mem->first && cond->astOperand2()->isNumber()) {
                    const int128_t rhsValue = MathLib::toLongNumber(cond->astOperand2()->str());
                    if (auto intRange = std::dynamic_pointer_cast<ExprEngine::IntRange>(mem->second)) {
                        if (cond->str() == ">") {
                            if (trueData && intRange->minValue <= rhsValue) {
                                auto val = std::make_shared<ExprEngine::IntRange>(getNewSymbolName(), rhsValue + 1, intRange->maxValue);
                                ret.trackAssignment(cond, val);
                                ret.memory[mem->first] = val;
                            } else if (!trueData && intRange->maxValue > rhsValue) {
                                auto val = std::make_shared<ExprEngine::IntRange>(getNewSymbolName(), intRange->minValue, rhsValue);
                                ret.trackAssignment(cond, val);
                                ret.memory[mem->first] = val;
                            }
                        }
                    }
                }
            }
            return ret;
        }

        std::string getNewSymbolName() {
            return "$" + std::to_string(++(*symbolValueIndex));
        }

        std::shared_ptr<ExprEngine::ArrayValue> getArrayValue(const Token *tok) {
            const Memory::iterator it = memory.find(tok->varId());
            if (it != memory.end())
                return std::dynamic_pointer_cast<ExprEngine::ArrayValue>(it->second);
            return std::shared_ptr<ExprEngine::ArrayValue>();
        }

        ExprEngine::ValuePtr getValue(unsigned int varId, const ValueType *valueType, const Token *tok) {
            const Memory::const_iterator it = memory.find(varId);
            if (it != memory.end())
                return it->second;
            if (!valueType)
                return ExprEngine::ValuePtr();
            ExprEngine::ValuePtr value = getValueRangeFromValueType(getNewSymbolName(), valueType, *settings);
            if (value) {
                if (tok)
                    trackAssignment(tok, value);
                memory[varId] = value;
            }
            return value;
        }

        void trackAssignment(const Token *tok, const ExprEngine::ValuePtr& value) {
            return mTrackExecution->newValue(tok, value);
        }

        void trackProgramState(const Token *tok) {
            if (memory.empty())
                return;
            const SymbolDatabase * const symbolDatabase = tokenizer->getSymbolDatabase();
            std::ostringstream s;
            s << "{"; // << dataIndex << ":";
            for (auto mem : memory) {
                ExprEngine::ValuePtr value = mem.second;
                s << " " << symbolDatabase->getVariableFromVarId(mem.first)->name() << "=";
                if (value->name[0] == '$')
                    s << "(" << value->name << "," << value->getRange() << ")";
                else
                    s << value->name;
            }
            s << "}";
            mTrackExecution->state(tok, s.str());
        }
    private:
        TrackExecution * const mTrackExecution;
        const int mDataIndex;
    };
}

void ExprEngine::ArrayValue::assign(const ExprEngine::ValuePtr& index, const ExprEngine::ValuePtr& value)
{
    auto i1 = std::dynamic_pointer_cast<ExprEngine::IntRange>(index);
    if (i1) {
        if (i1->minValue == i1->maxValue && i1->minValue >= 0 && i1->maxValue < data.size())
            data[i1->minValue] = value;
    }
}

ExprEngine::ValuePtr ExprEngine::ArrayValue::read(const ExprEngine::ValuePtr& index)
{
    auto i1 = std::dynamic_pointer_cast<ExprEngine::IntRange>(index);
    if (i1) {
        if (i1->minValue == i1->maxValue && i1->minValue >= 0 && i1->maxValue < data.size())
            return data[i1->minValue];
    }
    return ExprEngine::ValuePtr();
}

std::string ExprEngine::BinOpResult::getRange() const
{
    int128_t minValue, maxValue;
    getRange(&minValue, &maxValue);
    return "[" + str(minValue) + ":" + str(maxValue) + "]";
}

void ExprEngine::BinOpResult::getRange(int128_t *minValue, int128_t *maxValue) const
{
    std::map<ValuePtr, int> valueBit;
    // Assign a bit number for each leaf
    int bit = 0;
    for (ValuePtr v : mLeafs) {
        if (auto intRange = std::dynamic_pointer_cast<IntRange>(v)) {
            if (intRange->minValue == intRange->maxValue) {
                valueBit[v] = 30;
                continue;
            }
        }

        valueBit[v] = bit++;
    }

    if (bit > 24)
        throw std::runtime_error("Internal error: bits");

    for (int test = 0; test < (1 << bit); ++test) {
        int128_t result = evaluate(test, valueBit);
        if (test == 0)
            *minValue = *maxValue = result;
        else if (result < *minValue)
            *minValue = result;
        else if (result > *maxValue)
            *maxValue = result;
    }
}

bool ExprEngine::BinOpResult::isIntValueInRange(int value) const
{
    int128_t minValue, maxValue;
    getRange(&minValue, &maxValue);
    return value >= minValue && value <= maxValue;
}

int128_t ExprEngine::BinOpResult::evaluate(int test, const std::map<ExprEngine::ValuePtr, int> &valueBit) const
{
    const int128_t lhs = evaluateOperand(test, valueBit, op1);
    const int128_t rhs = evaluateOperand(test, valueBit, op2);
    if (binop == "+")
        return lhs + rhs;
    if (binop == "-")
        return lhs - rhs;
    if (binop == "*")
        return lhs * rhs;
    if (binop == "/" && rhs != 0)
        return lhs / rhs;
    if (binop == "%" && rhs != 0)
        return lhs % rhs;
    if (binop == "&")
        return lhs & rhs;
    if (binop == "|")
        return lhs | rhs;
    if (binop == "^")
        return lhs ^ rhs;
    if (binop == "<<")
        return lhs << rhs;
    if (binop == ">>")
        return lhs >> rhs;
    throw std::runtime_error("Internal error: Unhandled operator;" + binop);
}

int128_t ExprEngine::BinOpResult::evaluateOperand(int test, const std::map<ExprEngine::ValuePtr, int> &valueBit, const ExprEngine::ValuePtr& value) const
{
    auto binOpResult = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(value);
    if (binOpResult)
        return binOpResult->evaluate(test, valueBit);

    auto it = valueBit.find(value);
    if (it == valueBit.end())
        throw std::runtime_error("Internal error: valueBit not set properly");

    bool valueType = test & (1 << it->second);
    if (auto intRange = std::dynamic_pointer_cast<IntRange>(value))
        return valueType ? intRange->minValue : intRange->maxValue;
    throw std::runtime_error("Internal error: Unhandled value:" + std::to_string((int)value->type()));
}

// Todo: This is taken from ValueFlow and modified.. we should reuse it
static ExprEngine::ValuePtr getValueRangeFromValueType(const std::string &name, const ValueType *vt, const cppcheck::Platform &platform)
{
    if (!vt || !vt->isIntegral() || vt->pointer)
        return ExprEngine::ValuePtr();

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
        return ExprEngine::ValuePtr();
    };

    if (bits == 1) {
        return std::make_shared<ExprEngine::IntRange>(name, 0, 1);
    } else {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            return std::make_shared<ExprEngine::IntRange>(name, 0, ((int128_t)1 << bits) - 1);
        } else {
            return std::make_shared<ExprEngine::IntRange>(name, -((int128_t)1 << (bits - 1)), ((int128_t)1 << (bits - 1)) - 1);
        }
    }

    return ExprEngine::ValuePtr();
}

static void call(const std::vector<ExprEngine::Callback> &callbacks, const Token *tok, const ExprEngine::ValuePtr &value)
{
    if (value) {
        for (ExprEngine::Callback f : callbacks) {
            f(tok, *value);
        }
    }
}

static ExprEngine::ValuePtr executeExpression(const Token *tok, Data &data);

static ExprEngine::ValuePtr executeReturn(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr retval = executeExpression(tok->astOperand1(), data);
    call(data.callbacks, tok, retval);
    return retval;
}

static ExprEngine::ValuePtr executeAssign(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr rhsValue = executeExpression(tok->astOperand2(), data);
    call(data.callbacks, tok, rhsValue);

    const Token *lhsToken = tok->astOperand1();
    data.trackAssignment(lhsToken, rhsValue);
    if (lhsToken->varId() > 0) {
        data.memory[lhsToken->varId()] = rhsValue;
    } else if (lhsToken->str() == "[") {
        auto arrayValue = data.getArrayValue(lhsToken->astOperand1());
        if (arrayValue) {
            auto indexValue = executeExpression(lhsToken->astOperand2(), data);
            arrayValue->assign(indexValue, rhsValue);
        }
    } else if (lhsToken->isUnaryOp("*")) {
        auto pval = executeExpression(lhsToken->astOperand1(), data);
        if (pval && pval->type() == ExprEngine::ValueType::AddressOfValue) {
            auto val = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(pval);
            if (val)
                data.memory[val->varId] = rhsValue;
        }
    }
    return rhsValue;
}

static ExprEngine::ValuePtr executeFunctionCall(const Token *tok, Data &data)
{
    for (const Token *argtok : getArguments(tok))
        (void)executeExpression(argtok, data);
    auto val = getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
    call(data.callbacks, tok, val);
    return val;
}

static ExprEngine::ValuePtr executeArrayIndex(const Token *tok, Data &data)
{
    auto arrayValue = data.getArrayValue(tok->astOperand1());
    if (arrayValue) {
        auto indexValue = executeExpression(tok->astOperand2(), data);
        auto value = arrayValue->read(indexValue);
        call(data.callbacks, tok, value);
        return value;
    }
    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeDot(const Token *tok, Data &data)
{
    if (!tok->astOperand1() || !tok->astOperand1()->varId())
        return ExprEngine::ValuePtr();
    std::shared_ptr<ExprEngine::StructValue> structValue = std::dynamic_pointer_cast<ExprEngine::StructValue>(data.getValue(tok->astOperand1()->varId(), nullptr, nullptr));
    if (!structValue)
        return ExprEngine::ValuePtr();
    return structValue->getValueOfMember(tok->astOperand2()->str());
}

static ExprEngine::ValuePtr executeBinaryOp(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr v1 = executeExpression(tok->astOperand1(), data);
    ExprEngine::ValuePtr v2 = executeExpression(tok->astOperand2(), data);
    if (v1 && v2) {
        auto result = std::make_shared<ExprEngine::BinOpResult>(tok->str(), v1, v2);
        call(data.callbacks, tok, result);
        return result;
    }
    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeAddressOf(const Token *tok, Data &data)
{
    auto addr = std::make_shared<ExprEngine::AddressOfValue>(data.getNewSymbolName(), tok->astOperand1()->varId());
    call(data.callbacks, tok, addr);
    return addr;
}

static ExprEngine::ValuePtr executeDeref(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr pval = executeExpression(tok->astOperand1(), data);
    if (pval) {
        auto addressOf = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(pval);
        if (addressOf) {
            auto val = data.getValue(addressOf->varId, tok->valueType(), tok);
            call(data.callbacks, tok, val);
            return val;
        }
        auto pointer = std::dynamic_pointer_cast<ExprEngine::PointerValue>(pval);
        if (pointer) {
            auto val = pointer->data;
            call(data.callbacks, tok, val);
            return val;
        }
    }
    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeVariable(const Token *tok, Data &data)
{
    auto val = data.getValue(tok->varId(), tok->valueType(), tok);
    call(data.callbacks, tok, val);
    return val;
}

static ExprEngine::ValuePtr executeNumber(const Token *tok)
{
    int128_t value = MathLib::toLongNumber(tok->str());
    return std::make_shared<ExprEngine::IntRange>(tok->str(), value, value);
}

static ExprEngine::ValuePtr executeExpression(const Token *tok, Data &data)
{
    if (tok->str() == "return")
        return executeReturn(tok, data);

    if (tok->str() == "=")
        return executeAssign(tok, data);

    if (tok->astOperand1() && tok->astOperand2() && tok->str() == "[")
        return executeArrayIndex(tok, data);

    if (tok->str() == "(" && tok->astOperand2())
        return executeFunctionCall(tok, data);

    if (tok->str() == ".")
        return executeDot(tok, data);

    if (tok->astOperand1() && tok->astOperand2())
        return executeBinaryOp(tok, data);

    if (tok->isUnaryOp("&") && Token::Match(tok->astOperand1(), "%var%"))
        return executeAddressOf(tok, data);

    if (tok->isUnaryOp("*"))
        return executeDeref(tok, data);

    if (tok->varId())
        return executeVariable(tok, data);

    if (tok->isNumber())
        return executeNumber(tok);

    return ExprEngine::ValuePtr();
}

static void execute(const Token *start, const Token *end, Data &data)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (tok->str() == ";")
            data.trackProgramState(tok);
        if (tok->variable() && tok->variable()->nameToken() == tok) {
            if (tok->variable()->isArray() && tok->variable()->dimensions().size() == 1 && tok->variable()->dimensions()[0].known) {
                data.memory[tok->varId()] = std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), tok->variable()->dimension(0));
            }
            if (Token::Match(tok, "%name% ["))
                tok = tok->linkAt(1);
        }
        if (!tok->astParent() && (tok->astOperand1() || tok->astOperand2()))
            executeExpression(tok, data);

        if (Token::simpleMatch(tok, "if (")) {
            const Token *cond = tok->next()->astOperand2();
            /*const ExprEngine::ValuePtr condValue =*/ executeExpression(cond,data);
            Data trueData = data.getData(cond, true);
            Data falseData = data.getData(cond, false);
            const Token *thenStart = tok->linkAt(1)->next();
            const Token *thenEnd = thenStart->link();
            execute(thenStart->next(), end, trueData);
            if (Token::simpleMatch(thenEnd, "} else {")) {
                const Token *elseStart = thenEnd->tokAt(2);
                execute(elseStart->next(), end, falseData);
            } else {
                execute(thenEnd->next(), end, falseData);
            }
            return;
        }

        if (Token::simpleMatch(tok, "} else {"))
            tok = tok->linkAt(2);
    }
}

void ExprEngine::executeAllFunctions(const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks)
{
    const SymbolDatabase *symbolDatabase = tokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        executeFunction(functionScope, tokenizer, settings, callbacks);
    }
}

static ExprEngine::ValuePtr createVariableValue(const Variable &var, Data &data);

static ExprEngine::ValuePtr createStructVal(const Scope *structScope, Data &data)
{
    std::shared_ptr<ExprEngine::StructValue> structValue = std::make_shared<ExprEngine::StructValue>(data.getNewSymbolName());
    for (const Variable &member : structScope->varlist) {
        ExprEngine::ValuePtr memberValue = createVariableValue(member, data);
        if (memberValue)
            structValue->member[member.name()] = memberValue;
    }
    return structValue;
}

static ExprEngine::ValuePtr createVariableValue(const Variable &var, Data &data)
{
    if (!var.nameToken() || !var.valueType())
        return ExprEngine::ValuePtr();
    if (var.valueType()->pointer > 0)
        return std::make_shared<ExprEngine::PointerValue>(data.getNewSymbolName(), std::make_shared<ExprEngine::UninitValue>());
    if (var.valueType()->isIntegral())
        return getValueRangeFromValueType(data.getNewSymbolName(), var.valueType(), *data.settings);
    if (var.valueType()->type == ValueType::Type::RECORD)
        return createStructVal(var.valueType()->typeScope, data);
    return ExprEngine::ValuePtr();
}

void ExprEngine::executeFunction(const Scope *functionScope, const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks)
{
    if (!functionScope->bodyStart)
        return;
    const Function *function = functionScope->function;
    if (!function)
        return;

    int symbolValueIndex = 0;
    TrackExecution trackExecution;
    Data data(&symbolValueIndex, tokenizer, settings, callbacks, &trackExecution);

    for (const Variable &arg : function->argumentList) {
        ValuePtr val = createVariableValue(arg, data);
        if (val) {
            data.trackAssignment(arg.nameToken(), val);
            data.memory[arg.declarationId()] = val;
        }
    }

    execute(functionScope->bodyStart, functionScope->bodyEnd, data);

    if (settings->verification) {
        // TODO generate better output!!
        trackExecution.print();
    }
}

void ExprEngine::runChecks(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings)
{
    std::function<void(const Token *, const ExprEngine::Value &)> divByZero = [&](const Token *tok, const ExprEngine::Value &value) {
        if (!Token::simpleMatch(tok->astParent(), "/"))
            return;
        if (tok->astParent()->astOperand2() == tok && value.isIntValueInRange(0)) {
            std::list<const Token*> callstack{tok->astParent()};
            ErrorLogger::ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "verificationDivByZero", "Division by zero", false);
            errorLogger->reportErr(errmsg);
        }
    };

    std::function<void(const Token *, const ExprEngine::Value &)> integerOverflow = [&](const Token *tok, const ExprEngine::Value &value) {
        // Integer overflow..
        if (value.type() != ExprEngine::ValueType::BinOpResult)
            return;
        if (!tok->valueType() || tok->valueType()->pointer != 0 || tok->valueType()->type != ::ValueType::Type::INT)
            return;
        const ExprEngine::BinOpResult &b = static_cast<const ExprEngine::BinOpResult &>(value);
        int128_t minValue, maxValue;
        b.getRange(&minValue, &maxValue);
        if (tok->valueType()->sign == ::ValueType::Sign::UNSIGNED && (minValue < 0 || maxValue >= (1LL << 32))) {
            std::list<const Token*> callstack{tok};
            ErrorLogger::ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::warning, "verificationIntegerOverflow", "Unsigned integer overflow", false);
            errorLogger->reportErr(errmsg);
        }
    };
    std::vector<ExprEngine::Callback> callbacks;
    callbacks.push_back(divByZero);
    callbacks.push_back(integerOverflow);
    ExprEngine::executeAllFunctions(tokenizer, settings, callbacks);
}

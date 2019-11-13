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

#include <cstdlib>
#include <limits>
#include <memory>
#include <iostream>
#ifdef USE_Z3
#include <z3++.h>
#endif

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

        void symbolRange(const Token *tok, ExprEngine::ValuePtr value) {
            if (!tok || !value)
                return;
            const std::string &symbolicExpression = value->getSymbolicExpression();
            if (symbolicExpression[0] != '$')
                return;
            if (mSymbols.find(symbolicExpression) != mSymbols.end())
                return;
            mSymbols.insert(symbolicExpression);
            map[tok].push_back(symbolicExpression + "=" + value->getRange());

        }

        void state(const Token *tok, const std::string &s) {
            map[tok].push_back(s);
        }

        void print(std::ostream &out) {
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
                    if (column != tok->column())
                        continue;
                    const std::vector<std::string> &dumps = it.second;
                    for (const std::string &dump : dumps)
                        out << lineNumber << ":" << column << ": " << dump << "\n";
                }
            }
        }
    private:
        int mDataIndex;
        std::set<std::string> mSymbols;
    };

    class Data : public ExprEngine::DataBase {
    public:
        Data(int *symbolValueIndex, const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks, TrackExecution *trackExecution)
            : DataBase(settings)
            , symbolValueIndex(symbolValueIndex)
            , tokenizer(tokenizer)
            , callbacks(callbacks)
            , mTrackExecution(trackExecution)
            , mDataIndex(trackExecution->getNewDataIndex()) {}
        typedef std::map<nonneg int, std::shared_ptr<ExprEngine::Value>> Memory;
        Memory memory;
        int * const symbolValueIndex;
        const Tokenizer * const tokenizer;
        const std::vector<ExprEngine::Callback> &callbacks;
        std::vector<ExprEngine::ValuePtr> constraints;

        void assignValue(const Token *tok, unsigned int varId, ExprEngine::ValuePtr value) {
            mTrackExecution->symbolRange(tok, value);
            if (value) {
                if (auto arr = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(value)) {
                    mTrackExecution->symbolRange(tok, arr->size);
                    for (const auto &indexAndValue: arr->data)
                        mTrackExecution->symbolRange(tok, indexAndValue.value);
                } else if (auto s = std::dynamic_pointer_cast<ExprEngine::StructValue>(value)) {
                    for (const auto &m: s->member)
                        mTrackExecution->symbolRange(tok, m.second);
                }
            }
            memory[varId] = value;
        }

        void assignStructMember(const Token *tok, ExprEngine::StructValue *structVal, const std::string &memberName, ExprEngine::ValuePtr value) {
            mTrackExecution->symbolRange(tok, value);
            structVal->member[memberName] = value;
        }

        std::string getNewSymbolName() override {
            return "$" + std::to_string(++(*symbolValueIndex));
        }

        std::shared_ptr<ExprEngine::ArrayValue> getArrayValue(const Token *tok) {
            const Memory::iterator it = memory.find(tok->varId());
            if (it != memory.end())
                return std::dynamic_pointer_cast<ExprEngine::ArrayValue>(it->second);
            if (tok->varId() == 0)
                return std::shared_ptr<ExprEngine::ArrayValue>();
            auto val = std::make_shared<ExprEngine::ArrayValue>(this, tok->variable());
            assignValue(tok, tok->varId(), val);
            return val;
        }

        ExprEngine::ValuePtr getValue(unsigned int varId, const ValueType *valueType, const Token *tok) {
            const Memory::const_iterator it = memory.find(varId);
            if (it != memory.end())
                return it->second;
            if (!valueType)
                return ExprEngine::ValuePtr();
            ExprEngine::ValuePtr value = getValueRangeFromValueType(getNewSymbolName(), valueType, *settings);
            if (value) {
                assignValue(tok, varId, value);
            }
            return value;
        }

        void trackProgramState(const Token *tok) {
            if (memory.empty())
                return;
            const SymbolDatabase * const symbolDatabase = tokenizer->getSymbolDatabase();
            std::ostringstream s;
            s << "{"; // << mDataIndex << ":";
            for (auto mem : memory) {
                ExprEngine::ValuePtr value = mem.second;
                const Variable *var = symbolDatabase->getVariableFromVarId(mem.first);
                if (!var)
                    continue;
                s << " " << var->name() << "=";
                if (!value)
                    s << "(null)";
                else if (value->name[0] == '$' && value->getSymbolicExpression() != value->name)
                    s << "(" << value->name << "," << value->getSymbolicExpression() << ")";
                else
                    s << value->name;
            }
            s << "}";
            mTrackExecution->state(tok, s.str());
        }

        ExprEngine::ValuePtr notValue(ExprEngine::ValuePtr v) {
            auto b = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(v);
            if (b) {
                std::string binop;
                if (b->binop == "==")
                    binop = "!=";
                else if (b->binop == "!=")
                    binop = "==";
                else if (b->binop == ">=")
                    binop = "<";
                else if (b->binop == "<=")
                    binop = ">";
                else if (b->binop == ">")
                    binop = "<=";
                else if (b->binop == "<")
                    binop = ">=";
                if (!binop.empty())
                    return std::make_shared<ExprEngine::BinOpResult>(binop, b->op1, b->op2);
            }
            auto zero = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
            return std::make_shared<ExprEngine::BinOpResult>("==", v, zero);
        }

        void addConstraint(ExprEngine::ValuePtr condValue, bool trueCond) {
            if (!condValue)
                return;
            if (trueCond)
                constraints.push_back(condValue);
            else
                constraints.push_back(notValue(condValue));
        }

        void addConstraint(ExprEngine::ValuePtr lhsValue, ExprEngine::ValuePtr rhsValue, bool equals) {
            if (!lhsValue || !rhsValue)
                return;
            constraints.push_back(std::make_shared<ExprEngine::BinOpResult>(equals?"==":"!=", lhsValue, rhsValue));
        }

    private:
        TrackExecution * const mTrackExecution;
        const int mDataIndex;
    };
}

static ExprEngine::ValuePtr simplifyValue(ExprEngine::ValuePtr origValue)
{
    auto b = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(origValue);
    if (!b)
        return origValue;
    if (!b->op1 || !b->op2)
        return origValue;
    auto intRange1 = std::dynamic_pointer_cast<ExprEngine::IntRange>(b->op1);
    auto intRange2 = std::dynamic_pointer_cast<ExprEngine::IntRange>(b->op2);
    if (intRange1 && intRange2 && intRange1->minValue == intRange1->maxValue && intRange2->minValue == intRange2->maxValue) {
        const std::string &binop = b->binop;
        int128_t v;
        if (binop == "+")
            v = intRange1->minValue + intRange2->minValue;
        else if (binop == "-")
            v = intRange1->minValue - intRange2->minValue;
        else if (binop == "*")
            v = intRange1->minValue * intRange2->minValue;
        else if (binop == "/" && intRange2->minValue != 0)
            v = intRange1->minValue / intRange2->minValue;
        else if (binop == "%" && intRange2->minValue != 0)
            v = intRange1->minValue % intRange2->minValue;
        else
            return origValue;
        return std::make_shared<ExprEngine::IntRange>(ExprEngine::str(v), v, v);
    }
    return origValue;
}

static ExprEngine::ValuePtr translateUninitValueToRange(ExprEngine::ValuePtr value, const ::ValueType *valueType, Data &data)
{
    if (!value)
        return value;
    if (value->type == ExprEngine::ValueType::UninitValue) {
        auto rangeValue = getValueRangeFromValueType(data.getNewSymbolName(), valueType, *data.settings);
        if (rangeValue)
            return rangeValue;
    }
    if (auto conditionalValue = std::dynamic_pointer_cast<ExprEngine::ConditionalValue>(value)) {
        if (conditionalValue->values.size() == 1 && conditionalValue->values[0].second && conditionalValue->values[0].second->type == ExprEngine::ValueType::UninitValue) {
            auto rangeValue = getValueRangeFromValueType(data.getNewSymbolName(), valueType, *data.settings);
            if (rangeValue)
                return rangeValue;
        }
    }
    return value;
}

static int128_t truncateInt(int128_t value, int bits, char sign)
{
    value = value & (((int128_t)1 << bits) - 1);
    // Sign extension
    if (sign == 's' && value & (1ULL << (bits - 1)))
        value |= ~(((int128_t)1 << bits) - 1);
    return value;
}

ExprEngine::ArrayValue::ArrayValue(const std::string &name, ExprEngine::ValuePtr size, ExprEngine::ValuePtr value)
    : Value(name, ExprEngine::ValueType::ArrayValue)
    , size(size)
{
    assign(ExprEngine::ValuePtr(), value);
}

ExprEngine::ArrayValue::ArrayValue(DataBase *data, const Variable *var)
    : Value(data->getNewSymbolName(), ExprEngine::ValueType::ArrayValue)
{
    if (var) {
        int sz = 1;
        for (const auto &dim : var->dimensions()) {
            if (!dim.known) {
                sz = -1;
                break;
            }
            sz *= dim.num;
        }
        if (sz >= 1)
            size = std::make_shared<ExprEngine::IntRange>(std::to_string(sz), sz, sz);
    }
    ValuePtr val;
    if (var && !var->isGlobal() && !var->isStatic())
        val = std::make_shared<ExprEngine::UninitValue>();
    else if (var && var->valueType()) {
        ::ValueType vt(*var->valueType());
        vt.pointer = 0;
        val = getValueRangeFromValueType(data->getNewSymbolName(), &vt, *data->settings);
    }
    assign(ExprEngine::ValuePtr(), val);
}

void ExprEngine::ArrayValue::assign(ExprEngine::ValuePtr index, ExprEngine::ValuePtr value)
{
    if (!index)
        data.clear();
    if (value) {
        ExprEngine::ArrayValue::IndexAndValue indexAndValue = {index, value};
        data.push_back(indexAndValue);
    }
}

void ExprEngine::ArrayValue::clear()
{
    data.clear();
    ExprEngine::ArrayValue::IndexAndValue indexAndValue = {
        ExprEngine::ValuePtr(), std::make_shared<ExprEngine::IntRange>("0", 0, 0)
    };
    data.push_back(indexAndValue);
}

static bool isEqual(ExprEngine::ValuePtr v1, ExprEngine::ValuePtr v2)
{
    if (!v1 || !v2)
        return !v1 && !v2;
    return v1->name == v2->name;
}

static bool isNonOverlapping(ExprEngine::ValuePtr v1, ExprEngine::ValuePtr v2)
{
    if (!v1 || !v2)
        return false; // Don't know!
    auto intRange1 = std::dynamic_pointer_cast<ExprEngine::IntRange>(v1);
    auto intRange2 = std::dynamic_pointer_cast<ExprEngine::IntRange>(v2);
    if (intRange1 && intRange2 && (intRange1->minValue > intRange2->maxValue || intRange1->maxValue < intRange2->maxValue))
        return true;
    return false;
}

ExprEngine::ConditionalValue::Vector ExprEngine::ArrayValue::read(ExprEngine::ValuePtr index) const
{
    ExprEngine::ConditionalValue::Vector ret;
    if (!index)
        return ret;
    for (const auto indexAndValue : data) {
        if (isEqual(index, indexAndValue.index))
            ret.clear();
        if (isNonOverlapping(index, indexAndValue.index))
            continue;
        // Array contains string literal data...
        if (!indexAndValue.index && indexAndValue.value->type == ExprEngine::ValueType::StringLiteralValue) {
            auto stringLiteral = std::dynamic_pointer_cast<ExprEngine::StringLiteralValue>(indexAndValue.value);
            if (!stringLiteral) {
                ret.push_back(std::pair<ValuePtr,ValuePtr>(indexAndValue.index, std::make_shared<ExprEngine::IntRange>("", -128, 128)));
                continue;
            }
            if (auto i = std::dynamic_pointer_cast<ExprEngine::IntRange>(index)) {
                if (stringLiteral && i->minValue >= 0 && i->minValue == i->maxValue) {
                    int c = 0;
                    if (i->minValue < stringLiteral->size())
                        c = stringLiteral->string[i->minValue];
                    ret.push_back(std::pair<ValuePtr,ValuePtr>(indexAndValue.index, std::make_shared<ExprEngine::IntRange>(std::to_string(c), c, c)));
                    continue;
                }
            }
            int cmin = 0, cmax = 0;
            for (char c : stringLiteral->string) {
                if (c < cmin)
                    cmin = c;
                else if (c > cmax)
                    cmax = c;
            }
            ret.push_back(std::pair<ValuePtr,ValuePtr>(indexAndValue.index, std::make_shared<ExprEngine::IntRange>("", cmin, cmax)));
            continue;
        }

        // Rename IntRange
        if (auto i = std::dynamic_pointer_cast<ExprEngine::IntRange>(indexAndValue.value)) {
            ret.push_back(std::pair<ValuePtr,ValuePtr>(indexAndValue.index, std::make_shared<ExprEngine::IntRange>(indexAndValue.value->name + ":" + index->name, i->minValue, i->maxValue)));
            continue;
        }

        ret.push_back(std::pair<ValuePtr,ValuePtr>(indexAndValue.index, indexAndValue.value));
    }

    if (ret.size() == 1)
        ret[0].first = ExprEngine::ValuePtr();
    else if (ret.size() == 2 && !ret[0].first) {
        ret[0].first = std::make_shared<ExprEngine::BinOpResult>("!=", index, ret[1].first);
        ret[1].first = std::make_shared<ExprEngine::BinOpResult>("==", index, ret[1].first);
    } else {
        // FIXME!!
        ret.clear();
    }

    return ret;
}

std::string ExprEngine::ConditionalValue::getSymbolicExpression() const
{
    std::ostringstream ostr;
    ostr << "{";
    bool first = true;
    for (auto condvalue : values) {
        ValuePtr cond = condvalue.first;
        ValuePtr value = condvalue.second;

        if (!first)
            ostr << ",";
        first = false;
        ostr << "{"
             << (cond ? cond->getSymbolicExpression() : std::string("(null)"))
             << ","
             << value->getSymbolicExpression()
             << "}";
    }
    ostr << "}";
    return ostr.str();
}

std::string ExprEngine::ArrayValue::getSymbolicExpression() const
{
    std::ostringstream ostr;
    ostr << "size=" << (size ? size->name : std::string("(null)"));
    for (const auto indexAndValue : data) {
        ostr << ",["
             << (!indexAndValue.index ? std::string(":") : indexAndValue.index->name)
             << "]="
             << indexAndValue.value->name;
    }
    return ostr.str();
}

std::string ExprEngine::StructValue::getSymbolicExpression() const
{
    std::ostringstream ostr;
    ostr << "{";
    bool first = true;
    for (const auto& m: member) {
        const std::string &memberName = m.first;
        auto memberValue = m.second;
        if (!first)
            ostr << ",";
        first = false;
        ostr << memberName << "=" << (memberValue ? memberValue->getSymbolicExpression() : std::string("(null)"));
    }
    ostr << "}";
    return ostr.str();
}

std::string ExprEngine::PointerValue::getRange() const
{
    std::string r;
    if (data)
        r = "->" + data->getSymbolicExpression();
    if (null)
        r += std::string(r.empty() ? "" : ",") + "null";
    if (uninitData)
        r += std::string(r.empty() ? "" : ",") + "->?";
    return r;
}

std::string ExprEngine::IntegerTruncation::getSymbolicExpression() const
{
    return sign + std::to_string(bits) + "(" + inputValue->getSymbolicExpression() + ")";
}

#ifdef USE_Z3

struct ExprData {
    typedef std::map<std::string, z3::expr> ValueExpr;
    typedef std::vector<z3::expr> AssertionList;

    z3::context context;
    ValueExpr valueExpr;
    AssertionList assertionList;

    void addAssertions(z3::solver &solver) const {
        for (auto assertExpr : assertionList)
            solver.add(assertExpr);
    }

    z3::expr getExpr(const ExprEngine::BinOpResult *b) {
        auto op1 = getExpr(b->op1);
        auto op2 = getExpr(b->op2);

        if (b->binop == "+")
            return op1 + op2;
        if (b->binop == "-")
            return op1 - op2;
        if (b->binop == "*")
            return op1 * op2;
        if (b->binop == "/")
            return op1 / op2;
        if (b->binop == "%")
            return op1 % op2;
        if (b->binop == "==")
            return op1 == op2;
        if (b->binop == "!=")
            return op1 != op2;
        if (b->binop == ">=")
            return op1 >= op2;
        if (b->binop == "<=")
            return op1 <= op2;
        if (b->binop == ">")
            return op1 > op2;
        if (b->binop == "<")
            return op1 < op2;
        if (b->binop == "&&")
            return op1 && op2;
        if (b->binop == "||")
            return op1 || op2;
        throw std::runtime_error("Internal error: Unhandled operator");
    }

    z3::expr getExpr(ExprEngine::ValuePtr v) {
        if (auto intRange = std::dynamic_pointer_cast<ExprEngine::IntRange>(v)) {
            if (intRange->name[0] != '$')
                return context.int_val(int64_t(intRange->minValue));
            auto it = valueExpr.find(v->name);
            if (it != valueExpr.end())
                return it->second;
            auto e = context.int_const(v->name.c_str());
            valueExpr.emplace(v->name, e);
            if (intRange->maxValue <= INT_MAX)
                assertionList.push_back(e <= int(intRange->maxValue));
            if (intRange->minValue >= INT_MIN)
                assertionList.push_back(e >= int(intRange->minValue));
            return e;
        }

        if (auto b = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(v)) {
            return getExpr(b.get());
        }

        if (auto c = std::dynamic_pointer_cast<ExprEngine::ConditionalValue>(v)) {
            if (c->values.size() == 1)
                return getExpr(c->values[0].second);

            return z3::ite(getExpr(c->values[1].first),
                           getExpr(c->values[1].second),
                           getExpr(c->values[0].second));
        }

        if (auto integerTruncation = std::dynamic_pointer_cast<ExprEngine::IntegerTruncation>(v)) {
            return getExpr(integerTruncation->inputValue);
            //return getExpr(integerTruncation->inputValue) & ((1 << integerTruncation->bits) - 1);
        }

        if (v->type == ExprEngine::ValueType::UninitValue)
            return context.int_val(0);

        throw std::runtime_error("Internal error: Unhandled value type");
    }

    z3::expr getConstraintExpr(ExprEngine::ValuePtr v) {
        if (v->type == ExprEngine::ValueType::IntRange)
            return (getExpr(v) != 0);
        return getExpr(v);
    }
};
#endif

bool ExprEngine::IntRange::isIntValueInRange(DataBase *dataBase, int value) const
{
    if (value < minValue || value > maxValue)
        return false;

    const Data *data = dynamic_cast<Data *>(dataBase);
    if (data->constraints.empty())
        return true;
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.context.int_const(name.c_str());
        exprData.valueExpr.emplace(name, e);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        solver.add(e == value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        //std::cout << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::BinOpResult::isIntValueInRange(ExprEngine::DataBase *dataBase, int value) const
{
#ifdef USE_Z3
    ExprData exprData;
    z3::solver solver(exprData.context);
    z3::expr e = exprData.getExpr(this);
    exprData.addAssertions(solver);
    for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
        solver.add(exprData.getConstraintExpr(constraint));
    solver.add(e == value);
    return solver.check() == z3::sat;
#else
    (void)dataBase;
    (void)value;
    return false;
#endif
}

std::string ExprEngine::BinOpResult::getExpr(ExprEngine::DataBase *dataBase) const
{
#ifdef USE_Z3
    ExprData exprData;
    z3::solver solver(exprData.context);
    z3::expr e = exprData.getExpr(this);
    exprData.addAssertions(solver);
    for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
        solver.add(exprData.getConstraintExpr(constraint));
    solver.add(e);
    std::ostringstream os;
    os << solver;
    return os.str();
#else
    (void)dataBase;
    return "";
#endif
}


// Todo: This is taken from ValueFlow and modified.. we should reuse it
static int getIntBitsFromValueType(const ValueType *vt, const cppcheck::Platform &platform)
{
    if (!vt)
        return 0;

    switch (vt->type) {
    case ValueType::Type::BOOL:
        return 1;
    case ValueType::Type::CHAR:
        return platform.char_bit;
    case ValueType::Type::SHORT:
        return platform.short_bit;
    case ValueType::Type::INT:
        return platform.int_bit;
    case ValueType::Type::LONG:
        return platform.long_bit;
    case ValueType::Type::LONGLONG:
        return platform.long_long_bit;
    default:
        return 0;
    };
}

static ExprEngine::ValuePtr getValueRangeFromValueType(const std::string &name, const ValueType *vt, const cppcheck::Platform &platform)
{
    if (!vt || !(vt->isIntegral() || vt->isFloat()) || vt->pointer)
        return ExprEngine::ValuePtr();

    int bits = getIntBitsFromValueType(vt, platform);
    if (bits == 1) {
        return std::make_shared<ExprEngine::IntRange>(name, 0, 1);
    } else if (bits > 1) {
        if (vt->sign == ValueType::Sign::UNSIGNED) {
            return std::make_shared<ExprEngine::IntRange>(name, 0, ((int128_t)1 << bits) - 1);
        } else {
            return std::make_shared<ExprEngine::IntRange>(name, -((int128_t)1 << (bits - 1)), ((int128_t)1 << (bits - 1)) - 1);
        }
    }

    switch (vt->type) {
    case ValueType::Type::FLOAT:
        return std::make_shared<ExprEngine::FloatRange>(name, std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
    case ValueType::Type::DOUBLE:
        return std::make_shared<ExprEngine::FloatRange>(name, std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
    case ValueType::Type::LONGDOUBLE:
        return std::make_shared<ExprEngine::FloatRange>(name, std::numeric_limits<long double>::min(), std::numeric_limits<long double>::max());
    default:
        return ExprEngine::ValuePtr();
    };
}

static void call(const std::vector<ExprEngine::Callback> &callbacks, const Token *tok, ExprEngine::ValuePtr value, Data *dataBase)
{
    if (value) {
        for (ExprEngine::Callback f : callbacks) {
            f(tok, *value, dataBase);
        }
    }
}

static ExprEngine::ValuePtr executeExpression(const Token *tok, Data &data);

static ExprEngine::ValuePtr executeReturn(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr retval = executeExpression(tok->astOperand1(), data);
    call(data.callbacks, tok, retval, &data);
    return retval;
}

static ExprEngine::ValuePtr truncateValue(ExprEngine::ValuePtr val, const ValueType *valueType, Data &data)
{
    if (!valueType)
        return val;
    if (valueType->pointer != 0)
        return val;
    if (!valueType->isIntegral())
        return val; // TODO

    int bits = getIntBitsFromValueType(valueType, *data.settings);
    if (bits == 0)
        // TODO
        return val;

    if (auto range = std::dynamic_pointer_cast<ExprEngine::IntRange>(val)) {
        if (range->minValue == range->maxValue) {
            int128_t newValue = truncateInt(range->minValue, bits, valueType->sign == ValueType::Sign::SIGNED ? 's' : 'u');
            if (newValue == range->minValue)
                return val;
            return std::make_shared<ExprEngine::IntRange>(ExprEngine::str(newValue), newValue, newValue);
        }
        if (auto typeRange = getValueRangeFromValueType("", valueType, *data.settings)) {
            auto typeIntRange = std::dynamic_pointer_cast<ExprEngine::IntRange>(typeRange);
            if (typeIntRange) {
                if (range->minValue >= typeIntRange->minValue && range->maxValue <= typeIntRange->maxValue)
                    return val;
            }
        }

        return std::make_shared<ExprEngine::IntegerTruncation>(data.getNewSymbolName(), val, bits, valueType->sign == ValueType::Sign::SIGNED ? 's' : 'u');
    }
    // TODO
    return val;
}

static ExprEngine::ValuePtr executeAssign(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr rhsValue = executeExpression(tok->astOperand2(), data);
    ExprEngine::ValuePtr assignValue;
    if (tok->str() == "=")
        assignValue = rhsValue;
    else {
        // "+=" => "+"
        std::string binop(tok->str());
        binop = binop.substr(0, binop.size() - 1);
        ExprEngine::ValuePtr lhsValue = executeExpression(tok->astOperand1(), data);
        assignValue = simplifyValue(std::make_shared<ExprEngine::BinOpResult>(binop, lhsValue, rhsValue));
    }

    const Token *lhsToken = tok->astOperand1();
    assignValue = truncateValue(assignValue, lhsToken->valueType(), data);
    call(data.callbacks, tok, assignValue, &data);

    if (lhsToken->varId() > 0) {
        data.assignValue(lhsToken, lhsToken->varId(), assignValue);
    } else if (lhsToken->str() == "[") {
        auto arrayValue = data.getArrayValue(lhsToken->astOperand1());
        if (arrayValue) {
            // Is it array initialization?
            const Token *arrayInit = lhsToken->astOperand1();
            if (arrayInit && arrayInit->variable() && arrayInit->variable()->nameToken() == arrayInit) {
                if (assignValue->type == ExprEngine::ValueType::StringLiteralValue)
                    arrayValue->assign(ExprEngine::ValuePtr(), assignValue);
            } else {
                auto indexValue = executeExpression(lhsToken->astOperand2(), data);
                arrayValue->assign(indexValue, assignValue);
            }
        }
    } else if (lhsToken->isUnaryOp("*")) {
        auto pval = executeExpression(lhsToken->astOperand1(), data);
        if (pval && pval->type == ExprEngine::ValueType::AddressOfValue) {
            auto val = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(pval);
            if (val)
                data.assignValue(lhsToken, val->varId, assignValue);
        } else if (pval && pval->type == ExprEngine::ValueType::BinOpResult) {
            auto b = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(pval);
            if (b && b->binop == "+") {
                std::shared_ptr<ExprEngine::ArrayValue> arr;
                ExprEngine::ValuePtr offset;
                if (b->op1->type == ExprEngine::ValueType::ArrayValue) {
                    arr = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(b->op1);
                    offset = b->op2;
                } else {
                    arr = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(b->op2);
                    offset = b->op1;
                }
                if (arr && offset) {
                    arr->assign(offset, assignValue);
                }
            }
        }
    } else if (Token::Match(lhsToken, ". %name%")) {
        auto structVal = executeExpression(lhsToken->astOperand1(), data);
        if (structVal && structVal->type == ExprEngine::ValueType::StructValue)
            data.assignStructMember(tok, &*std::static_pointer_cast<ExprEngine::StructValue>(structVal), lhsToken->strAt(1), assignValue);
    }
    return assignValue;
}

static ExprEngine::ValuePtr executeFunctionCall(const Token *tok, Data &data)
{
    std::vector<ExprEngine::ValuePtr> argValues;
    for (const Token *argtok : getArguments(tok)) {
        auto val = executeExpression(argtok, data);
        argValues.push_back(val);
        if (!argtok->valueType() || (argtok->valueType()->constness & 1) == 1)
            continue;
        if (auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(val)) {
            ValueType vt(*argtok->valueType());
            vt.pointer = 0;
            auto anyVal = getValueRangeFromValueType(data.getNewSymbolName(), &vt, *data.settings);
            arrayValue->assign(ExprEngine::ValuePtr(), anyVal);
        } else if (auto addressOf = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(val)) {
            ValueType vt(*argtok->valueType());
            vt.pointer = 0;
            if (vt.isIntegral() && argtok->valueType()->pointer == 1)
                data.assignValue(argtok, addressOf->varId, getValueRangeFromValueType(data.getNewSymbolName(), &vt, *data.settings));
        }
    }

    if (!tok->valueType() && tok->astParent())
        throw std::runtime_error("Expression '" + tok->expressionString() + "' has unknown type!");

    auto val = getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
    call(data.callbacks, tok, val, &data);
    return val;
}

static ExprEngine::ValuePtr executeArrayIndex(const Token *tok, Data &data)
{
    auto arrayValue = data.getArrayValue(tok->astOperand1());
    if (arrayValue) {
        auto indexValue = executeExpression(tok->astOperand2(), data);
        auto conditionalValues = arrayValue->read(indexValue);
        for (auto value: conditionalValues)
            call(data.callbacks, tok, value.second, &data);
        if (conditionalValues.size() == 1 && !conditionalValues[0].first)
            return conditionalValues[0].second;
        return std::make_shared<ExprEngine::ConditionalValue>(data.getNewSymbolName(), conditionalValues);
    }

    // TODO: Pointer value..
    executeExpression(tok->astOperand1(), data);
    executeExpression(tok->astOperand2(), data);

    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeCast(const Token *tok, Data &data)
{
    const Token *expr = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();

    auto val = executeExpression(expr, data);

    if (expr->valueType() && expr->valueType()->type == ::ValueType::Type::VOID && expr->valueType()->pointer > 0) {
        ::ValueType vt(*tok->valueType());
        vt.pointer = 0;
        auto range = getValueRangeFromValueType(data.getNewSymbolName(), &vt, *data.settings);

        if (tok->valueType()->pointer == 0)
            return range;

        bool uninit = false, null = false;
        if (val && val->type == ExprEngine::ValueType::PointerValue) {
            null = std::static_pointer_cast<ExprEngine::PointerValue>(val)->null;
            uninit = std::static_pointer_cast<ExprEngine::PointerValue>(val)->uninitData;
        }

        return std::make_shared<ExprEngine::PointerValue>(data.getNewSymbolName(), range, null, uninit);
    }

    if (val)
        // TODO: Cast this..
        return val;

    return getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
}

static ExprEngine::ValuePtr executeDot(const Token *tok, Data &data)
{
    if (!tok->astOperand1() || !tok->astOperand1()->varId())
        return ExprEngine::ValuePtr();
    std::shared_ptr<ExprEngine::StructValue> structValue = std::dynamic_pointer_cast<ExprEngine::StructValue>(data.getValue(tok->astOperand1()->varId(), nullptr, nullptr));
    if (!structValue) {
        if (tok->originalName() == "->") {
            std::shared_ptr<ExprEngine::PointerValue> pointerValue = std::dynamic_pointer_cast<ExprEngine::PointerValue>(data.getValue(tok->astOperand1()->varId(), nullptr, nullptr));
            if (pointerValue) {
                call(data.callbacks, tok->astOperand1(), pointerValue, &data);
                structValue = std::dynamic_pointer_cast<ExprEngine::StructValue>(pointerValue->data);
            } else {
                call(data.callbacks, tok->astOperand1(), data.getValue(tok->astOperand1()->varId(), nullptr, nullptr), &data);
            }
        }
        if (!structValue)
            return ExprEngine::ValuePtr();
    }
    call(data.callbacks, tok->astOperand1(), structValue, &data);
    return structValue->getValueOfMember(tok->astOperand2()->str());
}

static ExprEngine::ValuePtr executeBinaryOp(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr v1 = executeExpression(tok->astOperand1(), data);
    ExprEngine::ValuePtr v2 = executeExpression(tok->astOperand2(), data);
    if (v1 && v2) {
        auto result = simplifyValue(std::make_shared<ExprEngine::BinOpResult>(tok->str(), v1, v2));
        call(data.callbacks, tok, result, &data);
        return result;
    }
    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeAddressOf(const Token *tok, Data &data)
{
    auto addr = std::make_shared<ExprEngine::AddressOfValue>(data.getNewSymbolName(), tok->astOperand1()->varId());
    call(data.callbacks, tok, addr, &data);
    return addr;
}

static ExprEngine::ValuePtr executeDeref(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr pval = executeExpression(tok->astOperand1(), data);
    if (pval) {
        auto addressOf = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(pval);
        if (addressOf) {
            auto val = data.getValue(addressOf->varId, tok->valueType(), tok);
            call(data.callbacks, tok, val, &data);
            return val;
        }
        auto pointer = std::dynamic_pointer_cast<ExprEngine::PointerValue>(pval);
        if (pointer) {
            auto val = pointer->data;
            call(data.callbacks, tok, val, &data);
            return val;
        }
    }
    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeVariable(const Token *tok, Data &data)
{
    auto val = data.getValue(tok->varId(), tok->valueType(), tok);
    call(data.callbacks, tok, val, &data);
    return val;
}

static ExprEngine::ValuePtr executeKnownMacro(const Token *tok, Data &data)
{
    auto val = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), tok->getKnownIntValue(), tok->getKnownIntValue());
    call(data.callbacks, tok, val, &data);
    return val;
}

static ExprEngine::ValuePtr executeNumber(const Token *tok)
{
    if (tok->valueType()->isFloat()) {
        long double value = MathLib::toDoubleNumber(tok->str());
        return std::make_shared<ExprEngine::FloatRange>(tok->str(), value, value);
    }
    int128_t value = MathLib::toLongNumber(tok->str());
    return std::make_shared<ExprEngine::IntRange>(tok->str(), value, value);
}

static ExprEngine::ValuePtr executeStringLiteral(const Token *tok, Data &data)
{
    std::string s = tok->str();
    return std::make_shared<ExprEngine::StringLiteralValue>(data.getNewSymbolName(), s.substr(1, s.size()-2));
}

static ExprEngine::ValuePtr executeExpression1(const Token *tok, Data &data)
{
    if (tok->str() == "return")
        return executeReturn(tok, data);

    if (tok->isAssignmentOp())
        // TODO: Handle more operators
        return executeAssign(tok, data);

    if (tok->astOperand1() && tok->astOperand2() && tok->str() == "[")
        return executeArrayIndex(tok, data);

    if (tok->str() == "(") {
        if (!tok->isCast())
            return executeFunctionCall(tok, data);
        return executeCast(tok, data);
    }

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

    if (tok->isName() && tok->hasKnownIntValue())
        return executeKnownMacro(tok, data);

    if (tok->isNumber() || tok->tokType() == Token::Type::eChar)
        return executeNumber(tok);

    if (tok->tokType() == Token::Type::eString)
        return executeStringLiteral(tok, data);

    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeExpression(const Token *tok, Data &data)
{
    return translateUninitValueToRange(executeExpression1(tok, data), tok->valueType(), data);
}

static ExprEngine::ValuePtr createVariableValue(const Variable &var, Data &data);

static void execute(const Token *start, const Token *end, Data &data)
{
    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (Token::Match(tok, "[;{}]"))
            data.trackProgramState(tok);

        if (Token::simpleMatch(tok, "while ( 0 ) ;")) {
            tok = tok->tokAt(4);
            continue;
        }

        if (tok->str() == "break") {
            const Scope *scope = tok->scope();
            while (scope->type == Scope::eIf || scope->type == Scope::eElse)
                scope = scope->nestedIn;
            tok = scope->bodyEnd;
        }

        if (Token::simpleMatch(tok, "try"))
            // TODO this is a bailout
            throw std::runtime_error("Unhandled:" + tok->str());

        if (Token::Match(tok, "for|while ("))
            // TODO this is a bailout
            throw std::runtime_error("Unhandled:" + tok->str());

        // Variable declaration..
        if (tok->variable() && tok->variable()->nameToken() == tok) {
            if (Token::Match(tok, "%varid% ; %varid% =", tok->varId())) {
                // if variable is not used in assignment rhs then we do not need to create a "confusing" variable value..
                bool foundInRhs = false;
                visitAstNodes(tok->tokAt(3)->astOperand2(), [&](const Token *rhs) {
                    if (rhs->varId()==tok->varId()) {
                        foundInRhs = true;
                        return ChildrenToVisit::done;
                    }
                    return ChildrenToVisit::op1_and_op2;
                });
                if (!foundInRhs) {
                    tok = tok->tokAt(2);
                    continue;
                }
            }
            if (tok->variable()->isArray()) {
                data.assignValue(tok, tok->varId(), std::make_shared<ExprEngine::ArrayValue>(&data, tok->variable()));
                if (Token::Match(tok, "%name% ["))
                    tok = tok->linkAt(1);
            } else if (Token::Match(tok, "%var% ;"))
                data.assignValue(tok, tok->varId(), createVariableValue(*tok->variable(), data));
        } else if (!tok->astParent() && (tok->astOperand1() || tok->astOperand2())) {
            executeExpression(tok, data);
            if (Token::Match(tok, "throw|return"))
                return;
        }

        else if (Token::simpleMatch(tok, "if (")) {
            const Token *cond = tok->next()->astOperand2(); // TODO: C++17 condition
            const ExprEngine::ValuePtr condValue = executeExpression(cond, data);
            Data ifData(data);
            Data elseData(data);
            ifData.addConstraint(condValue, true);
            elseData.addConstraint(condValue, false);

            const Token *thenStart = tok->linkAt(1)->next();
            const Token *thenEnd = thenStart->link();
            execute(thenStart->next(), end, ifData);
            if (Token::simpleMatch(thenEnd, "} else {")) {
                const Token *elseStart = thenEnd->tokAt(2);
                execute(elseStart->next(), end, elseData);
            } else {
                execute(thenEnd, end, elseData);
            }
            return;
        }

        else if (Token::simpleMatch(tok, "switch (")) {
            auto condValue = executeExpression(tok->next()->astOperand2(), data); // TODO: C++17 condition
            const Token *bodyStart = tok->linkAt(1)->next();
            const Token *bodyEnd = bodyStart->link();
            const Token *defaultStart = nullptr;
            Data defaultData(data);
            for (const Token *tok2 = bodyStart->next(); tok2 != bodyEnd; tok2 = tok2->next()) {
                if (tok2->str() == "{")
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "case %num% :")) {
                    auto caseValue = std::make_shared<ExprEngine::IntRange>(tok2->strAt(1), MathLib::toLongNumber(tok2->strAt(1)), MathLib::toLongNumber(tok2->strAt(1)));
                    Data caseData(data);
                    caseData.addConstraint(condValue, caseValue, true);
                    defaultData.addConstraint(condValue, caseValue, false);
                    execute(tok2->tokAt(2), end, caseData);
                } else if (Token::simpleMatch(tok2, "default :"))
                    defaultStart = tok2;
            }
            execute(defaultStart ? defaultStart : bodyEnd, end, defaultData);
            return;
        }

        if (Token::simpleMatch(tok, "} else {"))
            tok = tok->linkAt(2);
    }
}

void ExprEngine::executeAllFunctions(const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks, std::ostream &trace)
{
    const SymbolDatabase *symbolDatabase = tokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        try {
            executeFunction(functionScope, tokenizer, settings, callbacks, trace);
        } catch (const std::exception &e) {
            // FIXME.. there should not be exceptions
            std::string functionName = functionScope->function->name();
            std::cout << "Verify: Aborted analysis of function '" << functionName << "': " << e.what() << std::endl;
        }
    }
}

static ExprEngine::ValuePtr createStructVal(const Scope *structScope, bool uninitData, Data &data)
{
    if (!structScope)
        return ExprEngine::ValuePtr();
    std::shared_ptr<ExprEngine::StructValue> structValue = std::make_shared<ExprEngine::StructValue>(data.getNewSymbolName());
    auto uninitValue = std::make_shared<ExprEngine::UninitValue>();
    for (const Variable &member : structScope->varlist) {
        if (uninitData) {
            if (member.isPointer()) {
                structValue->member[member.name()] = uninitValue;
                continue;
            }
            if (member.valueType() && member.valueType()->type >= ::ValueType::Type::CHAR) {
                structValue->member[member.name()] = uninitValue;
                continue;
            }
        }
        if (member.valueType() && member.valueType()->isIntegral()) {
            ExprEngine::ValuePtr memberValue = createVariableValue(member, data);
            if (memberValue)
                structValue->member[member.name()] = memberValue;
        }
    }
    return structValue;
}

static ExprEngine::ValuePtr createVariableValue(const Variable &var, Data &data)
{
    if (!var.nameToken())
        return ExprEngine::ValuePtr();
    const ValueType *valueType = var.valueType();
    if (!valueType || valueType->type == ValueType::Type::UNKNOWN_TYPE)
        valueType = var.nameToken()->valueType();
    if (!valueType || valueType->type == ValueType::Type::UNKNOWN_TYPE) {
        // variable with unknown type
        if (var.isLocal() && var.isPointer() && !var.isArray())
            return std::make_shared<ExprEngine::UninitValue>();
        return ExprEngine::ValuePtr();
    }

    if (valueType->pointer > 0) {
        if (var.isLocal())
            return std::make_shared<ExprEngine::UninitValue>();
        ValueType vt(*valueType);
        vt.pointer = 0;
        auto range = getValueRangeFromValueType(data.getNewSymbolName(), &vt, *data.settings);
        return std::make_shared<ExprEngine::PointerValue>(data.getNewSymbolName(), range, true, true);
    }
    if (var.isArray())
        return std::make_shared<ExprEngine::ArrayValue>(&data, &var);
    if (valueType->isIntegral())
        return getValueRangeFromValueType(data.getNewSymbolName(), valueType, *data.settings);
    if (valueType->type == ValueType::Type::RECORD)
        return createStructVal(valueType->typeScope, var.isLocal() && !var.isStatic(), data);
    if (valueType->smartPointerType) {
        auto structValue = createStructVal(valueType->smartPointerType->classScope, var.isLocal() && !var.isStatic(), data);
        return std::make_shared<ExprEngine::PointerValue>(data.getNewSymbolName(), structValue, true, false);
    }
    if (valueType->container && valueType->container->stdStringLike) {
        auto size = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 0, ~0ULL);
        auto value = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), -128, 127);
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), size, value);
    }
    return ExprEngine::ValuePtr();
}

void ExprEngine::executeFunction(const Scope *functionScope, const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks, std::ostream &trace)
{
    if (!functionScope->bodyStart)
        return;
    const Function *function = functionScope->function;
    if (!function)
        return;
    if (functionScope->bodyStart->fileIndex() > 0)
        // TODO.. what about functions in headers?
        return;

    int symbolValueIndex = 0;
    TrackExecution trackExecution;
    Data data(&symbolValueIndex, tokenizer, settings, callbacks, &trackExecution);

    for (const Variable &arg : function->argumentList)
        data.assignValue(functionScope->bodyStart, arg.declarationId(), createVariableValue(arg, data));

    execute(functionScope->bodyStart, functionScope->bodyEnd, data);

    if (settings->debugVerification) {
        // TODO generate better output!!
        trackExecution.print(trace);
    }
}

void ExprEngine::runChecks(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings)
{
    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> divByZero = [=](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
        if (!Token::Match(tok->astParent(), "[/%]"))
            return;
        if (tok->astParent()->astOperand2() == tok && value.isIntValueInRange(dataBase, 0)) {
            std::list<const Token*> callstack{tok->astParent()};
            ErrorLogger::ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "verificationDivByZero", "There is division, cannot determine that there can't be a division by zero.", CWE(369), false);
            errorLogger->reportErr(errmsg);
        }
    };

#ifdef VERIFY_INTEGEROVERFLOW
    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> integerOverflow = [&](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
        if (!tok->isArithmeticalOp() || !tok->valueType() || !tok->valueType()->isIntegral() || tok->valueType()->pointer > 0)
            return;

        const ExprEngine::BinOpResult *b = dynamic_cast<const ExprEngine::BinOpResult *>(&value);
        if (!b)
            return;

        ExprEngine::BinOpResult::IntOrFloatValue minValue,maxValue;
        b->getRange(&minValue, &maxValue);
        if (minValue.isFloat() || maxValue.isFloat())
            return;

        int bits = getIntBitsFromValueType(tok->valueType(), *settings);
        if (bits == 0)
            return;

        if (tok->valueType()->sign == ::ValueType::Sign::SIGNED) {
            int128_t v = (int128_t)1 << (bits - 1);
            if (minValue.intValue >= -v && maxValue.intValue < v)
                return;
        } else {
            int128_t v = (int128_t)1 << bits;
            if (minValue.intValue >= 0 && maxValue.intValue < v)
                return;
        }

        std::string note;
        if (tok->valueType()->sign == ::ValueType::Sign::UNSIGNED)
            note = " Note that unsigned integer overflow is defined and will wrap around.";

        std::list<const Token*> callstack{tok};
        ErrorLogger::ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "verificationIntegerOverflow", "Integer overflow, " + tok->valueType()->str() + " result." + note, false);
        errorLogger->reportErr(errmsg);
    };
#endif

    std::vector<ExprEngine::Callback> callbacks;
    callbacks.push_back(divByZero);
#ifdef VERIFY_INTEGEROVERFLOW
    callbacks.push_back(integerOverflow);
#endif
    ExprEngine::executeAllFunctions(tokenizer, settings, callbacks, std::cout);
}

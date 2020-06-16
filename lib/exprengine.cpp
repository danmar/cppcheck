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

/**
 * @brief This is the ExprEngine component in Cppcheck. Its job is to
 * convert the C/C++ code into expressions that the Z3 prover understands.
 * We can then ask Z3 prover for instance if variable "x" can be 123 and
 * the Z3 prover can tell us that.
 *
 * Overview
 * ========
 *
 * The ExprEngine performs a "abstract execution" of each function.
 *  - ExprEngine performs "forward" analysis only. It starts at the top
 *    of the functions.
 *  - There is a abstract program state `Data::memory`.
 *  - The constraints are stored in the vector `Data::constraints`.
 *
 * Abstract program state
 * ======================
 *
 * The map `Data::memory` contains the abstract values of all variables
 * that are used in the current function scope.
 *
 * Use `--debug-bug-hunting --verbose` to dump out `Data::memory`.
 * Example output:
 * 2:5: { x=$1 y=$2}
 * Explanation:
 * At line 2, column 5: The memory has two variables. Variable x has the
 * value $1. Variable y has the value $2.
 *
 * Different value names:
 *  - Typical abstract value has name that starts with "$". The number is
 *    just a incremented value.
 *  - If a variable has a known value then the concrete value is written.
 *    Example: `{ x=1 }`.
 *  - For an uninitialized value the output says "?". For example: `{ a=? }`
 *  - For buffers the output is something like `{ buf=($3,size=10,[:]=?,[$1]=$2) }`
 *    The first item "$3" is the name of the buffer value.
 *    The second item says that the size of this buffer is 10.
 *    After that comes `[index]=value` items that show what values buffer items have:
 *    `[:]=?` means that all items are uninitialized.
 *    `[$1]=$2` means that the buffer item at index "$1" has value "$2".
 *
 * Abstract execution
 * ==================
 *
 * The function:
 * static void execute(const Token *start, const Token *end, Data &data)
 *
 * Perform abstract execution of the code from `start` to `end`. The
 * `data` is modified during the abstract execution.
 *
 * Each astTop token is executed. From that, operands are executed
 * recursively in the "execute.." functions. The result of an operand is
 * a abstract value.
 *
 * Branches
 * --------
 *
 * Imagine:
 *     code1
 *     if (x > 0)
 *         code2
 *     else
 *         code3
 *     code4
 *
 * When "if" is reached.. the current `data` is branched into `thenData`
 * and `elseData`.
 * For "thenData" a constraint is added: x>0
 * For "elseData" a constraint is added: !(x>0)
 *
 * Then analysis of `thenData` and `elseData` will continue separately,
 * by recursive execution. The "code4" block will be analysed both with
 * `thenData` and `elseData`.
 *
 * Z3
 * ==
 *
 * The ExprEngine will not execute Z3 unless a check wants it to.
 *
 * The abstract values and all their constraints is added to a Z3 solver
 * object and after that Z3 can tell us if some condition can be true.
 *
 * Z3 is a SMT solver:
 * https://en.wikipedia.org/wiki/Satisfiability_modulo_theories
 *
 * In SMT:
 *  - all variables are "constant". A variable can not be changed or assigned.
 *  - There is no "execution". The solver considers all equations simultaneously.
 *
 * Simple example (TestExpr::expr6):
 *
 *     void f(unsigned char x)
 *     {
 *          unsigned char y = 8 - x;\n"
 *          y > 1000;
 *     }
 *
 * If a check wants to know if "y > 1000" can be true, ExprEngine will
 * generate this Z3 input:
 *
 *     (declare-fun $1 () Int)
 *     (assert (and (>= $1 0) (<= $1 255)))
 *     (assert (> (- 8 $1) 1000))
 *
 * A symbol "$1" is created.
 * assert that "$1" is a value 0-255.
 * assert that "8-$1" is greater than 1000.
 *
 * Z3 can now determine if these assertions are possible or not. In this
 * case these assertions are not possible, there is no value for $1 between
 * 0-255 that means that "8-$1" is greater than 1000.
 */

#include "exprengine.h"

#include "astutils.h"
#include "errorlogger.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenize.h"

#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <iostream>
#ifdef USE_Z3
#include <z3++.h>
#include <z3_version.h>
#define GET_VERSION_INT(A,B,C)     ((A) * 10000 + (B) * 100 + (C))
#define Z3_VERSION_INT             GET_VERSION_INT(Z3_MAJOR_VERSION, Z3_MINOR_VERSION, Z3_BUILD_NUMBER)
#endif

namespace {
    struct BugHuntingException {
        BugHuntingException(const Token *tok, const std::string &what) : tok(tok), what(what) {}
        const Token *tok;
        const std::string what;
    };
}

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
        TrackExecution() : mDataIndex(0), mAbortLine(-1) {}

        int getNewDataIndex() {
            return mDataIndex++;
        }

        void symbolRange(const Token *tok, ExprEngine::ValuePtr value) {
            if (!tok || !value)
                return;
            if (tok->index() == 0)
                return;
            const std::string &symbolicExpression = value->getSymbolicExpression();
            if (symbolicExpression[0] != '$')
                return;
            if (mSymbols.find(symbolicExpression) != mSymbols.end())
                return;
            mSymbols.insert(symbolicExpression);
            mMap[tok].push_back(symbolicExpression + "=" + value->getRange());
        }

        void state(const Token *tok, const std::string &s) {
            mMap[tok].push_back(s);
        }

        void print(std::ostream &out) {
            std::set<std::pair<int,int>> locations;
            for (auto it : mMap) {
                locations.insert(std::pair<int,int>(it.first->linenr(), it.first->column()));
            }
            for (const std::pair<int,int> &loc : locations) {
                int lineNumber = loc.first;
                int column = loc.second;
                for (auto &it : mMap) {
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

        void report(std::ostream &out, const Scope *functionScope) {
            int linenr = -1;
            std::string code;
            for (const Token *tok = functionScope->bodyStart->next(); tok != functionScope->bodyEnd; tok = tok->next()) {
                if (tok->linenr() > linenr) {
                    if (!code.empty())
                        out << getStatus(linenr) << " " << code << std::endl;
                    linenr = tok->linenr();
                    code.clear();
                }
                code += " " + tok->str();
            }

            out << getStatus(linenr) << " " << code << std::endl;
        }

        void setAbortLine(int linenr) {
            if (linenr > 0 && (mAbortLine == -1 || linenr < mAbortLine))
                mAbortLine = linenr;
        }

        void addError(int linenr) {
            mErrors.insert(linenr);
        }

        bool isAllOk() const {
            return mErrors.empty();
        }

        void addMissingContract(const std::string &f) {
            mMissingContracts.insert(f);
        }

        const std::set<std::string> getMissingContracts() const {
            return mMissingContracts;
        }
    private:
        const char *getStatus(int linenr) const {
            if (mErrors.find(linenr) != mErrors.end())
                return "ERROR";
            if (mAbortLine > 0 && linenr >= mAbortLine)
                return "--";
            return "ok";
        }

        std::map<const Token *, std::vector<std::string>> mMap;

        int mDataIndex;
        int mAbortLine;
        std::set<std::string> mSymbols;
        std::set<int> mErrors;
        std::set<std::string> mMissingContracts;
    };

    class Data : public ExprEngine::DataBase {
    public:
        Data(int *symbolValueIndex, ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings, const std::string &currentFunction, const std::vector<ExprEngine::Callback> &callbacks, TrackExecution *trackExecution)
            : DataBase(currentFunction, settings)
            , symbolValueIndex(symbolValueIndex)
            , errorLogger(errorLogger)
            , tokenizer(tokenizer)
            , callbacks(callbacks)
            , mTrackExecution(trackExecution)
            , mDataIndex(trackExecution->getNewDataIndex()) {}
        typedef std::map<nonneg int, ExprEngine::ValuePtr> Memory;
        Memory memory;
        int * const symbolValueIndex;
        ErrorLogger *errorLogger;
        const Tokenizer * const tokenizer;
        const std::vector<ExprEngine::Callback> &callbacks;
        std::vector<ExprEngine::ValuePtr> constraints;

        ExprEngine::ValuePtr executeContract(const Function *function, ExprEngine::ValuePtr(*executeExpression)(const Token*, Data&)) {
            const auto it = settings->functionContracts.find(function->fullName());
            if (it == settings->functionContracts.end())
                return ExprEngine::ValuePtr();
            const std::string &expects = it->second;
            TokenList tokenList(settings);
            std::istringstream istr(expects);
            tokenList.createTokens(istr);
            tokenList.createAst();
            SymbolDatabase *symbolDatabase = const_cast<SymbolDatabase*>(tokenizer->getSymbolDatabase());
            for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
                for (const Variable &arg: function->argumentList) {
                    if (arg.name() == tok->str()) {
                        tok->variable(&arg);
                        tok->varId(arg.declarationId());
                    }
                }
            }
            symbolDatabase->setValueTypeInTokenList(false, tokenList.front());
            return executeExpression(tokenList.front()->astTop(), *this);
        }

        void contractConstraints(const Function *function, ExprEngine::ValuePtr(*executeExpression)(const Token*, Data&)) {
            auto value = executeContract(function, executeExpression);
            if (value)
                constraints.push_back(value);
        }

        void addError(int linenr) OVERRIDE {
            mTrackExecution->addError(linenr);
        }

        void assignValue(const Token *tok, unsigned int varId, ExprEngine::ValuePtr value) {
            if (varId == 0)
                return;
            mTrackExecution->symbolRange(tok, value);
            if (value) {
                if (auto arr = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(value)) {
                    for (const auto &dim: arr->size)
                        mTrackExecution->symbolRange(tok, dim);
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

        void functionCall() {
            // Remove values for global variables
            const SymbolDatabase *symbolDatabase = tokenizer->getSymbolDatabase();
            for (std::map<nonneg int, ExprEngine::ValuePtr>::iterator it = memory.begin(); it != memory.end();) {
                unsigned int varid = it->first;
                const Variable *var = symbolDatabase->getVariableFromVarId(varid);
                if (var && var->isGlobal())
                    it = memory.erase(it);
                else
                    ++it;
            }
        }

        std::string getNewSymbolName() OVERRIDE {
            return "$" + std::to_string(++(*symbolValueIndex));
        }

        std::shared_ptr<ExprEngine::ArrayValue> getArrayValue(const Token *tok) {
            const Memory::iterator it = memory.find(tok->varId());
            if (it != memory.end())
                return std::dynamic_pointer_cast<ExprEngine::ArrayValue>(it->second);
            if (tok->varId() == 0 || !tok->variable())
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
                if (tok->variable() && tok->variable()->nameToken())
                    addConstraints(value, tok->variable()->nameToken());
                assignValue(tok, varId, value);
            }
            return value;
        }

        void trackCheckContract(const Token *tok, const std::string &solverOutput) {
            std::ostringstream os;
            os << "checkContract:{\n";

            std::string line;
            std::istringstream istr(solverOutput);
            while (std::getline(istr, line))
                os << "        " << line << "\n";

            os << "}";

            mTrackExecution->state(tok, os.str());
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

        void addMissingContract(const std::string &f) {
            mTrackExecution->addMissingContract(f);
        }

        const std::set<std::string> getMissingContracts() const {
            return mTrackExecution->getMissingContracts();
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

        void addConstraints(ExprEngine::ValuePtr value, const Token *tok) {
            MathLib::bigint low;
            if (tok->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::LOW, &low))
                addConstraint(std::make_shared<ExprEngine::BinOpResult>(">=", value, std::make_shared<ExprEngine::IntRange>(std::to_string(low), low, low)), true);

            MathLib::bigint high;
            if (tok->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::HIGH, &high))
                addConstraint(std::make_shared<ExprEngine::BinOpResult>("<=", value, std::make_shared<ExprEngine::IntRange>(std::to_string(high), high, high)), true);
        }

    private:
        TrackExecution * const mTrackExecution;
        const int mDataIndex;
    };
}

#ifdef __clang__
// work around "undefined reference to `__muloti4'" linker error - see https://bugs.llvm.org/show_bug.cgi?id=16404
__attribute__((no_sanitize("undefined")))
#endif
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

ExprEngine::ArrayValue::ArrayValue(const std::string &name, ExprEngine::ValuePtr size, ExprEngine::ValuePtr value, bool pointer, bool nullPointer, bool uninitPointer)
    : Value(name, ExprEngine::ValueType::ArrayValue)
    , pointer(pointer), nullPointer(nullPointer), uninitPointer(uninitPointer)
    , size{size}
{
    assign(ExprEngine::ValuePtr(), value);
}

ExprEngine::ArrayValue::ArrayValue(DataBase *data, const Variable *var)
    : Value(data->getNewSymbolName(), ExprEngine::ValueType::ArrayValue)
    , pointer(var->isPointer()), nullPointer(var->isPointer()), uninitPointer(var->isPointer())
{
    if (var) {
        for (const auto &dim : var->dimensions()) {
            if (dim.known)
                size.push_back(std::make_shared<ExprEngine::IntRange>(std::to_string(dim.num), dim.num, dim.num));
            else
                size.push_back(std::make_shared<ExprEngine::IntRange>(data->getNewSymbolName(), 1, ExprEngine::ArrayValue::MAXSIZE));
        }
    } else {
        size.push_back(std::make_shared<ExprEngine::IntRange>(data->getNewSymbolName(), 1, ExprEngine::ArrayValue::MAXSIZE));
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

std::string ExprEngine::ArrayValue::getRange() const
{
    std::string r = getSymbolicExpression();
    if (nullPointer)
        r += std::string(r.empty() ? "" : ",") + "null";
    if (uninitPointer)
        r += std::string(r.empty() ? "" : ",") + "->?";
    return r;
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
    for (const auto &indexAndValue : data) {
        if (::isEqual(index, indexAndValue.index))
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
                if (i->minValue >= 0 && i->minValue == i->maxValue) {
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
    if (size.empty())
        ostr << "(null)";
    else {
        for (const auto &dim: size)
            ostr << "[" << (dim ? dim->name : std::string("(null)")) << "]";
    }
    for (const auto &indexAndValue : data) {
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

    z3::expr addInt(const std::string &name, int128_t minValue, int128_t maxValue) {
        z3::expr e = context.int_const(name.c_str());
        valueExpr.emplace(name, e);
        if (minValue >= INT_MIN && maxValue <= INT_MAX)
            assertionList.push_back(e >= int(minValue) && e <= int(maxValue));
        else if (maxValue <= INT_MAX)
            assertionList.push_back(e <= int(maxValue));
        else if (minValue >= INT_MIN)
            assertionList.push_back(e >= int(minValue));
        return e;
    }

    z3::expr addFloat(const std::string &name) {
#if Z3_VERSION_INT >= GET_VERSION_INT(4,8,0)
        z3::expr e = context.fpa_const(name.c_str(), 11, 53);
#else
        z3::expr e = context.real_const(name.c_str());
#endif
        valueExpr.emplace(name, e);
        return e;
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
#if Z3_VERSION_INT >= GET_VERSION_INT(4,8,5)
            return op1 % op2;
#else
            return op1 - (op1 / op2) * op2;
#endif
        if (b->binop == "==")
            return int_expr(op1) == int_expr(op2);
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
            return bool_expr(op1) && bool_expr(op2);
        if (b->binop == "||")
            return bool_expr(op1) || bool_expr(op2);
        if (b->binop == "<<")
            return op1 * z3::pw(context.int_val(2), op2);
        if (b->binop == ">>")
            return op1 / z3::pw(context.int_val(2), op2);
        throw BugHuntingException(nullptr, "Internal error: Unhandled operator " + b->binop);
    }

    z3::expr getExpr(ExprEngine::ValuePtr v) {
        if (!v)
            throw BugHuntingException(nullptr, "Can not solve expressions, operand value is null");
        if (auto intRange = std::dynamic_pointer_cast<ExprEngine::IntRange>(v)) {
            if (intRange->name[0] != '$')
#if Z3_VERSION_INT >= GET_VERSION_INT(4,7,1)
                return context.int_val(int64_t(intRange->minValue));
#else
                return context.int_val((long long)(intRange->minValue));
#endif
            auto it = valueExpr.find(v->name);
            if (it != valueExpr.end())
                return it->second;
            return addInt(v->name, intRange->minValue, intRange->maxValue);
        }

        if (auto floatRange = std::dynamic_pointer_cast<ExprEngine::FloatRange>(v)) {
            auto it = valueExpr.find(v->name);
            if (it != valueExpr.end())
                return it->second;
            return addFloat(v->name);
        }

        if (auto b = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(v)) {
            return getExpr(b.get());
        }

        if (auto c = std::dynamic_pointer_cast<ExprEngine::ConditionalValue>(v)) {
            if (c->values.empty())
                throw BugHuntingException(nullptr, "ConditionalValue is empty");

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

        throw BugHuntingException(nullptr, "Internal error: Unhandled value type");
    }

    z3::expr getConstraintExpr(ExprEngine::ValuePtr v) {
        if (v->type == ExprEngine::ValueType::IntRange)
            return (getExpr(v) != 0);
        return bool_expr(getExpr(v));
    }

private:

    z3::expr bool_expr(z3::expr e) {
        if (e.is_bool())
            return e;
        return e != 0;
    }

    z3::expr int_expr(z3::expr e) {
        if (e.is_bool())
            return z3::ite(e, context.int_val(1), context.int_val(0));
        return e;
    }
};
#endif

bool ExprEngine::IntRange::isEqual(DataBase *dataBase, int value) const
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
        z3::expr e = exprData.addInt(name, minValue, maxValue);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e == value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::IntRange::isGreaterThan(DataBase *dataBase, int value) const
{
    if (maxValue <= value)
        return false;

    const Data *data = dynamic_cast<Data *>(dataBase);
    if (data->constraints.empty())
        return true;
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addInt(name, minValue, maxValue);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e > value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::IntRange::isLessThan(DataBase *dataBase, int value) const
{
    if (minValue >= value)
        return false;

    const Data *data = dynamic_cast<Data *>(dataBase);
    if (data->constraints.empty())
        return true;
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addInt(name, minValue, maxValue);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e < value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::FloatRange::isEqual(DataBase *dataBase, int value) const
{
    const Data *data = dynamic_cast<Data *>(dataBase);
    if (data->constraints.empty())
        return true;
    if (MathLib::isFloat(name)) {
        float f = MathLib::toDoubleNumber(name);
        return value >= f - 0.00001 && value <= f + 0.00001;
    }
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addFloat(name);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e >= value && e <= value);
        return solver.check() != z3::unsat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::FloatRange::isGreaterThan(DataBase *dataBase, int value) const
{
    if (value < minValue || value > maxValue)
        return false;

    const Data *data = dynamic_cast<Data *>(dataBase);
    if (data->constraints.empty())
        return true;
    if (MathLib::isFloat(name))
        return value > MathLib::toDoubleNumber(name);
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addFloat(name);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e > value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::FloatRange::isLessThan(DataBase *dataBase, int value) const
{
    if (value < minValue || value > maxValue)
        return false;

    const Data *data = dynamic_cast<Data *>(dataBase);
    if (data->constraints.empty())
        return true;
    if (MathLib::isFloat(name))
        return value < MathLib::toDoubleNumber(name);
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addFloat(name);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e < value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}


bool ExprEngine::BinOpResult::isEqual(ExprEngine::DataBase *dataBase, int value) const
{
#ifdef USE_Z3
    ExprData exprData;
    z3::solver solver(exprData.context);
    z3::expr e = exprData.getExpr(this);
    for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
        solver.add(exprData.getConstraintExpr(constraint));
    exprData.addAssertions(solver);
    solver.add(e == value);
    return solver.check() == z3::sat;
#else
    (void)dataBase;
    (void)value;
    return false;
#endif
}

bool ExprEngine::BinOpResult::isGreaterThan(ExprEngine::DataBase *dataBase, int value) const
{
#ifdef USE_Z3
    try {
        ExprData exprData;
        z3::solver solver(exprData.context);
        z3::expr e = exprData.getExpr(this);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e > value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3:" << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    (void)dataBase;
    (void)value;
    return false;
#endif
}

bool ExprEngine::BinOpResult::isLessThan(ExprEngine::DataBase *dataBase, int value) const
{
#ifdef USE_Z3
    try {
        ExprData exprData;
        z3::solver solver(exprData.context);
        z3::expr e = exprData.getExpr(this);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e < value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3:" << exception << std::endl;
        return true;  // Safe option is to return true
    }
#else
    (void)dataBase;
    (void)value;
    return false;
#endif
}

std::string ExprEngine::BinOpResult::getExpr(ExprEngine::DataBase *dataBase) const
{
#ifdef USE_Z3
    try {
        ExprData exprData;
        z3::solver solver(exprData.context);
        z3::expr e = exprData.getExpr(this);
        for (auto constraint : dynamic_cast<const Data *>(dataBase)->constraints)
            solver.add(exprData.getConstraintExpr(constraint));
        exprData.addAssertions(solver);
        solver.add(e);
        std::ostringstream os;
        os << solver;
        switch (solver.check()) {
        case z3::sat:
            os << "\nz3::sat\n";
            break;
        case z3::unsat:
            os << "\nz3::unsat\n";
            break;
        case z3::unknown:
            os << "\nz3::unknown\n";
            break;
        }
        return os.str();
    } catch (const z3::exception &exception) {
        std::ostringstream os;
        os << "\nz3:" << exception << "\n";
        return os.str();
    }
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
    }
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

    if (vt->isFloat())
        return std::make_shared<ExprEngine::FloatRange>(name, -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());

    return ExprEngine::ValuePtr();
}

static void call(const std::vector<ExprEngine::Callback> &callbacks, const Token *tok, ExprEngine::ValuePtr value, Data *dataBase)
{
    if (value) {
        for (ExprEngine::Callback f : callbacks) {
            try {
                f(tok, *value, dataBase);
            } catch (const BugHuntingException &e) {
                throw BugHuntingException(tok, e.what);
            }
        }
    }
}

static ExprEngine::ValuePtr executeExpression(const Token *tok, Data &data);
static ExprEngine::ValuePtr executeExpression1(const Token *tok, Data &data);

static ExprEngine::ValuePtr calculateArrayIndex(const Token *tok, Data &data, const ExprEngine::ArrayValue &arrayValue)
{
    int nr = 1;
    const Token *tok2 = tok;
    while (Token::simpleMatch(tok2->astOperand1(), "[")) {
        tok2 = tok2->astOperand1();
        nr++;
    }

    ExprEngine::ValuePtr totalIndex;
    ExprEngine::ValuePtr dim;
    while (Token::simpleMatch(tok, "[")) {
        auto rawIndex = executeExpression(tok->astOperand2(), data);

        ExprEngine::ValuePtr index;
        if (dim)
            index = simplifyValue(std::make_shared<ExprEngine::BinOpResult>("*", dim, rawIndex));
        else
            index = rawIndex;

        if (!totalIndex)
            totalIndex = index;
        else
            totalIndex = simplifyValue(std::make_shared<ExprEngine::BinOpResult>("+", index, totalIndex));

        if (arrayValue.size.size() >= nr) {
            if (arrayValue.size[nr-1]) {
                if (!dim)
                    dim = arrayValue.size[nr-1];
                else
                    dim = simplifyValue(std::make_shared<ExprEngine::BinOpResult>("*", dim, arrayValue.size[nr-1]));
            }
        }

        nr--;
        tok = tok->astOperand1();
    }

    return totalIndex;
}

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

    if (!rhsValue) {
        const ValueType * const vt1 = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
        const ValueType * const vt2 = tok->astOperand2() ? tok->astOperand2()->valueType() : nullptr;

        if (vt1 && vt1->pointer == 0 && vt1->isIntegral())
            rhsValue = getValueRangeFromValueType(data.getNewSymbolName(), vt1, *data.settings);

        else if (vt2 && vt2->container && vt2->container->stdStringLike) {
            auto size = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 0, ~0ULL);
            auto value = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), -128, 127);
            rhsValue = std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), size, value, false, false, false);
            call(data.callbacks, tok->astOperand2(), rhsValue, &data);
        }
    }

    if (!rhsValue)
        throw BugHuntingException(tok, "Expression '" + tok->expressionString() + "'; Failed to evaluate RHS");

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
        const Token *tok2 = lhsToken;
        while (Token::simpleMatch(tok2->astOperand1(), "["))
            tok2 = tok2->astOperand1();
        auto arrayValue = data.getArrayValue(tok2->astOperand1());
        if (arrayValue) {
            // Is it array initialization?
            const Token *arrayInit = tok2->astOperand1();
            if (arrayInit && arrayInit->variable() && arrayInit->variable()->nameToken() == arrayInit) {
                if (assignValue->type == ExprEngine::ValueType::StringLiteralValue)
                    arrayValue->assign(ExprEngine::ValuePtr(), assignValue);
            } else {
                auto indexValue = calculateArrayIndex(lhsToken, data, *arrayValue);
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


#ifdef USE_Z3
static void checkContract(Data &data, const Token *tok, const Function *function, const std::vector<ExprEngine::ValuePtr> &argValues)
{
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        // Invert contract, we want to know if the contract might not be met
        solver.add(z3::ite(exprData.getConstraintExpr(data.executeContract(function, executeExpression1)), exprData.context.bool_val(false), exprData.context.bool_val(true)));

        bool bailoutValue = false;
        for (nonneg int i = 0; i < argValues.size(); ++i) {
            const Variable *argvar = function->getArgumentVar(i);
            if (!argvar || !argvar->nameToken())
                continue;

            ExprEngine::ValuePtr argValue = argValues[i];
            if (!argValue || argValue->type == ExprEngine::ValueType::BailoutValue) {
                bailoutValue = true;
                break;
            }

            if (argValue && argValue->type == ExprEngine::ValueType::IntRange) {
                solver.add(exprData.getExpr(data.getValue(argvar->declarationId(), nullptr, nullptr)) == exprData.getExpr(argValue));
            }
        }

        if (!bailoutValue) {
            for (auto constraint : data.constraints)
                solver.add(exprData.getConstraintExpr(constraint));

            exprData.addAssertions(solver);

            // Log solver expressions for debugging/testing purposes
            std::ostringstream os;
            os << solver;
            data.trackCheckContract(tok, os.str());
        }

        if (bailoutValue || solver.check() == z3::sat) {
            data.addError(tok->linenr());
            std::list<const Token*> callstack{tok};
            const char * const id = "bughuntingFunctionCall";
            const auto contractIt = data.settings->functionContracts.find(function->fullName());
            const std::string functionName = contractIt->first;
            const std::string functionExpects = contractIt->second;
            ErrorMessage errmsg(callstack,
                                &data.tokenizer->list,
                                Severity::SeverityType::error,
                                id,
                                "Function '" + function->name() + "' is called, can not determine that its contract '" + functionExpects + "' is always met.",
                                CWE(0),
                                false);
            errmsg.incomplete = bailoutValue;
            errmsg.function = functionName;
            data.errorLogger->reportErr(errmsg);
        }
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
    } catch (const BugHuntingException &e) {
        std::list<const Token*> callstack{tok};
        const char * const id = "internalErrorInExprEngine";
        const auto contractIt = data.settings->functionContracts.find(function->fullName());
        const std::string functionExpects = contractIt->second;
        ErrorMessage errmsg(callstack,
                            &data.tokenizer->list,
                            Severity::SeverityType::error,
                            id,
                            "Function '" + function->name() + "' is called, can not determine that its contract is always met.",
                            CWE(0),
                            false);
        errmsg.incomplete = true;
        data.errorLogger->reportErr(errmsg);
    }
}
#endif

static ExprEngine::ValuePtr executeFunctionCall(const Token *tok, Data &data)
{
    if (Token::simpleMatch(tok->previous(), "sizeof (")) {
        ExprEngine::ValuePtr retVal;
        if (tok->hasKnownIntValue()) {
            const MathLib::bigint value = tok->getKnownIntValue();
            retVal = std::make_shared<ExprEngine::IntRange>(std::to_string(value), value, value);
        } else {
            retVal = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 1, 0x7fffffff);
        }
        call(data.callbacks, tok, retVal, &data);
        return retVal;
    }

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

    if (tok->astOperand1()->function()) {
        const std::string &functionName = tok->astOperand1()->function()->fullName();
        const auto contractIt = data.settings->functionContracts.find(functionName);
        if (contractIt != data.settings->functionContracts.end()) {
#ifdef USE_Z3
            checkContract(data, tok, tok->astOperand1()->function(), argValues);
#endif
        } else if (!argValues.empty()) {
            bool bailout = false;
            for (const auto v: argValues)
                bailout |= (v && v->type == ExprEngine::ValueType::BailoutValue);
            if (!bailout)
                data.addMissingContract(functionName);
        }
    }

    auto val = getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
    call(data.callbacks, tok, val, &data);
    data.functionCall();
    return val;
}

static ExprEngine::ValuePtr executeArrayIndex(const Token *tok, Data &data)
{
    const Token *tok2 = tok;
    while (Token::simpleMatch(tok2->astOperand1(), "["))
        tok2 = tok2->astOperand1();
    auto arrayValue = data.getArrayValue(tok2->astOperand1());
    if (arrayValue) {
        auto indexValue = calculateArrayIndex(tok, data, *arrayValue);
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
        if (!tok->valueType() || expr->valueType()->pointer < tok->valueType()->pointer)
            return std::make_shared<ExprEngine::UninitValue>();

        ::ValueType vt(*tok->valueType());
        vt.pointer = 0;
        auto range = getValueRangeFromValueType(data.getNewSymbolName(), &vt, *data.settings);

        if (tok->valueType()->pointer == 0)
            return range;

        bool uninitPointer = false, nullPointer = false;
        if (val && val->type == ExprEngine::ValueType::ArrayValue) {
            nullPointer = std::static_pointer_cast<ExprEngine::ArrayValue>(val)->nullPointer;
            uninitPointer = std::static_pointer_cast<ExprEngine::ArrayValue>(val)->uninitPointer;
        }

        auto bufferSize = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 1, ~0UL);
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), bufferSize, range, true, nullPointer, uninitPointer);
    }

    if (val) {
        // TODO: Cast this..
        call(data.callbacks, tok, val, &data);
        return val;
    }

    val = getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
    call(data.callbacks, tok, val, &data);
    return val;
}

static ExprEngine::ValuePtr executeDot(const Token *tok, Data &data)
{
    if (!tok->astOperand1() || !tok->astOperand1()->varId()) {
        auto v = getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
        call(data.callbacks, tok, v, &data);
        return v;
    }
    std::shared_ptr<ExprEngine::StructValue> structValue = std::dynamic_pointer_cast<ExprEngine::StructValue>(data.getValue(tok->astOperand1()->varId(), nullptr, nullptr));
    if (!structValue) {
        if (tok->originalName() == "->") {
            std::shared_ptr<ExprEngine::ArrayValue> pointerValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(data.getValue(tok->astOperand1()->varId(), nullptr, nullptr));
            if (pointerValue && pointerValue->pointer && !pointerValue->data.empty()) {
                call(data.callbacks, tok->astOperand1(), pointerValue, &data);
                auto indexValue = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
                ExprEngine::ValuePtr ret;
                for (auto val: pointerValue->read(indexValue)) {
                    structValue = std::dynamic_pointer_cast<ExprEngine::StructValue>(val.second);
                    if (structValue) {
                        auto memberValue = structValue->getValueOfMember(tok->astOperand2()->str());
                        call(data.callbacks, tok, memberValue, &data);
                        if (!ret)
                            ret = memberValue;
                    }
                }
                return ret;
            } else {
                call(data.callbacks, tok->astOperand1(), data.getValue(tok->astOperand1()->varId(), nullptr, nullptr), &data);
            }
        }
        if (!structValue) {
            auto v = getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
            if (!v)
                v = std::make_shared<ExprEngine::BailoutValue>();
            call(data.callbacks, tok, v, &data);
            return v;
        }
    }
    call(data.callbacks, tok->astOperand1(), structValue, &data);
    ExprEngine::ValuePtr memberValue = structValue->getValueOfMember(tok->astOperand2()->str());
    call(data.callbacks, tok, memberValue, &data);
    return memberValue;
}

static ExprEngine::ValuePtr executeBinaryOp(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr v1 = executeExpression(tok->astOperand1(), data);
    ExprEngine::ValuePtr v2;

    if (tok->str() == "?") {
        if (tok->astOperand1()->hasKnownIntValue()) {
            if (tok->astOperand1()->getKnownIntValue())
                v2 = executeExpression(tok->astOperand2()->astOperand1(), data);
            else
                v2 = executeExpression(tok->astOperand2()->astOperand2(), data);
            call(data.callbacks, tok, v2, &data);
            return v2;
        }

        Data trueData(data);
        trueData.addConstraint(v1, true);
        auto trueValue = simplifyValue(executeExpression(tok->astOperand2()->astOperand1(), trueData));

        Data falseData(data);
        falseData.addConstraint(v1, false);
        auto falseValue = simplifyValue(executeExpression(tok->astOperand2()->astOperand2(), falseData));

        auto result = simplifyValue(std::make_shared<ExprEngine::BinOpResult>("?", v1, std::make_shared<ExprEngine::BinOpResult>(":", trueValue, falseValue)));
        call(data.callbacks, tok, result, &data);
        return result;

    } else if (tok->str() == "&&" || tok->str() == "||") {
        Data data2(data);
        data2.addConstraint(v1, tok->str() == "&&");
        v2 = executeExpression(tok->astOperand2(), data2);
    } else {
        v2 = executeExpression(tok->astOperand2(), data);
    }

    if (v1 && v2) {
        auto result = simplifyValue(std::make_shared<ExprEngine::BinOpResult>(tok->str(), v1, v2));
        call(data.callbacks, tok, result, &data);
        return result;
    }
    if (tok->str() == "&&" && (v1 || v2)) {
        auto result = v1 ? v1 : v2;
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
    if (!pval) {
        auto v = getValueRangeFromValueType(data.getNewSymbolName(), tok->valueType(), *data.settings);
        if (tok->astOperand1()->varId()) {
            pval = std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), ExprEngine::ValuePtr(), v, true, false, false);
            data.assignValue(tok->astOperand1(), tok->astOperand1()->varId(), pval);
        }
        call(data.callbacks, tok, v, &data);
        return v;
    }
    auto addressOf = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(pval);
    if (addressOf) {
        auto val = data.getValue(addressOf->varId, tok->valueType(), tok);
        call(data.callbacks, tok, val, &data);
        return val;
    }
    auto pointer = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(pval);
    if (pointer) {
        auto indexValue = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
        auto conditionalValues = pointer->read(indexValue);
        for (auto value: conditionalValues)
            call(data.callbacks, tok, value.second, &data);
        if (conditionalValues.size() == 1 && !conditionalValues[0].first)
            return conditionalValues[0].second;
        return std::make_shared<ExprEngine::ConditionalValue>(data.getNewSymbolName(), conditionalValues);
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

static ExprEngine::ValuePtr executeNumber(const Token *tok, Data &data)
{
    if (tok->valueType()->isFloat()) {
        long double value = MathLib::toDoubleNumber(tok->str());
        auto v = std::make_shared<ExprEngine::FloatRange>(tok->str(), value, value);
        call(data.callbacks, tok, v, &data);
        return v;
    }
    int128_t value = MathLib::toLongNumber(tok->str());
    auto v = std::make_shared<ExprEngine::IntRange>(tok->str(), value, value);
    call(data.callbacks, tok, v, &data);
    return v;
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

    if (tok->str() == "::" && tok->hasKnownIntValue()) { // TODO handle :: better
        auto v = tok->getKnownIntValue();
        return std::make_shared<ExprEngine::IntRange>(std::to_string(v), v, v);
    }

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
        return executeNumber(tok, data);

    if (tok->tokType() == Token::Type::eString)
        return executeStringLiteral(tok, data);

    return ExprEngine::ValuePtr();
}

static ExprEngine::ValuePtr executeExpression(const Token *tok, Data &data)
{
    return translateUninitValueToRange(executeExpression1(tok, data), tok->valueType(), data);
}

static ExprEngine::ValuePtr createVariableValue(const Variable &var, Data &data);

static void execute(const Token *start, const Token *end, Data &data, int recursion=0)
{
    if (++recursion > 20)
        // FIXME
        return;

    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (Token::Match(tok, "[;{}]"))
            data.trackProgramState(tok);

        if (Token::simpleMatch(tok, "while (") && (tok->linkAt(1), ") ;") && tok->next()->astOperand1()->hasKnownIntValue() && tok->next()->astOperand1()->getKnownIntValue() == 0) {
            tok = tok->tokAt(4);
            continue;
        }

        if (tok->str() == "break") {
            const Scope *scope = tok->scope();
            while (scope->type == Scope::eIf || scope->type == Scope::eElse)
                scope = scope->nestedIn;
            tok = scope->bodyEnd;
            if (!precedes(tok,end))
                return;
        }

        if (Token::simpleMatch(tok, "try"))
            // TODO this is a bailout
            throw BugHuntingException(tok, "Unhandled:" + tok->str());

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
            } else if (tok->variable()->isArray()) {
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
            Data thenData(data);
            Data elseData(data);
            thenData.addConstraint(condValue, true);
            elseData.addConstraint(condValue, false);

            const Token *thenStart = tok->linkAt(1)->next();
            const Token *thenEnd = thenStart->link();

            const Token *exceptionToken = nullptr;
            std::string exceptionMessage;
            auto exec = [&](const Token *tok1, const Token *tok2, Data& data) {
                try {
                    execute(tok1, tok2, data, recursion);
                } catch (BugHuntingException &e) {
                    if (!exceptionToken || (e.tok && precedes(e.tok, exceptionToken))) {
                        exceptionToken = e.tok;
                        exceptionMessage = e.what;
                    }
                }
            };

            exec(thenStart->next(), end, thenData);

            if (Token::simpleMatch(thenEnd, "} else {")) {
                const Token *elseStart = thenEnd->tokAt(2);
                exec(elseStart->next(), end, elseData);
            } else {
                exec(thenEnd, end, elseData);
            }

            if (exceptionToken)
                throw BugHuntingException(exceptionToken, exceptionMessage);
            return;
        }

        else if (Token::simpleMatch(tok, "switch (")) {
            auto condValue = executeExpression(tok->next()->astOperand2(), data); // TODO: C++17 condition
            const Token *bodyStart = tok->linkAt(1)->next();
            const Token *bodyEnd = bodyStart->link();
            const Token *defaultStart = nullptr;
            Data defaultData(data);
            const Token *exceptionToken = nullptr;
            std::string exceptionMessage;
            auto exec = [&](const Token *tok1, const Token *tok2, Data& data) {
                try {
                    execute(tok1, tok2, data, recursion);
                } catch (BugHuntingException &e) {
                    if (!exceptionToken || (e.tok && precedes(e.tok, exceptionToken))) {
                        exceptionToken = e.tok;
                        exceptionMessage = e.what;
                    }
                }
            };
            for (const Token *tok2 = bodyStart->next(); tok2 != bodyEnd; tok2 = tok2->next()) {
                if (tok2->str() == "{")
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "case %char%|%num% :")) {
                    const MathLib::bigint caseValue1 = tok2->next()->getKnownIntValue();
                    auto caseValue = std::make_shared<ExprEngine::IntRange>(MathLib::toString(caseValue1), caseValue1, caseValue1);
                    Data caseData(data);
                    caseData.addConstraint(condValue, caseValue, true);
                    defaultData.addConstraint(condValue, caseValue, false);
                    exec(tok2->tokAt(2), end, caseData);
                } else if (Token::Match(tok2, "case %name% :") && !Token::Match(tok2->tokAt(3), ";| case")) {
                    Data caseData(data);
                    exec(tok2->tokAt(2), end, caseData);
                } else if (Token::simpleMatch(tok2, "default :"))
                    defaultStart = tok2;
            }
            exec(defaultStart ? defaultStart : bodyEnd, end, defaultData);
            if (exceptionToken)
                throw BugHuntingException(exceptionToken, exceptionMessage);
            return;
        }

        if (Token::Match(tok, "for|while (") && Token::simpleMatch(tok->linkAt(1), ") {")) {
            const Token *bodyStart = tok->linkAt(1)->next();
            const Token *bodyEnd = bodyStart->link();

            // TODO this is very rough code
            std::set<int> changedVariables;
            for (const Token *tok2 = tok; tok2 != bodyEnd; tok2 = tok2->next()) {
                if (Token::Match(tok2, "%assign%")) {
                    const Token *lhs = tok2->astOperand1();
                    while (Token::simpleMatch(lhs, "["))
                        lhs = lhs->astOperand1();
                    if (!lhs)
                        throw BugHuntingException(tok2, "Unhandled assignment in loop");
                    if (Token::Match(lhs, ". %name% =|[") && lhs->astOperand1() && lhs->astOperand1()->valueType()) {
                        const Token *structToken = lhs->astOperand1();
                        if (!structToken->valueType() || !structToken->varId())
                            throw BugHuntingException(tok2, "Unhandled assignment in loop");
                        const Scope *structScope = structToken->valueType()->typeScope;
                        if (!structScope)
                            throw BugHuntingException(tok2, "Unhandled assignment in loop");
                        const std::string &memberName = tok2->previous()->str();
                        ExprEngine::ValuePtr memberValue;
                        for (const Variable &member : structScope->varlist) {
                            if (memberName == member.name() && member.valueType()) {
                                memberValue = createVariableValue(member, data);
                                break;
                            }
                        }
                        if (!memberValue)
                            throw BugHuntingException(tok2, "Unhandled assignment in loop");

                        ExprEngine::ValuePtr structVal1 = data.getValue(structToken->varId(), structToken->valueType(), structToken);
                        if (!structVal1)
                            structVal1 = createVariableValue(*structToken->variable(), data);
                        auto structVal = std::dynamic_pointer_cast<ExprEngine::StructValue>(structVal1);
                        if (!structVal)
                            throw BugHuntingException(tok2, "Unhandled assignment in loop");

                        data.assignStructMember(tok2, &*structVal, memberName, memberValue);
                        continue;
                    }
                    if (lhs->isUnaryOp("*") && lhs->astOperand1()->varId()) {
                        const Token *varToken = tok2->astOperand1()->astOperand1();
                        ExprEngine::ValuePtr val = data.getValue(varToken->varId(), varToken->valueType(), varToken);
                        if (val && val->type == ExprEngine::ValueType::ArrayValue) {
                            // Try to assign "any" value
                            auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(val);
                            arrayValue->assign(std::make_shared<ExprEngine::IntRange>("0", 0, 0), std::make_shared<ExprEngine::BailoutValue>());
                            continue;
                        }
                    }
                    if (!lhs->variable())
                        throw BugHuntingException(tok2, "Unhandled assignment in loop");
                    // give variable "any" value
                    int varid = lhs->varId();
                    if (changedVariables.find(varid) != changedVariables.end())
                        continue;
                    changedVariables.insert(varid);
                    auto oldValue = data.getValue(varid, nullptr, nullptr);
                    if (oldValue && oldValue->isUninit())
                        call(data.callbacks, lhs, oldValue, &data);
                    if (oldValue && oldValue->type == ExprEngine::ValueType::ArrayValue) {
                        // Try to assign "any" value
                        auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(oldValue);
                        arrayValue->assign(std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 0, ~0ULL), std::make_shared<ExprEngine::BailoutValue>());
                        continue;
                    }
                    data.assignValue(tok2, varid, getValueRangeFromValueType(data.getNewSymbolName(), lhs->valueType(), *data.settings));
                    continue;
                } else if (Token::Match(tok2, "++|--") && tok2->astOperand1() && tok2->astOperand1()->variable()) {
                    // give variable "any" value
                    const Token *vartok = tok2->astOperand1();
                    int varid = vartok->varId();
                    if (changedVariables.find(varid) != changedVariables.end())
                        continue;
                    changedVariables.insert(varid);
                    auto oldValue = data.getValue(varid, nullptr, nullptr);
                    if (oldValue && oldValue->type == ExprEngine::ValueType::UninitValue)
                        call(data.callbacks, tok2, oldValue, &data);
                    data.assignValue(tok2, varid, getValueRangeFromValueType(data.getNewSymbolName(), vartok->valueType(), *data.settings));
                }
            }
        }

        if (Token::simpleMatch(tok, "} else {"))
            tok = tok->linkAt(2);
    }
}

void ExprEngine::executeAllFunctions(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks, std::ostream &report)
{
    const SymbolDatabase *symbolDatabase = tokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        try {
            executeFunction(functionScope, errorLogger, tokenizer, settings, callbacks, report);
        } catch (const BugHuntingException &e) {
            // FIXME.. there should not be exceptions
            std::string functionName = functionScope->function->name();
            std::cout << "Verify: Aborted analysis of function '" << functionName << "':" << e.tok->linenr() << ": " << e.what << std::endl;
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
        auto bufferSize = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 1, ~0UL);
        ExprEngine::ValuePtr pointerValue;
        if (valueType->type == ValueType::Type::RECORD)
            pointerValue = createStructVal(valueType->typeScope, var.isLocal() && !var.isStatic(), data);
        else {
            ValueType vt(*valueType);
            vt.pointer = 0;
            if (vt.constness & 1)
                pointerValue = getValueRangeFromValueType(data.getNewSymbolName(), &vt, *data.settings);
            else
                pointerValue = std::make_shared<ExprEngine::UninitValue>();
        }
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), bufferSize, pointerValue, true, true, var.isLocal() && !var.isStatic());
    }
    if (var.isArray())
        return std::make_shared<ExprEngine::ArrayValue>(&data, &var);
    if (valueType->isIntegral() || valueType->isFloat()) {
        ExprEngine::ValuePtr value;
        if (var.isLocal() && !var.isStatic())
            value = std::make_shared<ExprEngine::UninitValue>();
        else
            value = getValueRangeFromValueType(data.getNewSymbolName(), valueType, *data.settings);
        data.addConstraints(value, var.nameToken());
        return value;
    }
    if (valueType->type == ValueType::Type::RECORD)
        return createStructVal(valueType->typeScope, var.isLocal() && !var.isStatic(), data);
    if (valueType->smartPointerType) {
        auto structValue = createStructVal(valueType->smartPointerType->classScope, var.isLocal() && !var.isStatic(), data);
        auto size = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 1, ~0UL);
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), size, structValue, true, true, false);
    }
    if (valueType->container) {
        ExprEngine::ValuePtr value;
        if (valueType->container->stdStringLike)
            value = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), -128, 127);
        else if (valueType->containerTypeToken) {
            ValueType vt = ValueType::parseDecl(valueType->containerTypeToken, data.settings);
            value = getValueRangeFromValueType(data.getNewSymbolName(), &vt, *data.settings);
        } else
            return ExprEngine::ValuePtr();
        auto bufferSize = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 0, ~0U);
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), bufferSize, value, false, false, false);
    }
    return ExprEngine::ValuePtr();
}

void ExprEngine::executeFunction(const Scope *functionScope, ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks, std::ostream &report)
{
    if (!functionScope->bodyStart)
        return;
    const Function *function = functionScope->function;
    if (!function)
        return;
    if (functionScope->bodyStart->fileIndex() > 0)
        // TODO.. what about functions in headers?
        return;

    const std::string currentFunction = function->fullName();

    int symbolValueIndex = 0;
    TrackExecution trackExecution;
    Data data(&symbolValueIndex, errorLogger, tokenizer, settings, currentFunction, callbacks, &trackExecution);

    for (const Variable &arg : function->argumentList)
        data.assignValue(functionScope->bodyStart, arg.declarationId(), createVariableValue(arg, data));

    data.contractConstraints(function, executeExpression1);

    try {
        execute(functionScope->bodyStart, functionScope->bodyEnd, data);
    } catch (BugHuntingException &e) {
        if (settings->debugBugHunting)
            report << "BugHuntingException tok.line:" << e.tok->linenr() << " what:" << e.what << "\n";
        trackExecution.setAbortLine(e.tok->linenr());
        auto bailoutValue = std::make_shared<BailoutValue>();
        for (const Token *tok = e.tok; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (Token::Match(tok, "return|throw|while|if|for (")) {
                tok = tok->next();
                continue;
            }
            call(callbacks, tok, bailoutValue, &data);
        }
    }

    const bool bugHuntingReport = !settings->bugHuntingReport.empty();

    if (settings->debugBugHunting && (settings->verbose || callbacks.empty() || !trackExecution.isAllOk())) {
        if (bugHuntingReport)
            report << "[debug]" << std::endl;
        trackExecution.print(report);
        if (!callbacks.empty()) {
            if (bugHuntingReport)
                report << "[details]" << std::endl;
            trackExecution.report(report, functionScope);
        }
    }

    // Write a report
    if (bugHuntingReport) {
        for (const std::string &f: trackExecution.getMissingContracts())
            report << "[missing contract] " << f << std::endl;
    }
}

static float getKnownFloatValue(const Token *tok, float def)
{
    for (const auto &value: tok->values()) {
        if (value.isKnown() && value.valueType == ValueFlow::Value::ValueType::FLOAT)
            return value.floatValue;
    }
    return def;
}

void ExprEngine::runChecks(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings)
{
    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> bufferOverflow = [=](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
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

        if (const Library::Function *func = settings->library.getFunction(ftok)) {
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

        dataBase->addError(tok->linenr());
        std::list<const Token*> callstack{tok};
        ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "bughuntingBufferOverflow", "Buffer read/write, when calling '" + ftok->str() +  "' it cannot be determined that " + std::to_string(overflowArgument) + getOrdinalText(overflowArgument) + " argument is not overflowed", CWE(120), false);
        if (value.type == ExprEngine::ValueType::BailoutValue)
            errmsg.incomplete = true;
        else
            errmsg.function = dataBase->currentFunction;
        errorLogger->reportErr(errmsg);
    };

    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> divByZero = [=](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
        if (!tok->astParent() || !std::strchr("/%", tok->astParent()->str()[0]))
            return;
        if (tok->hasKnownIntValue() && tok->getKnownIntValue() != 0)
            return;
        if (tok->isImpossibleIntValue(0))
            return;
        if (value.isUninit())
            return;
        float f = getKnownFloatValue(tok, 0.0f);
        if (f > 0.0f || f < 0.0f)
            return;
        if (value.type == ExprEngine::ValueType::BailoutValue) {
            if (Token::simpleMatch(tok->previous(), "sizeof ("))
                return;
        }
        if (tok->astParent()->astOperand2() == tok && value.isEqual(dataBase, 0)) {
            dataBase->addError(tok->linenr());
            std::list<const Token*> callstack{settings->clang ? tok : tok->astParent()};
            const char * const id = (tok->valueType() && tok->valueType()->isFloat()) ? "bughuntingDivByZeroFloat" : "bughuntingDivByZero";
            const bool bailout = (value.type == ExprEngine::ValueType::BailoutValue);
            ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, id, "There is division, cannot determine that there can't be a division by zero.", CWE(369), false);
            if (!bailout)
                errmsg.function = dataBase->currentFunction;
            else
                errmsg.incomplete = bailout;
            errorLogger->reportErr(errmsg);
        }
    };

#ifdef BUG_HUNTING_INTEGEROVERFLOW
    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> integerOverflow = [&](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
        if (!tok->isArithmeticalOp() || !tok->valueType() || !tok->valueType()->isIntegral() || tok->valueType()->pointer > 0)
            return;

        const ExprEngine::BinOpResult *b = dynamic_cast<const ExprEngine::BinOpResult *>(&value);
        if (!b)
            return;

        int bits = getIntBitsFromValueType(tok->valueType(), *settings);
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

        std::list<const Token*> callstack{tok};
        ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "bughuntingIntegerOverflow", errorMessage, false);
        errorLogger->reportErr(errmsg);
    };
#endif

    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> uninit = [=](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
        if (!tok->astParent())
            return;
        if (!value.isUninit())
            return;

        // lhs in assignment
        if (tok->astParent()->str() == "=" && tok == tok->astParent()->astOperand1())
            return;

        // Avoid FP when there is bailout..
        if (value.type == ExprEngine::ValueType::BailoutValue) {
            if (tok->hasKnownValue())
                return;
            if (tok->function())
                return;
            if (Token::Match(tok, "<<|>>|,"))
                // Only warn about the operands
                return;
            // lhs for scope operator
            if (Token::Match(tok, "%name% ::"))
                return;
            if (tok->astParent()->str() == "::" && tok == tok->astParent()->astOperand1())
                return;

            if (tok->str() == "(")
                // cast: result is not uninitialized if expression is initialized
                // function: does not return a uninitialized value
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
            if (var && !var->isPointer()) {
                if (!var->isLocal() || var->isStatic())
                    return;
            }
            if (var && (Token::Match(var->nameToken(), "%name% =") || Token::Match(var->nameToken(), "%varid% ; %varid% =", var->declarationId())))
                return;
            if (var && var->nameToken() == tok)
                return;

        }

        // Avoid FP for array declaration
        const Token *parent = tok->astParent();
        while (parent && parent->str() == "[")
            parent = parent->astParent();
        if (!parent)
            return;

        dataBase->addError(tok->linenr());
        std::list<const Token*> callstack{tok};
        ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "bughuntingUninit", "Cannot determine that '" + tok->expressionString() + "' is initialized", CWE_USE_OF_UNINITIALIZED_VARIABLE, false);
        errorLogger->reportErr(errmsg);
    };

    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> checkFunctionCall = [=](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
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
                    dataBase->addError(tok->linenr());
                    std::list<const Token*> callstack{tok};
                    ErrorMessage errmsg(callstack,
                                        &tokenizer->list,
                                        Severity::SeverityType::error,
                                        "bughuntingInvalidArgValue",
                                        "There is function call, cannot determine that " + std::to_string(num) + getOrdinalText(num) + " argument value meets the attribute " + bad, CWE(0), false);
                    errorLogger->reportErr(errmsg);
                    return;
                }
            }
        }

        // Check invalid function argument values..
        for (const Library::InvalidArgValue &invalidArgValue : Library::getInvalidArgValues(settings->library.validarg(parent->astOperand1(), num))) {
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
                dataBase->addError(tok->linenr());
                std::list<const Token*> callstack{tok};
                ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "bughuntingInvalidArgValue", "There is function call, cannot determine that " + std::to_string(num) + getOrdinalText(num) + " argument value is valid. Bad value: " + bad, CWE(0), false);
                errorLogger->reportErr(errmsg);
                break;
            }
        }

        // Uninitialized function argument..
        if (settings->library.isuninitargbad(parent->astOperand1(), num) && settings->library.isnullargbad(parent->astOperand1(), num) && value.type == ExprEngine::ValueType::ArrayValue) {
            const ExprEngine::ArrayValue &arrayValue = static_cast<const ExprEngine::ArrayValue &>(value);
            auto index0 = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
            for (const auto &v: arrayValue.read(index0)) {
                if (v.second->isUninit()) {
                    dataBase->addError(tok->linenr());
                    std::list<const Token*> callstack{tok};
                    ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "bughuntingUninitArg", "There is function call, cannot determine that " + std::to_string(num) + getOrdinalText(num) + " argument is initialized.", CWE_USE_OF_UNINITIALIZED_VARIABLE, false);
                    errorLogger->reportErr(errmsg);
                    break;
                }
            }
        }
    };

    std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> checkAssignment = [=](const Token *tok, const ExprEngine::Value &value, ExprEngine::DataBase *dataBase) {
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
            if (value.isLessThan(dataBase, low)) {
                dataBase->addError(tok->linenr());
                std::list<const Token*> callstack{tok};
                ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "bughuntingAssign", "There is assignment, cannot determine that value is greater or equal with " + std::to_string(low), CWE_INCORRECT_CALCULATION, false);
                errorLogger->reportErr(errmsg);
            }
        }

        MathLib::bigint high;
        if (vartok->getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type::HIGH, &high)) {
            if (value.isGreaterThan(dataBase, high)) {
                dataBase->addError(tok->linenr());
                std::list<const Token*> callstack{tok};
                ErrorMessage errmsg(callstack, &tokenizer->list, Severity::SeverityType::error, "bughuntingAssign", "There is assignment, cannot determine that value is lower or equal with " + std::to_string(high), CWE_INCORRECT_CALCULATION, false);
                errorLogger->reportErr(errmsg);
            }
        }
    };

    std::vector<ExprEngine::Callback> callbacks;
    callbacks.push_back(bufferOverflow);
    callbacks.push_back(divByZero);
    callbacks.push_back(checkFunctionCall);
    callbacks.push_back(checkAssignment);
#ifdef BUG_HUNTING_INTEGEROVERFLOW
    callbacks.push_back(integerOverflow);
#endif
    callbacks.push_back(uninit);

    std::ostringstream report;
    ExprEngine::executeAllFunctions(errorLogger, tokenizer, settings, callbacks, report);
    if (settings->bugHuntingReport.empty())
        std::cout << report.str();
    else if (errorLogger)
        errorLogger->bughuntingReport(report.str());
}

static void dumpRecursive(ExprEngine::ValuePtr val)
{
    if (!val) {
        std::cout << "NULL";
        return;
    }
    switch (val->type) {
    case ExprEngine::ValueType::AddressOfValue:
        std::cout << "AddressOfValue(" << std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(val)->varId << ")";
        break;
    case ExprEngine::ValueType::ArrayValue:
        std::cout << "ArrayValue";
        break;
    case ExprEngine::ValueType::BailoutValue:
        std::cout << "BailoutValue";
        break;
    case ExprEngine::ValueType::BinOpResult: {
        auto b = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(val);
        std::cout << "(";
        dumpRecursive(b->op1);
        std::cout << " " << b->binop << " ";
        dumpRecursive(b->op2);
        std::cout << ")";
    }
    break;
    case ExprEngine::ValueType::ConditionalValue:
        std::cout << "ConditionalValue";
        break;
    case ExprEngine::ValueType::FloatRange:
        std::cout << "FloatRange";
        break;
    case ExprEngine::ValueType::IntRange:
        std::cout << "IntRange";
        break;
    case ExprEngine::ValueType::IntegerTruncation:
        std::cout << "IntegerTruncation(";
        dumpRecursive(std::dynamic_pointer_cast<ExprEngine::IntegerTruncation>(val)->inputValue);
        std::cout << ")";
        break;
    case ExprEngine::ValueType::StringLiteralValue:
        std::cout << "StringLiteralValue";
        break;
    case ExprEngine::ValueType::StructValue:
        std::cout << "StructValue";
        break;
    case ExprEngine::ValueType::UninitValue:
        std::cout << "UninitValue";
        break;
    }
}

void ExprEngine::dump(ExprEngine::ValuePtr val)
{
    dumpRecursive(val);
    std::cout << "\n";
}



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
 * static std::string execute(const Token *start, const Token *end, Data &data)
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
#include "bughuntingchecks.h"
#include "errorlogger.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenize.h"

#include <limits>
#include <memory>
#include <iostream>
#include <tuple>
#ifdef USE_Z3
#include <z3++.h>
#include <z3_version.h>
#define GET_VERSION_INT(A,B,C)     ((A) * 10000 + (B) * 100 + (C))
#define Z3_VERSION_INT             GET_VERSION_INT(Z3_MAJOR_VERSION, Z3_MINOR_VERSION, Z3_BUILD_NUMBER)
#endif

namespace {
    struct ExprEngineException {
        ExprEngineException(const Token *tok, const std::string &what) : tok(tok), what(what) {}
        const Token *tok;
        const std::string what;
    };
    struct TerminateExpression {};
}

static std::string str(ExprEngine::ValuePtr val)
{
    const char *typestr = "???UnknownValueType???";
    switch (val->type) {
    case ExprEngine::ValueType::AddressOfValue:
        typestr = "AddressOfValue";
        break;
    case ExprEngine::ValueType::ArrayValue:
        typestr = "ArrayValue";
        break;
    case ExprEngine::ValueType::UninitValue:
        typestr = "UninitValue";
        break;
    case ExprEngine::ValueType::IntRange:
        typestr = "IntRange";
        break;
    case ExprEngine::ValueType::FloatRange:
        typestr = "FloatRange";
        break;
    case ExprEngine::ValueType::ConditionalValue:
        typestr = "ConditionalValue";
        break;
    case ExprEngine::ValueType::StringLiteralValue:
        typestr = "StringLiteralValue";
        break;
    case ExprEngine::ValueType::StructValue:
        typestr = "StructValue";
        break;
    case ExprEngine::ValueType::BinOpResult:
        typestr = "BinOpResult";
        break;
    case ExprEngine::ValueType::IntegerTruncation:
        typestr = "IntegerTruncation";
        break;
    case ExprEngine::ValueType::FunctionCallArgumentValues:
        typestr = "FunctionCallArgumentValues";
        break;
    case ExprEngine::ValueType::BailoutValue:
        typestr = "BailoutValue";
        break;
    }

    std::ostringstream ret;
    ret << val->name << "=" << typestr << "(" << val->getRange() << ")";
    return ret.str();
}

static size_t extfind(const std::string &str, const std::string &what, size_t pos)
{
    int indent = 0;
    for (; pos < str.size(); ++pos) {
        if (indent <= 0 && str[pos] == what[0])
            return pos;
        else if (str[pos] == '\"') {
            ++pos;
            while (pos < str.size()) {
                if (str[pos] == '\"')
                    break;
                if (pos == '\\')
                    ++pos;
                ++pos;
            }
        } else if (str[pos] == '(')
            ++indent;
        else if (str[pos] == ')')
            --indent;
    }
    return std::string::npos;
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
        TrackExecution() : mDataIndexCounter(0), mAbortLine(-1) {}

        int getNewDataIndex() {
            return mDataIndexCounter++;
        }

        void symbolRange(const Token *tok, ExprEngine::ValuePtr value) {
            if (!tok || !value)
                return;
            if (tok->index() == 0)
                return;
            const std::string &symbolicExpression = value->getSymbolicExpression();
            if (std::isdigit(symbolicExpression[0]) || value->type == ExprEngine::ValueType::BinOpResult || value->type == ExprEngine::ValueType::UninitValue)
                return;
            if (mSymbols.find(symbolicExpression) != mSymbols.end())
                return;
            mSymbols.insert(symbolicExpression);
            mMap[tok].push_back(str(value));
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

        void ifSplit(const Token *tok, unsigned int thenIndex, unsigned int elseIndex) {
            mMap[tok].push_back(std::to_string(thenIndex) + ": Split. Then:" + std::to_string(thenIndex) + " Else:" + std::to_string(elseIndex));
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

        int mDataIndexCounter;
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
            , recursion(0)
            , startTime(std::time(nullptr))
            , mTrackExecution(trackExecution)
            , mDataIndex(trackExecution->getNewDataIndex()) {}

        Data(const Data &old)
            : DataBase(old.currentFunction, old.settings)
            , memory(old.memory)
            , symbolValueIndex(old.symbolValueIndex)
            , errorLogger(old.errorLogger)
            , tokenizer(old.tokenizer)
            , callbacks(old.callbacks)
            , constraints(old.constraints)
            , recursion(old.recursion)
            , startTime(old.startTime)
            , mTrackExecution(old.mTrackExecution)
            , mDataIndex(mTrackExecution->getNewDataIndex()) {
            for (auto &it: memory) {
                if (!it.second)
                    continue;
                if (auto oldValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(it.second))
                    it.second = std::make_shared<ExprEngine::ArrayValue>(getNewSymbolName(), *oldValue);
            }
        }

        typedef std::map<nonneg int, ExprEngine::ValuePtr> Memory;
        Memory memory;
        int * const symbolValueIndex;
        ErrorLogger *errorLogger;
        const Tokenizer * const tokenizer;
        const std::vector<ExprEngine::Callback> &callbacks;
        std::vector<ExprEngine::ValuePtr> constraints;
        int recursion;
        std::time_t startTime;

        bool isC() const OVERRIDE {
            return tokenizer->isC();
        }
        bool isCPP() const OVERRIDE {
            return tokenizer->isCPP();
        }

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

            // constant value..
            const Variable *var = tokenizer->getSymbolDatabase()->getVariableFromVarId(varId);
            if (var && valueType->constness == 1 && Token::Match(var->nameToken(), "%var% =")) {
                const Token *initExpr = var->nameToken()->next()->astOperand2();
                if (initExpr && initExpr->hasKnownIntValue()) {
                    auto intval = initExpr->getKnownIntValue();
                    return std::make_shared<ExprEngine::IntRange>(std::to_string(intval), intval, intval);
                }
            }

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
            s << mDataIndex << ":" << "memory:{";
            bool first = true;
            for (auto mem : memory) {
                ExprEngine::ValuePtr value = mem.second;
                const Variable *var = symbolDatabase->getVariableFromVarId(mem.first);
                if (!var)
                    continue;
                if (!first)
                    s << " ";
                first = false;
                s << var->name() << "=";
                if (!value)
                    s << "(null)";
                else if (value->name[0] == '$' && value->getSymbolicExpression() != value->name)
                    s << "(" << value->name << "," << value->getSymbolicExpression() << ")";
                else
                    s << value->name;
            }
            s << "}";

            if (!constraints.empty()) {
                s << " constraints:{";
                first = true;
                for (auto constraint: constraints) {
                    if (!first)
                        s << " ";
                    first = false;
                    s << constraint->getSymbolicExpression();
                }
                s << "}";
            }
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
            if (std::dynamic_pointer_cast<ExprEngine::FloatRange>(v)) {
                auto zero = std::make_shared<ExprEngine::FloatRange>("0.0", 0.0, 0.0);
                return std::make_shared<ExprEngine::BinOpResult>("==", v, zero);
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

        void reportError(const Token *tok,
                         Severity::SeverityType severity,
                         const char id[],
                         const std::string &text,
                         CWE cwe,
                         bool inconclusive,
                         bool incomplete,
                         const std::string &functionName) OVERRIDE {
            if (errorPath.empty())
                mTrackExecution->addError(tok->linenr());

            ErrorPath e = errorPath;
            e.push_back(ErrorPathItem(tok, text));
            ErrorMessage errmsg(e, &tokenizer->list, severity, id, text, cwe, inconclusive);
            errmsg.incomplete = incomplete;
            errmsg.function = functionName.empty() ? currentFunction : functionName;
            errorLogger->reportErr(errmsg);
        }

        std::string str() const {
            std::ostringstream ret;
            std::map<std::string, ExprEngine::ValuePtr> vars;
            for (const auto &mem: memory) {
                if (!mem.second)
                    continue;
                const Variable *var = tokenizer->getSymbolDatabase()->getVariableFromVarId(mem.first);
                if (var && var->isLocal())
                    continue;
                ret << " @" << mem.first << ":" << mem.second->name;
                getSymbols(vars, mem.second);
            }
            for (const auto &var: vars) {
                if (var.second->name[0] == '$')
                    ret << " " << ::str(var.second);
            }
            for (const auto &c: constraints)
                ret << " (" << c->getSymbolicExpression() << ")";
            ret << std::endl;
            return ret.str();
        }

        void load(const std::string &s) {
            std::vector<ImportData> importData;
            parsestr(s, &importData);
            //simplify(importData);

            if (importData.empty())
                return;

            std::map<std::string, ExprEngine::ValuePtr> symbols;
            for (auto mem: memory) {
                getSymbols(symbols, mem.second);
            }

            // TODO: combined symbolvalue
            std::map<int, std::string> combinedMemory;
            for (const ImportData &d: importData) {
                for (const auto &mem: d.mem) {
                    auto c = combinedMemory.find(mem.first);
                    if (c == combinedMemory.end()) {
                        combinedMemory[mem.first] = mem.second;
                        continue;
                    }
                    if (c->second == mem.second)
                        continue;
                    if (c->second == "?" || mem.second == "?")
                        c->second = "?";
                    else
                        c->second.clear();
                }
            }

            for (const auto &mem: combinedMemory) {
                int varid = mem.first;
                const std::string &name = mem.second;
                auto it = memory.find(varid);
                if (it != memory.end() && it->second && it->second->name == name)
                    continue;
                if (name.empty()) {
                    if (it != memory.end())
                        memory.erase(it);
                    continue;
                }
                auto it2 = symbols.find(name);
                if (it2 != symbols.end()) {
                    memory[varid] = it2->second;
                    continue;
                }
                if (name == "?") {
                    auto uninitValue = std::make_shared<ExprEngine::UninitValue>();
                    symbols[name] = uninitValue;
                    memory[varid] = uninitValue;
                    continue;
                }
                if (std::isdigit(name[0])) {
                    long long v = std::stoi(name);
                    auto intRange = std::make_shared<ExprEngine::IntRange>(name, v, v);
                    symbols[name] = intRange;
                    memory[varid] = intRange;
                    continue;
                }
                // TODO: handle this value..
                if (it != memory.end())
                    memory.erase(it);
            }
        }

        static void ifSplit(const Token *tok, const Data& thenData, const Data& elseData) {
            thenData.mTrackExecution->ifSplit(tok, thenData.mDataIndex, elseData.mDataIndex);
        }

    private:
        TrackExecution * const mTrackExecution;
        const int mDataIndex;

        struct ImportData {
            std::map<int, std::string> mem;
            std::map<std::string, std::string> sym;
            std::vector<std::string> constraints;
        };

        void parsestr(const std::string &s, std::vector<ImportData> *importData) const {
            std::string line;
            std::istringstream istr(s);
            while (std::getline(istr, line)) {
                if (line.empty())
                    continue;
                line += " ";
                ImportData d;
                for (std::string::size_type pos = 0; pos < line.size();) {
                    pos = line.find_first_not_of(" ", pos);
                    if (pos == std::string::npos)
                        break;
                    if (line[pos] == '@') {
                        ++pos;
                        std::string::size_type colon = line.find(":", pos);
                        std::string::size_type end = line.find(" ", colon);
                        const std::string lhs = line.substr(pos, colon-pos);
                        pos = colon + 1;
                        const std::string rhs = line.substr(pos, end-pos);
                        d.mem[std::stoi(lhs)] = rhs;
                        pos = end;
                    } else if (line[pos] == '$') {
                        const std::string::size_type eq = line.find("=", pos);
                        const std::string lhs = line.substr(pos, eq-pos);
                        pos = eq + 1;
                        const std::string::size_type end = extfind(line, " ", pos);
                        const std::string rhs = line.substr(pos, end-pos);
                        pos = end;
                        d.sym[lhs] = rhs;
                    } else if (line[pos] == '(') {
                        const std::string::size_type end = extfind(line, " ", pos);
                        const std::string c = line.substr(pos, end-pos);
                        pos = end;
                        d.constraints.push_back(c);
                    } else {
                        throw ExprEngineException(nullptr, "Internal Error: Data::parsestr(), line:" + line);
                    }
                }
                importData->push_back(d);
            }
        }

        void getSymbols(std::map<std::string, ExprEngine::ValuePtr> &symbols, ExprEngine::ValuePtr val) const {
            if (!val)
                return;
            symbols[val->name] = val;
            if (auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(val)) {
                for (auto sizeValue: arrayValue->size)
                    getSymbols(symbols, sizeValue);
                for (auto indexValue: arrayValue->data) {
                    getSymbols(symbols, indexValue.index);
                    getSymbols(symbols, indexValue.value);
                }
            }
            if (auto structValue = std::dynamic_pointer_cast<ExprEngine::StructValue>(val)) {
                for (auto memberNameValue: structValue->member)
                    getSymbols(symbols, memberNameValue.second);
            }
        }
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
{
    this->size.push_back(size);
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

    const Token *initToken = var ? var->nameToken() : nullptr;
    while (initToken && initToken->str() != "=")
        initToken = initToken->astParent();

    ValuePtr val;
    if (var && !var->isGlobal() && !var->isStatic() && !(var->isArgument() && var->isConst()) && !initToken)
        val = std::make_shared<ExprEngine::UninitValue>();
    else if (var && var->valueType()) {
        ::ValueType vt(*var->valueType());
        vt.pointer = 0;
        val = getValueRangeFromValueType(data->getNewSymbolName(), &vt, *data->settings);
    }
    assign(ExprEngine::ValuePtr(), val);
}

ExprEngine::ArrayValue::ArrayValue(const std::string &name, const ExprEngine::ArrayValue &arrayValue)
    : Value(name, ExprEngine::ValueType::ArrayValue)
    , pointer(arrayValue.pointer), nullPointer(arrayValue.nullPointer), uninitPointer(arrayValue.uninitPointer)
    , data(arrayValue.data), size(arrayValue.size)
{
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
        if (index) {
            // Remove old item that will be "overwritten"
            for (size_t i = 0; i < data.size(); ++i) {
                if (data[i].index && data[i].index->name == index->name) {
                    data.erase(data.begin() + i);
                    break;
                }
            }
        }

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
             << "]=";
        if (indexAndValue.value->type == ExprEngine::ValueType::StructValue)
            ostr << "("
                 << indexAndValue.value->name
                 << ","
                 << indexAndValue.value->getSymbolicExpression()
                 << ")";
        else
            ostr << indexAndValue.value->name;
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

class ExprData {
public:
    typedef std::map<std::string, z3::expr> ValueExpr;
    typedef std::vector<z3::expr> AssertionList;

    class BailoutValueException: public ExprEngineException {
    public:
        BailoutValueException() : ExprEngineException(nullptr, "Incomplete analysis") {}
    };

    z3::context context;
    ValueExpr valueExpr;
    AssertionList assertionList;

    void addAssertions(z3::solver &solver) const {
        for (auto assertExpr : assertionList)
            solver.add(assertExpr);
    }

    void addConstraints(z3::solver &solver, const Data* data) {
        for (auto constraint : data->constraints) {
            try {
                solver.add(getConstraintExpr(constraint));
            } catch (const BailoutValueException &) {
            }
        }
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
        z3::expr e = z3_fp_const(name);
        valueExpr.emplace(name, e);
        return e;
    }

    z3::expr getExpr(const ExprEngine::BinOpResult *b) {
        auto op1 = getExpr(b->op1);
        auto op2 = getExpr(b->op2);

        // floating point promotion
        if (b->binop != "&&" && b->binop != "||" && b->binop != "<<" && b->binop != ">>") {
            if (z3_is_fp(op1) || z3_is_fp(op2)) {
                z3_to_fp(op1);
                z3_to_fp(op2);
            }
        }

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
        throw ExprEngineException(nullptr, "Internal error: Unhandled operator " + b->binop);
    }

    z3::expr getExpr(ExprEngine::ValuePtr v) {
        if (!v)
            throw ExprEngineException(nullptr, "Can not solve expressions, operand value is null");
        if (v->type == ExprEngine::ValueType::BailoutValue)
            throw BailoutValueException();
        if (auto intRange = std::dynamic_pointer_cast<ExprEngine::IntRange>(v)) {
            if (intRange->name[0] != '$')
                return z3_int_val(intRange->minValue);
            auto it = valueExpr.find(v->name);
            if (it != valueExpr.end())
                return it->second;
            return addInt(v->name, intRange->minValue, intRange->maxValue);
        }

        if (auto floatRange = std::dynamic_pointer_cast<ExprEngine::FloatRange>(v)) {
            if (floatRange->name[0] != '$')
                return z3_fp_val(floatRange->minValue, floatRange->name);

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
                throw ExprEngineException(nullptr, "ConditionalValue is empty");

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

        throw ExprEngineException(nullptr, "Internal error: Unhandled value type");
    }

    z3::expr getConstraintExpr(ExprEngine::ValuePtr v) {
        if (v->type == ExprEngine::ValueType::IntRange)
            return (getExpr(v) != 0);
        return bool_expr(getExpr(v));
    }

    z3::expr bool_expr(z3::expr e) {
        if (e.is_bool())
            return e;

        // Workaround for z3 bug: https://github.com/Z3Prover/z3/issues/4905
        if (z3_is_fp(e))
            return e != z3_fp_val(0.0, "0.0");

        return e != 0;
    }

    z3::expr int_expr(z3::expr e) {
        if (e.is_bool())
            return z3::ite(e, context.int_val(1), context.int_val(0));
        return e;
    }

    // Wrapper functions for Z3 interface. Instead of having ifdefs embedded
    // in the code we have wrapper functions with ifdefs. The code that use
    // these will be cleaner and hopefully more robust.

    z3::expr z3_fp_const(const std::string &name) {
        return context.real_const(name.c_str());
    }

    z3::expr z3_fp_val(long double value, std::string name) {
        (void)value;
        while (name.size() > 1 && (name.back() == 'f' || name.back() == 'F' || name.back() == 'l' || name.back() == 'L'))
            name.erase(name.size() - 1);
        return context.real_val(name.c_str());
    }

    bool z3_is_fp(z3::expr e) const {
        return e.is_real();
    }

    void z3_to_fp(z3::expr &e) {
        if (e.is_int())
            e = z3::to_real(e);
    }


    z3::expr z3_int_val(int128_t value) {
#if Z3_VERSION_INT >= GET_VERSION_INT(4,7,1)
        return context.int_val(int64_t(value));
#else
        return context.int_val((long long)(value));
#endif
    }
};
#endif

bool ExprEngine::IntRange::isEqual(const DataBase *dataBase, int value) const
{
    if (value < minValue || value > maxValue)
        return false;

    const Data *data = dynamic_cast<const Data *>(dataBase);
    if (data->constraints.empty())
        return true;
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addInt(name, minValue, maxValue);
        exprData.addConstraints(solver, data);
        exprData.addAssertions(solver);
        solver.add(e == value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::IntRange::isGreaterThan(const DataBase *dataBase, int value) const
{
    if (maxValue <= value)
        return false;

    const Data *data = dynamic_cast<const Data *>(dataBase);
    if (data->constraints.empty())
        return true;
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addInt(name, minValue, maxValue);
        exprData.addConstraints(solver, data);
        exprData.addAssertions(solver);
        solver.add(e > value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::IntRange::isLessThan(const DataBase *dataBase, int value) const
{
    if (minValue >= value)
        return false;

    const Data *data = dynamic_cast<const Data *>(dataBase);
    if (data->constraints.empty())
        return true;
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addInt(name, minValue, maxValue);
        exprData.addConstraints(solver, data);
        exprData.addAssertions(solver);
        solver.add(e < value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::FloatRange::isEqual(const DataBase *dataBase, int value) const
{
    if (MathLib::isFloat(name)) {
        float f = MathLib::toDoubleNumber(name);
        return value >= f - 0.00001 && value <= f + 0.00001;
    }
    const Data *data = dynamic_cast<const Data *>(dataBase);
    if (data->constraints.empty())
        return true;
#ifdef USE_Z3
    // Check the value against the constraints
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        z3::expr e = exprData.addFloat(name);
        exprData.addConstraints(solver, data);
        exprData.addAssertions(solver);
        // Workaround for z3 bug: https://github.com/Z3Prover/z3/issues/4905
#if Z3_VERSION_INT >= GET_VERSION_INT(4,8,0)
        z3::expr val_e = exprData.context.fpa_val(static_cast<double>(value));
#else
        z3::expr val_e = exprData.context.real_val(value);
#endif // Z3_VERSION_INT
        solver.add(e == val_e);
        return solver.check() != z3::unsat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::FloatRange::isGreaterThan(const DataBase *dataBase, int value) const
{
    if (value < minValue || value > maxValue)
        return false;

    const Data *data = dynamic_cast<const Data *>(dataBase);
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
        exprData.addConstraints(solver, data);
        exprData.addAssertions(solver);
        solver.add(e > value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}

bool ExprEngine::FloatRange::isLessThan(const DataBase *dataBase, int value) const
{
    if (value < minValue || value > maxValue)
        return false;

    const Data *data = dynamic_cast<const Data *>(dataBase);
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
        exprData.addConstraints(solver, data);
        exprData.addAssertions(solver);
        solver.add(e < value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    // The value may or may not be in range
    return false;
#endif
}


bool ExprEngine::BinOpResult::isEqual(const ExprEngine::DataBase *dataBase, int value) const
{
#ifdef USE_Z3
    try {
        ExprData exprData;
        z3::solver solver(exprData.context);
        z3::expr e = exprData.getExpr(this);
        exprData.addConstraints(solver, dynamic_cast<const Data *>(dataBase));
        exprData.addAssertions(solver);
        solver.add(exprData.int_expr(e) == value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3:" << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    (void)dataBase;
    (void)value;
    return false;
#endif
}

bool ExprEngine::BinOpResult::isGreaterThan(const ExprEngine::DataBase *dataBase, int value) const
{
#ifdef USE_Z3
    try {
        ExprData exprData;
        z3::solver solver(exprData.context);
        z3::expr e = exprData.getExpr(this);
        exprData.addConstraints(solver, dynamic_cast<const Data *>(dataBase));
        exprData.addAssertions(solver);
        solver.add(e > value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3:" << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    (void)dataBase;
    (void)value;
    return false;
#endif
}

bool ExprEngine::BinOpResult::isLessThan(const ExprEngine::DataBase *dataBase, int value) const
{
#ifdef USE_Z3
    try {
        ExprData exprData;
        z3::solver solver(exprData.context);
        z3::expr e = exprData.getExpr(this);
        exprData.addConstraints(solver, dynamic_cast<const Data *>(dataBase));
        exprData.addAssertions(solver);
        solver.add(e < value);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3:" << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    (void)dataBase;
    (void)value;
    return false;
#endif
}

bool ExprEngine::BinOpResult::isTrue(const ExprEngine::DataBase *dataBase) const
{
#ifdef USE_Z3
    try {
        ExprData exprData;
        z3::solver solver(exprData.context);
        z3::expr e = exprData.getExpr(this);
        exprData.addConstraints(solver, dynamic_cast<const Data *>(dataBase));
        exprData.addAssertions(solver);
        solver.add(exprData.int_expr(e) != 0);
        return solver.check() == z3::sat;
    } catch (const z3::exception &exception) {
        std::cerr << "z3:" << exception << std::endl;
        return true;  // Safe option is to return true
    } catch (const ExprData::BailoutValueException &) {
        return true;  // Safe option is to return true
    } catch (const ExprEngineException &) {
        return true;  // Safe option is to return true
    }
#else
    (void)dataBase;
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
        exprData.addConstraints(solver, dynamic_cast<const Data *>(dataBase));
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

static ExprEngine::ValuePtr getValueRangeFromValueType(const ValueType *valueType, Data &data)
{
    if (valueType && valueType->pointer) {
        ExprEngine::ValuePtr val = std::make_shared<ExprEngine::BailoutValue>();
        auto bufferSize = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 1, ExprEngine::ArrayValue::MAXSIZE);
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), bufferSize, val, true, true, false);
    }

    if (!valueType || valueType->pointer)
        return ExprEngine::ValuePtr();
    if (valueType->container) {
        ExprEngine::ValuePtr value;
        if (valueType->container->stdStringLike)
            value = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), -128, 127);
        else if (valueType->containerTypeToken) {
            ValueType vt = ValueType::parseDecl(valueType->containerTypeToken, data.settings);
            value = getValueRangeFromValueType(&vt, data);
        } else
            return ExprEngine::ValuePtr();
        auto bufferSize = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 0, ExprEngine::ArrayValue::MAXSIZE);
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), bufferSize, value, false, false, false);
    }
    return getValueRangeFromValueType(data.getNewSymbolName(), valueType, *data.settings);
}

static void call(const std::vector<ExprEngine::Callback> &callbacks, const Token *tok, ExprEngine::ValuePtr value, Data *dataBase)
{
    if (value) {
        for (ExprEngine::Callback f : callbacks) {
            try {
                f(tok, *value, dataBase);
            } catch (const ExprEngineException &e) {
                throw ExprEngineException(tok, e.what);
            }
        }
    }
}

static ExprEngine::ValuePtr executeExpression(const Token *tok, Data &data);
static ExprEngine::ValuePtr executeExpression1(const Token *tok, Data &data);
static std::string execute(const Token *start, const Token *end, Data &data);

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

static void assignExprValue(const Token *expr, ExprEngine::ValuePtr value, Data &data)
{
    if (!expr)
        return;
    if (expr->varId() > 0) {
        data.assignValue(expr, expr->varId(), value);
    } else if (expr->str() == "[") {
        // Find array token
        const Token *arrayToken = expr;
        while (Token::simpleMatch(arrayToken, "["))
            arrayToken = arrayToken->astOperand1();
        if (!arrayToken)
            return;
        if (auto arrayValue = data.getArrayValue(arrayToken)) {
            // Is it array initialization?
            if (arrayToken->variable() && arrayToken->variable()->nameToken() == arrayToken) {
                if (value->type == ExprEngine::ValueType::StringLiteralValue)
                    arrayValue->assign(ExprEngine::ValuePtr(), value);
            } else {
                auto indexValue = calculateArrayIndex(expr, data, *arrayValue);
                bool loopAssign = false;
                if (auto loopValue = std::dynamic_pointer_cast<ExprEngine::IntRange>(indexValue)) {
                    if (loopValue->loopScope == expr->scope()) {
                        loopAssign = true;
                        for (auto i = loopValue->minValue; i <= loopValue->maxValue; ++i)
                            arrayValue->assign(std::make_shared<ExprEngine::IntRange>(ExprEngine::str(i), i, i), value);
                    }
                }
                if (!loopAssign)
                    arrayValue->assign(indexValue, value);
            }
        } else {
            const Token * const indexToken = expr->astOperand2();
            auto indexValue = executeExpression(indexToken, data);
            call(data.callbacks, indexToken, indexValue, &data);
        }
    } else if (expr->isUnaryOp("*")) {
        auto pval = executeExpression(expr->astOperand1(), data);
        if (pval && pval->type == ExprEngine::ValueType::AddressOfValue) {
            auto val = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(pval);
            if (val)
                data.assignValue(expr, val->varId, value);
        } else if (pval && pval->type == ExprEngine::ValueType::ArrayValue) {
            auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(pval);
            auto indexValue = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
            arrayValue->assign(indexValue, value);
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
                    arr->assign(offset, value);
                }
            }
        }
    } else if (Token::Match(expr, ". %name%")) {
        auto structVal = executeExpression(expr->astOperand1(), data);
        if (structVal && structVal->type == ExprEngine::ValueType::StructValue)
            data.assignStructMember(expr, &*std::static_pointer_cast<ExprEngine::StructValue>(structVal), expr->next()->str(), value);
    }
}


static ExprEngine::ValuePtr executeAssign(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr rhsValue = executeExpression(tok->astOperand2(), data);

    if (!rhsValue) {
        const ValueType * const vt1 = tok->astOperand1() ? tok->astOperand1()->valueType() : nullptr;
        const ValueType * const vt2 = tok->astOperand2() ? tok->astOperand2()->valueType() : nullptr;

        rhsValue = getValueRangeFromValueType(vt1, data);
        if (!rhsValue && vt2 && vt2->pointer == 0) {
            rhsValue = getValueRangeFromValueType(vt2, data);
            if (rhsValue)
                call(data.callbacks, tok->astOperand2(), rhsValue, &data);
        }
        if (!rhsValue)
            rhsValue = std::make_shared<ExprEngine::BailoutValue>();
    }

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

    assignExprValue(lhsToken, assignValue, data);

    return assignValue;
}

static ExprEngine::ValuePtr executeIncDec(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr beforeValue = executeExpression(tok->astOperand1(), data);
    ExprEngine::ValuePtr assignValue = simplifyValue(std::make_shared<ExprEngine::BinOpResult>(tok->str().substr(0,1), beforeValue, std::make_shared<ExprEngine::IntRange>("1", 1, 1)));
    assignExprValue(tok->astOperand1(), assignValue, data);
    auto retVal = (precedes(tok, tok->astOperand1())) ? assignValue : beforeValue;
    call(data.callbacks, tok, retVal, &data);
    return retVal;
}

#ifdef USE_Z3
static void checkContract(Data &data, const Token *tok, const Function *function, const std::vector<ExprEngine::ValuePtr> &argValues)
{
    ExprData exprData;
    z3::solver solver(exprData.context);
    try {
        // Invert contract, we want to know if the contract might not be met
        try {
            solver.add(z3::ite(exprData.getConstraintExpr(data.executeContract(function, executeExpression1)), exprData.context.bool_val(false), exprData.context.bool_val(true)));
        } catch (const ExprData::BailoutValueException &) {
            throw ExprEngineException(tok, "Internal error: Bailout value used");
        }

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
            const char id[] = "bughuntingFunctionCall";
            const auto contractIt = data.settings->functionContracts.find(function->fullName());
            const std::string functionName = contractIt->first;
            const std::string functionExpects = contractIt->second;
            data.reportError(tok,
                             Severity::SeverityType::error,
                             id,
                             "Function '" + function->name() + "' is called, can not determine that its contract '" + functionExpects + "' is always met.",
                             CWE(0),
                             false,
                             bailoutValue,
                             functionName);
        }
    } catch (const z3::exception &exception) {
        std::cerr << "z3: " << exception << std::endl;
    } catch (const ExprEngineException &) {
        const char id[] = "internalErrorInExprEngine";
        const auto contractIt = data.settings->functionContracts.find(function->fullName());
        const std::string functionName = contractIt->first;
        const std::string functionExpects = contractIt->second;
        data.reportError(tok,
                         Severity::SeverityType::error,
                         id,
                         "Function '" + function->name() + "' is called, can not determine that its contract '" + functionExpects + "' is always met.",
                         CWE(0),
                         false,
                         true,
                         functionName);
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

    bool hasBody = tok->astOperand1()->function() && tok->astOperand1()->function()->hasBody();
    if (hasBody) {
        const Scope *functionScope = tok->scope();
        while (functionScope->isExecutable() && functionScope->type != Scope::ScopeType::eFunction)
            functionScope = functionScope->nestedIn;
        if (functionScope == tok->astOperand1()->function()->functionScope)
            hasBody = false;
        for (const auto &errorPathItem: data.errorPath) {
            if (errorPathItem.first == tok) {
                hasBody = false;
                break;
            }
        }
    }

    const std::vector<const Token *> &argTokens = getArguments(tok);
    std::vector<ExprEngine::ValuePtr> argValues;
    for (const Token *argtok : argTokens) {
        auto val = hasBody ? executeExpression1(argtok, data) : executeExpression(argtok, data);
        argValues.push_back(val);
        if (hasBody)
            continue;
        if (!argtok->valueType() || (argtok->valueType()->constness & 1) == 1)
            continue;
        if (auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(val)) {
            ValueType vt(*argtok->valueType());
            vt.pointer = 0;
            auto anyVal = getValueRangeFromValueType(&vt, data);
            arrayValue->assign(ExprEngine::ValuePtr(), anyVal);
        } else if (auto addressOf = std::dynamic_pointer_cast<ExprEngine::AddressOfValue>(val)) {
            ValueType vt(*argtok->valueType());
            vt.pointer = 0;
            if (vt.isIntegral() && argtok->valueType()->pointer == 1)
                data.assignValue(argtok, addressOf->varId, getValueRangeFromValueType(&vt, data));
        }
    }

    call(data.callbacks, tok, std::make_shared<ExprEngine::FunctionCallArgumentValues>(argValues), &data);

    if (tok->astOperand1()->function()) {
        const Function *function = tok->astOperand1()->function();
        const std::string &functionName = function->fullName();
        const auto contractIt = data.settings->functionContracts.find(functionName);
        if (contractIt != data.settings->functionContracts.end()) {
#ifdef USE_Z3
            checkContract(data, tok, function, argValues);
#endif
        } else if (!argValues.empty()) {
            bool bailout = false;
            for (const auto &v: argValues)
                bailout |= (v && v->type == ExprEngine::ValueType::BailoutValue);
            if (!bailout)
                data.addMissingContract(functionName);
        }

        // Execute subfunction..
        if (hasBody) {
            const Scope * const functionScope = function->functionScope;
            int argnr = 0;
            std::map<const Token *, nonneg int> refs;
            for (const Variable &arg: function->argumentList) {
                if (argnr < argValues.size() && arg.declarationId() > 0) {
                    if (arg.isReference())
                        refs[argTokens[argnr]] = arg.declarationId();
                    else
                        argValues[argnr] = translateUninitValueToRange(argValues[argnr], arg.valueType(), data);
                    data.assignValue(function->functionScope->bodyStart, arg.declarationId(), argValues[argnr]);
                }
                // TODO default values!
                argnr++;
            }
            data.contractConstraints(function, executeExpression1);
            data.errorPath.push_back(ErrorPathItem(tok, "Calling " + function->name()));
            try {
                data.load(execute(functionScope->bodyStart, functionScope->bodyEnd, data));
                for (auto ref: refs) {
                    auto v = data.getValue(ref.second, nullptr, nullptr);
                    assignExprValue(ref.first, v, data);
                }
            } catch (ExprEngineException &e) {
                data.errorPath.pop_back();
                e.tok = tok;
                throw e;
            }
            data.errorPath.pop_back();
        }
    }

    else if (const auto *f = data.settings->library.getAllocFuncInfo(tok->astOperand1())) {
        if (!f->initData) {
            const std::string name = data.getNewSymbolName();
            auto size = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 1, ~0U);
            auto val = std::make_shared<ExprEngine::UninitValue>();
            auto result = std::make_shared<ExprEngine::ArrayValue>(name, size, val, false, false, false);
            call(data.callbacks, tok, result, &data);
            data.functionCall();
            return result;
        }
    }

    auto result = getValueRangeFromValueType(tok->valueType(), data);
    call(data.callbacks, tok, result, &data);
    data.functionCall();
    return result;
}

static ExprEngine::ValuePtr executeArrayIndex(const Token *tok, Data &data)
{
    if (tok->tokType() == Token::eLambda)
        throw ExprEngineException(tok, "FIXME: lambda");
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

        auto range = std::make_shared<ExprEngine::UninitValue>();

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

    val = getValueRangeFromValueType(tok->valueType(), data);
    call(data.callbacks, tok, val, &data);
    return val;
}

static ExprEngine::ValuePtr executeDot(const Token *tok, Data &data)
{
    if (!tok->astOperand1()) {
        auto v = std::make_shared<ExprEngine::BailoutValue>();
        call(data.callbacks, tok, v, &data);
        return v;
    }
    std::shared_ptr<ExprEngine::StructValue> structValue = std::dynamic_pointer_cast<ExprEngine::StructValue>(executeExpression(tok->astOperand1(), data));
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

        auto v = getValueRangeFromValueType(tok->valueType(), data);
        if (!v)
            v = std::make_shared<ExprEngine::BailoutValue>();
        call(data.callbacks, tok, v, &data);
        return v;
    }
    call(data.callbacks, tok->astOperand1(), structValue, &data);
    ExprEngine::ValuePtr memberValue = structValue->getValueOfMember(tok->astOperand2()->str());
    call(data.callbacks, tok, memberValue, &data);
    return memberValue;
}

static void streamReadSetValue(const Token *tok, Data &data)
{
    if (!tok || !tok->valueType())
        return;
    auto rangeValue = getValueRangeFromValueType(tok->valueType(), data);
    if (rangeValue)
        assignExprValue(tok, rangeValue, data);
}

static ExprEngine::ValuePtr executeStreamRead(const Token *tok, Data &data)
{
    tok = tok->astOperand2();
    while (Token::simpleMatch(tok, ">>")) {
        streamReadSetValue(tok->astOperand1(), data);
        tok = tok->astOperand2();
    }
    streamReadSetValue(tok, data);
    return ExprEngine::ValuePtr();
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
        auto v = getValueRangeFromValueType(tok->valueType(), data);
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

static ExprEngine::ValuePtr executeNot(const Token *tok, Data &data)
{
    ExprEngine::ValuePtr v = executeExpression(tok->astOperand1(), data);
    if (!v)
        return v;
    ExprEngine::ValuePtr zero = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
    auto result = simplifyValue(std::make_shared<ExprEngine::BinOpResult>("==", v, zero));
    call(data.callbacks, tok, result, &data);
    return result;
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
    if (data.settings->terminated())
        throw TerminateExpression();

    if (tok->str() == "return")
        return executeReturn(tok, data);

    if (tok->isAssignmentOp())
        // TODO: Handle more operators
        return executeAssign(tok, data);

    if (tok->tokType() == Token::Type::eIncDecOp)
        return executeIncDec(tok, data);

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

    if (data.tokenizer->isCPP() && tok->str() == ">>" && !tok->astParent() && tok->isBinaryOp() && Token::Match(tok->astOperand1(), "%name%|::"))
        return executeStreamRead(tok, data);

    if (tok->astOperand1() && tok->astOperand2())
        return executeBinaryOp(tok, data);

    if (tok->isUnaryOp("&") && Token::Match(tok->astOperand1(), "%var%"))
        return executeAddressOf(tok, data);

    if (tok->isUnaryOp("*"))
        return executeDeref(tok, data);

    if (tok->isUnaryOp("!"))
        return executeNot(tok, data);

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

static std::tuple<bool, bool> checkConditionBranches(const ExprEngine::ValuePtr &condValue, const Data &data)
{
    bool canBeFalse = true;
    bool canBeTrue = true;
    if (auto b = std::dynamic_pointer_cast<ExprEngine::BinOpResult>(condValue)) {
        canBeFalse = b->isEqual(&data, 0);
        canBeTrue = b->isTrue(&data);
    } else if (auto i = std::dynamic_pointer_cast<ExprEngine::IntRange>(condValue)) {
        canBeFalse = i->isEqual(&data, 0);
        canBeTrue = ExprEngine::BinOpResult("!=", i, std::make_shared<ExprEngine::IntRange>("0", 0, 0)).isTrue(&data);
    } else if (std::dynamic_pointer_cast<ExprEngine::StringLiteralValue>(condValue)) {
        canBeFalse = false;
        canBeTrue = true;
    } else if (auto f = std::dynamic_pointer_cast<ExprEngine::FloatRange>(condValue)) {
        canBeFalse = f->isEqual(&data, 0);
        canBeTrue = ExprEngine::BinOpResult("!=", f, std::make_shared<ExprEngine::FloatRange>("0.0", 0.0, 0.0)).isTrue(&data);
    }
    return std::make_tuple(canBeFalse, canBeTrue);
}

static std::string execute(const Token *start, const Token *end, Data &data)
{
    if (data.recursion > 20)
        // FIXME
        return data.str();

    // Update data.recursion
    struct Recursion {
        Recursion(int *var, int value) : var(var), value(value) {
            *var = value + 1;
        }
        ~Recursion() {
            if (*var >= value) *var = value;
        }
        int *var;
        int value;
    };
    Recursion updateRecursion(&data.recursion, data.recursion);

    const std::time_t stopTime = data.startTime + data.settings->bugHuntingCheckFunctionMaxTime;

    for (const Token *tok = start; tok != end; tok = tok->next()) {
        if (Token::Match(tok, "[;{}]")) {
            data.trackProgramState(tok);
            if (tok->str() == ";") {
                const Token *prev = tok->previous();
                while (prev && !Token::Match(prev, "[;{}]"))
                    prev = prev->previous();
                if (Token::Match(prev, "[;{}] return|throw"))
                    return data.str();
            }
            if (std::time(nullptr) > stopTime)
                return "";
        }

        if (Token::simpleMatch(tok, "__CPPCHECK_BAILOUT__ ;"))
            // This is intended for testing
            throw ExprEngineException(tok, "__CPPCHECK_BAILOUT__");

        if (Token::simpleMatch(tok, "while (") && Token::simpleMatch(tok->linkAt(1), ") ;") && tok->next()->astOperand1()->hasKnownIntValue() && tok->next()->astOperand1()->getKnownIntValue() == 0) {
            tok = tok->tokAt(4);
            continue;
        }

        if (tok->str() == "break") {
            const Scope *scope = tok->scope();
            while (scope->type == Scope::eIf || scope->type == Scope::eElse)
                scope = scope->nestedIn;
            tok = scope->bodyEnd;
            if (!precedes(tok,end))
                return data.str();
        }

        if (Token::simpleMatch(tok, "try"))
            // TODO this is a bailout
            throw ExprEngineException(tok, "Unhandled:" + tok->str());

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
                data.assignValue(tok, tok->varId(), createVariableValue(*tok->variable(), data));
            } else if (tok->variable()->isArray()) {
                data.assignValue(tok, tok->varId(), std::make_shared<ExprEngine::ArrayValue>(&data, tok->variable()));
                if (Token::Match(tok, "%name% ["))
                    tok = tok->linkAt(1);
            } else if (Token::Match(tok, "%var% ;"))
                data.assignValue(tok, tok->varId(), createVariableValue(*tok->variable(), data));
        } else if (!tok->astParent() && (tok->astOperand1() || tok->astOperand2())) {
            executeExpression(tok, data);
            if (Token::Match(tok, "throw|return"))
                return data.str();
        }

        else if (Token::simpleMatch(tok, "if (")) {
            const Token *cond = tok->next()->astOperand2(); // TODO: C++17 condition
            const ExprEngine::ValuePtr condValue = executeExpression(cond, data);

            bool canBeFalse, canBeTrue;
            std::tie(canBeFalse, canBeTrue) = checkConditionBranches(condValue, data);

            Data &thenData(data);
            Data elseData(data);
            if (canBeFalse && canBeTrue) { // Avoid that constraints are overspecified
                thenData.addConstraint(condValue, true);
                elseData.addConstraint(condValue, false);
            }

            Data::ifSplit(tok, thenData, elseData);

            const Token *thenStart = tok->linkAt(1)->next();
            const Token *thenEnd = thenStart->link();

            const Token *exceptionToken = nullptr;
            std::string exceptionMessage;
            auto exec = [&](const Token *tok1, const Token *tok2, Data& data) {
                try {
                    execute(tok1, tok2, data);
                } catch (ExprEngineException &e) {
                    if (!exceptionToken || (e.tok && precedes(e.tok, exceptionToken))) {
                        exceptionToken = e.tok;
                        exceptionMessage = e.what;
                    }
                }
            };

            if (canBeTrue)
                exec(thenStart->next(), end, thenData);

            if (canBeFalse) {
                if (Token::simpleMatch(thenEnd, "} else {")) {
                    const Token *elseStart = thenEnd->tokAt(2);
                    exec(elseStart->next(), end, elseData);
                } else {
                    exec(thenEnd, end, elseData);
                }
            }

            if (exceptionToken)
                throw ExprEngineException(exceptionToken, exceptionMessage);

            return (canBeTrue ? thenData.str() : std::string()) +
                   (canBeFalse ? elseData.str() : std::string());
        }

        else if (Token::simpleMatch(tok, "switch (")) {
            auto condValue = executeExpression(tok->next()->astOperand2(), data); // TODO: C++17 condition
            const Token *bodyStart = tok->linkAt(1)->next();
            const Token *bodyEnd = bodyStart->link();
            const Token *defaultStart = nullptr;
            Data defaultData(data);
            const Token *exceptionToken = nullptr;
            std::string exceptionMessage;
            std::ostringstream ret;
            auto exec = [&](const Token *tok1, const Token *tok2, Data& data) {
                try {
                    execute(tok1, tok2, data);
                    ret << data.str();
                } catch (ExprEngineException &e) {
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
                throw ExprEngineException(exceptionToken, exceptionMessage);
            return ret.str();
        }

        if (Token::simpleMatch(tok, "for (")) {
            nonneg int varid;
            bool hasKnownInitValue, partialCond;
            MathLib::bigint initValue, stepValue, lastValue;
            if (extractForLoopValues(tok, &varid, &hasKnownInitValue, &initValue, &partialCond, &stepValue, &lastValue) && hasKnownInitValue && !partialCond) {
                auto loopValues = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), initValue, lastValue);
                data.assignValue(tok, varid, loopValues);
                tok = tok->linkAt(1);
                loopValues->loopScope = tok->next()->scope();
                // Check whether the condition expression is always false
                if (tok->next() && (initValue > lastValue)) {
                    tok = tok->next()->link();
                }
                continue;
            }
        }

        if (Token::Match(tok, "for|while (") && Token::simpleMatch(tok->linkAt(1), ") {")) {
            const Token *cond = tok->next()->astOperand2();
            const ExprEngine::ValuePtr condValue = executeExpression(cond, data);

            bool canBeFalse = false, canBeTrue = true;
            if (tok->str() == "while")
                std::tie(canBeFalse, canBeTrue) = checkConditionBranches(condValue, data);

            Data &bodyData(data);
            Data noexecData(data);
            if (canBeFalse && canBeTrue) { // Avoid that constraints are overspecified
                bodyData.addConstraint(condValue, true);
            }

            Data::ifSplit(tok, bodyData, noexecData);

            const Token *bodyStart = tok->linkAt(1)->next();
            const Token *bodyEnd = bodyStart->link();

            // TODO this is very rough code
            if (canBeTrue) {
                std::set<int> changedVariables;
                for (const Token *tok2 = tok; tok2 != bodyEnd; tok2 = tok2->next()) {
                    if (Token::Match(tok2, "%assign%")) {
                        const Token *lhs = tok2->astOperand1();
                        while (Token::simpleMatch(lhs, "["))
                            lhs = lhs->astOperand1();
                        if (!lhs)
                            throw ExprEngineException(tok2, "Unhandled assignment in loop");
                        if (Token::Match(lhs, ". %name% =|[") && Token::simpleMatch(lhs->astOperand1(), ".")) {
                            const Token *structToken = lhs;
                            while (Token::Match(structToken, ".|["))
                                structToken = structToken->astOperand1();
                            if (Token::Match(structToken, "%var%")) {
                                bodyData.assignValue(structToken, structToken->varId(), std::make_shared<ExprEngine::BailoutValue>());
                                changedVariables.insert(structToken->varId());
                                continue;
                            }
                        }
                        if (Token::Match(lhs, ". %name% =|[") && lhs->astOperand1() && lhs->astOperand1()->valueType()) {
                            const Token *structToken = lhs->astOperand1();
                            if (!structToken->valueType() || !structToken->varId())
                                throw ExprEngineException(tok2, "Unhandled assignment in loop");
                            const Scope *structScope = structToken->valueType()->typeScope;
                            if (!structScope)
                                throw ExprEngineException(tok2, "Unhandled assignment in loop");
                            const std::string &memberName = tok2->previous()->str();
                            ExprEngine::ValuePtr memberValue;
                            for (const Variable &member : structScope->varlist) {
                                if (memberName == member.name() && member.valueType()) {
                                    memberValue = createVariableValue(member, bodyData);
                                    break;
                                }
                            }
                            if (!memberValue)
                                throw ExprEngineException(tok2, "Unhandled assignment in loop");

                            ExprEngine::ValuePtr structVal1 = bodyData.getValue(structToken->varId(), structToken->valueType(), structToken);
                            if (!structVal1)
                                structVal1 = createVariableValue(*structToken->variable(), bodyData);
                            auto structVal = std::dynamic_pointer_cast<ExprEngine::StructValue>(structVal1);
                            if (!structVal) {
                                // Handle pointer to a struct
                                if (auto structPtr = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(structVal1)) {
                                    if (structPtr->pointer && !structPtr->data.empty()) {
                                        auto indexValue = std::make_shared<ExprEngine::IntRange>("0", 0, 0);
                                        for (auto val: structPtr->read(indexValue)) {
                                            structVal = std::dynamic_pointer_cast<ExprEngine::StructValue>(val.second);
                                        }
                                    }
                                }
                                if (!structVal)
                                    throw ExprEngineException(tok2, "Unhandled assignment in loop");
                            }

                            bodyData.assignStructMember(tok2, &*structVal, memberName, memberValue);
                            continue;
                        }
                        if (lhs->isUnaryOp("*") && lhs->astOperand1()->varId()) {
                            const Token *varToken = tok2->astOperand1()->astOperand1();
                            ExprEngine::ValuePtr val = bodyData.getValue(varToken->varId(), varToken->valueType(), varToken);
                            if (val && val->type == ExprEngine::ValueType::ArrayValue) {
                                // Try to assign "any" value
                                auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(val);
                                arrayValue->assign(std::make_shared<ExprEngine::IntRange>("0", 0, 0), std::make_shared<ExprEngine::BailoutValue>());
                                continue;
                            }
                        }
                        if (!lhs->variable())
                            throw ExprEngineException(tok2, "Unhandled assignment in loop");
                        // give variable "any" value
                        int varid = lhs->varId();
                        if (changedVariables.find(varid) != changedVariables.end())
                            continue;
                        changedVariables.insert(varid);
                        auto oldValue = bodyData.getValue(varid, nullptr, nullptr);
                        if (oldValue && oldValue->isUninit())
                            call(bodyData.callbacks, lhs, oldValue, &bodyData);
                        if (oldValue && oldValue->type == ExprEngine::ValueType::ArrayValue) {
                            // Try to assign "any" value
                            auto arrayValue = std::dynamic_pointer_cast<ExprEngine::ArrayValue>(oldValue);
                            arrayValue->assign(std::make_shared<ExprEngine::IntRange>(bodyData.getNewSymbolName(), 0, ~0ULL), std::make_shared<ExprEngine::BailoutValue>());
                            continue;
                        }
                        bodyData.assignValue(tok2, varid, getValueRangeFromValueType(lhs->valueType(), bodyData));
                        continue;
                    } else if (Token::Match(tok2, "++|--") && tok2->astOperand1() && tok2->astOperand1()->variable()) {
                        // give variable "any" value
                        const Token *vartok = tok2->astOperand1();
                        int varid = vartok->varId();
                        if (changedVariables.find(varid) != changedVariables.end())
                            continue;
                        changedVariables.insert(varid);
                        auto oldValue = bodyData.getValue(varid, nullptr, nullptr);
                        if (oldValue && oldValue->type == ExprEngine::ValueType::UninitValue)
                            call(bodyData.callbacks, tok2, oldValue, &bodyData);
                        bodyData.assignValue(tok2, varid, getValueRangeFromValueType(vartok->valueType(), bodyData));
                    }
                }
            }

            const Token *exceptionToken = nullptr;
            std::string exceptionMessage;
            auto exec = [&](const Token *tok1, const Token *tok2, Data& data) {
                try {
                    execute(tok1, tok2, data);
                } catch (ExprEngineException &e) {
                    if (!exceptionToken || (e.tok && precedes(e.tok, exceptionToken))) {
                        exceptionToken = e.tok;
                        exceptionMessage = e.what;
                    }
                }
            };

            if (canBeTrue)
                exec(bodyStart->next(), end, bodyData);
            if (canBeFalse)
                exec(bodyEnd, end, noexecData);

            if (exceptionToken)
                throw ExprEngineException(exceptionToken, exceptionMessage);

            return (canBeTrue ? bodyData.str() : std::string()) +
                   (canBeFalse ? noexecData.str() : std::string());
        }

        if (Token::simpleMatch(tok, "} else {"))
            tok = tok->linkAt(2);
    }

    return data.str();
}

void ExprEngine::executeAllFunctions(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings, const std::vector<ExprEngine::Callback> &callbacks, std::ostream &report)
{
    const SymbolDatabase *symbolDatabase = tokenizer->getSymbolDatabase();
    for (const Scope *functionScope : symbolDatabase->functionScopes) {
        try {
            executeFunction(functionScope, errorLogger, tokenizer, settings, callbacks, report);
        } catch (const ExprEngineException &e) {
            // FIXME.. there should not be exceptions
            std::string functionName = functionScope->function->name();
            std::cout << "Verify: Aborted analysis of function '" << functionName << "':" << e.tok->linenr() << ": " << e.what << std::endl;
        } catch (const std::exception &e) {
            // FIXME.. there should not be exceptions
            std::string functionName = functionScope->function->name();
            std::cout << "Verify: Aborted analysis of function '" << functionName << "': " << e.what() << std::endl;
        } catch (const TerminateExpression &) {
            break;
        }
    }
}

static ExprEngine::ValuePtr createStructVal(const Token *tok, const Scope *structScope, bool uninitData, Data &data)
{
    if (!structScope)
        return ExprEngine::ValuePtr();
    std::shared_ptr<ExprEngine::StructValue> structValue = std::make_shared<ExprEngine::StructValue>(data.getNewSymbolName());
    auto uninitValue = std::make_shared<ExprEngine::UninitValue>();
    for (const Variable &member : structScope->varlist) {
        if (uninitData && !member.isInit()) {
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
                data.assignStructMember(tok, structValue.get(), member.name(), memberValue);
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
            pointerValue = createStructVal(var.nameToken(), valueType->typeScope, var.isLocal() && !var.isStatic(), data);
        else {
            ValueType vt(*valueType);
            vt.pointer = 0;
            if (vt.constness & 1)
                pointerValue = getValueRangeFromValueType(&vt, data);
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
            value = getValueRangeFromValueType(valueType, data);
        data.addConstraints(value, var.nameToken());
        return value;
    }
    if (valueType->type == ValueType::Type::RECORD) {
        bool uninitData = true;
        if (var.isLocal() && !var.isStatic()) {
            uninitData = !valueType->typeScope ||
                         !valueType->typeScope->definedType ||
                         valueType->typeScope->definedType->needInitialization != Type::NeedInitialization::False;
        }
        if (var.isArgument() && var.isConst())
            uninitData = false;
        return createStructVal(var.nameToken(), valueType->typeScope, uninitData, data);
    }
    if (valueType->smartPointerType) {
        auto structValue = createStructVal(var.nameToken(), valueType->smartPointerType->classScope, var.isLocal() && !var.isStatic(), data);
        auto size = std::make_shared<ExprEngine::IntRange>(data.getNewSymbolName(), 1, ~0UL);
        return std::make_shared<ExprEngine::ArrayValue>(data.getNewSymbolName(), size, structValue, true, true, false);
    }
    return getValueRangeFromValueType(valueType, data);
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

    const std::time_t stopTime = data.startTime + data.settings->bugHuntingCheckFunctionMaxTime;

    try {
        execute(functionScope->bodyStart, functionScope->bodyEnd, data);
    } catch (const ExprEngineException &e) {
        if (settings->debugBugHunting)
            report << "ExprEngineException " << e.tok->linenr() << ":" << e.tok->column() << ": " << e.what << "\n";
        trackExecution.setAbortLine(e.tok->linenr());
        auto bailoutValue = std::make_shared<BailoutValue>();
        for (const Token *tok = e.tok; tok != functionScope->bodyEnd; tok = tok->next()) {
            if (std::time(nullptr) >= stopTime)
                break;
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
        std::set<std::string> intvars;
        for (const Scope &scope: tokenizer->getSymbolDatabase()->scopeList) {
            if (scope.isExecutable())
                continue;
            std::string path;
            bool valid = true;
            for (const Scope *s = &scope; s->type != Scope::ScopeType::eGlobal; s = s->nestedIn) {
                if (s->isExecutable()) {
                    valid = false;
                    break;
                }
                path = s->className + "::" + path;
            }
            if (!valid)
                continue;
            for (const Variable &var: scope.varlist) {
                if (var.nameToken() && !var.nameToken()->hasCppcheckAttributes() && var.valueType() && var.valueType()->pointer == 0 && var.valueType()->constness == 0 && var.valueType()->isIntegral())
                    intvars.insert(path + var.name());
            }
        }
        for (const std::string &v: intvars)
            report << "[intvar] " << v << std::endl;
        for (const std::string &f: trackExecution.getMissingContracts())
            report << "[missing contract] " << f << std::endl;
    }
}

void ExprEngine::runChecks(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings)
{

    std::vector<ExprEngine::Callback> callbacks;
    addBughuntingChecks(&callbacks);

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
    case ExprEngine::ValueType::FunctionCallArgumentValues: {
        std::cout << "FunctionCallArgumentValues(";
        const char *sep = "";
        for (auto arg: std::dynamic_pointer_cast<ExprEngine::FunctionCallArgumentValues>(val)->argValues) {
            std::cout << sep;
            sep = ",";
            if (!arg)
                std::cout << "NULL";
            else
                dumpRecursive(arg);
        }
        std::cout << ")";
    }
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



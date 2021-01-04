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

//---------------------------------------------------------------------------
#ifndef exprengineH
#define exprengineH
//---------------------------------------------------------------------------

#include "config.h"
#include "errortypes.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

class ErrorLogger;
class Tokenizer;
class Scope;
class Settings;
class Token;
class Variable;

#if defined(__GNUC__) && defined (__SIZEOF_INT128__)
typedef __int128_t   int128_t;
#else
typedef long long    int128_t;
#ifdef _MSC_VER
#pragma message(__FILE__ "(" _CRT_STRINGIZE(__LINE__) ")" ": warning: TODO No 128-bit integer type is available => Limited analysis of large integers...")
#else
#warning TODO No 128-bit integer type is available => Limited analysis of large integers
#endif
#endif

namespace ExprEngine {
    std::string str(int128_t);

    // TODO we need to handle floats, containers, pointers, aliases and structs and stuff
    enum class ValueType {
        UninitValue,
        IntRange,
        FloatRange,
        ConditionalValue,
        ArrayValue,
        StringLiteralValue,
        StructValue,
        AddressOfValue,
        BinOpResult,
        IntegerTruncation,
        FunctionCallArgumentValues,
        BailoutValue
    };

    class Value;
    typedef std::shared_ptr<Value> ValuePtr;

    class DataBase {
    public:
        explicit DataBase(const std::string &currentFunction, const Settings *settings)
            : currentFunction(currentFunction)
            , settings(settings) {
        }
        virtual std::string getNewSymbolName() = 0;
        const std::string currentFunction;
        const Settings * const settings;
        virtual bool isC() const = 0;
        virtual bool isCPP() const = 0;
        virtual void reportError(const Token *tok,
                                 Severity::SeverityType severity,
                                 const char id[],
                                 const std::string &text,
                                 CWE cwe,
                                 bool inconclusive,
                                 bool incomplete=false,
                                 const std::string &functionName = std::string()) = 0;
        ErrorPath errorPath;
    };

    class Value {
    public:
        Value(const std::string &name, const ValueType type) : name(name), type(type) {}
        virtual ~Value() {}
        virtual std::string getRange() const {
            return name;
        }
        virtual std::string getSymbolicExpression() const {
            return name;
        }
        virtual bool isEqual(const DataBase *dataBase, int value) const {
            (void)dataBase;
            (void)value;
            return false;
        }
        virtual bool isGreaterThan(const DataBase *dataBase, int value) const {
            (void)dataBase;
            (void)value;
            return false;
        }
        virtual bool isLessThan(const DataBase *dataBase, int value) const {
            (void)dataBase;
            (void)value;
            return false;
        }
        virtual bool isUninit() const {
            return false;
        }

        const std::string name;
        ValueType type;
    };

    class UninitValue: public Value {
    public:
        UninitValue() : Value("?", ValueType::UninitValue) {}
        bool isEqual(const DataBase *dataBase, int value) const OVERRIDE {
            (void)dataBase;
            (void)value;
            return true;
        }
        bool isUninit() const OVERRIDE {
            return true;
        }
    };

    class IntRange : public Value {
    public:
        IntRange(const std::string &name, int128_t minValue, int128_t maxValue)
            : Value(name, ValueType::IntRange)
            , minValue(minValue)
            , maxValue(maxValue)
            , loopScope(nullptr) {
        }
        std::string getRange() const OVERRIDE {
            if (minValue == maxValue)
                return str(minValue);
            return str(minValue) + ":" + str(maxValue);
        }
        bool isEqual(const DataBase *dataBase, int value) const OVERRIDE;
        bool isGreaterThan(const DataBase *dataBase, int value) const OVERRIDE;
        bool isLessThan(const DataBase *dataBase, int value) const OVERRIDE;

        int128_t minValue;
        int128_t maxValue;
        const Scope *loopScope;
    };

    class FloatRange : public Value {
    public:
        FloatRange(const std::string &name, long double minValue, long double maxValue)
            : Value(name, ValueType::FloatRange)
            , minValue(minValue)
            , maxValue(maxValue) {
        }

        std::string getRange() const OVERRIDE {
            return std::to_string(minValue) + ":" + std::to_string(maxValue);
        }

        bool isEqual(const DataBase *dataBase, int value) const OVERRIDE;
        bool isGreaterThan(const DataBase *dataBase, int value) const OVERRIDE;
        bool isLessThan(const DataBase *dataBase, int value) const OVERRIDE;

        long double minValue;
        long double maxValue;
    };

    class ConditionalValue : public Value {
    public:
        typedef std::vector<std::pair<ValuePtr,ValuePtr>> Vector;

        ConditionalValue(const std::string &name, const Vector &values) : Value(name, ValueType::ConditionalValue), values(values) {}

        std::string getSymbolicExpression() const OVERRIDE;

        Vector values;
    };

    // Array or pointer
    class ArrayValue: public Value {
    public:
        enum { MAXSIZE = 0x7fffffff };

        ArrayValue(const std::string &name, ValuePtr size, ValuePtr value, bool pointer, bool nullPointer, bool uninitPointer);
        ArrayValue(DataBase *data, const Variable *var);
        ArrayValue(const std::string &name, const ArrayValue &arrayValue);

        std::string getRange() const OVERRIDE;
        std::string getSymbolicExpression() const OVERRIDE;

        void assign(ValuePtr index, ValuePtr value);
        void clear();
        ConditionalValue::Vector read(ValuePtr index) const;

        bool pointer;
        bool nullPointer;
        bool uninitPointer;

        struct IndexAndValue {
            ValuePtr index;
            ValuePtr value;
        };
        std::vector<IndexAndValue> data;
        std::vector<ValuePtr> size;
    };

    class StringLiteralValue: public Value {
    public:
        StringLiteralValue(const std::string &name, const std::string &s) : Value(name, ValueType::StringLiteralValue), string(s) {}

        std::string getRange() const OVERRIDE {
            return "\"" + string + "\"";
        }

        int size() const {
            return string.size();
        }
        const std::string string;
    };

    class StructValue: public Value {
    public:
        explicit StructValue(const std::string &name) : Value(name, ValueType::StructValue) {}

        std::string getSymbolicExpression() const OVERRIDE;

        std::string getRange() const OVERRIDE {
            return getSymbolicExpression();
        }

        ValuePtr getValueOfMember(const std::string &n) const {
            auto it = member.find(n);
            return (it == member.end()) ? ValuePtr() : it->second;
        }

        std::string getUninitStructMember() const {
            for (auto memberNameValue: member) {
                if (memberNameValue.second && memberNameValue.second->isUninit())
                    return memberNameValue.first;
            }
            return std::string();
        }

        std::map<std::string, ValuePtr> member;
    };

    class AddressOfValue: public Value {
    public:
        AddressOfValue(const std::string &name, int varId)
            : Value(name, ValueType::AddressOfValue)
            , varId(varId)
        {}

        std::string getRange() const OVERRIDE {
            return "&@" + std::to_string(varId);
        }

        int varId;
    };

    class BinOpResult : public Value {
    public:
        BinOpResult(const std::string &binop, ValuePtr op1, ValuePtr op2)
            : Value(getName(binop, op1, op2), ValueType::BinOpResult)
            , binop(binop)
            , op1(op1)
            , op2(op2) {
        }

        bool isEqual(const DataBase *dataBase, int value) const OVERRIDE;
        bool isGreaterThan(const DataBase *dataBase, int value) const OVERRIDE;
        virtual bool isLessThan(const DataBase *dataBase, int value) const OVERRIDE;
        bool isTrue(const DataBase *dataBase) const;

        std::string getExpr(DataBase *dataBase) const;

        std::string binop;
        ValuePtr op1;
        ValuePtr op2;
    private:
        std::string getName(const std::string &binop, ValuePtr op1, ValuePtr op2) const {
            std::string name1 = op1 ? op1->name : std::string("null");
            std::string name2 = op2 ? op2->name : std::string("null");
            return "(" + name1 + ")" + binop + "(" + name2 + ")";
        }
    };

    class IntegerTruncation : public Value {
    public:
        IntegerTruncation(const std::string &name, ValuePtr inputValue, int bits, char sign)
            : Value(name, ValueType::IntegerTruncation)
            , inputValue(inputValue)
            , bits(bits)
            , sign(sign) {
        }

        std::string getSymbolicExpression() const OVERRIDE;

        ExprEngine::ValuePtr inputValue;
        int bits;
        char sign;
    };

    class FunctionCallArgumentValues: public Value {
    public:
        explicit FunctionCallArgumentValues(const std::vector<ExprEngine::ValuePtr> &argValues)
            : Value("argValues", ValueType::FunctionCallArgumentValues)
            , argValues(argValues)
        {}

        const std::vector<ExprEngine::ValuePtr> argValues;
    };

    class BailoutValue : public Value {
    public:
        BailoutValue() : Value("bailout", ValueType::BailoutValue) {}
        bool isEqual(const DataBase * /*dataBase*/, int /*value*/) const OVERRIDE {
            return true;
        }
        bool isUninit() const OVERRIDE {
            return true;
        }
    };

    typedef std::function<void(const Token *, const ExprEngine::Value &, ExprEngine::DataBase *)> Callback;

    /** Execute all functions */
    void CPPCHECKLIB executeAllFunctions(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings, const std::vector<Callback> &callbacks, std::ostream &report);
    void executeFunction(const Scope *functionScope, ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings, const std::vector<Callback> &callbacks, std::ostream &report);

    void runChecks(ErrorLogger *errorLogger, const Tokenizer *tokenizer, const Settings *settings);

    void dump(ExprEngine::ValuePtr val);
}
#endif // exprengineH

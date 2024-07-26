/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#ifndef vfValueFlowAnalyzer
#define vfValueFlowAnalyzer

#include "analyzer.h"
#include "config.h"
#include "mathlib.h"
#include "programmemory.h"
#include "token.h"
#include "valueptr.h"

#include <string>
#include <unordered_map>
#include <vector>

class Settings;
namespace ValueFlow {
    class Value;
}

struct ValueFlowAnalyzer : Analyzer {
    const Settings& settings;
    ProgramMemoryState pms;

    explicit ValueFlowAnalyzer(const Settings& s) : settings(s), pms(&settings) {}

    virtual const ValueFlow::Value* getValue(const Token* tok) const = 0;
    virtual ValueFlow::Value* getValue(const Token* tok) = 0;

    virtual void makeConditional() = 0;

    virtual void addErrorPath(const Token* tok, const std::string& s) = 0;

    virtual bool match(const Token* tok) const = 0;

    virtual bool internalMatch(const Token* /*tok*/) const {
        return false;
    }

    virtual bool isAlias(const Token* tok, bool& inconclusive) const = 0;

    using ProgramState = ProgramMemory::Map;

    virtual ProgramState getProgramState() const = 0;

    virtual int getIndirect(const Token* tok) const;

    virtual bool isGlobal() const {
        return false;
    }
    virtual bool dependsOnThis() const {
        return false;
    }
    virtual bool isVariable() const {
        return false;
    }

    const Settings& getSettings() const {
        return settings;
    }

    struct ConditionState {
        bool dependent = true;
        bool unknown = true;

        bool isUnknownDependent() const {
            return unknown && dependent;
        }
    };

    ConditionState analyzeCondition(const Token* tok, int depth = 20) const;

    virtual Action isModified(const Token* tok) const;

    virtual Action isAliasModified(const Token* tok, int indirect = -1) const;

    virtual Action isThisModified(const Token* tok) const;

    virtual Action isWritable(const Token* tok, Direction d) const;

    virtual void writeValue(ValueFlow::Value* value, const Token* tok, Direction d) const;

    virtual bool useSymbolicValues() const {
        return true;
    }

    virtual void internalUpdate(Token* /*tok*/, const ValueFlow::Value& /*v*/, Direction /*d*/);

private:
    // Returns Action::Match if its an exact match, return Action::Read if it partially matches the lifetime
    Action analyzeLifetime(const Token* tok) const;

    std::unordered_map<nonneg int, const Token*> getSymbols(const Token* tok) const;

    Action isGlobalModified(const Token* tok) const;

    const Token* findMatch(const Token* tok) const;

    bool isSameSymbolicValue(const Token* tok, ValueFlow::Value* value = nullptr) const;

    Action analyzeMatch(const Token* tok, Direction d) const;

    Action analyzeToken(const Token* ref, const Token* tok, Direction d, bool inconclusiveRef) const;

    Action analyze(const Token* tok, Direction d) const override;

    std::vector<MathLib::bigint> evaluateInt(const Token* tok) const;

    std::vector<MathLib::bigint> evaluate(Evaluate e, const Token* tok, const Token* ctx = nullptr) const override;

    void assume(const Token* tok, bool state, unsigned int flags) override;

    void updateState(const Token* tok) override;

    void update(Token* tok, Action a, Direction d) override;

    ValuePtr<Analyzer> reanalyze(Token* /*tok*/, const std::string& /*msg*/) const override {
        return {};
    }
};

#endif // vfValueFlowAnalyzer

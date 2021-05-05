/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#ifndef analyzerH
#define analyzerH

#include <string>
#include <vector>

class Token;
template <class T>
class ValuePtr;

struct Analyzer {
    struct Action {

        Action() : mFlag(0) {}

        // cppcheck-suppress noExplicitConstructor
        Action(unsigned int f) : mFlag(f) {}

        enum {
            None = 0,
            Read = (1 << 0),
            Write = (1 << 1),
            Invalid = (1 << 2),
            Inconclusive = (1 << 3),
            Match = (1 << 4),
            Idempotent = (1 << 5),
            Incremental = (1 << 6),
        };

        void set(unsigned int f, bool state = true) {
            mFlag = state ? mFlag | f : mFlag & ~f;
        }

        bool get(unsigned int f) const {
            return ((mFlag & f) != 0);
        }

        bool isRead() const {
            return get(Read);
        }

        bool isWrite() const {
            return get(Write);
        }

        bool isInvalid() const {
            return get(Invalid);
        }

        bool isInconclusive() const {
            return get(Inconclusive);
        }

        bool isNone() const {
            return mFlag == None;
        }

        bool isModified() const {
            return isWrite() || isInvalid();
        }

        bool isIdempotent() const {
            return get(Idempotent);
        }

        bool isIncremental() const {
            return get(Incremental);
        }

        bool matches() const {
            return get(Match);
        }

        Action& operator|=(Action a) {
            set(a.mFlag);
            return *this;
        }

        friend Action operator|(Action a, Action b) {
            a |= b;
            return a;
        }

        friend bool operator==(Action a, Action b) {
            return a.mFlag == b.mFlag;
        }

        friend bool operator!=(Action a, Action b) {
            return a.mFlag != b.mFlag;
        }

    private:
        unsigned int mFlag;
    };

    enum class Direction { Forward, Reverse };

    /// Analyze a token
    virtual Action analyze(const Token* tok, Direction d) const = 0;
    /// Update the state of the value
    virtual void update(Token* tok, Action a, Direction d) = 0;
    /// Try to evaluate the value of a token(most likely a condition)
    virtual std::vector<int> evaluate(const Token* tok) const = 0;
    /// Lower any values to possible
    virtual bool lowerToPossible() = 0;
    /// Lower any values to inconclusive
    virtual bool lowerToInconclusive() = 0;
    /// If the analysis is unsure whether to update a scope, this will return true if the analysis should bifurcate the scope
    virtual bool updateScope(const Token* endBlock, bool modified) const = 0;
    /// Called when a scope will be forked
    virtual void forkScope(const Token* /*endBlock*/) {}
    /// If the value is conditional
    virtual bool isConditional() const = 0;
    /// The condition that will be assumed during analysis
    virtual void assume(const Token* tok, bool state, const Token* at = nullptr) = 0;
    /// Return analyzer for expression at token
    virtual ValuePtr<Analyzer> reanalyze(Token* tok, const std::string& msg = "") const = 0;
    virtual ~Analyzer() {}
};

#endif

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

#ifndef GUARD_FORWARDANALYZER_H
#define GUARD_FORWARDANALYZER_H

#include "token.h"
#include "valueptr.h"
#include <vector>

struct ForwardAnalyzer
{
    struct Action {

        Action() : flag(0)
        {}

        Action(unsigned int f) : flag(f)
        {}

        enum {
            None         = 0,
            Read         = (1 << 0),
            Write        = (1 << 1),
            Invalid      = (1 << 2),
            Inconclusive = (1 << 3),
        };

        void set(unsigned int f, bool state = true) {
            flag = state ? flag | f : flag & ~f;
        }

        bool get(unsigned int f) const {
            return ((flag & f) != 0);
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
            return flag == None;
        }

        bool isModified() const {
            return isWrite() || isInvalid();
        }

        Action& operator|=(Action a) {
            set(a.flag);
            return *this;
        }

        friend Action operator|(Action a, Action b) {
            a |= b;
            return a;
        }

    private:

        unsigned int flag;
    };
    virtual Action Analyze(const Token* tok) const = 0;
    virtual void Update(Token* tok, Action a) = 0;
    virtual std::vector<int> Evaluate(const Token* tok) const = 0;
    virtual void LowerToPossible() = 0;
    virtual void LowerToInconclusive() = 0;
    virtual bool SkipLambda(const Token* tok) const {
        return true;
    }
    virtual ~ForwardAnalyzer() {}
};

void valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<ForwardAnalyzer>& fa);

#endif

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
    enum class Action {
        None,
        Read,
        Write,
        Invalid,
        Inconclusive
    };
    virtual Action Analyze(const Token* tok) const = 0;
    virtual void Update(Token* tok, Action a) = 0;
    virtual std::vector<int> Evaluate(const Token* tok) const = 0;
    virtual void LowerToPossible() = 0;
    virtual void LowerToInconclusive() = 0;
    virtual ~ForwardAnalyzer() {}
};

void valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<ForwardAnalyzer>& fa);

#endif

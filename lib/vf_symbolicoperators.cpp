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

#include "vf_symbolicoperators.h"

#include "astutils.h"
#include "config.h"
#include "infer.h"
#include "symboldatabase.h"
#include "token.h"
#include "vfvalue.h"

#include "vf_common.h"
#include "vf_settokenvalue.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ValueFlow
{
    static const Token* isStrlenOf(const Token* tok, const Token* expr, int depth = 10)
    {
        if (depth < 0)
            return nullptr;
        if (!tok)
            return nullptr;
        if (!expr)
            return nullptr;
        if (expr->exprId() == 0)
            return nullptr;
        if (Token::simpleMatch(tok->previous(), "strlen (")) {
            if (tok->astOperand2()->exprId() == expr->exprId())
                return tok;
        } else {
            for (const Value& v : tok->values()) {
                if (!v.isSymbolicValue())
                    continue;
                if (!v.isKnown())
                    continue;
                if (v.intvalue != 0)
                    continue;
                if (const Token* next = isStrlenOf(v.tokvalue, expr, depth - 1))
                    return next;
            }
        }
        return nullptr;
    }

    void analyzeSymbolicOperators(const SymbolDatabase& symboldatabase, const Settings& settings)
    {
        for (const Scope* scope : symboldatabase.functionScopes) {
            for (auto* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
                if (tok->hasKnownIntValue())
                    continue;

                if (Token::Match(tok, "abs|labs|llabs|fabs|fabsf|fabsl (")) {
                    const Token* arg = tok->next()->astOperand2();
                    if (!arg)
                        continue;
                    if (arg->exprId() == 0)
                        continue;
                    Value c = inferCondition(">=", arg, 0);
                    if (!c.isKnown())
                        continue;

                    Value v = makeSymbolic(arg);
                    v.errorPath = c.errorPath;
                    v.errorPath.emplace_back(tok, "Passed to " + tok->str());
                    if (c.intvalue == 0)
                        v.setImpossible();
                    else
                        v.setKnown();
                    setTokenValue(tok->next(), std::move(v), settings);
                } else if (Token::Match(tok, "*|/|<<|>>|^|+|-|%or%")) {
                    if (!tok->astOperand1())
                        continue;
                    if (!tok->astOperand2())
                        continue;
                    if (!astIsIntegral(tok->astOperand1(), false) && !astIsIntegral(tok->astOperand2(), false))
                        continue;
                    const Value* constant = nullptr;
                    const Token* vartok = nullptr;
                    if (tok->astOperand1()->hasKnownIntValue()) {
                        constant = &tok->astOperand1()->values().front();
                        vartok = tok->astOperand2();
                    }
                    if (tok->astOperand2()->hasKnownIntValue()) {
                        constant = &tok->astOperand2()->values().front();
                        vartok = tok->astOperand1();
                    }
                    if (!constant)
                        continue;
                    if (!vartok)
                        continue;
                    if (vartok->exprId() == 0)
                        continue;
                    if (Token::Match(tok, "<<|>>|/") && !astIsLHS(vartok))
                        continue;
                    if (Token::Match(tok, "<<|>>|^|+|-|%or%") && constant->intvalue != 0)
                        continue;
                    if (Token::Match(tok, "*|/") && constant->intvalue != 1)
                        continue;
                    std::vector<Value> values = {makeSymbolic(vartok)};
                    std::unordered_set<nonneg int> ids = {vartok->exprId()};
                    std::copy_if(vartok->values().cbegin(),
                                 vartok->values().cend(),
                                 std::back_inserter(values),
                                 [&](const Value& v) {
                        if (!v.isSymbolicValue())
                            return false;
                        if (!v.tokvalue)
                            return false;
                        return ids.insert(v.tokvalue->exprId()).second;
                    });
                    for (Value& v : values)
                        setTokenValue(tok, std::move(v), settings);
                } else if (Token::simpleMatch(tok, "[")) {
                    const Token* arrayTok = tok->astOperand1();
                    const Token* indexTok = tok->astOperand2();
                    if (!arrayTok)
                        continue;
                    if (!indexTok)
                        continue;
                    for (const Value& value : indexTok->values()) {
                        if (!value.isSymbolicValue())
                            continue;
                        if (value.intvalue != 0)
                            continue;
                        const Token* strlenTok = isStrlenOf(value.tokvalue, arrayTok);
                        if (!strlenTok)
                            continue;
                        Value v = value;
                        v.bound = Value::Bound::Point;
                        v.valueType = Value::ValueType::INT;
                        v.errorPath.emplace_back(strlenTok, "Return index of first '\\0' character in string");
                        setTokenValue(tok, std::move(v), settings);
                    }
                }
            }
        }
    }
}

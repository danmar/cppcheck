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

#include "vf_functionreturn.h"

#include "astutils.h"
#include "programmemory.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_bailout.h"
#include "vf_settokenvalue.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace ValueFlow
{
    static const Value* getKnownValueFromToken(const Token* tok)
    {
        if (!tok)
            return nullptr;
        auto it = std::find_if(tok->values().begin(), tok->values().end(), [&](const Value& v) {
            return (v.isIntValue() || v.isContainerSizeValue() || v.isFloatValue()) && v.isKnown();
        });
        if (it == tok->values().end())
            return nullptr;
        return std::addressof(*it);
    }

    static const Value* getKnownValueFromTokens(const std::vector<const Token*>& toks)
    {
        if (toks.empty())
            return nullptr;
        const Value* result = getKnownValueFromToken(toks.front());
        if (!result)
            return nullptr;
        if (!std::all_of(std::next(toks.begin()), toks.end(), [&](const Token* tok) {
            return std::any_of(tok->values().begin(), tok->values().end(), [&](const Value& v) {
                return v.equalValue(*result) && v.valueKind == result->valueKind;
            });
        }))
            return nullptr;
        return result;
    }

    static void setFunctionReturnValue(const Function* f, Token* tok, Value v, const Settings& settings)
    {
        if (f->hasVirtualSpecifier()) {
            if (v.isImpossible())
                return;
            v.setPossible();
        } else if (!v.isImpossible()) {
            v.setKnown();
        }
        v.errorPath.emplace_back(tok, "Calling function '" + f->name() + "' returns " + v.toString());
        setTokenValue(tok, std::move(v), settings);
    }

    void analyzeFunctionReturn(TokenList &tokenlist, ErrorLogger &errorLogger, const Settings& settings)
    {
        for (Token *tok = tokenlist.back(); tok; tok = tok->previous()) {
            if (tok->str() != "(" || !tok->astOperand1() || tok->isCast())
                continue;

            const Function* function = nullptr;
            if (Token::Match(tok->previous(), "%name% ("))
                function = tok->previous()->function();
            else
                function = tok->astOperand1()->function();
            if (!function)
                continue;
            // TODO: Check if member variable is a pointer or reference
            if (function->isImplicitlyVirtual() && !function->hasFinalSpecifier())
                continue;

            if (tok->hasKnownValue())
                continue;

            std::vector<const Token*> returns = Function::findReturns(function);
            if (returns.empty())
                continue;

            if (const Value* v = getKnownValueFromTokens(returns)) {
                setFunctionReturnValue(function, tok, *v, settings);
                continue;
            }

            // Arguments..
            std::vector<const Token*> arguments = getArguments(tok);

            ProgramMemory programMemory;
            for (std::size_t i = 0; i < arguments.size(); ++i) {
                const Variable * const arg = function->getArgumentVar(i);
                if (!arg) {
                    if (settings.debugwarnings)
                        bailout(tokenlist, errorLogger, tok, "function return; unhandled argument type");
                    programMemory.clear();
                    break;
                }
                const Value* v = getKnownValueFromToken(arguments[i]);
                if (!v)
                    continue;
                programMemory.setValue(arg->nameToken(), *v);
            }
            if (programMemory.empty() && !arguments.empty())
                continue;
            std::vector<Value> values = execute(function->functionScope, programMemory, settings);
            for (const Value& v : values) {
                if (v.isUninitValue())
                    continue;
                setFunctionReturnValue(function, tok, v, settings);
            }
        }
    }
}

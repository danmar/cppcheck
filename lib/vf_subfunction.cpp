/* -*- C++ -*-
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

#include "vf_subfunction.h"

#include "astutils.h"
#include "config.h"
#include "forwardanalyzer.h"
#include "library.h"
#include "mathlib.h"
#include "programmemory.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "utils.h"
#include "valueptr.h"
#include "vfvalue.h"

#include "vf_analyzers.h"
#include "vf_bailout.h"
#include "vf_common.h"
#include "vf_settokenvalue.h"

#include "checkuninitvar.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ValueFlow
{
    static std::list<Value> getFunctionArgumentValues(const Token *argtok)
    {
        std::list<Value> argvalues(argtok->values());
        removeImpossible(argvalues);
        if (argvalues.empty() && Token::Match(argtok, "%comp%|%oror%|&&|!")) {
            argvalues.emplace_back(0);
            argvalues.emplace_back(1);
        }
        return argvalues;
    }

    template<class Key, class F>
    static bool productParams(const Settings& settings, const std::unordered_map<Key, std::list<Value>>& vars, F f)
    {
        using Args = std::vector<std::unordered_map<Key, Value>>;
        Args args(1);
        // Compute cartesian product of all arguments
        for (const auto& p:vars) {
            if (p.second.empty())
                continue;
            args.back()[p.first] = p.second.front();
        }
        bool bail = false;
        int max = settings.vfOptions.maxSubFunctionArgs;
        for (const auto& p:vars) {
            if (args.size() > max) {
                bail = true;
                break;
            }
            if (p.second.empty())
                continue;
            std::for_each(std::next(p.second.begin()), p.second.end(), [&](const Value& value) {
                Args new_args;
                for (auto arg:args) {
                    if (value.path != 0) {
                        for (const auto& q:arg) {
                            if (q.first == p.first)
                                continue;
                            if (q.second.path == 0)
                                continue;
                            if (q.second.path != value.path)
                                return;
                        }
                    }
                    arg[p.first] = value;
                    new_args.push_back(std::move(arg));
                }
                std::copy(new_args.cbegin(), new_args.cend(), std::back_inserter(args));
            });
        }

        if (args.size() > max) {
            bail = true;
            args.resize(max);
            // TODO: add bailout message
        }

        for (const auto& arg:args) {
            if (arg.empty())
                continue;
            // Make sure all arguments are the same path
            const MathLib::bigint path = arg.cbegin()->second.path;
            if (std::any_of(arg.cbegin(), arg.cend(), [&](const std::pair<Key, Value>& p) {
                return p.second.path != path;
            }))
                continue;
            f(arg);
        }
        return !bail;
    }

    static void valueFlowLibraryFunction(Token *tok, const std::string &returnValue, const Settings &settings)
    {
        std::unordered_map<nonneg int, std::list<Value>> argValues;
        int argn = 1;
        for (const Token *argtok : getArguments(tok->previous())) {
            argValues[argn] = getFunctionArgumentValues(argtok);
            argn++;
        }
        if (returnValue.find("arg") != std::string::npos && argValues.empty())
            return;
        productParams(settings, argValues, [&](const std::unordered_map<nonneg int, Value>& arg) {
            Value value = evaluateLibraryFunction(arg, returnValue, settings, tok->isCpp());
            if (value.isUninitValue())
                return;
            Value::ValueKind kind = Value::ValueKind::Known;
            for (auto&& p : arg) {
                if (p.second.isPossible())
                    kind = p.second.valueKind;
                if (p.second.isInconclusive()) {
                    kind = p.second.valueKind;
                    break;
                }
            }
            if (value.isImpossible() && kind != Value::ValueKind::Known)
                return;
            if (!value.isImpossible())
                value.valueKind = kind;
            setTokenValue(tok, std::move(value), settings);
        });
    }

    static void valueFlowInjectParameter(const TokenList& tokenlist,
                                         ErrorLogger& errorLogger,
                                         const Settings& settings,
                                         const Scope* functionScope,
                                         const std::unordered_map<const Variable*, std::list<Value>>& vars)
    {
        const bool r = productParams(settings, vars, [&](const std::unordered_map<const Variable*, Value>& arg) {
            auto a = makeMultiValueFlowAnalyzer(arg, settings);
            valueFlowGenericForward(const_cast<Token*>(functionScope->bodyStart), functionScope->bodyEnd, a, tokenlist, errorLogger, settings);
        });
        if (!r) {
            std::string fname = "<unknown>";
            if (const Function* f = functionScope->function)
                fname = f->name();
            if (settings.debugwarnings)
                bailout(tokenlist, errorLogger, functionScope->bodyStart, "Too many argument passed to " + fname);
        }
    }

    void analyzeSubFunction(const TokenList& tokenlist, SymbolDatabase& symboldatabase, ErrorLogger& errorLogger, const Settings& settings)
    {
        int id = 0;
        for (auto it = symboldatabase.functionScopes.crbegin(); it != symboldatabase.functionScopes.crend(); ++it) {
            const Scope* scope = *it;
            const Function* function = scope->function;
            if (!function)
                continue;
            for (auto* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
                if (tok->isKeyword() || !Token::Match(tok, "%name% ("))
                    continue;

                const Function * const calledFunction = tok->function();
                if (!calledFunction) {
                    // library function?
                    const std::string& returnValue(settings.library.returnValue(tok));
                    if (!returnValue.empty())
                        valueFlowLibraryFunction(tok->next(), returnValue, settings);
                    continue;
                }

                const Scope * const calledFunctionScope = calledFunction->functionScope;
                if (!calledFunctionScope)
                    continue;

                id++;
                std::unordered_map<const Variable*, std::list<Value>> argvars;
                // TODO: Rewrite this. It does not work well to inject 1 argument at a time.
                const std::vector<const Token *> &callArguments = getArguments(tok);
                for (int argnr = 0U; argnr < callArguments.size(); ++argnr) {
                    const Token *argtok = callArguments[argnr];
                    // Get function argument
                    const Variable * const argvar = calledFunction->getArgumentVar(argnr);
                    if (!argvar)
                        break;

                    // passing value(s) to function
                    std::list<Value> argvalues(getFunctionArgumentValues(argtok));

                    // Remove non-local lifetimes
                    argvalues.remove_if([](const Value& v) {
                        if (v.isLifetimeValue())
                            return !v.isLocalLifetimeValue() && !v.isSubFunctionLifetimeValue();
                        return false;
                    });
                    // Remove uninit values if argument is passed by value
                    if (argtok->variable() && !argtok->variable()->isPointer() && argvalues.size() == 1 && argvalues.front().isUninitValue()) {
                        if (CheckUninitVar::isVariableUsage(argtok, settings.library, false, CheckUninitVar::Alloc::NO_ALLOC, 0))
                            continue;
                    }

                    if (argvalues.empty())
                        continue;

                    // Error path..
                    for (Value &v : argvalues) {
                        const std::string nr = std::to_string(argnr + 1) + getOrdinalText(argnr + 1);

                        v.errorPath.emplace_back(argtok,
                                                 "Calling function '" +
                                                 calledFunction->name() +
                                                 "', " +
                                                 nr +
                                                 " argument '" +
                                                 argtok->expressionString() +
                                                 "' value is " +
                                                 v.infoString());
                        v.path = 256 * v.path + id % 256;
                        // Change scope of lifetime values
                        if (v.isLifetimeValue())
                            v.lifetimeScope = Value::LifetimeScope::SubFunction;
                    }

                    // passed values are not "known"..
                    lowerToPossible(argvalues);

                    argvars[argvar] = std::move(argvalues);
                }
                valueFlowInjectParameter(tokenlist, errorLogger, settings, calledFunctionScope, argvars);
            }
        }
    }
}

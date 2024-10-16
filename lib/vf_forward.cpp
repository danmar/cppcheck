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

#include "vf_forward.h"

#include "forwardanalyzer.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "vfvalue.h"

#include "vf_common.h"
#include "vf_analyzers.h"

#include <utility>

namespace ValueFlow
{
    Analyzer::Result valueFlowForward(Token* startToken,
                                      const Token* endToken,
                                      const Token* exprTok,
                                      ValueFlow::Value value,
                                      const TokenList& tokenlist,
                                      ErrorLogger& errorLogger,
                                      const Settings& settings,
                                      SourceLocation loc)
    {
        if (settings.debugnormal)
            setSourceLocation(value, loc, startToken);
        return valueFlowGenericForward(startToken,
                                       endToken,
                                       makeAnalyzer(exprTok, std::move(value), settings),
                                       tokenlist,
                                       errorLogger,
                                       settings);
    }

    Analyzer::Result valueFlowForward(Token* startToken,
                                      const Token* endToken,
                                      const Token* exprTok,
                                      std::list<ValueFlow::Value> values,
                                      const TokenList& tokenlist,
                                      ErrorLogger& errorLogger,
                                      const Settings& settings,
                                      SourceLocation loc)
    {
        Analyzer::Result result{};
        for (ValueFlow::Value& v : values) {
            result.update(valueFlowForward(startToken, endToken, exprTok, std::move(v), tokenlist, errorLogger, settings, loc));
        }
        return result;
    }

    Analyzer::Result valueFlowForward(Token* startToken,
                                      const Token* exprTok,
                                      ValueFlow::Value v,
                                      const TokenList& tokenlist,
                                      ErrorLogger& errorLogger,
                                      const Settings& settings,
                                      SourceLocation loc)
    {
        const Token* endToken = nullptr;
        const Function* f = Scope::nestedInFunction(startToken->scope());
        if (f && f->functionScope)
            endToken = f->functionScope->bodyEnd;
        if (!endToken && exprTok && exprTok->variable() && !exprTok->variable()->isLocal())
            endToken = startToken->scope()->bodyEnd;
        return valueFlowForward(startToken, endToken, exprTok, std::move(v), tokenlist, errorLogger, settings, loc);
    }

    Analyzer::Result valueFlowForwardRecursive(Token* top,
                                               const Token* exprTok,
                                               std::list<ValueFlow::Value> values,
                                               const TokenList& tokenlist,
                                               ErrorLogger& errorLogger,
                                               const Settings& settings,
                                               SourceLocation loc)
    {
        Analyzer::Result result{};
        for (ValueFlow::Value& v : values) {
            if (settings.debugnormal)
                setSourceLocation(v, loc, top);
            result.update(
                valueFlowGenericForward(top, makeAnalyzer(exprTok, std::move(v), settings), tokenlist, errorLogger, settings));
        }
        return result;
    }
}

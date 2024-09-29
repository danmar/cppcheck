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

#include "vf_conditionexpressions.h"

#include "astutils.h"
#include "forwardanalyzer.h"
#include "symboldatabase.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "library.h"
#include "settings.h"
#include "sourcelocation.h"
#include "token.h"
#include "tokenlist.h"
#include "valueptr.h"
#include "vfvalue.h"

#include "vf_analyzers.h"
#include "vf_bailout.h"
#include "vf_common.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace ValueFlow
{
    static const Token* findIncompleteVar(const Token* start, const Token* end)
    {
        for (const Token* tok = start; tok != end; tok = tok->next()) {
            if (tok->isIncompleteVar())
                return tok;
        }
        return nullptr;
    }

    static std::vector<const Token*> getConditions(const Token* tok, const char* op)
    {
        std::vector<const Token*> conds = {tok};
        if (tok->str() == op) {
            std::vector<const Token*> args = astFlatten(tok, op);
            std::copy_if(args.cbegin(), args.cend(), std::back_inserter(conds), [&](const Token* tok2) {
                if (tok2->exprId() == 0)
                    return false;
                if (tok2->hasKnownIntValue())
                    return false;
                if (Token::Match(tok2, "%var%|.") && !astIsBool(tok2))
                    return false;
                return true;
            });
        }
        return conds;
    }

    static Value makeConditionValue(long long val,
                                    const Token* condTok,
                                    bool assume,
                                    bool impossible,
                                    const Settings& settings,
                                    SourceLocation loc = SourceLocation::current())
    {
        Value v(val);
        v.setKnown();
        if (impossible) {
            v.intvalue = !v.intvalue;
            v.setImpossible();
        }
        v.condition = condTok;
        if (assume)
            v.errorPath.emplace_back(condTok, "Assuming condition '" + condTok->expressionString() + "' is true");
        else
            v.errorPath.emplace_back(condTok, "Assuming condition '" + condTok->expressionString() + "' is false");
        if (settings.debugnormal)
            setSourceLocation(v, loc, condTok);
        return v;
    }

    static bool isEscapeScope(const Token* tok, const Settings& settings, bool unknown = false)
    {
        if (!Token::simpleMatch(tok, "{"))
            return false;
        // TODO this search for termTok in all subscopes. It should check the end of the scope.
        const Token * termTok = Token::findmatch(tok, "return|continue|break|throw|goto", tok->link());
        if (termTok && termTok->scope() == tok->scope())
            return true;
        std::string unknownFunction;
        if (settings.library.isScopeNoReturn(tok->link(), &unknownFunction))
            return unknownFunction.empty() || unknown;
        return false;
    }

    void analyzeConditionExpressions(const TokenList &tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger &errorLogger, const Settings &settings)
    {
        if (!settings.daca && !settings.vfOptions.doConditionExpressionAnalysis)
        {
            if (settings.debugwarnings) {
                ErrorMessage::FileLocation loc(tokenlist.getSourceFilePath(), 0, 0);
                const ErrorMessage errmsg({std::move(loc)}, tokenlist.getSourceFilePath(), Severity::debug, "Analysis of condition expressions is disabled. Use --check-level=exhaustive to enable it.", "normalCheckLevelConditionExpressions", Certainty::normal);
                errorLogger.reportErr(errmsg);
            }
            return;
        }

        for (const Scope * scope : symboldatabase.functionScopes) {
            if (const Token* incompleteTok = findIncompleteVar(scope->bodyStart, scope->bodyEnd)) {
                if (settings.debugwarnings)
                    bailoutIncompleteVar(tokenlist, errorLogger, incompleteTok, "Skipping function due to incomplete variable " + incompleteTok->str());
                continue;
            }

            if (settings.daca && !settings.vfOptions.doConditionExpressionAnalysis)
                continue;

            for (auto* tok = const_cast<Token*>(scope->bodyStart); tok != scope->bodyEnd; tok = tok->next()) {
                if (!Token::simpleMatch(tok, "if ("))
                    continue;
                Token* parenTok = tok->next();
                if (!Token::simpleMatch(parenTok->link(), ") {"))
                    continue;
                Token * blockTok = parenTok->link()->tokAt(1);
                const Token* condTok = parenTok->astOperand2();
                if (condTok->exprId() == 0)
                    continue;
                if (condTok->hasKnownIntValue())
                    continue;
                if (!isConstExpression(condTok, settings.library))
                    continue;
                const bool isOp = condTok->isComparisonOp() || condTok->tokType() == Token::eLogicalOp;
                const bool is1 = isOp || astIsBool(condTok);

                Token* startTok = blockTok;
                // Inner condition
                {
                    for (const Token* condTok2 : getConditions(condTok, "&&")) {
                        if (is1) {
                            const bool isBool = astIsBool(condTok2) || Token::Match(condTok2, "%comp%|%oror%|&&");
                            auto a1 = makeSameExpressionAnalyzer(condTok2, makeConditionValue(1, condTok2, /*assume*/ true, !isBool, settings), settings); // don't set '1' for non-boolean expressions
                            valueFlowGenericForward(startTok, startTok->link(), a1, tokenlist, errorLogger, settings);
                        }

                        auto a2 = makeOppositeExpressionAnalyzer(true, condTok2, makeConditionValue(0, condTok2, true, false, settings), settings);
                        valueFlowGenericForward(startTok, startTok->link(), a2, tokenlist, errorLogger, settings);
                    }
                }

                std::vector<const Token*> conds = getConditions(condTok, "||");

                // Check else block
                if (Token::simpleMatch(startTok->link(), "} else {")) {
                    startTok = startTok->link()->tokAt(2);
                    for (const Token* condTok2:conds) {
                        auto a1 = makeSameExpressionAnalyzer(condTok2, makeConditionValue(0, condTok2, false, false, settings), settings);
                        valueFlowGenericForward(startTok, startTok->link(), a1, tokenlist, errorLogger, settings);

                        if (is1) {
                            auto a2 = makeOppositeExpressionAnalyzer(true, condTok2, makeConditionValue(isOp, condTok2, false, false, settings), settings);
                            valueFlowGenericForward(startTok, startTok->link(), a2, tokenlist, errorLogger, settings);
                        }
                    }
                }

                // Check if the block terminates early
                if (isEscapeScope(blockTok, settings)) {
                    const Scope* scope2 = scope;
                    // If escaping a loop then only use the loop scope
                    if (isBreakOrContinueScope(blockTok->link())) {
                        scope2 = getLoopScope(blockTok->link());
                        if (!scope2)
                            continue;
                    }
                    for (const Token* condTok2:conds) {
                        auto a1 = makeSameExpressionAnalyzer(condTok2, makeConditionValue(0, condTok2, false, false, settings), settings);
                        valueFlowGenericForward(startTok->link()->next(), scope2->bodyEnd, a1, tokenlist, errorLogger, settings);

                        if (is1) {
                            auto a2 = makeOppositeExpressionAnalyzer(true, condTok2, makeConditionValue(1, condTok2, false, false, settings), settings);
                            valueFlowGenericForward(startTok->link()->next(), scope2->bodyEnd, a2, tokenlist, errorLogger, settings);
                        }
                    }
                }
            }
        }
    }
}

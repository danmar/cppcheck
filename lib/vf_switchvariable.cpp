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

#include "vf_switchvariable.h"

#include "mathlib.h"
#include "settings.h"
#include "sourcelocation.h"
#include "symboldatabase.h"
#include "token.h"
#include "vfvalue.h"

#include "vf_bailout.h"
#include "vf_reverse.h"

#include <list>
#include <string>
#include <utility>

namespace ValueFlow
{
    // Deprecated
    static void valueFlowReverse(const TokenList& tokenlist,
                                 Token* tok,
                                 const Token* const varToken,
                                 Value val,
                                 ErrorLogger& errorLogger,
                                 const Settings& settings,
                                 SourceLocation loc = SourceLocation::current())
    {
        valueFlowReverse(tok, nullptr, varToken, {std::move(val)}, tokenlist, errorLogger, settings, loc);
    }

    void analyzeSwitchVariable(const TokenList &tokenlist, const SymbolDatabase& symboldatabase, ErrorLogger &errorLogger, const Settings &settings)
    {
        for (const Scope &scope : symboldatabase.scopeList) {
            if (scope.type != Scope::ScopeType::eSwitch)
                continue;
            if (!Token::Match(scope.classDef, "switch ( %var% ) {"))
                continue;
            const Token *vartok = scope.classDef->tokAt(2);
            const Variable *var = vartok->variable();
            if (!var)
                continue;

            // bailout: global non-const variables
            if (!(var->isLocal() || var->isArgument()) && !var->isConst()) {
                if (settings.debugwarnings)
                    bailout(tokenlist, errorLogger, vartok, "switch variable " + var->name() + " is global");
                continue;
            }

            for (const Token *tok = scope.bodyStart->next(); tok != scope.bodyEnd; tok = tok->next()) {
                if (tok->str() == "{") {
                    tok = tok->link();
                    continue;
                }
                if (Token::Match(tok, "case %num% :")) {
                    std::list<Value> values;
                    values.emplace_back(MathLib::toBigNumber(tok->strAt(1)));
                    values.back().condition = tok;
                    values.back().errorPath.emplace_back(tok, "case " + tok->strAt(1) + ": " + vartok->str() + " is " + tok->strAt(1) + " here.");
                    bool known = false;
                    if ((Token::simpleMatch(tok->previous(), "{") || Token::simpleMatch(tok->tokAt(-2), "break ;")) && !Token::Match(tok->tokAt(3), ";| case"))
                        known = true;
                    while (Token::Match(tok->tokAt(3), ";| case %num% :")) {
                        known = false;
                        tok = tok->tokAt(3);
                        if (!tok->isName())
                            tok = tok->next();
                        values.emplace_back(MathLib::toBigNumber(tok->strAt(1)));
                        values.back().condition = tok;
                        values.back().errorPath.emplace_back(tok, "case " + tok->strAt(1) + ": " + vartok->str() + " is " + tok->strAt(1) + " here.");
                    }
                    for (auto val = values.cbegin(); val != values.cend(); ++val) {
                        valueFlowReverse(tokenlist,
                                         const_cast<Token*>(scope.classDef),
                                         vartok,
                                         *val,
                                         errorLogger,
                                         settings);
                    }
                    if (vartok->variable()->scope()) {
                        if (known)
                            values.back().setKnown();

                        // FIXME We must check if there is a return. See #9276
                        /*
                           valueFlowForwardVariable(tok->tokAt(3),
                                                 vartok->variable()->scope()->bodyEnd,
                                                 vartok->variable(),
                                                 vartok->varId(),
                                                 values,
                                                 values.back().isKnown(),
                                                 false,
                                                 tokenlist,
                                                 errorLogger,
                                                 settings);
                         */
                    }
                }
            }
        }
    }
}

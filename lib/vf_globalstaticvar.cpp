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

#include "vf_globalstaticvar.h"

#include "astutils.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <list>
#include <map>
#include <utility>

namespace ValueFlow
{
    void analyzeGlobalStaticVar(TokenList &tokenList, const Settings &settings)
    {
        // Get variable values...
        std::map<const Variable *, Value> vars;
        for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
            if (!tok->variable())
                continue;
            // Initialization...
            if (tok == tok->variable()->nameToken() &&
                tok->variable()->isStatic() &&
                !tok->variable()->isConst() &&
                tok->valueType() &&
                tok->valueType()->isIntegral() &&
                tok->valueType()->pointer == 0 &&
                tok->valueType()->constness == 0 &&
                Token::Match(tok, "%name% =") &&
                tok->next()->astOperand2() &&
                tok->next()->astOperand2()->hasKnownIntValue()) {
                vars[tok->variable()] = tok->next()->astOperand2()->values().front();
            } else {
                // If variable is written anywhere in TU then remove it from vars
                if (!tok->astParent())
                    continue;
                if (Token::Match(tok->astParent(), "++|--|&") && !tok->astParent()->astOperand2())
                    vars.erase(tok->variable());
                else if (tok->astParent()->isAssignmentOp()) {
                    if (tok == tok->astParent()->astOperand1())
                        vars.erase(tok->variable());
                    else if (tok->isCpp() && Token::Match(tok->astParent()->tokAt(-2), "& %name% ="))
                        vars.erase(tok->variable());
                } else if (isLikelyStreamRead(tok->astParent())) {
                    vars.erase(tok->variable());
                } else if (Token::Match(tok->astParent(), "[(,]"))
                    vars.erase(tok->variable());
            }
        }

        // Set values..
        for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
            if (!tok->variable())
                continue;
            const std::map<const Variable *, Value>::const_iterator var = vars.find(tok->variable());
            if (var == vars.end())
                continue;
            setTokenValue(tok, var->second, settings);
        }
    }
}

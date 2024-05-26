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

#include "vf_sameexpressions.h"

#include "astutils.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <utility>

namespace ValueFlow
{
    void analyzeSameExpressions(TokenList &tokenlist, const Settings& settings)
    {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->hasKnownIntValue())
                continue;

            if (!tok->astOperand1() || !tok->astOperand2())
                continue;

            if (tok->astOperand1()->isLiteral() || tok->astOperand2()->isLiteral())
                continue;

            if (!astIsIntegral(tok->astOperand1(), false) && !astIsIntegral(tok->astOperand2(), false))
                continue;

            Value val;

            if (Token::Match(tok, "==|>=|<=|/")) {
                val = ValueFlow::Value(1);
                val.setKnown();
            }

            if (Token::Match(tok, "!=|>|<|%|-")) {
                val = ValueFlow::Value(0);
                val.setKnown();
            }

            if (!val.isKnown())
                continue;

            if (isSameExpression(false, tok->astOperand1(), tok->astOperand2(), settings, true, true, &val.errorPath)) {
                setTokenValue(tok, std::move(val), settings);
            }
        }
    }
}

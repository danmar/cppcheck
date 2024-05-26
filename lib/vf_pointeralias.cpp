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

#include "vf_pointeralias.h"

#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <string>
#include <utility>

namespace ValueFlow
{
    void analyzePointerAlias(TokenList &tokenlist, const Settings& settings)
    {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            // not address of
            if (!tok->isUnaryOp("&"))
                continue;

            // parent should be a '='
            if (!Token::simpleMatch(tok->astParent(), "="))
                continue;

            // child should be some buffer or variable
            const Token *vartok = tok->astOperand1();
            while (vartok) {
                if (vartok->str() == "[")
                    vartok = vartok->astOperand1();
                else if (vartok->str() == "." || vartok->str() == "::")
                    vartok = vartok->astOperand2();
                else
                    break;
            }
            if (!(vartok && vartok->variable() && !vartok->variable()->isPointer()))
                continue;

            Value value;
            value.valueType = Value::ValueType::TOK;
            value.tokvalue = tok;
            setTokenValue(tok, std::move(value), settings);
        }
    }
}

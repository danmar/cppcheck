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

#include "vf_array.h"

#include "astutils.h"
#include "config.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <map>
#include <utility>

namespace ValueFlow
{
    void analyzeArray(TokenList &tokenlist, const Settings &settings)
    {
        std::map<nonneg int, const Token *> constantArrays;

        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->varId() > 0) {
                // array
                const std::map<nonneg int, const Token *>::const_iterator it = constantArrays.find(tok->varId());
                if (it != constantArrays.end()) {
                    Value value;
                    value.valueType = Value::ValueType::TOK;
                    value.tokvalue = it->second;
                    value.setKnown();
                    setTokenValue(tok, std::move(value), settings);
                }

                // const array decl
                else if (tok->variable() && tok->variable()->isArray() && tok->variable()->isConst() &&
                         tok->variable()->nameToken() == tok && Token::Match(tok, "%var% [ %num%| ] = {")) {
                    Token* rhstok = tok->linkAt(1)->tokAt(2);
                    constantArrays[tok->varId()] = rhstok;
                    tok = rhstok->link();
                }

                // pointer = array
                else if (tok->variable() && tok->variable()->isArray() && Token::simpleMatch(tok->astParent(), "=") &&
                         astIsRHS(tok) && tok->astParent()->astOperand1() &&
                         tok->astParent()->astOperand1()->variable() &&
                         tok->astParent()->astOperand1()->variable()->isPointer()) {
                    Value value;
                    value.valueType = Value::ValueType::TOK;
                    value.tokvalue = tok;
                    value.setKnown();
                    setTokenValue(tok, std::move(value), settings);
                }
                continue;
            }

            if (Token::Match(tok, "const %type% %var% [ %num%| ] = {")) {
                Token *vartok = tok->tokAt(2);
                Token *rhstok = vartok->linkAt(1)->tokAt(2);
                constantArrays[vartok->varId()] = rhstok;
                tok = rhstok->link();
                continue;
            }

            if (Token::Match(tok, "const char %var% [ %num%| ] = %str% ;")) {
                Token *vartok = tok->tokAt(2);
                Token *strtok = vartok->linkAt(1)->tokAt(2);
                constantArrays[vartok->varId()] = strtok;
                tok = strtok->next();
                continue;
            }
        }
    }
}

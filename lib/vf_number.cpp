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

#include "vf_number.h"

#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_common.h"
#include "vf_settokenvalue.h"

#include <string>
#include <utility>

namespace ValueFlow
{
    void analyzeNumber(TokenList &tokenlist, const Settings& settings)
    {
        for (Token *tok = tokenlist.front(); tok;) {
            tok = valueFlowSetConstantValue(tok, settings);
        }

        if (tokenlist.isCPP()) {
            for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
                if (tok->isName() && !tok->varId() && Token::Match(tok, "false|true")) {
                    ValueFlow::Value value(tok->str() == "true");
                    if (!tok->isTemplateArg())
                        value.setKnown();
                    setTokenValue(tok, std::move(value), settings);
                } else if (Token::Match(tok, "[(,] NULL [,)]")) {
                    // NULL function parameters are not simplified in the
                    // normal tokenlist
                    ValueFlow::Value value(0);
                    if (!tok->isTemplateArg())
                        value.setKnown();
                    setTokenValue(tok->next(), std::move(value), settings);
                }
            }
        }
    }
}

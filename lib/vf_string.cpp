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

#include "vf_string.h"

#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <utility>

namespace ValueFlow
{
    void analyzeString(TokenList &tokenlist, const Settings& settings)
    {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->tokType() == Token::eString) {
                Value strvalue;
                strvalue.valueType = Value::ValueType::TOK;
                strvalue.tokvalue = tok;
                strvalue.setKnown();
                setTokenValue(tok, std::move(strvalue), settings);
            }
        }
    }
}

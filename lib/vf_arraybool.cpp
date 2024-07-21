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

#include "vf_arraybool.h"

#include "astutils.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <algorithm>
#include <functional>
#include <list>
#include <utility>

namespace ValueFlow
{
    static bool isNonZero(const Token *tok)
    {
        return tok && (!tok->hasKnownIntValue() || tok->values().front().intvalue != 0);
    }

    static const Token *getOtherOperand(const Token *tok)
    {
        if (!tok)
            return nullptr;
        if (!tok->astParent())
            return nullptr;
        if (tok->astParent()->astOperand1() != tok)
            return tok->astParent()->astOperand1();
        if (tok->astParent()->astOperand2() != tok)
            return tok->astParent()->astOperand2();
        return nullptr;
    }

    void analyzeArrayBool(TokenList &tokenlist, const Settings &settings)
    {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->hasKnownIntValue())
                continue;
            const Variable *var = nullptr;
            bool known = false;
            const std::list<Value>::const_iterator val =
                std::find_if(tok->values().cbegin(), tok->values().cend(), std::mem_fn(&Value::isTokValue));
            if (val == tok->values().end()) {
                var = tok->variable();
                known = true;
            } else {
                var = val->tokvalue->variable();
                known = val->isKnown();
            }
            if (!var)
                continue;
            if (!var->isArray() || var->isArgument() || var->isStlType())
                continue;
            if (isNonZero(getOtherOperand(tok)) && Token::Match(tok->astParent(), "%comp%"))
                continue;
            // TODO: Check for function argument
            if ((astIsBool(tok->astParent()) && !Token::Match(tok->astParent(), "(|%name%")) ||
                (tok->astParent() && Token::Match(tok->astParent()->previous(), "if|while|for ("))) {
                Value value{1};
                if (known)
                    value.setKnown();
                setTokenValue(tok, std::move(value), settings);
            }
        }
    }
}

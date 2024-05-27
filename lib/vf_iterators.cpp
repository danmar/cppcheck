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

#include "vf_iterators.h"

#include "astutils.h"
#include "library.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <utility>

namespace ValueFlow
{
    static Library::Container::Yield findIteratorYield(Token* tok, const Token** ftok, const Settings &settings)
    {
        auto yield = astContainerYield(tok, ftok);
        if (ftok && *ftok)
            return yield;

        if (!tok->astParent())
            return yield;

        //begin/end free functions
        return astFunctionYield(tok->astParent()->previous(), settings, ftok);
    }

    void analyzeIterators(TokenList &tokenlist, const Settings &settings)
    {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (!tok->scope())
                continue;
            if (!tok->scope()->isExecutable())
                continue;
            if (!astIsContainer(tok))
                continue;
            Token* ftok = nullptr;
            const Library::Container::Yield yield = findIteratorYield(tok, const_cast<const Token**>(&ftok), settings);
            if (ftok) {
                Value v(0);
                v.setKnown();
                if (yield == Library::Container::Yield::START_ITERATOR) {
                    v.valueType = Value::ValueType::ITERATOR_START;
                    setTokenValue(ftok->next(), std::move(v), settings);
                } else if (yield == Library::Container::Yield::END_ITERATOR) {
                    v.valueType = Value::ValueType::ITERATOR_END;
                    setTokenValue(ftok->next(), std::move(v), settings);
                }
            }
        }
    }
}

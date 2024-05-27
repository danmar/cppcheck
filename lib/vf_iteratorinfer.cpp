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

#include "vf_iteratorinfer.h"

#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_common.h"
#include "vf_settokenvalue.h"

#include <list>
#include <utility>

namespace ValueFlow
{
    void analyzeIteratorInfer(TokenList &tokenlist, const Settings &settings)
    {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (!tok->scope())
                continue;
            if (!tok->scope()->isExecutable())
                continue;
            std::list<Value> values = getIteratorValues(tok->values());
            values.remove_if([&](const Value& v) {
                if (!v.isImpossible())
                    return true;
                if (!v.condition)
                    return true;
                if (v.bound != Value::Bound::Point)
                    return true;
                if (v.isIteratorEndValue() && v.intvalue <= 0)
                    return true;
                if (v.isIteratorStartValue() && v.intvalue >= 0)
                    return true;
                return false;
            });
            for (Value& v:values) {
                v.setPossible();
                if (v.isIteratorStartValue())
                    v.intvalue++;
                if (v.isIteratorEndValue())
                    v.intvalue--;
                setTokenValue(tok, std::move(v), settings);
            }
        }
    }
}

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

#include "vf_unknownfunctionreturn.h"

#include "library.h"
#include "mathlib.h"
#include "settings.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_common.h"
#include "vf_settokenvalue.h"

#include <set>
#include <string>
#include <vector>

namespace ValueFlow
{
    void analyzeUnknownFunctionReturn(TokenList &tokenlist, const Settings &settings)
    {
        if (settings.checkUnknownFunctionReturn.empty())
            return;
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (!tok->astParent() || tok->str() != "(" || !tok->previous()->isName())
                continue;
            if (settings.checkUnknownFunctionReturn.find(tok->strAt(-1)) == settings.checkUnknownFunctionReturn.end())
                continue;
            std::vector<MathLib::bigint> unknownValues = settings.library.unknownReturnValues(tok->astOperand1());
            if (unknownValues.empty())
                continue;

            // Get min/max values for return type
            const std::string &typestr = settings.library.returnValueType(tok->previous());
            MathLib::bigint minvalue, maxvalue;
            if (!getMinMaxValues(typestr, settings, tok->isCpp(), minvalue, maxvalue))
                continue;

            for (MathLib::bigint value : unknownValues) {
                if (value < minvalue)
                    value = minvalue;
                else if (value > maxvalue)
                    value = maxvalue;
                setTokenValue(tok, Value(value), settings);
            }
        }
    }
}

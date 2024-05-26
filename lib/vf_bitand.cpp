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

#include "vf_bitand.h"

#include "mathlib.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <string>

namespace ValueFlow
{
    void analyzeBitAnd(TokenList &tokenlist, const Settings& settings)
    {
        for (Token *tok = tokenlist.front(); tok; tok = tok->next()) {
            if (tok->str() != "&")
                continue;

            if (tok->hasKnownValue())
                continue;

            if (!tok->astOperand1() || !tok->astOperand2())
                continue;

            MathLib::bigint number;
            if (MathLib::isInt(tok->astOperand1()->str()))
                number = MathLib::toBigNumber(tok->astOperand1()->str());
            else if (MathLib::isInt(tok->astOperand2()->str()))
                number = MathLib::toBigNumber(tok->astOperand2()->str());
            else
                continue;

            int bit = 0;
            while (bit <= (MathLib::bigint_bits - 2) && ((((MathLib::bigint)1) << bit) < number))
                ++bit;

            if ((((MathLib::bigint)1) << bit) == number) {
                setTokenValue(tok, Value(0), settings);
                setTokenValue(tok, Value(number), settings);
            }
        }
    }
}

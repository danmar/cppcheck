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

#include "vf_rightshift.h"

#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include "vf_settokenvalue.h"

#include <list>
#include <string>
#include <utility>

namespace ValueFlow
{
    static bool getExpressionRange(const Token *expr, MathLib::bigint *minvalue, MathLib::bigint *maxvalue)
    {
        if (expr->hasKnownIntValue()) {
            if (minvalue)
                *minvalue = expr->values().front().intvalue;
            if (maxvalue)
                *maxvalue = expr->values().front().intvalue;
            return true;
        }

        if (expr->str() == "&" && expr->astOperand1() && expr->astOperand2()) {
            MathLib::bigint vals[4];
            const bool lhsHasKnownRange = getExpressionRange(expr->astOperand1(), &vals[0], &vals[1]);
            const bool rhsHasKnownRange = getExpressionRange(expr->astOperand2(), &vals[2], &vals[3]);
            if (!lhsHasKnownRange && !rhsHasKnownRange)
                return false;
            if (!lhsHasKnownRange || !rhsHasKnownRange) {
                if (minvalue)
                    *minvalue = lhsHasKnownRange ? vals[0] : vals[2];
                if (maxvalue)
                    *maxvalue = lhsHasKnownRange ? vals[1] : vals[3];
            } else {
                if (minvalue)
                    *minvalue = vals[0] & vals[2];
                if (maxvalue)
                    *maxvalue = vals[1] & vals[3];
            }
            return true;
        }

        if (expr->str() == "%" && expr->astOperand1() && expr->astOperand2()) {
            MathLib::bigint vals[4];
            if (!getExpressionRange(expr->astOperand2(), &vals[2], &vals[3]))
                return false;
            if (vals[2] <= 0)
                return false;
            const bool lhsHasKnownRange = getExpressionRange(expr->astOperand1(), &vals[0], &vals[1]);
            if (lhsHasKnownRange && vals[0] < 0)
                return false;
            // If lhs has unknown value, it must be unsigned
            if (!lhsHasKnownRange && (!expr->astOperand1()->valueType() || expr->astOperand1()->valueType()->sign != ValueType::Sign::UNSIGNED))
                return false;
            if (minvalue)
                *minvalue = 0;
            if (maxvalue)
                *maxvalue = vals[3] - 1;
            return true;
        }

        return false;
    }

    void analyzeRightShift(TokenList &tokenList, const Settings& settings)
    {
        for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
            if (tok->str() != ">>")
                continue;

            if (tok->hasKnownValue())
                continue;

            if (!tok->astOperand1() || !tok->astOperand2())
                continue;

            if (!tok->astOperand2()->hasKnownValue())
                continue;

            const MathLib::bigint rhsvalue = tok->astOperand2()->values().front().intvalue;
            if (rhsvalue < 0)
                continue;

            if (!tok->astOperand1()->valueType() || !tok->astOperand1()->valueType()->isIntegral())
                continue;

            if (!tok->astOperand2()->valueType() || !tok->astOperand2()->valueType()->isIntegral())
                continue;

            MathLib::bigint lhsmax=0;
            if (!getExpressionRange(tok->astOperand1(), nullptr, &lhsmax))
                continue;
            if (lhsmax < 0)
                continue;
            int lhsbits;
            if ((tok->astOperand1()->valueType()->type == ValueType::Type::CHAR) ||
                (tok->astOperand1()->valueType()->type == ValueType::Type::SHORT) ||
                (tok->astOperand1()->valueType()->type == ValueType::Type::WCHAR_T) ||
                (tok->astOperand1()->valueType()->type == ValueType::Type::BOOL) ||
                (tok->astOperand1()->valueType()->type == ValueType::Type::INT))
                lhsbits = settings.platform.int_bit;
            else if (tok->astOperand1()->valueType()->type == ValueType::Type::LONG)
                lhsbits = settings.platform.long_bit;
            else if (tok->astOperand1()->valueType()->type == ValueType::Type::LONGLONG)
                lhsbits = settings.platform.long_long_bit;
            else
                continue;
            if (rhsvalue >= lhsbits || rhsvalue >= MathLib::bigint_bits || (1ULL << rhsvalue) <= lhsmax)
                continue;

            Value val(0);
            val.setKnown();
            setTokenValue(tok, std::move(val), settings);
        }
    }
}

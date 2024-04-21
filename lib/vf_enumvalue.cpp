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

#include "vf_enumvalue.h"

#include "mathlib.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueflow.h"

#include <list>
#include <memory>
#include <vector>

namespace ValueFlow
{
    void analyzeEnumValue(SymbolDatabase & symboldatabase, const Settings & settings)
    {
        for (Scope & scope : symboldatabase.scopeList) {
            if (scope.type != Scope::eEnum)
                continue;
            MathLib::bigint value = 0;
            bool prev_enum_is_known = true;

            for (Enumerator & enumerator : scope.enumeratorList) {
                if (enumerator.start) {
                    auto* rhs = const_cast<Token*>(enumerator.start->previous()->astOperand2());
                    valueFlowConstantFoldAST(rhs, settings);
                    if (rhs && rhs->hasKnownIntValue()) {
                        enumerator.value = rhs->values().front().intvalue;
                        enumerator.value_known = true;
                        value = enumerator.value + 1;
                        prev_enum_is_known = true;
                    } else
                        prev_enum_is_known = false;
                } else if (prev_enum_is_known) {
                    enumerator.value = value++;
                    enumerator.value_known = true;
                }
            }
        }
    }
}

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#ifndef vfCommonH
#define vfCommonH

#include "mathlib.h"
#include "sourcelocation.h"
#include "symboldatabase.h"
#include "vfvalue.h"

#include <string>

class ValueType;
class Token;
class Settings;

namespace cppcheck {
    class Platform;
}

namespace ValueFlow
{
    bool getMinMaxValues(const ValueType* vt, const cppcheck::Platform& platform, MathLib::bigint* minValue, MathLib::bigint* maxValue);

    Token * valueFlowSetConstantValue(Token *tok, const Settings *settings, bool cpp);

    ValueFlow::Value castValue(ValueFlow::Value value, const ValueType::Sign sign, nonneg int bit);

    std::string debugString(const ValueFlow::Value& v);

    void setSourceLocation(ValueFlow::Value& v,
                           SourceLocation ctx,
                           const Token* tok,
                           SourceLocation local = SourceLocation::current());

    void setTokenValue(Token* tok,
                       ValueFlow::Value value,
                       const Settings* settings,
                       SourceLocation loc = SourceLocation::current());
}

#endif // vfCommonH

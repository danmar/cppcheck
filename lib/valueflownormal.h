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

//---------------------------------------------------------------------------
#ifndef valueflownormalH
#define valueflownormalH
//---------------------------------------------------------------------------

#include "config.h"
#include "vfvalue.h"

#include <list>
#include <string>
#include <utility>

class ErrorLogger;
class Settings;
class SymbolDatabase;
class Token;
class TokenList;
class Variable;

namespace ValueFlowNormal {

    /// Constant folding of expression. This can be used before the full ValueFlow has been executed (ValueFlow::setValues).
    const ValueFlow::Value * valueFlowConstantFoldAST(Token *expr, const Settings *settings);

    /// Perform valueflow analysis.
    void setValues(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings);

    const Variable *getLifetimeVariable(const Token *tok, ValueFlow::Value::ErrorPath &errorPath);

    bool isLifetimeBorrowed(const Token *tok, const Settings *settings);

    std::string lifetimeType(const Token *tok, const ValueFlow::Value *val);

    ValueFlow::Value getLifetimeObjValue(const Token *tok);
}

#endif // valueflowH

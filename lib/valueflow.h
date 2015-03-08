/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#ifndef valueflowH
#define valueflowH
//---------------------------------------------------------------------------

class Token;
class TokenList;
class SymbolDatabase;
class ErrorLogger;
class Settings;

namespace ValueFlow {
    class Value {
    public:
        explicit Value(long long val = 0) : intvalue(val), tokvalue(nullptr), varvalue(val), condition(0), varId(0U), conditional(false), inconclusive(false), defaultArg(false) {}
        Value(const Token *c, long long val) : intvalue(val), tokvalue(nullptr), varvalue(val), condition(c), varId(0U), conditional(false), inconclusive(false), defaultArg(false) {}

        /** int value */
        long long intvalue;

        /** token value - the token that has the value. this is used for pointer aliases, strings, etc. */
        const Token *tokvalue;

        /** For calculated values - variable value that calculated value depends on */
        long long varvalue;

        /** Condition that this value depends on (TODO: replace with a 'callstack') */
        const Token *condition;

        /** For calculated values - varId that calculated value depends on */
        unsigned int varId;

        /** Conditional value */
        bool conditional;

        /** Is this value inconclusive? */
        bool inconclusive;

        /** Is this value passed as default parameter to the function? */
        bool defaultArg;
    };

    void setValues(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings);
}

#endif // valueflowH

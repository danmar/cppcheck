/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
class ErrorLogger;
class Settings;

namespace ValueFlow {
    class Value {
    public:
        Value() : condition(0), conditional(false), intvalue(0), inconclusive(false), varId(0U), varvalue(0) {}
        Value(long long val) : condition(0), conditional(false), intvalue(val), inconclusive(false), varId(0U), varvalue(val) {}
        Value(const Token *c, long long val) : condition(c), conditional(false), intvalue(val), inconclusive(false), varId(0U), varvalue(val) {}

        /** Condition that this value depends on (TODO: replace with a 'callstack') */
        const Token *condition;

        /** Conditional value */
        bool conditional;

        /** int value */
        long long    intvalue;

        /** Is this value inconclusive? */
        bool         inconclusive;

        /** For calculated values - varId that calculated value depends on */
        unsigned int varId;

        /** For calculated values - variable value that calculated value depends on */
        long long    varvalue;
    };

    void setValues(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings);
}

#endif // valueflowH

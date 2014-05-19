/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
        Value(long long val = 0) : intvalue(val), varvalue(val), condition(0), varId(0U), conditional(false), inconclusive(false) {}
        Value(const Token *c, long long val) : intvalue(val), varvalue(val), condition(c), varId(0U), conditional(false), inconclusive(false) {}

        /** int value */
        long long intvalue;

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
    };

    void setValues(TokenList *tokenlist, ErrorLogger *errorLogger, const Settings *settings);
}

#endif // valueflowH

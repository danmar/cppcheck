/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

#include <string>
#include "config.h"

class Token;
class TokenList;
class SymbolDatabase;
class ErrorLogger;
class Settings;

namespace ValueFlow {
    class CPPCHECKLIB Value {
    public:
        explicit Value(long long val = 0) : valueType(INT), intvalue(val), tokvalue(nullptr), floatValue(0.0), moveKind(NonMovedVariable), varvalue(val), condition(0), varId(0U), conditional(false), inconclusive(false), defaultArg(false), valueKind(ValueKind::Possible) {}
        Value(const Token *c, long long val) : valueType(INT), intvalue(val), tokvalue(nullptr), floatValue(0.0), moveKind(NonMovedVariable), varvalue(val), condition(c), varId(0U), conditional(false), inconclusive(false), defaultArg(false), valueKind(ValueKind::Possible) {}

        bool operator==(const Value &rhs) const {
            if (valueType != rhs.valueType)
                return false;
            switch (valueType) {
            case INT:
                if (intvalue != rhs.intvalue)
                    return false;
                break;
            case TOK:
                if (tokvalue != rhs.tokvalue)
                    return false;
                break;
            case FLOAT:
                // TODO: Write some better comparison
                if (floatValue > rhs.floatValue || floatValue < rhs.floatValue)
                    return false;
                break;
            case MOVED:
                if (moveKind != rhs.moveKind)
                    return false;
                break;
            };

            return varvalue == rhs.varvalue &&
                   condition == rhs.condition &&
                   varId == rhs.varId &&
                   conditional == rhs.conditional &&
                   inconclusive == rhs.inconclusive &&
                   defaultArg == rhs.defaultArg &&
                   valueKind == rhs.valueKind;
        }

        enum ValueType { INT, TOK, FLOAT, MOVED } valueType;
        bool isIntValue() const {
            return valueType == INT;
        }
        bool isTokValue() const {
            return valueType == TOK;
        }
        bool isFloatValue() const {
            return valueType == FLOAT;
        }
        bool isMovedValue() const {
            return valueType == MOVED;
        }

        /** int value */
        long long intvalue;

        /** token value - the token that has the value. this is used for pointer aliases, strings, etc. */
        const Token *tokvalue;

        /** float value */
        double floatValue;

        /** kind of moved  */
        enum MoveKind {NonMovedVariable, MovedVariable, ForwardedVariable} moveKind;

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

        static const char * toString(MoveKind moveKind) {
            switch (moveKind) {
            case NonMovedVariable:
                return "NonMovedVariable";
            case MovedVariable:
                return "MovedVariable";
            case ForwardedVariable:
                return "ForwardedVariable";
            }
            return "";
        }

        /** How known is this value */
        enum ValueKind {
            /** This value is possible, other unlisted values may also be possible */
            Possible,
            /** Only listed values are possible */
            Known
        } valueKind;

        void setKnown() {
            valueKind = ValueKind::Known;
        }

        bool isKnown() const {
            return valueKind == ValueKind::Known;
        }

        void setPossible() {
            valueKind = ValueKind::Possible;
        }

        bool isPossible() const {
            return valueKind == ValueKind::Possible;
        }

        void changeKnownToPossible() {
            if (isKnown())
                valueKind = ValueKind::Possible;
        }
    };

    /// Constant folding of expression. This can be used before the full ValueFlow has been executed (ValueFlow::setValues).
    const ValueFlow::Value * valueFlowConstantFoldAST(const Token *expr, const Settings *settings);

    /// Perform valueflow analysis.
    void setValues(TokenList *tokenlist, SymbolDatabase* symboldatabase, ErrorLogger *errorLogger, const Settings *settings);

    std::string eitherTheConditionIsRedundant(const Token *condition);
}

#endif // valueflowH

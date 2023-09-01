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
#ifndef checktypeH
#define checktypeH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "tokenize.h"
#include "vfvalue.h"

#include <list>
#include <string>

class ErrorLogger;
class Settings;
class Token;
class ValueType;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB CheckType : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckType() : Check(myName()) {}

    /** @brief This constructor is used when running checks. */
    CheckType(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override {
        // These are not "simplified" because casts can't be ignored
        CheckType checkType(&tokenizer, tokenizer.getSettings(), errorLogger);
        checkType.checkTooBigBitwiseShift();
        checkType.checkIntegerOverflow();
        checkType.checkSignConversion();
        checkType.checkLongCast();
        checkType.checkFloatToIntegerOverflow();
    }

    /** @brief %Check for bitwise shift with too big right operand */
    void checkTooBigBitwiseShift();

    /** @brief %Check for integer overflow */
    void checkIntegerOverflow();

    /** @brief %Check for dangerous sign conversion */
    void checkSignConversion();

    /** @brief %Check for implicit long cast of int result */
    void checkLongCast();

    /** @brief %Check for float to integer overflow */
    void checkFloatToIntegerOverflow();
    void checkFloatToIntegerOverflow(const Token *tok, const ValueType *vtint, const ValueType *vtfloat, const std::list<ValueFlow::Value> &floatValues);

private:

    // Error messages..
    void tooBigBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits);
    void tooBigSignedBitwiseShiftError(const Token *tok, int lhsbits, const ValueFlow::Value &rhsbits);
    void integerOverflowError(const Token *tok, const ValueFlow::Value &value);
    void signConversionError(const Token *tok, const ValueFlow::Value *negativeValue, const bool constvalue);
    void longCastAssignError(const Token *tok, const ValueType* src = nullptr, const ValueType* tgt = nullptr);
    void longCastReturnError(const Token *tok, const ValueType* src = nullptr, const ValueType* tgt = nullptr);
    void floatToIntegerOverflowError(const Token *tok, const ValueFlow::Value &value);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override {
        CheckType c(nullptr, settings, errorLogger);
        c.tooBigBitwiseShiftError(nullptr, 32, ValueFlow::Value(64));
        c.tooBigSignedBitwiseShiftError(nullptr, 31, ValueFlow::Value(31));
        c.integerOverflowError(nullptr, ValueFlow::Value(1LL<<32));
        c.signConversionError(nullptr, nullptr, false);
        c.longCastAssignError(nullptr);
        c.longCastReturnError(nullptr);
        ValueFlow::Value f;
        f.valueType = ValueFlow::Value::ValueType::FLOAT;
        f.floatValue = 1E100;
        c.floatToIntegerOverflowError(nullptr, f);
    }

    static std::string myName() {
        return "Type";
    }

    std::string classInfo() const override {
        return "Type checks\n"
               "- bitwise shift by too many bits (only enabled when --platform is used)\n"
               "- signed integer overflow (only enabled when --platform is used)\n"
               "- dangerous sign conversion, when signed value can be negative\n"
               "- possible loss of information when assigning int result to long variable\n"
               "- possible loss of information when returning int result as long return value\n"
               "- float conversion overflow\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checktypeH

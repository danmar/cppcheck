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
#ifndef CheckSizeofH
#define CheckSizeofH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include "settings.h"

class Token;
class Function;
class Variable;

/// @addtogroup Checks
/// @{


/** @brief checks on usage of sizeof() operator */

class CPPCHECKLIB CheckSizeof : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckSizeof() : Check(myName())
    { }

    /** @brief This constructor is used when running checks. */
    CheckSizeof(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger) {
        CheckSizeof checkSizeof(tokenizer, settings, errorLogger);

        // Checks
        checkSizeof.sizeofsizeof();
        checkSizeof.sizeofCalculation();
        checkSizeof.suspiciousSizeofCalculation();
        checkSizeof.checkSizeofForArrayParameter();
        checkSizeof.checkSizeofForPointerSize();
        checkSizeof.checkSizeofForNumericParameter();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer*, const Settings*, ErrorLogger*) {
    }

    /** @brief %Check for 'sizeof sizeof ..' */
    void sizeofsizeof();

    /** @brief %Check for calculations inside sizeof */
    void sizeofCalculation();

    /** @brief %Check for suspicious calculations with sizeof results */
    void suspiciousSizeofCalculation();

    /** @brief %Check for using sizeof with array given as function argument */
    void checkSizeofForArrayParameter();

    /** @brief %Check for using sizeof of a variable when allocating it */
    void checkSizeofForPointerSize();

    /** @brief %Check for using sizeof with numeric given as function argument */
    void checkSizeofForNumericParameter();

private:
    // Error messages..
    void sizeofsizeofError(const Token* tok);
    void sizeofCalculationError(const Token* tok, bool inconclusive);
    void multiplySizeofError(const Token* tok);
    void divideSizeofError(const Token* tok);
    void sizeofForArrayParameterError(const Token* tok);
    void sizeofForPointerError(const Token* tok, const std::string &varname);
    void sizeofForNumericParameterError(const Token* tok);

    void getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const {
        CheckSizeof c(0, settings, errorLogger);

        c.sizeofForArrayParameterError(0);
        c.sizeofForPointerError(0, "varname");
        c.sizeofForNumericParameterError(0);
        c.sizeofsizeofError(0);
        c.sizeofCalculationError(0, false);
        c.multiplySizeofError(0);
        c.divideSizeofError(0);
    }

    static std::string myName() {
        return "Sizeof";
    }

    std::string classInfo() const {
        return "sizeof() usage checks\n"

               "* sizeof for array given as function argument\n"
               "* sizeof for numeric given as function argument\n"
               "* using sizeof(pointer) instead of the size of pointed data\n"
               "* look for 'sizeof sizeof ..'\n"
               "* look for calculations inside sizeof()\n"
               "* look for suspicious calculations with sizeof()\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif

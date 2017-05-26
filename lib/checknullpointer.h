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
#ifndef checknullpointerH
#define checknullpointerH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "valueflow.h"

#include <list>
#include <string>

class ErrorLogger;
class Library;
class Settings;
class Token;
class Tokenizer;


/// @addtogroup Checks
/// @{


/** @brief check for null pointer dereferencing */

class CPPCHECKLIB CheckNullPointer : public Check {
public:
    /** @brief This constructor is used when registering the CheckNullPointer */
    CheckNullPointer() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckNullPointer(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckNullPointer checkNullPointer(tokenizer, settings, errorLogger);
        checkNullPointer.nullPointer();
        checkNullPointer.arithmetic();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckNullPointer checkNullPointer(tokenizer, settings, errorLogger);
        checkNullPointer.nullConstantDereference();
    }

    /**
     * @brief parse a function call and extract information about variable usage
     * @param tok first token
     * @param var variables that the function read / write.
     * @param library --library files data
     */
    static void parseFunctionCall(const Token &tok,
                                  std::list<const Token *> &var,
                                  const Library *library);

    /**
     * Is there a pointer dereference? Everything that should result in
     * a nullpointer dereference error message will result in a true
     * return value. If it's unknown if the pointer is dereferenced false
     * is returned.
     * @param tok token for the pointer
     * @param unknown it is not known if there is a pointer dereference (could be reported as a debug message)
     * @return true => there is a dereference
     */
    static bool isPointerDeRef(const Token *tok, bool &unknown);

    /** @brief possible null pointer dereference */
    void nullPointer();

    /** @brief dereferencing null constant (after Tokenizer::simplifyKnownVariables) */
    void nullConstantDereference();

    void nullPointerError(const Token *tok) {
        ValueFlow::Value v(0);
        v.setKnown();
        nullPointerError(tok, "", &v, false);
    }
    void nullPointerError(const Token *tok, const std::string &varname, const ValueFlow::Value* value, bool inconclusive);
private:

    /** Get error messages. Used by --errorlist */
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckNullPointer c(nullptr, settings, errorLogger);
        c.nullPointerError(nullptr, "pointer", nullptr, false);
        c.arithmeticError(nullptr, nullptr);
    }

    /** Name of check */
    static std::string myName() {
        return "Null pointer";
    }

    /** class info in WIKI format. Used by --doc */
    std::string classInfo() const {
        return "Null pointers\n"
               "- null pointer dereferencing\n"
               "- undefined null pointer arithmetic\n";
    }

    /**
     * @brief Does one part of the check for nullPointer().
     * looping through items in a linked list in a inner loop..
     */
    void nullPointerLinkedList();

    /**
     * @brief Does one part of the check for nullPointer().
     * Dereferencing a pointer and then checking if it's NULL..
     */
    void nullPointerByDeRefAndChec();

    /** undefined null pointer arithmetic */
    void arithmetic();
    void arithmeticError(const Token *tok, const ValueFlow::Value *value);
};
/// @}
//---------------------------------------------------------------------------
#endif // checknullpointerH

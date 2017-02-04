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
#ifndef checkexceptionsafetyH
#define checkexceptionsafetyH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include "utils.h"

// CWE ID used:
static const struct CWE CWE398(398U);   // Indicator of Poor Code Quality
static const struct CWE CWE703(703U);   // Improper Check or Handling of Exceptional Conditions


/// @addtogroup Checks
/// @{


/**
 * @brief %Check exception safety (exceptions shouldn't cause leaks nor corrupt data)
 *
 * The problem with these checks is that Cppcheck can't determine what the valid
 * values are for variables. But in some cases (dead pointers) it can be determined
 * that certain variable values are corrupt.
 */

class CPPCHECKLIB CheckExceptionSafety : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckExceptionSafety() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckExceptionSafety(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** Checks that uses the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        if (tokenizer->isC())
            return;

        CheckExceptionSafety checkExceptionSafety(tokenizer, settings, errorLogger);
        checkExceptionSafety.destructors();
        checkExceptionSafety.deallocThrow();
        checkExceptionSafety.checkRethrowCopy();
        checkExceptionSafety.checkCatchExceptionByValue();
        checkExceptionSafety.nothrowThrows();
        checkExceptionSafety.unhandledExceptionSpecification();
    }

    /** Don't throw exceptions in destructors */
    void destructors();

    /** deallocating memory and then throw (dead pointer) */
    void deallocThrow();

    /** Don't rethrow a copy of the caught exception; use a bare throw instead */
    void checkRethrowCopy();

    /** @brief %Check for exceptions that are caught by value instead of by reference */
    void checkCatchExceptionByValue();

    /** @brief %Check for functions that throw that shouldn't */
    void nothrowThrows();

    /** @brief %Check for unhandled exception specification */
    void unhandledExceptionSpecification();

private:
    /** Don't throw exceptions in destructors */
    void destructorsError(const Token * const tok, const std::string &className) {
        reportError(tok, Severity::warning, "exceptThrowInDestructor",
                    "Class " + className + " is not safe, destructor throws exception\n"
                    "The class " + className + " is not safe because its destructor "
                    "throws an exception. If " + className + " is used and an exception "
                    "is thrown that is caught in an outer scope the program will terminate.", CWE398, false);
    }

    void deallocThrowError(const Token * const tok, const std::string &varname) {
        reportError(tok, Severity::warning, "exceptDeallocThrow", "Exception thrown in invalid state, '" +
                    varname + "' points at deallocated memory.", CWE398, false);
    }

    void rethrowCopyError(const Token * const tok, const std::string &varname) {
        reportError(tok, Severity::style, "exceptRethrowCopy",
                    "Throwing a copy of the caught exception instead of rethrowing the original exception.\n"
                    "Rethrowing an exception with 'throw " + varname + ";' creates an unnecessary copy of '" + varname + "'. "
                    "To rethrow the caught exception without unnecessary copying or slicing, use a bare 'throw;'.", CWE398, false);
    }

    void catchExceptionByValueError(const Token *tok) {
        reportError(tok, Severity::style,
                    "catchExceptionByValue", "Exception should be caught by reference.\n"
                    "The exception is caught by value. It could be caught "
                    "as a (const) reference which is usually recommended in C++.", CWE398, false);
    }

    void noexceptThrowError(const Token * const tok) {
        reportError(tok, Severity::error, "throwInNoexceptFunction", "Exception thrown in function declared not to throw exceptions.", CWE398, false);
    }

    /** Missing exception specification */
    void unhandledExceptionSpecificationError(const Token * const tok1, const Token * const tok2, const std::string & funcname) {
        std::string str1(tok1 ? tok1->str() : "foo");
        const std::list<const Token*> locationList = make_container< std::list<const Token*> > () << tok1 << tok2;
        reportError(locationList, Severity::style, "unhandledExceptionSpecification",
                    "Unhandled exception specification when calling function " + str1 + "().\n"
                    "Unhandled exception specification when calling function " + str1 + "(). "
                    "Either use a try/catch around the function call, or add a exception specification for " + funcname + "() also.", CWE703, true);
    }

    /** Generate all possible errors (for --errorlist) */
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckExceptionSafety c(nullptr, settings, errorLogger);
        c.destructorsError(nullptr, "Class");
        c.deallocThrowError(nullptr, "p");
        c.rethrowCopyError(nullptr, "varname");
        c.catchExceptionByValueError(nullptr);
        c.noexceptThrowError(nullptr);
        c.unhandledExceptionSpecificationError(nullptr, 0, "funcname");
    }

    /** Short description of class (for --doc) */
    static std::string myName() {
        return "Exception Safety";
    }

    /** wiki formatted description of the class (for --doc) */
    std::string classInfo() const {
        return "Checking exception safety\n"
               "- Throwing exceptions in destructors\n"
               "- Throwing exception during invalid state\n"
               "- Throwing a copy of a caught exception instead of rethrowing the original exception\n"
               "- Exception caught by value instead of by reference\n"
               "- Throwing exception in noexcept, nothrow(), __attribute__((nothrow)) or __declspec(nothrow) function\n"
               "- Unhandled exception specification when calling function foo()\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkexceptionsafetyH

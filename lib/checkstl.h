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
#ifndef checkstlH
#define checkstlH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"


/// @addtogroup Checks
/// @{


/** @brief %Check STL usage (invalidation of iterators, mismatching containers, etc) */
class CPPCHECKLIB CheckStl : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckStl() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckStl(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** Simplified checks. The token list is simplified. */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        if (!tokenizer->isCPP())
            return;

        CheckStl checkStl(tokenizer, settings, errorLogger);

        checkStl.stlOutOfBounds();
        checkStl.iterators();
        checkStl.mismatchingContainers();
        checkStl.erase();
        checkStl.pushback();
        checkStl.stlBoundaries();
        checkStl.if_find();
        checkStl.string_c_str();
        checkStl.checkAutoPointer();
        checkStl.uselessCalls();
        checkStl.checkDereferenceInvalidIterator();

        // Style check
        checkStl.size();
        checkStl.redundantCondition();
        checkStl.missingComparison();
        checkStl.readingEmptyStlContainer();
    }


    /**
     * Finds errors like this:
     * for (unsigned ii = 0; ii <= foo.size(); ++ii)
     */
    void stlOutOfBounds();

    /**
     * Finds errors like this:
     * for (it = foo.begin(); it != bar.end(); ++it)
     */
    void iterators();

    /**
     * Mismatching containers:
     * std::find(foo.begin(), bar.end(), x)
     */
    void mismatchingContainers();

    /**
     * Dangerous usage of erase. The iterator is invalidated by erase so
     * it is bad to dereference it after the erase.
     */
    void erase();


    /**
     * Dangerous usage of push_back and insert
     */
    void pushback();

    /**
     * bad condition.. "it < alist.end()"
     */
    void stlBoundaries();

    /** if (a.find(x)) - possibly incorrect condition */
    void if_find();

    /**
     * Suggest using empty() instead of checking size() against zero for containers.
     * Item 4 from Scott Meyers book "Effective STL".
     */
    void size();

    /**
     * Check for redundant condition 'if (ints.find(1) != ints.end()) ints.remove(123);'
     * */
    void redundantCondition();

    /**
     * @brief Missing inner comparison, when incrementing iterator inside loop
     * Dangers:
     *  - may increment iterator beyond end
     *  - may unintentionally skip elements in list/set etc
     */
    void missingComparison();

    /** Check for common mistakes when using the function string::c_str() */
    void string_c_str();

    /** @brief %Check for use and copy auto pointer */
    void checkAutoPointer();

    /** @brief %Check calls that using them is useless */
    void uselessCalls();

    /** @brief %Check for dereferencing an iterator that is invalid */
    void checkDereferenceInvalidIterator();

    /**
     * Dereferencing an erased iterator
     * @param erased token where the erase occurs
     * @param deref token where the dereference occurs
     * @param itername iterator name
     */
    void dereferenceErasedError(const Token* erased, const Token* deref, const std::string &itername);

    /** @brief Reading from empty stl container */
    void readingEmptyStlContainer();

private:

    /**
     * Helper function used by the 'erase' function
     * This function parses a loop
     * @param it iterator token
     */
    void eraseCheckLoop(const Token *it);

    void missingComparisonError(const Token *incrementToken1, const Token *incrementToken2);
    void string_c_strThrowError(const Token *tok);
    void string_c_strError(const Token *tok);
    void string_c_strReturn(const Token *tok);
    void string_c_strParam(const Token *tok, unsigned int number);

    void stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var, bool at);
    void invalidIteratorError(const Token *tok, const std::string &iteratorName);
    void iteratorsError(const Token *tok, const std::string &container1, const std::string &container2);
    void mismatchingContainersError(const Token *tok);
    void invalidIteratorError(const Token *tok, const std::string &func, const std::string &iterator_name);
    void invalidPointerError(const Token *tok, const std::string &func, const std::string &pointer_name);
    void stlBoundariesError(const Token *tok, const std::string &container_name);
    void if_findError(const Token *tok, bool str);
    void sizeError(const Token *tok);
    void redundantIfRemoveError(const Token *tok);

    void autoPointerError(const Token *tok);
    void autoPointerContainerError(const Token *tok);
    void autoPointerArrayError(const Token *tok);
    void autoPointerMallocError(const Token *tok, const std::string& allocFunction);

    void uselessCallsReturnValueError(const Token *tok, const std::string &varname, const std::string &function);
    void uselessCallsSwapError(const Token *tok, const std::string &varname);
    void uselessCallsSubstrError(const Token *tok, bool empty);
    void uselessCallsEmptyError(const Token *tok);
    void uselessCallsRemoveError(const Token *tok, const std::string& function);

    void dereferenceInvalidIteratorError(const Token* deref, const std::string &itername);

    void readingEmptyStlContainerError(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckStl c(0, settings, errorLogger);
        c.invalidIteratorError(0, "iterator");
        c.iteratorsError(0, "container1", "container2");
        c.mismatchingContainersError(0);
        c.dereferenceErasedError(0, 0, "iter");
        c.stlOutOfBoundsError(0, "i", "foo", false);
        c.invalidIteratorError(0, "push_back|push_front|insert", "iterator");
        c.invalidPointerError(0, "push_back", "pointer");
        c.stlBoundariesError(0, "container");
        c.if_findError(0, false);
        c.if_findError(0, true);
        c.string_c_strError(0);
        c.string_c_strReturn(0);
        c.string_c_strParam(0, 0);
        c.sizeError(0);
        c.missingComparisonError(0, 0);
        c.redundantIfRemoveError(0);
        c.autoPointerError(0);
        c.autoPointerContainerError(0);
        c.autoPointerArrayError(0);
        c.autoPointerMallocError(0, "malloc");
        c.uselessCallsReturnValueError(0, "str", "find");
        c.uselessCallsSwapError(0, "str");
        c.uselessCallsSubstrError(0, false);
        c.uselessCallsEmptyError(0);
        c.uselessCallsRemoveError(0, "remove");
        c.dereferenceInvalidIteratorError(0, "i");
        c.readingEmptyStlContainerError(0);
    }

    static std::string myName() {
        return "STL usage";
    }

    std::string classInfo() const {
        return "Check for invalid usage of STL:\n"
               "- out of bounds errors\n"
               "- misuse of iterators when iterating through a container\n"
               "- mismatching containers in calls\n"
               "- dereferencing an erased iterator\n"
               "- for vectors: using iterator/pointer after push_back has been used\n"
               "- optimisation: use empty() instead of size() to guarantee fast code\n"
               "- suspicious condition when using find\n"
               "- redundant condition\n"
               "- common mistakes when using string::c_str()\n"
               "- using auto pointer (auto_ptr)\n"
               "- useless calls of string and STL functions\n"
               "- dereferencing an invalid iterator\n"
               "- reading from empty STL container\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkstlH

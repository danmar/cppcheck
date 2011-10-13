/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#include "check.h"

class Token;

/// @addtogroup Checks
/// @{


/** @brief %Check STL usage (invalidation of iterators, mismatching containers, etc) */
class CheckStl : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckStl() : Check(myName())
    { }

    /** This constructor is used when running checks. */
    CheckStl(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** Simplified checks. The token list is simplified. */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckStl checkStl(tokenizer, settings, errorLogger);

        checkStl.stlOutOfBounds();
        checkStl.iterators();
        checkStl.mismatchingContainers();
        checkStl.erase();
        checkStl.pushback();
        checkStl.stlBoundries();
        checkStl.if_find();
        checkStl.string_c_str();
        checkStl.checkAutoPointer();

        // Style check
        checkStl.size();
        checkStl.redundantCondition();
        checkStl.missingComparison();

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
     * Dereferencing an erased iterator
     * @param tok token where error occurs
     * @param itername iterator name
     */
    void dereferenceErasedError(const Token *tok, const std::string &itername);

    /**
     * Dangerous usage of erase. The iterator is invalidated by erase so
     * it is bad to dereference it after the erase.
     */
    void erase();
    void eraseError(const Token *tok);


    /**
     * Dangerous usage of push_back and insert
     */
    void pushback();

    /**
     * bad condition.. "it < alist.end()"
     */
    void stlBoundries();

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
    void missingComparisonError(const Token *incrementToken1, const Token *incrementToken2);

    /** Check for common mistakes when using the function string::c_str() */
    void string_c_str();
    void string_c_strError(const Token *tok);

    /** @brief %Check for use and copy auto pointer */
    void checkAutoPointer();


private:

    /**
     * Helper function used by the 'erase' function
     * This function parses a loop
     * @param it iterator token
     */
    void eraseCheckLoop(const Token *it);

    void stlOutOfBoundsError(const Token *tok, const std::string &num, const std::string &var);
    void invalidIteratorError(const Token *tok, const std::string &iteratorName);
    void iteratorsError(const Token *tok, const std::string &container1, const std::string &container2);
    void mismatchingContainersError(const Token *tok);
    void invalidIteratorError(const Token *tok, const std::string &func, const std::string &iterator_name);
    void invalidPointerError(const Token *tok, const std::string &pointer_name);
    void stlBoundriesError(const Token *tok, const std::string &container_name);
    void if_findError(const Token *tok, bool str);
    void sizeError(const Token *tok);
    void redundantIfRemoveError(const Token *tok);

    void autoPointerError(const Token *tok);
    void autoPointerContainerError(const Token *tok);
    void autoPointerArrayError(const Token *tok);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) {
        CheckStl c(0, settings, errorLogger);
        c.invalidIteratorError(0, "iterator");
        c.iteratorsError(0, "container1", "container2");
        c.mismatchingContainersError(0);
        c.dereferenceErasedError(0, "iter");
        c.stlOutOfBoundsError(0, "i", "foo");
        c.eraseError(0);
        c.invalidIteratorError(0, "push_back|push_front|insert", "iterator");
        c.invalidPointerError(0, "pointer");
        c.stlBoundriesError(0, "container");
        c.if_findError(0, false);
        c.if_findError(0, true);
        c.string_c_strError(0);
        c.sizeError(0);
        c.redundantIfRemoveError(0);
        c.autoPointerError(0);
        c.autoPointerContainerError(0);
        c.autoPointerArrayError(0);
    }

    std::string myName() const {
        return "STL usage";
    }

    std::string classInfo() const {
        return "Check for invalid usage of STL:\n"
               "* out of bounds errors\n"
               "* misuse of iterators when iterating through a container\n"
               "* mismatching containers in calls\n"
               "* dereferencing an erased iterator\n"
               "* for vectors: using iterator/pointer after push_back has been used\n"
               "* optimisation: use empty() instead of size() to guarantee fast code\n"
               "* suspicious condition when using find\n"
               "* redundant condition\n"
               "* common mistakes when using string::c_str()\n"
               "* using auto pointer (auto_ptr)";
    }

    bool isStlContainer(unsigned int varid);
};
/// @}
//---------------------------------------------------------------------------
#endif


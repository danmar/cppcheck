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
#ifndef checkstlH
#define checkstlH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "errortypes.h"
#include "tokenize.h"
#include "vfvalue.h"

#include <string>

class Scope;
class Settings;
class Token;
class Variable;
class ErrorLogger;


/// @addtogroup Checks
/// @{


/** @brief %Check STL usage (invalidation of iterators, mismatching containers, etc) */
class CPPCHECKLIB CheckStl : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckStl() : Check(myName()) {}

    /** This constructor is used when running checks. */
    CheckStl(const Tokenizer* tokenizer, const Settings* settings, ErrorLogger* errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    /** run checks, the token list is not simplified */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override {
        if (!tokenizer.isCPP()) {
            return;
        }

        CheckStl checkStl(&tokenizer, tokenizer.getSettings(), errorLogger);
        checkStl.erase();
        checkStl.if_find();
        checkStl.checkFindInsert();
        checkStl.iterators();
        checkStl.missingComparison();
        checkStl.outOfBounds();
        checkStl.outOfBoundsIndexExpression();
        checkStl.redundantCondition();
        checkStl.string_c_str();
        checkStl.uselessCalls();
        checkStl.useStlAlgorithm();

        checkStl.stlOutOfBounds();
        checkStl.negativeIndex();

        checkStl.invalidContainer();
        checkStl.mismatchingContainers();
        checkStl.mismatchingContainerIterator();
        checkStl.knownEmptyContainer();

        checkStl.stlBoundaries();
        checkStl.checkDereferenceInvalidIterator();
        checkStl.checkDereferenceInvalidIterator2();
        checkStl.checkMutexes();

        // Style check
        checkStl.size();
    }

    /** Accessing container out of bounds using ValueFlow */
    void outOfBounds();

    /** Accessing container out of bounds, following index expression */
    void outOfBoundsIndexExpression();

    /**
     * Finds errors like this:
     * for (unsigned ii = 0; ii <= foo.size(); ++ii)
     */
    void stlOutOfBounds();

    /**
     * negative index for array like containers
     */
    void negativeIndex();

    /**
     * Finds errors like this:
     * for (it = foo.begin(); it != bar.end(); ++it)
     */
    void iterators();

    void invalidContainer();

    bool checkIteratorPair(const Token* tok1, const Token* tok2);

    /**
     * Mismatching containers:
     * std::find(foo.begin(), bar.end(), x)
     */
    void mismatchingContainers();

    void mismatchingContainerIterator();

    /**
     * Dangerous usage of erase. The iterator is invalidated by erase so
     * it is bad to dereference it after the erase.
     */
    void erase();
    void eraseCheckLoopVar(const Scope& scope, const Variable* var);

    /**
     * bad condition.. "it < alist.end()"
     */
    void stlBoundaries();

    /** if (a.find(x)) - possibly incorrect condition */
    void if_find();

    void checkFindInsert();

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

    /** @brief %Check calls that using them is useless */
    void uselessCalls();

    /** @brief %Check for dereferencing an iterator that is invalid */
    void checkDereferenceInvalidIterator();
    void checkDereferenceInvalidIterator2();

    /**
     * Dereferencing an erased iterator
     * @param erased token where the erase occurs
     * @param deref token where the dereference occurs
     * @param itername iterator name
     * @param inconclusive inconclusive flag
     */
    void dereferenceErasedError(const Token* erased, const Token* deref, const std::string& itername, bool inconclusive);

    /** @brief Look for loops that can replaced with std algorithms */
    void useStlAlgorithm();

    void knownEmptyContainer();

    void checkMutexes();

private:
    bool isContainerSize(const Token *containerToken, const Token *expr) const;
    bool isContainerSizeGE(const Token * containerToken, const Token *expr) const;

    void missingComparisonError(const Token* incrementToken1, const Token* incrementToken2);
    void string_c_strThrowError(const Token* tok);
    void string_c_strError(const Token* tok);
    void string_c_strReturn(const Token* tok);
    void string_c_strParam(const Token* tok, nonneg int number, const std::string& argtype = "std::string");
    void string_c_strConstructor(const Token* tok, const std::string& argtype = "std::string");
    void string_c_strAssignment(const Token* tok, const std::string& argtype = "std::string");
    void string_c_strConcat(const Token* tok);
    void string_c_strStream(const Token* tok);

    void outOfBoundsError(const Token *tok, const std::string &containerName, const ValueFlow::Value *containerSize, const std::string &index, const ValueFlow::Value *indexValue);
    void outOfBoundsIndexExpressionError(const Token *tok, const Token *index);
    void stlOutOfBoundsError(const Token* tok, const std::string& num, const std::string& var, bool at);
    void negativeIndexError(const Token* tok, const ValueFlow::Value& index);
    void invalidIteratorError(const Token* tok, const std::string& iteratorName);
    void iteratorsError(const Token* tok, const std::string& containerName1, const std::string& containerName2);
    void iteratorsError(const Token* tok, const Token* containerTok, const std::string& containerName1, const std::string& containerName2);
    void iteratorsError(const Token* tok, const Token* containerTok, const std::string& containerName);
    void mismatchingContainerIteratorError(const Token* tok, const Token* iterTok);
    void mismatchingContainersError(const Token* tok1, const Token* tok2);
    void mismatchingContainerExpressionError(const Token *tok1, const Token *tok2);
    void sameIteratorExpressionError(const Token *tok);
    void stlBoundariesError(const Token* tok);
    void if_findError(const Token* tok, bool str);
    void checkFindInsertError(const Token *tok);
    void sizeError(const Token* tok);
    void redundantIfRemoveError(const Token* tok);
    void invalidContainerLoopError(const Token* tok, const Token* loopTok, ErrorPath errorPath);
    void invalidContainerError(const Token *tok, const Token * contTok, const ValueFlow::Value *val, ErrorPath errorPath);
    void invalidContainerReferenceError(const Token* tok, const Token* contTok, ErrorPath errorPath);

    void uselessCallsReturnValueError(const Token* tok, const std::string& varname, const std::string& function);
    void uselessCallsSwapError(const Token* tok, const std::string& varname);
    enum class SubstrErrorType { EMPTY, COPY, PREFIX, PREFIX_CONCAT };
    void uselessCallsSubstrError(const Token* tok, SubstrErrorType type);
    void uselessCallsEmptyError(const Token* tok);
    void uselessCallsRemoveError(const Token* tok, const std::string& function);
    void uselessCallsConstructorError(const Token* tok);

    void dereferenceInvalidIteratorError(const Token* deref, const std::string& iterName);
    void dereferenceInvalidIteratorError(const Token* tok, const ValueFlow::Value *value, bool inconclusive);

    void useStlAlgorithmError(const Token *tok, const std::string &algoName);

    void knownEmptyContainerError(const Token *tok, const std::string& algo);

    void globalLockGuardError(const Token *tok);
    void localMutexError(const Token *tok);

    void getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const override {
        ErrorPath errorPath;
        CheckStl c(nullptr, settings, errorLogger);
        c.outOfBoundsError(nullptr, "container", nullptr, "x", nullptr);
        c.invalidIteratorError(nullptr, "iterator");
        c.iteratorsError(nullptr, "container1", "container2");
        c.iteratorsError(nullptr, nullptr, "container0", "container1");
        c.iteratorsError(nullptr, nullptr, "container");
        c.invalidContainerLoopError(nullptr, nullptr, errorPath);
        c.invalidContainerError(nullptr, nullptr, nullptr, errorPath);
        c.mismatchingContainerIteratorError(nullptr, nullptr);
        c.mismatchingContainersError(nullptr, nullptr);
        c.mismatchingContainerExpressionError(nullptr, nullptr);
        c.sameIteratorExpressionError(nullptr);
        c.dereferenceErasedError(nullptr, nullptr, "iter", false);
        c.stlOutOfBoundsError(nullptr, "i", "foo", false);
        c.negativeIndexError(nullptr, ValueFlow::Value(-1));
        c.stlBoundariesError(nullptr);
        c.if_findError(nullptr, false);
        c.if_findError(nullptr, true);
        c.checkFindInsertError(nullptr);
        c.string_c_strError(nullptr);
        c.string_c_strReturn(nullptr);
        c.string_c_strParam(nullptr, 0);
        c.string_c_strThrowError(nullptr);
        c.sizeError(nullptr);
        c.missingComparisonError(nullptr, nullptr);
        c.redundantIfRemoveError(nullptr);
        c.uselessCallsReturnValueError(nullptr, "str", "find");
        c.uselessCallsSwapError(nullptr, "str");
        c.uselessCallsSubstrError(nullptr, SubstrErrorType::COPY);
        c.uselessCallsEmptyError(nullptr);
        c.uselessCallsRemoveError(nullptr, "remove");
        c.dereferenceInvalidIteratorError(nullptr, "i");
        c.useStlAlgorithmError(nullptr, emptyString);
        c.knownEmptyContainerError(nullptr, emptyString);
        c.globalLockGuardError(nullptr);
        c.localMutexError(nullptr);
    }

    static std::string myName() {
        return "STL usage";
    }

    std::string classInfo() const override {
        return "Check for invalid usage of STL:\n"
               "- out of bounds errors\n"
               "- misuse of iterators when iterating through a container\n"
               "- mismatching containers in calls\n"
               "- same iterators in calls\n"
               "- dereferencing an erased iterator\n"
               "- for vectors: using iterator/pointer after push_back has been used\n"
               "- optimisation: use empty() instead of size() to guarantee fast code\n"
               "- suspicious condition when using find\n"
               "- unnecessary searching in associative containers\n"
               "- redundant condition\n"
               "- common mistakes when using string::c_str()\n"
               "- useless calls of string and STL functions\n"
               "- dereferencing an invalid iterator\n"
               "- reading from empty STL container\n"
               "- iterating over an empty STL container\n"
               "- consider using an STL algorithm instead of raw loop\n"
               "- incorrect locking with mutex\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkstlH

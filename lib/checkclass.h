/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#ifndef CheckClassH
#define CheckClassH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"
#include "symboldatabase.h"

class Token;

/// @addtogroup Checks
/// @{


/** @brief %Check classes. Uninitialized member variables, non-conforming operators, missing virtual destructor, etc */
class CheckClass : public Check
{
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckClass() : Check(), symbolDatabase(NULL)
    { }

    /** @brief This constructor is used when running checks. */
    CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);

    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckClass checkClass(tokenizer, settings, errorLogger);

        // can't be a simplified check .. the 'sizeof' is used.
        checkClass.noMemset();
    }

    /** @brief Run checks on the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckClass checkClass(tokenizer, settings, errorLogger);

        // Coding style checks
        checkClass.constructors();
        checkClass.operatorEq();
        checkClass.privateFunctions();
        checkClass.operatorEqRetRefThis();
        checkClass.thisSubtraction();
        checkClass.operatorEqToSelf();

        checkClass.virtualDestructor();
        checkClass.checkConst();
    }


    /** @brief %Check that all class constructors are ok */
    void constructors();

    /** @brief %Check that all private functions are called */
    void privateFunctions();

    /**
     * @brief %Check that the memsets are valid.
     * The 'memset' function can do dangerous things if used wrong. If it
     * is used on STL containers for instance it will clear all its data
     * and then the STL container may leak memory or worse have an invalid state.
     * It can also overwrite the virtual table.
     * Important: The checking doesn't work on simplified tokens list.
     */
    void noMemset();

    /** @brief 'operator=' should return something and it should not be const. */
    void operatorEq();

    /** @brief 'operator=' should return reference to *this */
    void operatorEqRetRefThis();    // Warning upon no "return *this;"

    /** @brief 'operator=' should check for assignment to self */
    void operatorEqToSelf();    // Warning upon no check for assignment to self

    /** @brief The destructor in a base class should be virtual */
    void virtualDestructor();

    /** @brief warn for "this-x". The indented code may be "this->x"  */
    void thisSubtraction();

    /** @brief can member function be const? */
    void checkConst();

private:
    /**
     * @brief Create symbol database. For performance reasons, only call
     * it if it's needed.
     */
    void createSymbolDatabase();

    SymbolDatabase *symbolDatabase;

    // Reporting errors..
    void noConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    void uninitVarError(const Token *tok, const std::string &classname, const std::string &varname);
    void operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname);
    void unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname);
    void memsetClassError(const Token *tok, const std::string &memfunc);
    void memsetStructError(const Token *tok, const std::string &memfunc, const std::string &classname);
    void operatorEqReturnError(const Token *tok);
    void virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived);
    void thisSubtractionError(const Token *tok);
    void operatorEqRetRefThisError(const Token *tok);
    void operatorEqToSelfError(const Token *tok);
    void checkConstError(const Token *tok, const std::string &classname, const std::string &funcname);
    void checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname);

    void getErrorMessages()
    {
        noConstructorError(0, "classname", false);
        uninitVarError(0, "classname", "varname");
        operatorEqVarError(0, "classname", "");
        unusedPrivateFunctionError(0, "classname", "funcname");
        memsetClassError(0, "memfunc");
        memsetStructError(0, "memfunc", "classname");
        operatorEqReturnError(0);
        //virtualDestructorError(0, "Base", "Derived");
        thisSubtractionError(0);
        operatorEqRetRefThisError(0);
        operatorEqToSelfError(0);
        checkConstError(0, "class", "function");
    }

    std::string name() const
    {
        return "Class";
    }

    std::string classInfo() const
    {
        return "Check the code for each class.\n"
               "* Missing constructors\n"
               "* Are all variables initialized by the constructors?\n"
               "* [[CheckMemset|Warn if memset, memcpy etc are used on a class]]\n"
               //"* If it's a base class, check that the destructor is virtual\n"
               "* Are there unused private functions\n"
               "* 'operator=' should return reference to self\n"
               "* 'operator=' should check for assignment to self\n"
               "* Constness for member functions\n";
    }

    // operatorEqRetRefThis helper function
    void checkReturnPtrThis(const SymbolDatabase::SpaceInfo *info, const SymbolDatabase::Func *func, const Token *tok, const Token *last);

    // operatorEqToSelf helper functions
    bool hasDeallocation(const Token *first, const Token *last);
    bool hasAssignSelf(const Token *first, const Token *last, const Token *rhs);


};
/// @}
//---------------------------------------------------------------------------
#endif


/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

class Token;

/// @addtogroup Checks
/// @{


/** @brief %Check classes. Uninitialized member variables, non-conforming operators, missing virtual destructor, etc */
class CheckClass : public Check
{
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckClass() : Check()
    { }

    /** @brief This constructor is used when running checks. */
    CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

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

        if (settings->_checkCodingStyle)
        {
            checkClass.constructors();
            checkClass.operatorEq();
            checkClass.privateFunctions();
            checkClass.operatorEqRetRefThis();
            if (settings->_showAll)
            {
                checkClass.thisSubtraction();
                checkClass.operatorEqToSelf();
            }
        }
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

    /** @brief 'operator=' should return something. */
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

    /** @brief Information about a member variable. Used when checking for uninitialized variables */
    class Var
    {
    public:
        Var(const std::string &name_, bool init_ = false, bool priv_ = false, bool mutable_ = false, Var *next_ = 0)
                : name(name_),
                init(init_),
                priv(priv_),
                isMutable(mutable_),
                next(next_)
        {
        }

        /** @brief name of variable */
        const std::string name;

        /** @brief has this variable been initialized? */
        bool        init;

        /** @brief is this variable declared in the private section? */
        bool        priv;

        /** @brief is this variable mutable? */
        bool        isMutable;

        /** @brief next Var item */
        Var *next;
    };

    /**
     * @brief parse a scope for a constructor or member function and set the "init" flags in the provided varlist
     * @param tok1 pointer to class declaration
     * @param ftok pointer to the function that should be checked
     * @param varlist variable list (the "init" flag will be set in these variables)
     * @param classname name of class
     * @param callstack the function doesn't look into recursive function calls.
     * @param isStruct if this is a struct instead of a class
     */
    void initializeVarList(const Token *tok1, const Token *ftok, Var *varlist, const std::string &classname, std::list<std::string> &callstack, bool isStruct);

    /** @brief initialize a variable in the varlist */
    void initVar(Var *varlist, const std::string &varname);

    /**
     * @brief get varlist from a class definition
     * @param tok1 pointer to class definition
     * @param withClasses if class variables should be extracted too.
     * @param isStruct is this a struct?
     */
    Var *getVarList(const Token *tok1, bool withClasses, bool isStruct);

    // Check constructors for a specified class
    void checkConstructors(const Token *tok1, const std::string &funcname, bool hasPrivateConstructor, bool isStruct);

    bool sameFunc(int nest, const Token *firstEnd, const Token *secondEnd);
    bool isMemberFunc(const Token *tok);
    bool isMemberVar(const std::string &classname, const Var *varlist, const Token *tok);
    bool checkConstFunc(const std::string &classname, const Var *varlist, const Token *tok);

    // Reporting errors..
    void noConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    void uninitVarError(const Token *tok, const std::string &classname, const std::string &varname, bool hasPrivateConstructor);
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
        uninitVarError(0, "classname", "varname", false);
        operatorEqVarError(0, "classname", "");
        unusedPrivateFunctionError(0, "classname", "funcname");
        memsetClassError(0, "memfunc");
        memsetStructError(0, "memfunc", "classname");
        operatorEqReturnError(0);
        virtualDestructorError(0, "Base", "Derived");
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
               "* If it's a base class, check that the destructor is virtual\n"
               "* Are there unused private functions\n"
               "* 'operator=' should return reference to self\n"
               "* 'operator=' should check for assignment to self\n"
               "* Constness for member functions\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


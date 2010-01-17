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


class CheckClass : public Check
{
public:
    /** This constructor is used when registering the CheckClass */
    CheckClass() : Check()
    { }

    /** This constructor is used when running checks.. */
    CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
            : Check(tokenizer, settings, errorLogger)
    { }

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckClass checkClass(tokenizer, settings, errorLogger);
        checkClass.noMemset();
    }

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
    }


    // Check that all class constructors are ok.
    void constructors();

    // Check that all private functions are called.
    void privateFunctions();

    // Check that the memsets are valid.
    // The 'memset' function can do dangerous things if used wrong.
    // Important: The checking doesn't work on simplified tokens list.
    void noMemset();

    // 'operator=' should return something..
    void operatorEq();    // Warning upon "void operator=(.."

    // 'operator=' should return reference to *this
    void operatorEqRetRefThis();    // Warning upon no "return *this;"

    // 'operator=' should check for assignment to self
    void operatorEqToSelf();    // Warning upon no check for assignment to self

    // The destructor in a base class should be virtual
    void virtualDestructor();

    void thisSubtraction();
private:
    class Var
    {
    public:
        Var(const char *name = 0, bool init = false, bool priv = false, Var *next = 0)
        {
            this->name = name;
            this->init = init;
            this->priv = priv;
            this->next = next;
        }

        const char *name;
        bool        init;
        bool        priv;
        Var *next;
    };

    void initializeVarList(const Token *tok1, const Token *ftok, Var *varlist, const char classname[], std::list<std::string> &callstack, bool isStruct);
    void initVar(Var *varlist, const char varname[]);
    Var *getVarList(const Token *tok1, bool withClasses, bool isStruct);

    // Check constructors for a specified class
    void checkConstructors(const Token *tok1, const char funcname[], bool hasPrivateConstructor, bool isStruct);

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
               "* The operator= should return a constant reference to itself\n"
               "* Are there unused private functions\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif


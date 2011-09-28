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
    CheckClass() : Check(myName()), symbolDatabase(NULL)
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
        checkClass.initializerList();

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
    void checkMemsetType(const Scope *start, const Token *tok, const Scope *type);

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

    /** @brief Check initializer list order */
    void initializerList();

private:
    /**
     * @brief Create symbol database. For performance reasons, only call
     * it if it's needed.
     */
    void createSymbolDatabase();

    const SymbolDatabase *symbolDatabase;

    // Reporting errors..
    void noConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    void uninitVarError(const Token *tok, const std::string &classname, const std::string &varname);
    void operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname);
    void unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname);
    void memsetError(const Token *tok, const std::string &memfunc, const std::string &classname, const std::string &type);
    void operatorEqReturnError(const Token *tok, const std::string &className);
    void virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived);
    void thisSubtractionError(const Token *tok);
    void operatorEqRetRefThisError(const Token *tok);
    void operatorEqToSelfError(const Token *tok);
    void checkConstError(const Token *tok, const std::string &classname, const std::string &funcname);
    void checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname);
    void initializerListError(const Token *tok1,const Token *tok2, const std::string & classname, const std::string &varname);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
    {
        CheckClass c(0, settings, errorLogger);
        c.noConstructorError(0, "classname", false);
        c.uninitVarError(0, "classname", "varname");
        c.operatorEqVarError(0, "classname", "");
        c.unusedPrivateFunctionError(0, "classname", "funcname");
        c.memsetError(0, "memfunc", "classname", "class");
        c.operatorEqReturnError(0, "class");
        //c.virtualDestructorError(0, "Base", "Derived");
        c.thisSubtractionError(0);
        c.operatorEqRetRefThisError(0);
        c.operatorEqToSelfError(0);
        c.checkConstError(0, "class", "function");
        c.initializerListError(0, 0, "class", "variable");
    }

    std::string myName() const
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
    void checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last);

    // operatorEqToSelf helper functions
    bool hasDeallocation(const Function *func);
    bool hasAssignSelf(const Function *func, const Token *rhs);

    // checkConst helper functions
    bool isMemberVar(const Scope *scope, const Token *tok);
    bool isConstMemberFunc(const Scope *scope, const Token *tok);
    bool checkConstFunc(const Scope *scope, const Token *tok);
    /** @brief check if this function is virtual in the base classes */
    bool isVirtualFunc(const Scope *scope, const Token *functionToken) const;

    // constructors helper function
    /** @brief Information about a member variable. Used when checking for uninitialized variables */
    struct Usage
    {
        Usage() : assign(false), init(false) { }

        /** @brief has this variable been assigned? */
        bool        assign;

        /** @brief has this variable been initialized? */
        bool        init;
    };

    bool isBaseClassFunc(const Token *tok, const Scope *scope);

    /**
     * @brief assign a variable in the varlist
     * @param varname name of variable to mark assigned
     * @param scope pointer to variable Scope
     * @param usage reference to usage vector
     */
    void assignVar(const std::string &varname, const Scope *scope, std::vector<Usage> &usage);

    /**
     * @brief initialize a variable in the varlist
     * @param varname name of variable to mark initialized
     * @param scope pointer to variable Scope
     * @param usage reference to usage vector
     */
    void initVar(const std::string &varname, const Scope *scope, std::vector<Usage> &usage);

    /**
     * @brief set all variables in list assigned
     * @param usage reference to usage vector
     */
    void assignAllVar(std::vector<Usage> &usage);

    /**
     * @brief set all variables in list not assigned and not initialized
     * @param usage reference to usage vector
     */
    void clearAllVar(std::vector<Usage> &usage);

    /**
     * @brief parse a scope for a constructor or member function and set the "init" flags in the provided varlist
     * @param func reference to the function that should be checked
     * @param callstack the function doesn't look into recursive function calls.
     * @param scope pointer to variable Scope
     * @param usage reference to usage vector
     */
    void initializeVarList(const Function &func, std::list<std::string> &callstack, const Scope *scope, std::vector<Usage> &usage);

    bool canNotCopy(const Scope *scope) const;
};
/// @}
//---------------------------------------------------------------------------
#endif


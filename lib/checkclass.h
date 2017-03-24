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
#ifndef checkclassH
#define checkclassH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

class Scope;
class Function;

/// @addtogroup Checks
/// @{


/** @brief %Check classes. Uninitialized member variables, non-conforming operators, missing virtual destructor, etc */
class CPPCHECKLIB CheckClass : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckClass() : Check(myName()), symbolDatabase(nullptr) {
    }

    /** @brief This constructor is used when running checks. */
    CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);

    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        if (tokenizer->isC())
            return;

        CheckClass checkClass(tokenizer, settings, errorLogger);

        // can't be a simplified check .. the 'sizeof' is used.
        checkClass.checkMemset();
    }

    /** @brief Run checks on the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        if (tokenizer->isC())
            return;

        CheckClass checkClass(tokenizer, settings, errorLogger);

        // Coding style checks
        checkClass.constructors();
        checkClass.operatorEq();
        checkClass.privateFunctions();
        checkClass.operatorEqRetRefThis();
        checkClass.thisSubtraction();
        checkClass.operatorEqToSelf();
        checkClass.initializerListOrder();
        checkClass.initializationListUsage();
        checkClass.checkSelfInitialization();

        checkClass.virtualDestructor();
        checkClass.checkConst();
        checkClass.copyconstructors();
        checkClass.checkPureVirtualFunctionCall();

        checkClass.checkDuplInheritedMembers();
        checkClass.checkExplicitConstructors();
        checkClass.checkCopyCtorAndEqOperator();
    }


    /** @brief %Check that all class constructors are ok */
    void constructors();

    /** @brief %Check that constructors with single parameter are explicit,
     *  if they has to be.*/
    void checkExplicitConstructors();

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
    void checkMemset();
    void checkMemsetType(const Scope *start, const Token *tok, const Scope *type, bool allocation, std::set<const Scope *> parsedTypes);

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
    void initializerListOrder();

    /** @brief Suggest using initialization list */
    void initializationListUsage();

    /** @brief Check for initialization of a member with itself */
    void checkSelfInitialization();

    void copyconstructors();

    /** @brief call of pure virtual function */
    void checkPureVirtualFunctionCall();

    /** @brief Check duplicated inherited members */
    void checkDuplInheritedMembers();

    /** @brief Check that copy constructor and operator defined together */
    void checkCopyCtorAndEqOperator();

private:
    const SymbolDatabase *symbolDatabase;

    // Reporting errors..
    void noConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    void noExplicitConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    //void copyConstructorMallocError(const Token *cctor, const Token *alloc, const std::string& var_name);
    void copyConstructorShallowCopyError(const Token *tok, const std::string& varname);
    void noCopyConstructorError(const Token *tok, const std::string &classname, bool isStruct);
    void uninitVarError(const Token *tok, const std::string &classname, const std::string &varname, bool inconclusive);
    void operatorEqVarError(const Token *tok, const std::string &classname, const std::string &varname, bool inconclusive);
    void unusedPrivateFunctionError(const Token *tok, const std::string &classname, const std::string &funcname);
    void memsetError(const Token *tok, const std::string &memfunc, const std::string &classname, const std::string &type);
    void memsetErrorReference(const Token *tok, const std::string &memfunc, const std::string &type);
    void memsetErrorFloat(const Token *tok, const std::string &type);
    void mallocOnClassError(const Token* tok, const std::string &memfunc, const Token* classTok, const std::string &classname);
    void mallocOnClassWarning(const Token* tok, const std::string &memfunc, const Token* classTok);
    void operatorEqReturnError(const Token *tok, const std::string &className);
    void virtualDestructorError(const Token *tok, const std::string &Base, const std::string &Derived, bool inconclusive);
    void thisSubtractionError(const Token *tok);
    void operatorEqRetRefThisError(const Token *tok);
    void operatorEqShouldBeLeftUnimplementedError(const Token *tok);
    void operatorEqMissingReturnStatementError(const Token *tok, bool error);
    void operatorEqToSelfError(const Token *tok);
    void checkConstError(const Token *tok, const std::string &classname, const std::string &funcname, bool suggestStatic);
    void checkConstError2(const Token *tok1, const Token *tok2, const std::string &classname, const std::string &funcname, bool suggestStatic);
    void initializerListError(const Token *tok1,const Token *tok2, const std::string & classname, const std::string &varname);
    void suggestInitializationList(const Token *tok, const std::string& varname);
    void selfInitializationError(const Token* tok, const std::string& varname);
    void callsPureVirtualFunctionError(const Function & scopeFunction, const std::list<const Token *> & tokStack, const std::string &purefuncname);
    void duplInheritedMembersError(const Token* tok1, const Token* tok2, const std::string &derivedname, const std::string &basename, const std::string &variablename, bool derivedIsStruct, bool baseIsStruct);
    void copyCtorAndEqOperatorError(const Token *tok, const std::string &classname, bool isStruct, bool hasCopyCtor);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckClass c(nullptr, settings, errorLogger);
        c.noConstructorError(nullptr, "classname", false);
        c.noExplicitConstructorError(nullptr, "classname", false);
        //c.copyConstructorMallocError(nullptr, 0, "var");
        c.copyConstructorShallowCopyError(nullptr, "var");
        c.noCopyConstructorError(nullptr, "class", false);
        c.uninitVarError(nullptr, "classname", "varname", false);
        c.operatorEqVarError(nullptr, "classname", emptyString, false);
        c.unusedPrivateFunctionError(nullptr, "classname", "funcname");
        c.memsetError(nullptr, "memfunc", "classname", "class");
        c.memsetErrorReference(nullptr, "memfunc", "class");
        c.memsetErrorFloat(nullptr, "class");
        c.mallocOnClassWarning(nullptr, "malloc", 0);
        c.mallocOnClassError(nullptr, "malloc", 0, "std::string");
        c.operatorEqReturnError(nullptr, "class");
        c.virtualDestructorError(nullptr, "Base", "Derived", false);
        c.thisSubtractionError(nullptr);
        c.operatorEqRetRefThisError(nullptr);
        c.operatorEqMissingReturnStatementError(nullptr, true);
        c.operatorEqShouldBeLeftUnimplementedError(nullptr);
        c.operatorEqToSelfError(nullptr);
        c.checkConstError(nullptr, "class", "function", false);
        c.checkConstError(nullptr, "class", "function", true);
        c.initializerListError(nullptr, 0, "class", "variable");
        c.suggestInitializationList(nullptr, "variable");
        c.selfInitializationError(nullptr, "var");
        c.duplInheritedMembersError(nullptr, 0, "class", "class", "variable", false, false);
        c.copyCtorAndEqOperatorError(nullptr, "class", false, false);
    }

    static std::string myName() {
        return "Class";
    }

    std::string classInfo() const {
        return "Check the code for each class.\n"
               "- Missing constructors and copy constructors\n"
               //"- Missing allocation of memory in copy constructor\n"
               "- Constructors which should be explicit\n"
               "- Are all variables initialized by the constructors?\n"
               "- Are all variables assigned by 'operator='?\n"
               "- Warn if memset, memcpy etc are used on a class\n"
               "- Warn if memory for classes is allocated with malloc()\n"
               "- If it's a base class, check that the destructor is virtual\n"
               "- Are there unused private functions?\n"
               "- 'operator=' should return reference to self\n"
               "- 'operator=' should check for assignment to self\n"
               "- Constness for member functions\n"
               "- Order of initializations\n"
               "- Suggest usage of initialization list\n"
               "- Initialization of a member with itself\n"
               "- Suspicious subtraction from 'this'\n"
               "- Call of pure virtual function in constructor/destructor\n"
               "- Duplicated inherited data members\n"
               "- If 'copy constructor' defined, 'operator=' also should be defined and vice versa\n";
    }

    // operatorEqRetRefThis helper functions
    void checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last);
    void checkReturnPtrThis(const Scope *scope, const Function *func, const Token *tok, const Token *last, std::set<const Function*>& analyzedFunctions);

    // operatorEqToSelf helper functions
    bool hasAllocation(const Function *func, const Scope* scope) const;
    static bool hasAssignSelf(const Function *func, const Token *rhs);

    // checkConst helper functions
    bool isMemberVar(const Scope *scope, const Token *tok) const;
    bool isMemberFunc(const Scope *scope, const Token *tok) const;
    bool isConstMemberFunc(const Scope *scope, const Token *tok) const;
    bool checkConstFunc(const Scope *scope, const Function *func, bool& memberAccessed) const;

    // constructors helper function
    /** @brief Information about a member variable. Used when checking for uninitialized variables */
    struct Usage {
        Usage() : assign(false), init(false) { }

        /** @brief has this variable been assigned? */
        bool assign;

        /** @brief has this variable been initialized? */
        bool init;
    };

    static bool isBaseClassFunc(const Token *tok, const Scope *scope);

    /**
     * @brief assign a variable in the varlist
     * @param varid id of variable to mark assigned
     * @param scope pointer to variable Scope
     * @param usage reference to usage vector
     */
    static void assignVar(unsigned int varid, const Scope *scope, std::vector<Usage> &usage);

    /**
     * @brief initialize a variable in the varlist
     * @param varid id of variable to mark initialized
     * @param scope pointer to variable Scope
     * @param usage reference to usage vector
     */
    static void initVar(unsigned int varid, const Scope *scope, std::vector<Usage> &usage);

    /**
     * @brief set all variables in list assigned
     * @param usage reference to usage vector
     */
    static void assignAllVar(std::vector<Usage> &usage);

    /**
     * @brief set all variables in list not assigned and not initialized
     * @param usage reference to usage vector
     */
    static void clearAllVar(std::vector<Usage> &usage);

    /**
     * @brief parse a scope for a constructor or member function and set the "init" flags in the provided varlist
     * @param func reference to the function that should be checked
     * @param callstack the function doesn't look into recursive function calls.
     * @param scope pointer to variable Scope
     * @param usage reference to usage vector
     */
    void initializeVarList(const Function &func, std::list<const Function *> &callstack, const Scope *scope, std::vector<Usage> &usage);

    /**
     * @brief gives a list of tokens where pure virtual functions are called directly or indirectly
     * @param function function to be checked
     * @param callsPureVirtualFunctionMap map of results for already checked functions
     * @return list of tokens where pure virtual functions are called
     */
    const std::list<const Token *> & callsPureVirtualFunction(
        const Function & function,
        std::map<const Function *, std::list<const Token *> > & callsPureVirtualFunctionMap);

    /**
     * @brief looks for the first pure virtual function call stack
     * @param callsPureVirtualFunctionMap map of results obtained from callsPureVirtualFunction
     * @param pureCall token where pure virtual function is called directly or indirectly
     * @param[in,out] pureFuncStack list to append the stack
     */
    void getFirstPureVirtualFunctionCallStack(
        std::map<const Function *, std::list<const Token *> > & callsPureVirtualFunctionMap,
        const Token & pureCall,
        std::list<const Token *> & pureFuncStack);

    static bool canNotCopy(const Scope *scope);

    static bool canNotMove(const Scope *scope);
};
/// @}
//---------------------------------------------------------------------------
#endif // checkclassH

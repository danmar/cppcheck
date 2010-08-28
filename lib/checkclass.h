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
#include <map>

class Token;

/// @addtogroup Checks
/// @{


/** @brief %Check classes. Uninitialized member variables, non-conforming operators, missing virtual destructor, etc */
class CheckClass : public Check
{
public:
    /** @brief This constructor is used when registering the CheckClass */
    CheckClass() : Check(), hasSymbolDatabase(false)
    { }

    /** @brief This constructor is used when running checks. */
    CheckClass(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);

    ~CheckClass();

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

    /**
     * @brief Access control. This needs to be public, otherwise it
     * doesn't work to compile with Borland C++
     */
    enum AccessControl { Public, Protected, Private };

private:
    /**
     * @brief Create symbol database. For performance reasons, only call
     * it if it's needed.
     */
    void createSymbolDatabase();

    /**
     * @brief Prevent creating symbol database more than once.
     *
     * Initialize this flag to false in the constructors. If this flag
     * is true the createSymbolDatabase should just bail out. If it is
     * false the createSymbolDatabase will set it to true and create
     * the symbol database.
     */
    bool hasSymbolDatabase;

    /** @brief Information about a member variable. Used when checking for uninitialized variables */
    class Var
    {
    public:
        Var(const Token *token_, bool init_ = false, AccessControl access_ = Public, bool mutable_ = false, bool static_ = false, bool class_ = false)
            : token(token_),
              init(init_),
              access(access_),
              isMutable(mutable_),
              isStatic(static_),
              isClass(class_)
        {
        }

        /** @brief variable token */
        const Token *token;

        /** @brief has this variable been initialized? */
        bool        init;

        /** @brief what section is this variable declared in? */
        AccessControl access;  // public/protected/private

        /** @brief is this variable mutable? */
        bool        isMutable;

        /** @brief is this variable static? */
        bool        isStatic;

        /** @brief is this variable a class (or unknown type)? */
        bool        isClass;
    };

    class Func
    {
    public:
        enum Type { Constructor, CopyConstructor, OperatorEqual, Destructor, Function };

        Func()
            : tokenDef(NULL),
              token(NULL),
              access(Public),
              hasBody(false),
              isInline(false),
              isConst(false),
              isVirtual(false),
              isPure(false),
              isStatic(false),
              isFriend(false),
              isExplicit(false),
              isOperator(false),
              type(Function)
        {
        }

        const Token *tokenDef; // function name token in class definition
        const Token *token;    // function name token in implementation
        AccessControl access;  // public/protected/private
        bool hasBody;          // has implementation
        bool isInline;         // implementation in class definition
        bool isConst;          // is const
        bool isVirtual;        // is virtual
        bool isPure;           // is pure virtual
        bool isStatic;         // is static
        bool isFriend;         // is friend
        bool isExplicit;       // is explicit
        bool isOperator;       // is operator
        Type type;             // constructor, destructor, ...
    };

    struct SpaceInfo;

    struct BaseInfo
    {
        AccessControl access;  // public/protected/private
        std::string name;
        SpaceInfo *spaceInfo;
    };

    struct FriendInfo
    {
        std::string name;
        SpaceInfo *spaceInfo;
    };

    class SpaceInfo
    {
    public:
        SpaceInfo(CheckClass *check_, const Token *classDef_, SpaceInfo *nestedIn_);

        CheckClass *check;
        bool isNamespace;
        std::string className;
        const Token *classDef;   // class/struct/namespace token
        const Token *classStart; // '{' token
        const Token *classEnd;   // '}' token
        std::list<Func> functionList;
        std::list<Var> varlist;
        std::vector<BaseInfo> derivedFrom;
        std::list<FriendInfo> friendList;
        SpaceInfo *nestedIn;
        std::list<SpaceInfo *> nestedList;
        AccessControl access;
        unsigned int numConstructors;

        /**
         * @brief initialize a variable in the varlist
         * @param varname name of variable to mark initialized
         */
        void initVar(const std::string &varname);

        /**
         * @brief mark all variables in list
         * @param value state to mark all variables
         */
        void markAllVar(bool value);

        /** @brief initialize varlist */
        void getVarList();

        /**
         * @brief parse a scope for a constructor or member function and set the "init" flags in the provided varlist
         * @param func reference to the function that should be checked
         * @param callstack the function doesn't look into recursive function calls.
         */
        void initializeVarList(const Func &func, std::list<std::string> &callstack);

        const Func *getDestructor() const
        {
            std::list<Func>::const_iterator it;
            for (it = functionList.begin(); it != functionList.end(); ++it)
            {
                if (it->type == Func::Destructor)
                    return &*it;
            }
            return 0;
        }
    };

    /** @brief Information about all namespaces/classes/structrues */
    std::multimap<std::string, SpaceInfo *> spaceInfoMMap;

    bool argsMatch(const Token *first, const Token *second, const std::string &path, unsigned int depth) const;

    bool isMemberVar(const SpaceInfo *info, const Token *tok);
    bool checkConstFunc(const SpaceInfo *info, const Token *tok);

    const Token *initBaseInfo(SpaceInfo *info, const Token *tok);

    /** @brief check if this function is virtual in the base classes */
    bool isVirtual(const SpaceInfo *info, const Token *functionToken) const;

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


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
#ifndef SymbolDatabaseH
#define SymbolDatabaseH
//---------------------------------------------------------------------------

#include <string>
#include <list>
#include <vector>

class Token;
class Tokenizer;
class Settings;
class ErrorLogger;

class SymbolDatabase
{
public:
    /**
     * @brief Access control.
     */
    enum AccessControl { Public, Protected, Private };

    SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    ~SymbolDatabase();

    class SpaceInfo;

    /** @brief Information about a member variable. Used when checking for uninitialized variables */
    class Var
    {
    public:
        Var(const Token *token_, unsigned int index_, AccessControl access_, bool mutable_, bool static_, bool const_, bool class_, const SpaceInfo *type_)
            : token(token_),
              index(index_),
              assign(false),
              init(false),
              access(access_),
              isMutable(mutable_),
              isStatic(static_),
              isConst(const_),
              isClass(class_),
              type(type_)
        {
        }

        /** @brief variable token */
        const Token *token;

        /** @brief order declared */
        unsigned int index;

        /** @brief has this variable been assigned? */
        bool        assign;

        /** @brief has this variable been initialized? */
        bool        init;

        /** @brief what section is this variable declared in? */
        AccessControl access;  // public/protected/private

        /** @brief is this variable mutable? */
        bool        isMutable;

        /** @brief is this variable static? */
        bool        isStatic;

        /** @brief is this variable const? */
        bool        isConst;

        /** @brief is this variable a class (or unknown type)? */
        bool        isClass;

        /** @brief pointer to user defined type info (for known types) */
        const SpaceInfo   *type;
    };

    class Func
    {
    public:
        enum Type { Constructor, CopyConstructor, OperatorEqual, Destructor, Function };

        Func()
            : tokenDef(NULL),
              argDef(NULL),
              token(NULL),
              arg(NULL),
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
              retFuncPtr(false),
              type(Function)
        {
        }

        const Token *tokenDef; // function name token in class definition
        const Token *argDef;   // function argument start '(' in class definition
        const Token *token;    // function name token in implementation
        const Token *arg;      // function argument start '('
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
        bool retFuncPtr;       // returns function pointer
        Type type;             // constructor, destructor, ...
    };

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
        enum SpaceType { Global, Class, Struct, Union, Namespace, Function };
        enum NeedInitialization { Unknown, True, False };

        SpaceInfo(SymbolDatabase *check_, const Token *classDef_, SpaceInfo *nestedIn_);

        SymbolDatabase *check;
        SpaceType type;
        std::string className;
        const Token *classDef;   // class/struct/union/namespace token
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
        NeedInitialization needInitialization;
        SpaceInfo * functionOf; // class/struct this function belongs to

        bool isClassOrStruct() const
        {
            return (type == Class || type == Struct);
        }

        /**
         * @brief find if name is in nested list
         * @param name name of nested space
         */
        SpaceInfo * findInNestedList(const std::string & name);

        /**
         * @brief assign a variable in the varlist
         * @param varname name of variable to mark assigned
         */
        void assignVar(const std::string &varname);

        /**
         * @brief initialize a variable in the varlist
         * @param varname name of variable to mark initialized
         */
        void initVar(const std::string &varname);

        void addVar(const Token *token_, AccessControl access_, bool mutable_, bool static_, bool const_, bool class_, const SpaceInfo *type_)
        {
            varlist.push_back(Var(token_, varlist.size(), access_, mutable_, static_, const_, class_, type_));
        }

        /**
         * @brief set all variables in list assigned
         */
        void assignAllVar();

        /**
         * @brief set all variables in list not assigned and not initialized
         */
        void clearAllVar();

        /** @brief initialize varlist */
        void getVarList();

        /**
         * @brief parse a scope for a constructor or member function and set the "init" flags in the provided varlist
         * @param func reference to the function that should be checked
         * @param callstack the function doesn't look into recursive function calls.
         */
        void initializeVarList(const Func &func, std::list<std::string> &callstack);

        const Func *getDestructor() const;

        /**
         * @brief get the number of nested spaces that are not functions
         *
         * This returns the number of user defined types (class, struct, union)
         * that are defined in this user defined type or namespace.
         */
        unsigned int getNestedNonFunctions() const;

        bool isBaseClassFunc(const Token *tok);
    };

    bool isMemberVar(const SpaceInfo *info, const Token *tok);
    bool isConstMemberFunc(const SpaceInfo *info, const Token *tok);
    bool checkConstFunc(const SpaceInfo *info, const Token *tok);

    const Token *initBaseInfo(SpaceInfo *info, const Token *tok);

    /** @brief check if this function is virtual in the base classes */
    bool isVirtualFunc(const SymbolDatabase::SpaceInfo *info, const Token *functionToken) const;

    /** @brief Information about all namespaces/classes/structrues */
    std::list<SpaceInfo *> spaceInfoList;

private:
    void addFunction(SpaceInfo **info, const Token **tok, const Token *argStart);
    void addNewFunction(SpaceInfo **info, const Token **tok);

    bool isFunction(const Token *tok, const Token **funcStart, const Token **argStart) const;
    bool argsMatch(const SpaceInfo *info, const Token *first, const Token *second, const std::string &path, unsigned int depth) const;

    const SpaceInfo *findVarType(const SpaceInfo *start, const Token *type) const;

    const Tokenizer *_tokenizer;
    const Settings *_settings;
    ErrorLogger *_errorLogger;
};

#endif


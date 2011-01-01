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
#ifndef SymbolDatabaseH
#define SymbolDatabaseH
//---------------------------------------------------------------------------

#include <string>
#include <list>
#include <vector>
#include <set>

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

    /** @brief Information about a member variable. */
    class Var
    {
    public:
        Var(const Token *token_, std::size_t index_, AccessControl access_, bool mutable_, bool static_, bool const_, bool class_, const SpaceInfo *type_)
            : token(token_),
              index(index_),
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
        std::size_t index;

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

        unsigned int argCount() const;
        unsigned int initializedArgCount() const;

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

        void addVar(const Token *token_, AccessControl access_, bool mutable_, bool static_, bool const_, bool class_, const SpaceInfo *type_)
        {
            varlist.push_back(Var(token_, varlist.size(), access_, mutable_, static_, const_, class_, type_));
        }

        /** @brief initialize varlist */
        void getVarList();

        const Func *getDestructor() const;

        /**
         * @brief get the number of nested spaces that are not functions
         *
         * This returns the number of user defined types (class, struct, union)
         * that are defined in this user defined type or namespace.
         */
        unsigned int getNestedNonFunctions() const;

        bool hasDefaultConstructor() const;

    private:
        /**
         * @brief helper function for getVarList()
         * @param tok pointer to token to check
         * @param vartok populated with pointer to the variable token, if found
         * @param typetok populated with pointer to the type token, if found
         * @return true if tok points to a variable declaration, false otherwise
         */
        bool isVariableDeclaration(const Token* tok, const Token*& vartok, const Token*& typetok) const;
        bool isSimpleVariable(const Token* tok) const;
        bool isArrayVariable(const Token* tok) const;
        bool findClosingBracket(const Token* tok, const Token*& close) const;
    };

    /** @brief Information about all namespaces/classes/structrues */
    std::list<SpaceInfo *> spaceInfoList;

    /**
     * @brief find a variable type if it's a user defined type
     * @param start scope to start looking in
     * @param type token containing variable type
     * @return pointer to type if found or NULL if not found
     */
    const SpaceInfo *findVarType(const SpaceInfo *start, const Token *type) const;

    bool argsMatch(const SpaceInfo *info, const Token *first, const Token *second, const std::string &path, unsigned int depth) const;

    bool isClassOrStruct(const std::string &type) const
    {
        return bool(classAndStructTypes.find(type) != classAndStructTypes.end());
    }

private:

    // Needed by Borland C++:
    friend class SpaceInfo;

    void addFunction(SpaceInfo **info, const Token **tok, const Token *argStart);
    void addNewFunction(SpaceInfo **info, const Token **tok);
    const Token *initBaseInfo(SpaceInfo *info, const Token *tok);
    bool isFunction(const Token *tok, const Token **funcStart, const Token **argStart) const;

    /** class/struct types */
    std::set<std::string> classAndStructTypes;

    const Tokenizer *_tokenizer;
    const Settings *_settings;
    ErrorLogger *_errorLogger;
};

#endif


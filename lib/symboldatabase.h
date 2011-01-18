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

#include "token.h"

class Tokenizer;
class Settings;
class ErrorLogger;

class Scope;
class SymbolDatabase;

/**
 * @brief Access control enumerations.
 */
enum AccessControl { Public, Protected, Private };

/** @brief Information about a member variable. */
class Variable
{
    /** @brief flags mask used to access specific bit. */
    enum
    {
        fIsMutable = (1 << 0), /** @brief mutable variable */
        fIsStatic  = (1 << 1), /** @brief static variable */
        fIsConst   = (1 << 2), /** @brief const variable */
        fIsClass   = (1 << 3)  /** @brief user defined type */
    };

    /**
     * Get specified flag state.
     * @param flag_ flag to get state of
     * @return true if flag set or false in flag not set
     */
    bool getFlag(int flag_) const
    {
        return bool(_flags & flag_);
    }

    /**
     * Set specified flag state.
     * @param flag_ flag to set state
     * @param state_ new state of flag
     */
    void setFlag(int flag_, bool state_)
    {
        _flags = state_ ? _flags | flag_ : _flags & ~flag_;
    }

public:
    Variable(const Token *name_, std::size_t index_, AccessControl access_,
             bool mutable_, bool static_, bool const_, bool class_,
             const Scope *type_)
        : _name(name_),
          _index(index_),
          _access(access_),
          _flags(0),
          _type(type_)
    {
        setFlag(fIsMutable, mutable_);
        setFlag(fIsStatic, static_);
        setFlag(fIsConst, const_);
        setFlag(fIsClass, class_);
    }

    /**
     * Get name token.
     * @return name token
     */
    const Token *nameToken() const
    {
        return _name;
    }

    /**
     * Get name string.
     * @return name string
     */
    const std::string &name() const
    {
        return _name->str();
    }

    /**
     * Get variable ID.
     * @return variable ID
     */
    unsigned int varId() const
    {
        return _name->varId();
    }

    /**
     * Get index of variable in declared order.
     * @return varaible index
     */
    std::size_t index() const
    {
        return _index;
    }

    /**
     * Is variable public.
     * @return true if public, false if not
     */
    bool isPublic() const
    {
        return _access == Public;
    }

    /**
     * Is variable protected.
     * @return true if protected, false if not
     */
    bool isProtected() const
    {
        return _access == Protected;
    }

    /**
     * Is variable private.
     * @return true if private, false if not
     */
    bool isPrivate() const
    {
        return _access == Private;
    }

    /**
     * Is variable mutable.
     * @return true if mutable, false if not
     */
    bool isMutable() const
    {
        return getFlag(fIsMutable);
    }

    /**
     * Is variable static.
     * @return true if static, false if not
     */
    bool isStatic() const
    {
        return getFlag(fIsStatic);
    }

    /**
     * Is variable const.
     * @return true if const, false if not
     */
    bool isConst() const
    {
        return getFlag(fIsConst);
    }

    /**
     * Is variable a user defined (or unknown) type.
     * @return true if user defined type, false if not
     */
    bool isClass() const
    {
        return getFlag(fIsClass);
    }

    /**
     * Get Scope pointer of known type.
     * @return pointer to type if known, NULL if not known
     */
    const Scope *type() const
    {
        return _type;
    }

private:
    /** @brief variable name token */
    const Token *_name;

    /** @brief order declared */
    std::size_t _index;

    /** @brief what section is this variable declared in? */
    AccessControl _access;  // public/protected/private

    /** @brief flags */
    int _flags;

    /** @brief pointer to user defined type info (for known types) */
    const Scope *_type;
};

class Function
{
public:
    enum Type { eConstructor, eCopyConstructor, eOperatorEqual, eDestructor, eFunction };

    Function()
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
          type(eFunction)
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

class Scope
{
    // let tests access private function for testing
    friend class TestSymbolDatabase;

public:
    struct BaseInfo
    {
        AccessControl access;  // public/protected/private
        std::string name;
        Scope *spaceInfo;
    };

    struct FriendInfo
    {
        std::string name;
        Scope *spaceInfo;
    };

    enum SpaceType { eGlobal, eClass, eStruct, eUnion, eNamespace, eFunction };
    enum NeedInitialization { Unknown, True, False };

    Scope(SymbolDatabase *check_, const Token *classDef_, Scope *nestedIn_);

    SymbolDatabase *check;
    SpaceType type;
    std::string className;
    const Token *classDef;   // class/struct/union/namespace token
    const Token *classStart; // '{' token
    const Token *classEnd;   // '}' token
    std::list<Function> functionList;
    std::list<Variable> varlist;
    std::vector<BaseInfo> derivedFrom;
    std::list<FriendInfo> friendList;
    Scope *nestedIn;
    std::list<Scope *> nestedList;
    AccessControl access;
    unsigned int numConstructors;
    NeedInitialization needInitialization;
    Scope * functionOf; // class/struct this function belongs to

    bool isClassOrStruct() const
    {
        return (type == eClass || type == eStruct);
    }

    /**
     * @brief find if name is in nested list
     * @param name name of nested space
     */
    Scope * findInNestedList(const std::string & name);

    void addVariable(const Token *token_, AccessControl access_, bool mutable_, bool static_, bool const_, bool class_, const Scope *type_)
    {
        varlist.push_back(Variable(token_, varlist.size(), access_, mutable_, static_, const_, class_, type_));
    }

    /** @brief initialize varlist */
    void getVariableList();

    const Function *getDestructor() const;

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
     * @brief helper function for getVariableList()
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

class SymbolDatabase
{
public:
    SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    ~SymbolDatabase();

    /** @brief Information about all namespaces/classes/structrues */
    std::list<Scope *> spaceInfoList;

    /**
     * @brief find a variable type if it's a user defined type
     * @param start scope to start looking in
     * @param type token containing variable type
     * @return pointer to type if found or NULL if not found
     */
    const Scope *findVariableType(const Scope *start, const Token *type) const;

    bool argsMatch(const Scope *info, const Token *first, const Token *second, const std::string &path, unsigned int depth) const;

    bool isClassOrStruct(const std::string &type) const
    {
        return bool(classAndStructTypes.find(type) != classAndStructTypes.end());
    }

private:

    // Needed by Borland C++:
    friend class Scope;

    void addFunction(Scope **info, const Token **tok, const Token *argStart);
    void addNewFunction(Scope **info, const Token **tok);
    const Token *initBaseInfo(Scope *info, const Token *tok);
    bool isFunction(const Token *tok, const Token **funcStart, const Token **argStart) const;

    /** class/struct types */
    std::set<std::string> classAndStructTypes;

    const Tokenizer *_tokenizer;
    const Settings *_settings;
    ErrorLogger *_errorLogger;
};

#endif


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
#include "mathlib.h"

class Tokenizer;
class Settings;
class ErrorLogger;

class Scope;
class SymbolDatabase;

/**
 * @brief Access control enumerations.
 */
enum AccessControl { Public, Protected, Private, Global, Namespace, Argument, Local };

/**
 * @brief Array dimension information.
 */
struct Dimension
{
    Dimension() : start(NULL), end(NULL), num(0) { }

    const Token *start;  // size start token
    const Token *end;    // size end token
    MathLib::bigint num; // dimension length when size is a number, 0 if not known
};

/** @brief Information about a member variable. */
class Variable
{
    /** @brief flags mask used to access specific bit. */
    enum
    {
        fIsMutable  = (1 << 0), /** @brief mutable variable */
        fIsStatic   = (1 << 1), /** @brief static variable */
        fIsConst    = (1 << 2), /** @brief const variable */
        fIsClass    = (1 << 3), /** @brief user defined type */
        fIsArray    = (1 << 4), /** @brief array variable */
        fHasDefault = (1 << 5)  /** @brief function argument with default value */
    };

    /**
     * Get specified flag state.
     * @param flag_ flag to get state of
     * @return true if flag set or false in flag not set
     */
    bool getFlag(int flag_) const
    {
        return bool((_flags & flag_) != 0);
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
    Variable(const Token *name_, const Token *start_, const Token *end_,
             std::size_t index_, AccessControl access_, bool mutable_,
             bool static_, bool const_, bool class_, const Scope *type_,
             const Scope *scope_, bool array_, bool default_,
             const std::vector<Dimension> &dimensions_)
        : _name(name_),
          _start(start_),
          _end(end_),
          _index(index_),
          _access(access_),
          _flags(0),
          _type(type_),
          _scope(scope_)
    {
        setFlag(fIsMutable, mutable_);
        setFlag(fIsStatic, static_);
        setFlag(fIsConst, const_);
        setFlag(fIsClass, class_);
        setFlag(fIsArray, array_);
        setFlag(fHasDefault, default_);
        _dimensions = dimensions_;
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
     * Get type start token.
     * @return type start token
     */
    const Token *typeStartToken() const
    {
        return _start;
    }

    /**
     * Get type end token.
     * @return type end token
     */
    const Token *typeEndToken() const
    {
        return _end;
    }

    /**
     * Get name string.
     * @return name string
     */
    const std::string &name() const
    {
        static const std::string noname;

        // name may not exist for function arguments
        if (_name)
            return _name->str();

        return noname;
    }

    /**
     * Get variable ID.
     * @return variable ID
     */
    unsigned int varId() const
    {
        // name may not exist for function arguments
        if (_name)
            return _name->varId();

        return 0;
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
     * Is variable global.
     * @return true if global, false if not
     */
    bool isGlobal() const
    {
        return _access == Global;
    }

    /**
     * Is variable in a namespace.
     * @return true if in a namespace, false if not
     */
    bool isNamespace() const
    {
        return _access == Namespace;
    }

    /**
     * Is variable a function argument.
     * @return true if a function argument, false if not
     */
    bool isArgument() const
    {
        return _access == Argument;
    }

    /**
     * Is variable local.
     * @return true if local, false if not
     */
    bool isLocal() const
    {
        return _access == Local;
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
     * Is variable an array.
     * @return true if array, false if not
     */
    bool isArray() const
    {
        return getFlag(fIsArray);
    }

    /**
     * Does variable have a default value.
     * @return true if has a default falue, false if not
     */
    bool hasDefault() const
    {
        return getFlag(fHasDefault);
    }

    /**
     * Get Scope pointer of known type.
     * @return pointer to type if known, NULL if not known
     */
    const Scope *type() const
    {
        return _type;
    }

    /**
     * Get Scope pointer of enclosing scope.
     * @return pointer to enclosing scope
     */
    const Scope *scope() const
    {
        return _scope;
    }

    /**
     * Get array dimensions.
     * @return array dimensions vector
     */
    const std::vector<Dimension> &dimensions() const
    {
        return _dimensions;
    }

    /**
     * Get array dimension length.
     * @return length of dimension
     */
    MathLib::bigint dimension(size_t index_) const
    {
        return _dimensions[index_].num;
    }

private:
    /** @brief variable name token */
    const Token *_name;

    /** @brief variable type start token */
    const Token *_start;

    /** @brief variable type end token */
    const Token *_end;

    /** @brief order declared */
    std::size_t _index;

    /** @brief what section is this variable declared in? */
    AccessControl _access;  // public/protected/private

    /** @brief flags */
    int _flags;

    /** @brief pointer to user defined type info (for known types) */
    const Scope *_type;

    /** @brief pointer to scope this variable is in */
    const Scope *_scope;

    /** @brief array dimensions */
    std::vector<Dimension> _dimensions;
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
          start(NULL),
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
          type(eFunction),
          functionScope(NULL)
    {
    }

    unsigned int argCount() const
    {
        return argumentList.size();
    }
    unsigned int initializedArgCount() const;
    void addArguments(const SymbolDatabase *symbolDatabase, const Function *func, const Scope *scope);

    const Token *tokenDef; // function name token in class definition
    const Token *argDef;   // function argument start '(' in class definition
    const Token *token;    // function name token in implementation
    const Token *arg;      // function argument start '('
    const Token *start;    // function start '{'
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
    Scope *functionScope;  // scope of function body
    std::list<Variable> argumentList; // argument list
};

class Scope
{
    // let tests access private function for testing
    friend class TestSymbolDatabase;

public:
    struct BaseInfo
    {
        AccessControl access;  // public/protected/private
        bool isVirtual;
        std::string name;
        Scope *scope;
    };

    struct FriendInfo
    {
        std::string name;
        Scope *scope;
    };

    enum ScopeType { eGlobal, eClass, eStruct, eUnion, eNamespace, eFunction, eIf, eElse, eElseIf, eFor, eWhile, eDo, eSwitch, eUnconditional };
    enum NeedInitialization { Unknown, True, False };

    Scope(SymbolDatabase *check_, const Token *classDef_, Scope *nestedIn_);
    Scope(SymbolDatabase *check_, const Token *classDef_, Scope *nestedIn_, ScopeType type_, const Token *start_);

    SymbolDatabase *check;
    ScopeType type;
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
    std::list<const Token *> usingList;

    // function specific fields
    Scope *functionOf; // scope this function belongs to
    Function *function; // function info for this function

    bool isClassOrStruct() const
    {
        return (type == eClass || type == eStruct);
    }

    bool isLocal() const
    {
        return (type == eIf || type == eElse || type == eElseIf ||
                type == eFor || type == eWhile || type == eDo ||
                type == eSwitch || type == eUnconditional);
    }

    /**
     * @brief find if name is in nested list
     * @param name name of nested scope
     */
    Scope * findInNestedList(const std::string & name);

    /**
     * @brief find if name is in nested list
     * @param name name of nested scope
     */
    Scope * findInNestedListRecursive(const std::string & name);

    const Scope * findQualifiedScope(const std::string & name) const;

    void addVariable(const Token *token_, const Token *start_,
                     const Token *end_, AccessControl access_, bool mutable_,
                     bool static_, bool const_, bool class_, const Scope *type_,
                     const Scope *scope_, bool array_,
                     const std::vector<Dimension> &dimensions_)
    {
        varlist.push_back(Variable(token_, start_, end_, varlist.size(),
                                   access_, mutable_, static_, const_, class_,
                                   type_, scope_, array_, false, dimensions_));
    }

    /** @brief initialize varlist */
    void getVariableList();

    const Function *getDestructor() const;

    /**
     * @brief get the number of nested scopes that are not functions
     *
     * This returns the number of user defined types (class, struct, union)
     * that are defined in this user defined type or namespace.
     */
    unsigned int getNestedNonFunctions() const;

    bool hasDefaultConstructor() const;

    AccessControl defaultAccess() const;

    /**
     * @brief check if statement is variable declaration and add it if it is
     * @param tok pointer to start of statement
     * @param varaccess access control of statement
     * @return pointer to last token
     */
    const Token *checkVariable(const Token *tok, AccessControl varaccess);

    /**
     * @brief get variable from name
     * @param varname name of variable
     * @return pointer to variable
     */
    const Variable *getVariable(const std::string &varname) const;

private:
    /**
     * @brief helper function for getVariableList()
     * @param tok pointer to token to check
     * @param vartok populated with pointer to the variable token, if found
     * @param typetok populated with pointer to the type token, if found
     * @param isArray reference to variable to set if array is found
     * @return true if tok points to a variable declaration, false otherwise
     */
    bool isVariableDeclaration(const Token* tok, const Token*& vartok, const Token*& typetok, bool &isArray) const;
    bool isSimpleVariable(const Token* tok) const;
    bool isArrayVariable(const Token* tok) const;
    bool findClosingBracket(const Token* tok, const Token*& close) const;
};

class SymbolDatabase
{
public:
    SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);

    /** @brief Information about all namespaces/classes/structrues */
    std::list<Scope> scopeList;

    /**
     * @brief find a variable type if it's a user defined type
     * @param start scope to start looking in
     * @param type token containing variable type
     * @return pointer to type if found or NULL if not found
     */
    const Scope *findVariableType(const Scope *start, const Token *type) const;

    const Scope *findFunctionScopeByToken(const Token *tok) const;

    const Function *findFunctionByToken(const Token *tok) const;

    bool argsMatch(const Scope *info, const Token *first, const Token *second, const std::string &path, unsigned int depth) const;

    bool isClassOrStruct(const std::string &type) const
    {
        return bool(classAndStructTypes.find(type) != classAndStructTypes.end());
    }

    const Variable *getVariableFromVarId(unsigned int varId) const
    {
        return _variableList[varId];
    }

    /**
     * @brief output a debug message
     */
    void debugMessage(const Token *tok, const std::string &msg) const;

    /**
     * @brief parse and save array dimension information
     * @param dimensions array dimensions vector
     * @param tokenizer tokenizer pointer
     * @return true if array, false if not
     */
    bool arrayDimensions(std::vector<Dimension> &dimensions, const Token *tok) const;

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

    /** variable symbol table */
    std::vector<const Variable *> _variableList;
};

#endif


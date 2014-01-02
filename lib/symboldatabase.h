/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#ifndef symboldatabaseH
#define symboldatabaseH
//---------------------------------------------------------------------------

#include <string>
#include <list>
#include <vector>
#include <set>

#include "config.h"
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
enum AccessControl { Public, Protected, Private, Global, Namespace, Argument, Local, Throw };

/**
 * @brief Array dimension information.
 */
struct Dimension {
    Dimension() : start(NULL), end(NULL), num(0), known(true) { }

    const Token *start;  // size start token
    const Token *end;    // size end token
    MathLib::bigint num; // (assumed) dimension length when size is a number, 0 if not known
    bool known;          // Known size
};

/** @brief Information about a class type. */
class CPPCHECKLIB Type {
public:
    const Token* classDef; // Points to "class" token
    const Scope* classScope;
    const Scope* enclosingScope;
    enum NeedInitialization {
        Unknown, True, False
    } needInitialization;

    struct BaseInfo {
        std::string name;
        const Type* type;
        const Token* nameTok;
        AccessControl access;  // public/protected/private
        bool isVirtual;
    };

    struct FriendInfo {
        const Token* nameStart;
        const Token* nameEnd;
        std::string name;
        const Type* type;
    };

    std::vector<BaseInfo> derivedFrom;
    std::list<FriendInfo> friendList;

    Type(const Token* classDef_ = 0, const Scope* classScope_ = 0, const Scope* enclosingScope_ = 0) :
        classDef(classDef_),
        classScope(classScope_),
        enclosingScope(enclosingScope_),
        needInitialization(Unknown) {
    }

    const std::string& name() const {
        static const std::string empty;
        return classDef->next()->isName() ? classDef->strAt(1) : empty;
    }

    const Token *initBaseInfo(const Token *tok, const Token *tok1);

    const Function* getFunction(const std::string& funcName) const;
};

/** @brief Information about a member variable. */
class CPPCHECKLIB Variable {
    /** @brief flags mask used to access specific bit. */
    enum {
        fIsMutable   = (1 << 0), /** @brief mutable variable */
        fIsStatic    = (1 << 1), /** @brief static variable */
        fIsConst     = (1 << 2), /** @brief const variable */
        fIsExtern    = (1 << 3), /** @brief extern variable */
        fIsClass     = (1 << 4), /** @brief user defined type */
        fIsArray     = (1 << 5), /** @brief array variable */
        fIsPointer   = (1 << 6), /** @brief pointer variable */
        fIsReference = (1 << 7), /** @brief reference variable */
        fIsRValueRef = (1 << 8), /** @brief rvalue reference variable */
        fHasDefault  = (1 << 9)  /** @brief function argument with default value */
    };

    /**
     * Get specified flag state.
     * @param flag_ flag to get state of
     * @return true if flag set or false in flag not set
     */
    bool getFlag(int flag_) const {
        return bool((_flags & flag_) != 0);
    }

    /**
     * Set specified flag state.
     * @param flag_ flag to set state
     * @param state_ new state of flag
     */
    void setFlag(int flag_, bool state_) {
        _flags = state_ ? _flags | flag_ : _flags & ~flag_;
    }

    /**
     * @brief parse and save array dimension information
     * @param dimensions array dimensions vector
     * @param tok the first '[' token of array declaration
     * @return true if array, false if not
     */
    static bool arrayDimensions(std::vector<Dimension> &dimensions, const Token *tok);

public:
    Variable(const Token *name_, const Token *start_, const Token *end_,
             std::size_t index_, AccessControl access_, const Type *type_,
             const Scope *scope_)
        : _name(name_),
          _start(start_),
          _end(end_),
          _index(index_),
          _access(access_),
          _flags(0),
          _type(type_),
          _scope(scope_) {
        evaluate();
    }

    /**
     * Get name token.
     * @return name token
     */
    const Token *nameToken() const {
        return _name;
    }

    /**
     * Get type start token.
     * The type start token doesn't account 'static' and 'const' qualifiers
     * E.g.:
     *     static const int * const p = ...;
     * type start token ^
     * @return type start token
     */
    const Token *typeStartToken() const {
        return _start;
    }

    /**
     * Get type end token.
     * The type end token doesn't account the forward 'const' qualifier
     * E.g.:
     *     static const int * const p = ...;
     *       type end token ^
     * @return type end token
     */
    const Token *typeEndToken() const {
        return _end;
    }

    /**
     * Get name string.
     * @return name string
     */
    const std::string &name() const {
        static const std::string noname;

        // name may not exist for function arguments
        if (_name)
            return _name->str();

        return noname;
    }

    /**
     * Get declaration ID (varId used for variable in its declaration).
     * @return declaration ID
     */
    unsigned int declarationId() const {
        // name may not exist for function arguments
        if (_name)
            return _name->varId();

        return 0;
    }

    /**
     * Get index of variable in declared order.
     * @return variable index
     */
    std::size_t index() const {
        return _index;
    }

    /**
     * Is variable public.
     * @return true if public, false if not
     */
    bool isPublic() const {
        return _access == Public;
    }

    /**
     * Is variable protected.
     * @return true if protected, false if not
     */
    bool isProtected() const {
        return _access == Protected;
    }

    /**
     * Is variable private.
     * @return true if private, false if not
     */
    bool isPrivate() const {
        return _access == Private;
    }

    /**
     * Is variable global.
     * @return true if global, false if not
     */
    bool isGlobal() const {
        return _access == Global;
    }

    /**
     * Is variable in a namespace.
     * @return true if in a namespace, false if not
     */
    bool isNamespace() const {
        return _access == Namespace;
    }

    /**
     * Is variable a function argument.
     * @return true if a function argument, false if not
     */
    bool isArgument() const {
        return _access == Argument;
    }

    /**
     * Is variable local.
     * @return true if local, false if not
     */
    bool isLocal() const {
        return (_access == Local) && !isExtern();
    }

    /**
     * Is variable mutable.
     * @return true if mutable, false if not
     */
    bool isMutable() const {
        return getFlag(fIsMutable);
    }

    /**
     * Is variable static.
     * @return true if static, false if not
     */
    bool isStatic() const {
        return getFlag(fIsStatic);
    }

    /**
     * Is variable extern.
     * @return true if extern, false if not
     */
    bool isExtern() const {
        return getFlag(fIsExtern);
    }

    /**
     * Is variable const.
     * @return true if const, false if not
     */
    bool isConst() const {
        return getFlag(fIsConst);
    }

    /**
     * Is variable a throw type.
     * @return true if throw type, false if not
     */
    bool isThrow() const {
        return _access == Throw;
    }

    /**
     * Is variable a user defined (or unknown) type.
     * @return true if user defined type, false if not
     */
    bool isClass() const {
        return getFlag(fIsClass);
    }

    /**
     * Is variable an array.
     * @return true if array, false if not
     */
    bool isArray() const {
        return getFlag(fIsArray);
    }

    /**
     * Is pointer variable.
     * @return true if pointer, false otherwise
     */
    bool isPointer() const {
        return getFlag(fIsPointer);
    }

    /**
     * Is array or pointer variable.
     * @return true if pointer or array, false otherwise
     */
    bool isArrayOrPointer() const {
        return getFlag(fIsArray) || getFlag(fIsPointer);
    }

    /**
     * Is reference variable.
     * @return true if reference, false otherwise
     */
    bool isReference() const {
        return getFlag(fIsReference);
    }

    /**
     * Is reference variable.
     * @return true if reference, false otherwise
     */
    bool isRValueReference() const {
        return getFlag(fIsRValueRef);
    }

    /**
     * Does variable have a default value.
     * @return true if has a default falue, false if not
     */
    bool hasDefault() const {
        return getFlag(fHasDefault);
    }

    /**
     * Get Type pointer of known type.
     * @return pointer to type if known, NULL if not known
     */
    const Type *type() const {
        return _type;
    }

    /**
     * Get Scope pointer of known type.
     * @return pointer to type scope if known, NULL if not known
     */
    const Scope *typeScope() const {
        return _type ? _type->classScope : 0;
    }

    /**
     * Get Scope pointer of enclosing scope.
     * @return pointer to enclosing scope
     */
    const Scope *scope() const {
        return _scope;
    }

    /**
     * Get array dimensions.
     * @return array dimensions vector
     */
    const std::vector<Dimension> &dimensions() const {
        return _dimensions;
    }

    /**
     * Get array dimension length.
     * @return length of dimension
     */
    MathLib::bigint dimension(std::size_t index_) const {
        return _dimensions[index_].num;
    }

    /**
     * Get array dimension known.
     * @return length of dimension known
     */
    bool dimensionKnown(std::size_t index_) const {
        return _dimensions[index_].known;
    }

private:
    // only symbol database can change the type
    friend class SymbolDatabase;

    /**
     * Set Type pointer to known type.
     * @param t type
     */
    void type(const Type * t) {
        _type = t;
    }

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
    const Type *_type;

    /** @brief pointer to scope this variable is in */
    const Scope *_scope;

    /** @brief array dimensions */
    std::vector<Dimension> _dimensions;

    /** @brief fill in information, depending on Tokens given at instantiation */
    void evaluate();
};

class CPPCHECKLIB Function {
public:
    enum Type { eConstructor, eCopyConstructor, eMoveConstructor, eOperatorEqual, eDestructor, eFunction };

    Function()
        : tokenDef(NULL),
          argDef(NULL),
          token(NULL),
          arg(NULL),
          retDef(NULL),
          retType(NULL),
          functionScope(NULL),
          nestedIn(NULL),
          initArgCount(0),
          type(eFunction),
          access(Public),
          hasBody(false),
          isInline(false),
          isConst(false),
          isVirtual(false),
          isPure(false),
          isStatic(false),
          isFriend(false),
          isExplicit(false),
          isDefault(false),
          isDelete(false),
          isOperator(false),
          retFuncPtr(false) {
    }

    const std::string &name() const {
        return tokenDef->str();
    }

    std::size_t argCount() const {
        return argumentList.size();
    }
    std::size_t minArgCount() const {
        return argumentList.size() - initArgCount;
    }
    const Variable* getArgumentVar(std::size_t num) const;
    unsigned int initializedArgCount() const {
        return initArgCount;
    }
    void addArguments(const SymbolDatabase *symbolDatabase, const Scope *scope);
    /** @brief check if this function is virtual in the base classes */
    bool isImplicitlyVirtual(bool defaultVal = false) const;

    bool isConstructor() const {
        return type==eConstructor ||
               type==eCopyConstructor ||
               type==eMoveConstructor;
    }

    bool isDestructor() const {
        return type==eDestructor;
    }

    const Token *tokenDef; // function name token in class definition
    const Token *argDef;   // function argument start '(' in class definition
    const Token *token;    // function name token in implementation
    const Token *arg;      // function argument start '('
    const Token *retDef;   // function return type token
    const ::Type *retType; // function return type
    const Scope *functionScope; // scope of function body
    const Scope* nestedIn; // Scope the function is declared in
    std::list<Variable> argumentList; // argument list
    unsigned int initArgCount; // number of args with default values
    Type type;             // constructor, destructor, ...
    AccessControl access;  // public/protected/private
    bool hasBody;          // has implementation
    bool isInline;         // implementation in class definition
    bool isConst;          // is const
    bool isVirtual;        // is virtual
    bool isPure;           // is pure virtual
    bool isStatic;         // is static
    bool isFriend;         // is friend
    bool isExplicit;       // is explicit
    bool isDefault;        // is default
    bool isDelete;         // is delete
    bool isOperator;       // is operator
    bool retFuncPtr;       // returns function pointer

    static bool argsMatch(const Scope *info, const Token *first, const Token *second, const std::string &path, unsigned int depth);

private:
    bool isImplicitlyVirtual_rec(const ::Type* type, bool& safe) const;
};

class CPPCHECKLIB Scope {
    // let tests access private function for testing
    friend class TestSymbolDatabase;

public:
    struct UsingInfo {
        const Token *start;
        const Scope *scope;
    };

    enum ScopeType { eGlobal, eClass, eStruct, eUnion, eNamespace, eFunction, eIf, eElse, eElseIf, eFor, eWhile, eDo, eSwitch, eUnconditional, eTry, eCatch };

    Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_);
    Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_, ScopeType type_, const Token *start_);

    const SymbolDatabase *check;
    std::string className;
    const Token *classDef;   // class/struct/union/namespace token
    const Token *classStart; // '{' token
    const Token *classEnd;   // '}' token
    std::list<Function> functionList;
    std::list<Variable> varlist;
    const Scope *nestedIn;
    std::list<Scope *> nestedList;
    unsigned int numConstructors;
    unsigned int numCopyOrMoveConstructors;
    std::list<UsingInfo> usingList;
    ScopeType type;
    Type* definedType;
    std::list<Type*> definedTypes;

    // function specific fields
    const Scope *functionOf; // scope this function belongs to
    Function *function; // function info for this function

    bool isClassOrStruct() const {
        return (type == eClass || type == eStruct);
    }

    bool isExecutable() const {
        return type != eClass && type != eStruct && type != eUnion && type != eGlobal && type != eNamespace;
    }

    bool isLocal() const {
        return (type == eIf || type == eElse || type == eElseIf ||
                type == eFor || type == eWhile || type == eDo ||
                type == eSwitch || type == eUnconditional ||
                type == eTry || type == eCatch);
    }

    /**
     * @brief find a function
     * @param tok token of function call
     * @return pointer to function if found or NULL if not found
     */
    const Function *findFunction(const Token *tok) const;

    /**
     * @brief find if name is in nested list
     * @param name name of nested scope
     */
    Scope *findInNestedList(const std::string & name);

    const Scope *findRecordInNestedList(const std::string & name) const;
    Scope *findRecordInNestedList(const std::string & name) {
        return const_cast<Scope *>(static_cast<const Scope *>(this)->findRecordInNestedList(name));
    }

    const Type* findType(const std::string& name) const;
    Type* findType(const std::string& name) {
        return const_cast<Type*>(static_cast<const Scope *>(this)->findType(name));
    }

    /**
     * @brief find if name is in nested list
     * @param name name of nested scope
     */
    Scope *findInNestedListRecursive(const std::string & name);

    void addVariable(const Token *token_, const Token *start_,
                     const Token *end_, AccessControl access_, const Type *type_,
                     const Scope *scope_) {
        varlist.push_back(Variable(token_, start_, end_, varlist.size(),
                                   access_,
                                   type_, scope_));
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
     * @return true if tok points to a variable declaration, false otherwise
     */
    bool isVariableDeclaration(const Token* tok, const Token*& vartok, const Token*& typetok) const;
};

class CPPCHECKLIB SymbolDatabase {
public:
    SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);

    /** @brief Information about all namespaces/classes/structrues */
    std::list<Scope> scopeList;

    /** @brief Fast access to function scopes */
    std::vector<const Scope *> functionScopes;

    /** @brief Fast access to class and struct scopes */
    std::vector<const Scope *> classAndStructScopes;

    /** @brief Fast access to types */
    std::list<Type> typeList;

    /**
     * @brief find a variable type if it's a user defined type
     * @param start scope to start looking in
     * @param type token containing variable type
     * @return pointer to type if found or NULL if not found
     */
    const Type *findVariableType(const Scope *start, const Token *type) const;

    /**
     * @brief find a function
     * @param tok token of function call
     * @return pointer to function if found or NULL if not found
     */
    const Function *findFunction(const Token *tok) const;

    const Scope *findScopeByName(const std::string& name) const;

    const Type* findType(const Token *tok, const Scope *startScope) const;
    Type* findType(const Token *tok, Scope *startScope) const {
        return const_cast<Type*>(this->findType(tok, static_cast<const Scope *>(startScope)));
    }

    const Scope *findScope(const Token *tok, const Scope *startScope) const;
    Scope *findScope(const Token *tok, Scope *startScope) const {
        return const_cast<Scope *>(this->findScope(tok, static_cast<const Scope *>(startScope)));
    }

    bool isClassOrStruct(const std::string &type) const {
        for (std::list<Type>::const_iterator i = typeList.begin(); i != typeList.end(); ++i)
            if (i->name() == type)
                return true;
        return false;
    }

    const Variable *getVariableFromVarId(std::size_t varId) const {
        return _variableList[varId];
    }

    std::size_t getVariableListSize() const {
        return _variableList.size();
    }

    /**
     * @brief output a debug message
     */
    void debugMessage(const Token *tok, const std::string &msg) const;

    void printOut(const char * title = NULL) const;
    void printVariable(const Variable *var, const char *indent) const;

    bool isCPP() const;

private:

    // Needed by Borland C++:
    friend class Scope;

    void addClassFunction(Scope **info, const Token **tok, const Token *argStart);
    Function *addGlobalFunctionDecl(Scope*& scope, const Token* tok, const Token *argStart, const Token* funcStart);
    Function *addGlobalFunction(Scope*& scope, const Token*& tok, const Token *argStart, const Token* funcStart);
    void addNewFunction(Scope **info, const Token **tok);
    static bool isFunction(const Token *tok, const Scope* outerScope, const Token **funcStart, const Token **argStart);
    const Type *findTypeInNested(const Token *tok, const Scope *startScope) const;

    const Tokenizer *_tokenizer;
    const Settings *_settings;
    ErrorLogger *_errorLogger;

    /** variable symbol table */
    std::vector<const Variable *> _variableList;

    /** list for missing types */
    std::list<Type> _blankTypes;
};
//---------------------------------------------------------------------------
#endif // symboldatabaseH

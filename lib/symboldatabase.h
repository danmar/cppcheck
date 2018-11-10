/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "config.h"
#include "library.h"
#include "mathlib.h"
#include "token.h"

#include <cstddef>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class ErrorLogger;
class Function;
class Scope;
class Settings;
class SymbolDatabase;
class Tokenizer;
class ValueType;

/**
 * @brief Access control enumerations.
 */
enum AccessControl { Public, Protected, Private, Global, Namespace, Argument, Local, Throw };

/**
 * @brief Array dimension information.
 */
struct Dimension {
    Dimension() : start(nullptr), end(nullptr), num(0), known(true) { }

    const Token *start;  ///< size start token
    const Token *end;    ///< size end token
    MathLib::bigint num; ///< (assumed) dimension length when size is a number, 0 if not known
    bool known;          ///< Known size
};

/** @brief Information about a class type. */
class CPPCHECKLIB Type {
public:
    const Token* classDef;     ///< Points to "class" token
    const Scope* classScope;
    const Scope* enclosingScope;
    enum NeedInitialization {
        Unknown, True, False
    } needInitialization;

    class BaseInfo {
    public:
        BaseInfo() :
            type(nullptr), nameTok(nullptr), access(Public), isVirtual(false) {
        }

        std::string name;
        const Type* type;
        const Token* nameTok;
        AccessControl access;  // public/protected/private
        bool isVirtual;
        // allow ordering within containers
        bool operator<(const BaseInfo& rhs) const {
            return this->type < rhs.type;
        }
    };

    struct FriendInfo {
        FriendInfo() :
            nameStart(nullptr), nameEnd(nullptr), type(nullptr) {
        }

        const Token* nameStart;
        const Token* nameEnd;
        const Type* type;
    };

    std::vector<BaseInfo> derivedFrom;
    std::vector<FriendInfo> friendList;

    const Token * typeStart;
    const Token * typeEnd;

    Type(const Token* classDef_ = nullptr, const Scope* classScope_ = nullptr, const Scope* enclosingScope_ = nullptr) :
        classDef(classDef_),
        classScope(classScope_),
        enclosingScope(enclosingScope_),
        needInitialization(Unknown),
        typeStart(nullptr),
        typeEnd(nullptr) {
        if (classDef_ && classDef_->str() == "enum")
            needInitialization = True;
        else if (classDef_ && classDef_->str() == "using") {
            typeStart = classDef->tokAt(3);
            typeEnd = typeStart;
            while (typeEnd->next() && typeEnd->next()->str() != ";")
                typeEnd = typeEnd->next();
        }
    }

    const std::string& name() const;

    const std::string& type() const {
        return classDef ? classDef->str() : emptyString;
    }

    bool isClassType() const {
        return classDef && classDef->str() == "class";
    }

    bool isEnumType() const {
        return classDef && classDef->str() == "enum";
    }

    bool isTypeAlias() const {
        return classDef && classDef->str() == "using";
    }

    bool isStructType() const {
        return classDef && classDef->str() == "struct";
    }

    const Token *initBaseInfo(const Token *tok, const Token *tok1);

    const Function* getFunction(const std::string& funcName) const;

    /**
    * Check for circulare dependencies, i.e. loops within the class hierarchy
    * @param ancestors list of ancestors. For internal usage only, clients should not supply this argument.
    * @return true if there is a circular dependency
    */
    bool hasCircularDependencies(std::set<BaseInfo>* ancestors = nullptr) const;

    /**
    * Check for dependency
    * @param ancestor potential ancestor
    * @return true if there is a dependency
    */
    bool findDependency(const Type* ancestor) const;

    bool isDerivedFrom(const std::string & ancestor) const;
};

class CPPCHECKLIB Enumerator {
public:
    explicit Enumerator(const Scope * scope_) : scope(scope_), name(nullptr), value(0), start(nullptr), end(nullptr), value_known(false) { }
    const Scope * scope;
    const Token * name;
    MathLib::bigint value;
    const Token * start;
    const Token * end;
    bool value_known;
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
        fHasDefault  = (1 << 9), /** @brief function argument with default value */
        fIsStlType   = (1 << 10), /** @brief STL type ('std::') */
        fIsStlString = (1 << 11), /** @brief std::string|wstring|basic_string&lt;T&gt;|u16string|u32string */
        fIsFloatType = (1 << 12), /** @brief Floating point type */
        fIsVolatile  = (1 << 13)  /** @brief volatile */
    };

    /**
     * Get specified flag state.
     * @param flag_ flag to get state of
     * @return true if flag set or false in flag not set
     */
    bool getFlag(unsigned int flag_) const {
        return ((mFlags & flag_) != 0);
    }

    /**
     * Set specified flag state.
     * @param flag_ flag to set state
     * @param state_ new state of flag
     */
    void setFlag(unsigned int flag_, bool state_) {
        mFlags = state_ ? mFlags | flag_ : mFlags & ~flag_;
    }

    /**
     * @brief parse and save array dimension information
     * @param lib Library instance
     * @return true if array, false if not
     */
    bool arrayDimensions(const Library* lib);

public:
    Variable(const Token *name_, const Token *start_, const Token *end_,
             std::size_t index_, AccessControl access_, const Type *type_,
             const Scope *scope_, const Settings* settings)
        : mNameToken(name_),
          mTypeStartToken(start_),
          mTypeEndToken(end_),
          mIndex(index_),
          mAccess(access_),
          mFlags(0),
          mType(type_),
          mScope(scope_),
          mValueType(nullptr) {
        evaluate(settings);
    }

    ~Variable();

    /**
     * Get name token.
     * @return name token
     */
    const Token *nameToken() const {
        return mNameToken;
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
        return mTypeStartToken;
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
        return mTypeEndToken;
    }

    /**
     * Get end token of variable declaration
     * E.g.
     * int i[2][3] = ...
     *   end token ^
     * @return variable declaration end token
     */
    const Token *declEndToken() const;

    /**
     * Get name string.
     * @return name string
     */
    const std::string &name() const {
        // name may not exist for function arguments
        if (mNameToken)
            return mNameToken->str();

        return emptyString;
    }

    /**
     * Get declaration ID (varId used for variable in its declaration).
     * @return declaration ID
     */
    unsigned int declarationId() const {
        // name may not exist for function arguments
        if (mNameToken)
            return mNameToken->varId();

        return 0;
    }

    /**
     * Get index of variable in declared order.
     * @return variable index
     */
    std::size_t index() const {
        return mIndex;
    }

    /**
     * Is variable public.
     * @return true if public, false if not
     */
    bool isPublic() const {
        return mAccess == Public;
    }

    /**
     * Is variable protected.
     * @return true if protected, false if not
     */
    bool isProtected() const {
        return mAccess == Protected;
    }

    /**
     * Is variable private.
     * @return true if private, false if not
     */
    bool isPrivate() const {
        return mAccess == Private;
    }

    /**
     * Is variable global.
     * @return true if global, false if not
     */
    bool isGlobal() const {
        return mAccess == Global;
    }

    /**
     * Is variable in a namespace.
     * @return true if in a namespace, false if not
     */
    bool isNamespace() const {
        return mAccess == Namespace;
    }

    /**
     * Is variable a function argument.
     * @return true if a function argument, false if not
     */
    bool isArgument() const {
        return mAccess == Argument;
    }

    /**
     * Is variable local.
     * @return true if local, false if not
     */
    bool isLocal() const {
        return (mAccess == Local) && !isExtern();
    }

    /**
     * Is variable mutable.
     * @return true if mutable, false if not
     */
    bool isMutable() const {
        return getFlag(fIsMutable);
    }

    /**
     * Is variable volatile.
     * @return true if volatile, false if not
     */
    bool isVolatile() const {
        return getFlag(fIsVolatile);
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
        return mAccess == Throw;
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
        return getFlag(fIsArray) && !getFlag(fIsPointer);
    }

    /**
     * Is pointer variable.
     * @return true if pointer, false otherwise
     */
    bool isPointer() const {
        return getFlag(fIsPointer);
    }

    /**
     * Is variable a pointer to an array
     * @return true if pointer to array, false otherwise
     */
    bool isPointerToArray() const {
        return isPointer() && getFlag(fIsArray);
    }

    /**
     * Is variable an array of pointers
     * @return true if array or pointers, false otherwise
     */
    bool isPointerArray() const;

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
        return mType;
    }

    /**
     * Get Scope pointer of known type.
     * @return pointer to type scope if known, NULL if not known
     */
    const Scope *typeScope() const {
        return mType ? mType->classScope : nullptr;
    }

    /**
     * Get Scope pointer of enclosing scope.
     * @return pointer to enclosing scope
     */
    const Scope *scope() const {
        return mScope;
    }

    /**
     * Get array dimensions.
     * @return array dimensions vector
     */
    const std::vector<Dimension> &dimensions() const {
        return mDimensions;
    }

    /**
     * Get array dimension length.
     * @return length of dimension
     */
    MathLib::bigint dimension(std::size_t index_) const {
        return mDimensions[index_].num;
    }

    /**
     * Get array dimension known.
     * @return length of dimension known
     */
    bool dimensionKnown(std::size_t index_) const {
        return mDimensions[index_].known;
    }

    /**
    * Checks if the variable is an STL type ('std::')
    * E.g.:
    *   std::string s;
    *   ...
    *   sVar->isStlType() == true
    * @return true if it is an stl type and its type matches any of the types in 'stlTypes'
    */
    bool isStlType() const {
        return getFlag(fIsStlType);
    }

    /**
     * Checks if the variable is an STL type ('std::')
     * E.g.:
     *   std::string s;
     *   ...
     *   sVar->isStlType() == true
     * @return true if it is an stl type and its type matches any of the types in 'stlTypes'
     */
    bool isStlStringType() const {
        return getFlag(fIsStlString);
    }

    /**
     * Checks if the variable is of any of the STL types passed as arguments ('std::')
     * E.g.:
     *   std::string s;
     *   ...
     *   const char *str[] = {"string", "wstring"};
     *   sVar->isStlType(str) == true
     * @param stlType stl type
     * @return true if it is an stl type and its type matches any of the types in 'stlTypes'
     */
    bool isStlType(const std::string& stlType) const {
        return isStlType() && stlType==mTypeStartToken->strAt(2);
    }

    /**
     * Checks if the variable is of any of the STL types passed as arguments ('std::')
     * E.g.:
     *   std::string s;
     *   ...
     *   const std::set<std::string> str = make_container< std::set<std::string> >() << "string" << "wstring";
     *   sVar->isStlType(str) == true
     * @param stlTypes set of stl types
     * @return true if it is an stl type and its type matches any of the types in 'stlTypes'
     */
    bool isStlType(const std::set<std::string>& stlTypes) const {
        return isStlType() && stlTypes.find(mTypeStartToken->strAt(2))!=stlTypes.end();
    }

    /**
    * Determine whether it's a floating number type
    * @return true if the type is known and it's a floating type (float, double and long double) or a pointer/array to it
    */
    bool isFloatingType() const {
        return getFlag(fIsFloatType);
    }

    /**
    * Determine whether it's an enumeration type
    * @return true if the type is known and it's an enumeration type
    */
    bool isEnumType() const {
        return type() && type()->isEnumType();
    }

    const ValueType *valueType() const {
        return mValueType;
    }

    void setValueType(const ValueType &valueType);

    AccessControl accessControl() const {
        return mAccess;
    }

private:
    // only symbol database can change the type
    friend class SymbolDatabase;

    /**
     * Set Type pointer to known type.
     * @param t type
     */
    void type(const Type * t) {
        mType = t;
    }

    /** @brief variable name token */
    const Token *mNameToken;

    /** @brief variable type start token */
    const Token *mTypeStartToken;

    /** @brief variable type end token */
    const Token *mTypeEndToken;

    /** @brief order declared */
    std::size_t mIndex;

    /** @brief what section is this variable declared in? */
    AccessControl mAccess;  // public/protected/private

    /** @brief flags */
    unsigned int mFlags;

    /** @brief pointer to user defined type info (for known types) */
    const Type *mType;

    /** @brief pointer to scope this variable is in */
    const Scope *mScope;

    ValueType *mValueType;

    /** @brief array dimensions */
    std::vector<Dimension> mDimensions;

    /** @brief fill in information, depending on Tokens given at instantiation */
    void evaluate(const Settings* settings);
};

class CPPCHECKLIB Function {
    /** @brief flags mask used to access specific bit. */
    enum {
        fHasBody              = (1 << 0),  ///< @brief has implementation
        fIsInline             = (1 << 1),  ///< @brief implementation in class definition
        fIsConst              = (1 << 2),  ///< @brief is const
        fIsVirtual            = (1 << 3),  ///< @brief is virtual
        fIsPure               = (1 << 4),  ///< @brief is pure virtual
        fIsStatic             = (1 << 5),  ///< @brief is static
        fIsStaticLocal        = (1 << 6),  ///< @brief is static local
        fIsExtern             = (1 << 7),  ///< @brief is extern
        fIsFriend             = (1 << 8),  ///< @brief is friend
        fIsExplicit           = (1 << 9),  ///< @brief is explicit
        fIsDefault            = (1 << 10), ///< @brief is default
        fIsDelete             = (1 << 11), ///< @brief is delete
        fHasOverrideSpecifier = (1 << 12), ///< @brief does declaration contain 'override' specifier?
        fHasFinalSpecifier    = (1 << 13), ///< @brief does declaration contain 'final' specifier?
        fIsNoExcept           = (1 << 14), ///< @brief is noexcept
        fIsThrow              = (1 << 15), ///< @brief is throw
        fIsOperator           = (1 << 16), ///< @brief is operator
        fHasLvalRefQual       = (1 << 17), ///< @brief has & lvalue ref-qualifier
        fHasRvalRefQual       = (1 << 18), ///< @brief has && rvalue ref-qualifier
        fIsVariadic           = (1 << 19), ///< @brief is variadic
        fIsVolatile           = (1 << 20)  ///< @brief is volatile
    };

    /**
     * Get specified flag state.
     * @param flag flag to get state of
     * @return true if flag set or false in flag not set
     */
    bool getFlag(unsigned int flag) const {
        return ((mFlags & flag) != 0);
    }

    /**
     * Set specified flag state.
     * @param flag flag to set state
     * @param state new state of flag
     */
    void setFlag(unsigned int flag, bool state) {
        mFlags = state ? mFlags | flag : mFlags & ~flag;
    }

public:
    enum Type { eConstructor, eCopyConstructor, eMoveConstructor, eOperatorEqual, eDestructor, eFunction };

    Function(const Tokenizer *mTokenizer, const Token *tok, const Scope *scope, const Token *tokDef, const Token *tokArgDef);

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

    /** @brief get function in base class that is overridden */
    const Function *getOverridenFunction(bool *foundAllBaseClasses = nullptr) const;

    bool isConstructor() const {
        return type==eConstructor ||
               type==eCopyConstructor ||
               type==eMoveConstructor;
    }

    bool isDestructor() const {
        return type==eDestructor;
    }
    bool isAttributeConstructor() const {
        return tokenDef->isAttributeConstructor();
    }
    bool isAttributeDestructor() const {
        return tokenDef->isAttributeDestructor();
    }
    bool isAttributePure() const {
        return tokenDef->isAttributePure();
    }
    bool isAttributeConst() const {
        return tokenDef->isAttributeConst();
    }
    bool isAttributeNoreturn() const {
        return tokenDef->isAttributeNoreturn();
    }
    bool isAttributeNothrow() const {
        return tokenDef->isAttributeNothrow();
    }
    bool isAttributeNodiscard() const {
        return tokenDef->isAttributeNodiscard();
    }

    bool hasBody() const {
        return getFlag(fHasBody);
    }
    bool isInline() const {
        return getFlag(fIsInline);
    }
    bool isConst() const {
        return getFlag(fIsConst);
    }
    bool isVirtual() const {
        return getFlag(fIsVirtual);
    }
    bool isPure() const {
        return getFlag(fIsPure);
    }
    bool isStatic() const {
        return getFlag(fIsStatic);
    }
    bool isStaticLocal() const {
        return getFlag(fIsStaticLocal);
    }
    bool isExtern() const {
        return getFlag(fIsExtern);
    }
    bool isFriend() const {
        return getFlag(fIsFriend);
    }
    bool isExplicit() const {
        return getFlag(fIsExplicit);
    }
    bool isDefault() const {
        return getFlag(fIsDefault);
    }
    bool isDelete() const {
        return getFlag(fIsDelete);
    }
    bool isNoExcept() const {
        return getFlag(fIsNoExcept);
    }
    bool isThrow() const {
        return getFlag(fIsThrow);
    }
    bool hasOverrideSpecifier() const {
        return getFlag(fHasOverrideSpecifier);
    }
    bool hasFinalSpecifier() const {
        return getFlag(fHasFinalSpecifier);
    }
    bool isOperator() const {
        return getFlag(fIsOperator);
    }
    bool hasLvalRefQualifier() const {
        return getFlag(fHasLvalRefQual);
    }
    bool hasRvalRefQualifier() const {
        return getFlag(fHasRvalRefQual);
    }
    bool isVariadic() const {
        return getFlag(fIsVariadic);
    }
    bool isVolatile() const {
        return getFlag(fIsVolatile);
    }

    void hasBody(bool state) {
        setFlag(fHasBody, state);
    }

    const Token *tokenDef;            ///< function name token in class definition
    const Token *argDef;              ///< function argument start '(' in class definition
    const Token *token;               ///< function name token in implementation
    const Token *arg;                 ///< function argument start '('
    const Token *retDef;              ///< function return type token
    const ::Type *retType;            ///< function return type
    const Scope *functionScope;       ///< scope of function body
    const Scope* nestedIn;            ///< Scope the function is declared in
    std::list<Variable> argumentList; ///< argument list
    unsigned int initArgCount;        ///< number of args with default values
    Type type;                        ///< constructor, destructor, ...
    AccessControl access;             ///< public/protected/private
    const Token *noexceptArg;         ///< noexcept token
    const Token *throwArg;            ///< throw token

    static bool argsMatch(const Scope *scope, const Token *first, const Token *second, const std::string &path, unsigned int path_length);

    /**
     * @return token to ":" if the function is a constructor
     * and it contains member initialization otherwise a nullptr is returned
     */
    const Token * constructorMemberInitialization() const;

private:
    /** Recursively determine if this function overrides a virtual method in a base class */
    const Function * getOverridenFunctionRecursive(const ::Type* baseType, bool *foundAllBaseClasses) const;

    unsigned int mFlags;

    void isInline(bool state) {
        setFlag(fIsInline, state);
    }
    void isConst(bool state) {
        setFlag(fIsConst, state);
    }
    void isVirtual(bool state) {
        setFlag(fIsVirtual, state);
    }
    void isPure(bool state) {
        setFlag(fIsPure, state);
    }
    void isStatic(bool state) {
        setFlag(fIsStatic, state);
    }
    void isStaticLocal(bool state) {
        setFlag(fIsStaticLocal, state);
    }
    void isExtern(bool state) {
        setFlag(fIsExtern, state);
    }
    void isFriend(bool state) {
        setFlag(fIsFriend, state);
    }
    void isExplicit(bool state) {
        setFlag(fIsExplicit, state);
    }
    void isDefault(bool state) {
        setFlag(fIsDefault, state);
    }
    void isDelete(bool state) {
        setFlag(fIsDelete, state);
    }
    void isNoExcept(bool state) {
        setFlag(fIsNoExcept, state);
    }
    void isThrow(bool state) {
        setFlag(fIsThrow, state);
    }
    void isOperator(bool state) {
        setFlag(fIsOperator, state);
    }
    void hasLvalRefQualifier(bool state) {
        setFlag(fHasLvalRefQual, state);
    }
    void hasRvalRefQualifier(bool state) {
        setFlag(fHasRvalRefQual, state);
    }
    void isVariadic(bool state) {
        setFlag(fIsVariadic, state);
    }
    void isVolatile(bool state) {
        setFlag(fIsVolatile, state);
    }
};

class CPPCHECKLIB Scope {
    // let tests access private function for testing
    friend class TestSymbolDatabase;

public:
    struct UsingInfo {
        const Token *start;
        const Scope *scope;
    };

    enum ScopeType { eGlobal, eClass, eStruct, eUnion, eNamespace, eFunction, eIf, eElse, eFor, eWhile, eDo, eSwitch, eUnconditional, eTry, eCatch, eLambda, eEnum };

    Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_);
    Scope(const SymbolDatabase *check_, const Token *classDef_, const Scope *nestedIn_, ScopeType type_, const Token *start_);

    const SymbolDatabase *check;
    std::string className;
    const Token *classDef;   ///< class/struct/union/namespace token
    const Token *bodyStart;  ///< '{' token
    const Token *bodyEnd;    ///< '}' token
    std::list<Function> functionList;
    std::multimap<std::string, const Function *> functionMap;
    std::list<Variable> varlist;
    const Scope *nestedIn;
    std::list<Scope *> nestedList;
    unsigned int numConstructors;
    unsigned int numCopyOrMoveConstructors;
    std::list<UsingInfo> usingList;
    ScopeType type;
    Type* definedType;
    std::map<std::string, Type*> definedTypesMap;

    // function specific fields
    const Scope *functionOf; ///< scope this function belongs to
    Function *function; ///< function info for this function

    // enum specific fields
    const Token * enumType;
    bool enumClass;

    std::vector<Enumerator> enumeratorList;

    const Enumerator * findEnumerator(const std::string & name) const {
        for (std::size_t i = 0, end = enumeratorList.size(); i < end; ++i) {
            if (enumeratorList[i].name->str() == name)
                return &enumeratorList[i];
        }
        return nullptr;
    }

    bool isNestedIn(const Scope * outer) const {
        if (!outer)
            return false;
        if (outer == this)
            return true;
        const Scope * parent = nestedIn;
        while (outer != parent && parent)
            parent = parent->nestedIn;
        if (parent && parent == outer)
            return true;
        return false;
    }

    bool isClassOrStruct() const {
        return (type == eClass || type == eStruct);
    }

    bool isClassOrStructOrUnion() const {
        return (type == eClass || type == eStruct || type == eUnion);
    }

    bool isExecutable() const {
        return type != eClass && type != eStruct && type != eUnion && type != eGlobal && type != eNamespace && type != eEnum;
    }

    bool isLocal() const {
        return (type == eIf || type == eElse ||
                type == eFor || type == eWhile || type == eDo ||
                type == eSwitch || type == eUnconditional ||
                type == eTry || type == eCatch);
    }

    // Is there lambda/inline function(s) in this scope?
    bool hasInlineOrLambdaFunction() const;

    /**
     * @brief find a function
     * @param tok token of function call
     * @param requireConst if const refers to a const variable only const methods should be matched
     * @return pointer to function if found or NULL if not found
     */
    const Function *findFunction(const Token *tok, bool requireConst=false) const;

    /**
     * @brief find if name is in nested list
     * @param name name of nested scope
     */
    Scope *findInNestedList(const std::string & name);

    const Scope *findRecordInNestedList(const std::string & name) const;
    Scope *findRecordInNestedList(const std::string & name) {
        return const_cast<Scope *>(const_cast<const Scope *>(this)->findRecordInNestedList(name));
    }

    const Type* findType(const std::string& name) const;
    Type* findType(const std::string& name) {
        return const_cast<Type*>(const_cast<const Scope *>(this)->findType(name));
    }

    /**
     * @brief find if name is in nested list
     * @param name name of nested scope
     */
    Scope *findInNestedListRecursive(const std::string & name);

    void addVariable(const Token *token_, const Token *start_,
                     const Token *end_, AccessControl access_, const Type *type_,
                     const Scope *scope_, const Settings* settings) {
        varlist.emplace_back(token_, start_, end_, varlist.size(),
                             access_,
                             type_, scope_, settings);
    }

    /** @brief initialize varlist */
    void getVariableList(const Settings* settings);

    const Function *getDestructor() const;

    void addFunction(const Function & func) {
        functionList.push_back(func);

        const Function * back = &functionList.back();

        functionMap.insert(make_pair(back->tokenDef->str(), back));
    }

    bool hasDefaultConstructor() const;

    AccessControl defaultAccess() const;

    /**
     * @brief check if statement is variable declaration and add it if it is
     * @param tok pointer to start of statement
     * @param varaccess access control of statement
     * @param settings Settings
     * @return pointer to last token
     */
    const Token *checkVariable(const Token *tok, AccessControl varaccess, const Settings* settings);

    /**
     * @brief get variable from name
     * @param varname name of variable
     * @return pointer to variable
     */
    const Variable *getVariable(const std::string &varname) const;

    const Token * addEnum(const Token * tok, bool isCpp);

private:
    /**
     * @brief helper function for getVariableList()
     * @param tok pointer to token to check
     * @param vartok populated with pointer to the variable token, if found
     * @param typetok populated with pointer to the type token, if found
     * @return true if tok points to a variable declaration, false otherwise
     */
    bool isVariableDeclaration(const Token* const tok, const Token*& vartok, const Token*& typetok) const;

    void findFunctionInBase(const std::string & name, size_t args, std::vector<const Function *> & matches) const;
};


/** Value type */
class CPPCHECKLIB ValueType {
public:
    enum Sign { UNKNOWN_SIGN, SIGNED, UNSIGNED } sign;
    enum Type { UNKNOWN_TYPE, NONSTD, RECORD, CONTAINER, ITERATOR, VOID, BOOL, CHAR, SHORT, INT, LONG, LONGLONG, UNKNOWN_INT, FLOAT, DOUBLE, LONGDOUBLE } type;
    unsigned int bits;                    ///< bitfield bitcount
    unsigned int pointer;                 ///< 0=>not pointer, 1=>*, 2=>**, 3=>***, etc
    unsigned int constness;               ///< bit 0=data, bit 1=*, bit 2=**
    const Scope *typeScope;               ///< if the type definition is seen this point out the type scope
    const Library::Container *container;  ///< If the type is a container defined in a cfg file, this is the used container
    const Token *containerTypeToken;      ///< The container type token. the template argument token that defines the container element type.
    std::string originalTypeName;         ///< original type name as written in the source code. eg. this might be "uint8_t" when type is CHAR.

    ValueType() : sign(UNKNOWN_SIGN), type(UNKNOWN_TYPE), bits(0), pointer(0U), constness(0U), typeScope(nullptr), container(nullptr), containerTypeToken(nullptr) {}
    ValueType(const ValueType &vt) : sign(vt.sign), type(vt.type), bits(vt.bits), pointer(vt.pointer), constness(vt.constness), typeScope(vt.typeScope), container(vt.container), containerTypeToken(vt.containerTypeToken), originalTypeName(vt.originalTypeName) {}
    ValueType(enum Sign s, enum Type t, unsigned int p) : sign(s), type(t), bits(0), pointer(p), constness(0U), typeScope(nullptr), container(nullptr), containerTypeToken(nullptr) {}
    ValueType(enum Sign s, enum Type t, unsigned int p, unsigned int c) : sign(s), type(t), bits(0), pointer(p), constness(c), typeScope(nullptr), container(nullptr), containerTypeToken(nullptr) {}
    ValueType(enum Sign s, enum Type t, unsigned int p, unsigned int c, const std::string &otn) : sign(s), type(t), bits(0), pointer(p), constness(c), typeScope(nullptr), container(nullptr), containerTypeToken(nullptr), originalTypeName(otn) {}
    ValueType &operator=(const ValueType &other) = delete;

    static ValueType parseDecl(const Token *type, const Settings *settings);

    static Type typeFromString(const std::string &typestr, bool longType);

    bool isIntegral() const {
        return (type >= ValueType::Type::BOOL && type <= ValueType::Type::UNKNOWN_INT);
    }

    bool isFloat() const {
        return (type >= ValueType::Type::FLOAT && type <= ValueType::Type::LONGDOUBLE);
    }

    bool fromLibraryType(const std::string &typestr, const Settings *settings);

    bool isEnum() const {
        return typeScope && typeScope->type == Scope::eEnum;
    }

    std::string str() const;
    std::string dump() const;
};


class CPPCHECKLIB SymbolDatabase {
    friend class TestSymbolDatabase;
public:
    SymbolDatabase(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger);
    ~SymbolDatabase();

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
     * @param typeTok token containing variable type
     * @return pointer to type if found or NULL if not found
     */
    const Type *findVariableType(const Scope *start, const Token *typeTok) const;

    /**
     * @brief find a function
     * @param tok token of function call
     * @return pointer to function if found or NULL if not found
     */
    const Function *findFunction(const Token *tok) const;

    const Scope *findScopeByName(const std::string& name) const;

    const Type* findType(const Token *startTok, const Scope *startScope) const;
    Type* findType(const Token *startTok, Scope *startScope) const {
        return const_cast<Type*>(this->findType(startTok, const_cast<const Scope *>(startScope)));
    }

    const Scope *findScope(const Token *tok, const Scope *startScope) const;
    Scope *findScope(const Token *tok, Scope *startScope) const {
        return const_cast<Scope *>(this->findScope(tok, const_cast<const Scope *>(startScope)));
    }

    const Variable *getVariableFromVarId(std::size_t varId) const {
        return mVariableList.at(varId);
    }

    const std::vector<const Variable *> & variableList() const {
        return mVariableList;
    }

    /**
     * @brief output a debug message
     */
    void debugMessage(const Token *tok, const std::string &msg) const;

    void printOut(const char * title = nullptr) const;
    void printVariable(const Variable *var, const char *indent) const;
    void printXml(std::ostream &out) const;

    bool isCPP() const;

    /*
     * @brief Do a sanity check
     */
    void validate() const;

    void validateExecutableScopes() const;
    /**
     * @brief Check variable list, e.g. variables w/o scope
     */
    void validateVariables() const;

    /** Set valuetype in provided tokenlist */
    void setValueTypeInTokenList();

    /**
     * Calculates sizeof value for given type.
     * @param type Token which will contain e.g. "int", "*", or string.
     * @return sizeof for given type, or 0 if it can't be calculated.
     */
    unsigned int sizeOfType(const Token *type) const;

private:
    friend class Scope;
    friend class Function;

    // Create symboldatabase...
    void createSymbolDatabaseFindAllScopes();
    void createSymbolDatabaseClassInfo();
    void createSymbolDatabaseVariableInfo();
    void createSymbolDatabaseCopyAndMoveConstructors();
    void createSymbolDatabaseFunctionScopes();
    void createSymbolDatabaseClassAndStructScopes();
    void createSymbolDatabaseFunctionReturnTypes();
    void createSymbolDatabaseNeedInitialization();
    void createSymbolDatabaseVariableSymbolTable();
    void createSymbolDatabaseSetScopePointers();
    void createSymbolDatabaseSetFunctionPointers(bool firstPass);
    void createSymbolDatabaseSetVariablePointers();
    void createSymbolDatabaseSetTypePointers();
    void createSymbolDatabaseEnums();
    void createSymbolDatabaseUnknownArrayDimensions();

    void addClassFunction(Scope **scope, const Token **tok, const Token *argStart);
    Function *addGlobalFunctionDecl(Scope*& scope, const Token* tok, const Token *argStart, const Token* funcStart);
    Function *addGlobalFunction(Scope*& scope, const Token*& tok, const Token *argStart, const Token* funcStart);
    void addNewFunction(Scope **scope, const Token **tok);
    bool isFunction(const Token *tok, const Scope* outerScope, const Token **funcStart, const Token **argStart, const Token** declEnd) const;
    const Type *findTypeInNested(const Token *startTok, const Scope *startScope) const;
    const Scope *findNamespace(const Token * tok, const Scope * scope) const;
    Function *findFunctionInScope(const Token *func, const Scope *ns, const std::string & path, unsigned int path_length);
    const Type *findVariableTypeInBase(const Scope *scope, const Token *typeTok) const;

    typedef std::map<unsigned int, unsigned int> MemberIdMap;
    typedef std::map<unsigned int, MemberIdMap> VarIdMap;

    void fixVarId(VarIdMap & varIds, const Token * vartok, Token * membertok, const Variable * membervar);

    /** Whether iName is a keyword as defined in http://en.cppreference.com/w/c/keyword and http://en.cppreference.com/w/cpp/keyword*/
    bool isReservedName(const std::string& iName) const;

    const Enumerator * findEnumerator(const Token * tok) const;

    void setValueType(Token *tok, const ValueType &valuetype);
    void setValueType(Token *tok, const Variable &var);
    void setValueType(Token *tok, const Enumerator &enumerator);

    const Tokenizer *mTokenizer;
    const Settings *mSettings;
    ErrorLogger *mErrorLogger;

    /** variable symbol table */
    std::vector<const Variable *> mVariableList;

    /** list for missing types */
    std::list<Type> mBlankTypes;

    bool mIsCpp;
    ValueType::Sign mDefaultSignedness;

    /** "negative cache" list of tokens that we find are not enumeration values */
    mutable std::set<std::string> mTokensThatAreNotEnumeratorValues;
};


//---------------------------------------------------------------------------
#endif // symboldatabaseH

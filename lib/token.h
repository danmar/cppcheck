/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
#ifndef tokenH
#define tokenH
//---------------------------------------------------------------------------

#include "config.h"
#include "mathlib.h"
#include "templatesimplifier.h"
#include "utils.h"
#include "vfvalue.h"

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

struct Enumerator;
class Function;
class Scope;
class Settings;
class Type;
class ValueType;
class Variable;
class TokenList;
class ConstTokenRange;
class Token;

/**
 * @brief This struct stores pointers to the front and back tokens of the list this token is in.
 */
struct TokensFrontBack {
    Token *front{};
    Token* back{};
    const TokenList* list{};
};

struct ScopeInfo2 {
    ScopeInfo2(std::string name_, const Token *bodyEnd_, std::set<std::string> usingNamespaces_ = std::set<std::string>()) : name(std::move(name_)), bodyEnd(bodyEnd_), usingNamespaces(std::move(usingNamespaces_)) {}
    std::string name;
    const Token* const bodyEnd{};
    std::set<std::string> usingNamespaces;
};

enum class TokenDebug { None, ValueFlow, ValueType };

struct TokenImpl {
    nonneg int mVarId{};
    nonneg int mFileIndex{};
    nonneg int mLineNumber{};
    nonneg int mColumn{};
    nonneg int mExprId{};

    /**
     * A value from 0-100 that provides a rough idea about where in the token
     * list this token is located.
     */
    nonneg int mProgressValue{};

    /**
     * Token index. Position in token list
     */
    nonneg int mIndex{};

    /** Bitfield bit count. */
    unsigned char mBits{};

    // AST..
    Token* mAstOperand1{};
    Token* mAstOperand2{};
    Token* mAstParent{};

    // symbol database information
    const Scope* mScope{};
    union {
        const Function *mFunction;
        const Variable *mVariable;
        const ::Type* mType;
        const Enumerator *mEnumerator;
    };

    // original name like size_t
    std::string* mOriginalName{};

    // ValueType
    ValueType* mValueType{};

    // ValueFlow
    std::list<ValueFlow::Value>* mValues{};
    static const std::list<ValueFlow::Value> mEmptyValueList;

    // Pointer to a template in the template simplifier
    std::set<TemplateSimplifier::TokenAndName*>* mTemplateSimplifierPointers{};

    // Pointer to the object representing this token's scope
    std::shared_ptr<ScopeInfo2> mScopeInfo;

    // __cppcheck_in_range__
    struct CppcheckAttributes {
        enum Type { LOW, HIGH } type = LOW;
        MathLib::bigint value{};
        CppcheckAttributes* next{};
    };
    CppcheckAttributes* mCppcheckAttributes{};

    // For memoization, to speed up parsing of huge arrays #8897
    enum class Cpp11init { UNKNOWN, CPP11INIT, NOINIT } mCpp11init = Cpp11init::UNKNOWN;

    TokenDebug mDebug{};

    void setCppcheckAttribute(CppcheckAttributes::Type type, MathLib::bigint value);
    bool getCppcheckAttribute(CppcheckAttributes::Type type, MathLib::bigint &value) const;

    TokenImpl() : mFunction(nullptr) {}

    ~TokenImpl();
};

/// @addtogroup Core
/// @{

/**
 * @brief The token list that the TokenList generates is a linked-list of this class.
 *
 * Tokens are stored as strings. The "if", "while", etc are stored in plain text.
 * The reason the Token class is needed (instead of using the string class) is that some extra functionality is also needed for tokens:
 *  - location of the token is stored (fileIndex, linenr, column)
 *  - functions for classifying the token (isName, isNumber, isBoolean, isStandardType)
 *
 * The Token class also has other functions for management of token list, matching tokens, etc.
 */
class CPPCHECKLIB Token {
    friend class TestToken;

private:
    TokensFrontBack* mTokensFrontBack{};

public:
    Token(const Token &) = delete;
    Token& operator=(const Token &) = delete;

    enum Type {
        eVariable, eType, eFunction, eKeyword, eName, // Names: Variable (varId), Type (typeId, later), Function (FuncId, later), Language keyword, Name (unknown identifier)
        eNumber, eString, eChar, eBoolean, eLiteral, eEnumerator, // Literals: Number, String, Character, Boolean, User defined literal (C++11), Enumerator
        eArithmeticalOp, eComparisonOp, eAssignmentOp, eLogicalOp, eBitOp, eIncDecOp, eExtendedOp, // Operators: Arithmetical, Comparison, Assignment, Logical, Bitwise, ++/--, Extended
        eBracket, // {, }, <, >: < and > only if link() is set. Otherwise they are comparison operators.
        eLambda, // A function without a name
        eEllipsis, // "..."
        eOther,
        eNone
    };

    explicit Token(TokensFrontBack *tokensFrontBack = nullptr);
    ~Token();

    ConstTokenRange until(const Token * t) const;

    template<typename T>
    void str(T&& s) {
        mStr = s;
        mImpl->mVarId = 0;

        update_property_info();
    }

    /**
     * Concatenate two (quoted) strings. Automatically cuts of the last/first character.
     * Example: "hello ""world" -> "hello world". Used by the token simplifier.
     */
    void concatStr(std::string const& b);

    const std::string &str() const {
        return mStr;
    }

    /**
     * Unlink and delete the next 'count' tokens.
     */
    void deleteNext(nonneg int count = 1);

    /**
     * Unlink and delete the previous 'count' tokens.
     */
    void deletePrevious(nonneg int count = 1);

    /**
     * Swap the contents of this token with the next token.
     */
    void swapWithNext();

    /**
     * @return token in given index, related to this token.
     * For example index 1 would return next token, and 2
     * would return next from that one.
     */
    const Token *tokAt(int index) const;
    Token *tokAt(int index) {
        return const_cast<Token *>(const_cast<const Token *>(this)->tokAt(index));
    }

    /**
     * @return the link to the token in given index, related to this token.
     * For example index 1 would return the link to next token.
     */
    const Token *linkAt(int index) const;
    Token *linkAt(int index) {
        return const_cast<Token *>(const_cast<const Token *>(this)->linkAt(index));
    }

    /**
     * @return String of the token in given index, related to this token.
     * If that token does not exist, an empty string is being returned.
     */
    const std::string &strAt(int index) const;

    /**
     * Match given token (or list of tokens) to a pattern list.
     *
     * Possible patterns
     * "someRandomText" If token contains "someRandomText".
     * @note Use Match() if you want to use flags in patterns
     *
     * The patterns can be also combined to compare to multiple tokens at once
     * by separating tokens with a space, e.g.
     * ") void {" will return true if first token is ')' next token
     * is "void" and token after that is '{'. If even one of the tokens does
     * not match its pattern, false is returned.
     *
     * @param tok List of tokens to be compared to the pattern
     * @param pattern The pattern against which the tokens are compared,
     * e.g. "const" or ") void {".
     * @return true if given token matches with given pattern
     *         false if given token does not match with given pattern
     */
    template<size_t count>
    static bool simpleMatch(const Token *tok, const char (&pattern)[count]) {
        return simpleMatch(tok, pattern, count-1);
    }

    static bool simpleMatch(const Token *tok, const char pattern[], size_t pattern_len);

    /**
     * Match given token (or list of tokens) to a pattern list.
     *
     * Possible patterns
     * - "%any%" any token
     * - "%assign%" a assignment operand
     * - "%bool%" true or false
     * - "%char%" Any token enclosed in &apos;-character.
     * - "%comp%" Any token such that isComparisonOp() returns true.
     * - "%cop%" Any token such that isConstOp() returns true.
     * - "%name%" any token which is a name, variable or type e.g. "hello" or "int"
     * - "%num%" Any numeric token, e.g. "23"
     * - "%op%" Any token such that isOp() returns true.
     * - "%or%" A bitwise-or operator '|'
     * - "%oror%" A logical-or operator '||'
     * - "%type%" Anything that can be a variable type, e.g. "int", but not "delete".
     * - "%str%" Any token starting with &quot;-character (C-string).
     * - "%var%" Match with token with varId > 0
     * - "%varid%" Match with parameter varid
     * - "[abc]" Any of the characters 'a' or 'b' or 'c'
     * - "int|void|char" Any of the strings, int, void or char
     * - "int|void|char|" Any of the strings, int, void or char or empty string
     * - "!!else" No tokens or any token that is not "else".
     * - "someRandomText" If token contains "someRandomText".
     *
     * multi-compare patterns such as "int|void|char" can contain %%or%, %%oror% and %%op%
     * it is recommended to put such an %%cmd% as the first pattern.
     * For example: "%var%|%num%|)" means yes to a variable, a number or ')'.
     *
     * The patterns can be also combined to compare to multiple tokens at once
     * by separating tokens with a space, e.g.
     * ") const|void {" will return true if first token is ')' next token is either
     * "const" or "void" and token after that is '{'. If even one of the tokens does not
     * match its pattern, false is returned.
     *
     * @param tok List of tokens to be compared to the pattern
     * @param pattern The pattern against which the tokens are compared,
     * e.g. "const" or ") const|volatile| {".
     * @param varid if %%varid% is given in the pattern the Token::varId
     * will be matched against this argument
     * @return true if given token matches with given pattern
     *         false if given token does not match with given pattern
     */
    static bool Match(const Token *tok, const char pattern[], nonneg int varid = 0);

    /**
     * @return length of C-string.
     *
     * Should be called for %%str%% tokens only.
     *
     * @param tok token with C-string
     **/
    static nonneg int getStrLength(const Token *tok);

    /**
     * @return array length of C-string.
     *
     * Should be called for %%str%% tokens only.
     *
     * @param tok token with C-string
     **/
    static nonneg int getStrArraySize(const Token *tok);

    /**
     * @return sizeof of C-string.
     *
     * Should be called for %%str%% tokens only.
     *
     * @param tok token with C-string
     * @param settings Settings
     **/
    static nonneg int getStrSize(const Token *tok, const Settings *const settings);

    const ValueType *valueType() const {
        return mImpl->mValueType;
    }
    void setValueType(ValueType *vt);

    const ValueType *argumentType() const {
        const Token *top = this;
        while (top && !Token::Match(top->astParent(), ",|("))
            top = top->astParent();
        return top ? top->mImpl->mValueType : nullptr;
    }

    Token::Type tokType() const {
        return mTokType;
    }
    void tokType(Token::Type t) {
        mTokType = t;

        const bool memoizedIsName = (mTokType == eName || mTokType == eType || mTokType == eVariable ||
                                     mTokType == eFunction || mTokType == eKeyword || mTokType == eBoolean ||
                                     mTokType == eEnumerator); // TODO: "true"/"false" aren't really a name...
        setFlag(fIsName, memoizedIsName);

        const bool memoizedIsLiteral = (mTokType == eNumber || mTokType == eString || mTokType == eChar ||
                                        mTokType == eBoolean || mTokType == eLiteral || mTokType == eEnumerator);
        setFlag(fIsLiteral, memoizedIsLiteral);
    }
    bool isKeyword() const {
        return mTokType == eKeyword;
    }
    bool isName() const {
        return getFlag(fIsName);
    }
    bool isNameOnly() const {
        return mFlags == fIsName && mTokType == eName;
    }
    bool isUpperCaseName() const;
    bool isLiteral() const {
        return getFlag(fIsLiteral);
    }
    bool isNumber() const {
        return mTokType == eNumber;
    }
    bool isEnumerator() const {
        return mTokType == eEnumerator;
    }
    bool isVariable() const {
        return mTokType == eVariable;
    }
    bool isOp() const {
        return (isConstOp() ||
                isAssignmentOp() ||
                mTokType == eIncDecOp);
    }
    bool isConstOp() const {
        return (isArithmeticalOp() ||
                mTokType == eLogicalOp ||
                mTokType == eComparisonOp ||
                mTokType == eBitOp);
    }
    bool isExtendedOp() const {
        return isConstOp() ||
               mTokType == eExtendedOp;
    }
    bool isArithmeticalOp() const {
        return mTokType == eArithmeticalOp;
    }
    bool isComparisonOp() const {
        return mTokType == eComparisonOp;
    }
    bool isAssignmentOp() const {
        return mTokType == eAssignmentOp;
    }
    bool isBoolean() const {
        return mTokType == eBoolean;
    }
    bool isIncDecOp() const {
        return mTokType == eIncDecOp;
    }
    bool isBinaryOp() const {
        return astOperand1() != nullptr && astOperand2() != nullptr;
    }
    bool isUnaryOp(const std::string &s) const {
        return s == mStr && astOperand1() != nullptr && astOperand2() == nullptr;
    }
    bool isUnaryPreOp() const;

    uint64_t flags() const {
        return mFlags;
    }
    void flags(const uint64_t flags_) {
        mFlags = flags_;
    }
    bool isUnsigned() const {
        return getFlag(fIsUnsigned);
    }
    void isUnsigned(const bool sign) {
        setFlag(fIsUnsigned, sign);
    }
    bool isSigned() const {
        return getFlag(fIsSigned);
    }
    void isSigned(const bool sign) {
        setFlag(fIsSigned, sign);
    }
    bool isPointerCompare() const {
        return getFlag(fIsPointerCompare);
    }
    void isPointerCompare(const bool b) {
        setFlag(fIsPointerCompare, b);
    }
    bool isLong() const {
        return getFlag(fIsLong);
    }
    void isLong(bool size) {
        setFlag(fIsLong, size);
    }
    bool isStandardType() const {
        return getFlag(fIsStandardType);
    }
    void isStandardType(const bool b) {
        setFlag(fIsStandardType, b);
    }
    bool isExpandedMacro() const {
        return getFlag(fIsExpandedMacro);
    }
    void isExpandedMacro(const bool m) {
        setFlag(fIsExpandedMacro, m);
    }
    bool isCast() const {
        return getFlag(fIsCast);
    }
    void isCast(bool c) {
        setFlag(fIsCast, c);
    }
    bool isAttributeConstructor() const {
        return getFlag(fIsAttributeConstructor);
    }
    void isAttributeConstructor(const bool ac) {
        setFlag(fIsAttributeConstructor, ac);
    }
    bool isAttributeDestructor() const {
        return getFlag(fIsAttributeDestructor);
    }
    void isAttributeDestructor(const bool value) {
        setFlag(fIsAttributeDestructor, value);
    }
    bool isAttributeUnused() const {
        return getFlag(fIsAttributeUnused);
    }
    void isAttributeUnused(bool unused) {
        setFlag(fIsAttributeUnused, unused);
    }
    bool isAttributeUsed() const {
        return getFlag(fIsAttributeUsed);
    }
    void isAttributeUsed(const bool unused) {
        setFlag(fIsAttributeUsed, unused);
    }
    bool isAttributePure() const {
        return getFlag(fIsAttributePure);
    }
    void isAttributePure(const bool value) {
        setFlag(fIsAttributePure, value);
    }
    bool isAttributeConst() const {
        return getFlag(fIsAttributeConst);
    }
    void isAttributeConst(bool value) {
        setFlag(fIsAttributeConst, value);
    }
    bool isAttributeNoreturn() const {
        return getFlag(fIsAttributeNoreturn);
    }
    void isAttributeNoreturn(const bool value) {
        setFlag(fIsAttributeNoreturn, value);
    }
    bool isAttributeNothrow() const {
        return getFlag(fIsAttributeNothrow);
    }
    void isAttributeNothrow(const bool value) {
        setFlag(fIsAttributeNothrow, value);
    }
    bool isAttributeExport() const {
        return getFlag(fIsAttributeExport);
    }
    void isAttributeExport(const bool value) {
        setFlag(fIsAttributeExport, value);
    }
    bool isAttributePacked() const {
        return getFlag(fIsAttributePacked);
    }
    void isAttributePacked(const bool value) {
        setFlag(fIsAttributePacked, value);
    }
    bool isAttributeNodiscard() const {
        return getFlag(fIsAttributeNodiscard);
    }
    void isAttributeNodiscard(const bool value) {
        setFlag(fIsAttributeNodiscard, value);
    }
    bool isAttributeMaybeUnused() const {
        return getFlag(fIsAttributeMaybeUnused);
    }
    void isAttributeMaybeUnused(const bool value) {
        setFlag(fIsAttributeMaybeUnused, value);
    }
    void setCppcheckAttribute(TokenImpl::CppcheckAttributes::Type type, MathLib::bigint value) {
        mImpl->setCppcheckAttribute(type, value);
    }
    bool getCppcheckAttribute(TokenImpl::CppcheckAttributes::Type type, MathLib::bigint &value) const {
        return mImpl->getCppcheckAttribute(type, value);
    }
    bool hasCppcheckAttributes() const {
        return nullptr != mImpl->mCppcheckAttributes;
    }
    bool isControlFlowKeyword() const {
        return getFlag(fIsControlFlowKeyword);
    }
    bool isOperatorKeyword() const {
        return getFlag(fIsOperatorKeyword);
    }
    void isOperatorKeyword(const bool value) {
        setFlag(fIsOperatorKeyword, value);
    }
    bool isComplex() const {
        return getFlag(fIsComplex);
    }
    void isComplex(const bool value) {
        setFlag(fIsComplex, value);
    }
    bool isEnumType() const {
        return getFlag(fIsEnumType);
    }
    void isEnumType(const bool value) {
        setFlag(fIsEnumType, value);
    }
    bool isAtAddress() const {
        return getFlag(fAtAddress);
    }
    void isAtAddress(bool b) {
        setFlag(fAtAddress, b);
    }
    bool isIncompleteVar() const {
        return getFlag(fIncompleteVar);
    }
    void isIncompleteVar(bool b) {
        setFlag(fIncompleteVar, b);
    }

    bool isSimplifiedTypedef() const {
        return getFlag(fIsSimplifiedTypedef);
    }
    void isSimplifiedTypedef(bool b) {
        setFlag(fIsSimplifiedTypedef, b);
    }

    bool isIncompleteConstant() const {
        return getFlag(fIsIncompleteConstant);
    }
    void isIncompleteConstant(bool b) {
        setFlag(fIsIncompleteConstant, b);
    }

    bool isConstexpr() const {
        return getFlag(fConstexpr);
    }
    void isConstexpr(bool b) {
        setFlag(fConstexpr, b);
    }

    bool isExternC() const {
        return getFlag(fExternC);
    }
    void isExternC(bool b) {
        setFlag(fExternC, b);
    }

    bool isSplittedVarDeclComma() const {
        return getFlag(fIsSplitVarDeclComma);
    }
    void isSplittedVarDeclComma(bool b) {
        setFlag(fIsSplitVarDeclComma, b);
    }

    bool isSplittedVarDeclEq() const {
        return getFlag(fIsSplitVarDeclEq);
    }
    void isSplittedVarDeclEq(bool b) {
        setFlag(fIsSplitVarDeclEq, b);
    }

    bool isImplicitInt() const {
        return getFlag(fIsImplicitInt);
    }
    void isImplicitInt(bool b) {
        setFlag(fIsImplicitInt, b);
    }

    bool isInline() const {
        return getFlag(fIsInline);
    }
    void isInline(bool b) {
        setFlag(fIsInline, b);
    }

    bool isAtomic() const {
        return getFlag(fIsAtomic);
    }
    void isAtomic(bool b) {
        setFlag(fIsAtomic, b);
    }

    bool isRestrict() const {
        return getFlag(fIsRestrict);
    }
    void isRestrict(bool b) {
        setFlag(fIsRestrict, b);
    }

    bool isRemovedVoidParameter() const {
        return getFlag(fIsRemovedVoidParameter);
    }
    void setRemovedVoidParameter(bool b) {
        setFlag(fIsRemovedVoidParameter, b);
    }

    bool isTemplate() const {
        return getFlag(fIsTemplate);
    }
    void isTemplate(bool b) {
        setFlag(fIsTemplate, b);
    }

    bool isSimplifiedScope() const {
        return getFlag(fIsSimplifedScope);
    }
    void isSimplifiedScope(bool b) {
        setFlag(fIsSimplifedScope, b);
    }

    bool isFinalType() const {
        return getFlag(fIsFinalType);
    }
    void isFinalType(bool b) {
        setFlag(fIsFinalType, b);
    }

    bool isInitComma() const {
        return getFlag(fIsInitComma);
    }
    void isInitComma(bool b) {
        setFlag(fIsInitComma, b);
    }

    bool isBitfield() const {
        return mImpl->mBits > 0;
    }
    unsigned char bits() const {
        return mImpl->mBits;
    }
    const std::set<TemplateSimplifier::TokenAndName*>* templateSimplifierPointers() const {
        return mImpl->mTemplateSimplifierPointers;
    }
    std::set<TemplateSimplifier::TokenAndName*>* templateSimplifierPointers() {
        return mImpl->mTemplateSimplifierPointers;
    }
    void templateSimplifierPointer(TemplateSimplifier::TokenAndName* tokenAndName) {
        if (!mImpl->mTemplateSimplifierPointers)
            mImpl->mTemplateSimplifierPointers = new std::set<TemplateSimplifier::TokenAndName*>;
        mImpl->mTemplateSimplifierPointers->insert(tokenAndName);
    }
    void setBits(const unsigned char b) {
        mImpl->mBits = b;
    }

    bool isUtf8() const {
        return (((mTokType == eString) && isPrefixStringCharLiteral(mStr, '"', "u8")) ||
                ((mTokType == eChar) && isPrefixStringCharLiteral(mStr, '\'', "u8")));
    }

    bool isUtf16() const {
        return (((mTokType == eString) && isPrefixStringCharLiteral(mStr, '"', "u")) ||
                ((mTokType == eChar) && isPrefixStringCharLiteral(mStr, '\'', "u")));
    }

    bool isUtf32() const {
        return (((mTokType == eString) && isPrefixStringCharLiteral(mStr, '"', "U")) ||
                ((mTokType == eChar) && isPrefixStringCharLiteral(mStr, '\'', "U")));
    }

    bool isCChar() const {
        return (((mTokType == eString) && isPrefixStringCharLiteral(mStr, '"', emptyString)) ||
                ((mTokType ==  eChar) && isPrefixStringCharLiteral(mStr, '\'', emptyString) && mStr.length() == 3));
    }

    bool isCMultiChar() const {
        return (((mTokType ==  eChar) && isPrefixStringCharLiteral(mStr, '\'', emptyString)) &&
                (mStr.length() > 3));
    }
    /**
     * @brief Is current token a template argument?
     *
     * Original code:
     *
     *     template<class C> struct S {
     *         C x;
     *     };
     *     S<int> s;
     *
     * Resulting code:
     *
     *     struct S<int> {
     *         int x ;  // <- "int" is a template argument
     *     }
     *     S<int> s;
     */
    bool isTemplateArg() const {
        return getFlag(fIsTemplateArg);
    }
    void isTemplateArg(const bool value) {
        setFlag(fIsTemplateArg, value);
    }

    template<size_t count>
    static const Token *findsimplematch(const Token * const startTok, const char (&pattern)[count]) {
        return findsimplematch(startTok, pattern, count-1);
    }
    static const Token *findsimplematch(const Token * const startTok, const char pattern[], size_t pattern_len);

    template<size_t count>
    static const Token *findsimplematch(const Token * const startTok, const char (&pattern)[count], const Token * const end) {
        return findsimplematch(startTok, pattern, count-1, end);
    }
    static const Token *findsimplematch(const Token * const startTok, const char pattern[], size_t pattern_len, const Token * const end);

    static const Token *findmatch(const Token * const startTok, const char pattern[], const nonneg int varId = 0);
    static const Token *findmatch(const Token * const startTok, const char pattern[], const Token * const end, const nonneg int varId = 0);

    template<size_t count>
    static Token *findsimplematch(Token * const startTok, const char (&pattern)[count]) {
        return findsimplematch(startTok, pattern, count-1);
    }
    static Token *findsimplematch(Token * const startTok, const char pattern[], size_t pattern_len) {
        return const_cast<Token *>(findsimplematch(const_cast<const Token *>(startTok), pattern, pattern_len));
    }
    template<size_t count>
    static Token *findsimplematch(Token * const startTok, const char (&pattern)[count], const Token * const end) {
        return findsimplematch(startTok, pattern, count-1, end);
    }
    static Token *findsimplematch(Token * const startTok, const char pattern[], size_t pattern_len, const Token * const end) {
        return const_cast<Token *>(findsimplematch(const_cast<const Token *>(startTok), pattern, pattern_len, end));
    }

    static Token *findmatch(Token * const startTok, const char pattern[], const nonneg int varId = 0) {
        return const_cast<Token *>(findmatch(const_cast<const Token *>(startTok), pattern, varId));
    }
    static Token *findmatch(Token * const startTok, const char pattern[], const Token * const end, const nonneg int varId = 0) {
        return const_cast<Token *>(findmatch(const_cast<const Token *>(startTok), pattern, end, varId));
    }

private:
    /**
     * Needle is build from multiple alternatives. If one of
     * them is equal to haystack, return value is 1. If there
     * are no matches, but one alternative to needle is empty
     * string, return value is 0. If needle was not found, return
     * value is -1.
     *
     * @param tok Current token (needle)
     * @param haystack e.g. "one|two" or "|one|two"
     * @param varid optional varid of token
     * @return 1 if needle is found from the haystack
     *         0 if needle was empty string
     *        -1 if needle was not found
     */
    static int multiCompare(const Token *tok, const char *haystack, nonneg int varid);

public:
    nonneg int fileIndex() const {
        return mImpl->mFileIndex;
    }
    void fileIndex(nonneg int indexOfFile) {
        mImpl->mFileIndex = indexOfFile;
    }

    nonneg int linenr() const {
        return mImpl->mLineNumber;
    }
    void linenr(nonneg int lineNumber) {
        mImpl->mLineNumber = lineNumber;
    }

    nonneg int column() const {
        return mImpl->mColumn;
    }
    void column(nonneg int c) {
        mImpl->mColumn = c;
    }

    Token* next() {
        return mNext;
    }

    const Token* next() const {
        return mNext;
    }

    /**
     * Delete tokens between begin and end. E.g. if begin = 1
     * and end = 5, tokens 2,3 and 4 would be erased.
     *
     * @param begin Tokens after this will be erased.
     * @param end Tokens before this will be erased.
     */
    static void eraseTokens(Token *begin, const Token *end);

    /**
     * Insert new token after this token. This function will handle
     * relations between next and previous token also.
     * @param tokenStr String for the new token.
     * @param originalNameStr String used for Token::originalName().
     * @param prepend Insert the new token before this token when it's not
     * the first one on the tokens list.
     */
    Token* insertToken(const std::string& tokenStr, const std::string& originalNameStr = emptyString, bool prepend = false);

    Token* insertTokenBefore(const std::string& tokenStr, const std::string& originalNameStr = emptyString)
    {
        return insertToken(tokenStr, originalNameStr, true);
    }

    Token* previous() {
        return mPrevious;
    }

    const Token* previous() const {
        return mPrevious;
    }

    nonneg int varId() const {
        return mImpl->mVarId;
    }
    void varId(nonneg int id) {
        mImpl->mVarId = id;
        if (id != 0) {
            tokType(eVariable);
            isStandardType(false);
        } else {
            update_property_info();
        }
    }

    nonneg int exprId() const {
        if (mImpl->mExprId)
            return mImpl->mExprId;
        return mImpl->mVarId;
    }
    void exprId(nonneg int id) {
        mImpl->mExprId = id;
    }

    void setUniqueExprId()
    {
        assert(mImpl->mExprId > 0);
        mImpl->mExprId |= 1 << efIsUnique;
    }

    bool isUniqueExprId() const
    {
        if (mImpl->mExprId > 0) {
            return (mImpl->mExprId & (1 << efIsUnique)) != 0;
        }
        return false;
    }

    /**
     * For debugging purposes, prints token and all tokens
     * followed by it.
     * @param title Title for the printout or use default parameter or 0
     * for no title.
     */
    void printOut(const char *title = nullptr) const;

    /**
     * For debugging purposes, prints token and all tokens
     * followed by it.
     * @param title Title for the printout or use default parameter or 0
     * for no title.
     * @param fileNames Prints out file name instead of file index.
     * File index should match the index of the string in this vector.
     */
    void printOut(const char *title, const std::vector<std::string> &fileNames) const;

    /**
     * print out tokens - used for debugging
     */
    void printLines(int lines=5) const;

    /**
     * Replace token replaceThis with tokens between start and end,
     * including start and end. The replaceThis token is deleted.
     * @param replaceThis This token will be deleted.
     * @param start This will be in the place of replaceThis
     * @param end This is also in the place of replaceThis
     */
    static void replace(Token *replaceThis, Token *start, Token *end);

    struct stringifyOptions {
        bool varid = false;
        bool exprid = false;
        bool idtype = false; // distinguish varid / exprid
        bool attributes = false;
        bool macro = false;
        bool linenumbers = false;
        bool linebreaks = false;
        bool files = false;
        static stringifyOptions forDebug() {
            stringifyOptions options;
            options.attributes = true;
            options.macro = true;
            options.linenumbers = true;
            options.linebreaks = true;
            options.files = true;
            return options;
        }
        static stringifyOptions forDebugVarId() {
            stringifyOptions options = forDebug();
            options.varid = true;
            return options;
        }
        static stringifyOptions forDebugExprId() {
            stringifyOptions options = forDebug();
            options.exprid = true;
            return options;
        }
        static stringifyOptions forPrintOut() {
            stringifyOptions options = forDebug();
            options.exprid = true;
            options.varid = true;
            options.idtype = true;
            return options;
        }
    };

    std::string stringify(const stringifyOptions& options) const;

    /**
     * Stringify a token
     * @param varid Print varids. (Style: "varname\@id")
     * @param attributes Print attributes of tokens like "unsigned" in front of it.
     * @param macro Prints $ in front of the token if it was expanded from a macro.
     */
    std::string stringify(bool varid, bool attributes, bool macro) const;

    std::string stringifyList(const stringifyOptions& options, const std::vector<std::string>* fileNames = nullptr, const Token* end = nullptr) const;
    std::string stringifyList(const Token* end, bool attributes = true) const;
    std::string stringifyList(bool varid = false) const;

    /**
     * Stringify a list of token, from current instance on.
     * @param varid Print varids. (Style: "varname\@id")
     * @param attributes Print attributes of tokens like "unsigned" in front of it.
     * @param linenumbers Print line number in front of each line
     * @param linebreaks Insert "\\n" into string when line number changes
     * @param files print Files as numbers or as names (if fileNames is given)
     * @param fileNames Vector of filenames. Used (if given) to print filenames as strings instead of numbers.
     * @param end Stringification ends before this token is reached. 0 to stringify until end of list.
     * @return Stringified token list as a string
     */
    std::string stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames = nullptr, const Token* end = nullptr) const;

    /**
     * Remove the contents for this token from the token list.
     *
     * The contents are replaced with the contents of the next token and
     * the next token is unlinked and deleted from the token list.
     *
     * So this token will still be valid after the 'deleteThis()'.
     */
    void deleteThis();

    /**
     * Create link to given token
     * @param linkToToken The token where this token should link
     * to.
     */
    void link(Token *linkToToken) {
        mLink = linkToToken;
        if (mStr == "<" || mStr == ">")
            update_property_info();
    }

    /**
     * Return token where this token links to.
     * Supported links are:
     * "{" <-> "}"
     * "(" <-> ")"
     * "[" <-> "]"
     *
     * @return The token where this token links to.
     */
    const Token* link() const {
        return mLink;
    }

    Token* link() {
        return mLink;
    }

    /**
     * Associate this token with given scope
     * @param s Scope to be associated
     */
    void scope(const Scope *s) {
        mImpl->mScope = s;
    }

    /**
     * @return a pointer to the scope containing this token.
     */
    const Scope *scope() const {
        return mImpl->mScope;
    }

    /**
     * Associate this token with given function
     * @param f Function to be associated
     */
    void function(const Function *f);

    /**
     * @return a pointer to the Function associated with this token.
     */
    const Function *function() const {
        return mTokType == eFunction || mTokType == eLambda ? mImpl->mFunction : nullptr;
    }

    /**
     * Associate this token with given variable
     * @param v Variable to be associated
     */
    void variable(const Variable *v) {
        mImpl->mVariable = v;
        if (v || mImpl->mVarId)
            tokType(eVariable);
        else if (mTokType == eVariable)
            tokType(eName);
    }

    /**
     * @return a pointer to the variable associated with this token.
     */
    const Variable *variable() const {
        return mTokType == eVariable ? mImpl->mVariable : nullptr;
    }

    /**
     * Associate this token with given type
     * @param t Type to be associated
     */
    void type(const ::Type *t);

    /**
     * @return a pointer to the type associated with this token.
     */
    const ::Type *type() const {
        return mTokType == eType ? mImpl->mType : nullptr;
    }

    static const ::Type* typeOf(const Token* tok, const Token** typeTok = nullptr);

    /**
     * @return the declaration associated with this token.
     * @param pointedToType return the pointed-to type?
     */
    static std::pair<const Token*, const Token*> typeDecl(const Token* tok, bool pointedToType = false);

    static std::string typeStr(const Token* tok);

    /**
     * @return a pointer to the Enumerator associated with this token.
     */
    const Enumerator *enumerator() const {
        return mTokType == eEnumerator ? mImpl->mEnumerator : nullptr;
    }

    /**
     * Associate this token with given enumerator
     * @param e Enumerator to be associated
     */
    void enumerator(const Enumerator *e) {
        mImpl->mEnumerator = e;
        if (e)
            tokType(eEnumerator);
        else if (mTokType == eEnumerator)
            tokType(eName);
    }

    /**
     * Links two elements against each other.
     **/
    static void createMutualLinks(Token *begin, Token *end);

    /**
     * This can be called only for tokens that are strings, else
     * the assert() is called. If Token is e.g. '"hello"', this will return
     * 'hello' (removing the double quotes).
     * @return String value
     */
    std::string strValue() const;

    /**
     * Move srcStart and srcEnd tokens and all tokens between them
     * into new a location. Only links between tokens are changed.
     * @param srcStart This is the first token to be moved
     * @param srcEnd The last token to be moved
     * @param newLocation srcStart will be placed after this token.
     */
    static void move(Token *srcStart, Token *srcEnd, Token *newLocation);

    /** Get progressValue (0 - 100) */
    nonneg int progressValue() const {
        return mImpl->mProgressValue;
    }

    /** Calculate progress values for all tokens */
    static void assignProgressValues(Token *tok);

    /**
     * @return the first token of the next argument. Does only work on argument
     * lists. Requires that Tokenizer::createLinks2() has been called before.
     * Returns 0, if there is no next argument.
     */
    const Token* nextArgument() const;
    Token *nextArgument() {
        return const_cast<Token *>(const_cast<const Token *>(this)->nextArgument());
    }

    /**
     * @return the first token of the next argument. Does only work on argument
     * lists. Should be used only before Tokenizer::createLinks2() was called.
     * Returns 0, if there is no next argument.
     */
    const Token* nextArgumentBeforeCreateLinks2() const;

    /**
     * @return the first token of the next template argument. Does only work on template argument
     * lists. Requires that Tokenizer::createLinks2() has been called before.
     * Returns 0, if there is no next argument.
     */
    const Token* nextTemplateArgument() const;

    /**
     * Returns the closing bracket of opening '<'. Should only be used if link()
     * is unavailable.
     * @return closing '>', ')', ']' or '}'. if no closing bracket is found, NULL is returned
     */
    const Token* findClosingBracket() const;
    Token* findClosingBracket();

    const Token* findOpeningBracket() const;
    Token* findOpeningBracket();

    /**
     * @return the original name.
     */
    const std::string & originalName() const {
        return mImpl->mOriginalName ? *mImpl->mOriginalName : emptyString;
    }

    const std::list<ValueFlow::Value>& values() const {
        return mImpl->mValues ? *mImpl->mValues : TokenImpl::mEmptyValueList;
    }

    /**
     * Sets the original name.
     */
    template<typename T>
    void originalName(T&& name) {
        if (!mImpl->mOriginalName)
            mImpl->mOriginalName = new std::string(name);
        else
            *mImpl->mOriginalName = name;
    }

    bool hasKnownIntValue() const;
    bool hasKnownValue() const;
    bool hasKnownValue(ValueFlow::Value::ValueType t) const;
    bool hasKnownSymbolicValue(const Token* tok) const;

    const ValueFlow::Value* getKnownValue(ValueFlow::Value::ValueType t) const;
    MathLib::bigint getKnownIntValue() const {
        return mImpl->mValues->front().intvalue;
    }

    const ValueFlow::Value* getValue(const MathLib::bigint val) const;

    const ValueFlow::Value* getMaxValue(bool condition, MathLib::bigint path = 0) const;
    const ValueFlow::Value* getMinValue(bool condition, MathLib::bigint path = 0) const;

    const ValueFlow::Value* getMovedValue() const;

    const ValueFlow::Value * getValueLE(const MathLib::bigint val, const Settings *settings) const;
    const ValueFlow::Value * getValueGE(const MathLib::bigint val, const Settings *settings) const;

    const ValueFlow::Value * getInvalidValue(const Token *ftok, nonneg int argnr, const Settings *settings) const;

    const ValueFlow::Value* getContainerSizeValue(const MathLib::bigint val) const;

    const Token *getValueTokenMaxStrLength() const;
    const Token *getValueTokenMinStrSize(const Settings *settings, MathLib::bigint* path = nullptr) const;

    /** Add token value. Return true if value is added. */
    bool addValue(const ValueFlow::Value &value);

    void removeValues(std::function<bool(const ValueFlow::Value &)> pred) {
        if (mImpl->mValues)
            mImpl->mValues->remove_if(std::move(pred));
    }

    nonneg int index() const {
        return mImpl->mIndex;
    }

    void assignIndexes();

private:

    void next(Token *nextToken) {
        mNext = nextToken;
    }
    void previous(Token *previousToken) {
        mPrevious = previousToken;
    }

    /** used by deleteThis() to take data from token to delete */
    void takeData(Token *fromToken);

    /**
     * Works almost like strcmp() except returns only true or false and
     * if str has empty space &apos; &apos; character, that character is handled
     * as if it were &apos;\\0&apos;
     */
    static bool firstWordEquals(const char *str, const char *word);

    /**
     * Works almost like strchr() except
     * if str has empty space &apos; &apos; character, that character is handled
     * as if it were &apos;\\0&apos;
     */
    static const char *chrInFirstWord(const char *str, char c);

    std::string mStr;

    Token* mNext{};
    Token* mPrevious{};
    Token* mLink{};

    enum : uint64_t {
        fIsUnsigned             = (1ULL << 0),
        fIsSigned               = (1ULL << 1),
        fIsPointerCompare       = (1ULL << 2),
        fIsLong                 = (1ULL << 3),
        fIsStandardType         = (1ULL << 4),
        fIsExpandedMacro        = (1ULL << 5),
        fIsCast                 = (1ULL << 6),
        fIsAttributeConstructor = (1ULL << 7),  // __attribute__((constructor)) __attribute__((constructor(priority)))
        fIsAttributeDestructor  = (1ULL << 8),  // __attribute__((destructor))  __attribute__((destructor(priority)))
        fIsAttributeUnused      = (1ULL << 9),  // __attribute__((unused))
        fIsAttributePure        = (1ULL << 10), // __attribute__((pure))
        fIsAttributeConst       = (1ULL << 11), // __attribute__((const))
        fIsAttributeNoreturn    = (1ULL << 12), // __attribute__((noreturn)), __declspec(noreturn)
        fIsAttributeNothrow     = (1ULL << 13), // __attribute__((nothrow)), __declspec(nothrow)
        fIsAttributeUsed        = (1ULL << 14), // __attribute__((used))
        fIsAttributePacked      = (1ULL << 15), // __attribute__((packed))
        fIsAttributeExport      = (1ULL << 16), // __attribute__((__visibility__("default"))), __declspec(dllexport)
        fIsAttributeMaybeUnused = (1ULL << 17), // [[maybe_unsed]]
        fIsAttributeNodiscard   = (1ULL << 18), // __attribute__ ((warn_unused_result)), [[nodiscard]]
        fIsControlFlowKeyword   = (1ULL << 19), // if/switch/while/...
        fIsOperatorKeyword      = (1ULL << 20), // operator=, etc
        fIsComplex              = (1ULL << 21), // complex/_Complex type
        fIsEnumType             = (1ULL << 22), // enumeration type
        fIsName                 = (1ULL << 23),
        fIsLiteral              = (1ULL << 24),
        fIsTemplateArg          = (1ULL << 25),
        fAtAddress              = (1ULL << 26), // @ 0x4000
        fIncompleteVar          = (1ULL << 27),
        fConstexpr              = (1ULL << 28),
        fExternC                = (1ULL << 29),
        fIsSplitVarDeclComma    = (1ULL << 30), // set to true when variable declarations are split up ('int a,b;' => 'int a; int b;')
        fIsSplitVarDeclEq       = (1ULL << 31), // set to true when variable declaration with initialization is split up ('int a=5;' => 'int a; a=5;')
        fIsImplicitInt          = (1ULL << 32), // Is "int" token implicitly added?
        fIsInline               = (1ULL << 33), // Is this a inline type
        fIsTemplate             = (1ULL << 34),
        fIsSimplifedScope       = (1ULL << 35), // scope added when simplifying e.g. if (int i = ...; ...)
        fIsRemovedVoidParameter = (1ULL << 36), // A void function parameter has been removed
        fIsIncompleteConstant   = (1ULL << 37),
        fIsRestrict             = (1ULL << 38), // Is this a restrict pointer type
        fIsAtomic               = (1ULL << 39), // Is this a _Atomic declaration
        fIsSimplifiedTypedef    = (1ULL << 40),
        fIsFinalType            = (1ULL << 41), // Is this a type with final specifier
        fIsInitComma            = (1ULL << 42), // Is this comma located inside some {..}. i.e: {1,2,3,4}
    };

    enum : uint64_t {
        efMaxSize = sizeof(nonneg int) * 8,
        efIsUnique = efMaxSize - 2,
    };

    Token::Type mTokType = eNone;

    uint64_t mFlags{};

    TokenImpl* mImpl{};

    /**
     * Get specified flag state.
     * @param flag_ flag to get state of
     * @return true if flag set or false in flag not set
     */
    bool getFlag(uint64_t flag_) const {
        return ((mFlags & flag_) != 0);
    }

    /**
     * Set specified flag state.
     * @param flag_ flag to set state
     * @param state_ new state of flag
     */
    void setFlag(uint64_t flag_, bool state_) {
        mFlags = state_ ? mFlags | flag_ : mFlags & ~flag_;
    }

    /** Updates internal property cache like _isName or _isBoolean.
        Called after any mStr() modification. */
    void update_property_info();

    /** Update internal property cache about isStandardType() */
    void update_property_isStandardType();

    /** Update internal property cache about string and char literals */
    void update_property_char_string_literal();

    /** Internal helper function to avoid excessive string allocations */
    void astStringVerboseRecursive(std::string& ret, const nonneg int indent1 = 0, const nonneg int indent2 = 0) const;

public:
    void astOperand1(Token *tok);
    void astOperand2(Token *tok);
    void astParent(Token* tok);

    Token * astOperand1() {
        return mImpl->mAstOperand1;
    }
    const Token * astOperand1() const {
        return mImpl->mAstOperand1;
    }
    Token * astOperand2() {
        return mImpl->mAstOperand2;
    }
    const Token * astOperand2() const {
        return mImpl->mAstOperand2;
    }
    Token * astParent() {
        return mImpl->mAstParent;
    }
    const Token * astParent() const {
        return mImpl->mAstParent;
    }
    Token * astSibling() {
        if (!astParent())
            return nullptr;
        if (this == astParent()->astOperand1())
            return astParent()->astOperand2();
        if (this == astParent()->astOperand2())
            return astParent()->astOperand1();
        return nullptr;

    }
    const Token * astSibling() const {
        if (!astParent())
            return nullptr;
        if (this == astParent()->astOperand1())
            return astParent()->astOperand2();
        if (this == astParent()->astOperand2())
            return astParent()->astOperand1();
        return nullptr;

    }
    Token *astTop() {
        Token *ret = this;
        while (ret->mImpl->mAstParent)
            ret = ret->mImpl->mAstParent;
        return ret;
    }

    const Token *astTop() const {
        const Token *ret = this;
        while (ret->mImpl->mAstParent)
            ret = ret->mImpl->mAstParent;
        return ret;
    }

    std::pair<const Token *, const Token *> findExpressionStartEndTokens() const;

    /**
     * Is current token a calculation? Only true for operands.
     * For '*' and '&' tokens it is looked up if this is a
     * dereference or address-of. A dereference or address-of is not
     * counted as a calculation.
     * @return returns true if current token is a calculation
     */
    bool isCalculation() const;

    void clearValueFlow() {
        delete mImpl->mValues;
        mImpl->mValues = nullptr;
    }

    std::string astString(const char *sep = "") const {
        std::string ret;
        if (mImpl->mAstOperand1)
            ret = mImpl->mAstOperand1->astString(sep);
        if (mImpl->mAstOperand2)
            ret += mImpl->mAstOperand2->astString(sep);
        return ret + sep + mStr;
    }

    std::string astStringVerbose() const;

    std::string astStringZ3() const;

    std::string expressionString() const;

    void printAst(bool verbose, bool xml, const std::vector<std::string> &fileNames, std::ostream &out) const;

    void printValueFlow(bool xml, std::ostream &out) const;

    void scopeInfo(std::shared_ptr<ScopeInfo2> newScopeInfo);
    std::shared_ptr<ScopeInfo2> scopeInfo() const;

    void setCpp11init(bool cpp11init) const {
        mImpl->mCpp11init=cpp11init ? TokenImpl::Cpp11init::CPP11INIT : TokenImpl::Cpp11init::NOINIT;
    }
    TokenImpl::Cpp11init isCpp11init() const {
        return mImpl->mCpp11init;
    }

    TokenDebug getTokenDebug() const {
        return mImpl->mDebug;
    }
    void setTokenDebug(TokenDebug td) {
        mImpl->mDebug = td;
    }

    /** defaults to C++ if it cannot be determined */
    bool isCpp() const;
};

Token* findTypeEnd(Token* tok);
Token* findLambdaEndScope(Token* tok);
const Token* findLambdaEndScope(const Token* tok);

/// @}
//---------------------------------------------------------------------------
#endif // tokenH

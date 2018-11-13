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
#ifndef tokenH
#define tokenH
//---------------------------------------------------------------------------

#include "config.h"
#include "mathlib.h"
#include "valueflow.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <ostream>
#include <string>
#include <vector>

class Enumerator;
class Function;
class Scope;
class Settings;
class Type;
class ValueType;
class Variable;

/**
 * @brief This struct stores pointers to the front and back tokens of the list this token is in.
 */
struct TokensFrontBack {
    Token *front;
    Token *back;
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
private:
    TokensFrontBack* mTokensFrontBack;

    // Not implemented..
    Token(const Token &);
    Token operator=(const Token &);

public:
    enum Type {
        eVariable, eType, eFunction, eKeyword, eName, // Names: Variable (varId), Type (typeId, later), Function (FuncId, later), Language keyword, Name (unknown identifier)
        eNumber, eString, eChar, eBoolean, eLiteral, eEnumerator, // Literals: Number, String, Character, Boolean, User defined literal (C++11), Enumerator
        eArithmeticalOp, eComparisonOp, eAssignmentOp, eLogicalOp, eBitOp, eIncDecOp, eExtendedOp, // Operators: Arithmetical, Comparison, Assignment, Logical, Bitwise, ++/--, Extended
        eBracket, // {, }, <, >: < and > only if link() is set. Otherwise they are comparison operators.
        eOther,
        eNone
    };

    explicit Token(TokensFrontBack *tokensFrontBack = nullptr);
    ~Token();

    template<typename T>
    void str(T&& s) {
        mStr = s;
        mVarId = 0;

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
     * Unlink and delete the next 'index' tokens.
     */
    void deleteNext(unsigned long index = 1);

    /**
    * Unlink and delete the previous 'index' tokens.
    */
    void deletePrevious(unsigned long index = 1);

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
    static bool simpleMatch(const Token *tok, const char pattern[]);

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
    static bool Match(const Token *tok, const char pattern[], unsigned int varid = 0);

    /**
     * @return length of C-string.
     *
     * Should be called for %%str%% tokens only.
     *
     * @param tok token with C-string
     **/
    static std::size_t getStrLength(const Token *tok);

    /**
     * @return sizeof of C-string.
     *
     * Should be called for %%str%% tokens only.
     *
     * @param tok token with C-string
     **/
    static std::size_t getStrSize(const Token *tok);

    /**
     * @return char of C-string at index (possible escaped "\\n")
     *
     * Should be called for %%str%% tokens only.
     *
     * @param tok token with C-string
     * @param index position of character
     **/
    static std::string getCharAt(const Token *tok, std::size_t index);

    const ValueType *valueType() const {
        return mValueType;
    }
    void setValueType(ValueType *vt);

    const ValueType *argumentType() const {
        const Token *top = this;
        while (top && !Token::Match(top->astParent(), ",|("))
            top = top->astParent();
        return top ? top->mValueType : nullptr;
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
    void isKeyword(const bool kwd) {
        if (kwd)
            tokType(eKeyword);
        else if (mTokType == eKeyword)
            tokType(eName);
    }
    bool isKeyword() const {
        return mTokType == eKeyword;
    }
    bool isName() const {
        return getFlag(fIsName);
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
    bool isBinaryOp() const {
        return astOperand1() != nullptr && astOperand2() != nullptr;
    }
    bool isUnaryOp(const std::string &s) const {
        return s == mStr && astOperand1() != nullptr && astOperand2() == nullptr;
    }
    bool isUnaryPreOp() const;

    unsigned int flags() const {
        return mFlags;
    }
    void flags(const unsigned int flags_) {
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

    bool isBitfield() const {
        return mBits > 0;
    }
    unsigned char bits() const {
        return mBits;
    }
    bool hasTemplateSimplifierPointer() const {
        return getFlag(fHasTemplateSimplifierPointer);
    }
    void hasTemplateSimplifierPointer(const bool value) {
        setFlag(fHasTemplateSimplifierPointer, value);
    }
    void setBits(const unsigned char b) {
        mBits = b;
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

    static const Token *findsimplematch(const Token * const startTok, const char pattern[]);
    static const Token *findsimplematch(const Token * const startTok, const char pattern[], const Token * const end);
    static const Token *findmatch(const Token * const startTok, const char pattern[], const unsigned int varId = 0U);
    static const Token *findmatch(const Token * const startTok, const char pattern[], const Token * const end, const unsigned int varId = 0U);
    static Token *findsimplematch(Token * const startTok, const char pattern[]) {
        return const_cast<Token *>(findsimplematch(const_cast<const Token *>(startTok), pattern));
    }
    static Token *findsimplematch(Token * const startTok, const char pattern[], const Token * const end) {
        return const_cast<Token *>(findsimplematch(const_cast<const Token *>(startTok), pattern, end));
    }
    static Token *findmatch(Token * const startTok, const char pattern[], const unsigned int varId = 0U) {
        return const_cast<Token *>(findmatch(const_cast<const Token *>(startTok), pattern, varId));
    }
    static Token *findmatch(Token * const startTok, const char pattern[], const Token * const end, const unsigned int varId = 0U) {
        return const_cast<Token *>(findmatch(const_cast<const Token *>(startTok), pattern, end, varId));
    }

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
    static int multiCompare(const Token *tok, const char *haystack, unsigned int varid);

    unsigned int fileIndex() const {
        return mFileIndex;
    }
    void fileIndex(unsigned int indexOfFile) {
        mFileIndex = indexOfFile;
    }

    unsigned int linenr() const {
        return mLineNumber;
    }
    void linenr(unsigned int lineNumber) {
        mLineNumber = lineNumber;
    }

    unsigned int col() const {
        return mColumn;
    }
    void col(unsigned int c) {
        mColumn = c;
    }

    Token *next() const {
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
    void insertToken(const std::string &tokenStr, const std::string &originalNameStr=emptyString, bool prepend=false);

    Token *previous() const {
        return mPrevious;
    }


    unsigned int varId() const {
        return mVarId;
    }
    void varId(unsigned int id) {
        mVarId = id;
        if (id != 0) {
            tokType(eVariable);
            isStandardType(false);
        } else {
            update_property_info();
        }
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
     * Replace token replaceThis with tokens between start and end,
     * including start and end. The replaceThis token is deleted.
     * @param replaceThis This token will be deleted.
     * @param start This will be in the place of replaceThis
     * @param end This is also in the place of replaceThis
     */
    static void replace(Token *replaceThis, Token *start, Token *end);

    /**
     * Stringify a token
     * @param os The result is shifted into that output stream
     * @param varid Print varids. (Style: "varname@id")
     * @param attributes Print attributes of tokens like "unsigned" in front of it.
     * @param macro Prints $ in front of the token if it was expanded from a macro.
     */
    void stringify(std::ostream& os, bool varid, bool attributes, bool macro) const;

    /**
     * Stringify a list of token, from current instance on.
     * @param varid Print varids. (Style: "varname@id")
     * @param attributes Print attributes of tokens like "unsigned" in front of it.
     * @param linenumbers Print line number in front of each line
     * @param linebreaks Insert "\\n" into string when line number changes
     * @param files print Files as numbers or as names (if fileNames is given)
     * @param fileNames Vector of filenames. Used (if given) to print filenames as strings instead of numbers.
     * @param end Stringification ends before this token is reached. 0 to stringify until end of list.
     * @return Stringified token list as a string
     */
    std::string stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames = nullptr, const Token* end = nullptr) const;
    std::string stringifyList(const Token* end, bool attributes = true) const;
    std::string stringifyList(bool varid = false) const;

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
    Token *link() const {
        return mLink;
    }

    /**
     * Associate this token with given scope
     * @param s Scope to be associated
     */
    void scope(const Scope *s) {
        mScope = s;
    }

    /**
     * @return a pointer to the scope containing this token.
     */
    const Scope *scope() const {
        return mScope;
    }

    /**
     * Associate this token with given function
     * @param f Function to be associated
     */
    void function(const Function *f) {
        mFunction = f;
        if (f)
            tokType(eFunction);
        else if (mTokType == eFunction)
            tokType(eName);
    }

    /**
     * @return a pointer to the Function associated with this token.
     */
    const Function *function() const {
        return mTokType == eFunction ? mFunction : nullptr;
    }

    /**
     * Associate this token with given variable
     * @param v Variable to be associated
     */
    void variable(const Variable *v) {
        mVariable = v;
        if (v || mVarId)
            tokType(eVariable);
        else if (mTokType == eVariable)
            tokType(eName);
    }

    /**
     * @return a pointer to the variable associated with this token.
     */
    const Variable *variable() const {
        return mTokType == eVariable ? mVariable : nullptr;
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
        return mTokType == eType ? mType : nullptr;
    }

    /**
    * @return a pointer to the Enumerator associated with this token.
    */
    const Enumerator *enumerator() const {
        return mTokType == eEnumerator ? mEnumerator : nullptr;
    }

    /**
     * Associate this token with given enumerator
     * @param e Enumerator to be associated
     */
    void enumerator(const Enumerator *e) {
        mEnumerator = e;
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

    /** Get progressValue */
    unsigned int progressValue() const {
        return mProgressValue;
    }

    /** Calculate progress values for all tokens */
    static void assignProgressValues(Token *tok);

    /**
     * @return the first token of the next argument. Does only work on argument
     * lists. Requires that Tokenizer::createLinks2() has been called before.
     * Returns 0, if there is no next argument.
     */
    Token* nextArgument() const;

    /**
     * @return the first token of the next argument. Does only work on argument
     * lists. Should be used only before Tokenizer::createLinks2() was called.
     * Returns 0, if there is no next argument.
     */
    Token* nextArgumentBeforeCreateLinks2() const;

    /**
    * @return the first token of the next template argument. Does only work on template argument
    * lists. Requires that Tokenizer::createLinks2() has been called before.
    * Returns 0, if there is no next argument.
    */
    Token* nextTemplateArgument() const;

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
        return mOriginalName ? *mOriginalName : emptyString;
    }

    const std::list<ValueFlow::Value>& values() const {
        return mValues ? *mValues : mEmptyValueList;
    }

    /**
     * Sets the original name.
     */
    template<typename T>
    void originalName(T&& name) {
        if (!mOriginalName)
            mOriginalName = new std::string(name);
        else
            *mOriginalName = name;
    }

    bool hasKnownIntValue() const {
        return hasKnownValue() && std::any_of(mValues->begin(), mValues->end(), std::mem_fn(&ValueFlow::Value::isIntValue));
    }

    bool hasKnownValue() const {
        return mValues && std::any_of(mValues->begin(), mValues->end(), std::mem_fn(&ValueFlow::Value::isKnown));
    }

    const ValueFlow::Value * getValue(const MathLib::bigint val) const {
        if (!mValues)
            return nullptr;
        for (const ValueFlow::Value &value : *mValues) {
            if (value.isIntValue() && value.intvalue == val)
                return &value;
        }
        return nullptr;
    }

    const ValueFlow::Value * getMaxValue(bool condition) const {
        if (!mValues)
            return nullptr;
        const ValueFlow::Value *ret = nullptr;
        for (const ValueFlow::Value &value : *mValues) {
            if (!value.isIntValue())
                continue;
            if ((!ret || value.intvalue > ret->intvalue) &&
                ((value.condition != nullptr) == condition))
                ret = &value;
        }
        return ret;
    }

    const ValueFlow::Value * getMovedValue() const {
        if (!mValues)
            return nullptr;
        for (const ValueFlow::Value &value : *mValues) {
            if (value.isMovedValue() && value.moveKind != ValueFlow::Value::NonMovedVariable)
                return &value;
        }
        return nullptr;
    }

    const ValueFlow::Value * getValueLE(const MathLib::bigint val, const Settings *settings) const;
    const ValueFlow::Value * getValueGE(const MathLib::bigint val, const Settings *settings) const;

    const ValueFlow::Value * getInvalidValue(const Token *ftok, unsigned int argnr, const Settings *settings) const;

    const ValueFlow::Value * getContainerSizeValue(const MathLib::bigint val) const {
        if (!mValues)
            return nullptr;
        for (const ValueFlow::Value &value : *mValues) {
            if (value.isContainerSizeValue() && value.intvalue == val)
                return &value;
        }
        return nullptr;
    }

    const Token *getValueTokenMaxStrLength() const;
    const Token *getValueTokenMinStrSize() const;

    const Token *getValueTokenDeadPointer() const;

    /** Add token value. Return true if value is added. */
    bool addValue(const ValueFlow::Value &value);

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

    Token *mNext;
    Token *mPrevious;
    Token *mLink;

    // symbol database information
    const Scope *mScope;
    union {
        const Function *mFunction;
        const Variable *mVariable;
        const ::Type* mType;
        const Enumerator *mEnumerator;
    };

    unsigned int mVarId;
    unsigned int mFileIndex;
    unsigned int mLineNumber;
    unsigned int mColumn;

    /**
     * A value from 0-100 that provides a rough idea about where in the token
     * list this token is located.
     */
    unsigned int mProgressValue;

    Token::Type mTokType;

    enum {
        fIsUnsigned             = (1 << 0),
        fIsSigned               = (1 << 1),
        fIsPointerCompare       = (1 << 2),
        fIsLong                 = (1 << 3),
        fIsStandardType         = (1 << 4),
        fIsExpandedMacro        = (1 << 5),
        fIsCast                 = (1 << 6),
        fIsAttributeConstructor = (1 << 7),  // __attribute__((constructor)) __attribute__((constructor(priority)))
        fIsAttributeDestructor  = (1 << 8),  // __attribute__((destructor))  __attribute__((destructor(priority)))
        fIsAttributeUnused      = (1 << 9),  // __attribute__((unused))
        fIsAttributePure        = (1 << 10), // __attribute__((pure))
        fIsAttributeConst       = (1 << 11), // __attribute__((const))
        fIsAttributeNoreturn    = (1 << 12), // __attribute__((noreturn)), __declspec(noreturn)
        fIsAttributeNothrow     = (1 << 13), // __attribute__((nothrow)), __declspec(nothrow)
        fIsAttributeUsed        = (1 << 14), // __attribute__((used))
        fIsAttributePacked      = (1 << 15), // __attribute__((packed))
        fIsControlFlowKeyword   = (1 << 16), // if/switch/while/...
        fIsOperatorKeyword      = (1 << 17), // operator=, etc
        fIsComplex              = (1 << 18), // complex/_Complex type
        fIsEnumType             = (1 << 19), // enumeration type
        fIsName                 = (1 << 20),
        fIsLiteral              = (1 << 21),
        fIsTemplateArg          = (1 << 22),
        fIsAttributeNodiscard   = (1 << 23), // __attribute__ ((warn_unused_result)), [[nodiscard]]
        fHasTemplateSimplifierPointer = (1 << 24), // used by template simplifier to indicate it has a pointer to this token
    };

    unsigned int mFlags;

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

    /** Updates internal property cache like _isName or _isBoolean.
        Called after any mStr() modification. */
    void update_property_info();

    /** Update internal property cache about isStandardType() */
    void update_property_isStandardType();

    /** Bitfield bit count. */
    unsigned char mBits;

    // AST..
    Token *mAstOperand1;
    Token *mAstOperand2;
    Token *mAstParent;

    // original name like size_t
    std::string* mOriginalName;

    // ValueType
    ValueType *mValueType;

    // ValueFlow
    std::list<ValueFlow::Value>* mValues;
    static const std::list<ValueFlow::Value> mEmptyValueList;

public:
    void astOperand1(Token *tok);
    void astOperand2(Token *tok);

    const Token * astOperand1() const {
        return mAstOperand1;
    }
    const Token * astOperand2() const {
        return mAstOperand2;
    }
    const Token * astParent() const {
        return mAstParent;
    }
    const Token *astTop() const {
        const Token *ret = this;
        while (ret->mAstParent)
            ret = ret->mAstParent;
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

    void clearAst() {
        mAstOperand1 = mAstOperand2 = mAstParent = nullptr;
    }

    void clearValueFlow() {
        delete mValues;
        mValues = nullptr;
    }

    std::string astString(const char *sep = "") const {
        std::string ret;
        if (mAstOperand1)
            ret = mAstOperand1->astString(sep);
        if (mAstOperand2)
            ret += mAstOperand2->astString(sep);
        return ret + sep + mStr;
    }

    std::string astStringVerbose(const unsigned int indent1, const unsigned int indent2) const;

    std::string expressionString() const;

    void printAst(bool verbose, bool xml, std::ostream &out) const;

    void printValueFlow(bool xml, std::ostream &out) const;
};

/// @}
//---------------------------------------------------------------------------
#endif // tokenH

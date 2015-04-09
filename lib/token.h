/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include <list>
#include <string>
#include <vector>
#include <ostream>
#include "config.h"
#include "valueflow.h"
#include "mathlib.h"

class Scope;
class Function;
class Variable;
class Settings;

/// @addtogroup Core
/// @{

/**
 * @brief The token list that the TokenList generates is a linked-list of this class.
 *
 * Tokens are stored as strings. The "if", "while", etc are stored in plain text.
 * The reason the Token class is needed (instead of using the string class) is that some extra functionality is also needed for tokens:
 *  - location of the token is stored (linenr, fileIndex)
 *  - functions for classifying the token (isName, isNumber, isBoolean, isStandardType)
 *
 * The Token class also has other functions for management of token list, matching tokens, etc.
 */
class CPPCHECKLIB Token {
private:
    Token **tokensBack;

    // Not implemented..
    Token();
    Token(const Token &);
    Token operator=(const Token &);

public:
    enum Type {
        eVariable, eType, eFunction, eKeyword, eName, // Names: Variable (varId), Type (typeId, later), Function (FuncId, later), Language keyword, Name (unknown identifier)
        eNumber, eString, eChar, eBoolean, eLiteral, // Literals: Number, String, Character, User defined literal (C++11)
        eArithmeticalOp, eComparisonOp, eAssignmentOp, eLogicalOp, eBitOp, eIncDecOp, eExtendedOp, // Operators: Arithmetical, Comparison, Assignment, Logical, Bitwise, ++/--, Extended
        eBracket, // {, }, <, >: < and > only if link() is set. Otherwise they are comparison operators.
        eOther,
        eNone
    };

    explicit Token(Token **tokensBack);
    ~Token();

    template<typename T>
    void str(T&& s) {
        _str = s;
        _varId = 0;

        update_property_info();
    }

    /**
     * Concatenate two (quoted) strings. Automatically cuts of the last/first character.
     * Example: "hello ""world" -> "hello world". Used by the token simplifier.
     */
    void concatStr(std::string const& b);

    const std::string &str() const {
        return _str;
    }

    /**
     * Unlink and delete the next 'index' tokens.
     */
    void deleteNext(unsigned long index = 1);

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
        return const_cast<Token *>(static_cast<const Token *>(this)->tokAt(index));
    }

    /**
     * @return the link to the token in given index, related to this token.
     * For example index 1 would return the link to next token.
     */
    const Token *linkAt(int index) const;
    Token *linkAt(int index) {
        return const_cast<Token *>(static_cast<const Token *>(this)->linkAt(index));
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
     * - "%name%" any token which is a name, variable or type e.g. "hello" or "int"
     * - "%type%" Anything that can be a variable type, e.g. "int", but not "delete".
     * - "%num%" Any numeric token, e.g. "23"
     * - "%bool%" true or false
     * - "%char%" Any token enclosed in &apos;-character.
     * - "%comp%" Any token such that isComparisonOp() returns true.
     * - "%str%" Any token starting with &quot;-character (C-string).
     * - "%var%" Match with token with varId > 0
     * - "%varid%" Match with parameter varid
     * - "%op%" Any token such that isOp() returns true.
     * - "%cop%" Any token such that isConstOp() returns true.
     * - "%or%" A bitwise-or operator '|'
     * - "%oror%" A logical-or operator '||'
     * - "[abc]" Any of the characters 'a' or 'b' or 'c'
     * - "int|void|char" Any of the strings, int, void or char
     * - "int|void|char|" Any of the strings, int, void or char or empty string
     * - "!!else" No tokens or any token that is not "else".
     * - "someRandomText" If token contains "someRandomText".
     *
     * multi-compare patterns such as "int|void|char" can contain %%or%, %%oror% and %%op%
     * but it is not recommended to put such an %%cmd% as the first pattern.
     *
     * It's possible to use multi-compare patterns with all the other %%cmds%,
     * except for %%varid%, and normal names, but the %%cmds% should be put as
     * the first patterns in the list, then the normal names.
     * For example: "%var%|%num%|)" means yes to a variable, a number or ')'.
     *
     * @todo Make it possible to use the %%cmds% and the normal names in the
     * multicompare list without an order.
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

    Type type() const {
        return _type;
    }
    void type(Type t) {
        _type = t;
    }
    void isKeyword(bool kwd) {
        if (kwd)
            _type = eKeyword;
        else if (_type == eKeyword)
            _type = eName;
    }
    bool isKeyword() const {
        return _type == eKeyword;
    }
    bool isName() const {
        return _type == eName || _type == eType || _type == eVariable || _type == eFunction || _type == eKeyword ||
               _type == eBoolean; // TODO: "true"/"false" aren't really a name...
    }
    bool isUpperCaseName() const;
    bool isLiteral() const {
        return _type == eNumber || _type == eString || _type == eChar ||
               _type == eBoolean || _type == eLiteral;
    }
    bool isNumber() const {
        return _type == eNumber;
    }
    bool isOp() const {
        return (isConstOp() ||
                isAssignmentOp() ||
                _type == eIncDecOp);
    }
    bool isConstOp() const {
        return (isArithmeticalOp() ||
                _type == eLogicalOp ||
                _type == eComparisonOp ||
                _type == eBitOp);
    }
    bool isExtendedOp() const {
        return isConstOp() ||
               _type == eExtendedOp;
    }
    bool isArithmeticalOp() const {
        return _type == eArithmeticalOp;
    }
    bool isComparisonOp() const {
        return _type == eComparisonOp;
    }
    bool isAssignmentOp() const {
        return _type == eAssignmentOp;
    }
    bool isBoolean() const {
        return _type == eBoolean;
    }

    unsigned int flags() const {
        return _flags;
    }
    void flags(unsigned int flags_) {
        _flags = flags_;
    }
    bool isUnsigned() const {
        return getFlag(fIsUnsigned);
    }
    void isUnsigned(bool sign) {
        setFlag(fIsUnsigned, sign);
    }
    bool isSigned() const {
        return getFlag(fIsSigned);
    }
    void isSigned(bool sign) {
        setFlag(fIsSigned, sign);
    }
    bool isPointerCompare() const {
        return getFlag(fIsPointerCompare);
    }
    void isPointerCompare(bool b) {
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
    void isStandardType(bool b) {
        setFlag(fIsStandardType, b);
    }
    bool isExpandedMacro() const {
        return getFlag(fIsExpandedMacro);
    }
    void isExpandedMacro(bool m) {
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
    void isAttributeConstructor(bool ac) {
        setFlag(fIsAttributeConstructor, ac);
    }
    bool isAttributeDestructor() const {
        return getFlag(fIsAttributeDestructor);
    }
    void isAttributeDestructor(bool value) {
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
    void isAttributeUsed(bool unused) {
        setFlag(fIsAttributeUsed, unused);
    }
    bool isAttributePure() const {
        return getFlag(fIsAttributePure);
    }
    void isAttributePure(bool value) {
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
    void isAttributeNoreturn(bool value) {
        setFlag(fIsAttributeNoreturn, value);
    }
    bool isAttributeNothrow() const {
        return getFlag(fIsAttributeNothrow);
    }
    void isAttributeNothrow(bool value) {
        setFlag(fIsAttributeNothrow, value);
    }

    static const Token *findsimplematch(const Token *tok, const char pattern[]);
    static const Token *findsimplematch(const Token *tok, const char pattern[], const Token *end);
    static const Token *findmatch(const Token *tok, const char pattern[], unsigned int varId = 0);
    static const Token *findmatch(const Token *tok, const char pattern[], const Token *end, unsigned int varId = 0);
    static Token *findsimplematch(Token *tok, const char pattern[]) {
        return const_cast<Token *>(findsimplematch(static_cast<const Token *>(tok), pattern));
    }
    static Token *findsimplematch(Token *tok, const char pattern[], const Token *end) {
        return const_cast<Token *>(findsimplematch(static_cast<const Token *>(tok), pattern, end));
    }
    static Token *findmatch(Token *tok, const char pattern[], unsigned int varId = 0) {
        return const_cast<Token *>(findmatch(static_cast<const Token *>(tok), pattern, varId));
    }
    static Token *findmatch(Token *tok, const char pattern[], const Token *end, unsigned int varId = 0) {
        return const_cast<Token *>(findmatch(static_cast<const Token *>(tok), pattern, end, varId));
    }

    /**
     * Needle is build from multiple alternatives. If one of
     * them is equal to haystack, return value is 1. If there
     * are no matches, but one alternative to needle is empty
     * string, return value is 0. If needle was not found, return
     * value is -1.
     *
     * @param needle Current token
     * @param haystack e.g. "one|two" or "|one|two"
     * @param varid optional varid of token
     * @return 1 if needle is found from the haystack
     *         0 if needle was empty string
     *        -1 if needle was not found
     */
    static int multiCompare(const Token *needle, const char *haystack, unsigned int varid);

    unsigned int linenr() const {
        return _linenr;
    }
    void linenr(unsigned int lineNumber) {
        _linenr = lineNumber;
    }

    unsigned int fileIndex() const {
        return _fileIndex;
    }
    void fileIndex(unsigned int indexOfFile) {
        _fileIndex = indexOfFile;
    }

    Token *next() const {
        return _next;
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
     * @param prepend Insert the new token before this token when it's not
     * the first one on the tokens list.
     */
    void insertToken(const std::string &tokenStr, bool prepend=false);

    void insertToken(const std::string &tokenStr, const std::string &originalNameStr, bool prepend=false);

    Token *previous() const {
        return _previous;
    }


    unsigned int varId() const {
        return _varId;
    }
    void varId(unsigned int id) {
        _varId = id;
        if (id != 0)
            _type = eVariable;
        else
            update_property_info();
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
    std::string stringifyList(bool varid, bool attributes, bool linenumbers, bool linebreaks, bool files, const std::vector<std::string>* fileNames = 0, const Token* end = 0) const;
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
        _link = linkToToken;
        if (_str == "<" || _str == ">")
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
        return _link;
    }

    /**
     * Associate this token with given scope
     * @param s Scope to be associated
     */
    void scope(const Scope *s) {
        _scope = s;
    }

    /**
     * @return a pointer to the scope containing this token.
     */
    const Scope *scope() const {
        return _scope;
    }

    /**
     * Associate this token with given function
     * @param f Function to be associated
     */
    void function(const Function *f) {
        _function = f;
        if (f)
            _type = eFunction;
        else if (_type == eFunction)
            _type = eName;
    }

    /**
     * @return a pointer to the Function associated with this token.
     */
    const Function *function() const {
        return _type == eFunction ? _function : 0;
    }

    /**
     * Associate this token with given variable
     * @param v Variable to be associated
     */
    void variable(const Variable *v) {
        _variable = v;
        if (v || _varId)
            _type = eVariable;
        else if (_type == eVariable)
            _type = eName;
    }

    /**
     * @return a pointer to the variable associated with this token.
     */
    const Variable *variable() const {
        return _type == eVariable ? _variable : 0;
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
        return _progressValue;
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

    /**
     * @return the original name.
     */
    const std::string & originalName() const {
        return _originalName ? *_originalName : emptyString;
    }

    /**
     * Sets the original name.
     */
    template<typename T>
    void originalName(T&& name) {
        if (!_originalName)
            _originalName = new std::string(name);
        else
            *_originalName = name;
    }

    /** Values of token */
    std::list<ValueFlow::Value> values;

    const ValueFlow::Value * getValue(const MathLib::bigint val) const {
        std::list<ValueFlow::Value>::const_iterator it;
        for (it = values.begin(); it != values.end(); ++it) {
            if (it->intvalue == val && !it->tokvalue)
                return &(*it);
        }
        return NULL;
    }

    const ValueFlow::Value * getMaxValue(bool condition) const {
        const ValueFlow::Value *ret = nullptr;
        std::list<ValueFlow::Value>::const_iterator it;
        for (it = values.begin(); it != values.end(); ++it) {
            if (it->tokvalue)
                continue;
            if ((!ret || it->intvalue > ret->intvalue) &&
                ((it->condition != NULL) == condition))
                ret = &(*it);
        }
        return ret;
    }

    const ValueFlow::Value * getValueLE(const MathLib::bigint val, const Settings *settings) const;
    const ValueFlow::Value * getValueGE(const MathLib::bigint val, const Settings *settings) const;

    const Token *getValueTokenMaxStrLength() const;
    const Token *getValueTokenMinStrSize() const;

    const Token *getValueTokenDeadPointer() const;

private:

    void next(Token *nextToken) {
        _next = nextToken;
    }
    void previous(Token *previousToken) {
        _previous = previousToken;
    }

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

    std::string _str;

    Token *_next;
    Token *_previous;
    Token *_link;

    // symbol database information
    const Scope *_scope;
    union {
        const Function *_function;
        const Variable *_variable;
    };

    unsigned int _varId;
    unsigned int _fileIndex;
    unsigned int _linenr;

    /**
     * A value from 0-100 that provides a rough idea about where in the token
     * list this token is located.
     */
    unsigned int _progressValue;

    Type _type;

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
        fIsAttributeUsed        = (1 << 14)  // __attribute__((used))
    };

    unsigned int _flags;

    /**
     * Get specified flag state.
     * @param flag_ flag to get state of
     * @return true if flag set or false in flag not set
     */
    bool getFlag(unsigned int flag_) const {
        return bool((_flags & flag_) != 0);
    }

    /**
     * Set specified flag state.
     * @param flag_ flag to set state
     * @param state_ new state of flag
     */
    void setFlag(unsigned int flag_, bool state_) {
        _flags = state_ ? _flags | flag_ : _flags & ~flag_;
    }

    /** Updates internal property cache like _isName or _isBoolean.
        Called after any _str() modification. */
    void update_property_info();

    /** Update internal property cache about isStandardType() */
    void update_property_isStandardType();

    // AST..
    Token *_astOperand1;
    Token *_astOperand2;
    Token *_astParent;

    // original name like size_t
    std::string* _originalName;

public:
    void astOperand1(Token *tok);
    void astOperand2(Token *tok);

    const Token * astOperand1() const {
        return _astOperand1;
    }
    const Token * astOperand2() const {
        return _astOperand2;
    }
    const Token * astParent() const {
        return _astParent;
    }
    const Token *astTop() const {
        const Token *ret = this;
        while (ret->_astParent)
            ret = ret->_astParent;
        return ret;
    }

    /**
     * Is current token a calculation? Only true for operands.
     * For '*' and '&' tokens it is looked up if this is a
     * dereference or address-of. A dereference or address-of is not
     * counted as a calculation.
     * @return returns true if current token is a calculation
     */
    bool isCalculation() const;

    void clearAst() {
        _astOperand1 = _astOperand2 = _astParent = NULL;
    }

    std::string astString(const char *sep = "") const {
        std::string ret;
        if (_astOperand1)
            ret = _astOperand1->astString(sep);
        if (_astOperand2)
            ret += _astOperand2->astString(sep);
        return ret + sep + _str;
    }

    std::string astStringVerbose(const unsigned int indent1, const unsigned int indent2) const;

    std::string expressionString() const;

    void printAst(bool verbose, bool xml, std::ostream &out) const;

    void printValueFlow(bool xml, std::ostream &out) const;
};

/// @}
//---------------------------------------------------------------------------
#endif // tokenH

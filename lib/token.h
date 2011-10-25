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

#ifndef TokenH
#define TokenH

#include <string>
#include <vector>

/// @addtogroup Core
/// @{

/**
 * @brief The token list that the Tokenizer generates is a linked-list of this class.
 *
 * Tokens are stored as strings. The "if", "while", etc are stored in plain text.
 * The reason the Token class is needed (instead of using the string class) is that some extra functionality is also needed for tokens:
 *  - location of the token is stored (linenr, fileIndex)
 *  - functions for classifying the token (isName, isNumber, isBoolean, isStandardType)
 *
 * The Token class also has other functions for management of token list, matching tokens, etc.
 */
class Token {
private:
    Token **tokensBack;

    // Not implemented..
    Token();

public:
    Token(Token **tokensBack);
    ~Token();

    void str(const std::string &s);

    /**
     * Concatenate two (quoted) strings. Automatically cuts of the last/first character.
     * Example: "hello ""world" -> "hello world". Used by the token simplifier.
     */
    void concatStr(std::string const& b);

    const std::string &str() const {
        return _str;
    }

    /**
     * Unlink and delete next token.
     */
    void deleteNext();

    /**
     * Returns token in given index, related to this token.
     * For example index 1 would return next token, and 2
     * would return next from that one.
     */
    const Token *tokAt(int index) const;
    Token *tokAt(int index);

    std::string strAt(int index) const;

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
     * - "%var%" any token which is a name or type e.g. "hello" or "int"
     * - "%type%" Anything that can be a variable type, e.g. "int", but not "delete".
     * - "%num%" Any numeric token, e.g. "23"
     * - "%bool%" true or false
     * - "%str%" Any token starting with &quot;-character (C-string).
     * - "%varid%" Match with parameter varid
     * - "%or%" A bitwise-or operator '|'
     * - "%oror%" A logical-or operator '||'
     * - "[abc]" Any of the characters 'a' or 'b' or 'c'
     * - "int|void|char" Any of the strings, int, void or char
     * - "int|void|char|" Any of the strings, int, void or char or empty string
     * - "!!else" No tokens or any token that is not "else".
     * - "someRandomText" If token contains "someRandomText".
     *
     * The patterns can be also combined to compare to multiple tokens at once
     * by separating tokens with a space, e.g.
     * ") const|void {" will return true if first token is ')' next token is either
     * "const" or "void" and token after that is '{'. If even one of the tokens does not
     * match its pattern, false is returned.
     *
     * @todo pattern "%type%|%num%" should mean either a type or a num.
     *
     * @param tok List of tokens to be compared to the pattern
     * @param pattern The pattern against which the tokens are compared,
     * e.g. "const" or ") const|volatile| {".
     * @param varid if %varid% is given in the pattern the Token::varId will be matched against this argument
     * @return true if given token matches with given pattern
     *         false if given token does not match with given pattern
     */
    static bool Match(const Token *tok, const char pattern[], unsigned int varid = 0);

    /**
     * Return length of C-string.
     *
     * Should be called for %str% tokens only.
     *
     * @param tok token with C-string
     **/
    static size_t getStrLength(const Token *tok);

    bool isName() const {
        return _isName;
    }
    void isName(bool name) {
        _isName = name;
    }
    bool isNumber() const {
        return _isNumber;
    }
    void isNumber(bool number) {
        _isNumber = number;
    }
    bool isArithmeticalOp() const {
        return (this && (_str=="<<" || _str==">>" || (_str.size()==1 && _str.find_first_of("+-*/%") != std::string::npos)));
    }
    bool isOp() const {
        if (!this)
            return false;

        return (isArithmeticalOp() ||
                _str == "&&" ||
                _str == "||" ||
                _str == "==" ||
                _str == "!=" ||
                _str == "<"  ||
                _str == "<=" ||
                _str == ">"  ||
                _str == ">=" ||
                (_str.size() == 1 && _str.find_first_of("&|^~!") != std::string::npos));
    }
    bool isExtendedOp() const {
        return isOp() ||
               (this && _str.size() == 1 && _str.find_first_of(",[]()?:") != std::string::npos);
    }
    bool isAssignmentOp() const {
        if (!this)
            return false;

        return (_str == "="   ||
                _str == "+="  ||
                _str == "-="  ||
                _str == "*="  ||
                _str == "/="  ||
                _str == "%="  ||
                _str == "&="  ||
                _str == "^="  ||
                _str == "|="  ||
                _str == "<<=" ||
                _str == ">>=");
    }
    bool isBoolean() const {
        return _isBoolean;
    }
    void isBoolean(bool boolean) {
        _isBoolean = boolean;
    }
    bool isUnsigned() const {
        return _isUnsigned;
    }
    void isUnsigned(bool sign) {
        _isUnsigned = sign;
    }
    bool isSigned() const {
        return _isSigned;
    }
    void isSigned(bool sign) {
        _isSigned = sign;
    }
    bool isPointerCompare() const {
        return _isPointerCompare;
    }
    void isPointerCompare(bool b) {
        _isPointerCompare = b;
    }
    bool isLong() const {
        return _isLong;
    }
    void isLong(bool size) {
        _isLong = size;
    }
    bool isUnused() const {
        return _isUnused;
    }
    void isUnused(bool used) {
        _isUnused = used;
    }
    bool isStandardType() const;

    static const Token *findmatch(const Token *tok, const char pattern[], unsigned int varId = 0);
    static const Token *findmatch(const Token *tok, const char pattern[], const Token *end, unsigned int varId = 0);

    /**
     * Needle is build from multiple alternatives. If one of
     * them is equal to haystack, return value is 1. If there
     * are no matches, but one alternative to needle is empty
     * string, return value is 0. If needle was not found, return
     * value is -1.
     *
     * @param haystack e.g. "one|two" or "|one|two"
     * @param needle e.g. "one", "two" or "invalid"
     * @return 1 if needle is found from the haystack
     *         0 if needle was empty string
     *        -1 if needle was not found
     */
    static int multiCompare(const char *haystack, const char *needle);

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
     */
    void insertToken(const std::string &tokenStr);

    Token *previous() const {
        return _previous;
    }


    unsigned int varId() const {
        return _varId;
    }
    void varId(unsigned int id) {
        _varId = id;
    }

    /**
     * For debugging purposes, prints token and all tokens
     * followed by it.
     * @param title Title for the printout or use default parameter or 0
     * for no title.
     */
    void printOut(const char *title = 0) const;

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

    /** Stringify a token list (with or without varId) */
    std::string stringifyList(bool varid = false, const char *title = 0) const;
    std::string stringifyList(bool varid, const char *title, const std::vector<std::string> &fileNames) const;

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
     * Move srcStart and srcEnd tokens and all tokens between then
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
    void assignProgressValues() {
        unsigned int total_count = 0;
        for (Token *tok = this; tok; tok = tok->next())
            ++total_count;
        unsigned int count = 0;
        for (Token *tok = this; tok; tok = tok->next())
            tok->_progressValue = count++ * 100 / total_count;
    }

    /**
     * Returns the first token of the next argument. Does only work on argument
     * lists. Returns 0, if there is no next argument
     */
    const Token* nextArgument() const;

private:
    void next(Token *nextToken) {
        _next = nextToken;
    }
    void previous(Token *previousToken) {
        _previous = previousToken;
    }

    /**
     * Works almost like strcmp() except returns only 0 or 1 and
     * if str has empty space &apos; &apos; character, that character is handled
     * as if it were &apos;\\0&apos;
     */
    static int firstWordEquals(const char *str, const char *word);

    /**
     * Works almost like strchr() except
     * if str has empty space &apos; &apos; character, that character is handled
     * as if it were &apos;\\0&apos;
     */
    static const char *chrInFirstWord(const char *str, char c);

    /**
     * Works almost like strlen() except
     * if str has empty space &apos; &apos; character, that character is handled
     * as if it were &apos;\\0&apos;
     */
    static int firstWordLen(const char *str);


    Token *_next;
    Token *_previous;
    Token *_link;

    bool _isName;
    bool _isNumber;
    bool _isBoolean;
    bool _isUnsigned;
    bool _isSigned;
    bool _isPointerCompare;
    bool _isLong;
    bool _isUnused;
    unsigned int _varId;
    unsigned int _fileIndex;
    unsigned int _linenr;

    /** Updates internal property cache like _isName or _isBoolean.
        Called after any _str() modification. */
    void update_property_info();

    /**
     * A value from 0-100 that provides a rough idea about where in the token
     * list this token is located.
     */
    unsigned int _progressValue;

    std::string _str;
};

/// @}

#endif // TokenH

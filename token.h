/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <string>

class TOKEN
{
public:
    TOKEN();
    ~TOKEN();
    void setstr( const char s[] );

    const std::string &str() const
    { return _str; }

    const char *aaaa() const
    { return _cstr; }

    char aaaa0() const
    { return _cstr[0]; }

    char aaaa1() const
    { return _cstr[1]; }

    /**
     * Unlink and delete next token.
     */
    void deleteNext();

    /**
     * Returns token in given index, related to this token.
     * For example index 1 would return next token, and 2
     * would return next from that one.
     */
    const TOKEN *tokAt(int index) const;

    const char *strAt(int index) const;

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
    static bool simpleMatch(const TOKEN *tok, const char pattern[]);

    /**
     * Match given token (or list of tokens) to a pattern list.
     *
     * Possible patterns
     * "%any%" any token
     * "%var%" any token which is a name or type e.g. "hello" or "int"
     * "%num%" Any numeric token, e.g. "23"
     * "%bool%" true or false
     * "%str%" Any token starting with "-character (C-string).
     * "%var1%" Match with parameter varname1
     * "%varid%" Match with parameter varid
     * "[abc]" Any of the characters 'a' or 'b' or 'c'
     * "int|void|char" Any of the strings, int, void or char
     * "int|void|char|" Any of the strings, int, void or char or empty string
     * "!!else" No tokens or any token that is not "else".
     * "someRandomText" If token contains "someRandomText".
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
     * @param varname1 Used with pattern "%var1%"
     * @return true if given token matches with given pattern
     *         false if given token does not match with given pattern
     */
    static bool Match(const TOKEN *tok, const char pattern[], const char *varname1[]=0, unsigned int varid=0);

    bool isName() const;
    bool isNumber() const;
    bool isBoolean() const;
    bool isStandardType() const;
    static const TOKEN *findmatch(const TOKEN *tok, const char pattern[], const char *varname1[]=0);

    /**
     * Needle is build from multiple alternatives. If one of
     * them is equal to haystack, return value is 1. If there
     * are no matches, but one alternative to needle is empty
     * string, return value is 0. If needle was not found, return
     * value is -1.
     *
     * @param needle e.g. "one|two" or "|one|two"
     * @param haystack e.g. "one", "two" or "invalid"
     * @return 1 if needle is found from the haystack
     *         0 if needle was empty string
     *        -1 if needle was not found
     */
    static int multiCompare( const char *needle, const char *haystack );


    unsigned int linenr() const;
    void linenr( unsigned int linenr );

    unsigned int fileIndex() const;
    void fileIndex( unsigned int fileIndex );

    TOKEN *next() const;


    /**
     * Delete tokens between begin and end. E.g. if begin = 1
     * and end = 5, tokens 2,3 and 4 would be erased.
     *
     * @param begin Tokens after this will be erased.
     * @param end Tokens before this will be erased.
     */
    static void eraseTokens( TOKEN *begin, const TOKEN *end );

    /**
     * Insert new token after this token. This function will handle
     * relations between next and previous token also.
     * @param str String for the new token.
     */
    void insertToken( const char str[] );

    TOKEN *previous() const;


    unsigned int varId() const;
    void varId( unsigned int id );

    /**
     * For debugging purposes, prints token and all tokens
     * followed by it.
     * @param title Title for the printout or use default parameter or 0
     * for no title.
     */
    void printOut( const char *title = 0 ) const;

private:
    void next( TOKEN *next );
    void previous( TOKEN *previous );

    std::string _str;
    char * _cstr;
    bool _isName;
    bool _isNumber;
    bool _isBoolean;
    unsigned int _varId;
    TOKEN *_next;
    TOKEN *_previous;
    unsigned int _fileIndex;
    unsigned int _linenr;
};

#endif // TOKEN_H

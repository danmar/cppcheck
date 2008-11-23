/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki and Reijo Tomperi
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

class TOKEN
{
public:
    TOKEN();
    ~TOKEN();
    void setstr( const char s[] );

    /**
     * Combine two tokens that belong to each other.
     * Ex: "<" and "=" may become "<="
     */
    void combineWithNext(const char str1[], const char str2[]);

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

    static bool Match(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);
    bool isName() const;
    bool isNumber() const;
    static bool IsStandardType(const char str[]);
    static const TOKEN *findmatch(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);
    static const TOKEN *findtoken(const TOKEN *tok1, const char *tokenstr[]);
    const char *str;
    unsigned int FileIndex;
    unsigned int linenr;
    TOKEN *next;

private:
    char * _str;
    bool _isName;
    bool _isNumber;
};

#endif // TOKEN_H

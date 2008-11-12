/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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


//---------------------------------------------------------------------------
#ifndef tokenizeH
#define tokenizeH
//---------------------------------------------------------------------------

#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

extern std::vector<std::string> Files;


class TOKEN
{
private:
    char * _str;

public:
    TOKEN()
    { FileIndex = 0; _str = 0; linenr = 0; next = 0; }

    ~TOKEN()
    { std::free(_str); }

    void setstr( const char s[] )
    {
        std::free(_str);
#ifndef _MSC_VER
        _str = strdup(s);
#else
        _str = _strdup(s);
#endif
        str = _str ? _str : "";
    }

    const char *str;

    unsigned int FileIndex;
    unsigned int linenr;
    TOKEN *next;
};

extern TOKEN *tokens, *tokens_back;

class Tokenizer
{
public:

    void Tokenize(std::istream &code, const char FileName[]);

    // Deallocate lists..
    void DeallocateTokens();

    // Simplify tokenlist
    // -----------------------------
    void SimplifyTokenList();

    void TokenizeCode(std::istream &code, const unsigned int FileIndex=0);

    // Helper functions for handling the tokens list..
    static const TOKEN *findtoken(const TOKEN *tok1, const char *tokenstr[]);
    static const TOKEN *gettok(const TOKEN *tok, int index);
    static const char *getstr(const TOKEN *tok, int index);

    // Return size.
    static int SizeOfType(const char type[]);

    std::vector<std::string> _files;
    TOKEN *_tokens;
    TOKEN *_tokens_back;
private:




    static void Define(const char Name[], const char Value[]);

    static void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

    static void combine_2tokens(TOKEN *tok, const char str1[], const char str2[]);

    static void DeleteNextToken(TOKEN *tok);

    static TOKEN *_gettok(TOKEN *tok, int index);

    static void InsertTokens(TOKEN *dest, TOKEN *src, unsigned int n);
};





//---------------------------------------------------------------------------
#endif


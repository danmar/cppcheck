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
#include <list>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include <cstring>
#include "settings.h"
#include "errorlogger.h"

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

class Tokenizer
{
private:
    // Deallocate lists..
    void DeallocateTokens();

public:
    Tokenizer();
    ~Tokenizer();

    void Tokenize(std::istream &code, const char FileName[]);

    // Simplify tokenlist
    // -----------------------------
    void SimplifyTokenList();

    void TokenizeCode(std::istream &code, const unsigned int FileIndex=0);

    // Helper functions for handling the tokens list..
    static const TOKEN *findtoken(const TOKEN *tok1, const char *tokenstr[]);
    static const TOKEN *gettok(const TOKEN *tok, int index);
    static const char *getstr(const TOKEN *tok, int index);
    static void deleteTokens(TOKEN *tok);
    static const char *getParameterName( const TOKEN *ftok, int par );
    static const TOKEN *findmatch(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);
    static bool Match(const TOKEN *tok, const char pattern[], const char *varname1[]=0, const char *varname2[]=0);
    static bool SameFileName( const char fname1[], const char fname2[] );
    static bool IsName(const char str[]);
    static bool IsNumber(const char str[]);
    static bool IsStandardType(const char str[]);

    std::string fileLine( const TOKEN *tok ) const;

    // Return size.
    int SizeOfType(const char type[]) const;

    void initTokens();

    std::vector<std::string> *getFiles();

    void FillFunctionList(const unsigned int file_id);
    const TOKEN *GetFunctionTokenByName( const char funcname[] ) const;
    void settings( const Settings &settings );
    const TOKEN *tokens() const;


#ifndef UNIT_TESTING
private:
#endif

    struct DefineSymbol
    {
        char *name;
        char *value;
        struct DefineSymbol *next;
    };

    void Define(const char Name[], const char Value[]);

    void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

    void combine_2tokens(TOKEN *tok, const char str1[], const char str2[]);

    void DeleteNextToken(TOKEN *tok);

    bool simplifyConditions();

    TOKEN *_gettok(TOKEN *tok, int index);

    void InsertTokens(TOKEN *dest, TOKEN *src, unsigned int n);

    TOKEN *tokens_back;
    std::map<std::string, unsigned int> TypeSize;
    std::vector<const TOKEN *> FunctionList;
    std::vector<std::string> Files;
    Settings _settings;


    struct DefineSymbol * dsymlist;
    TOKEN *_tokens;
};





//---------------------------------------------------------------------------
#endif


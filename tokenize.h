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
#include "settings.h"
#include "errorlogger.h"
#include "token.h"

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

    static void deleteTokens(TOKEN *tok);
    static const char *getParameterName( const TOKEN *ftok, int par );

    static bool SameFileName( const char fname1[], const char fname2[] );


    std::string fileLine( const TOKEN *tok ) const;

    // Return size.
    int SizeOfType(const char type[]) const;

    void initTokens();

    const std::vector<std::string> *getFiles() const;

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

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
public:
    Tokenizer();
    ~Tokenizer();

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
    int SizeOfType(const char type[]);

    void initTokens();

    std::vector<std::string> *getFiles();



    void FillFunctionList(const unsigned int file_id);
    const TOKEN *GetFunctionTokenByName( const char funcname[] ) const;
    void CheckGlobalFunctionUsage(const std::vector<std::string> &filenames);
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

    class GlobalFunction
    {
    private:
        unsigned int _FileId;
        std::string  _FuncName;

    public:
        GlobalFunction( const unsigned int FileId, const char FuncName[] )
        {
            _FileId = FileId;
            _FuncName = FuncName;
        }

        unsigned int file_id() const { return _FileId; }
        const std::string &name() const { return _FuncName; }
    };

    void Define(const char Name[], const char Value[]);

    void addtoken(const char str[], const unsigned int lineno, const unsigned int fileno);

    void combine_2tokens(TOKEN *tok, const char str1[], const char str2[]);

    void DeleteNextToken(TOKEN *tok);

    TOKEN *_gettok(TOKEN *tok, int index);

    void InsertTokens(TOKEN *dest, TOKEN *src, unsigned int n);

    TOKEN *tokens_back;
    std::map<std::string, unsigned int> TypeSize;
    std::vector<const TOKEN *> FunctionList;
    std::list< GlobalFunction > GlobalFunctions;
    std::list< GlobalFunction > UsedGlobalFunctions;
    std::vector<std::string> Files;
    Settings _settings;


    struct DefineSymbol * dsymlist;
    TOKEN *_tokens;
};





//---------------------------------------------------------------------------
#endif


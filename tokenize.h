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

    /**
     * Helper function for "tokenize". This recursively parses into included header files.
     */
    void tokenizeCode(std::istream &code, const unsigned int FileIndex=0);

public:
    Tokenizer();
    ~Tokenizer();

    /**
     * Tokenize code
     * @param code input stream for code
     * @param FileName The filename
     */
    void tokenize(std::istream &code, const char FileName[]);

    /** Set variable id */
    void setVarId();

    /** Simplify tokenlist */
    void simplifyTokenList();


    // Helper functions for handling the tokens list..

    static void deleteTokens(TOKEN *tok);
    static const char *getParameterName( const TOKEN *ftok, int par );

    static bool SameFileName( const char fname1[], const char fname2[] );


    std::string fileLine( const TOKEN *tok ) const;

    // Return size.
    int SizeOfType(const char type[]) const;

    void initTokens();

    const std::vector<std::string> *getFiles() const;

    void fillFunctionList();
    const TOKEN *GetFunctionTokenByName( const char funcname[] ) const;
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

    /** Simplify conditions
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyConditions();

    /** Simplify casts
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyCasts();

    /** Simplify function calls - constant return value
     * @return true if something is modified
     *         false if nothing is done.
     */
    bool simplifyFunctionReturn();

    /**
     * A simplify function that replaces a variable with its value in cases
     * when the value is known. e.g. "x=10; if(x)" => "x=10;if(10)"
     *
     * @param token The token list to check and modify.
     * @return true if modifications to token-list are done.
     *         false if no modifications are done.
     */
    bool simplifyKnownVariables();

    TOKEN *_gettok(TOKEN *tok, int index);

    void InsertTokens(TOKEN *dest, TOKEN *src, unsigned int n);

    TOKEN *_tokensBack;
    std::map<std::string, unsigned int> _typeSize;
    std::vector<const TOKEN *> _functionList;
    std::vector<std::string> _files;
    struct DefineSymbol * _dsymlist;
    TOKEN *_tokens;
};

//---------------------------------------------------------------------------
#endif

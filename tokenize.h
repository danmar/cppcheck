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

#ifdef __BORLANDC__
#include <stdlib.h>     // <- free
#endif

extern std::vector<std::string> Files;

class TOKEN
{
private:
    char * _str;

public:
    TOKEN()
    { FileIndex = 0; _str = 0; linenr = 0; next = 0; }

    ~TOKEN()
    { free(_str); }

    void setstr( const char s[] )
    { 
        free(_str);
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


void Tokenize(std::istream &code, const char FileName[]);

void TokenizeCode(std::istream &code, const unsigned int FileIndex=0);

// Return size.
int SizeOfType(const char type[]);

// Simplify tokenlist
// -----------------------------
void SimplifyTokenList();


// Deallocate lists..
void DeallocateTokens();


// Helper functions for handling the tokens list..
const TOKEN *findtoken(const TOKEN *tok1, const char *tokenstr[]);
const TOKEN *gettok(const TOKEN *tok, int index);
const char *getstr(const TOKEN *tok, int index);


//---------------------------------------------------------------------------
#endif


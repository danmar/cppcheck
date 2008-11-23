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

#include "token.h"
#include <cstdlib>
#include <cstring>
#include <string>

TOKEN::TOKEN()
{
    FileIndex = 0;
    _str = 0;
    linenr = 0;
    next = 0;
    _isName = false;
    _isNumber = false;
}

TOKEN::~TOKEN()
{
    std::free(_str);
}

void TOKEN::setstr( const char s[] )
{
    std::free(_str);
#ifndef _MSC_VER
    _str = strdup(s);
#else
    _str = _strdup(s);
#endif
    str = _str ? _str : "";

    _isName = bool(str[0]=='_' || isalpha(str[0]));
    _isNumber = bool(isdigit(str[0]) != 0);
}

void TOKEN::combineWithNext(const char str1[], const char str2[])
{
    if (!(next))
        return;
    if (strcmp(str,str1) || strcmp(next->str,str2))
        return;

	std::string newstr(std::string(str1) + std::string(str2));
	setstr( newstr.c_str() );
    deleteNext();
}

void TOKEN::deleteNext()
{
    TOKEN *n = next;
    next = n->next;
    delete n;
}

const TOKEN *TOKEN::tokAt(int index) const
{
    const TOKEN *tok = this;
    while (index>0 && tok)
    {
        tok = tok->next;
        index--;
    }
    return tok;
}

const char *TOKEN::strAt(int index) const
{
    const TOKEN *tok = this->tokAt(index);
    return tok ? tok->str : "";
}

bool TOKEN::Match(const TOKEN *tok, const char pattern[], const char *varname1[], const char *varname2[])
{
    if (!tok)
        return false;

    const char *p = pattern;
    while (*p)
    {
        // Skip spaces in pattern..
        while ( *p == ' ' )
            p++;

        // Extract token from pattern..
        char str[50];
        char *s = str;
        while (*p && *p!=' ')
        {
            *s = *p;
            s++;
            p++;
        }
        *s = 0;

        // No token => Success!
        if (str[0] == 0)
            return true;

        // Any symbolname..
        if (strcmp(str,"%var%")==0 || strcmp(str,"%type%")==0)
        {
            if (!tok->isName())
                return false;
        }

        // Variable name..
        else if (strcmp(str,"%var1%")==0 || strcmp(str,"%var2%")==0)
        {
            const char **varname = (strcmp(str,"%var1%")==0) ? varname1 : varname2;

            if ( ! varname )
                return false;

            if (strcmp(tok->str, varname[0]) != 0)
                return false;

            for ( int i = 1; varname[i]; i++ )
            {
                if ( !(tok->tokAt(2)) )
                    return false;

                if ( strcmp(tok->strAt( 1), ".") )
                    return false;

                if ( strcmp(tok->strAt( 2), varname[i]) )
                    return false;

                tok = tok->tokAt(2);
            }
        }

        else if (strcmp(str,"%num%")==0)
        {
            if ( ! tok->isNumber() )
                return false;
        }


        else if (strcmp(str,"%str%")==0)
        {
            if ( tok->str[0] != '\"' )
                return false;
        }

        // [.. => search for a one-character token..
        else if (str[0]=='[' && strchr(str, ']') && tok->str[1] == 0)
        {
            *strrchr(str, ']') = 0;
            if ( strchr( str + 1, tok->str[0] ) == 0 )
                return false;
        }

        else if (strcmp(str, tok->str) != 0)
            return false;

        tok = tok->next;
        if (!tok && *p)
            return false;
    }

    // The end of the pattern has been reached and nothing wrong has been found
    return true;
}

bool TOKEN::isName() const
{
    return _isName;
}

bool TOKEN::isNumber() const
{
    return _isNumber;
}


bool TOKEN::IsStandardType(const char str[])
{
    if (!str)
        return false;
    bool Ret = false;
    const char *type[] = {"bool","char","short","int","long","float","double",0};
    for (int i = 0; type[i]; i++)
        Ret |= (strcmp(str,type[i])==0);
    return Ret;
}

//---------------------------------------------------------------------------

const TOKEN *TOKEN::findmatch(const TOKEN *tok, const char pattern[], const char *varname1[], const char *varname2[])
{
    for ( ; tok; tok = tok->next)
    {
        if ( TOKEN::Match(tok, pattern, varname1, varname2) )
            return tok;
    }
    return 0;
}

const TOKEN *TOKEN::findtoken(const TOKEN *tok1, const char *tokenstr[])
{
    for (const TOKEN *ret = tok1; ret; ret = ret->next)
    {
        unsigned int i = 0;
        const TOKEN *tok = ret;
        while (tokenstr[i])
        {
            if (!tok)
                return NULL;
            if (*(tokenstr[i]) && strcmp(tokenstr[i],tok->str))
                break;
            tok = tok->next;
            i++;
        }
        if (!tokenstr[i])
            return ret;
    }
    return NULL;
}

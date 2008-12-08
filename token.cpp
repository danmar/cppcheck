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

#ifdef __BORLANDC__
#include <ctype.h>  // isalpha, isdigit
#endif

TOKEN::TOKEN()
{
    _fileIndex = 0;
    _cstr = 0;
    _str = "";
    _linenr = 0;
    _next = 0;
    _varId = 0;
    _isName = false;
    _isNumber = false;
}

TOKEN::~TOKEN()
{
    std::free(_cstr);
}

void TOKEN::setstr( const char s[] )
{
    _str = s;
    std::free(_cstr);
#ifndef _MSC_VER
    _cstr = strdup(s);
#else
    _cstr = _strdup(s);
#endif
    _isName = bool(_str[0]=='_' || isalpha(_str[0]));
    _isNumber = bool(isdigit(_str[0]) != 0);
}

void TOKEN::combineWithNext(const char str1[], const char str2[])
{
    if (!(_next))
        return;
    if (_str!=str1 || _next->_str!=str2)
        return;

	std::string newstr(std::string(str1) + std::string(str2));
	setstr( newstr.c_str() );
    deleteNext();
}

void TOKEN::deleteNext()
{
    TOKEN *n = _next;
    _next = n->next();
    delete n;
}

const TOKEN *TOKEN::tokAt(int index) const
{
    const TOKEN *tok = this;
    while (index>0 && tok)
    {
        tok = tok->next();
        index--;
    }
    return tok;
}

const char *TOKEN::strAt(int index) const
{
    const TOKEN *tok = this->tokAt(index);
    return tok ? tok->_cstr : "";
}

int TOKEN::multiCompare( const char *needle, const char *haystack )
{
    bool emptyStringFound = false;
    bool findNextOr = false;
    const char *haystackPointer = haystack;
    for( ; *needle; ++needle )
    {
        if( *needle == '|' )
        {
            // If needle and haystack are both at the end, we have a match.
            if( *haystackPointer == 0 )
                return 1;

            haystackPointer = haystack;
            if( findNextOr )
                findNextOr = false;
            else
                emptyStringFound = true;

            continue;
        }

        if( findNextOr )
            continue;

        // If haystack and needle don't share the same character, reset
        // haystackpointer and find next '|' character.
        if( *haystackPointer != *needle )
        {
            haystackPointer = haystack;
            findNextOr = true;
            continue;
        }

        // All characters in haystack and needle have matched this far
        haystackPointer++;
    }

    // If both needle and haystack are at the end, then we have a match.
    if( *haystackPointer == 0 )
        return 1;

    // If empty string was found or if last character in needle was '|'
    if( emptyStringFound || findNextOr == false )
        return 0;

    return -1;
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
        // TODO: Refactor this so there can't be buffer overflows
        char str[500];
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

		bool useVar1;
        // Any symbolname..
        if (strcmp(str,"%var%")==0 || strcmp(str,"%type%")==0)
        {
            if (!tok->isName())
                return false;
        }

        // Accept any token
        else if (strcmp(str,"%any%")==0 )
        {

        }

        // Variable name..
        else if ((useVar1 = (strcmp(str,"%var1%")==0)) || strcmp(str,"%var2%")==0)
        {
            const char **varname = useVar1 ? varname1 : varname2;

            if ( ! varname )
                return false;

            if (tok->_str != varname[0])
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
            if ( tok->_str[0] != '\"' )
                return false;
        }

        // [.. => search for a one-character token..
        else if (str[0]=='[' && strchr(str, ']') && tok->_str[1] == 0)
        {
            *strrchr(str, ']') = 0;
            if ( strchr( str + 1, tok->_str[0] ) == 0 )
                return false;
        }

        // Parse multi options, such as void|int|char (accept token which is one of these 3)
        else if ( strchr(str, '|') && strlen( str ) > 2 )
        {
            int res = multiCompare( str, tok->_cstr );
            if( res == 0 )
            {
                // Empty alternative matches, use the same token on next round
                continue;
            }
            else if( res == -1 )
            {
                // No match
                return false;
            }
        }

        else if (str != tok->_str)
            return false;

        tok = tok->next();
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

bool TOKEN::isStandardType() const
{
    bool ret = false;
    const char *type[] = {"bool","char","short","int","long","float","double",0};
    for (int i = 0; type[i]; i++)
        ret |= (_str == type[i]);
    return ret;
}

//---------------------------------------------------------------------------

const TOKEN *TOKEN::findmatch(const TOKEN *tok, const char pattern[], const char *varname1[], const char *varname2[])
{
    for ( ; tok; tok = tok->next())
    {
        if ( TOKEN::Match(tok, pattern, varname1, varname2) )
            return tok;
    }
    return 0;
}

const TOKEN *TOKEN::findtoken(const TOKEN *tok1, const char *tokenstr[])
{
    for (const TOKEN *ret = tok1; ret; ret = ret->next())
    {
        unsigned int i = 0;
        const TOKEN *tok = ret;
        while (tokenstr[i])
        {
            if (!tok)
                return NULL;
            if (*(tokenstr[i]) && (tokenstr[i] != tok->_str))
                break;
            tok = tok->next();
            i++;
        }
        if (!tokenstr[i])
            return ret;
    }
    return NULL;
}

unsigned int TOKEN::varId() const
{
    return _varId;
}

void TOKEN::varId( unsigned int id )
{
    _varId = id;
}

TOKEN *TOKEN::next() const
{
    return _next;
}

void TOKEN::next( TOKEN *next )
{
    _next = next;
}

unsigned int TOKEN::fileIndex() const
{
    return _fileIndex;
}

void TOKEN::fileIndex( unsigned int fileIndex )
{
    _fileIndex = fileIndex;
}

unsigned int TOKEN::linenr() const
{
    return _linenr;
}

void TOKEN::linenr( unsigned int linenr )
{
    _linenr = linenr;
}

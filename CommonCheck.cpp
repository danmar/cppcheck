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
#include "CommonCheck.h"
#include "tokenize.h"
#include <stdlib.h>     // free
#include <iostream>
#include <sstream>
#include <list>
#include <algorithm>
#include <cstring>

#ifdef __BORLANDC__
#include <ctype.h>
#endif
//---------------------------------------------------------------------------
bool OnlyReportUniqueErrors;
std::ostringstream errout;



//---------------------------------------------------------------------------

bool SameFileName( const char fname1[], const char fname2[] )
{
#ifdef __linux__
    return bool( strcmp(fname1, fname2) == 0 );
#endif
#ifdef __GNUC__
    return bool( strcasecmp(fname1, fname2) == 0 );
#endif
#ifdef __BORLANDC__
    return bool( stricmp(fname1, fname2) == 0 );
#endif
#ifdef _MSC_VER
    return bool( _stricmp(fname1, fname2) == 0 );
#endif
}
//---------------------------------------------------------------------------

std::list<std::string> ErrorList;

void ReportErr(const std::string &errmsg)
{
    if ( OnlyReportUniqueErrors )
    {
        if ( std::find( ErrorList.begin(), ErrorList.end(), errmsg ) != ErrorList.end() )
            return;
        ErrorList.push_back( errmsg );
    }
    errout << errmsg << std::endl;
}
//---------------------------------------------------------------------------

bool IsName(const char str[])
{
    return bool(str[0]=='_' || isalpha(str[0]));
}
//---------------------------------------------------------------------------

bool IsNumber(const char str[])
{
    return bool(isdigit(str[0]) != 0);
}
//---------------------------------------------------------------------------

bool IsStandardType(const char str[])
{
    if (!str)
        return false;
    bool Ret = false;
    const char *type[] = {"bool","char","short","int","long","float","double",0};
    for (int i = 0; type[i]; i++)
        Ret |= (strcmp(str,type[i])==0);
    return Ret;
}

//--------------------------------------------------------------------------

bool Match(const TOKEN *tok, const char pattern[], const char *varname1[], const char *varname2[])
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
            if (!IsName(tok->str))
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
                if ( ! Tokenizer::gettok(tok, 2) )
                    return false;

                if ( strcmp(Tokenizer::getstr(tok, 1), ".") )
                    return false;

                if ( strcmp(Tokenizer::getstr(tok, 2), varname[i]) )
                    return false;

                tok = Tokenizer::gettok(tok, 2);
            }
        }

        else if (strcmp(str,"%num%")==0)
        {
            if ( ! IsNumber(tok->str) )
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
        if (!tok)
            return false;
    }

    // The end of the pattern has been reached and nothing wrong has been found
    return true;
}

//---------------------------------------------------------------------------




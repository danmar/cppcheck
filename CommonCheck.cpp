//---------------------------------------------------------------------------
#include "CommonCheck.h"
#include "tokenize.h"
#include <iostream>
#include <sstream>
#include <list>
#include <algorithm>
//---------------------------------------------------------------------------
bool HasErrors;
bool OnlyReportUniqueErrors;
std::ostringstream errout;
std::list<const TOKEN *> FunctionList;
//---------------------------------------------------------------------------

std::string FileLine(const TOKEN *tok)
{
    std::ostringstream ostr;
    ostr << "[" << Files[tok->FileIndex] << ":" << tok->linenr << "]";
    return ostr.str();
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
    HasErrors = true;
}
//---------------------------------------------------------------------------

bool IsName(const char str[])
{
    return (str[0]=='_' || std::isalpha(str[0]));
}
//---------------------------------------------------------------------------

bool IsNumber(const char str[])
{
    return std::isdigit(str[0]);
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
//---------------------------------------------------------------------------

void FillFunctionList()
{
    int indentlevel = 0;
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
            indentlevel--;

        else if (indentlevel==0 && Match(tok, "%var% ("))
        {
            // Check if this is the first token of a function implementation..
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if ( tok2->str[0] == ';' )
                {
                    tok = tok2;
                    break;
                }

                else if ( tok2->str[0] == '{' )
                {
                    break;
                }

                else if ( tok2->str[0] == ')' )
                {
                    if ( Match(tok2, ") {") )
                    {
                        FunctionList.push_back( tok );
                        tok = tok2;
                    }
                    else
                    {
                        tok = tok2;
                        while (tok->next && !strchr(";{", tok->next->str[0]))
                            tok = tok->next;
                    }
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------

const TOKEN *GetFunctionTokenByName( const char funcname[] )
{
    std::list<const TOKEN *>::const_iterator it;
    for ( it = FunctionList.begin(); it != FunctionList.end(); it++ )
    {
        if ( strcmp( (*it)->str, funcname ) == 0 )
        {
            return *it;
        }
    }
    return NULL;
}
//---------------------------------------------------------------------------

bool Match(const TOKEN *tok, const char pattern[], const char *varname[])
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
        else if (strcmp(str,"%var1%")==0)
        {
            if (strcmp(tok->str, varname[0]) != 0)
                return false;

            for ( int i = 1; varname[i]; i++ )
            {
                if ( ! gettok(tok, 2) )
                    return false;

                if ( strcmp(getstr(tok, 1), ".") )
                    return false;

                if ( strcmp(getstr(tok, 2), varname[i]) )
                    return false;

                tok = gettok(tok, 2);
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


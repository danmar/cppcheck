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

const TOKEN *FindFunction( const TOKEN *tok, const char funcname[] )
{
    int indentlevel = 0;
    for ( ; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
            indentlevel--;

        else if (indentlevel==0 && match(tok,"var ("))
        {
            // Check if this is the first token of a function implementation..
            bool haspar = false;
            bool foundname = false;
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                haspar |= bool(tok2->str[0] == '(');
                if ( ! haspar && match(tok2,"var (") )
                {
                    if ( funcname && strcmp(funcname, tok2->str) != 0 )
                        break;
                    foundname = true;
                }
                if ( tok2->str[0] == ';' )
                {
                    tok = tok2;
                    break;
                }
                if ( tok2->str[0] == '{' )
                    break;
                if ( foundname && haspar && match(tok2, ") {") )
                    return tok;
            }
        }
    }
    return NULL;
}


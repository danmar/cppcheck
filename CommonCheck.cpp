//---------------------------------------------------------------------------
#include "CommonCheck.h"
#include "tokenize.h"
#include <iostream>
#include <sstream>
#include <list>
#include <algorithm>
//---------------------------------------------------------------------------
extern bool CheckCodingStyle;
bool OnlyReportUniqueErrors;
std::ostringstream errout;
static std::list<const TOKEN *> FunctionList;

class clGlobalFunction
{
private:
    unsigned int _FileId;
    std::string  _FuncName;

public:
    clGlobalFunction( const unsigned int FileId, const char FuncName[] )
    {
        _FileId = FileId;
        _FuncName = FuncName;
    }

    const unsigned int file_id() const { return _FileId; }
    const std::string &name() const { return _FuncName; }
};

static std::list< clGlobalFunction > GlobalFunctions;
static std::list< clGlobalFunction > UsedGlobalFunctions;

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

void FillFunctionList(const unsigned int file_id)
{
    FunctionList.clear();

    std::list<const char *> _usedfunc;
    if ( file_id == 0 )
    {
        GlobalFunctions.clear();
    }

    bool staticfunc = false;
    bool classfunc = false;

    int indentlevel = 0;
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
            indentlevel--;


        if (indentlevel > 0)
        {
            if ( CheckCodingStyle )
            {
                const char *funcname = 0;

                if ( Match(tok,"%var% (") )
                    funcname = tok->str;
                else if ( Match(tok, "= %var% ;") ||
                          Match(tok, "= %var% ,") )
                    funcname = tok->next->str;

                if ( std::find(_usedfunc.begin(), _usedfunc.end(), funcname) == _usedfunc.end() )
                    _usedfunc.push_back( funcname );
            }

            continue;
        }

        if (strchr("};", tok->str[0]))
            staticfunc = classfunc = false;

        else if ( strcmp( tok->str, "static" ) == 0 )
            staticfunc = true;

        else if ( strcmp( tok->str, "::" ) == 0 )
            classfunc = true;

        else if (Match(tok, "%var% ("))
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
                        if (CheckCodingStyle && !staticfunc && !classfunc && tok->FileIndex==0)
                            GlobalFunctions.push_back( clGlobalFunction(file_id, tok->str) );
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

    for (std::list<const char *>::const_iterator it = _usedfunc.begin(); it != _usedfunc.end(); ++it)
    {
        if ( *it != 0 )
        {
            UsedGlobalFunctions.push_back( clGlobalFunction(file_id, *it) );
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

void CheckGlobalFunctionUsage(const std::vector<std::string> &filenames)
{
    // Iterator for GlobalFunctions
    std::list<clGlobalFunction>::const_iterator func;

    // Iterator for UsedGlobalFunctions
    std::list<clGlobalFunction>::const_iterator usedfunc;

    // Check that every function in GlobalFunctions are used
    for ( func = GlobalFunctions.begin(); func != GlobalFunctions.end(); func++ )
    {
        if ( func->name() == "main" )
            continue;

        // Check if this global function is used in any of the other files..
        bool UsedOtherFile = false;
        bool UsedAnyFile = false;
        for ( usedfunc = UsedGlobalFunctions.begin(); usedfunc != UsedGlobalFunctions.end(); usedfunc++ )
        {
            if ( func->name() == usedfunc->name() )
            {
                UsedAnyFile = true;
                UsedOtherFile |= (func->file_id() != usedfunc->file_id());
            }
        }

        std::string file = "[" + filenames[func->file_id()] + "]: ";

        if ( ! UsedAnyFile )
        {
            std::ostringstream errmsg;
            errmsg << file << "The function '" << func->name() << "' is never used.";
            ReportErr( errmsg.str() );
        }
        else if ( ! UsedOtherFile )
        {
            std::ostringstream errmsg;
            errmsg << file << "The linkage of the function '" << func->name() << "' can be local (static) instead of global";
            ReportErr( errmsg.str() );
        }
    }
}
//---------------------------------------------------------------------------

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


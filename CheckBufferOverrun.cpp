//---------------------------------------------------------------------------
#include "CheckBufferOverrun.h"
#include "tokenize.h"
#include "CommonCheck.h"

#include <sstream>

#include <stdlib.h>     // <- strtoul

//---------------------------------------------------------------------------

TOKEN *findfunction(TOKEN *tok)
{
    int indentlevel = 0, parlevel = 0;
    for (; tok; tok = tok->next)
    {
        if (tok->str[0] == '{')
            indentlevel++;
        else if (tok->str[0] == '}')
            indentlevel--;
        else if (tok->str[0] == '(')
            parlevel++;
        else if (tok->str[0] == ')')
            parlevel--;

        if (!tok->next)
            break;

        if (indentlevel==0 && parlevel==0 && IsName(tok->str) && tok->next->str[0]=='(')
        {
            for (TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next)
            {
                if (tok2->str[0] == ')' && tok2->next)
                {
                    if (tok2->next->str[0] == '{')
                        return tok;
                    break;
                }
            }
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// Writing dynamic data in buffer without bounds checking
//---------------------------------------------------------------------------

extern bool ShowAll;

static void _DynamicDataCheck(const TOKEN *ftok, const TOKEN *tok)
{
    const char *var2 = tok->str;
    bool decl = false;
    unsigned int Var2Count = 0;
    for ( const TOKEN *tok2 = ftok; tok2; tok2 = tok2->next )
    {
        if (tok2 == tok)
            break;
        if (match(tok2,"char * var"))
        {
            decl |= (strcmp(getstr(tok2,2),var2)==0);
            tok2 = gettok(tok2,3);
            if ( strcmp(tok2->str, "=") == 0 )
            {
                Var2Count++;
                break;
            }
        }
        if (match(tok2,"char var [ ]"))
        {
            decl |= (strcmp(getstr(tok2,1),var2)==0);
            tok2 = gettok(tok2,3);
        }

        // If ShowAll, only strlen(var2) counts
        if ( ShowAll )
        {
            if (match(tok2,"strlen ( var )") &&
                strcmp(getstr(tok2,2),var2)==0)
            {
                Var2Count++;
                break;
            }
        }

        // If not ShowAll, all usage of "var2" counts
        else
        {
            if (strcmp(tok2->str,var2)==0)
            {
                Var2Count++;
                break;
            }
        }
    }

    // The size of Var2 isn't checked, is it?
    if (decl && Var2Count == 0)
    {
        std::ostringstream ostr;
        ostr << FileLine(tok) << ": A string with unknown length is copied to buffer.";
        ReportErr(ostr.str());
    }
}


static void _DynamicData()
{
    for (const TOKEN *ftok = findfunction(tokens); ftok; ftok = findfunction(ftok->next))
    {
        int indentlevel = 0;
        for (const TOKEN *tok = ftok; tok; tok = tok->next)
        {
            if (tok->str[0] == '{')
                indentlevel++;
            else if (tok->str[0] == '}')
            {
                indentlevel--;
                if (indentlevel <= 0)
                    break;
            }


            if (match(tok,"strcpy ( var , var )") ||
                match(tok,"strcat ( var , var )") )
            {
                _DynamicDataCheck(ftok,gettok(tok,4));
            }

            if (match(tok,"sprintf ( var"))
            {
                for ( const TOKEN *tok2 = gettok(tok,3); tok2; tok2 = tok2->next )
                {
                    if (tok2->str[0] == ')')
                        break;
                    if (match(tok2,", var ,") || match(tok2,", var )"))
                    {
                        _DynamicDataCheck(ftok,tok2->next);
                    }
                }
            }

        }
    }
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Buffer overrun..
//---------------------------------------------------------------------------

static void CheckBufferOverrun_LocalVariable()
{
    _DynamicData();

    int indentlevel = 0;
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
            indentlevel--;

        else if (indentlevel > 0)
        {
            // Declaring array..
            if (match(tok, "type var [ num ] ;"))
            {
                const char *varname = getstr(tok,1);
                unsigned int size = strtoul(getstr(tok,3), NULL, 10);
                int total_size = size * SizeOfType(tok->str);
                if (total_size == 0)
                    continue;
                int _indentlevel = indentlevel;
                for (const TOKEN *tok2 = gettok(tok,5); tok2; tok2 = tok2->next)
                {
                    if (tok2->str[0]=='{')
                    {
                        _indentlevel++;
                    }
                    else if (tok2->str[0]=='}')
                    {
                        _indentlevel--;
                        if (_indentlevel < indentlevel)
                            break;
                    }
                    else
                    {
                        // Array index..
                        if (strcmp(tok2->str,varname)==0 &&
                            strcmp(getstr(tok2,1),"[")==0 &&
                            IsNumber(getstr(tok2,2)) &&
                            strcmp(getstr(tok2,3),"]")==0 )
                        {
                            const char *str = getstr(tok2, 2);
                            if (strtoul(str, NULL, 10) >= size)
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(tok2) << ": Array index out of bounds";
                                ReportErr(ostr.str());
                            }
                        }


                        // memset, memcmp, memcpy, strncpy, fgets..
                        if (strcmp(tok2->str,"memset")==0 ||
                            strcmp(tok2->str,"memcpy")==0 ||
                            strcmp(tok2->str,"memmove")==0 ||
                            strcmp(tok2->str,"memcmp")==0 ||
                            strcmp(tok2->str,"strncpy")==0 ||
                            strcmp(tok2->str,"fgets")==0 )
                        {
                            if (match(tok2->next,"( var , num , num )") ||
                                match(tok2->next,"( var , var , num )") )
                            {
                                const char *var1 = getstr(tok2, 2);
                                const char *var2 = getstr(tok2, 4);
                                const char *num  = getstr(tok2, 6);

                                if ( atoi(num)>total_size &&
                                    (strcmp(var1,varname)==0 ||
                                     strcmp(var2,varname)==0 ) )
                                {
                                    std::ostringstream ostr;
                                    ostr << FileLine(tok2) << ": Buffer overrun";
                                    ReportErr(ostr.str());
                                }
                            }
                        }


                        // Loop..
                        const char *strindex = 0;
                        int value = 0;
                        if ( match(tok2, "for ( var = 0 ;") )
                        {
                            if (match(tok2,"for ( var = 0 ; var < num ; var + + )"))
                            {
                                strindex = getstr(tok2,2);
                                value = atoi(getstr(tok2,8));
                            }
                            else if (match(tok2,"for ( var = 0 ; var <= num ; var + + )"))
                            {
                                strindex = getstr(tok2,2);
                                value = 1 + atoi(getstr(tok2,8));
                            }
                            else if (match(tok2,"for ( var = 0 ; var < num ; + + var )"))
                            {
                                strindex = getstr(tok2,2);
                                value = atoi(getstr(tok2,8));
                            }
                            else if (match(tok2,"for ( var = 0 ; var <= num ; + + var )"))
                            {
                                strindex = getstr(tok2,2);
                                value = 1 + atoi(getstr(tok2,8));
                            }
                        }
                        if (strindex && value>(int)size)
                        {
                            const TOKEN *tok3 = tok2;
                            while (tok3 && strcmp(tok3->str,")"))
                                tok3 = tok3->next;
                            if (!tok3)
                                break;
                            tok3 = tok3->next;
                            if (tok3->str[0] == '{')
                                tok3 = tok3->next;
                            while (tok3 && !strchr(";}",tok3->str[0]))
                            {
                                if (strcmp(tok3->str,varname)==0 &&
                                    strcmp(getstr(tok3,1),"[")==0 &&
                                    strcmp(getstr(tok3,2),strindex)==0 &&
                                    strcmp(getstr(tok3,3),"]")==0 )
                                {
                                    std::ostringstream ostr;
                                    ostr << FileLine(tok3) << ": Buffer overrun";
                                    ReportErr(ostr.str());
                                    break;
                                }
                                tok3 = tok3->next;
                            }
                        }


                        // Writing data into array..
                        if (match(tok2,"strcpy ( var , "))
                        {
                            int len = 0;
                            if (strcmp(getstr(tok2, 2), varname) == 0)
                            {
                                const char *str = getstr(tok2, 4);
                                if (str[0] == '\"')
                                {
                                    while (*str)
                                    {
                                        if (*str=='\\')
                                            str++;
                                        str++;
                                        len++;
                                    }
                                }
                            }
                            if (len > 2 && len >= (int)size + 2)
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(tok2) << ": Buffer overrun";
                                ReportErr(ostr.str());
                            }
                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------

static void CheckBufferOverrun_StructVariable_CheckVar( const TOKEN *tok1, const char varname[], const char dot[], const char arrname[], const int arrsize )
{
    const char *badpattern[] = {"varname",".","arrname","[","","]",NULL};
    badpattern[0] = varname;
    badpattern[1] = dot;
    badpattern[2] = arrname;
    const TOKEN *tok2 = findtoken( tok1, badpattern );
    while (tok2)
    {
        if ( IsNumber( getstr(tok2, 4) ) )
        {
            if ( atoi( getstr(tok2, 4) ) >= arrsize )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(tok2) << ": Array index out of bounds";
                ReportErr(errmsg.str());
            }
        }
        tok2 = findtoken( tok2->next, badpattern );
    }
}
//---------------------------------------------------------------------------

static void CheckBufferOverrun_StructVariable()
{
    const char *declstruct_pattern[] = {"struct","","{",0};
    for ( const TOKEN * tok = findtoken( tokens, declstruct_pattern );
          tok;
          tok = findtoken( tok->next, declstruct_pattern ) )
    {
        const char *structname = tok->next->str;

        if ( ! IsName( structname ) )
            continue;

        // Found a struct declaration. Search for arrays..
        for ( TOKEN * tok2 = tok->next->next; tok2; tok2 = tok2->next )
        {
            if ( tok2->str[0] == '}' )
                break;

            if ( strchr( ";{,(", tok2->str[0] ) )
            {
                // Declare array..
                if ( match(tok2->next, "var var [ num ] ;") )
                {
                    const char *arrname = getstr(tok2, 2);
                    const char *arrsize = getstr(tok2, 4);

                    for ( const TOKEN *tok3 = tokens; tok3; tok3 = tok3->next )
                    {
                        if ( strcmp(tok3->str, structname) )
                            continue;

                        // Declare variable: Fred fred1;
                        if ( match( tok3->next, "var ;" ) )
                        {
                            const char *varname = tok3->next->str;
                            CheckBufferOverrun_StructVariable_CheckVar( tok3, varname, ".", arrname, atoi(arrsize) );
                        }

                        // Declare pointer: Fred *fred1
                        else if ( match(tok3->next, "* var") && tok3->next->next->next && strchr(",);=", tok3->next->next->next->str[0]) )
                        {
                            const char *varname = tok3->next->next->str;
                            CheckBufferOverrun_StructVariable_CheckVar( tok3, varname, "->", arrname, atoi(arrsize) );
                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------

void CheckBufferOverrun()
{
    CheckBufferOverrun_LocalVariable();
    CheckBufferOverrun_StructVariable();
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// Dangerous functions
//---------------------------------------------------------------------------

void WarningDangerousFunctions()
{
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (match(tok, "gets ("))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Found 'gets'. You should use 'fgets' instead";
            ReportErr(ostr.str());
        }

        else if (match(tok, "scanf (") && strcmp(getstr(tok,2),"\"%s\"") == 0)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Found 'scanf'. You should use 'fgets' instead";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// Buffer overrun..
//---------------------------------------------------------------------------

#include "CheckBufferOverrun.h"
#include "tokenize.h"
#include "CommonCheck.h"

#include <sstream>
#include <list>

#include <stdlib.h>     // <- strtoul

//---------------------------------------------------------------------------
extern bool ShowAll;
//---------------------------------------------------------------------------

// CallStack used when parsing into subfunctions.
static std::list<const TOKEN *> CallStack;

// Modified version of 'ReportError' that also reports the callstack
static void ReportError(const TOKEN *tok, const char errmsg[])
{
    std::ostringstream ostr;
    std::list<const TOKEN *>::const_iterator it;
    for ( it = CallStack.begin(); it != CallStack.end(); it++ )
        ostr << FileLine(*it) << " -> ";
    ostr << FileLine(tok) << ": " << errmsg;
    ReportErr(ostr.str());
}


static bool Match1(const TOKEN *tok, const char pattern[], const char *varname[])
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
            if (!IsName(tok->str))
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


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

static void CheckBufferOverrun_CheckScope( const TOKEN *tok, const char *varname[], const int size, const int total_size )
{
    unsigned int varc = 1;
    while ( varname[varc] )
        varc++;
    varc = 2 * ( varc - 1 );

    int indentlevel = 0;
    for ( ; tok; tok = tok->next )
    {
        if (tok->str[0]=='{')
        {
            indentlevel++;
            continue;
        }

        if (tok->str[0]=='}')
        {
            indentlevel--;
            if ( indentlevel < 0 )
                return;
            continue;
        }


        // Array index..
        if ( Match1(tok, "%var1% [ %num% ]", varname) )
        {
            const char *num = getstr(tok, 2 + varc);
            if (strtol(num, NULL, 10) >= size)
            {
                ReportError(tok, "Array index out of bounds");
            }
            continue;
        }


        // memset, memcmp, memcpy, strncpy, fgets..
        if (strcmp(tok->str,"memset")==0 ||
            strcmp(tok->str,"memcpy")==0 ||
            strcmp(tok->str,"memmove")==0 ||
            strcmp(tok->str,"memcmp")==0 ||
            strcmp(tok->str,"strncpy")==0 ||
            strcmp(tok->str,"fgets")==0 )
        {
            if ( Match1( tok->next, "( %var1% , %num% , %num% )", varname ) ||
                 Match1( tok->next, "( %var% , %var1% , %num% )", varname ) )
            {
                const char *num  = getstr(tok, varc + 6);
                if ( atoi(num) > total_size )
                {
                    ReportError(tok, "Buffer overrun");
                }
            }
            continue;
        }


        // Loop..
        if ( match(tok, "for (") )
        {
            const TOKEN *tok2 = gettok( tok, 2 );

            // for - setup..
            if ( match(tok2, "var = 0 ;") )
                tok2 = gettok(tok2, 4);
            else if ( match(tok2, "type var = 0 ;") )
                tok2 = gettok(tok2, 5);
            else if ( match(tok2, "type type var = 0 ;") )
                tok2 = gettok(tok2, 6);
            else
                continue;

            // for - condition..
            if ( ! match(tok2, "var < num ;") && ! match(tok2, "var <= num ;"))
                continue;

            // Get index variable and stopsize.
            const char *strindex = tok2->str;
            int value = (tok2->next->str[1] ? 1 : 0) + atoi(getstr(tok2, 2));
            if ( value <= size )
                continue;

            // Goto the end of the for loop..
            while (tok2 && strcmp(tok2->str,")"))
                tok2 = tok2->next;
            if (!gettok(tok2,5))
                break;

            std::ostringstream pattern;
            pattern << "%var1% [ " << strindex << " ]";

            int indentlevel2 = 0;
            while (tok2)
            {
                if ( tok2->str[0] == ';' && indentlevel == 0 )
                    break;

                if ( tok2->str[0] == '{' )
                    indentlevel2++;

                if ( tok2->str[0] == '}' )
                {
                    indentlevel2--;
                    if ( indentlevel2 <= 0 )
                        break;
                }

                if ( Match1( tok2, pattern.str().c_str(), varname ) )
                {
                    ReportError(tok2, "Buffer overrun");
                    break;
                }

                tok2 = tok2->next;
            }
            continue;
        }


        // Writing data into array..
        if ( Match1(tok, "strcpy ( %var1% , %str% )", varname) )
        {
            int len = 0;
            const char *str = getstr(tok, varc + 4 );
            while ( *str )
            {
                if (*str=='\\')
                    str++;
                str++;
                len++;
            }
            if (len > 2 && len >= (int)size + 2)
            {
                 ReportError(tok, "Buffer overrun");
            }
            continue;
        }


        // Function call..
        // Todo: Handle struct member variables..
        if ( varc == 0 && match( tok, "var (" ) )
        {
            // Don't make recursive checking..
            if (std::find(CallStack.begin(), CallStack.end(), tok) != CallStack.end())
                continue;

            unsigned int parlevel = 0, par = 0;
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if ( tok2->str[0] == '(' )
                {
                    parlevel++;
                }

                else if ( tok2->str[0] == ')' )
                {
                    parlevel--;
                    if ( parlevel < 1 )
                    {
                        par = 0;
                        break;
                    }
                }

                else if ( parlevel == 1 && tok2->str[0] == ',' )
                {
                    par++;
                }

                if ( parlevel == 1 &&
                    strchr( "(,", *getstr(tok2,0) ) &&
                    strcmp( varname[0], getstr(tok2, 1) ) == 0 &&
                    strchr( ",)", *getstr(tok2,2) ) )
                {
                    par++;
                    break;
                }
            }

            if ( par == 0 )
                continue;

            // Find function..
            const TOKEN *ftok = FindFunction( tokens, tok->str );
            if ( ! ftok )
                continue;

            // Parse head of function..
            ftok = gettok( ftok, 2 );
            parlevel = 1;
            while ( ftok && parlevel == 1 && par >= 1 )
            {
                if ( ftok->str[0] == '(' )
                    parlevel++;

                else if ( ftok->str[0] == ')' )
                    parlevel--;

                else if ( ftok->str[0] == ',' )
                    par--;

                else if (par==1 && parlevel==1 && (match(ftok,"var ,") || match(ftok,"var )")))
                {
                    // Parameter name..
                    const char *parname[2];
                    parname[0] = ftok->str;
                    parname[1] = 0;

                    // Goto function body..
                    while ( ftok && ftok->str[0] != '{' )
                        ftok = ftok->next;
                    ftok = ftok ? ftok->next : 0;

                    // Check variable usage in the function..
                    CallStack.push_back( tok );
                    CheckBufferOverrun_CheckScope( ftok, parname, size, total_size );
                    CallStack.pop_back();

                    // break out..
                    break;
                }

                ftok = ftok->next;
            }
        }
    }
}


//---------------------------------------------------------------------------
// Checking local variables in a scope
//---------------------------------------------------------------------------

static void CheckBufferOverrun_LocalVariable()
{
    int indentlevel = 0;
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
            indentlevel--;

        else if (indentlevel > 0 && match(tok, "type var [ num ] ;"))
        {
            const char *varname[2];
            varname[0] = getstr(tok,1);
            varname[1] = 0;
            unsigned int size = strtoul(getstr(tok,3), NULL, 10);
            int total_size = size * SizeOfType(tok->str);
            if (total_size == 0)
                continue;

            // The callstack is empty
            CallStack.clear();
            CheckBufferOverrun_CheckScope( gettok(tok,5), varname, size, total_size );
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checking member variables of structs..
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
                    const char *varname[3] = {0,0,0};
                    varname[1] = getstr(tok2, 2);
                    int arrsize = atoi(getstr(tok2, 4));
                    int total_size = arrsize * SizeOfType(tok2->next->str);
                    if (total_size == 0)
                        continue;

                    for ( const TOKEN *tok3 = tokens; tok3; tok3 = tok3->next )
                    {
                        if ( strcmp(tok3->str, structname) )
                            continue;

                        // Declare variable: Fred fred1;
                        if ( match( tok3->next, "var ;" ) )
                        {
                            varname[0] = getstr(tok3, 1);
                            CheckBufferOverrun_CheckScope( tok3, varname, arrsize, total_size );
                        }

                        // Declare pointer: Fred *fred1
                        else if ( match(tok3->next, "* var") && tok3->next->next->next && strchr(",);=", tok3->next->next->next->str[0]) )
                        {
                            varname[0] = getstr(tok3, 2);
                            CheckBufferOverrun_CheckScope( tok3, varname, arrsize, total_size );
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





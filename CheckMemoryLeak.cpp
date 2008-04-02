
#include "CheckMemoryLeak.h"

#include "tokenize.h"

#include "CommonCheck.h"

#include <vector>
#include <sstream>

#ifdef __linux__
#include <string.h>
#else
#include <mem.h>     // <- memset
#endif

//---------------------------------------------------------------------------

extern bool ShowAll;

//---------------------------------------------------------------------------

static void CheckMemoryLeak_CheckScope( const TOKEN *Tok1, const char *varname[] )
{
    // Goto end of statement..
    const TOKEN *tok = Tok1;
    while (tok && tok->str[0] != ';')
        tok = tok->next;

    int indentlevel = 0;
    for ( ; tok; tok = tok->next )
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
        {
            indentlevel--;
            if ( indentlevel < 0 )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(Tok1) << ": Memory leak:" << varname[0];
                ReportErr( errmsg.str() );
                return;
            }
        }

        // Deallocated..
        if ( Match(tok, "delete %var1% ;", varname) )
            return;

        if ( Match(tok, "delete [ ] %var1% ;", varname) )
            return;

        if ( Match(tok, "free ( %var1% ) ;", varname) )
            return;

        if ( Match(tok, "kfree ( %var1% ) ;", varname) )
            return;

        // Used..
        if ( strchr("=,(", tok->str[0]) && Match( tok->next, "%var1%", varname )  )
            return;
        if ( Match( tok, "return %var1%", varname ) )
            return;

    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Checks for memory leaks inside function..
//---------------------------------------------------------------------------

static void CheckMemoryLeak_InFunction()
{
    int indentlevel = 0;
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
            indentlevel--;

        else if (indentlevel > 0 && Match(tok, "%var% = "))
        {
            // What we may have...
            //     * var = (char *)malloc(10);
            //     * var = new char[10];
            //     * var = strdup("hello");
            const TOKEN *tok2 = gettok(tok, 2);
            if ( tok2 && tok2->str[0] == '(' )
            {
                while ( tok2 && tok2->str[0] != ')' )
                    tok2 = tok2->next;
                tok2 = tok2 ? tok2->next : NULL;
            }
            if ( ! tok2 )
                continue;

            // tok2 now points on "malloc", "new", "strdup" or something like that..
            const char *allocfunc[] = {"malloc", "strdup", "kmalloc", "new", 0};
            for ( unsigned int i = 0; allocfunc[i]; i++ )
            {
                if ( strcmp(allocfunc[i], tok2->str) == 0 )
                {
                    const char *varname[2] = {0,0};
                    varname[0] = tok->str;
                    CheckMemoryLeak_CheckScope(tok, varname);
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Checks for memory leaks in classes..
//---------------------------------------------------------------------------

static void CheckMemoryLeak_ClassMembers()
{

}


//---------------------------------------------------------------------------
// Checks for memory leaks..
//---------------------------------------------------------------------------

void CheckMemoryLeak()
{
    // Check for memory leaks inside functions..
    CheckMemoryLeak_InFunction();

    // Check that all class members are deallocated..
    CheckMemoryLeak_ClassMembers();
}
//---------------------------------------------------------------------------






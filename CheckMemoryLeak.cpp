
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
    // Check input pointers..
    if ( ! (Tok1 && varname && varname[0] ) )
        return;

    int varc = 1;
    while ( varname[varc] )
        varc++;
    varc = (varc - 1) * 2;

    enum {No, Malloc, New, NewA} Alloc = No;

    int indentlevel = 0;
    for (const TOKEN *tok = Tok1 ; tok; tok = tok->next )
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
        {
            indentlevel--;
            if ( indentlevel < 0 && Alloc != No )
            {
                std::ostringstream errmsg;
                errmsg << FileLine(Tok1) << ": Memory leak:" << varname[0];
                ReportErr( errmsg.str() );
                return;
            }
        }

        // Allocated..
        if ( Match(tok, "%var1% =", varname) )
        {
            // What we may have...
            //     * var = (char *)malloc(10);
            //     * var = new char[10];
            //     * var = strdup("hello");
            const TOKEN *tok2 = gettok(tok, varc+2);
            if ( tok2 && tok2->str[0] == '(' )
            {
                while ( tok2 && tok2->str[0] != ')' )
                    tok2 = tok2->next;
                tok2 = tok2 ? tok2->next : NULL;
            }
            if ( ! tok2 )
                continue;

            // Does tok2 point on "malloc", "strdup" or "kmalloc"..
            const char *mallocfunc[] = {"malloc", "strdup", "kmalloc", 0};
            for ( unsigned int i = 0; mallocfunc[i]; i++ )
            {
                if ( strcmp(mallocfunc[i], tok2->str) == 0 )
                {
                    Alloc = Malloc;
                    break;
                }
            }

            if ( Match( tok2, "new %type% [;(]" ) )
                Alloc = New;

            if ( Match( tok2, "new %type% [" ) )
                Alloc = NewA;
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
        if ( Match( tok, "[=,(] %var1%", varname )  )
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
    bool infunc = false;
    int indentlevel = 0;
    for (const TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
            indentlevel--;


        // In function..
        if ( indentlevel == 0 )
        {
            if ( Match(tok, ") {") )
                infunc = true;

            if ( Match(tok, "[;}]") )
                infunc = false;
        }

        // Declare a local variable => Check
        if (indentlevel>0 && infunc)
        {
            if ( Match(tok, "[{};] %type% * %var% [;=]") )
            {
                const char *varname[2] = {0,0};
                varname[0] = getstr(tok, 3);
                CheckMemoryLeak_CheckScope( tok->next, varname );
            }

            if ( Match(tok, "[{};] %type% %type% * %var% [;=]") )
            {
                const char *varname[2] = {0,0};
                varname[0] = getstr(tok, 4);
                CheckMemoryLeak_CheckScope( tok->next, varname );
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






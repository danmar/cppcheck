
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

static void MismatchError( const TOKEN *Tok1, const char *varname[] )
{
    std::ostringstream errmsg;
    errmsg << FileLine(Tok1) << ": Mismatching allocation and deallocation: " << varname[0];
    ReportErr( errmsg.str() );
}
//---------------------------------------------------------------------------

static void MemoryLeak( const TOKEN *tok, const char *varname[] )
{
    std::ostringstream errmsg;
    errmsg << FileLine(tok) << ": Memory leak: " << varname[0];
    ReportErr( errmsg.str() );
}
//---------------------------------------------------------------------------

static void CheckMemoryLeak_CheckScope( const TOKEN *Tok1, const char *varname[] )
{
    int varc = 1;
    while ( varname[varc] )
        varc++;
    varc = (varc - 1) * 2;

    enum {No, Malloc, New, NewA} Alloc = No;

    int alloc_indentlevel = 0;

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
                MemoryLeak( Tok1, varname );
                return;
            }

            if ( indentlevel < alloc_indentlevel )
                alloc_indentlevel = -1;
        }

        // Skip stuff like: if (!var) ...
        if ( Match(tok, "if ( ! %var1% )", varname) ||
             Match(tok, "if ( %var1% == NULL )", varname) ||
             Match(tok, "if ( %var1% == 0 )", varname) )
        {
            int _indentlevel = 0;
            while ( tok )
            {
                if ( tok->str[0] == '{' )
                    _indentlevel++;
                else if ( tok->str[0] == '}' )
                {
                    _indentlevel--;
                    if ( _indentlevel <= 0 )
                        break;
                }
                else if (_indentlevel==0 && tok->str[0]==';')
                    break;
                tok = tok->next;
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
                    alloc_indentlevel = indentlevel;
                    break;
                }
            }

            if ( Match( tok2, "new %type% [;(]" ) )
            {
                alloc_indentlevel = indentlevel;
                Alloc = New;
            }

            if ( Match( tok2, "new %type% [" ) )
            {
                alloc_indentlevel = indentlevel;
                Alloc = NewA;
            }
        }


        // Deallocated..
        if ( Match(tok, "delete %var1% ;", varname) )
        {
            if ( Alloc != No && Alloc != New )
                MismatchError( Tok1, varname );
            return;
        }

        if ( Match(tok, "delete [ ] %var1% ;", varname) )
        {
            if ( Alloc != No && Alloc != NewA )
                MismatchError( Tok1, varname );
            return;
        }

        if ( Match(tok, "free ( %var1% ) ;", varname) || Match(tok, "kfree ( %var1% ) ;", varname) )
        {
            if ( Alloc != No && Alloc != Malloc )
                MismatchError( Tok1, varname );
            return;
        }

        // Used..
        if ( Match( tok, "[=,(] %var1%", varname )  )
            return;
        if ( Match( tok, "return %var1%", varname ) )
            return;

        if ( Alloc != No && alloc_indentlevel >= 0 && Match(tok, "return") )
        {
            MemoryLeak( tok, varname );
            return;
        }
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

static void CheckMemoryLeak_ClassMembers_ParseClass( const TOKEN *tok1, std::vector<const char *> &classname );

static void CheckMemoryLeak_ClassMembers()
{
    int indentlevel = 0;
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
            indentlevel--;

        else if ( indentlevel == 0 && Match(tok, "class %var% [{:]") )
        {
            std::vector<const char *> classname;
            classname.push_back( getstr(tok, 1) );
            CheckMemoryLeak_ClassMembers_ParseClass( tok, classname );
        }
    }
}

static void CheckMemoryLeak_ClassMembers_ParseClass( const TOKEN *tok1, std::vector<const char *> &classname )
{
    // Go into class.
    while ( tok1 && tok1->str[0] != '{' )
        tok1 = tok1->next;
    if ( tok1 )
        tok1 = tok1->next;

    int indentlevel = 0;
    for ( const TOKEN *tok = tok1; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
        {
            indentlevel--;
            if ( indentlevel < 0 )
                return;
        }

        else if ( indentlevel == 0 && Match(tok, "class %var% [{:]") )
        {
            classname.push_back( getstr(tok, 1) );
            CheckMemoryLeak_ClassMembers_ParseClass( tok, classname );
            classname.pop_back();
        }

        else if ( indentlevel == 1 && Match(tok, "[:;] %type% * %var% ;") )
        {
            const char *func_pattern[] = {"classname", "::", "", "("};
            func_pattern[0] = classname.back();
            for ( const TOKEN *ftok = findtoken(tokens, func_pattern);
                  ftok;
                  ftok = findtoken(ftok->next, func_pattern) )
            {
                for ( const TOKEN *tok2 = ftok; tok2; tok2 = tok2->next )
                {
                    if ( tok->str[0] == ';' )
                        break;
                    if ( tok->str[0] == '{' )
                    {  
                        classname.push_back( getstr(tok, 3) );
                        classname.push_back( 0 );
                        // Todo check the function..
                        classname.pop_back();
                        classname.pop_back();
                    }
                }
            }
        }
    }
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






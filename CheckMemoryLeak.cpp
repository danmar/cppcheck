
#include "CheckMemoryLeak.h"

#include "tokenize.h"

#include "CommonCheck.h"

#include <vector>
#include <sstream>

#ifdef __BORLANDC__
#include <mem.h>     // <- memset
#else
#include <string.h>
#endif

//---------------------------------------------------------------------------

extern bool ShowAll;

//---------------------------------------------------------------------------

enum AllocType { No, Malloc, New, NewA };

static AllocType GetAllocationType( const TOKEN *tok2 )
{
    // What we may have...
    //     * var = (char *)malloc(10);
    //     * var = new char[10];
    //     * var = strdup("hello");
    if ( tok2 && tok2->str[0] == '(' )
    {
        while ( tok2 && tok2->str[0] != ')' )
            tok2 = tok2->next;
        tok2 = tok2 ? tok2->next : NULL;
    }
    if ( ! tok2 )
        return No;

    // Does tok2 point on "malloc", "strdup" or "kmalloc"..
    const char *mallocfunc[] = {"malloc",
                                "calloc",
                                "realloc",
                                "strdup",
                                "kmalloc",
                                "kzalloc",
                                "g_malloc",
                                0};
    for ( unsigned int i = 0; mallocfunc[i]; i++ )
    {
        if ( strcmp(mallocfunc[i], tok2->str) == 0 )
            return Malloc;
    }

    if ( Match( tok2, "new %type% [;(]" ) )
        return New;

    if ( Match( tok2, "new %type% [" ) )
        return NewA;

    return No;
}

static AllocType GetDeallocationType( const TOKEN *tok, const char *varnames[] )
{
    if ( Match(tok, "delete %var1% ;", varnames) )
        return New;

    if ( Match(tok, "delete [ ] %var1% ;", varnames) )
        return NewA;

    if ( Match(tok, "free ( %var1% ) ;", varnames) ||
         Match(tok, "kfree ( %var1% ) ;", varnames) ||
         Match(tok, "g_free ( %var1% ) ;", varnames) )
    {
        return Malloc;
    }

    return No;
}
//---------------------------------------------------------------------------

static void MismatchError( const TOKEN *Tok1, const char varname[] )
{
    std::ostringstream errmsg;
    errmsg << FileLine(Tok1) << ": Mismatching allocation and deallocation: " << varname;
    ReportErr( errmsg.str() );
}
//---------------------------------------------------------------------------

static void MemoryLeak( const TOKEN *tok, const char varname[] )
{
    std::ostringstream errmsg;
    errmsg << FileLine(tok) << ": Memory leak: " << varname;
    ReportErr( errmsg.str() );
}
//---------------------------------------------------------------------------

static void CheckMemoryLeak_CheckScope( const TOKEN *Tok1, const char varname[] )
{
    const char *varnames[2];
    varnames[0] = varname;
    varnames[1] = 0;

    AllocType Alloc = No;

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
        if ( Match(tok, "if ( ! %var1% )", varnames) ||
             Match(tok, "if ( %var1% == NULL )", varnames) ||
             Match(tok, "if ( %var1% == 0 )", varnames) )
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
        if ( Match(tok, "%var1% =", varnames) )
        {
            AllocType alloc = GetAllocationType( gettok(tok, 2) );
            if ( alloc != No )
            {
                Alloc = alloc;
                alloc_indentlevel = indentlevel;
            }
        }


        // Deallocated..
        AllocType dealloc = GetDeallocationType( tok, varnames );
        if ( dealloc != No )
        {
            if ( Alloc != No && Alloc != dealloc )
                MismatchError( Tok1, varname );
            return;
        }

        // Used..
        //     list.push_back( var1 );
        //     listtail->next = var1;
        //     foo( var1 );
        if ( Match( tok, "[=,(] %var1% [,);]", varnames )  )
            return;

        // continue/break loop..
        if (Alloc != No &&
            alloc_indentlevel == indentlevel &&
            (Match(tok,"continue") || Match(tok,"break")))
        {
            MemoryLeak( tok, varname );
            return;
        }

        // Return without deallocating the memory..
        if ( Alloc != No && alloc_indentlevel >= 0 && Match(tok, "return") )
        {
            bool retvar = false;
            for ( const TOKEN *tok2 = tok->next; tok2; tok2 = tok2->next )
            {
                if ( Match( tok2, "%var1%", varnames ) )
                {
                    retvar = true;
                    break;
                }

                if ( tok2->str[0] == ';' )
                {
                    break;
                }
            }

            if ( ! retvar )
                MemoryLeak( tok, varname );

            if ( indentlevel <= alloc_indentlevel )
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
                CheckMemoryLeak_CheckScope( tok->next, getstr(tok, 3) );

            else if ( Match(tok, "[{};] %type% %type% * %var% [;=]") )
                CheckMemoryLeak_CheckScope( tok->next, getstr(tok, 4) );
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Checks for memory leaks in classes..
//---------------------------------------------------------------------------

static void CheckMemoryLeak_ClassMembers_ParseClass( const TOKEN *tok1, std::vector<const char *> &classname );
static void CheckMemoryLeak_ClassMembers_Variable( const std::vector<const char *> &classname, const char varname[] );


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

        // Only parse this particular class.. not subclasses
        if ( indentlevel > 0 )
            continue;

        // Declaring subclass.. recursive checking
        if ( Match(tok, "class %var% [{:]") )
        {
            classname.push_back( getstr(tok, 1) );
            CheckMemoryLeak_ClassMembers_ParseClass( tok, classname );
            classname.pop_back();
        }

        // Declaring member variable.. check allocations and deallocations
        if ( Match(tok->next, "%type% * %var% ;") )
        {
            if ( IsName(tok->str) || strchr(";}", tok->str[0]) )
                CheckMemoryLeak_ClassMembers_Variable( classname, getstr(tok, 3) );
        }
    }
}

static void CheckMemoryLeak_ClassMembers_Variable( const std::vector<const char *> &classname, const char varname[] )
{
    // Function pattern.. Check if member function
    char fpattern[500] = {0};
    for ( unsigned int i = 0; i < classname.size(); i++ )
    {
        strcat( fpattern, classname[i] );
        strcat( fpattern, " :: " );
    }
    strcat( fpattern, "%var% (" );

    // Destructor pattern.. Check if class destructor..
    char destructor[500] = {0};
    for ( unsigned int i = 0; i < classname.size(); i++ )
    {
        strcat( destructor, classname[i] );
        strcat( destructor, " :: " );
    }
    strcat( destructor, " ~" );
    strcat( destructor, classname.back() );
    strcat( destructor, " (" );

    // Pattern used in member function. "Var = ..."
    std::ostringstream varname_eq;
    varname_eq << varname << " =";

    // Full variable name..
    std::ostringstream FullVariableName;
    for ( unsigned int i = 0; i < classname.size(); i++ )
        FullVariableName << classname[i] << "::";
    FullVariableName << varname;

    // Check if member variable has been allocated and deallocated..
    AllocType Alloc = No;
    AllocType Dealloc = No;

    // Loop through all tokens. Inspect member functions
    bool memberfunction = false;
    int indentlevel = 0;
    for ( const TOKEN *tok = tokens; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
            indentlevel++;

        else if ( tok->str[0] == '}' )
            indentlevel--;

        // Set the 'memberfunction' variable..
        if ( indentlevel == 0 )
        {
            if ( strchr(";}", tok->str[0]) )
                memberfunction = false;
            else if ( Match( tok, fpattern ) || Match( tok, destructor ) )
                memberfunction = true;
        }

        // Parse member function..
        if ( indentlevel > 0 && memberfunction )
        {
            // Allocate..
            if ( Match( tok, varname_eq.str().c_str() ) )
            {
                AllocType alloc = GetAllocationType( gettok( tok, 2 ) );
                if ( alloc != No )
                {
                    if ( Dealloc != No && Dealloc != alloc )
                        MismatchError( tok, FullVariableName.str().c_str() );
                    if ( Alloc != No && Alloc != alloc )
                        MismatchError( tok, FullVariableName.str().c_str() );
                    Alloc = alloc;
                }
            }

            // Deallocate..
            const char *varnames[2] = { "var", 0 };
            varnames[0] = varname;
            AllocType dealloc = GetDeallocationType( tok, varnames );
            if ( dealloc != No )
            {
                if ( Dealloc != No && Dealloc != dealloc )
                    MismatchError( tok, FullVariableName.str().c_str() );
                if ( Alloc != No && Alloc != dealloc )
                    MismatchError( tok, FullVariableName.str().c_str() );
                Dealloc = dealloc;
            }
        }
    }

    if ( Alloc != No && Dealloc == No )
    {
        MemoryLeak( tokens, FullVariableName.str().c_str() );
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






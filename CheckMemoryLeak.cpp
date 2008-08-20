
#include "CheckMemoryLeak.h"

#include "tokenize.h"

#include "CommonCheck.h"

#include <stdlib.h> // free

#include <vector>
#include <sstream>

#ifdef __BORLANDC__
#include <mem.h>     // <- memset
#else
#include <string.h>
#endif

//---------------------------------------------------------------------------


static bool isclass( const std::string &typestr )
{
    if ( typestr == "char" ||
         typestr == "short" ||
         typestr == "int" ||
         typestr == "long" ||
         typestr == "float" ||
         typestr == "double" )
        return false;

    std::ostringstream pattern;
    pattern << "struct " << typestr << " [;{]";
    if ( findmatch( tokens, pattern.str().c_str() ) )
        return false;

    return true;
}
//---------------------------------------------------------------------------


enum AllocType { No, Malloc, New, NewA };

// Extra allocation..
class AllocFunc
{
    public:
        const char *funcname;
        AllocType   alloctype;

        AllocFunc(const char f[], AllocType a)
        {
            funcname = f;
            alloctype = a;
        }
};
static std::list<AllocFunc> listallocfunc;

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

    // Userdefined allocation function..
    std::list<AllocFunc>::const_iterator it = listallocfunc.begin();
    while ( it != listallocfunc.end() )
    {
        if ( strcmp(tok2->str, it->funcname) == 0 )
            return it->alloctype;
        ++it;
    }

    return No;
}

static AllocType GetDeallocationType( const TOKEN *tok, const char *varnames[] )
{
    // Redundant condition..
    if ( Match(tok, "if ( %var1% )", varnames) )
    {
        tok = gettok( tok, 4 );
        if ( Match(tok,"{") )
            tok = tok->next;
    }

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

static void CheckMemoryLeak_CheckScope_All( const TOKEN *Tok1, const char varname[] )
{
    const char *varnames[2];
    varnames[0] = varname;
    varnames[1] = 0;

    AllocType Alloc = No;

    int alloc_indentlevel = -1;
    int dealloc_indentlevel = -1;
    std::list<int> loop_indentlevel;
    std::list<int> switch_indentlevel;

    bool isif = false;

    int indentlevel = 0;
    for (const TOKEN *tok = Tok1 ; tok; tok = tok->next )
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
        {
            indentlevel--;
            if ( indentlevel < 0 )
            {
                if ( Alloc != No )
                    MemoryLeak( tok, varname );
                return;
            }

            if ( !loop_indentlevel.empty() && indentlevel <= loop_indentlevel.back() )
                loop_indentlevel.pop_back();

            if ( !switch_indentlevel.empty() && indentlevel <= switch_indentlevel.back() )
                switch_indentlevel.pop_back();

            if ( indentlevel < alloc_indentlevel )
                alloc_indentlevel = -1;

            if ( indentlevel < dealloc_indentlevel )
                dealloc_indentlevel = -1;
        }

        if ( Alloc != No && Match(tok, ". %var% (") )
        {
            bool isused = false;
            while (tok && !Match(tok, "[;{]"))
            {
                if ( Match(tok, "[(,] %var1% [,)]", varnames) )
                    isused = true;
                tok = tok->next;
            }

            // Don't know what happens, assume that it's deallocated.
            if ( isused )
            {
                if ( indentlevel == 0 )
                    return;

                dealloc_indentlevel = indentlevel;
            }
        }

        // Check subfunction...
        if (Alloc != No && Match(tok,"[{};] %var% ("))
        {
            AllocType dealloc = GetDeallocationType(tok->next, varnames);

            const char *funcname = getstr( tok, 1 );
            if (dealloc==No && strcmp(funcname,"if") && strcmp(funcname,"for") && strcmp(funcname,"while"))
            {
                unsigned int param = 0;
                for (const TOKEN *tok2 = gettok(tok,2); tok2; tok2 = tok2->next)
                {
                    if ( tok2->str[0] == ';' || tok2->str[0] == ')' )
                        break;
                    if ( tok2->str[0] == ',' )
                        param++;

                    if ( Match(tok2, "[(,] %var1% [,)]", varnames) )
                    {
                        // Find function..
                        const TOKEN *ftok = GetFunctionTokenByName( funcname );
                        ftok = gettok(ftok,2);
                        if ( ! ftok )
                        {
                            // Can't find the function but to avoid false
                            // positives it is assumed that the variable is
                            // deallocated..

                            // Deallocated at same indentlevel as the allocation => no memory leak
                            if ( alloc_indentlevel == indentlevel )
                                return;

                            dealloc_indentlevel = indentlevel;
                        }

                        else
                        {
                            // Goto function parameter..
                            for ( unsigned int fparam = 0; ftok && fparam < param; ftok = ftok->next )
                            {
                                if ( ftok->str[0] == ',' )
                                    ++fparam;
                            }
                            for ( ; ftok; ftok = ftok->next )
                            {
                                if ( ! Match(ftok,"%var% [,)]") )
                                    continue;

                                const char *paramname[2] = {0};
                                paramname[0] = ftok->str;
                                // parse function and check if it deallocates the parameter..
                                int _indentlevel = 0;
                                while (_indentlevel>=0 && ftok)
                                {
                                    if ( ftok->str[0] == '{' )
                                        _indentlevel++;
                                    else if ( ftok->str[0] == '}' )
                                    {
                                        _indentlevel--;
                                        if ( _indentlevel <= 0 )
                                            break;
                                    }

                                    if ( _indentlevel >= 1 )
                                    {
                                        AllocType dealloc = GetDeallocationType(ftok,paramname);
                                        if ( dealloc != No )
                                        {
                                            if ( Alloc != No && Alloc != dealloc )
                                            {
                                                MismatchError( Tok1, varname );
                                                return;
                                            }

                                            // Deallocated at same indentlevel as the allocation => no memory leak
                                            if ( alloc_indentlevel == indentlevel )
                                                return;

                                            dealloc_indentlevel = indentlevel;
                                            break;
                                        }
                                    }

                                    ftok = ftok->next;
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }

        // for, while set loop level..
        if ( Match(tok,"while") || Match(tok,"for") )
            loop_indentlevel.push_back( indentlevel );

        // switch..
        if (Match(tok,"switch"))
            switch_indentlevel.push_back( indentlevel );

        // Skip stuff like: if (!var) ...
        if ( Match(tok, "if ( ! %var1% )", varnames) ||
             Match(tok, "if ( unlikely ( ! %var1% ) )", varnames) ||
             Match(tok, "if ( %var1% == NULL )", varnames) ||
             Match(tok, "if ( NULL == %var1% )", varnames) ||
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

        // if..
        if ( Match(tok,"if") )
            isif = true;
        if ( strchr(";{}", tok->str[0]) )
            isif = false;

        // Allocated..
        if ( Match(tok, "[(;{}] %var1% =", varnames) )
        {
            AllocType alloc = GetAllocationType( gettok(tok, 3) );
            if ( alloc != No )
            {
                tok = tok->next;
                Alloc = alloc;
                alloc_indentlevel = indentlevel;

                if ( isif )
                {
                    while ( tok )
                    {
                        if ( tok->str[0] == '{' )
                        {
                            indentlevel++;
                        }
                        else if ( tok->str[0] == '}' )
                        {
                            indentlevel--;
                            if ( indentlevel <= alloc_indentlevel )
                                break;
                        }
                        else if ( tok->str[0] == ';' && indentlevel == alloc_indentlevel )
                        {
                            break;
                        }
                        tok = tok->next;
                    }
                }
            }
        }


        // Deallocated..
        AllocType dealloc = GetDeallocationType( tok, varnames );
        if ( dealloc != No )
        {
            if ( Alloc != No && Alloc != dealloc )
            {
                MismatchError( tok, varname );
                return;
            }

            // Deallocated at same indentlevel as the allocation => no memory leak
            if ( alloc_indentlevel == indentlevel )
                return;

            dealloc_indentlevel = indentlevel;
            while ( tok && tok->str[0] != ';' )
                tok = tok->next;
        }

        // Used..
        //     list.push_back( var1 );
        //     listtail->next = var1;
        if ( Match( tok, "[=] %var1% [;]", varnames ) )
        {
            return;
        }
        if ( Match( tok, "[=,(] ( %type% * ) %var1% [,);]", varnames ) )
        {
            return;
        }
        if ( Match( tok, "[=,(] ( %type% %type% * ) %var1% [,);]", varnames ) )
        {
            return;
        }

        // Used. Todo: check if "p" is the first member in the struct.
        //     p = &var1->p;
        if ( Match( tok, "= & %var1% . %var% ;", varnames ) )
        {
            return;
        }

        // Linux lists.. todo: check if the first struct member is passed
        if ( Match( tok, "%var% ( & %var1% .", varnames ) ||
             Match( tok, ", & %var1% .", varnames ) )
        {
            return;
        }

        // continue/break loop..
        if (Alloc != No &&
            loop_indentlevel.empty() &&
            switch_indentlevel.empty() &&
            (Match(tok,"continue") || Match(tok,"break")))
        {
            MemoryLeak( tok, varname );
            return;
        }

        // Return without deallocating the memory..
        if ( Alloc != No && (indentlevel==0 || (alloc_indentlevel >= 0 && dealloc_indentlevel <= 0)) && Match(tok, "return") )
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

            else
            {
                // The allocated memory is returned.. check that it is deallocated

                // Get function name..
                const char *funcname = 0;
                int indentlevel = 0;
                for ( const TOKEN *ftok = tokens; ftok && ftok != tok; ftok = ftok->next )
                {
                    if ( ftok->str[0] == '{' )
                        indentlevel++;

                    else if ( ftok->str[0] == '}' )
                        indentlevel--;

                    if ( indentlevel <= 0 )
                    {
                        if ( Match(ftok, "[};]") )
                            funcname = 0;
                        else if ( Match(ftok, "%var% (") )
                            funcname = ftok->str;
                    }
                }

                if ( funcname )
                {
                    listallocfunc.push_back( AllocFunc(funcname, Alloc) );
                }
            }

            if ( indentlevel == 0 )
                return;

            if ( indentlevel <= alloc_indentlevel )
            {
                Alloc = No;
                alloc_indentlevel = -1;
                dealloc_indentlevel = -1;
            }
        }
    }
}
//---------------------------------------------------------------------------

extern bool ShowAll;

static TOKEN *getcode(const TOKEN *tok, const char varname[])
{
    const char *varnames[2];
    varnames[0] = varname;
    varnames[1] = 0;

    TOKEN *rethead = 0, *rettail = 0;
    #define addtoken(_str)                  \
    {                                       \
        TOKEN *newtok = new TOKEN;          \
        newtok->str = strdup(_str);         \
        newtok->linenr = tok->linenr;       \
        newtok->FileIndex = tok->FileIndex; \
        newtok->next = 0;                   \
        if (rettail)                        \
            rettail->next = newtok;         \
        else                                \
            rethead = newtok;               \
        rettail=newtok;                     \
    }

    AllocType alloctype = No;
    AllocType dealloctype = No;

    int indentlevel = 0;
    int parlevel = 0;
    for ( ; tok; tok = tok->next )
    {
        if ( tok->str[0] == '{' )
        {
            addtoken( "{" );
            indentlevel++;
        }
        else if ( tok->str[0] == '}' )
        {
            addtoken( "}" );
            if ( indentlevel <= 0 )
                break;
            indentlevel--;
        }

        if ( tok->str[0] == '(' )
            parlevel++;
        else if ( tok->str[0] == ')' )
            parlevel--;

        if ( parlevel == 0 && tok->str[0]==';')
            addtoken(";");

        if (Match(tok, "[(;{}] %var1% = ", varnames))
        {
            AllocType alloc = GetAllocationType(gettok(tok,3));

            // If "--all" hasn't been given, don't check classes..
            if ( alloc == New && ! ShowAll )
            {
                if ( Match(gettok(tok,3), "new %var% ;") )
                {
                    if ( isclass( getstr(tok, 4) ) )
                        alloc = No;
                }
            }

            if ( alloc != No )
            {
                addtoken("alloc");
                if (alloctype!=No && alloctype!=alloc)
                    MismatchError(tok, varname);
                if (dealloctype!=No && dealloctype!=alloc)
                    MismatchError(tok, varname);
                alloctype = alloc;
            }
        }

        AllocType dealloc = GetDeallocationType(tok, varnames);
        if ( dealloc != No )
        {
            addtoken("dealloc");
            if (alloctype!=No && alloctype!=dealloc)
                MismatchError(tok, varname);
            if (dealloctype!=No && dealloctype!=dealloc)
                MismatchError(tok, varname);
            dealloctype = dealloc;
        }

        // if else switch
        if ( Match(tok, "if ( %var1% )", varnames) ||
             Match(tok, "if ( %var1% != NULL )", varnames)   )
        {
            addtoken("if(var)");
        }
        else if ( Match(tok, "if ( ! %var1% )", varnames) ||
                  Match(tok, "if ( unlikely ( ! %var1% ) )", varnames) ||
                  Match(tok, "if ( unlikely ( %var1% == NULL ) )", varnames) ||
                  Match(tok, "if ( %var1% == NULL )", varnames) ||
                  Match(tok, "if ( NULL == %var1% )", varnames) ||
                  Match(tok, "if ( %var1% == 0 )", varnames) )
        {
            addtoken("if(!var)");
        }
        else
        {
            if (Match(tok, "if"))
                addtoken("if");
            if (Match(tok, "else"))
                addtoken("else");
            if (Match(tok, "switch"))
                addtoken("switch");
        }

        // Loops..
        if ( Match(tok, "for") )
            addtoken("loop");
        if ( Match(tok, "while") )
            addtoken("loop");
        if ( Match(tok, "do") )
            addtoken("loop");

        // continue / break..
        if ( Match(tok, "continue") )
            addtoken("continue");
        if ( Match(tok, "break") )
            addtoken("break");

        // Return..
        if ( Match(tok, "return") )
        {
            addtoken("return");
            if ( Match(tok, "return %var1%", varnames) ||
                 Match(tok, "return & %var1%", varnames) )
                addtoken("use");
        }

        // Assignment..
        if ( Match(tok,"[)=] %var1%", varnames) )
            addtoken("use");

        // Function parameter..
        if ( Match(tok, "[(,] %var1% [,)]", varnames) )
            addtoken("use");

        // Linux lists..
        if ( Match( tok, "[=(,] & %var1% [.[]", varnames ) )
            addtoken("use");
    }

    return rethead;
}

static void erase(TOKEN *begin, const TOKEN *end)
{
    if ( ! begin )
        return;

    while ( begin->next && begin->next != end )
    {
        TOKEN *next = begin->next;
        begin->next = begin->next->next;
        free(next->str);
        delete next;
    }
}


// Simpler but less powerful than "CheckMemoryLeak_CheckScope_All"
static void CheckMemoryLeak_CheckScope( const TOKEN *Tok1, const char varname[] )
{
    if ( ShowAll )
    {
        CheckMemoryLeak_CheckScope_All(Tok1, varname);
        return;
    }

    TOKEN *tok = getcode( Tok1, varname );

    // If the variable is not allocated at all => no memory leak
    if (findmatch(tok, "alloc") == 0)
    {
        deleteTokens(tok);
        return;
    }


    // reduce the code..
    bool done = false;
    while ( ! done )
    {
        done = true;

        for (TOKEN *tok2 = tok ; tok2; tok2 = tok2->next )
        {
            // Delete extra ";"
            while (Match(tok2,"[;{}] ;"))
            {
                erase(tok2, gettok(tok2,2));
                done = false;
            }

            // Replace "{ }" with ";"
            if ( Match(tok2->next, "{ }") )
            {
                tok2->next->str[0] = ';';
                erase(tok2->next, gettok(tok2,3));
                done = false;
            }

            // Delete braces around a single instruction..
            if ( Match(tok2->next, "{ %var% ; }") )
            {
                erase( tok2, gettok(tok2,2) );
                erase( tok2->next->next, gettok(tok2,4) );
                done = false;
            }
            if ( Match(tok2->next, "{ return use ; }") )
            {
                erase( tok2, gettok(tok2,2) );
                erase( tok2->next->next->next, gettok(tok2,5) );
                done = false;
            }

            // Delete empty if
            if ( Match(tok2,"[;{}] if ;") ||
                 Match(tok2,"[;{}] if(var) ;") ||
                 Match(tok2,"[;{}] if(!var) ;") )
            {
                if ( ! Match(gettok(tok2,3), "else") )
                {
                    erase(tok2, gettok(tok2, 3));
                    done = false;
                    continue;
                }
            }

            // Delete "else ;" and "else if ;"
            if ( Match(tok2->next, "else if ;") )
            {
                erase(tok2, gettok(tok2,4));
                done = false;
            }
            if ( Match(tok2->next, "else ;") )
            {
                erase(tok2, gettok(tok2,3));
                done = false;
            }

            // Delete if block: "alloc; if return use ;"
            if (Match(tok2,"alloc ; if return use ;") && !Match(gettok(tok2,6),"else"))
            {
                erase(tok2, gettok(tok2,5));
                done = false;
            }
            
            // "[;{}] if alloc ; else return ;" => "[;{}] alloc ;"
            if (Match(tok2,"[;{}] if alloc ; else return ;"))
            {
                erase(tok2, gettok(tok2,2));        // Remove "if"
                erase(tok2->next, gettok(tok2,5));  // Remove "; else return"
                done = false;
            }

            // Delete "loop ;"
            if ( Match(tok2->next, "loop ;") )
            {
                erase(tok2, gettok(tok2,3));
                done = false;
            }

            // Delete if block in "alloc ; if(!var) return ;"
            if ( Match(tok2, "alloc ; if(!var) return ;") )
            {
                erase(tok2, gettok(tok2,4));
                done = false;
            }

            // Delete second use in "use ; use;"
            while (Match(tok2, "use ; use ;"))
            {
                erase(tok2, gettok(tok2,3));
                done = false;
            }
        }
    }


    if ( findmatch(tok, "alloc ; if continue ;") )
    {
        MemoryLeak(gettok(findmatch(tok, "alloc ; if continue ;"), 3), varname);
    }

    else if ( findmatch(tok, "alloc ; if return ;") )
    {
        MemoryLeak(gettok(findmatch(tok, "alloc ; if return ;"), 3), varname);
    }

    else if ( findmatch(tok, "alloc ; return ;") )
    {
        MemoryLeak(gettok(findmatch(tok,"alloc ; return ;"),2), varname);
    }

    else if ( ! findmatch(tok,"dealloc") &&
              ! findmatch(tok,"use") &&
              ! findmatch(tok,"return use ;") )
    {
        const TOKEN *last = tok;
        while (last->next)
            last = last->next;
        MemoryLeak(last, varname);
    }

    deleteTokens(tok);
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
            {
                if (ShowAll || !isclass(getstr(tok,1)))
                    CheckMemoryLeak_ClassMembers_Variable( classname, getstr(tok, 3) );
            }
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
    listallocfunc.clear();

    // Check for memory leaks inside functions..
    CheckMemoryLeak_InFunction();

    // Check that all class members are deallocated..
    CheckMemoryLeak_ClassMembers();
}
//---------------------------------------------------------------------------






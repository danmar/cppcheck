/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


#include "checkmemoryleak.h"

#include <stdlib.h> // free

#include <algorithm>

#include <iostream>
#include <sstream>

#ifdef __BORLANDC__
#include <mem.h>     // <- memset
#else
#include <string.h>
#endif


//---------------------------------------------------------------------------

CheckMemoryLeakClass::CheckMemoryLeakClass( const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger )
 : _settings(settings)
{
    _tokenizer = tokenizer;
    _errorLogger = errorLogger;
}

CheckMemoryLeakClass::~CheckMemoryLeakClass()
{

}

bool CheckMemoryLeakClass::isclass( const TOKEN *tok )
{
    if ( tok->isStandardType() )
        return false;

    std::ostringstream pattern;
    pattern << "struct " << tok->str();
    if ( TOKEN::findmatch( _tokenizer->tokens(), pattern.str().c_str() ) )
        return false;

    return true;
}
//---------------------------------------------------------------------------

CheckMemoryLeakClass::AllocType CheckMemoryLeakClass::GetAllocationType( const TOKEN *tok2 )
{
    // What we may have...
    //     * var = (char *)malloc(10);
    //     * var = new char[10];
    //     * var = strdup("hello");
    if ( tok2 && tok2->str() == "(" )
    {
        while ( tok2 && tok2->str() != ")" )
            tok2 = tok2->next();
        tok2 = tok2 ? tok2->next() : NULL;
    }
    if ( ! tok2 )
        return No;
    if ( ! tok2->isName() )
        return No;

    // Does tok2 point on "malloc", "strdup" or "kmalloc"..
    const char *mallocfunc[] = {"malloc",
                                "calloc",
                                "strdup",
                                "kmalloc",
                                "kzalloc",
                                "kcalloc",
                                0};
    for ( unsigned int i = 0; mallocfunc[i]; i++ )
    {
        if ( tok2->str() == mallocfunc[i] )
            return Malloc;
    }

    // Does tok2 point on "g_malloc", "g_strdup", ..
    const char *gmallocfunc[] = {"g_new",
                                 "g_new0",
                                 "g_try_new",
                                 "g_try_new0",
                                 "g_malloc",
                                 "g_malloc0",
                                 "g_try_malloc",
                                 "g_try_malloc0",
                                 "g_strdup",
                                 "g_strndup",
                                 0};
    for ( unsigned int i = 0; gmallocfunc[i]; i++ )
    {
        if ( tok2->str() == gmallocfunc[i] )
            return gMalloc;
    }

    if ( TOKEN::Match( tok2, "new %type% [;(]" ) )
        return New;

    if ( TOKEN::Match( tok2, "new %type% [" ) )
        return NewA;

    if ( TOKEN::Match( tok2, "fopen (" ) )
        return FOPEN;

    if ( TOKEN::Match( tok2, "popen (" ) )
        return POPEN;

    // Userdefined allocation function..
    std::list<AllocFunc>::const_iterator it = _listAllocFunc.begin();
    while ( it != _listAllocFunc.end() )
    {
        if ( tok2->str() == it->funcname )
            return it->alloctype;
        ++it;
    }

    return No;
}



CheckMemoryLeakClass::AllocType CheckMemoryLeakClass::GetReallocationType( const TOKEN *tok2 )
{
    // What we may have...
    //     * var = (char *)realloc(..;
    if ( tok2 && tok2->str() == "(" )
    {
        while ( tok2 && tok2->str() != ")" )
            tok2 = tok2->next();
        tok2 = tok2 ? tok2->next() : NULL;
    }
    if ( ! tok2 )
        return No;

    if ( TOKEN::Match(tok2, "realloc") )
        return Malloc;

    // GTK memory reallocation..
    if ( TOKEN::Match(tok2, "g_realloc|g_try_realloc|g_renew|g_try_renew") )
        return gMalloc;

    return No;
}


CheckMemoryLeakClass::AllocType CheckMemoryLeakClass::GetDeallocationType( const TOKEN *tok, const char *varnames[] )
{
    if ( TOKEN::Match(tok, "delete %var1% ;", varnames) )
        return New;

    if ( TOKEN::Match(tok, "delete [ ] %var1% ;", varnames) )
        return NewA;

    if ( TOKEN::Match(tok, "free ( %var1% ) ;", varnames) ||
         TOKEN::Match(tok, "kfree ( %var1% ) ;", varnames) )
    {
        return Malloc;
    }

    if ( TOKEN::Match(tok, "g_free ( %var1% ) ;", varnames) )
        return gMalloc;

    if ( TOKEN::Match(tok, "fclose ( %var1% )", varnames) )
        return FOPEN;

    if ( TOKEN::Match(tok, "pclose ( %var1% )", varnames) )
        return POPEN;

    return No;
}
//--------------------------------------------------------------------------

const char * CheckMemoryLeakClass::call_func( const TOKEN *tok, std::list<const TOKEN *> callstack, const char *varnames[], AllocType &alloctype, AllocType &dealloctype )
{
    // Keywords that are not function calls..
    if (TOKEN::Match(tok,"if|for|while"))
        return 0;

    // String functions that are not allocating nor deallocating memory..
    if (TOKEN::Match(tok, "strcpy|strncpy|strcat|strncat|strcmp|strncmp|strcasecmp|stricmp|sprintf|strchr|strrchr|strstr"))
        return 0;

    // Memory functions that are not allocating nor deallocating memory..
    if (TOKEN::Match(tok, "memset|memcpy|memmove|memchr"))
        return 0;

    // I/O functions that are not allocating nor deallocating memory..
    if (TOKEN::Match(tok, "fgets|fgetc|fputs|fputc|printf"))
        return 0;

    // Convert functions that are not allocating nor deallocating memory..
    if (TOKEN::Match(tok, "atoi|atof|atol|strtol|strtoul|strtod"))
        return 0;

    if (GetAllocationType(tok)!=No || GetReallocationType(tok)!=No || GetDeallocationType(tok,varnames)!=No)
        return 0;

    if ( callstack.size() > 2 )
        return "dealloc";

    const char *funcname = tok->aaaa();
    for ( std::list<const TOKEN *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it )
    {
        if ( (*it)->str() == funcname )
            return "dealloc";
    }
    callstack.push_back(tok);

    int par = 1;
    int parlevel = 0;
    for ( ; tok; tok = tok->next() )
    {
        if ( tok->str() == "(" )
            ++parlevel;
        else if ( tok->str() == ")" )
        {
            --parlevel;
            if ( parlevel < 1 )
                return NULL;
        }

        if ( parlevel == 1 )
        {
            if ( tok->str() == "," )
                ++par;
            if ( TOKEN::Match(tok, "[,()] %var1% [,()]", varnames) )
            {
                const TOKEN *ftok = _tokenizer->GetFunctionTokenByName(funcname);
                const char *parname = Tokenizer::getParameterName( ftok, par );
                if ( ! parname )
                    return "use";
                // Check if the function deallocates the variable..
                while ( ftok && (ftok->str() != "{") )
                    ftok = ftok->next();
                TOKEN *func = getcode( ftok->tokAt(1), callstack, parname, alloctype, dealloctype );
                simplifycode( func );
                const TOKEN *func_ = func;
                while ( func_ && func_->str() == ";" )
                    func_ = func_->next();
                /*
                for (const TOKEN *t = func; t; t = t->next())
                {
                    std::cout << t->str() << "\n";
                }*/

                const char *ret = 0;
                if (TOKEN::findmatch(func_, "goto"))
                {
                    // TODO : "goto" isn't handled well
                    if ( TOKEN::findmatch(func_, "dealloc") )
                        ret = "dealloc";
                    else if ( TOKEN::findmatch(func_, "use") )
                        ret = "use";
                }
                else if ( TOKEN::findmatch(func_, "dealloc") )
                    ret = "dealloc";
                else if ( TOKEN::findmatch(func_, "use") )
                    ret = "use";

                Tokenizer::deleteTokens(func);
                return ret;
            }
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------

void CheckMemoryLeakClass::MismatchError( const TOKEN *Tok1, const std::list<const TOKEN *> &callstack, const char varname[] )
{
    std::ostringstream errmsg;
    for ( std::list<const TOKEN *>::const_iterator tok = callstack.begin(); tok != callstack.end(); ++tok )
        errmsg << _tokenizer->fileLine(*tok) << " -> ";
    errmsg << _tokenizer->fileLine(Tok1) << ": Mismatching allocation and deallocation: " << varname;
    _errorLogger->reportErr( errmsg.str() );
}
//---------------------------------------------------------------------------

void CheckMemoryLeakClass::MemoryLeak( const TOKEN *tok, const char varname[], AllocType alloctype )
{
    std::ostringstream errmsg;
    errmsg << _tokenizer->fileLine(tok);

    if( alloctype == CheckMemoryLeakClass::FOPEN ||
        alloctype == CheckMemoryLeakClass::POPEN )
        errmsg << ": Resource leak: ";
    else
        errmsg << ": Memory leak: ";

    errmsg << varname;
    _errorLogger->reportErr( errmsg.str() );
}
//---------------------------------------------------------------------------

void CheckMemoryLeakClass::instoken(TOKEN *tok, const char str[])
{
    tok->insertToken( str );
}
//---------------------------------------------------------------------------

bool CheckMemoryLeakClass::notvar(const TOKEN *tok, const char *varnames[])
{
    return bool( TOKEN::Match(tok, "! %var1% [;)&|]", varnames) ||
                 TOKEN::Match(tok, "! ( %var1% )", varnames) ||
                 TOKEN::Match(tok, "unlikely ( ! %var1% )", varnames) ||
                 TOKEN::Match(tok, "unlikely ( %var1% == 0 )", varnames) ||
                 TOKEN::Match(tok, "0 == %var1% [;)&|]", varnames) ||
                 TOKEN::Match(tok, "%var1% == 0", varnames) );
}

TOKEN *CheckMemoryLeakClass::getcode(const TOKEN *tok, std::list<const TOKEN *> callstack, const char varname[], AllocType &alloctype, AllocType &dealloctype)
{
    const char *varnames[2];
    varnames[0] = varname;
    varnames[1] = 0;

    TOKEN *rethead = 0, *rettail = 0;
    #define addtoken(_str)                  \
    {                                       \
        if (rettail)                        \
        {                                   \
            rettail->insertToken(_str);     \
            rettail = rettail->next();      \
        }                                   \
        else                                \
        {                                   \
            rethead = new TOKEN;            \
            rettail = rethead;              \
            rettail->str(_str);             \
        }                                   \
                                            \
        rettail->linenr( tok->linenr() );   \
        rettail->fileIndex( tok->fileIndex() ); \
    }

    // The first token should be ";"
    addtoken(";");

    bool isloop = false;

    int indentlevel = 0;
    int parlevel = 0;
    for ( ; tok; tok = tok->next() )
    {
        if ( tok->str() == "{" )
        {
            addtoken( "{" );
            indentlevel++;
        }
        else if ( tok->str() == "}" )
        {
            addtoken( "}" );
            if ( indentlevel <= 0 )
                break;
            indentlevel--;
        }

        if ( tok->str() == "(" )
            parlevel++;
        else if ( tok->str() == ")" )
            parlevel--;
        isloop &= ( parlevel > 0 );

        if ( parlevel == 0 && tok->str()==";")
            addtoken(";");

        if (TOKEN::Match(tok, "[(;{}] %var1% =", varnames))
        {
            AllocType alloc = GetAllocationType(tok->tokAt(3));

            if ( alloc == No )
            {
                alloc = GetReallocationType( tok->tokAt(3) );
                if ( alloc != No )
                {
                    addtoken( "dealloc" );
                    addtoken( ";" );
                }
            }

            // If "--all" hasn't been given, don't check classes..
            if ( alloc == New && ! _settings._showAll )
            {
                if ( TOKEN::Match(tok->tokAt(3), "new %type% [(;]") )
                {
                    if ( isclass( tok->tokAt(4) ) )
                        alloc = No;
                }
            }

            if ( alloc != No )
            {
                addtoken("alloc");
                if (alloctype!=No && alloctype!=alloc)
                    MismatchError(tok, callstack, varname);
                if (dealloctype!=No && dealloctype!=alloc)
                    MismatchError(tok, callstack, varname);
                alloctype = alloc;
            }

            // assignment..
            else
            {
                // is the pointer in rhs?
                bool rhs = false;
                std::string pattern("[=+] " + std::string(varname));
                for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next() )
                {
                    if ( tok2->str() == ";" )
                        break;

                    if ( TOKEN::Match(tok2, pattern.c_str()) )
                    {
                        rhs = true;
                        break;
                    }
                }

                addtoken( (rhs ? "use" : "assign") );
            }
        }

        if ( TOKEN::Match(tok->previous(), "[;{})] %var%") )
        {
            AllocType dealloc = GetDeallocationType(tok, varnames);
            if ( dealloc != No )
            {
                addtoken("dealloc");
                if (alloctype!=No && alloctype!=dealloc)
                    MismatchError(tok, callstack, varname);
                if (dealloctype!=No && dealloctype!=dealloc)
                    MismatchError(tok, callstack, varname);
                dealloctype = dealloc;
                continue;
            }
        }

        // if else switch
        if ( TOKEN::Match(tok, "if ( %var1% )", varnames) ||
             TOKEN::Match(tok, "if ( %var1% != 0 )", varnames) ||
             TOKEN::Match(tok, "if ( 0 != %var1% )", varnames)  )
        {
            addtoken("if(var)");

            // Make sure the "use" will not be added
            while ( tok->str() != ")" )
                tok = tok->next();
        }
        else if ( TOKEN::Match(tok, "if (") && notvar(tok->tokAt(2), varnames) )
        {
            addtoken("if(!var)");
        }
        else if ( TOKEN::Match(tok, "if") )
        {
            // Check if the condition depends on var somehow..
            bool dep = false;
            int parlevel = 0;
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next() )
            {
                if ( tok2->str() == "(" )
                    ++parlevel;
                if ( tok2->str() == ")" )
                {
                    --parlevel;
                    if ( parlevel <= 0 )
                        break;
                }
                if ( TOKEN::Match(tok2, "fclose ( %var1% )", varnames) )
                {
                    addtoken( "dealloc" );
                    addtoken( ";" );
                    dep = true;
                    break;
                }
                if ( (tok2->str() != ".") &&
                     TOKEN::Match(tok2->next(), "%var1%", varnames) &&
                     !TOKEN::Match(tok2->next(), "%var1% .", varnames) )
                {
                    dep = true;
                    break;
                }
            }
            addtoken( (dep ? "ifv" : "if") );
        }
        else if ( (tok->str() == "else") || (tok->str() == "switch") )
        {
            addtoken(tok->aaaa());
        }

        if ( (tok->str() == "case") )
        {
            addtoken("case");
            addtoken(";");
        }

        if ( (tok->str() == "default") )
        {
            addtoken("case");
            addtoken(";");
        }

        // Loops..
        if ((tok->str() == "for") || (tok->str() == "while") )
        {
            addtoken("loop");
            isloop = true;
        }
        if ( (tok->str() == "do") )
        {
            addtoken("do");
        }
        if ( isloop && notvar(tok,varnames) )
            addtoken("!var");

        // continue / break..
        if ( tok->str() == "continue" )
            addtoken("continue");
        if ( tok->str() == "break" )
            addtoken("break");

        // goto..
        if ( tok->str() == "goto" )
        {
            addtoken("goto");
        }

        // Return..
        if ( tok->str() == "return" )
        {
            addtoken("return");
            if ( TOKEN::Match(tok, "return %var1%", varnames) ||
                 TOKEN::Match(tok, "return & %var1%", varnames) )
                addtoken("use");
            if (TOKEN::simpleMatch(tok->next(), "("))
            {
                for (const TOKEN *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next() )
                {
                    if ( tok2->str() == "(" || tok2->str() == ")" )
                        break;

                    if ( tok2->str() == varname )
                    {
                        addtoken("use");
                        break;
                    }
                }
            }
        }

        // throw..
        if ( TOKEN::Match(tok, "try|throw|catch") )
            addtoken(tok->strAt(0));

        // Assignment..
        if ( TOKEN::Match(tok,"[)=] %var1% [+;)]", varnames) ||
             TOKEN::Match(tok, "%var1% +=|-=", varnames) ||
             TOKEN::Match(tok, "+=|<< %var1% ;", varnames) )
            addtoken("use");

        // Investigate function calls..
        if ( TOKEN::Match(tok, "%var% (") )
        {
            const char *str = call_func(tok, callstack, varnames, alloctype, dealloctype);
            if ( str )
                addtoken( str );
        }

        // Callback..
        if ( TOKEN::Match(tok, "( * %var% ) (") ||
             TOKEN::Match(tok, "( %var% ) (") )
        {
            for ( const TOKEN *tok2 = tok->tokAt(4); tok2; tok2 = tok2->next() )
            {
                if ( TOKEN::Match(tok2, "[;{]") )
                    break;
                else if ( tok2->str() == varname )
                {
                    addtoken("use");
                    break;
                }
            }
        }

        // Linux lists..
        if ( TOKEN::Match( tok, "[=(,] & %var1% [.[]", varnames ) )
        {
            // todo: better checking
            addtoken("&use");
        }
    }

    return rethead;
}

void CheckMemoryLeakClass::erase(TOKEN *begin, const TOKEN *end)
{
    TOKEN::eraseTokens( begin, end );
}

void CheckMemoryLeakClass::simplifycode(TOKEN *tok)
{
    // Replace "throw" that is not in a try block with "return"
    int indentlevel = 0;
    int trylevel = -1;
    for (TOKEN *tok2 = tok; tok2; tok2 = tok2->next())
    {
        if ( tok2->str() == "{" )
            ++indentlevel;
        else if ( tok2->str() == "}" )
        {
            --indentlevel;
            if ( indentlevel <= trylevel )
                trylevel = -1;
        }
        else if ( trylevel == -1 && tok2->str() == "try" )
            trylevel = indentlevel;
        else if ( trylevel == -1 && tok2->str() == "throw" )
            tok2->str("return");
    }

    // reduce the code..
    bool done = false;
    while ( ! done )
    {
        done = true;

        for ( TOKEN *tok2 = tok; tok2; tok2 = tok2 ? tok2->next() : NULL )
        {
            // Delete extra ";"
            while (TOKEN::Match(tok2,"[;{}] ;"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Replace "{ }" with ";"
            if ( TOKEN::Match(tok2->next(), "{ }") )
            {
                tok2->next()->str(";");
                erase(tok2->next(), tok2->tokAt(3));
                done = false;
            }

            // Delete braces around a single instruction..
            if ( TOKEN::Match(tok2->next(), "{ %var% ; }") )
            {
                erase( tok2, tok2->tokAt(2) );
                erase( tok2->next()->next(), tok2->tokAt(4) );
                done = false;
            }
            if ( TOKEN::Match(tok2->next(), "{ %var% %var% ; }") )
            {
                erase( tok2, tok2->tokAt(2) );
                erase( tok2->next()->next()->next(), tok2->tokAt(5) );
                done = false;
            }


            if ( TOKEN::simpleMatch(tok2->next(), "if") )
            {
                // Delete empty if that is not followed by an else
                if (TOKEN::Match(tok2->next(), "if ; !!else") )
                {
                    erase(tok2, tok2->tokAt(2));
                    done = false;
                }

                // Delete "if ; else ;"
                else if ( TOKEN::Match(tok2->next(), "if ; else ;") )
                {
                    erase( tok2, tok2->tokAt(4) );
                    done = false;
                }

                // Two "if alloc ;" after one another.. perhaps only one of them can be executed each time
                else if (!_settings._showAll && TOKEN::Match(tok2, "[;{}] if alloc ; if alloc ;"))
                {
                    erase(tok2, tok2->tokAt(4));
                    done = false;
                }

                // TODO Make this more generic. Delete "if ; else use ; use"
                else if ( TOKEN::Match(tok2, "; if ; else assign|use ; assign|use") ||
                     TOKEN::Match(tok2, "; if assign|use ; else ; assign|use")  )
                {
                    erase( tok2, tok2->tokAt(4) );
                    done = false;
                }


                // Reduce "if assign|dealloc|use ;" that is not followed by an else..
                // If "--all" has been given these are deleted
                // Otherwise, only the "if" will be deleted
                else if (TOKEN::Match(tok2, "[;{}] if assign|dealloc|use ; !!else") )
                {
                    if ( _settings._showAll )
                        erase(tok2, tok2->tokAt(3));
                    else
                        erase( tok2, tok2->tokAt(2) );
                    done = false;
                }

                // Reduce "if if" => "if"
                else if ( TOKEN::Match(tok2, "if if") )
                {
                    erase(tok2, tok2->tokAt(2));
                    done = false;
                }

                // Reduce "if return ; alloc ;" => "alloc ;"
                else if (TOKEN::Match(tok2, "[;{}] if return ; alloc ;"))
                {
                    erase(tok2, tok2->tokAt(4));
                    done = false;
                }

                // "[;{}] if alloc ; else return ;" => "[;{}] alloc ;"
                else if (TOKEN::Match(tok2,"[;{}] if alloc ; else return ;"))
                {
                    erase(tok2, tok2->tokAt(2));        // Remove "if"
                    erase(tok2->next(), tok2->tokAt(5));  // Remove "; else return"
                    done = false;
                }

                // Reduce "if ; else %var% ;" => "if %var% ;"
                else if ( TOKEN::Match(tok2->next(), "if ; else %var% ;") )
                {
                    erase( tok2->next(), tok2->tokAt(4) );
                    done = false;
                }

                // Reduce "if ; else return use ;" => "if return use ;"
                else if ( TOKEN::Match(tok2->next(), "if ; else return use ;") )
                {
                    erase( tok2->next(), tok2->tokAt(4) );
                    done = false;
                }

                // Reduce "if return ; if return ;" => "if return ;"
                else if ( TOKEN::Match(tok2->next(), "if return ; if return ;") )
                {
                    erase( tok2, tok2->tokAt(4) );
                    done = false;
                }

                // Delete first if in .. "if { dealloc|assign|use ; return ; } if return ;"
                else if ( TOKEN::Match(tok2, "[;{}] if { dealloc|assign|use ; return ; } if return ;") )
                {
                    erase(tok2, tok2->tokAt(8));
                    done = false;
                }

                // Reducing if..
                else if ( _settings._showAll )
                {
                    if (TOKEN::Match(tok2->next(), "if assign|dealloc|use ; else"))
                    {
                        erase(tok2->next(), tok2->tokAt(3));
                        done = false;
                    }
                    if (TOKEN::Match(tok2,"[;{}] if { assign|dealloc|use ; return ; } !!else") )
                    {
                        erase(tok2,tok2->tokAt(8));
                        done = false;
                    }
                }

                continue;
            }

            // Reduce "if(var) dealloc ;" and "if(var) use ;" that is not followed by an else..
            if (TOKEN::Match(tok2, "[;{}] if(var) assign|dealloc|use ; !!else") )
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Reduce "; if(!var) alloc ; !!else" => "; dealloc ; alloc ;"
            if ( TOKEN::Match(tok2, "; if(!var) alloc ; !!else") )
            {
                // Remove the "if(!var)"
                erase( tok2, tok2->tokAt(2) );

                // Insert "dealloc ;" before the "alloc ;"
                tok2->insertToken( ";" );
                tok2->insertToken( "dealloc" );

                done = false;
            }

            // Remove "catch ;"
            if ( TOKEN::simpleMatch(tok2->next(), "catch ;") )
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Reduce "if* ;" that is not followed by an else..
            if (TOKEN::Match(tok2->next(), "if(var)|if(!var)|ifv ; !!else") )
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Reduce "else ;" => ";"
            if ( TOKEN::Match(tok2->next(), "else ;") )
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Delete if block: "alloc; if return use ;"
            if (TOKEN::Match(tok2,"alloc ; if return use ; !!else") )
            {
                erase(tok2, tok2->tokAt(5));
                done = false;
            }


            // Replace "dealloc use ;" with "dealloc ;"
            if ( TOKEN::Match(tok2, "dealloc use ;") )
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Remove the "if break|continue ;" that follows "dealloc ; alloc ;"
            if ( ! _settings._showAll && TOKEN::Match(tok2, "dealloc ; alloc ; if break|continue ;") )
            {
                tok2 = tok2->next()->next()->next();
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Reduce "do { alloc ; } " => "alloc ;"
            // TODO: If the loop can be executed twice reduce to "loop alloc ;" instead
            if ( TOKEN::Match(tok2->next(), "do { alloc ; }") )
            {
                erase(tok2, tok2->tokAt(3));
                erase(tok2->next()->next(), tok2->tokAt(4));
                done = false;
            }

            // Reduce "loop if break ; => ";"
            if ( TOKEN::Match( tok2->next(), "loop if break|continue ; !!else") )
            {
                erase( tok2, tok2->tokAt(4) );
                done = false;
            }

            // Reduce "loop { assign|dealloc|use ; alloc ; if break ; }" to "assign|dealloc|use ; alloc ;"
            if ( TOKEN::Match( tok2->next(), "loop { assign|dealloc|use ; alloc ; if break|continue ; }" ) )
            {
                // erase "loop {"
                erase( tok2, tok2->tokAt(3) );
                // erase "if break|continue ; }"
                tok2 = tok2->next()->next()->next()->next();
                erase( tok2, tok2->tokAt(5) );
                done = false;
            }

            // Replace "loop { X ; break ; }" with "X ;"
            if ( TOKEN::Match(tok2->next(), "loop { %var% ; break ; }") )
            {
                erase(tok2, tok2->tokAt(3));
                erase(tok2->next()->next(), tok2->tokAt(6));
                done = false;
            }

            // Replace "loop ;" with ";"
            if ( TOKEN::Match(tok2->next(), "loop ;") )
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Replace "loop !var ;" with ";"
            if ( TOKEN::Match(tok2->next(), "loop !var ;") )
            {
                erase(tok2, tok2->tokAt(4));
                done = false;
            }

            // Replace "loop !var alloc ;" with " alloc ;"
            if ( TOKEN::Match(tok2->next(), "loop !var alloc ;") )
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Delete if block in "alloc ; if(!var) return ;"
            if ( TOKEN::Match(tok2, "alloc ; if(!var) return ;") )
            {
                erase(tok2, tok2->tokAt(4));
                done = false;
            }

            // Delete if block: "alloc; if return use ;"
            if (TOKEN::Match(tok2,"alloc ; if return use ; !!else") )
            {
                erase(tok2, tok2->tokAt(5));
                done = false;
            }

            // Reduce "[;{}] return ; %var%" => "[;{}] return ;"
            if ( TOKEN::Match(tok2, "[;{}] return ; %var%") )
            {
                erase( tok2->next()->next(), tok2->tokAt(4) );
                done = false;
            }

            // Reduce "[;{}] return use ; %var%" => "[;{}] return use ;"
            if ( TOKEN::Match(tok2, "[;{}] return use ; %var%") )
            {
                erase( tok2->next()->next()->next(), tok2->tokAt(5) );
                done = false;
            }

            // Reduce "if(var) return use ;" => "return use ;"
            if ( TOKEN::Match(tok2->next(), "if(var) return use ; !!else") )
            {
                erase( tok2, tok2->tokAt(2) );
                done = false;
            }

            // Reduce "if(var) assign|dealloc|use ;" => "assign|dealloc|use ;"
            if ( TOKEN::Match(tok2->next(), "if(var) assign|dealloc|use ; !!else") )
            {
                erase( tok2, tok2->tokAt(2) );
                done = false;
            }

            // Reduce "[;{}] alloc ; dealloc ;" => "[;{}]"
            if ( TOKEN::Match(tok2, "[;{}] alloc ; dealloc ;") )
            {
                erase( tok2, tok2->tokAt(5) );
                done = false;
            }

            // Reduce "if* alloc ; dealloc ;" => ";"
            if ( TOKEN::Match(tok2->tokAt(2), "alloc ; dealloc ;") &&
                 tok2->next()->str().find("if") == 0 )
            {
                erase( tok2, tok2->tokAt(5) );
                done = false;
            }

            // Delete second use in "use ; use ;"
            while (TOKEN::Match(tok2, "[;{}] use ; use ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Delete first part in "use ; dealloc ;"
            if (TOKEN::Match(tok2, "[;{}] use ; dealloc ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Delete first part in "use ; return use ;"
            if (TOKEN::Match(tok2, "[;{}] use ; return use ;"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Delete second case in "case ; case ;"
            while (TOKEN::Match(tok2, "case ; case ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Replace switch with if (if not complicated)
            if (TOKEN::Match(tok2, "switch {"))
            {
                // Right now, I just handle if there are a few case and perhaps a default.
                bool valid = false;
                bool incase = false;
                for ( const TOKEN * _tok = tok2->tokAt(2); _tok; _tok = _tok->next() )
                {
                    if ( _tok->str() == "{" )
                        break;

                    else if ( _tok->str() == "}" )
                    {
                        valid = true;
                        break;
                    }

                    else if (strncmp(_tok->aaaa(),"if",2)==0)
                        break;

                    else if (_tok->str() == "switch")
                        break;

                    else if (_tok->str() == "loop")
                        break;

                    else if (incase && TOKEN::Match(_tok,"case"))
                        break;

                    incase |= TOKEN::Match(_tok,"case");
                    incase &= !TOKEN::Match(_tok,"break");
                }

                if ( !incase && valid )
                {
                    done = false;
                    tok2->str(";");
                    erase( tok2, tok2->tokAt(2) );
                    tok2 = tok2->next();
                    bool first = true;
                    while (TOKEN::Match(tok2,"case") || TOKEN::Match(tok2,"default"))
                    {
                        bool def = TOKEN::Match(tok2, "default");
                        tok2->str(first ? "if" : "}");
                        if ( first )
                        {
                            first = false;
                            instoken( tok2, "{" );
                        }
                        else
                        {
                            // Insert "else [if] {
                            instoken( tok2, "{" );
                            if ( ! def )
                                instoken( tok2, "if" );
                            instoken( tok2, "else" );
                        }
                        while ( tok2 && tok2->str() != "}" && ! TOKEN::Match(tok2,"break ;") )
                            tok2 = tok2->next();
                        if (TOKEN::Match(tok2,"break ;"))
                        {
                            tok2->str(";");
                            tok2 = tok2->next()->next();
                        }
                    }
                }
            }
        }
    }
}





// Check for memory leaks for a function variable.
void CheckMemoryLeakClass::CheckMemoryLeak_CheckScope( const TOKEN *Tok1, const char varname[] )
{
    std::list<const TOKEN *> callstack;

    AllocType alloctype = No;
    AllocType dealloctype = No;

    const TOKEN *result;

    TOKEN *tok = getcode( Tok1, callstack, varname, alloctype, dealloctype );
    //tok->printOut( "getcode result" );

    // Simplify the code and check if freed memory is used..
    for ( TOKEN *tok2 = tok; tok2; tok2 = tok2->next() )
    {
        while ( TOKEN::Match(tok2, "[;{}] ;") )
            erase(tok2, tok2->tokAt(2));
    }
    if ( (result = TOKEN::findmatch(tok, "dealloc [;{}] use ;")) != NULL )
    {
        std::ostringstream errmsg;
        errmsg << _tokenizer->fileLine(result->tokAt(2)) << ": Using \"" << varname << "\" after it has been deallocated / released";
        _errorLogger->reportErr( errmsg.str() );
    }

    // Replace "&use" with "use"
    for ( TOKEN *tok2 = tok; tok2; tok2 = tok2->next() )
    {
        if (tok2->str() == "&use")
            tok2->str("use");
    }

    simplifycode( tok );
    //tok->printOut( "simplifycode result" );

    // If the variable is not allocated at all => no memory leak
    if (TOKEN::findmatch(tok, "alloc") == 0)
    {
        Tokenizer::deleteTokens(tok);
        return;
    }

    // TODO : handle "goto"
    if (TOKEN::findmatch(tok, "goto"))
    {
        Tokenizer::deleteTokens(tok);
        return;
    }

    if ( (result = TOKEN::findmatch(tok, "loop alloc ;")) != NULL )
    {
        MemoryLeak(result, varname, alloctype);
    }

    else if ( (result = TOKEN::findmatch(tok, "alloc ; if break|continue|return ;")) != NULL )
    {
        MemoryLeak(result->tokAt(3), varname, alloctype);
    }

    else if ( _settings._showAll && (result = TOKEN::findmatch(tok, "alloc ; ifv break|continue|return ;")) != NULL )
    {
        MemoryLeak(result->tokAt(3), varname, alloctype);
    }

    else if ( (result = TOKEN::findmatch(tok, "alloc ; alloc|assign|return ;")) != NULL )
    {
        MemoryLeak(result->tokAt(2), varname, alloctype);
    }

    else if ( ! TOKEN::findmatch(tok,"dealloc") &&
              ! TOKEN::findmatch(tok,"use") &&
              ! TOKEN::findmatch(tok,"return use ;") )
    {
        const TOKEN *last = tok;
        while (last->next())
            last = last->next();
        MemoryLeak(last, varname, alloctype);
    }

    // detect cases that "simplifycode" don't handle well..
    else if ( _settings._debug )
    {
        TOKEN *first = tok;
        while ( first && first->str() == ";" )
            first = first->next();

        bool noerr = false;
        noerr |= TOKEN::Match( first, "alloc ; }" );
        noerr |= TOKEN::Match( first, "alloc ; dealloc ; }" );
        noerr |= TOKEN::Match( first, "alloc ; return use ; }" );
        noerr |= TOKEN::Match( first, "alloc ; use ; }" );
        noerr |= TOKEN::Match( first, "alloc ; use ; return ; }" );
        noerr |= TOKEN::Match( first, "if alloc ; dealloc ; }" );
        noerr |= TOKEN::Match( first, "if alloc ; return use ; }" );
        noerr |= TOKEN::Match( first, "if alloc ; use ; }" );
        noerr |= TOKEN::Match( first, "alloc ; ifv return ; dealloc ; }" );
        noerr |= TOKEN::Match( first, "alloc ; if return ; dealloc; }" );

        // Unhandled case..
        if ( ! noerr )
        {
            std::cout << "Token listing..\n  ";
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next() )
                std::cout << " " << tok2->str();
            std::cout << "\n";
        }
    }

    Tokenizer::deleteTokens(tok);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Checks for memory leaks inside function..
//---------------------------------------------------------------------------

void CheckMemoryLeakClass::CheckMemoryLeak_InFunction()
{
    bool infunc = false;
    int indentlevel = 0;
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            indentlevel++;

        else if (tok->str() == "}")
            indentlevel--;


        // In function..
        if ( indentlevel == 0 )
        {
            if ( TOKEN::Match(tok, ") {") )
                infunc = true;

            else if ( TOKEN::Match(tok, "[;}]") )
                infunc = false;
        }

        // Declare a local variable => Check
        if (indentlevel>0 && infunc)
        {
            if ( TOKEN::Match(tok, "[{};] %type% * %var% [;=]") )
                CheckMemoryLeak_CheckScope( tok->next(), tok->strAt(3) );

            else if ( TOKEN::Match(tok, "[{};] %type% %type% * %var% [;=]") )
                CheckMemoryLeak_CheckScope( tok->next(), tok->strAt(4) );
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Checks for memory leaks in classes..
//---------------------------------------------------------------------------



void CheckMemoryLeakClass::CheckMemoryLeak_ClassMembers()
{
    int indentlevel = 0;
    for ( const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next() )
    {
        if ( tok->str() == "{" )
            indentlevel++;

        else if ( tok->str() == "}" )
            indentlevel--;

        else if ( indentlevel == 0 && TOKEN::Match(tok, "class %var% [{:]") )
        {
            std::vector<const char *> classname;
            classname.push_back( tok->strAt(1) );
            CheckMemoryLeak_ClassMembers_ParseClass( tok, classname );
        }
    }
}


void CheckMemoryLeakClass::CheckMemoryLeak_ClassMembers_ParseClass( const TOKEN *tok1, std::vector<const char *> &classname )
{
    // Go into class.
    while ( tok1 && tok1->str() != "{" )
        tok1 = tok1->next();
    if ( tok1 )
        tok1 = tok1->next();

    int indentlevel = 0;
    for ( const TOKEN *tok = tok1; tok; tok = tok->next() )
    {
        if ( tok->str() == "{" )
            indentlevel++;

        else if ( tok->str() == "}" )
        {
            indentlevel--;
            if ( indentlevel < 0 )
                return;
        }

        // Only parse this particular class.. not subclasses
        if ( indentlevel > 0 )
            continue;

        // Declaring subclass.. recursive checking
        if ( TOKEN::Match(tok, "class %var% [{:]") )
        {
            classname.push_back( tok->strAt(1) );
            CheckMemoryLeak_ClassMembers_ParseClass( tok, classname );
            classname.pop_back();
        }

        // Declaring member variable.. check allocations and deallocations
        if ( TOKEN::Match(tok->next(), "%type% * %var% ;") )
        {
            if ( tok->isName() || TOKEN::Match(tok, "[;}]"))
            {
                if (_settings._showAll || !isclass(tok->tokAt(1)))
                    CheckMemoryLeak_ClassMembers_Variable( classname, tok->strAt(3) );
            }
        }
    }
}

void CheckMemoryLeakClass::CheckMemoryLeak_ClassMembers_Variable( const std::vector<const char *> &classname, const char varname[] )
{
    // Function pattern.. Check if member function
	std::ostringstream fpattern;
    for ( unsigned int i = 0; i < classname.size(); i++ )
    {
		fpattern << classname[i] << " :: ";
    }
    fpattern << "%var% (";

    // Destructor pattern.. Check if class destructor..
	std::ostringstream destructor;
    for ( unsigned int i = 0; i < classname.size(); i++ )
    {
		destructor << classname[i] << " :: ";
    }
    destructor << " ~ " << classname.back() << " (";

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
    for ( const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next() )
    {
        if ( tok->str() == "{" )
            indentlevel++;

        else if ( tok->str() == "}" )
            indentlevel--;

        // Set the 'memberfunction' variable..
        if ( indentlevel == 0 )
        {
            if ( TOKEN::Match(tok, "[;}]") )
                memberfunction = false;
            else if ( TOKEN::Match( tok, fpattern.str().c_str() ) || TOKEN::Match( tok, destructor.str().c_str() ) )
                memberfunction = true;
        }

        // Parse member function..
        if ( indentlevel > 0 && memberfunction )
        {
            // Allocate..
            if ( TOKEN::Match( tok, varname_eq.str().c_str() ) )
            {
                AllocType alloc = GetAllocationType( tok->tokAt(2) );
                if ( alloc != No )
                {
                    std::list<const TOKEN *> callstack;
                    if ( Dealloc != No && Dealloc != alloc )
                        MismatchError( tok, callstack, FullVariableName.str().c_str() );
                    if ( Alloc != No && Alloc != alloc )
                        MismatchError( tok, callstack, FullVariableName.str().c_str() );
                    Alloc = alloc;
                }
            }

            // Deallocate..
            const char *varnames[2] = { "var", 0 };
            varnames[0] = varname;
            AllocType dealloc = GetDeallocationType( tok, varnames );
            if ( dealloc != No )
            {
                std::list<const TOKEN *> callstack;
                if ( Dealloc != No && Dealloc != dealloc )
                    MismatchError( tok, callstack, FullVariableName.str().c_str() );
                if ( Alloc != No && Alloc != dealloc )
                    MismatchError( tok, callstack, FullVariableName.str().c_str() );
                Dealloc = dealloc;
            }
        }
    }

    if ( Alloc != No && Dealloc == No )
    {
        MemoryLeak( _tokenizer->tokens(), FullVariableName.str().c_str(), Alloc );
    }
}




//---------------------------------------------------------------------------
// Checks for memory leaks..
//---------------------------------------------------------------------------

void CheckMemoryLeakClass::CheckMemoryLeak()
{
    _listAllocFunc.clear();

    // Check for memory leaks inside functions..
    CheckMemoryLeak_InFunction();

    // Check that all class members are deallocated..
    if ( _settings._showAll )
        CheckMemoryLeak_ClassMembers();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Non-recursive function analysis
//---------------------------------------------------------------------------

TOKEN * CheckMemoryLeakClass::functionParameterCode(const TOKEN *ftok, int parameter)
{
    int param = 1;  // First parameter has index 1

    // Extract the code for specified parameter...
    for ( ; ftok; ftok = ftok->next() )
    {
        if ( ftok->str() == ")" )
            break;

        if ( ftok->str() == "," )
        {
            ++param;
            if ( param > parameter )
                break;
        }

        if ( param != parameter )
            continue;

        if ( ! TOKEN::Match(ftok, "* %var% [,)]") )
            continue;

        // Extract and return the code for this parameter..
        const char *parname = ftok->strAt(1);

        // Goto function implementation..
        while ( ftok && ftok->str() != "{" )
            ftok = ftok->next();
        ftok = ftok ? ftok->next() : NULL;

        // Return the code..
        AllocType alloc=No, dealloc=No;
        std::list<const TOKEN *> callstack;
        TOKEN *code = getcode( ftok, callstack, parname, alloc, dealloc );
        simplifycode( code );
        return code;
    }

    return NULL;
}


/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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

//---------------------------------------------------------------------------
// Buffer overrun..
//---------------------------------------------------------------------------

#include "CheckBufferOverrun.h"
#include "CommonCheck.h"

#include <algorithm>
#include <sstream>
#include <list>
#include <cstring>

#include <stdlib.h>     // <- strtoul

//---------------------------------------------------------------------------

// CallStack used when parsing into subfunctions.


CheckBufferOverrunClass::CheckBufferOverrunClass( Tokenizer *tokenizer )
{
    _tokenizer = tokenizer;
}

CheckBufferOverrunClass::~CheckBufferOverrunClass()
{

}

// Modified version of 'ReportError' that also reports the callstack
void CheckBufferOverrunClass::ReportError(const TOKEN *tok, const char errmsg[])
{
    std::ostringstream ostr;
    std::list<const TOKEN *>::const_iterator it;
    for ( it = CallStack.begin(); it != CallStack.end(); it++ )
        ostr << FileLine(*it, _tokenizer ) << " -> ";
    ostr << FileLine(tok, _tokenizer) << ": " << errmsg;
    ReportErr(ostr.str());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

void CheckBufferOverrunClass::CheckBufferOverrun_CheckScope( const TOKEN *tok, const char *varname[], const int size, const int total_size )
{
    unsigned int varc = 1;
    while ( varname[varc] )
        varc++;
    varc = 2 * ( varc - 1 );


    // Array index..
    if ( Match(tok, "%var1% [ %num% ]", varname) )
    {
        const char *num = Tokenizer::getstr(tok, 2 + varc);
        if (strtol(num, NULL, 10) >= size)
        {
            ReportError(tok->next, "Array index out of bounds");
        }
    }


    int indentlevel = 0;
    for ( ; tok; tok = tok->next )
    {
        if (Match(tok, "{"))
        {
            indentlevel++;
        }

        else if (Match(tok, "}"))
        {
            indentlevel--;
            if ( indentlevel < 0 )
                return;
        }

        // Array index..
        if ( !Match(tok, "%var%") && !Match(tok,"[.&]") && Match(tok->next, "%var1% [ %num% ]", varname) )
        {
            const char *num = Tokenizer::getstr(tok->next, 2 + varc);
            if (strtol(num, NULL, 10) >= size)
            {
                ReportError(tok->next, "Array index out of bounds");
            }
            tok = Tokenizer::gettok(tok, 4);
            continue;
        }


        // memset, memcmp, memcpy, strncpy, fgets..
        if (Match(tok,"memset") ||
            Match(tok,"memcpy") ||
            Match(tok,"memmove") ||
            Match(tok,"memcmp") ||
            Match(tok,"strncpy") ||
            Match(tok,"fgets") )
        {
            if ( Match( tok->next, "( %var1% , %num% , %num% )", varname ) ||
                 Match( tok->next, "( %var% , %var1% , %num% )", varname ) )
            {
                const char *num  = Tokenizer::getstr(tok, varc + 6);
                if ( atoi(num) > total_size )
                {
                    ReportError(tok, "Buffer overrun");
                }
            }
            continue;
        }


        // Loop..
        if ( Match(tok, "for (") )
        {
            const TOKEN *tok2 = Tokenizer::gettok( tok, 2 );

            // for - setup..
            if ( Match(tok2, "%var% = 0 ;") )
                tok2 = Tokenizer::gettok(tok2, 4);
            else if ( Match(tok2, "%type% %var% = 0 ;") )
                tok2 = Tokenizer::gettok(tok2, 5);
            else if ( Match(tok2, "%type% %type% %var% = 0 ;") )
                tok2 = Tokenizer::gettok(tok2, 6);
            else
                continue;

            // for - condition..
            if ( ! Match(tok2, "%var% < %num% ;") && ! Match(tok2, "%var% <= %num% ;"))
                continue;

            // Get index variable and stopsize.
            const char *strindex = tok2->str;
            int value = (tok2->next->str[1] ? 1 : 0) + atoi(Tokenizer::getstr(tok2, 2));
            if ( value <= size )
                continue;

            // Goto the end of the for loop..
            while (tok2 && !Match(tok2,")"))
                tok2 = tok2->next;
            if (!Tokenizer::gettok(tok2,5))
                break;

            std::ostringstream pattern;
            pattern << "%var1% [ " << strindex << " ]";

            int indentlevel2 = 0;
            while (tok2)
            {
                if ( Match(tok2, ";") && indentlevel2 == 0 )
                    break;

                if ( Match(tok2, "{") )
                    indentlevel2++;

                if ( Match(tok2, "}") )
                {
                    indentlevel2--;
                    if ( indentlevel2 <= 0 )
                        break;
                }

                if ( Match( tok2, pattern.str().c_str(), varname ) )
                {
                    ReportError(tok2, "Buffer overrun");
                    break;
                }

                tok2 = tok2->next;
            }
            continue;
        }


        // Writing data into array..
        if ( Match(tok, "strcpy ( %var1% , %str% )", varname) )
        {
            int len = 0;
            const char *str = Tokenizer::getstr(tok, varc + 4 );
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
        // It's not interesting to check what happens when the whole struct is
        // sent as the parameter, that is checked separately anyway.
        if ( Match( tok, "%var% (" ) )
        {
            // Don't make recursive checking..
            if (std::find(CallStack.begin(), CallStack.end(), tok) != CallStack.end())
                continue;

            unsigned int parlevel = 0, par = 0;
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next )
            {
                if ( Match(tok2, "(") )
                {
                    parlevel++;
                }

                else if ( Match(tok2, ")") )
                {
                    parlevel--;
                    if ( parlevel < 1 )
                    {
                        par = 0;
                        break;
                    }
                }

                else if ( parlevel == 1 && Match(tok2, ",") )
                {
                    par++;
                }

                if ( parlevel == 1 && Match(tok2, "[(,] %var1% [,)]", varname) )
                {
                    par++;
                    break;
                }
            }

            if ( par == 0 )
                continue;

            // Find function..
            const TOKEN *ftok = _tokenizer->GetFunctionTokenByName( tok->str );
            if ( ! ftok )
                continue;

            // Parse head of function..
            ftok = Tokenizer::gettok( ftok, 2 );
            parlevel = 1;
            while ( ftok && parlevel == 1 && par >= 1 )
            {
                if ( Match(ftok, "(") )
                    parlevel++;

                else if ( Match(ftok, ")") )
                    parlevel--;

                else if ( Match(ftok, ",") )
                    par--;

                else if (par==1 && parlevel==1 && (Match(ftok, "%var% ,") || Match(ftok, "%var% )")))
                {
                    // Parameter name..
                    const char *parname[2];
                    parname[0] = ftok->str;
                    parname[1] = 0;

                    // Goto function body..
                    while ( ftok && !Match(ftok,"{") )
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

void CheckBufferOverrunClass::CheckBufferOverrun_LocalVariable()
{
    int indentlevel = 0;
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        if (Match(tok, "{"))
            indentlevel++;

        else if (Match(tok, "}"))
            indentlevel--;

        else if (indentlevel > 0)
        {
            const char *varname[2] = {0};
            unsigned int size = 0;
            const char *type = 0;

            if (Match(tok, "%type% %var% [ %num% ] ;"))
            {
                varname[0] = Tokenizer::getstr(tok,1);
                size = strtoul(Tokenizer::getstr(tok,3), NULL, 10);
                type = tok->str;
            }
            else if (indentlevel > 0 && Match(tok, "[*;{}] %var% = new %type% [ %num% ]"))
            {
                varname[0] = Tokenizer::getstr(tok,1);
                size = strtoul(Tokenizer::getstr(tok,6), NULL, 10);
                type = Tokenizer::getstr(tok, 4);
            }
            else
            {
                continue;
            }

            int total_size = size * _tokenizer->SizeOfType(type);
            if (total_size == 0)
                continue;

            // The callstack is empty
            CallStack.clear();
            CheckBufferOverrun_CheckScope( Tokenizer::gettok(tok,5), varname, size, total_size );
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checking member variables of structs..
//---------------------------------------------------------------------------

void CheckBufferOverrunClass::CheckBufferOverrun_StructVariable()
{
    const char *declstruct_pattern[] = {"","","{",0};
    for ( const TOKEN * tok = Tokenizer::findtoken( _tokenizer->tokens(), declstruct_pattern );
          tok;
          tok = Tokenizer::findtoken( tok->next, declstruct_pattern ) )
    {
        if (!Match(tok,"struct") && !Match(tok,"class"))
            continue;

        const char *structname = tok->next->str;

        if ( ! IsName( structname ) )
            continue;

        // Found a struct declaration. Search for arrays..
        for ( TOKEN * tok2 = tok->next->next; tok2; tok2 = tok2->next )
        {
            if ( Match(tok2, "}") )
                break;

            int ivar = 0;
            if ( Match(tok2->next, "%type% %var% [ %num% ] ;") )
                ivar = 2;
            else if ( Match(tok2->next, "%type% %type% %var% [ %num% ] ;") )
                ivar = 3;
            else if ( Match(tok2->next, "%type% * %var% [ %num% ] ;") )
                ivar = 3;
            else if ( Match(tok2->next, "%type% %type% * %var% [ %num% ] ;") )
                ivar = 4;
            else
                continue;

            const char *varname[3] = {0,0,0};
            varname[1] = Tokenizer::getstr(tok2, ivar);
            int arrsize = atoi(Tokenizer::getstr(tok2, ivar+2));
            int total_size = arrsize * _tokenizer->SizeOfType(tok2->next->str);
            if (total_size == 0)
                continue;


            // Class member variable => Check functions
            if ( Match(tok, "class") )
            {
                std::string func_pattern(structname + std::string(" :: %var% ("));
                const TOKEN *tok3 = findmatch(_tokenizer->tokens(), func_pattern.c_str());
                while ( tok3 )
                {
                    for ( const TOKEN *tok4 = tok3; tok4; tok4 = tok4->next )
                    {
                        if ( Match(tok4,"[;{}]") )
                            break;

                        if ( Match(tok4, ") {") )
                        {
                            const char *names[2] = {varname[1], 0};
                            CheckBufferOverrun_CheckScope( Tokenizer::gettok(tok4, 2), names, arrsize, total_size );
                            break;
                        }
                    }
                    tok3 = findmatch(tok3->next, func_pattern.c_str());
                }
            }

            for ( const TOKEN *tok3 = _tokenizer->tokens(); tok3; tok3 = tok3->next )
            {
                if ( strcmp(tok3->str, structname) )
                    continue;

                // Declare variable: Fred fred1;
                if ( Match( tok3->next, "%var% ;" ) )
                    varname[0] = Tokenizer::getstr(tok3, 1);

                // Declare pointer: Fred *fred1
                else if ( Match(tok3->next, "* %var% [,);=]") )
                    varname[0] = Tokenizer::getstr(tok3, 2);

                else
                    continue;


                // Goto end of statement.
                const TOKEN *CheckTok = NULL;
                while ( tok3 )
                {
                    // End of statement.
                    if ( Match(tok3, ";") )
                    {
                        CheckTok = tok3;
                        break;
                    }

                    // End of function declaration..
                    if ( Match(tok3, ") ;") )
                        break;

                    // Function implementation..
                    if ( Match(tok3, ") {") )
                    {
                        CheckTok = Tokenizer::gettok(tok3, 2);
                        break;
                    }

                    tok3 = tok3->next;
                }

                if ( ! tok3 )
                    break;

                if ( ! CheckTok )
                    continue;

                // Check variable usage..
                CheckBufferOverrun_CheckScope( CheckTok, varname, arrsize, total_size );
            }
        }
    }
}
//---------------------------------------------------------------------------



void CheckBufferOverrunClass::CheckBufferOverrun()
{
    CheckBufferOverrun_LocalVariable();
    CheckBufferOverrun_StructVariable();
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// Dangerous functions
//---------------------------------------------------------------------------

void CheckBufferOverrunClass::WarningDangerousFunctions()
{
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next)
    {
        if (Match(tok, "gets ("))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok, _tokenizer) << ": Found 'gets'. You should use 'fgets' instead";
            ReportErr(ostr.str());
        }

        else if (Match(tok, "scanf (") && strcmp(Tokenizer::getstr(tok,2),"\"%s\"") == 0)
        {
            std::ostringstream ostr;
            ostr << FileLine(tok, _tokenizer) << ": Found 'scanf'. You should use 'fgets' instead";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------





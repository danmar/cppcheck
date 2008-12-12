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

#include <algorithm>
#include <sstream>
#include <list>
#include <cstring>


#include <stdlib.h>     // <- strtoul

//---------------------------------------------------------------------------

// _callStack used when parsing into subfunctions.


CheckBufferOverrunClass::CheckBufferOverrunClass( const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger )
  :  _settings(settings)
{
    _tokenizer = tokenizer;
    _errorLogger = errorLogger;
}

CheckBufferOverrunClass::~CheckBufferOverrunClass()
{

}

// Modified version of 'ReportError' that also reports the callstack
void CheckBufferOverrunClass::ReportError(const TOKEN *tok, const char errmsg[])
{
    std::ostringstream ostr;
    std::list<const TOKEN *>::const_iterator it;
    for ( it = _callStack.begin(); it != _callStack.end(); it++ )
        ostr << _tokenizer->fileLine(*it ) << " -> ";
    ostr << _tokenizer->fileLine(tok) << ": " << errmsg;
    _errorLogger->reportErr(ostr.str());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

void CheckBufferOverrunClass::CheckBufferOverrun_CheckScope( const TOKEN *tok, const char *varname[], const int size, const int total_size, unsigned int varid )
{
    unsigned int varc = 1;
    while ( varname[varc] )
        varc++;
    varc = 2 * ( varc - 1 );


    // Array index..
    if ( varid > 0 )
    {
        if ( TOKEN::Match(tok, "%varid% [ %num% ]", 0, 0, varid) )
        {
            const char *num = tok->strAt(2);
            if (strtol(num, NULL, 10) >= size)
            {
                ReportError(tok->next(), "Array index out of bounds");
            }
        }
    }
    else if ( TOKEN::Match(tok, "%var1% [ %num% ]", varname) )
    {
        const char *num = tok->strAt(2 + varc);
        if (strtol(num, NULL, 10) >= size)
        {
            ReportError(tok->next(), "Array index out of bounds");
        }
    }


    int indentlevel = 0;
    for ( ; tok; tok = tok->next() )
    {
        if (tok->str() == "{")
        {
            indentlevel++;
        }

        else if (tok->str() == "}")
        {
            indentlevel--;
            if ( indentlevel < 0 )
                return;
        }

        // Array index..
        if ( varid > 0 )
        {
            if ( !tok->isName() && !TOKEN::Match(tok,"[.&]") && TOKEN::Match(tok->next(), "%varid% [ %num% ]", 0, 0, varid) )
            {
                const char *num = tok->strAt(3);
                if (strtol(num, NULL, 10) >= size)
                {
                    ReportError(tok->next(), "Array index out of bounds");
                }
            }
        }
        else if ( !tok->isName() && !TOKEN::Match(tok,"[.&]") && TOKEN::Match(tok->next(), "%var1% [ %num% ]", varname) )
        {
            const char *num = tok->next()->strAt(2 + varc);
            if (strtol(num, NULL, 10) >= size)
            {
                ReportError(tok->next(), "Array index out of bounds");
            }
            tok = tok->tokAt(4);
            continue;
        }


        // memset, memcmp, memcpy, strncpy, fgets..
        if (TOKEN::Match(tok,"memset|memcpy|memmove|memcmp|strncpy|fgets") )
        {
            if ( TOKEN::Match( tok->next(), "( %var1% , %num% , %num% )", varname ) ||
                 TOKEN::Match( tok->next(), "( %var% , %var1% , %num% )", varname ) )
            {
                const char *num  = tok->strAt(varc + 6);
                if ( atoi(num) > total_size )
                {
                    ReportError(tok, "Buffer overrun");
                }
            }
            continue;
        }


        // Loop..
        if ( TOKEN::Match(tok, "for (") )
        {
            const TOKEN *tok2 = tok->tokAt(2);

            // for - setup..
            if ( TOKEN::Match(tok2, "%var% = 0 ;") )
                tok2 = tok2->tokAt(4);
            else if ( TOKEN::Match(tok2, "%type% %var% = 0 ;") )
                tok2 = tok2->tokAt(5);
            else if ( TOKEN::Match(tok2, "%type% %type% %var% = 0 ;") )
                tok2 = tok2->tokAt(6);
            else
                continue;

            // for - condition..
            if ( ! TOKEN::Match(tok2, "%var% < %num% ;") && ! TOKEN::Match(tok2, "%var% <= %num% ;"))
                continue;

            // Get index variable and stopsize.
            const char *strindex = tok2->aaaa();
            int value = ((tok2->next()->aaaa1() == '=') ? 1 : 0) + atoi(tok2->strAt(2));
            if ( value <= size )
                continue;

            // Goto the end of the for loop..
            while (tok2 && !TOKEN::Match(tok2,")"))
                tok2 = tok2->next();
            if (!(tok2->tokAt(5)))
                break;

            std::ostringstream pattern;
            pattern << "%var1% [ " << strindex << " ]";

            int indentlevel2 = 0;
            while (tok2)
            {
                if ( (tok2->str() == ";") && indentlevel2 == 0 )
                    break;

                if ( tok2->str() == "{" )
                    indentlevel2++;

                if ( tok2->str() == "}" )
                {
                    indentlevel2--;
                    if ( indentlevel2 <= 0 )
                        break;
                }

                if ( TOKEN::Match( tok2, pattern.str().c_str(), varname ) )
                {
                    ReportError(tok2, "Buffer overrun");
                    break;
                }

                tok2 = tok2->next();
            }
            continue;
        }


        // Writing data into array..
        if ( TOKEN::Match(tok, "strcpy ( %var1% , %str% )", varname) )
        {
            int len = 0;
            const char *str = tok->strAt(varc + 4 );
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
        if ( TOKEN::Match( tok, "%var% (" ) )
        {
            // Don't make recursive checking..
            if (std::find(_callStack.begin(), _callStack.end(), tok) != _callStack.end())
                continue;

            // Only perform this checking if showAll setting is enabled..
            if ( ! _settings._showAll )
                continue;

            unsigned int parlevel = 0, par = 0;
            for ( const TOKEN *tok2 = tok; tok2; tok2 = tok2->next() )
            {
                if ( tok2->str() == "(" )
                {
                    parlevel++;
                }

                else if ( tok2->str() == ")" )
                {
                    parlevel--;
                    if ( parlevel < 1 )
                    {
                        par = 0;
                        break;
                    }
                }

                else if ( parlevel == 1 && (tok2->str() == ",") )
                {
                    par++;
                }

                if ( parlevel == 1 && TOKEN::Match(tok2, "[(,] %var1% [,)]", varname) )
                {
                    par++;
                    break;
                }
            }

            if ( par == 0 )
                continue;

            // Find function..
            const TOKEN *ftok = _tokenizer->GetFunctionTokenByName( tok->aaaa() );
            if ( ! ftok )
                continue;

            // Parse head of function..
            ftok = ftok->tokAt(2);
            parlevel = 1;
            while ( ftok && parlevel == 1 && par >= 1 )
            {
                if ( ftok->str() == "(" )
                    parlevel++;

                else if ( ftok->str() == ")" )
                    parlevel--;

                else if ( ftok->str() == "," )
                    par--;

                else if (par==1 && parlevel==1 && (TOKEN::Match(ftok, "%var% ,") || TOKEN::Match(ftok, "%var% )")))
                {
                    // Parameter name..
                    const char *parname[2];
                    parname[0] = ftok->aaaa();
                    parname[1] = 0;

                    // Goto function body..
                    while ( ftok && (ftok->str() != "{") )
                        ftok = ftok->next();
                    ftok = ftok ? ftok->next() : 0;

                    // Check variable usage in the function..
                    _callStack.push_back( tok );
                    CheckBufferOverrun_CheckScope( ftok, parname, size, total_size, 0 );
                    _callStack.pop_back();

                    // break out..
                    break;
                }

                ftok = ftok->next();
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
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            indentlevel++;

        else if (tok->str() == "}")
            indentlevel--;

        else if (indentlevel > 0)
        {
            const char *varname[2] = {0};
            unsigned int size = 0;
            const char *type = 0;
            unsigned int varid = 0;

            if (TOKEN::Match(tok, "%type% %var% [ %num% ] ;"))
            {
                varname[0] = tok->strAt(1);
                size = strtoul(tok->strAt(3), NULL, 10);
                type = tok->aaaa();
                varid = tok->tokAt(1)->varId();
            }
            else if (indentlevel > 0 && TOKEN::Match(tok, "[*;{}] %var% = new %type% [ %num% ]"))
            {
                varname[0] = tok->strAt(1);
                size = strtoul(tok->strAt(6), NULL, 10);
                type = tok->strAt(4);
                varid = tok->tokAt(1)->varId();
            }
            else
            {
                continue;
            }

            int total_size = size * _tokenizer->SizeOfType(type);
            if (total_size == 0)
                continue;

            // The callstack is empty
            _callStack.clear();
            CheckBufferOverrun_CheckScope( tok->tokAt(5), varname, size, total_size, varid );
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
    for ( const TOKEN * tok = TOKEN::findtoken( _tokenizer->tokens(), declstruct_pattern );
          tok;
          tok = TOKEN::findtoken( tok->next(), declstruct_pattern ) )
    {
        if ( ! TOKEN::Match(tok,"struct|class") )
            continue;

        const char *structname = tok->next()->aaaa();

        if ( !(tok->next()->isName()) )
            continue;

        // Found a struct declaration. Search for arrays..
        for ( const TOKEN * tok2 = tok->next()->next(); tok2; tok2 = tok2->next() )
        {
            if ( TOKEN::Match(tok2, "}") )
                break;

            int ivar = 0;
            if ( TOKEN::Match(tok2->next(), "%type% %var% [ %num% ] ;") )
                ivar = 2;
            else if ( TOKEN::Match(tok2->next(), "%type% %type% %var% [ %num% ] ;") )
                ivar = 3;
            else if ( TOKEN::Match(tok2->next(), "%type% * %var% [ %num% ] ;") )
                ivar = 3;
            else if ( TOKEN::Match(tok2->next(), "%type% %type% * %var% [ %num% ] ;") )
                ivar = 4;
            else
                continue;

            const char *varname[3] = {0,0,0};
            varname[1] = tok2->strAt(ivar);
            int arrsize = atoi(tok2->strAt(ivar+2));
            int total_size = arrsize * _tokenizer->SizeOfType(tok2->next()->aaaa());
            if (total_size == 0)
                continue;


            // Class member variable => Check functions
            if ( TOKEN::Match(tok, "class") )
            {
                std::string func_pattern(structname + std::string(" :: %var% ("));
                const TOKEN *tok3 = TOKEN::findmatch(_tokenizer->tokens(), func_pattern.c_str());
                while ( tok3 )
                {
                    for ( const TOKEN *tok4 = tok3; tok4; tok4 = tok4->next() )
                    {
                        if ( TOKEN::Match(tok4,"[;{}]") )
                            break;

                        if ( TOKEN::Match(tok4, ") {") )
                        {
                            const char *names[2] = {varname[1], 0};
                            CheckBufferOverrun_CheckScope( tok4->tokAt(2), names, arrsize, total_size, 0 );
                            break;
                        }
                    }
                    tok3 = TOKEN::findmatch(tok3->next(), func_pattern.c_str());
                }
            }

            for ( const TOKEN *tok3 = _tokenizer->tokens(); tok3; tok3 = tok3->next() )
            {
                if ( strcmp(tok3->aaaa(), structname) )
                    continue;

                // Declare variable: Fred fred1;
                if ( TOKEN::Match( tok3->next(), "%var% ;" ) )
                    varname[0] = tok3->strAt(1);

                // Declare pointer: Fred *fred1
                else if ( TOKEN::Match(tok3->next(), "* %var% [,);=]") )
                    varname[0] = tok3->strAt(2);

                else
                    continue;


                // Goto end of statement.
                const TOKEN *CheckTok = NULL;
                while ( tok3 )
                {
                    // End of statement.
                    if ( TOKEN::Match(tok3, ";") )
                    {
                        CheckTok = tok3;
                        break;
                    }

                    // End of function declaration..
                    if ( TOKEN::Match(tok3, ") ;") )
                        break;

                    // Function implementation..
                    if ( TOKEN::Match(tok3, ") {") )
                    {
                        CheckTok = tok3->tokAt(2);
                        break;
                    }

                    tok3 = tok3->next();
                }

                if ( ! tok3 )
                    break;

                if ( ! CheckTok )
                    continue;

                // Check variable usage..
                CheckBufferOverrun_CheckScope( CheckTok, varname, arrsize, total_size, 0 );
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
    for (const TOKEN *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (TOKEN::Match(tok, "gets ("))
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Found 'gets'. You should use 'fgets' instead";
            _errorLogger->reportErr(ostr.str());
        }

        else if (TOKEN::Match(tok, "scanf (") && strcmp(tok->strAt(2),"\"%s\"") == 0)
        {
            std::ostringstream ostr;
            ostr << _tokenizer->fileLine(tok) << ": Found 'scanf'. You should use 'fgets' instead";
            _errorLogger->reportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------





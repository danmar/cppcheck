/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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

#include "checkbufferoverrun.h"
#include "errormessage.h"

#include <algorithm>
#include <sstream>
#include <list>
#include <cstring>


#include <cstdlib>     // <- strtoul

//---------------------------------------------------------------------------

// _callStack used when parsing into subfunctions.


CheckBufferOverrunClass::CheckBufferOverrunClass(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        :  _settings(settings)
{
    _tokenizer = tokenizer;
    _errorLogger = errorLogger;
}

CheckBufferOverrunClass::~CheckBufferOverrunClass()
{

}

void CheckBufferOverrunClass::arrayIndexOutOfBounds(const Token *tok)
{
    _callStack.push_back(tok);
    ErrorMessage::arrayIndexOutOfBounds(_errorLogger, _tokenizer, _callStack);
    _callStack.pop_back();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

void CheckBufferOverrunClass::CheckBufferOverrun_CheckScope(const Token *tok, const char *varname[], const int size, const int total_size, unsigned int varid)
{
    unsigned int varc = 0;

    std::string varnames;
    while (varname[varc])
    {
        if (varc > 0)
            varnames += " . ";

        varnames += varname[varc];

        ++varc;
    }

    if (varc == 0)
        varc = 1;

    varc = 2 * (varc - 1);

    // Array index..
    if (varid > 0)
    {
        if (Token::Match(tok, "%varid% [ %num% ]", varid))
        {
            const char *num = tok->strAt(2);
            if (std::strtol(num, NULL, 10) >= size)
            {
                arrayIndexOutOfBounds(tok->next());
            }
        }
    }
    else if (Token::Match(tok, std::string(varnames + " [ %num% ]").c_str()))
    {
        const char *num = tok->strAt(2 + varc);
        if (std::strtol(num, NULL, 10) >= size)
        {
            arrayIndexOutOfBounds(tok->next());
        }
    }


    int indentlevel = 0;
    for (; tok; tok = tok->next())
    {
        if (tok->str() == "{")
        {
            ++indentlevel;
        }

        else if (tok->str() == "}")
        {
            --indentlevel;
            if (indentlevel < 0)
                return;
        }

        // Array index..
        if (varid > 0)
        {
            if (!tok->isName() && !Token::Match(tok, "[.&]") && Token::Match(tok->next(), "%varid% [ %num% ]", varid))
            {
                const char *num = tok->strAt(3);
                if (std::strtol(num, NULL, 10) >= size)
                {
                    arrayIndexOutOfBounds(tok->next());
                }
            }
        }
        else if (!tok->isName() && !Token::Match(tok, "[.&]") && Token::Match(tok->next(), std::string(varnames + " [ %num% ]").c_str()))
        {
            const char *num = tok->next()->strAt(2 + varc);
            if (std::strtol(num, NULL, 10) >= size)
            {
                arrayIndexOutOfBounds(tok->next());
            }
            tok = tok->tokAt(4);
            continue;
        }


        // memset, memcmp, memcpy, strncpy, fgets..
        if (varid > 0)
        {
            if (Token::Match(tok, "memset|memcpy|memmove|memcmp|strncpy|fgets"))
            {
                if (Token::Match(tok->next(), "( %varid% , %num% , %num% )", varid) ||
                    Token::Match(tok->next(), "( %var% , %varid% , %num% )", varid))
                {
                    const char *num  = tok->strAt(6);
                    if (std::atoi(num) > total_size)
                    {
                        ErrorMessage::bufferOverrun(_errorLogger, _tokenizer, tok);
                    }
                }
                continue;
            }
        }
        else if (Token::Match(tok, "memset|memcpy|memmove|memcmp|strncpy|fgets"))
        {
            if (Token::Match(tok->next(), std::string("( " + varnames + " , %num% , %num% )").c_str()) ||
                Token::Match(tok->next(), std::string("( %var% , " + varnames + " , %num% )").c_str()))
            {
                const char *num  = tok->strAt(varc + 6);
                if (std::atoi(num) > total_size)
                {
                    ErrorMessage::bufferOverrun(_errorLogger, _tokenizer, tok);
                }
            }
            continue;
        }


        // Loop..
        if (Token::simpleMatch(tok, "for ("))
        {
            const Token *tok2 = tok->tokAt(2);

            // for - setup..
            if (Token::Match(tok2, "%var% = 0 ;"))
                tok2 = tok2->tokAt(4);
            else if (Token::Match(tok2, "%type% %var% = 0 ;"))
                tok2 = tok2->tokAt(5);
            else if (Token::Match(tok2, "%type% %type% %var% = 0 ;"))
                tok2 = tok2->tokAt(6);
            else
                continue;

            // for - condition..
            if (!Token::Match(tok2, "%var% < %num% ;") && !Token::Match(tok2, "%var% <= %num% ;"))
                continue;

            // Get index variable and stopsize.
            const char *strindex = tok2->aaaa();
            int value = ((tok2->next()->aaaa1() == '=') ? 1 : 0) + std::atoi(tok2->strAt(2));
            if (value <= size)
                continue;

            // Goto the end of the for loop..
            while (tok2 && tok2->str() != ")")
                tok2 = tok2->next();
            if (!tok2 || !tok2->tokAt(5))
                break;

            std::ostringstream pattern;
            pattern << varnames << " [ " << strindex << " ]";

            int indentlevel2 = 0;
            while ((tok2 = tok2->next()))
            {
                if (tok2->str() == ";" && indentlevel2 == 0)
                    break;

                if (tok2->str() == "{")
                    ++indentlevel2;

                if (tok2->str() == "}")
                {
                    --indentlevel2;
                    if (indentlevel2 <= 0)
                        break;
                }

                if (Token::Match(tok2, pattern.str().c_str()))
                {
                    ErrorMessage::bufferOverrun(_errorLogger, _tokenizer, tok2);
                    break;
                }

            }
            continue;
        }


        // Writing data into array..
        if (Token::Match(tok, std::string("strcpy ( " + varnames + " , %str% )").c_str()))
        {
            int len = 0;
            const char *str = tok->strAt(varc + 4);
            while (*str)
            {
                if (*str == '\\')
                    ++str;
                ++str;
                ++len;
            }
            if (len > 2 && len >= (int)size + 2)
            {
                ErrorMessage::bufferOverrun(_errorLogger, _tokenizer, tok);
            }
            continue;
        }

        // sprintf..
        if (varid > 0 && Token::Match(tok, "sprintf ( %varid% , %str% ,", varid))
        {
            int len = 0;
            for (const Token *tok2 = tok->tokAt(6); tok2 && tok2->str() != ")"; tok2 = tok2->next())
            {
                if (tok2->aaaa0() == '\"')
                {
                    len -= 2;
                    const char *str = tok->strAt(0);
                    while (*str)
                    {
                        if (*str == '\\')
                            ++str;
                        ++str;
                        ++len;
                    }
                }
            }
            if (len > (int)size)
            {
                ErrorMessage::bufferOverrun(_errorLogger, _tokenizer, tok);
            }
        }

        // snprintf..
        if (varid > 0 && Token::Match(tok, "snprintf ( %varid% , %num%", varid))
        {
            int n = std::atoi(tok->strAt(4));
            if (n > size)
                ErrorMessage::outOfBounds(_errorLogger, _tokenizer, tok->tokAt(4), "snprintf size");
        }


        // Function call..
        // It's not interesting to check what happens when the whole struct is
        // sent as the parameter, that is checked separately anyway.
        if (Token::Match(tok, "%var% ("))
        {
            // Don't make recursive checking..
            if (std::find(_callStack.begin(), _callStack.end(), tok) != _callStack.end())
                continue;

            // Only perform this checking if showAll setting is enabled..
            if (!_settings._showAll)
                continue;

            unsigned int parlevel = 0, par = 0;
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                {
                    ++parlevel;
                }

                else if (tok2->str() == ")")
                {
                    --parlevel;
                    if (parlevel < 1)
                    {
                        par = 0;
                        break;
                    }
                }

                else if (parlevel == 1 && (tok2->str() == ","))
                {
                    ++par;
                }

                if (parlevel == 1 && Token::Match(tok2, std::string("[(,] " + varnames + " [,)]").c_str()))
                {
                    ++par;
                    break;
                }
            }

            if (par == 0)
                continue;

            // Find function..
            const Token *ftok = _tokenizer->GetFunctionTokenByName(tok->aaaa());
            if (!ftok)
                continue;

            // Parse head of function..
            ftok = ftok->tokAt(2);
            parlevel = 1;
            while (ftok && parlevel == 1 && par >= 1)
            {
                if (ftok->str() == "(")
                    ++parlevel;

                else if (ftok->str() == ")")
                    --parlevel;

                else if (ftok->str() == ",")
                    --par;

                else if (par == 1 && parlevel == 1 && Token::Match(ftok, "%var% [,)]"))
                {
                    // Parameter name..
                    const char *parname[2];
                    parname[0] = ftok->aaaa();
                    parname[1] = 0;

                    // Goto function body..
                    while (ftok && (ftok->str() != "{"))
                        ftok = ftok->next();
                    ftok = ftok ? ftok->next() : 0;

                    // Check variable usage in the function..
                    _callStack.push_back(tok);
                    CheckBufferOverrun_CheckScope(ftok, parname, size, total_size, 0);
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
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
            --indentlevel;

        else if (indentlevel > 0)
        {
            const char *varname[2] = {0};
            unsigned int size = 0;
            const char *type = 0;
            unsigned int varid = 0;
            int nextTok = 0;

            if (Token::Match(tok, "%type% %var% [ %num% ] ;"))
            {
                varname[0] = tok->strAt(1);
                size = std::strtoul(tok->strAt(3), NULL, 10);
                type = tok->aaaa();
                varid = tok->tokAt(1)->varId();
                nextTok = 6;
            }
            else if (Token::Match(tok, "[*;{}] %var% = new %type% [ %num% ]"))
            {
                varname[0] = tok->strAt(1);
                size = std::strtoul(tok->strAt(6), NULL, 10);
                type = tok->strAt(4);
                varid = tok->tokAt(1)->varId();
                nextTok = 8;
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
            CheckBufferOverrun_CheckScope(tok->tokAt(nextTok), varname, size, total_size, varid);
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checking member variables of structs..
//---------------------------------------------------------------------------

void CheckBufferOverrunClass::CheckBufferOverrun_StructVariable()
{
    const char declstruct[] = "struct|class %var% {";
    for (const Token *tok = Token::findmatch(_tokenizer->tokens(), declstruct);
         tok; tok = Token::findmatch(tok->next(), declstruct))
    {
        const std::string &structname = tok->next()->str();

        // Found a struct declaration. Search for arrays..
        for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "}")
                break;

            int ivar = 0;
            if (Token::Match(tok2->next(), "%type% %var% [ %num% ] ;"))
                ivar = 2;
            else if (Token::Match(tok2->next(), "%type% %type% %var% [ %num% ] ;"))
                ivar = 3;
            else if (Token::Match(tok2->next(), "%type% * %var% [ %num% ] ;"))
                ivar = 3;
            else if (Token::Match(tok2->next(), "%type% %type% * %var% [ %num% ] ;"))
                ivar = 4;
            else
                continue;

            const char *varname[3] = {0, 0, 0};
            varname[1] = tok2->strAt(ivar);
            int arrsize = std::atoi(tok2->strAt(ivar + 2));
            int total_size = arrsize * _tokenizer->SizeOfType(tok2->next()->aaaa());
            if (total_size == 0)
                continue;


            // Class member variable => Check functions
            if (tok->str() == "class")
            {
                std::string func_pattern(structname + " :: %var% (");
                const Token *tok3 = Token::findmatch(_tokenizer->tokens(), func_pattern.c_str());
                while (tok3)
                {
                    for (const Token *tok4 = tok3; tok4; tok4 = tok4->next())
                    {
                        if (Token::Match(tok4, "[;{}]"))
                            break;

                        if (Token::simpleMatch(tok4, ") {"))
                        {
                            const char *names[2] = {varname[1], 0};
                            CheckBufferOverrun_CheckScope(tok4->tokAt(2), names, arrsize, total_size, 0);
                            break;
                        }
                    }
                    tok3 = Token::findmatch(tok3->next(), func_pattern.c_str());
                }
            }

            for (const Token *tok3 = _tokenizer->tokens(); tok3; tok3 = tok3->next())
            {
                if (tok3->str() != structname)
                    continue;

                // Declare variable: Fred fred1;
                if (Token::Match(tok3->next(), "%var% ;"))
                    varname[0] = tok3->strAt(1);

                // Declare pointer: Fred *fred1
                else if (Token::Match(tok3->next(), "* %var% [,);=]"))
                    varname[0] = tok3->strAt(2);

                else
                    continue;


                // Goto end of statement.
                const Token *CheckTok = NULL;
                while (tok3)
                {
                    // End of statement.
                    if (tok3->str() == ";")
                    {
                        CheckTok = tok3;
                        break;
                    }

                    // End of function declaration..
                    if (Token::simpleMatch(tok3, ") ;"))
                        break;

                    // Function implementation..
                    if (Token::simpleMatch(tok3, ") {"))
                    {
                        CheckTok = tok3->tokAt(2);
                        break;
                    }

                    tok3 = tok3->next();
                }

                if (!tok3)
                    break;

                if (!CheckTok)
                    continue;

                // Check variable usage..
                CheckBufferOverrun_CheckScope(CheckTok, varname, arrsize, total_size, 0);
            }
        }
    }
}
//---------------------------------------------------------------------------



void CheckBufferOverrunClass::bufferOverrun()
{
    CheckBufferOverrun_LocalVariable();
    CheckBufferOverrun_StructVariable();
}
//---------------------------------------------------------------------------









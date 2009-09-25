/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

#include "tokenize.h"
#include "errorlogger.h"
#include "mathlib.h"

#include <algorithm>
#include <sstream>
#include <list>
#include <cstring>
#include <cctype>


#include <cstdlib>     // <- strtoul

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckBufferOverrun instance;
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::arrayIndexOutOfBounds(const Token *tok)
{
    if (!tok)
        arrayIndexOutOfBounds();
    else
    {
        _callStack.push_back(tok);
        arrayIndexOutOfBounds();
        _callStack.pop_back();
    }
}

void CheckBufferOverrun::arrayIndexOutOfBounds()
{
    reportError(_callStack, Severity::possibleError, "arrayIndexOutOfBounds", "Array index out of bounds");
}

void CheckBufferOverrun::bufferOverrun(const Token *tok)
{
    reportError(tok, Severity::possibleError, "bufferOverrun", "Buffer overrun");
}

void CheckBufferOverrun::strncatUsage(const Token *tok)
{
    reportError(tok, Severity::possibleError, "strncatUsage", "Dangerous usage of strncat. Tip: the 3rd parameter means maximum number of characters to append");
}

void CheckBufferOverrun::outOfBounds(const Token *tok, const std::string &what)
{
    reportError(tok, Severity::error, "outOfBounds", what + " is out of bounds");
}

void CheckBufferOverrun::sizeArgumentAsChar(const Token *tok)
{
    reportError(tok, Severity::possibleError, "sizeArgumentAsChar", "The size argument is given as a char constant");
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

void CheckBufferOverrun::checkScope(const Token *tok, const char *varname[], const int size, const int total_size, unsigned int varid)
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
                if (Token::Match(tok->next(), "( %varid% , %any% , %any% )", varid) ||
                    Token::Match(tok->next(), "( %var% , %varid% , %any% )", varid))
                {
                    const Token *tokSz = tok->tokAt(6);
                    if (tokSz->str()[0] == '\'')
                        sizeArgumentAsChar(tok);
                    else if (tokSz->isNumber())
                    {
                        const char *num  = tok->strAt(6);
                        if (std::atoi(num) > total_size)
                        {
                            bufferOverrun(tok);
                        }
                    }
                }
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
                    bufferOverrun(tok);
                }
            }
            continue;
        }


        // Loop..
        if (Token::simpleMatch(tok, "for ("))
        {
            const Token *tok2 = tok->tokAt(2);

            unsigned int counter_varid = 0;
            std::string min_counter_value;
            std::string max_counter_value;

            // for - setup..
            if (Token::Match(tok2, "%var% = %any% ;"))
            {
                if (tok2->tokAt(2)->isNumber())
                {
                    min_counter_value = tok2->strAt(2);
                }

                counter_varid = tok2->varId();
                tok2 = tok2->tokAt(4);
            }
            else if (Token::Match(tok2, "%type% %var% = %any% ;"))
            {
                if (tok2->tokAt(3)->isNumber())
                {
                    min_counter_value = tok2->strAt(3);
                }

                counter_varid = tok2->next()->varId();
                tok2 = tok2->tokAt(5);
            }
            else if (Token::Match(tok2, "%type% %type% %var% = %any% ;"))
            {
                if (tok->tokAt(4)->isNumber())
                {
                    min_counter_value = tok2->strAt(4);
                }

                counter_varid = tok2->tokAt(2)->varId();
                tok2 = tok2->tokAt(6);
            }
            else
                continue;

            if (counter_varid)
            {
                if (Token::Match(tok2, "%varid% < %num% ;", counter_varid))
                {
                    max_counter_value = MathLib::toString<long>(std::atol(tok2->strAt(2)) - 1);
                }
                else if (Token::Match(tok2, "%varid% <= %num% ;", counter_varid))
                {
                    max_counter_value = tok2->strAt(2);
                }
            }

            // Get index variable and stopsize.
            const char *strindex = tok2->str().c_str();
            bool condition_out_of_bounds = true;
            int value = ((tok2->strAt(1)[1] == '=') ? 1 : 0) + std::atoi(tok2->strAt(2));
            if (value <= size)
                condition_out_of_bounds = false;;

            // Goto the end paranthesis of the for-statement: "for (x; y; z)" ..
            tok2 = tok->next()->link();
            if (!tok2 || !tok2->tokAt(5))
                break;

            std::ostringstream pattern;
            pattern << varnames << " [ " << strindex << " ]";

            int indentlevel2 = 0;
            while ((tok2 = tok2->next()) != 0)
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

                if (tok2->str() == "if")
                {
                    // Bailout
                    break;
                }

                if (Token::Match(tok2, pattern.str().c_str()) && condition_out_of_bounds)
                {
                    bufferOverrun(tok2);
                    break;
                }

                else if (varid > 0 && counter_varid > 0 && !min_counter_value.empty() && !max_counter_value.empty())
                {
                    int min_index = 0;
                    int max_index = 0;

                    if (Token::Match(tok2, "%varid% [ %var% +|-|*|/ %num% ]", varid) &&
                        tok2->tokAt(2)->varId() == counter_varid)
                    {
                        char action = *(tok2->strAt(3));
                        const std::string &second(tok2->tokAt(4)->str());

                        //printf("min_index: %s %c %s\n", min_counter_value.c_str(), action, second.c_str());
                        //printf("max_index: %s %c %s\n", max_counter_value.c_str(), action, second.c_str());

                        min_index = std::atoi(MathLib::calculate(min_counter_value, second, action).c_str());
                        max_index = std::atoi(MathLib::calculate(max_counter_value, second, action).c_str());
                    }
                    else if (Token::Match(tok2, "%varid% [ %num% +|-|*|/ %var% ]", varid) &&
                             tok2->tokAt(4)->varId() == counter_varid)
                    {
                        char action = *(tok2->strAt(3));
                        const std::string &first(tok2->tokAt(2)->str());

                        //printf("min_index: %s %c %s\n", first.c_str(), action, min_counter_value.c_str());
                        //printf("max_index: %s %c %s\n", first.c_str(), action, max_counter_value.c_str());

                        min_index = std::atoi(MathLib::calculate(first, min_counter_value, action).c_str());
                        max_index = std::atoi(MathLib::calculate(first, max_counter_value, action).c_str());
                    }

                    //printf("min_index = %d, max_index = %d, size = %d\n", min_index, max_index, size);
                    if (min_index >= size || max_index >= size)
                    {
                        arrayIndexOutOfBounds(tok2->next());
                    }
                }

            }
            continue;
        }


        // Writing data into array..
        if (Token::Match(tok, ("strcpy|strcat ( " + varnames + " , %str% )").c_str()))
        {
            size_t len = Token::getStrLength(tok->tokAt(varc + 4));
            if (len >= static_cast<size_t>(size))
            {
                bufferOverrun(tok);
                continue;
            }
        }


        // Dangerous usage of strncat..
        if (varid > 0 && Token::Match(tok, "strncat ( %varid% , %any% , %num% )", varid))
        {
            int n = std::atoi(tok->strAt(6));
            if (n >= (size - 1))
                strncatUsage(tok);
        }


        // Dangerous usage of strncpy + strncat..
        if (varid > 0 && Token::Match(tok, "strncpy|strncat ( %varid% , %any% , %num% ) ; strncat ( %varid% , %any% , %num% )", varid))
        {
            int n = std::atoi(tok->strAt(6)) + std::atoi(tok->strAt(15));
            if (n > size)
                strncatUsage(tok->tokAt(9));
        }

        // Detect few strcat() calls
        if (varid > 0 && Token::Match(tok, "strcat ( %varid% , %str% ) ;", varid))
        {
            size_t charactersAppend = 0;
            const Token *tok2 = tok;

            while (tok2 && Token::Match(tok2, "strcat ( %varid% , %str% ) ;", varid))
            {
                charactersAppend += Token::getStrLength(tok2->tokAt(4));
                if (charactersAppend >= static_cast<size_t>(size))
                {
                    bufferOverrun(tok2);
                    break;
                }
                tok2 = tok2->tokAt(7);
            }
        }

        // sprintf..
        if (varid > 0 && Token::Match(tok, "sprintf ( %varid% , %str% [,)]", varid))
        {
            int len = -2;

            // check format string
            const char *fmt = tok->strAt(4);
            while (*fmt)
            {
                if (*fmt == '\\')
                {
                    ++fmt;
                }
                else if (*fmt == '%')
                {
                    ++fmt;

                    // skip field width
                    while (std::isdigit(*fmt))
                    {
                        ++fmt;
                    }

                    // FIXME: better handling for format specifiers
                    ++fmt;
                    continue;
                }
                ++fmt;
                ++len;
            }

            if (len >= (int)size)
            {
                bufferOverrun(tok);
            }

            // check arguments (if they exists)
            if (tok->tokAt(5)->str() == ",")
            {
                len = 0;
                const Token *end = tok->next()->link();
                for (const Token *tok2 = tok->tokAt(6); tok2 && tok2 != end; tok2 = tok2->next())
                {
                    if (tok2->str()[0] == '\"')
                    {
                        len += (int)Token::getStrLength(tok2);
                    }
                }
                if (len >= (int)size)
                {
                    bufferOverrun(tok);
                }
            }
        }

        // snprintf..
        if (varid > 0 && Token::Match(tok, "snprintf ( %varid% , %num% ,", varid))
        {
            int n = std::atoi(tok->strAt(4));
            if (n > size)
                outOfBounds(tok->tokAt(4), "snprintf size");
        }

        // cin..
        if (varid > 0 && Token::Match(tok, "cin >> %varid% ;", varid))
        {
            bufferOverrun(tok);
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
            if (!_settings->_showAll)
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
            const Token *ftok = _tokenizer->getFunctionTokenByName(tok->str().c_str());
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
                    parname[0] = ftok->str().c_str();
                    parname[1] = 0;

                    // Goto function body..
                    while (ftok && (ftok->str() != "{"))
                        ftok = ftok->next();
                    ftok = ftok ? ftok->next() : 0;

                    // Check variable usage in the function..
                    _callStack.push_back(tok);
                    checkScope(ftok, parname, size, total_size, 0);
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

void CheckBufferOverrun::checkGlobalAndLocalVariable()
{
    int indentlevel = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
            --indentlevel;

        const char *varname[2] = {0};
        unsigned int size = 0;
        const char *type = 0;
        unsigned int varid = 0;
        int nextTok = 0;

        // if the previous token exists, it must be either a variable name or "[;{}]"
        if (tok->previous() && (!tok->previous()->isName() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        if (Token::Match(tok, "%type% *| %var% [ %num% ] [;=]"))
        {
            unsigned int varpos = 1;
            if (tok->next()->str() == "*")
                ++varpos;
            varname[0] = tok->strAt(varpos);
            size = std::strtoul(tok->strAt(varpos + 2), NULL, 10);
            type = tok->strAt(varpos - 1);
            varid = tok->tokAt(varpos)->varId();
            nextTok = varpos + 5;
        }
        else if (indentlevel > 0 && Token::Match(tok, "[*;{}] %var% = new %type% [ %num% ]"))
        {
            varname[0] = tok->strAt(1);
            size = std::strtoul(tok->strAt(6), NULL, 10);
            type = tok->strAt(4);
            varid = tok->tokAt(1)->varId();
            nextTok = 8;
        }
        else if (indentlevel > 0 && Token::Match(tok, "[*;{}] %var% = malloc ( %num% ) ;"))
        {
            varname[0] = tok->strAt(1);
            size = std::strtoul(tok->strAt(5), NULL, 10);
            type = "char";
            varid = tok->tokAt(1)->varId();
            nextTok = 7;
        }
        else
        {
            continue;
        }

        int total_size = size * ((*type == '*') ? 4 : _tokenizer->sizeOfType(type));
        if (total_size == 0)
            continue;

        // The callstack is empty
        _callStack.clear();
        checkScope(tok->tokAt(nextTok), varname, size, total_size, varid);
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checking member variables of structs..
//---------------------------------------------------------------------------

void CheckBufferOverrun::checkStructVariable()
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
            int total_size = arrsize * _tokenizer->sizeOfType(tok2->strAt(1));
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
                            checkScope(tok4->tokAt(2), names, arrsize, total_size, 0);
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
                checkScope(CheckTok, varname, arrsize, total_size, 0);
            }
        }
    }
}
//---------------------------------------------------------------------------

void CheckBufferOverrun::bufferOverrun()
{
    checkGlobalAndLocalVariable();
    checkStructVariable();
}
//---------------------------------------------------------------------------


int CheckBufferOverrun::count(const std::string &input_string)
{



    int flag = 1;
    int input_string_size = 0;
    int on_on_next = 0;
    std::string digits_string = "";
    int digits = 0;

    for (std::string::size_type i = 0; i != input_string.size(); i++)
    {

        if (on_on_next == 1)
        {
            flag = 1;
            on_on_next = 0;
        }
        switch (input_string[i])
        {
        case 'd':
        case 'i':
        case 'c':
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'o':
        case 's':
        case 'u':
        case 'x':
        case 'X':
        case 'p':
        case 'n':
            if (flag == 0) on_on_next = 1;
            break;
        case '%':
            if (flag == 1) flag = 0;
            break;
        case '/':
            input_string_size--;
            break;
        }

        if (flag) input_string_size++;
        else
            digits_string += input_string[i];

        if (on_on_next == 1 && flag == 0)
        {
            digits_string = digits_string.substr(1, digits_string.size());
            digits += abs(atoi(digits_string.c_str()));
            digits_string = "";

        }


    }


    return input_string_size + 1 + digits;
}





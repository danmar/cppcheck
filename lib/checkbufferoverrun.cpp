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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include <cassert>     // <- assert
#include <cstdlib>     // <- strtoul

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace
{
CheckBufferOverrun instance;
}

//---------------------------------------------------------------------------

void CheckBufferOverrun::arrayIndexOutOfBounds(const Token *tok, int size, int index)
{
    if (!tok)
        arrayIndexOutOfBounds(size, index);
    else
    {
        _callStack.push_back(tok);
        arrayIndexOutOfBounds(size, index);
        _callStack.pop_back();
    }
}

void CheckBufferOverrun::arrayIndexOutOfBounds(int size, int index)
{
    Severity::e severity;
    if (size <= 1 || _callStack.size() > 1)
    {
        severity = Severity::possibleError;
        if (_settings->_showAll == false)
            return;
    }
    else
    {
        severity = Severity::error;
    }

    if (_callStack.size() == 1)
    {
        std::ostringstream oss;
        oss << "Array '" << (*_callStack.begin())->str() << "[" << size << "]' index " << index << " out of bounds";
        reportError(_callStack, severity, "arrayIndexOutOfBounds", oss.str().c_str());
    }
    else
        reportError(_callStack, severity, "arrayIndexOutOfBounds", "Array index out of bounds");
}

void CheckBufferOverrun::bufferOverrun(const Token *tok)
{
    Severity::e severity;
    if (_callStack.size() > 0)
    {
        severity = Severity::possibleError;
        if (_settings->_showAll == false)
            return;
    }
    else
    {
        severity = Severity::error;
    }

    reportError(tok, severity, "bufferAccessOutOfBounds", "Buffer access out-of-bounds");
}

void CheckBufferOverrun::dangerousStdCin(const Token *tok)
{
    if (_settings && _settings->_showAll == false)
        return;

    reportError(tok, Severity::possibleError, "dangerousStdCin", "Dangerous usage of std::cin, possible buffer overrun");
}

void CheckBufferOverrun::strncatUsage(const Token *tok)
{
    if (_settings && _settings->_showAll == false)
        return;

    reportError(tok, Severity::possibleError, "strncatUsage", "Dangerous usage of strncat. Tip: the 3rd parameter means maximum number of characters to append");
}

void CheckBufferOverrun::outOfBounds(const Token *tok, const std::string &what)
{
    reportError(tok, Severity::error, "outOfBounds", what + " is out of bounds");
}

void CheckBufferOverrun::sizeArgumentAsChar(const Token *tok)
{
    if (_settings && _settings->_showAll == false)
        return;

    reportError(tok, Severity::possibleError, "sizeArgumentAsChar", "The size argument is given as a char constant");
}


void CheckBufferOverrun::terminateStrncpyError(const Token *tok)
{
    reportError(tok, Severity::style, "terminateStrncpy", "After a strncpy() the buffer should be zero-terminated");
}


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

void CheckBufferOverrun::checkScope(const Token *tok, const std::vector<std::string> &varname, const int size, const int total_size, unsigned int varid)
{
    std::string varnames;
    for (unsigned int i = 0; i < varname.size(); ++i)
        varnames += (i == 0 ? "" : " . ") + varname[i];

    unsigned int varc = varname.size();
    if (varc == 0)
        varc = 1;

    varc = 2 * (varc - 1);
    if (Token::Match(tok, "return"))
    {
        tok = tok->next();
        if (!tok)
            return;
    }

    // Array index..
    if (varid > 0)
    {
        if (Token::Match(tok, "%varid% [ %num% ]", varid))
        {
            int index = MathLib::toLongNumber(tok->strAt(2));
            if (index < 0 || index >= size)
            {
                arrayIndexOutOfBounds(tok, size, index);
            }
        }
    }
    else if (Token::Match(tok, std::string(varnames + " [ %num% ]").c_str()))
    {
        int index = MathLib::toLongNumber(tok->strAt(2 + varc));
        if (index < 0 || index >= size)
        {
            arrayIndexOutOfBounds(tok->tokAt(varc), size, index);
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

        if (varid != 0 && Token::Match(tok, "%varid% = new|malloc|realloc", varid))
        {
            // Abort
            break;
        }

        // Array index..
        if (varid > 0)
        {
            if (!tok->isName() && !Token::Match(tok, "[.&]") && Token::Match(tok->next(), "%varid% [ %num% ]", varid))
            {
                int index = MathLib::toLongNumber(tok->strAt(3));
                if (index < 0 || index >= size)
                {
                    if (index > size || !Token::Match(tok->previous(), "& ("))
                    {
                        arrayIndexOutOfBounds(tok->next(), size, index);
                    }
                }
            }
        }
        else if (!tok->isName() && !Token::Match(tok, "[.&]") && Token::Match(tok->next(), std::string(varnames + " [ %num% ]").c_str()))
        {
            int index = MathLib::toLongNumber(tok->strAt(3 + varc));
            if (index < 0 || index >= size)
            {
                arrayIndexOutOfBounds(tok->tokAt(1 + varc), size, index);
            }
            tok = tok->tokAt(4);
            continue;
        }


        // memset, memcmp, memcpy, strncpy, fgets..
        if (varid > 0)
        {
            if (_settings->_checkCodingStyle)
            {
                // check for strncpy which is not terminated
                if (Token::Match(tok, "strncpy ( %varid% , %any% , %any% )", varid))
                {
                    const Token *tokSz = tok->tokAt(6);
                    if (tokSz->isNumber())
                    {
                        // strncpy takes entire variable length as input size
                        const std::string num = tok->strAt(6);
                        if (MathLib::toLongNumber(num) == total_size)
                        {
                            const Token *tok2 = tok->next()->link()->next()->next();
                            for (; tok2; tok2 = tok2->next())
                            {
                                if (Token::Match(tok2, "%varid%", tok->tokAt(2)->varId()))
                                {
                                    if (!Token::Match(tok2, "%varid% [ %any% ]  = 0 ;", tok->tokAt(2)->varId()))
                                    {
                                        terminateStrncpyError(tok);
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (Token::Match(tok, "memset|memcpy|memmove|memcmp|strncpy|fgets ( %varid% , %any% , %any% )", varid) ||
                Token::Match(tok, "memset|memcpy|memmove|memcmp|fgets ( %var% , %varid% , %any% )", varid))
            {
                const Token *tokSz = tok->tokAt(6);
                if (tokSz->str()[0] == '\'')
                    sizeArgumentAsChar(tok);
                else if (tokSz->isNumber())
                {
                    const std::string num = tok->strAt(6);
                    if (MathLib::toLongNumber(num) < 0 || MathLib::toLongNumber(num) > total_size)
                    {
                        bufferOverrun(tok);
                    }
                }
            }
        }
        else if (Token::Match(tok, "memset|memcpy|memmove|memcmp|strncpy|fgets"))
        {
            if (Token::Match(tok->next(), std::string("( " + varnames + " , %num% , %num% )").c_str()) ||
                Token::Match(tok->next(), std::string("( %var% , " + varnames + " , %num% )").c_str()))
            {
                const std::string num  = tok->strAt(varc + 6);
                if (MathLib::toLongNumber(num) < 0 || MathLib::toLongNumber(num) > total_size)
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

            int value = 0;
            if (counter_varid == 0)
                continue;

            const Token *strindextoken;
            if (Token::Match(tok2, "%varid% < %num% ;", counter_varid))
            {
                value = MathLib::toLongNumber(tok2->strAt(2));
                max_counter_value = MathLib::toString<long>(value - 1);
                strindextoken = tok2;
            }
            else if (Token::Match(tok2, "%varid% <= %num% ;", counter_varid))
            {
                value = MathLib::toLongNumber(tok2->strAt(2)) + 1;
                max_counter_value = tok2->strAt(2);
                strindextoken = tok2;
            }
            else if (Token::Match(tok2, " %num% < %varid% ;", counter_varid))
            {
                max_counter_value = min_counter_value;
                min_counter_value = MathLib::toString<long>(value + 1);
                value = MathLib::toLongNumber(max_counter_value.c_str());
                strindextoken = tok2->tokAt(2);
            }
            else if (Token::Match(tok2, "%num% <= %varid% ;", counter_varid))
            {
                max_counter_value = min_counter_value;
                min_counter_value = tok2->str();
                value = MathLib::toLongNumber(max_counter_value.c_str());
                strindextoken = tok2->tokAt(2);
            }

            // Get index variable and stopsize.
            const char *strindex = strindextoken->str().c_str();
            bool condition_out_of_bounds = true;
            if (value <= size)
                condition_out_of_bounds = false;

            const Token *tok3 = tok2->tokAt(4);
            assert(tok3 != NULL);
            if (Token::Match(tok3, "%varid% += %num% )", counter_varid) ||
                Token::Match(tok3, "%varid%  = %num% + %varid% )", counter_varid))
            {
                if (!MathLib::isInt(tok3->strAt(2)))
                    continue;

                const int num = MathLib::toLongNumber(tok3->strAt(2));

                // We have for example code: "for(i=2;i<22;i+=6)
                // We can calculate that max value for i is 20, not 21
                // 21-2 = 19
                // 19/6 = 3
                // 6*3+2 = 20
                long max = MathLib::toLongNumber(max_counter_value);
                long min = MathLib::toLongNumber(min_counter_value);
                max = ((max - min) / num) * num + min;
                max_counter_value = MathLib::toString<long>(max);
                if (max <= size)
                    condition_out_of_bounds = false;
            }
            else if (Token::Match(tok3, "%varid% = %varid% + %num% )", counter_varid))
            {
                if (!MathLib::isInt(tok3->strAt(4)))
                    continue;

                const int num = MathLib::toLongNumber(tok3->strAt(4));
                long max = MathLib::toLongNumber(max_counter_value);
                long min = MathLib::toLongNumber(min_counter_value);
                max = ((max - min) / num) * num + min;
                max_counter_value = MathLib::toString<long>(max);
                if (max <= size)
                    condition_out_of_bounds = false;
            }
            else if (Token::Match(tok3, "%varid% -= %num% )", counter_varid) ||
                     Token::Match(tok3, "%varid%  = %num% - %varid% )", counter_varid))
            {
                if (!MathLib::isInt(tok3->strAt(2)))
                    continue;

                const int num = MathLib::toLongNumber(tok3->strAt(2));

                long max = MathLib::toLongNumber(max_counter_value);
                long min = MathLib::toLongNumber(min_counter_value);
                max = ((max - min) / num) * num + min;
                max_counter_value = MathLib::toString<long>(max);
                if (max <= size)
                    condition_out_of_bounds = false;
            }
            else if (Token::Match(tok3, "%varid% = %varid% - %num% )", counter_varid))
            {
                if (!MathLib::isInt(tok3->strAt(4)))
                    continue;

                const int num = MathLib::toLongNumber(tok3->strAt(4));
                long max = MathLib::toLongNumber(max_counter_value);
                long min = MathLib::toLongNumber(min_counter_value);
                max = ((max - min) / num) * num + min;
                max_counter_value = MathLib::toString<long>(max);
                if (max <= size)
                    condition_out_of_bounds = false;
            }
            else if (! Token::Match(tok3, "++|--| %varid% ++|--| )", counter_varid))
            {
                continue;
            }

            // Goto the end paranthesis of the for-statement: "for (x; y; z)" ..
            tok2 = tok->next()->link();
            if (!tok2 || !tok2->tokAt(5))
                break;

            // Check is the counter variable increased elsewhere inside the loop or used
            // for anything else except reading
            bool bailOut = false;
            for (Token *loopTok = tok2->next(); loopTok && loopTok != tok2->next()->link(); loopTok = loopTok->next())
            {
                if (loopTok->varId() == counter_varid)
                {
                    // Counter variable used inside loop
                    if (Token::Match(loopTok->next(), "+=|-=|++|--|=") ||
                        Token::Match(loopTok->previous(), "++|--"))
                    {
                        bailOut = true;
                        break;
                    }
                }
            }

            if (bailOut)
            {
                break;
            }

            std::ostringstream pattern;
            if (varid > 0)
                pattern << "%varid% [ " << strindex << " ]";
            else
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

                if (Token::Match(tok2, "if|switch"))
                {
                    // Bailout
                    break;
                }

                if (condition_out_of_bounds && Token::Match(tok2, pattern.str().c_str(), varid))
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
                        const char action = tok2->strAt(3)[0];
                        const std::string &second(tok2->tokAt(4)->str());

                        //printf("min_index: %s %c %s\n", min_counter_value.c_str(), action, second.c_str());
                        //printf("max_index: %s %c %s\n", max_counter_value.c_str(), action, second.c_str());

                        min_index = std::atoi(MathLib::calculate(min_counter_value, second, action).c_str());
                        max_index = std::atoi(MathLib::calculate(max_counter_value, second, action).c_str());
                    }
                    else if (Token::Match(tok2, "%varid% [ %num% +|-|*|/ %var% ]", varid) &&
                             tok2->tokAt(4)->varId() == counter_varid)
                    {
                        const char action = tok2->strAt(3)[0];
                        const std::string &first(tok2->tokAt(2)->str());

                        //printf("min_index: %s %c %s\n", first.c_str(), action, min_counter_value.c_str());
                        //printf("max_index: %s %c %s\n", first.c_str(), action, max_counter_value.c_str());

                        min_index = std::atoi(MathLib::calculate(first, min_counter_value, action).c_str());
                        max_index = std::atoi(MathLib::calculate(first, max_counter_value, action).c_str());
                    }

                    //printf("min_index = %d, max_index = %d, size = %d\n", min_index, max_index, size);
                    if (min_index >= size || max_index >= size)
                    {
                        arrayIndexOutOfBounds(tok2, size, min_index > max_index ? min_index : max_index);
                    }
                }

            }
            continue;
        }


        // Writing data into array..
        if ((varid > 0 && Token::Match(tok, "strcpy|strcat ( %varid% , %str% )", varid)) ||
            (varid == 0 && Token::Match(tok, ("strcpy|strcat ( " + varnames + " , %str% )").c_str())))
        {
            size_t len = Token::getStrLength(tok->tokAt(varc + 4));
            if (len >= static_cast<size_t>(total_size))
            {
                bufferOverrun(tok);
                continue;
            }
        }

        // Writing data into array..
        if (varid > 0 &&
            Token::Match(tok, "read|write ( %any% , %varid% , %num% )", varid) &&
            MathLib::isInt(tok->strAt(6)))
        {
            size_t len = MathLib::toLongNumber(tok->strAt(6));
            if (len > static_cast<size_t>(total_size))
            {
                bufferOverrun(tok);
                continue;
            }
        }

        // Writing data into array..
        if (varid > 0 &&
            Token::Match(tok, "fgets ( %varid% , %num% , %any% )", varid) &&
            MathLib::isInt(tok->strAt(4)))
        {
            size_t len = MathLib::toLongNumber(tok->strAt(4));
            if (len > static_cast<size_t>(total_size))
            {
                bufferOverrun(tok);
                continue;
            }
        }

        // Dangerous usage of strncat..
        if (varid > 0 && Token::Match(tok, "strncat ( %varid% , %any% , %num% )", varid))
        {
            int n = MathLib::toLongNumber(tok->strAt(6));
            if (n >= total_size)
                strncatUsage(tok);
        }


        // Dangerous usage of strncpy + strncat..
        if (varid > 0 && Token::Match(tok, "strncpy|strncat ( %varid% , %any% , %num% ) ; strncat ( %varid% , %any% , %num% )", varid))
        {
            int n = MathLib::toLongNumber(tok->strAt(6)) + MathLib::toLongNumber(tok->strAt(15));
            if (n > total_size)
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
                if (charactersAppend >= static_cast<size_t>(total_size))
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
            checkSprintfCall(tok, total_size);
        }

        // snprintf..
        if (varid > 0 && Token::Match(tok, "snprintf ( %varid% , %num% ,", varid))
        {
            int n = MathLib::toLongNumber(tok->strAt(4));
            if (n > total_size)
                outOfBounds(tok->tokAt(4), "snprintf size");
        }

        // cin..
        if (varid > 0 && Token::Match(tok, "cin >> %varid% ;", varid))
        {
            dangerousStdCin(tok);
        }

        // Function call..
        // It's not interesting to check what happens when the whole struct is
        // sent as the parameter, that is checked separately anyway.
        if (Token::Match(tok, "%var% ("))
        {
            // Only perform this checking if showAll setting is enabled..
            if (!_settings->_showAll)
                continue;

            unsigned int parlevel = 0, par = 0;
            const Token * tok1 = tok;
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

                if (parlevel == 1)
                {
                    if (varid > 0 && Token::Match(tok2, std::string("[(,] %varid% [,)]").c_str(), varid))
                    {
                        ++par;
                        tok1 = tok2->next();
                        break;
                    }
                    else if (varid == 0 &&  Token::Match(tok2, std::string("[(,] " + varnames + " [,)]").c_str()))
                    {
                        ++par;
                        tok1 = tok2->tokAt(varc + 1);
                        break;
                    }
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
                    std::vector<std::string> parname;
                    parname.push_back(ftok->str());

                    const unsigned int parId = ftok->varId();

                    // Goto function body..
                    while (ftok && (ftok->str() != "{"))
                        ftok = ftok->next();
                    ftok = ftok ? ftok->next() : 0;

                    // Don't make recursive checking..
                    if (std::find(_callStack.begin(), _callStack.end(), tok1) != _callStack.end())
                        continue;

                    // Check variable usage in the function..
                    _callStack.push_back(tok1);
                    checkScope(ftok, parname, size, total_size, parId);
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

        unsigned int size = 0;
        std::string type;
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
            size = MathLib::toLongNumber(tok->strAt(varpos + 2));
            type = tok->strAt(varpos - 1);
            varid = tok->tokAt(varpos)->varId();
            nextTok = varpos + 5;
        }
        else if (indentlevel > 0 && Token::Match(tok, "[*;{}] %var% = new %type% [ %num% ]"))
        {
            size = MathLib::toLongNumber(tok->strAt(6));
            type = tok->strAt(4);
            varid = tok->tokAt(1)->varId();
            nextTok = 8;
        }
        else if (indentlevel > 0 && Token::Match(tok, "[*;{}] %var% = new %type% ( %num% )"))
        {
            size = 1;
            type = tok->strAt(4);
            varid = tok->tokAt(1)->varId();
            nextTok = 8;
        }
        else if (indentlevel > 0 && Token::Match(tok, "[*;{}] %var% = malloc ( %num% ) ;"))
        {
            size = MathLib::toLongNumber(tok->strAt(5));
            type = "char";
            varid = tok->tokAt(1)->varId();
            nextTok = 7;

            // "int * x ; x = malloc (y);"
            const Token *declTok = tok->tokAt(-3);
            if (varid > 0 && declTok && Token::Match(declTok, "%type% * %varid% ;", varid))
            {
                type = declTok->strAt(0);
                // malloc() gets count of bytes and not count of
                // elements, so we should calculate count of elements
                // manually
                unsigned int sizeOfType = _tokenizer->sizeOfType(declTok);
                if (sizeOfType > 0)
                    size /= sizeOfType;
            }
        }
        else
        {
            continue;
        }

        if (varid == 0)
            continue;

        Token sizeTok(0);
        sizeTok.str(type);
        int total_size = size * _tokenizer->sizeOfType(&sizeTok);
        if (total_size == 0)
            continue;

        // The callstack is empty
        _callStack.clear();
        std::vector<std::string> v;
        checkScope(tok->tokAt(nextTok), v, size, total_size, varid);
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

            std::vector<std::string> varname(2, "");
            const unsigned int varId = tok2->tokAt(ivar)->varId();
            varname[1] = tok2->strAt(ivar);
            int arrsize = MathLib::toLongNumber(tok2->strAt(ivar + 2));
            int total_size = arrsize * _tokenizer->sizeOfType(tok2->tokAt(1));
            if (tok2->tokAt(2)->str() == "*")
                total_size = arrsize * _tokenizer->sizeOfType(tok2->tokAt(2));
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
                            std::vector<std::string> v;
                            checkScope(tok4->tokAt(2), v, arrsize, total_size, varId);
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


int CheckBufferOverrun::countSprintfLength(const std::string &input_string, const std::list<const Token*> &parameters)
{
    bool percentCharFound = false;
    int input_string_size = 1;
    bool handleNextParameter = false;
    std::string digits_string = "";
    bool i_d_x_f_found = false;
    std::list<const Token*>::const_iterator paramIter = parameters.begin();
    unsigned int parameterLength = 0;
    for (std::string::size_type i = 0; i < input_string.length(); ++i)
    {
        if (input_string[i] == '\\')
        {
            if (input_string[i+1] == '0')
                break;

            ++input_string_size;
            ++i;
            continue;
        }

        if (percentCharFound)
        {
            switch (input_string[i])
            {
            case 'f':
            case 'x':
            case 'X':
            case 'i':
                i_d_x_f_found = true;
            case 'c':
            case 'e':
            case 'E':
            case 'g':
            case 'o':
            case 'u':
            case 'p':
            case 'n':
                handleNextParameter = true;
                break;
            case 'd':
                i_d_x_f_found = true;
                if (paramIter != parameters.end() && *paramIter && (*paramIter)->str()[0] != '"')
                    parameterLength = (*paramIter)->str().length();

                handleNextParameter = true;
                break;
            case 's':
                if (paramIter != parameters.end() && *paramIter && (*paramIter)->str()[0] == '"')
                    parameterLength = Token::getStrLength(*paramIter);

                handleNextParameter = true;
                break;
            }
        }

        if (input_string[i] == '%')
            percentCharFound = !percentCharFound;
        else if (percentCharFound)
        {
            digits_string.append(1, input_string[i]);
        }

        if (!percentCharFound)
            input_string_size++;

        if (handleNextParameter)
        {
            unsigned int tempDigits = std::abs(std::atoi(digits_string.c_str()));
            if (i_d_x_f_found)
                tempDigits = std::max(static_cast<int>(tempDigits), 1);

            if (digits_string.find('.') != std::string::npos)
            {
                const std::string endStr = digits_string.substr(digits_string.find('.') + 1);
                unsigned int maxLen = std::max(std::abs(std::atoi(endStr.c_str())), 1);

                if (input_string[i] == 's')
                {
                    // For strings, the length after the dot "%.2s" will limit
                    // the length of the string.
                    if (parameterLength > maxLen)
                        parameterLength = maxLen;
                }
                else
                {
                    // For integers, the length after the dot "%.2d" can
                    // increase required length
                    if (tempDigits < maxLen)
                        tempDigits = maxLen;
                }
            }

            if (tempDigits < parameterLength)
                input_string_size += parameterLength;
            else
                input_string_size += tempDigits;

            parameterLength = 0;
            digits_string = "";
            i_d_x_f_found = false;
            percentCharFound = false;
            handleNextParameter = false;
            if (paramIter != parameters.end())
                ++paramIter;
        }
    }

    return input_string_size;
}


void CheckBufferOverrun::checkSprintfCall(const Token *tok, int size)
{
    std::list<const Token*> parameters;
    if (tok->tokAt(5)->str() == ",")
    {
        const Token *end = tok->next()->link();
        for (const Token *tok2 = tok->tokAt(5); tok2 && tok2 != end; tok2 = tok2->next())
        {
            if (Token::Match(tok2, ", %any% [,)]"))
            {

                if (Token::Match(tok2->next(), "%str%"))
                    parameters.push_back(tok2->next());

                else if (Token::Match(tok2->next(), "%num%"))
                    parameters.push_back(tok2->next());

                // TODO, get value of the variable if possible and use that instead of 0
                else
                    parameters.push_back(0);
            }
            else
            {
                // Parameter is more complex, than just a value or variable. Ignore it for now
                // and skip to next token.
                parameters.push_back(0);
                int ind = 0;
                for (const Token *tok3 = tok2->next(); tok3; tok3 = tok3->next())
                {
                    if (tok3->str() == "(")
                        ++ind;

                    else if (tok3->str() == ")")
                    {
                        --ind;
                        if (ind < 0)
                            break;
                    }

                    else if (ind == 0 && tok3->str() == ",")
                    {
                        tok2 = tok3->previous();
                        break;
                    }
                }

                if (ind < 0)
                    break;
            }
        }
    }

    int len = countSprintfLength(tok->tokAt(4)->strValue(), parameters);
    if (len > size)
    {
        bufferOverrun(tok);
    }
}


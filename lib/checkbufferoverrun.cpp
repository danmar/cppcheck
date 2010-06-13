/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#include <climits>

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
    if (size > 1)
    {
        std::ostringstream errmsg;
        errmsg << "Array '";
        if (tok)
            errmsg << tok->str();
        else
            errmsg << "array";
        errmsg << "[" << size << "]' index " << index << " out of bounds";
        reportError(tok, Severity::error, "arrayIndexOutOfBounds", errmsg.str().c_str());
    }
}

void CheckBufferOverrun::arrayIndexOutOfBounds(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<int> &index)
{
    std::ostringstream oss;
    oss << "Array '" << arrayInfo.varname;
    for (unsigned int i = 0; i < arrayInfo.num.size(); ++i)
        oss << "[" << arrayInfo.num[i] << "]";
    oss << "' index ";
    if (index.size() == 1)
        oss << index[0];
    else
    {
        oss << arrayInfo.varname;
        for (unsigned int i = 0; i < index.size(); ++i)
            oss << "[" << index[i] << "]";
    }
    oss << " out of bounds";
    reportError(tok, Severity::error, "arrayIndexOutOfBounds", oss.str().c_str());
}

void CheckBufferOverrun::bufferOverrun(const Token *tok, const std::string &varnames)
{
    std::string v = varnames;
    while (v.find(" ") != std::string::npos)
        v.erase(v.find(" "), 1);

    std::string errmsg("Buffer access out-of-bounds");
    if (!v.empty())
        errmsg += ": " + v;

    reportError(tok, Severity::error, "bufferAccessOutOfBounds",  errmsg);
}

void CheckBufferOverrun::strncatUsage(const Token *tok)
{
    if (_settings && !_settings->_checkCodingStyle)
        return;

    reportError(tok, Severity::style, "strncatUsage", "Dangerous usage of strncat. Tip: the 3rd parameter means maximum number of characters to append");
}

void CheckBufferOverrun::outOfBounds(const Token *tok, const std::string &what)
{
    reportError(tok, Severity::error, "outOfBounds", what + " is out of bounds");
}

void CheckBufferOverrun::sizeArgumentAsChar(const Token *tok)
{
    if (_settings && !_settings->_checkCodingStyle)
        return;
    reportError(tok, Severity::style, "sizeArgumentAsChar", "The size argument is given as a char constant");
}


void CheckBufferOverrun::terminateStrncpyError(const Token *tok)
{
    reportError(tok, Severity::style, "terminateStrncpy", "After a strncpy() the buffer should be zero-terminated");
}

void CheckBufferOverrun::cmdLineArgsError(const Token *tok)
{
    reportError(tok, Severity::error, "insecureCmdLineArgs", "Buffer overrun possible for long cmd-line args");
}


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

/**
 * @brief This is a helper class to be used with std::find_if
 */
class TokenStrEquals
{
public:
    /**
     * @param str Token::str() is compared against this.
     */
    TokenStrEquals(const std::string &str)
        : value(str)
    {
    }

    /**
     * Called automatically by std::find_if
     * @param tok Token inside the list
     */
    bool operator()(const Token *tok) const
    {
        return value == tok->str();
    }

private:
    TokenStrEquals& operator=(const TokenStrEquals&); // disallow assignments

    const std::string value;
};

/**
 * Parse for loop initialization statement. Look for a counter variable
 * \param tok [in] first token inside the parentheses
 * \param varid [out] varid of counter variable
 * \param init_value [out] init value of counter variable
 * \return success => pointer to the for loop condition. fail => 0
 */
static const Token *for_init(const Token *tok, unsigned int &varid, std::string &init_value)
{
    if (Token::Match(tok, "%var% = %any% ;"))
    {
        if (tok->tokAt(2)->isNumber())
        {
            init_value = tok->strAt(2);
        }

        varid = tok->varId();
        tok = tok->tokAt(4);
    }
    else if (Token::Match(tok, "%type% %var% = %any% ;"))
    {
        if (tok->tokAt(3)->isNumber())
        {
            init_value = tok->strAt(3);
        }

        varid = tok->next()->varId();
        tok = tok->tokAt(5);
    }
    else if (Token::Match(tok, "%type% %type% %var% = %any% ;"))
    {
        if (tok->tokAt(4)->isNumber())
        {
            init_value = tok->strAt(4);
        }

        varid = tok->tokAt(2)->varId();
        tok = tok->tokAt(6);
    }
    else
        return 0;

    return tok;
}


/** Parse for condition */
static bool for_condition(const Token * const tok2, unsigned int varid, std::string &min_value, std::string &max_value, std::string &strindex, bool &maxMinFlipped)
{
    if (Token::Match(tok2, "%varid% < %num% ;", varid))
    {
        maxMinFlipped = false;
        long value = MathLib::toLongNumber(tok2->strAt(2));
        max_value = MathLib::toString<long>(value - 1);
    }
    else if (Token::Match(tok2, "%varid% <= %num% ;", varid))
    {
        maxMinFlipped = false;
        max_value = tok2->strAt(2);
    }
    else if (Token::Match(tok2, " %num% < %varid% ;", varid))
    {
        maxMinFlipped = true;
        long value = MathLib::toLongNumber(tok2->str());
        max_value = min_value;
        min_value = MathLib::toString<long>(value + 1);
    }
    else if (Token::Match(tok2, "%num% <= %varid% ;", varid))
    {
        maxMinFlipped = true;
        max_value = min_value;
        min_value = tok2->str();
    }
    else
    {
        return false;
    }

    strindex = tok2->isName() ? tok2->str() : tok2->strAt(2);

    return true;
}



/**
 * Parse the third sub-statement in for head
 * \param tok first token
 * \param varid variable id of counter
 * \param min_value min value of counter
 * \param max_value max value of counter
 * \param maxMinFlipped counting from max to min
 */
static bool for3(const Token * const tok,
                 unsigned int varid,
                 std::string &min_value,
                 std::string &max_value,
                 const bool maxMinFlipped)
{
    assert(tok != 0);
    if (Token::Match(tok, "%varid% += %num% )", varid) ||
        Token::Match(tok, "%varid%  = %num% + %varid% )", varid))
    {
        if (!MathLib::isInt(tok->strAt(2)))
            return false;

        const int num = MathLib::toLongNumber(tok->strAt(2));

        // We have for example code: "for(i=2;i<22;i+=6)
        // We can calculate that max value for i is 20, not 21
        // 21-2 = 19
        // 19/6 = 3
        // 6*3+2 = 20
        long max = MathLib::toLongNumber(max_value);
        long min = MathLib::toLongNumber(min_value);
        max = ((max - min) / num) * num + min;
        max_value = MathLib::toString<long>(max);
    }
    else if (Token::Match(tok, "%varid% = %varid% + %num% )", varid))
    {
        if (!MathLib::isInt(tok->strAt(4)))
            return false;

        const int num = MathLib::toLongNumber(tok->strAt(4));
        long max = MathLib::toLongNumber(max_value);
        long min = MathLib::toLongNumber(min_value);
        max = ((max - min) / num) * num + min;
        max_value = MathLib::toString<long>(max);
    }
    else if (Token::Match(tok, "%varid% -= %num% )", varid) ||
             Token::Match(tok, "%varid%  = %num% - %varid% )", varid))
    {
        if (!MathLib::isInt(tok->strAt(2)))
            return false;

        const int num = MathLib::toLongNumber(tok->strAt(2));

        long max = MathLib::toLongNumber(max_value);
        long min = MathLib::toLongNumber(min_value);
        max = ((max - min) / num) * num + min;
        max_value = MathLib::toString<long>(max);
    }
    else if (Token::Match(tok, "%varid% = %varid% - %num% )", varid))
    {
        if (!MathLib::isInt(tok->strAt(4)))
            return false;

        const int num = MathLib::toLongNumber(tok->strAt(4));
        long max = MathLib::toLongNumber(max_value);
        long min = MathLib::toLongNumber(min_value);
        max = ((max - min) / num) * num + min;
        max_value = MathLib::toString<long>(max);
    }
    else if (Token::Match(tok, "--| %varid% --| )", varid))
    {
        if (!maxMinFlipped && MathLib::toLongNumber(min_value) < MathLib::toLongNumber(max_value))
        {
            // Code relies on the fact that integer will overflow:
            // for (unsigned int i = 3; i < 5; --i)

            // Set min value in this case to zero.
            max_value = min_value;
            min_value = "0";
        }
    }
    else if (! Token::Match(tok, "++| %varid% ++| )", varid))
    {
        return false;
    }
    return true;
}



/**
 * Check is the counter variable increased elsewhere inside the loop or used
 * for anything else except reading
 * \param tok first token of for-body
 * \param varid counter variable id
 * \return bailout needed => true
 */
static bool for_bailout(const Token * const tok1, unsigned int varid)
{
    for (const Token *loopTok = tok1; loopTok && loopTok != tok1->link(); loopTok = loopTok->next())
    {
        if (loopTok->varId() == varid)
        {
            // Counter variable used inside loop
            if (Token::Match(loopTok->next(), "+=|-=|++|--|=") ||
                Token::Match(loopTok->previous(), "++|--"))
            {
                return true;
            }
        }
    }
    return false;
}


void CheckBufferOverrun::parse_for_body(const Token *tok2, const ArrayInfo &arrayInfo, const std::string &strindex, bool condition_out_of_bounds, unsigned int counter_varid, const std::string &min_counter_value, const std::string &max_counter_value)
{
    const std::string pattern((arrayInfo.varid ? std::string("%varid%") : arrayInfo.varname) + " [ " + strindex + " ]");

    int indentlevel2 = 0;
    for (; tok2; tok2 = tok2->next())
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

        if (condition_out_of_bounds && Token::Match(tok2, pattern.c_str(), arrayInfo.varid))
        {
            bufferOverrun(tok2, arrayInfo.varname);
            break;
        }

        else if (arrayInfo.varid && counter_varid > 0 && !min_counter_value.empty() && !max_counter_value.empty())
        {
            int min_index = 0;
            int max_index = 0;

            if (Token::Match(tok2, "%varid% [ %var% +|-|*|/ %num% ]", arrayInfo.varid) &&
                tok2->tokAt(2)->varId() == counter_varid)
            {
                const char action = tok2->strAt(3)[0];
                const std::string &second(tok2->tokAt(4)->str());

                //printf("min_index: %s %c %s\n", min_counter_value.c_str(), action, second.c_str());
                //printf("max_index: %s %c %s\n", max_counter_value.c_str(), action, second.c_str());

                min_index = std::atoi(MathLib::calculate(min_counter_value, second, action).c_str());
                max_index = std::atoi(MathLib::calculate(max_counter_value, second, action).c_str());
            }
            else if (Token::Match(tok2, "%varid% [ %num% +|-|*|/ %var% ]", arrayInfo.varid) &&
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
            if (min_index < 0 || max_index < 0)
            {
                arrayIndexOutOfBounds(tok2, (int)arrayInfo.num[0], std::min(min_index, max_index));
            }
            if (min_index >= (int)arrayInfo.num[0] || max_index >= (int)arrayInfo.num[0])
            {
                arrayIndexOutOfBounds(tok2, (int)arrayInfo.num[0], std::max(min_index, max_index));
            }
        }
    }
}



void CheckBufferOverrun::checkFunctionCall(const Token &tok, unsigned int par, const ArrayInfo &arrayInfo)
{
    std::map<std::string, unsigned int> total_size;
    total_size["fgets"] = 2;	// The second argument for fgets can't exceed the total size of the array
    total_size["memcmp"] = 3;
    total_size["memcpy"] = 3;
    total_size["memmove"] = 3;

    if (par == 1)
    {
        // reading from array
        // if it is zero terminated properly there won't be buffer overruns
        total_size["strncat"] = 3;
        total_size["strncpy"] = 3;
        total_size["memset"] = 3;
        total_size["fread"] = 1001;     // parameter 2 * parameter 3
        total_size["fwrite"] = 1001;    // parameter 2 * parameter 3
    }

    if (par == 2)
    {
        total_size["read"] = 3;
        total_size["write"] = 3;
    }

    std::map<std::string, unsigned int>::const_iterator it = total_size.find(tok.str());
    if (it != total_size.end())
    {
        unsigned int arg = it->second;
        for (const Token *tok2 = tok.tokAt(2); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "(")
            {
                tok2 = tok2->link();
                continue;
            }
            if (tok2->str() == ")")
                break;
            if (tok2->str() == ",")
            {
                --arg;
                if (arg == 1)
                {
                    if (Token::Match(tok2, ", %num% ,|)"))
                    {
                        const long sz = MathLib::toLongNumber(tok2->strAt(1));
                        unsigned int elements = 1;
                        for (unsigned int i = 0; i < arrayInfo.num.size(); ++i)
                            elements *= arrayInfo.num[i];
                        if (sz < 0 || sz > int(elements * arrayInfo.element_size))
                        {
                            bufferOverrun(&tok, arrayInfo.varname);
                        }
                    }

                    else if (Token::Match(tok2, ", %any% ,|)") && tok2->next()->str()[0] == '\'')
                    {
                        sizeArgumentAsChar(tok2->next());
                    }

                    break;
                }

                if (arg == 1000)  // special code. This parameter multiplied with the next must not exceed total_size
                {
                    if (Token::Match(tok2, ", %num% , %num% ,|)"))
                    {
                        const long sz = MathLib::toLongNumber(tok2->strAt(1)) * MathLib::toLongNumber(tok2->strAt(3));
                        unsigned int elements = 1;
                        for (unsigned int i = 0; i < arrayInfo.num.size(); ++i)
                            elements *= arrayInfo.num[i];
                        if (sz < 0 || sz > int(elements * arrayInfo.element_size))
                        {
                            bufferOverrun(&tok, arrayInfo.varname);
                        }
                    }
                    break;
                }
            }
        }
    }
}



void CheckBufferOverrun::checkScope(const Token *tok, const std::vector<std::string> &varname, const int size, const int total_size, unsigned int varid)
{
    std::string varnames;
    for (unsigned int i = 0; i < varname.size(); ++i)
        varnames += (i == 0 ? "" : " . ") + varname[i];

    const unsigned int varc(varname.empty() ? 0 : (varname.size() - 1) * 2);

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
            if (index >= size)
            {
                arrayIndexOutOfBounds(tok, size, index);
            }
        }
    }
    else if (Token::Match(tok, (varnames + " [ %num% ]").c_str()))
    {
        int index = MathLib::toLongNumber(tok->strAt(2 + varc));
        if (index >= size)
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
        else if (!tok->isName() && !Token::Match(tok, "[.&]") && Token::Match(tok->next(), (varnames + " [ %num% ]").c_str()))
        {
            int index = MathLib::toLongNumber(tok->strAt(3 + varc));
            if (index >= size)
            {
                arrayIndexOutOfBounds(tok->tokAt(1 + varc), size, index);
            }
            tok = tok->tokAt(4);
            continue;
        }


        // memset, memcmp, memcpy, strncpy, fgets..
        if (varid == 0)
        {
            ArrayInfo arrayInfo(0, varnames, 1, total_size);
            if (Token::Match(tok, ("%var% ( " + varnames + " ,").c_str()))
                checkFunctionCall(*tok, 1, arrayInfo);
            if (Token::Match(tok, ("%var% ( %var% , " + varnames + " ,").c_str()))
                checkFunctionCall(*tok, 2, arrayInfo);
        }

        // Loop..
        if (Token::simpleMatch(tok, "for ("))
        {
            const Token *tok2 = tok->tokAt(2);

            unsigned int counter_varid = 0;
            std::string min_counter_value;
            std::string max_counter_value;

            tok2 = for_init(tok2, counter_varid, min_counter_value);
            if (tok2 == 0 || counter_varid == 0)
                continue;

            bool maxMinFlipped = false;
            std::string strindex;
            if (!for_condition(tok2, counter_varid, min_counter_value, max_counter_value, strindex, maxMinFlipped))
                continue;

            // Get index variable and stopsize.
            bool condition_out_of_bounds = true;
            if (MathLib::toLongNumber(max_counter_value) < size)
                condition_out_of_bounds = false;

            if (!for3(tok2->tokAt(4), counter_varid, min_counter_value, max_counter_value, maxMinFlipped))
                continue;

            if (Token::Match(tok2->tokAt(4), "%var% =|+=|-=") && MathLib::toLongNumber(max_counter_value) <= size)
                condition_out_of_bounds = false;

            // Goto the end paranthesis of the for-statement: "for (x; y; z)" ..
            tok2 = tok->next()->link();
            if (!tok2 || !tok2->tokAt(5))
                break;

            // Check is the counter variable increased elsewhere inside the loop or used
            // for anything else except reading
            if (for_bailout(tok2->next(), counter_varid))
                break;

            ArrayInfo arrayInfo(varid, varnames, size, total_size);
            parse_for_body(tok2->next(), arrayInfo, strindex, condition_out_of_bounds, counter_varid, min_counter_value, max_counter_value);

            continue;
        }


        // Writing data into array..
        if ((varid > 0 && Token::Match(tok, "strcpy|strcat ( %varid% , %str% )", varid)) ||
            (varid == 0 && Token::Match(tok, ("strcpy|strcat ( " + varnames + " , %str% )").c_str())))
        {
            const long len = Token::getStrLength(tok->tokAt(varc + 4));
            if (len < 0 || len >= total_size)
            {
                bufferOverrun(tok, varid > 0 ? "" : varnames.c_str());
                continue;
            }
        }


        // Detect few strcat() calls
        const std::string strcatPattern = varid > 0 ? "strcat ( %varid% , %str% ) ;" : ("strcat ( " + varnames + " , %str% ) ;");
        if (Token::Match(tok, strcatPattern.c_str(), varid))
        {
            size_t charactersAppend = 0;
            const Token *tok2 = tok;

            while (tok2 && Token::Match(tok2, strcatPattern.c_str(), varid))
            {
                charactersAppend += Token::getStrLength(tok2->tokAt(4 + varc));
                if (charactersAppend >= static_cast<size_t>(total_size))
                {
                    bufferOverrun(tok2);
                    break;
                }
                tok2 = tok2->tokAt(7 + varc);
            }
        }

        // sprintf..
        const std::string sprintfPattern = varid > 0 ? "sprintf ( %varid% , %str% [,)]" : ("sprintf ( " + varnames + " , %str% [,)]");
        if (Token::Match(tok, sprintfPattern.c_str(), varid))
        {
            checkSprintfCall(tok, total_size);
        }

        // snprintf..
        const std::string snprintfPattern = varid > 0 ? "snprintf ( %varid% , %num% ," : ("snprintf ( " + varnames + " , %num% ,");
        if (Token::Match(tok, snprintfPattern.c_str(), varid))
        {
            int n = MathLib::toLongNumber(tok->strAt(4 + varc));
            if (n > total_size)
                outOfBounds(tok->tokAt(4 + varc), "snprintf size");
        }

        // Function calls not handled
        if (Token::Match(tok, "%var% ("))
        {
            continue;
        }
    }
}


void CheckBufferOverrun::checkScope(const Token *tok, const ArrayInfo &arrayInfo)
{
    const unsigned int total_size = arrayInfo.num[0] * arrayInfo.element_size;

    unsigned int indentlevel = 0;
    for (; tok; tok = tok->next())
    {
        if (tok->str() == "{")
        {
            ++indentlevel;
        }

        else if (tok->str() == "}")
        {
            if (indentlevel == 0)
                return;
            --indentlevel;
        }

        else if (Token::Match(tok, "%varid% [ %num% ]", arrayInfo.varid))
        {
            std::vector<int> indexes;
            for (const Token *tok2 = tok->next(); Token::Match(tok2, "[ %num% ]"); tok2 = tok2->tokAt(3))
            {
                const int index = MathLib::toLongNumber(tok2->strAt(1));
                if (index < 0)
                {
                    indexes.clear();
                    break;
                }
                indexes.push_back(index);
            }
            if (indexes.size() == arrayInfo.num.size())
            {
                // Check if the indexes point outside the whole array..
                // char a[10][10];
                // a[0][20]  <-- ok.
                // a[9][20]  <-- error.

                // total number of elements of array..
                unsigned int totalElements = 1;

                // total index..
                unsigned int totalIndex = 0;

                // calculate the totalElements and totalIndex..
                for (unsigned int i = 0; i < indexes.size(); ++i)
                {
                    unsigned int ri = indexes.size() - 1 - i;
                    totalIndex += indexes[ri] * totalElements;
                    totalElements *= arrayInfo.num[ri];
                }

                // just taking the address?
                const bool addr(Token::Match(tok->previous(), "[.&]") ||
                                Token::simpleMatch(tok->tokAt(-2), "& ("));

                // Is totalIndex in bounds?
                if (totalIndex > totalElements || (!addr && totalIndex == totalElements))
                {
                    arrayIndexOutOfBounds(tok, arrayInfo, indexes);
                }
            }

        }

        // Loop..
        else if (Token::simpleMatch(tok, "for ("))
        {
            const Token *tok2 = tok->tokAt(2);

            unsigned int counter_varid = 0;
            std::string min_counter_value;
            std::string max_counter_value;

            tok2 = for_init(tok2, counter_varid, min_counter_value);
            if (tok2 == 0 || counter_varid == 0)
                continue;

            bool maxMinFlipped = false;
            std::string strindex;
            if (!for_condition(tok2, counter_varid, min_counter_value, max_counter_value, strindex, maxMinFlipped))
                continue;

            // Get index variable and stopsize.
            bool condition_out_of_bounds = true;
            if (MathLib::toLongNumber(max_counter_value) < (int)arrayInfo.num[0])
                condition_out_of_bounds = false;

            if (!for3(tok2->tokAt(4), counter_varid, min_counter_value, max_counter_value, maxMinFlipped))
                continue;

            if (Token::Match(tok2->tokAt(4), "%var% =|+=|-=") && MathLib::toLongNumber(max_counter_value) <= (int)arrayInfo.num[0])
                condition_out_of_bounds = false;

            // Goto the end paranthesis of the for-statement: "for (x; y; z)" ..
            tok2 = tok->next()->link();
            if (!tok2 || !tok2->tokAt(5))
                break;

            // Check is the counter variable increased elsewhere inside the loop or used
            // for anything else except reading
            if (for_bailout(tok2->next(), counter_varid))
                break;

            parse_for_body(tok2->next(), arrayInfo, strindex, condition_out_of_bounds, counter_varid, min_counter_value, max_counter_value);

            continue;
        }


        // Check function call..
        if (Token::Match(tok, "%var% ("))
        {
            // 1st parameter..
            if (Token::Match(tok->tokAt(2), "%varid% ,", arrayInfo.varid))
                checkFunctionCall(*tok, 1, arrayInfo);
            else if (Token::Match(tok->tokAt(2), "%varid% + %num% ,", arrayInfo.varid))
            {
                const ArrayInfo ai(arrayInfo.limit(MathLib::toLongNumber(tok->strAt(4))));
                checkFunctionCall(*tok, 1, ai);
            }

            // goto 2nd parameter and check it..
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                {
                    tok2 = tok2->link();
                    continue;
                }
                if (tok2->str() == ";" || tok2->str() == ")")
                    break;
                if (tok2->str() == ",")
                {
                    if (Token::Match(tok2, ", %varid% ,", arrayInfo.varid))
                        checkFunctionCall(*tok, 2, arrayInfo);
                    else if (Token::Match(tok2, ", %varid% + %num% ,", arrayInfo.varid))
                    {
                        const ArrayInfo ai(arrayInfo.limit(MathLib::toLongNumber(tok2->strAt(3))));
                        checkFunctionCall(*tok, 2, ai);
                    }
                    break;
                }
            }
        }

        if (_settings->_checkCodingStyle)
        {
            // check for strncpy which is not terminated
            if ((Token::Match(tok, "strncpy ( %varid% , %var% , %num% )", arrayInfo.varid)))
            {
                // strncpy takes entire variable length as input size
                if ((unsigned int)MathLib::toLongNumber(tok->strAt(6)) >= total_size)
                {
                    const Token *tok2 = tok->next()->link()->next();
                    for (; tok2; tok2 = tok2->next())
                    {
                        if (tok2->varId() == tok->tokAt(2)->varId())
                        {
                            if (!Token::Match(tok2, "%varid% [ %any% ]  = 0 ;", tok->tokAt(2)->varId()))
                            {
                                terminateStrncpyError(tok);
                            }

                            break;
                        }
                    }
                }
            }
        }

        // Dangerous usage of strncat..
        if (Token::Match(tok, "strncpy|strncat ( %varid% , %any% , %num% )", arrayInfo.varid))
        {
            if (tok->str() == "strncat")
            {
                const unsigned int n = MathLib::toLongNumber(tok->strAt(6));
                if (n >= total_size)
                    strncatUsage(tok);
            }

            // Dangerous usage of strncpy + strncat..
            if (Token::Match(tok->tokAt(8), "; strncat ( %varid% , %any% , %num% )", arrayInfo.varid))
            {
                const unsigned int n = MathLib::toLongNumber(tok->strAt(6)) + MathLib::toLongNumber(tok->strAt(15));
                if (n > total_size)
                    strncatUsage(tok->tokAt(9));
            }
        }

        // Writing data into array..
        if (Token::Match(tok, "strcpy|strcat ( %varid% , %str% )", arrayInfo.varid))
        {
            const unsigned long len = Token::getStrLength(tok->tokAt(4));
            if (len >= total_size)
            {
                bufferOverrun(tok, arrayInfo.varname);
                continue;
            }
        }

        // Detect few strcat() calls
        if (Token::Match(tok, "strcat ( %varid% , %str% ) ;", arrayInfo.varid))
        {
            size_t charactersAppend = 0;
            const Token *tok2 = tok;

            while (tok2 && Token::Match(tok2, "strcat ( %varid% , %str% ) ;", arrayInfo.varid))
            {
                charactersAppend += Token::getStrLength(tok2->tokAt(4));
                if (charactersAppend >= static_cast<size_t>(total_size))
                {
                    bufferOverrun(tok2, arrayInfo.varname);
                    break;
                }
                tok2 = tok2->tokAt(7);
            }
        }


        if (Token::Match(tok, "sprintf ( %varid% , %str% [,)]", arrayInfo.varid))
        {
            checkSprintfCall(tok, total_size);
        }

        // snprintf..
        if (Token::Match(tok, "snprintf ( %varid% , %num% ,", arrayInfo.varid))
        {
            const unsigned int n = MathLib::toLongNumber(tok->strAt(4));
            if (n > total_size)
                outOfBounds(tok->tokAt(4), "snprintf size");
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

        int size = 0;
        std::string type;
        unsigned int varid = 0;
        int nextTok = 0;

        // if the previous token exists, it must be either a variable name or "[;{}]"
        if (tok->previous() && (!tok->previous()->isName() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        ArrayInfo arrayInfo;
        if (arrayInfo.declare(tok, *_tokenizer))
        {
            while (tok && tok->str() != ";")
                tok = tok->next();
            if (!tok)
                break;
            checkScope(tok, arrayInfo);
            continue;
        }

        if (Token::Match(tok, "%type% *| %var% [ %var% ] [;=]"))
        {
            unsigned int varpos = 1;
            if (tok->next()->str() == "*")
                ++varpos;

            // make sure the variable is defined
            if (tok->tokAt(varpos + 2)->varId() == 0)
                continue; // FIXME we loose the check for negative index when we bail

            // get maximum size from type
            // find where this token is defined
            const Token *index_type = Token::findmatch(_tokenizer->tokens(), "%varid%", tok->tokAt(varpos + 2)->varId());

            index_type = index_type->previous();

            if (index_type->str() == "char")
            {
                if (index_type->isUnsigned())
                    size = UCHAR_MAX + 1;
                else if (index_type->isSigned())
                    size = SCHAR_MAX + 1;
                else
                    size = CHAR_MAX + 1;
            }
            else if (index_type->str() == "short")
            {
                if (index_type->isUnsigned())
                    size = USHRT_MAX + 1;
                else
                    size = SHRT_MAX + 1;
            }

            // checkScope assumes size is signed int so we limit the following sizes to INT_MAX
            else if (index_type->str() == "int")
            {
                if (index_type->isUnsigned())
                    size = INT_MAX; // should be UINT_MAX + 1U;
                else
                    size = INT_MAX; // should be INT_MAX + 1U;
            }
            else if (index_type->str() == "long")
            {
                if (index_type->isUnsigned())
                {
                    if (index_type->isLong())
                        size = INT_MAX; // should be ULLONG_MAX + 1ULL;
                    else
                        size = INT_MAX; // should be ULONG_MAX + 1UL;
                }
                else
                {
                    if (index_type->isLong())
                        size = INT_MAX; // should be LLONG_MAX + 1LL;
                    else
                        size = INT_MAX; // should be LONG_MAX + 1L;
                }
            }

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
            type = "char";   // minimum type, typesize=1
            varid = tok->tokAt(1)->varId();
            nextTok = 7;

            if (varid > 0)
            {
                // get type of variable
                const Token *declTok = Token::findmatch(_tokenizer->tokens(), "[;{}] %type% * %varid% ;", varid);
                if (!declTok)
                    continue;

                type = declTok->next()->str();

                // malloc() gets count of bytes and not count of
                // elements, so we should calculate count of elements
                // manually
                unsigned int sizeOfType = _tokenizer->sizeOfType(declTok->next());
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
    const char declstruct[] = "struct|class %var% {|:";
    for (const Token *tok = Token::findmatch(_tokenizer->tokens(), declstruct);
         tok; tok = Token::findmatch(tok->next(), declstruct))
    {
        const std::string &structname = tok->next()->str();
        const Token *tok2 = tok;

        while (tok2->str() != "{")
            tok2 = tok2->next();

        // Found a struct declaration. Search for arrays..
        for (; tok2; tok2 = tok2->next())
        {
            // skip inner scopes..
            if (tok2->next() && tok2->next()->str() == "{")
            {
                tok2 = tok2->next()->link();
                continue;
            }

            if (tok2->str() == "}")
                break;

            ArrayInfo arrayInfo;
            if (!arrayInfo.declare(tok2->next(), *_tokenizer))
                continue;

            // Only handling 1-dimensional arrays yet..
            if (arrayInfo.num.size() > 1)
                continue;

            std::vector<std::string> varname;
            varname.push_back("");
            varname.push_back(arrayInfo.varname);

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
                            checkScope(tok4->tokAt(2), v, arrayInfo.num[0], arrayInfo.num[0] * arrayInfo.element_size, arrayInfo.varid);
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
                checkScope(CheckTok, varname, arrayInfo.num[0], arrayInfo.num[0] * arrayInfo.element_size, 0);
            }
        }
    }
}
//---------------------------------------------------------------------------

void CheckBufferOverrun::bufferOverrun()
{
    checkGlobalAndLocalVariable();
    checkStructVariable();
    checkBufferAllocatedWithStrlen();
    checkInsecureCmdLineArgs();
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
    const Token *end = tok->next()->link();

    // Count the number of tokens in the buffer variable's name
    int varc = 0;
    for (const Token *tok1 = tok->tokAt(3); tok1 != end; tok1 = tok1->next())
    {
        if (tok1->str() == ",")
            break;
        ++ varc;
    }

    std::list<const Token*> parameters;
    if (tok->tokAt(5 + varc)->str() == ",")
    {
        for (const Token *tok2 = tok->tokAt(5 + varc); tok2 && tok2 != end; tok2 = tok2->next())
        {
            if (Token::Match(tok2, ", %any% [,)]"))
            {

                if (Token::Match(tok2->next(), "%str%"))
                    parameters.push_back(tok2->next());

                else if (Token::Match(tok2->next(), "%num%"))
                    parameters.push_back(tok2->next());

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

    int len = countSprintfLength(tok->tokAt(4 + varc)->strValue(), parameters);
    if (len > size)
    {
        bufferOverrun(tok);
    }
}



//---------------------------------------------------------------------------
// Checking for allocating insufficient memory for copying a string by
// allocating only strlen(src) bytes instead of strlen(src) + 1 bytes (one
// extra for the terminating null character).
// Example:
//   char *b = malloc(strlen(a));   // Should be malloc(strlen(a) + 1);
//   strcpy(b, a);                  // <== Buffer overrun
//---------------------------------------------------------------------------
void CheckBufferOverrun::checkBufferAllocatedWithStrlen()
{
    const char pattern[] = "%var% = new|malloc|g_malloc|g_try_malloc|realloc|g_realloc|g_try_realloc";
    for (const Token *tok = Token::findmatch(_tokenizer->tokens(), pattern); tok; tok = Token::findmatch(tok->next(),pattern))
    {
        unsigned int dstVarId;
        unsigned int srcVarId;

        // Look for allocation of a buffer based on the size of a string
        if (Token::Match(tok, "%var% = malloc|g_malloc|g_try_malloc ( strlen ( %var% ) )"))
        {
            dstVarId = tok->varId();
            srcVarId = tok->tokAt(6)->varId();
            tok      = tok->tokAt(8);
        }
        else if (Token::Match(tok, "%var% = new char [ strlen ( %var% ) ]"))
        {
            dstVarId = tok->varId();
            srcVarId = tok->tokAt(7)->varId();
            tok      = tok->tokAt(9);
        }
        else if (Token::Match(tok, "%var% = realloc|g_realloc|g_try_realloc ( %var% , strlen ( %var% ) )"))
        {
            dstVarId = tok->varId();
            srcVarId = tok->tokAt(8)->varId();
            tok      = tok->tokAt(10);
        }
        else
            continue;

        int indentlevel = 0;
        for (; tok && tok->next(); tok = tok->next())
        {
            // To avoid false positives and added complexity, we will only look for
            // improper usage of the buffer within the block that it was allocated
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

            // If the buffers are modified, we can't be sure of their sizes
            if (tok->varId() == srcVarId || tok->varId() == dstVarId)
                break;

            if (Token::Match(tok, "strcpy ( %varid% , %var% )", dstVarId) &&
                tok->tokAt(4)->varId() == srcVarId)
            {
                bufferOverrun(tok);
            }
            else if (Token::Match(tok, "sprintf ( %varid% , %str% , %var% )", dstVarId) &&
                     tok->tokAt(6)->varId() == srcVarId &&
                     tok->tokAt(4)->str().find("%s") != std::string::npos)
            {
                bufferOverrun(tok);
            }

        }

    }
}

//---------------------------------------------------------------------------
// Checking for buffer overflow caused by copying command line arguments
// into fixed-sized buffers without checking to make sure that the command
// line arguments will not overflow the buffer.
//
// int main(int argc, char* argv[])
// {
//   char prog[10];
//   strcpy(prog, argv[0]);      <-- Possible buffer overrun
// }
//---------------------------------------------------------------------------
void CheckBufferOverrun::checkInsecureCmdLineArgs()
{
    const char pattern[] = "main ( int %var% , char *";
    for (const Token *tok = Token::findmatch(_tokenizer->tokens(), pattern); tok; tok = Token::findmatch(tok->next(),pattern))
    {
        // Get the name of the argv variable
        unsigned int varid = 0;
        if (Token::Match(tok, "main ( int %var% , char * %var% [ ] ,|)"))
        {
            varid = tok->tokAt(7)->varId();

        }
        else if (Token::Match(tok, "main ( int %var% , char * * %var% ,|)"))
        {
            varid = tok->tokAt(8)->varId();
        }
        if (varid == 0)
            continue;

        // Jump to the opening curly brace
        tok = tok->next()->link();
        if (!Token::simpleMatch(tok, ") {"))
            continue;
        tok = tok->next();

        // Search within main() for possible buffer overruns involving argv
        int indentlevel = -1;
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

            // If argv is modified or tested, its size may be being limited properly
            if (tok->varId() == varid)
                break;

            // Match common patterns that can result in a buffer overrun
            // e.g. strcpy(buffer, argv[0])
            if (Token::Match(tok, "strcpy|strcat ( %var% , * %varid%", varid) ||
                Token::Match(tok, "strcpy|strcat ( %var% , %varid% [", varid))
            {
                cmdLineArgsError(tok);
            }
            else if (Token::Match(tok, "sprintf ( %var% , %str% , %varid% [", varid) &&
                     tok->tokAt(4)->str().find("%s") != std::string::npos)
            {
                cmdLineArgsError(tok);
            }
            else if (Token::Match(tok, "sprintf ( %var% , %str% , * %varid%", varid) &&
                     tok->tokAt(4)->str().find("%s") != std::string::npos)
            {
                cmdLineArgsError(tok);
            }

        }

    }
}
//---------------------------------------------------------------------------


void CheckBufferOverrun::negativeIndexError(const Token *tok, long index)
{
    std::ostringstream ostr;
    ostr << "Array index " << index << " is out of bounds";
    reportError(tok, Severity::error, "negativeIndex", ostr.str());
}

void CheckBufferOverrun::negativeIndex()
{
    const char pattern[] = "[ %num% ]";
    for (const Token *tok = Token::findmatch(_tokenizer->tokens(), pattern); tok; tok = Token::findmatch(tok->next(),pattern))
    {
        const long index = MathLib::toLongNumber(tok->next()->str());
        if (index < 0)
        {
            // Multidimensional index => error
            if (Token::simpleMatch(tok->previous(), "]") || Token::simpleMatch(tok->tokAt(3), "["))
                negativeIndexError(tok, index);

            // 1-dimensional array => error
            else if (tok->previous() && tok->previous()->varId())
            {
                const Token *tok2 = Token::findmatch(_tokenizer->tokens(), "%varid%", tok->previous()->varId());
                if (tok2 && Token::Match(tok2->next(), "[ %any% ] ;"))
                    negativeIndexError(tok, index);
            }
        }
    }
}




#include "executionpath.h"

/// @addtogroup Checks
/// @{



CheckBufferOverrun::ArrayInfo::ArrayInfo()
    :	num(_num), element_size(_element_size), varid(_varid), varname(_varname)
{
    _element_size = 0;
    _varid = 0;
}

CheckBufferOverrun::ArrayInfo::ArrayInfo(const CheckBufferOverrun::ArrayInfo &ai)
    :	num(_num), element_size(_element_size), varid(_varid), varname(_varname)
{
    *this = ai;
}

const CheckBufferOverrun::ArrayInfo & CheckBufferOverrun::ArrayInfo::operator=(const CheckBufferOverrun::ArrayInfo &ai)
{
    if (&ai != this)
    {
        _element_size = ai.element_size;
        _num = ai.num;
        _varid = ai.varid;
        _varname = ai.varname;
    }
    return *this;
}

/**
 * Create array info with specified data
 * The intention is that this is only a temporary solution.. all
 * checking should be based on ArrayInfo from the start and then
 * this will not be needed as the declare can be used instead.
 */
CheckBufferOverrun::ArrayInfo::ArrayInfo(unsigned int id, const std::string &name, unsigned int size1, unsigned int n)
    :	num(_num), element_size(_element_size), varid(_varid), varname(_varname)
{
    _element_size = size1;
    _num.push_back(n);
    _varid = id;
    _varname = name;
}

CheckBufferOverrun::ArrayInfo CheckBufferOverrun::ArrayInfo::limit(long value) const
{
    unsigned int n = 1;
    for (unsigned int i = 0; i < num.size(); ++i)
        n *= num[i];
    return ArrayInfo(varid, varname, element_size, value > (int)n ? 0 : n - value);
}

bool CheckBufferOverrun::ArrayInfo::declare(const Token *tok, const Tokenizer &tokenizer)
{
    _num.clear();
    _element_size = 0;
    _varname.clear();

    if (!tok->isName())
        return false;

    int ivar = 0;
    if (Token::Match(tok, "%type% *| %var% ["))
        ivar = 1;
    else if (Token::Match(tok, "%type% %type% *| %var% ["))
        ivar = 2;
    else
        return false;

    // Goto variable name token, get element size..
    const Token *vartok = tok->tokAt(ivar);
    if (vartok->str() == "*")
    {
        _element_size = tokenizer.sizeOfType(vartok);
        vartok = vartok->next();
    }
    else
    {
        _element_size = tokenizer.sizeOfType(tok);
    }
    if (_element_size == 0)
        return false;

    _varname = vartok->str();
    _varid = vartok->varId();
    if (!varid)
        return false;

    const Token *atok = vartok->tokAt(2);

    if (!Token::Match(atok, "%num% ] ;|=|["))
        return false;

    while (Token::Match(atok, "%num% ] ;|=|["))
    {
        _num.push_back(MathLib::toLongNumber(atok->str()));
        atok = atok->next();
        if (Token::simpleMatch(atok, "] ["))
            atok = atok->tokAt(2);
    }

    return (!_num.empty() && Token::Match(atok, "] ;|="));
}



/**
 * @brief %Check for buffer overruns (using ExecutionPath)
 */

class ExecutionPathBufferOverrun : public ExecutionPath
{
public:
    /** Startup constructor */
    ExecutionPathBufferOverrun(Check *c, const std::map<unsigned int, CheckBufferOverrun::ArrayInfo> &arrayinfo)
        : ExecutionPath(c, 0), arrayInfo(arrayinfo)
    {
    }

private:
    /** Copy this check */
    ExecutionPath *copy()
    {
        return new ExecutionPathBufferOverrun(*this);
    }

    /** is other execution path equal? */
    bool is_equal(const ExecutionPath *e) const
    {
        const ExecutionPathBufferOverrun *c = static_cast<const ExecutionPathBufferOverrun *>(e);
        return (value == c->value);
    }

    /** @brief buffer information */
    const std::map<unsigned int, CheckBufferOverrun::ArrayInfo> &arrayInfo;

    /** no implementation => compiler error if used by accident */
    void operator=(const ExecutionPathBufferOverrun &);

    /** internal constructor for creating extra checks */
    ExecutionPathBufferOverrun(Check *c, const std::map<unsigned int, CheckBufferOverrun::ArrayInfo> &arrayinfo, unsigned int varid_)
        : ExecutionPath(c, varid_),
          arrayInfo(arrayinfo)
    {
        // Pretend that variables are initialized to 0
        // This checking is not about uninitialized variables
        value = 0;
    }

    unsigned int value;

    /** @brief Assign value to a variable */
    static void assign_value(std::list<ExecutionPath *> &checks, unsigned int varid, const std::string &value)
    {
        if (varid == 0)
            return;

        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            ExecutionPathBufferOverrun *c = dynamic_cast<ExecutionPathBufferOverrun *>(*it);
            if (c && c->varId == varid)
                c->value = MathLib::toLongNumber(value);
        }
    }

    /** @brief Found array usage.. */
    static void array_index(const Token *tok, std::list<ExecutionPath *> &checks, unsigned int varid1, unsigned int varid2)
    {
        if (checks.empty() || varid1 == 0 || varid2 == 0)
            return;

        // Locate array info corresponding to varid1
        CheckBufferOverrun::ArrayInfo ai;
        {
            ExecutionPathBufferOverrun *c = dynamic_cast<ExecutionPathBufferOverrun *>(checks.front());
            std::map<unsigned int, CheckBufferOverrun::ArrayInfo>::const_iterator it;
            it = c->arrayInfo.find(varid1);
            if (it == c->arrayInfo.end())
                return;
            ai = it->second;
        }

        // Check if varid2 variable has a value that is out-of-bounds
        std::list<ExecutionPath *>::const_iterator it;
        for (it = checks.begin(); it != checks.end(); ++it)
        {
            ExecutionPathBufferOverrun *c = dynamic_cast<ExecutionPathBufferOverrun *>(*it);
            if (c && c->varId == varid2 && c->value >= ai.num[0])
            {
                // variable value is out of bounds, report error
                CheckBufferOverrun *checkBufferOverrun = dynamic_cast<CheckBufferOverrun *>(c->owner);
                if (checkBufferOverrun)
                {
                    std::vector<int> index;
                    index.push_back(c->value);
                    checkBufferOverrun->arrayIndexOutOfBounds(tok, ai, index);
                    break;
                }
            }
        }
    }

    const Token *parse(const Token &tok, std::list<ExecutionPath *> &checks) const
    {
        if (Token::Match(tok.previous(), "[;{}]"))
        {
            // Declaring variable..
            if (Token::Match(&tok, "%type% %var% ;") && tok.isStandardType())
            {
                checks.push_back(new ExecutionPathBufferOverrun(owner, arrayInfo, tok.next()->varId()));
                return tok.tokAt(2);
            }

            // Assign variable..
            if (Token::Match(&tok, "%var% = %num% ;"))
            {
                assign_value(checks, tok.varId(), tok.strAt(2));
                return tok.tokAt(3);
            }
        }

        // Assign variable (unknown value = 0)..
        if (Token::Match(&tok, "%var% ="))
        {
            assign_value(checks, tok.varId(), "0");
            return &tok;
        }

        // Array index..
        if (Token::Match(&tok, "%var% [ %var% ]"))
        {
            array_index(&tok, checks, tok.varId(), tok.tokAt(2)->varId());
            return tok.tokAt(3);
        }

        return &tok;
    }
};

/// @}


void CheckBufferOverrun::executionPaths()
{
    // Parse all tokens and extract array info..
    std::map<unsigned int, ArrayInfo> arrayInfo;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;{}] %type%"))
        {
            ArrayInfo ai;
            if (!ai.declare(tok->next(), *_tokenizer))
                continue;
            arrayInfo[ai.varid] = ai;
        }
    }

    // Perform checking - check how the arrayInfo arrays are used
    ExecutionPathBufferOverrun c(this, arrayInfo);
    checkExecutionPaths(_tokenizer->tokens(), &c);
}




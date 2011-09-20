/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include "symboldatabase.h"

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

void CheckBufferOverrun::arrayIndexOutOfBoundsError(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index)
{
    std::ostringstream oss;
    oss << "Array '" << arrayInfo.varname();
    for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
        oss << "[" << arrayInfo.num(i) << "]";
    oss << "' index ";
    if (index.size() == 1)
        oss << index[0];
    else
    {
        oss << arrayInfo.varname();
        for (unsigned int i = 0; i < index.size(); ++i)
            oss << "[" << index[i] << "]";
    }
    oss << " out of bounds";
    reportError(tok, Severity::error, "arrayIndexOutOfBounds", oss.str().c_str());
}

void CheckBufferOverrun::arrayIndexOutOfBoundsError(const std::list<const Token *> &callstack, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index)
{
    std::ostringstream oss;
    oss << "Array '" << arrayInfo.varname();
    for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
        oss << "[" << arrayInfo.num(i) << "]";
    oss << "' index ";
    if (index.size() == 1)
        oss << index[0];
    else
    {
        oss << arrayInfo.varname();
        for (unsigned int i = 0; i < index.size(); ++i)
            oss << "[" << index[i] << "]";
    }
    oss << " out of bounds";
    reportError(callstack, Severity::error, "arrayIndexOutOfBounds", oss.str().c_str());
}

void CheckBufferOverrun::bufferOverrunError(const Token *tok, const std::string &varnames)
{
    std::string v = varnames;
    while (v.find(" ") != std::string::npos)
        v.erase(v.find(" "), 1);

    std::string errmsg("Buffer access out-of-bounds");
    if (!v.empty())
        errmsg += ": " + v;

    reportError(tok, Severity::error, "bufferAccessOutOfBounds",  errmsg);
}

void CheckBufferOverrun::possibleBufferOverrunError(const Token *tok, const std::string &src, const std::string &dst, bool cat)
{
    if (cat)
        reportError(tok, Severity::warning, "possibleBufferAccessOutOfBounds",
                    "Possible buffer overflow if strlen(" + src + ") is larger than sizeof(" + dst + ")-strlen(" + dst +").\n"
                    "The source buffer is larger than the destination buffer so there is the potential for overflowing the destination buffer.");
    else
        reportError(tok, Severity::warning, "possibleBufferAccessOutOfBounds",
                    "Possible buffer overflow if strlen(" + src + ") is larger than or equal to sizeof(" + dst + ").\n"
                    "The source buffer is larger than the destination buffer so there is the potential for overflowing the destination buffer.");
}

void CheckBufferOverrun::strncatUsageError(const Token *tok)
{
    if (_settings && !_settings->isEnabled("style"))
        return;

    reportError(tok, Severity::warning, "strncatUsage",
                "Dangerous usage of strncat - 3rd parameter is the maximum number of characters to append.\n"
                "strncat appends at max its 3rd parameter's amount of characters. The safe way to use "
                "strncat is to calculate remaining space in the buffer and use it as 3rd parameter.");
}

void CheckBufferOverrun::outOfBoundsError(const Token *tok, const std::string &what)
{
    reportError(tok, Severity::error, "outOfBounds", what + " is out of bounds");
}

void CheckBufferOverrun::pointerOutOfBoundsError(const Token *tok, const std::string &object)
{
    reportError(tok, Severity::portability, "pointerOutOfBounds", "Undefined behaviour: pointer arithmetic result does not point into or just past the end of the " + object + "\n"
                "Undefined behaviour: Using pointer arithmetic so that the result does not point into or just past the end of the same object. Further information: https://www.securecoding.cert.org/confluence/display/seccode/ARR30-C.+Do+not+form+or+use+out+of+bounds+pointers+or+array+subscripts");
}

void CheckBufferOverrun::sizeArgumentAsCharError(const Token *tok)
{
    if (_settings && !_settings->isEnabled("style"))
        return;
    reportError(tok, Severity::warning, "sizeArgumentAsChar", "The size argument is given as a char constant");
}


void CheckBufferOverrun::terminateStrncpyError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "terminateStrncpy",
                "The buffer '" + varname + "' may not be zero-terminated after the call to strncpy().\n"
                "The use of strncpy() usually indicates that the programmer wants to ensure "
                "the buffer is zero-terminated after the call. However if the (buffer) size given for "
                "the strncpy() call matches the actual buffer size strncpy() does not add the "
                "zero at the end of the buffer. This may cause bugs later in the code if "
                "the code assumes buffer is zero-terminated.");
}

void CheckBufferOverrun::cmdLineArgsError(const Token *tok)
{
    reportError(tok, Severity::error, "insecureCmdLineArgs", "Buffer overrun possible for long cmd-line args");
}

void CheckBufferOverrun::bufferNotZeroTerminatedError(const Token *tok, const std::string &varname, const std::string &function)
{
    reportError(tok, Severity::warning, "bufferNotZeroTerminated",
                "The buffer '" + varname + "' is not zero-terminated after the call to " + function + "().\n"
                "This will cause bugs later in the code if the code assumes buffer is zero-terminated.");
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
 * bailout if variable is used inside if/else/switch block or if there is "break"
 * @param tok token for "if" or "switch"
 * @param varid variable id
 * @return is bailout recommended?
 */
static bool bailoutIfSwitch(const Token *tok, const unsigned int varid)
{
    // Used later to check if the body belongs to a "if"
    const std::string str1(tok->str());

    // Count { and }
    unsigned int indentlevel = 0;
    for (; tok; tok = tok->next())
    {
        if (tok->str() == "{")
            indentlevel++;
        else if (tok->str() == "}")
        {
            // scan the else-block
            if (indentlevel == 1 && Token::simpleMatch(tok, "} else {"))
            {
                --indentlevel;
                continue;
            }
            else if (indentlevel <= 1)
            {
                break;
            }

            --indentlevel;
        }

        // If scanning a "if" block then bailout for "break"
        else if (str1 == "if" && tok->str() == "break")
            return true;

        // bailout for "return"
        else if (tok->str() == "return")
            return true;

        // bailout if varid is found
        else if (tok->varId() == varid)
            return true;
    }

    // No bailout stuff found => return false
    return false;
}

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
    if (Token::Match(tok2, "%varid% < %num% ;", varid) ||
        Token::Match(tok2, "%varid% != %num% ; ++ %varid%", varid) ||
        Token::Match(tok2, "%varid% != %num% ; %varid% ++", varid))
    {
        maxMinFlipped = false;
        const MathLib::bigint value = MathLib::toLongNumber(tok2->strAt(2));
        max_value = MathLib::toString<MathLib::bigint>(value - 1);
    }
    else if (Token::Match(tok2, "%varid% <= %num% ;", varid))
    {
        maxMinFlipped = false;
        max_value = tok2->strAt(2);
    }
    else if (Token::Match(tok2, "%num% < %varid% ;", varid) ||
             Token::Match(tok2, "%num% != %varid% ; ++ %varid%", varid) ||
             Token::Match(tok2, "%num% != %varid% ; %varid% ++", varid))
    {
        maxMinFlipped = true;
        const MathLib::bigint value = MathLib::toLongNumber(tok2->str());
        max_value = min_value;
        min_value = MathLib::toString<MathLib::bigint>(value + 1);
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
 * calculate maximum value of loop variable
 * @param stepvalue token that contains the step value
 * @param min_value the minimum value of loop variable
 * @param max_value maximum value of the loop variable
 */
static bool for_maxvalue(const Token * const stepvalue, const std::string &min_value, std::string &max_value)
{
    if (!MathLib::isInt(stepvalue->str()))
        return false;

    // We have for example code: "for(i=2;i<22;i+=6)
    // We can calculate that max value for i is 20, not 21
    // 21-2 = 19
    // 19/6 = 3
    // 6*3+2 = 20
    const MathLib::bigint num = MathLib::toLongNumber(stepvalue->str());
    MathLib::bigint max = MathLib::toLongNumber(max_value);
    const MathLib::bigint min = MathLib::toLongNumber(min_value);
    max = ((max - min) / num) * num + min;
    max_value = MathLib::toString<MathLib::bigint>(max);
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
        if (!for_maxvalue(tok->tokAt(2), min_value, max_value))
            return false;
    }
    else if (Token::Match(tok, "%varid% = %varid% + %num% )", varid))
    {
        if (!for_maxvalue(tok->tokAt(4), min_value, max_value))
            return false;
    }
    else if (Token::Match(tok, "%varid% -= %num% )", varid) ||
             Token::Match(tok, "%varid%  = %num% - %varid% )", varid))
    {
        if (!for_maxvalue(tok->tokAt(2), min_value, max_value))
            return false;
    }
    else if (Token::Match(tok, "%varid% = %varid% - %num% )", varid))
    {
        if (!for_maxvalue(tok->tokAt(4), min_value, max_value))
            return false;
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
 * \param tok1 first token of for-body
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
    const std::string pattern((arrayInfo.varid() ? std::string("%varid%") : arrayInfo.varname()) + " [ " + strindex + " ]");

    // count { and } for tok2
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

        // TODO: try to reduce false negatives. This is just a quick fix
        // for TestBufferOverrun::array_index_for_question
        if (tok2->str() == "?")
            break;

        if (Token::simpleMatch(tok2, "for (") && Token::simpleMatch(tok2->next()->link(), ") {"))
        {
            const Token *endpar = tok2->next()->link();
            const Token *startbody = endpar->next();
            const Token *endbody = startbody->link();
            tok2 = endbody;
            continue;
        }

        if (Token::Match(tok2, "if|switch"))
        {
            if (bailoutIfSwitch(tok2, arrayInfo.varid()))
                break;
        }

        if (condition_out_of_bounds && Token::Match(tok2, pattern.c_str(), arrayInfo.varid()))
        {
            bufferOverrunError(tok2, arrayInfo.varname());
            break;
        }

        else if (arrayInfo.varid() && counter_varid > 0 && !min_counter_value.empty() && !max_counter_value.empty())
        {
            // Is the loop variable used to calculate the array index?
            // In this scope it is determined if such calculated
            // array indexes are out of bounds.
            // Only the minimum and maximum results of the calculation is
            // determined

            // Minimum calculated array index
            int min_index = 0;

            // Maximum calculated array index
            int max_index = 0;

            if (Token::Match(tok2, "%varid% [ %var% +|-|*|/ %num% ]", arrayInfo.varid()) &&
                tok2->tokAt(2)->varId() == counter_varid)
            {
                // operator: +-*/
                const char action = tok2->strAt(3)[0];

                // second operator
                const std::string &second(tok2->tokAt(4)->str());

                //printf("min_index: %s %c %s\n", min_counter_value.c_str(), action, second.c_str());
                //printf("max_index: %s %c %s\n", max_counter_value.c_str(), action, second.c_str());

                min_index = std::atoi(MathLib::calculate(min_counter_value, second, action, _tokenizer).c_str());
                max_index = std::atoi(MathLib::calculate(max_counter_value, second, action, _tokenizer).c_str());
            }
            else if (Token::Match(tok2, "%varid% [ %num% +|-|*|/ %var% ]", arrayInfo.varid()) &&
                     tok2->tokAt(4)->varId() == counter_varid)
            {
                // operator: +-*/
                const char action = tok2->strAt(3)[0];

                // first operand
                const std::string &first(tok2->tokAt(2)->str());

                //printf("min_index: %s %c %s\n", first.c_str(), action, min_counter_value.c_str());
                //printf("max_index: %s %c %s\n", first.c_str(), action, max_counter_value.c_str());

                min_index = std::atoi(MathLib::calculate(first, min_counter_value, action, _tokenizer).c_str());
                max_index = std::atoi(MathLib::calculate(first, max_counter_value, action, _tokenizer).c_str());
            }

            //printf("min_index = %d, max_index = %d, size = %d\n", min_index, max_index, size);
            if (min_index < 0 || max_index < 0)
            {
                std::vector<MathLib::bigint> indexes;
                indexes.push_back(std::min(min_index, max_index));
                arrayIndexOutOfBoundsError(tok2, arrayInfo, indexes);
            }
            if (min_index >= (int)arrayInfo.num(0) || max_index >= (int)arrayInfo.num(0))
            {
                std::vector<MathLib::bigint> indexes;
                indexes.push_back(std::max(min_index, max_index));
                arrayIndexOutOfBoundsError(tok2, arrayInfo, indexes);
            }
        }
    }
}



void CheckBufferOverrun::checkFunctionParameter(const Token &tok, unsigned int par, const ArrayInfo &arrayInfo)
{
    // total_size : which parameter in function call takes the total size?
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
        total_size["pread"] = 3;
        total_size["write"] = 3;
        total_size["recv"] = 3;
        total_size["recvfrom"] = 3;
        total_size["send"] = 3;
        total_size["sendto"] = 3;
    }

    std::map<std::string, unsigned int>::const_iterator it = total_size.find(tok.str());
    if (it != total_size.end())
    {
        if (arrayInfo.element_size() == 0)
            return;

        // arg : the index of the "wanted" argument in the function call.
        unsigned int arg = it->second;

        // Parse function call. When a ',' is seen, arg is decremented.
        // if arg becomes 1 then the current function parameter is the wanted parameter.
        // if arg becomes 1000 then multiply current and next argument.
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
                        const MathLib::bigint sz = MathLib::toLongNumber(tok2->strAt(1));
                        MathLib::bigint elements = 1;
                        for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
                            elements *= arrayInfo.num(i);
                        if (sz < 0 || sz > int(elements * arrayInfo.element_size()))
                        {
                            bufferOverrunError(&tok, arrayInfo.varname());
                        }
                    }

                    else if (Token::Match(tok2, ", %any% ,|)") && tok2->next()->str()[0] == '\'')
                    {
                        sizeArgumentAsCharError(tok2->next());
                    }

                    break;
                }

                if (arg == 1000)  // special code. This parameter multiplied with the next must not exceed total_size
                {
                    if (Token::Match(tok2, ", %num% , %num% ,|)"))
                    {
                        const MathLib::bigint sz = MathLib::toLongNumber(MathLib::multiply(tok2->strAt(1), tok2->strAt(3)));
                        MathLib::bigint elements = 1;
                        for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
                            elements *= arrayInfo.num(i);
                        if (sz < 0 || sz > int(elements * arrayInfo.element_size()))
                        {
                            bufferOverrunError(&tok, arrayInfo.varname());
                        }
                    }
                    break;
                }
            }
        }
    }

    // Calling a user function?
    // only 1-dimensional arrays can be checked currently
    else if (arrayInfo.num().size() == 1)
    {
        const Token *ftok = _tokenizer->getFunctionTokenByName(tok.str().c_str());
        if (Token::Match(ftok, "%var% (") && Token::Match(ftok->next()->link(), ") const| {"))
        {
            // Get varid for the corresponding parameter..
            unsigned int parameter = 1;
            unsigned int parameterVarId = 0;
            for (const Token *ftok2 = ftok->tokAt(2); ftok2; ftok2 = ftok2->next())
            {
                if (ftok2->str() == ",")
                {
                    if (parameter >= par)
                        break;
                    ++parameter;
                }
                else if (ftok2->str() == ")")
                    break;
                else if (parameter == par && Token::Match(ftok2, "%var% ,|)|["))
                {
                    // check type..
                    const Token *type = ftok2->previous();
                    while (Token::Match(type, "*|const"))
                        type = type->previous();
                    if (type && _tokenizer->sizeOfType(type) == arrayInfo.element_size())
                        parameterVarId = ftok2->varId();
                }
            }

            // No parameterVarId => bail out
            if (parameterVarId == 0)
                return;

            // Step into the function scope..
            ftok = ftok->next()->link();
            if (!Token::Match(ftok, ") const| {"))
                return;
            ftok = Token::findmatch(ftok, "{");
            ftok = ftok->next();

            // Check the parameter usage in the function scope..
            for (; ftok; ftok = ftok->next())
            {
                if (Token::Match(ftok, "if|for|while ("))
                {
                    // bailout if there is buffer usage..
                    if (bailoutIfSwitch(ftok, parameterVarId))
                    {
                        break;
                    }

                    // no bailout is needed. skip the if-block
                    else
                    {
                        // goto end of if block..
                        ftok = ftok->next()->link()->next()->link();
                        if (Token::simpleMatch(ftok, "} else {"))
                            ftok = ftok->tokAt(2)->link();
                        if (!ftok)
                            break;
                        continue;
                    }
                }

                if (ftok->str() == "}")
                    break;

                if (ftok->varId() == parameterVarId)
                {
                    if (Token::Match(ftok->previous(), "-- %var%") ||
                        Token::Match(ftok, "%var% --"))
                        break;

                    if (Token::Match(ftok->previous(), "=|;|{|}|%op% %var% [ %num% ]"))
                    {
                        const MathLib::bigint index = MathLib::toLongNumber(ftok->strAt(2));
                        if (index >= 0 && arrayInfo.num(0) > 0 && index >= arrayInfo.num(0))
                        {
                            std::list<const Token *> callstack;
                            callstack.push_back(&tok);
                            callstack.push_back(ftok);

                            std::vector<MathLib::bigint> indexes;
                            indexes.push_back(index);

                            arrayIndexOutOfBoundsError(callstack, arrayInfo, indexes);
                        }
                    }
                }
            }
        }
    }
}


void CheckBufferOverrun::checkFunctionCall(const Token *tok, const ArrayInfo &arrayInfo)
{

    // 1st parameter..
    if (Token::Match(tok->tokAt(2), "%varid% ,|)", arrayInfo.varid()))
        checkFunctionParameter(*tok, 1, arrayInfo);
    else if (Token::Match(tok->tokAt(2), "%varid% + %num% ,|)", arrayInfo.varid()))
    {
        const ArrayInfo ai(arrayInfo.limit(MathLib::toLongNumber(tok->strAt(4))));
        checkFunctionParameter(*tok, 1, ai);
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
            if (Token::Match(tok2, ", %varid% ,|)", arrayInfo.varid()))
                checkFunctionParameter(*tok, 2, arrayInfo);
            else if (Token::Match(tok2, ", %varid% + %num% ,|)", arrayInfo.varid()))
            {
                const ArrayInfo ai(arrayInfo.limit(MathLib::toLongNumber(tok2->strAt(3))));
                checkFunctionParameter(*tok, 2, ai);
            }
            break;
        }
    }
}


void CheckBufferOverrun::checkScopeForBody(const Token *tok, const ArrayInfo &arrayInfo, bool &bailout)
{
    bailout = false;
    const Token *tok2 = tok->tokAt(2);
    const MathLib::bigint size = arrayInfo.num(0);

    // Check if there is a break in the body..
    {
        const Token *bodyStart = tok->next()->link()->next();
        const Token *bodyEnd = bodyStart->link();
        if (Token::findmatch(bodyStart, "break ;", bodyEnd))
            return;
    }

    unsigned int counter_varid = 0;
    std::string min_counter_value;
    std::string max_counter_value;

    tok2 = for_init(tok2, counter_varid, min_counter_value);
    if (tok2 == 0 || counter_varid == 0)
        return;

    bool maxMinFlipped = false;
    std::string strindex;
    if (!for_condition(tok2, counter_varid, min_counter_value, max_counter_value, strindex, maxMinFlipped))
        return;

    // Get index variable and stopsize.
    bool condition_out_of_bounds = bool(size > 0);
    if (MathLib::toLongNumber(max_counter_value) < size)
        condition_out_of_bounds = false;

    if (!for3(tok2->tokAt(4), counter_varid, min_counter_value, max_counter_value, maxMinFlipped))
        return;

    if (Token::Match(tok2->tokAt(4), "%var% =|+=|-=") && MathLib::toLongNumber(max_counter_value) <= size)
        condition_out_of_bounds = false;

    // Goto the end parenthesis of the for-statement: "for (x; y; z)" ..
    tok2 = tok->next()->link();
    if (!tok2 || !tok2->tokAt(5))
    {
        bailout = true;
        return;
    }

    // Check is the counter variable increased elsewhere inside the loop or used
    // for anything else except reading
    if (for_bailout(tok2->next(), counter_varid))
    {
        bailout = true;
        return;
    }

    parse_for_body(tok2->next(), arrayInfo, strindex, condition_out_of_bounds, counter_varid, min_counter_value, max_counter_value);
}


void CheckBufferOverrun::checkScope(const Token *tok, const std::vector<std::string> &varname, const ArrayInfo &arrayInfo)
{
    const MathLib::bigint size = arrayInfo.num(0);
    const MathLib::bigint total_size = arrayInfo.element_size() * arrayInfo.num(0);
    unsigned int varid = arrayInfo.varid();

    std::string varnames;
    for (unsigned int i = 0; i < varname.size(); ++i)
        varnames += (i == 0 ? "" : " . ") + varname[i];

    const unsigned char varc(static_cast<unsigned char>(varname.empty() ? 0U : (varname.size() - 1) * 2U));

    if (Token::simpleMatch(tok, "return"))
    {
        tok = tok->next();
        if (!tok)
            return;
    }

    // Array index..
    if ((varid > 0 && Token::Match(tok, "%varid% [ %num% ]", varid)) ||
        (varid == 0 && Token::Match(tok, (varnames + " [ %num% ]").c_str())))
    {
        const MathLib::bigint index = MathLib::toLongNumber(tok->strAt(2 + varc));
        if (index >= size)
        {
            std::vector<MathLib::bigint> indexes;
            indexes.push_back(index);
            arrayIndexOutOfBoundsError(tok->tokAt(varc), arrayInfo, indexes);
        }
    }

    // If the result of pointer arithmetic means that the pointer is
    // out of bounds then this flag will be set.
    bool pointerIsOutOfBounds = false;

    // Count { and } for tok
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

        // reassign buffer => bailout
        if (varid > 0 &&
            Token::Match(tok, "[;{}] %varid% =", varid) &&
            !Token::Match(tok->tokAt(3), "%varid%", varid))
            break;

        // Array index..
        if ((varid > 0 && ((tok->str() == "return" || (!tok->isName() && !Token::Match(tok, "[.&]"))) && Token::Match(tok->next(), "%varid% [ %num% ]", varid))) ||
            (varid == 0 && ((tok->str() == "return" || (!tok->isName() && !Token::Match(tok, "[.&]"))) && Token::Match(tok->next(), (varnames + " [ %num% ]").c_str()))))
        {
            std::vector<MathLib::bigint> indexes;
            const Token *tok2 = tok->tokAt(2 + varc);
            for (; Token::Match(tok2, "[ %num% ]"); tok2 = tok2->tokAt(3))
            {
                const MathLib::bigint index = MathLib::toLongNumber(tok2->strAt(1));
                indexes.push_back(index);
            }

            if (indexes.size() == arrayInfo.num().size())
            {
                // Check if the indexes point outside the whole array..
                // char a[10][10];
                // a[0][20]  <-- ok.
                // a[9][20]  <-- error.

                // total number of elements of array..
                MathLib::bigint totalElements = 1;

                // total index..
                MathLib::bigint totalIndex = 0;

                // calculate the totalElements and totalIndex..
                for (unsigned int i = 0; i < indexes.size(); ++i)
                {
                    std::size_t ri = indexes.size() - 1 - i;
                    totalIndex += indexes[ri] * totalElements;
                    totalElements *= arrayInfo.num(ri);
                }

                // totalElements == 0 => Unknown size
                if (totalElements == 0)
                    continue;

                const Token *tok3 = tok->previous();
                while (tok3 && Token::Match(tok3->tokAt(-1), "%var% ."))
                    tok3 = tok3->tokAt(-2);

                // just taking the address?
                const bool addr(Token::Match(tok3, "&") ||
                                Token::simpleMatch(tok3->tokAt(-1), "& ("));

                // taking address of 1 past end?
                if (addr && totalIndex == totalElements)
                    continue;

                // Is totalIndex in bounds?
                if (totalIndex > totalElements || totalIndex < 0)
                {
                    arrayIndexOutOfBoundsError(tok->tokAt(1 + varc), arrayInfo, indexes);
                }
                // Is any array index out of bounds?
                else
                {
                    // check each index for overflow
                    for (unsigned int i = 0; i < indexes.size(); ++i)
                    {
                        if (indexes[i] >= arrayInfo.num(i))
                        {
                            // The access is still within the memory range for the array
                            // so it may be intentional.
                            if (_settings->inconclusive)
                            {
                                arrayIndexOutOfBoundsError(tok->tokAt(1 + varc), arrayInfo, indexes);
                                break; // only warn about the first one
                            }
                        }
                    }
                }
            }
            tok = tok2;
            continue;
        }

        // memset, memcmp, memcpy, strncpy, fgets..
        if (varid == 0 && size > 0)
        {
            if (Token::Match(tok, ("%var% ( " + varnames + " ,").c_str()))
                checkFunctionParameter(*tok, 1, arrayInfo);
            if (Token::Match(tok, ("%var% ( %var% , " + varnames + " ,").c_str()))
                checkFunctionParameter(*tok, 2, arrayInfo);
        }

        // Loop..
        if (Token::simpleMatch(tok, "for ("))
        {
            const ArrayInfo arrayInfo1(varid, varnames, (unsigned int)size, (unsigned int)total_size);
            bool bailout = false;
            checkScopeForBody(tok, arrayInfo1, bailout);
            if (bailout)
                break;
            continue;
        }

        // Writing data into array..
        if ((varid > 0 && Token::Match(tok, "strcpy|strcat ( %varid% , %str% )", varid)) ||
            (varid == 0 && Token::Match(tok, ("strcpy|strcat ( " + varnames + " , %str% )").c_str())))
        {
            const std::size_t len = Token::getStrLength(tok->tokAt(varc + 4));
            if (total_size > 0 && len >= (unsigned int)total_size)
            {
                bufferOverrunError(tok, varid > 0 ? "" : varnames.c_str());
                continue;
            }
        }
        else if ((varid > 0 && Token::Match(tok, "strcpy|strcat ( %varid% , %var% )", varid)) ||
                 (varid == 0 && Token::Match(tok, ("strcpy|strcat ( " + varnames + " , %var% )").c_str())))
        {
            const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok->tokAt(4)->varId());
            if (var && var->isArray() && var->dimensions().size() == 1)
            {
                const std::size_t len = (std::size_t)var->dimension(0);
                if (total_size > 0 && len > (unsigned int)total_size)
                {
                    if (_settings->inconclusive)
                        possibleBufferOverrunError(tok, tok->strAt(4), tok->strAt(2), tok->str() == "strcat");
                    continue;
                }
            }
        }

        // Detect few strcat() calls
        const std::string strcatPattern = varid > 0 ? std::string("strcat ( %varid% , %str% ) ;") : ("strcat ( " + varnames + " , %str% ) ;");
        if (Token::Match(tok, strcatPattern.c_str(), varid))
        {
            size_t charactersAppend = 0;
            const Token *tok2 = tok;

            while (tok2 && Token::Match(tok2, strcatPattern.c_str(), varid))
            {
                charactersAppend += Token::getStrLength(tok2->tokAt(4 + varc));
                if (charactersAppend >= static_cast<size_t>(total_size))
                {
                    bufferOverrunError(tok2);
                    break;
                }
                tok2 = tok2->tokAt(7 + varc);
            }
        }

        // sprintf..
        // TODO: change total_size to an unsigned value and remove the "&& total_size > 0" check.
        const std::string sprintfPattern = varid > 0 ? std::string("sprintf ( %varid% , %str% [,)]") : ("sprintf ( " + varnames + " , %str% [,)]");
        if (Token::Match(tok, sprintfPattern.c_str(), varid) && total_size > 0)
        {
            checkSprintfCall(tok, static_cast<unsigned int>(total_size));
        }

        // snprintf..
        const std::string snprintfPattern = varid > 0 ? std::string("snprintf ( %varid% , %num% ,") : ("snprintf ( " + varnames + " , %num% ,");
        if (Token::Match(tok, snprintfPattern.c_str(), varid))
        {
            const MathLib::bigint n = MathLib::toLongNumber(tok->strAt(4 + varc));
            if (n > total_size)
                outOfBoundsError(tok->tokAt(4 + varc), "snprintf size");
        }

        // Check function call..
        if (Token::Match(tok, "%var% (") && total_size > 0)
        {
            // No varid => function calls are not handled
            if (varid == 0)
                continue;

            const ArrayInfo arrayInfo1(varid, varnames, size, total_size / size);
            checkFunctionCall(tok, arrayInfo1);
        }

        // undefined behaviour: result of pointer arithmetic is out of bounds
        if (varid && Token::Match(tok, "= %varid% + %num% ;", varid))
        {
            const MathLib::bigint index = MathLib::toLongNumber(tok->strAt(3));
            if (index > size && _settings->isEnabled("portability"))
                pointerOutOfBoundsError(tok->next(), "buffer");
            if (index >= size && Token::Match(tok->tokAt(-2), "[;{}] %varid% =", varid))
                pointerIsOutOfBounds = true;
        }

        if (pointerIsOutOfBounds && Token::Match(tok, "[;{}=] * %varid% [;=]", varid))
        {
            outOfBoundsError(tok->tokAt(2), tok->strAt(2));
        }
    }
}


void CheckBufferOverrun::checkScope(const Token *tok, const ArrayInfo &arrayInfo)
{
    const MathLib::bigint total_size = arrayInfo.num(0) * arrayInfo.element_size();

    // Count { and } for tok
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

        // Skip array declarations
        else if (Token::Match(tok, "[;{}] %type% *| %var% [") &&
                 tok->strAt(1) != "return")
        {
            tok = tok->tokAt(3);
            continue;
        }

        else if (Token::Match(tok, "%varid% [ %num% ]", arrayInfo.varid()))
        {
            std::vector<MathLib::bigint> indexes;
            for (const Token *tok2 = tok->next(); Token::Match(tok2, "[ %num% ]"); tok2 = tok2->tokAt(3))
            {
                const MathLib::bigint index = MathLib::toLongNumber(tok2->strAt(1));
                if (index < 0)
                {
                    indexes.clear();
                    break;
                }
                indexes.push_back(index);
            }
            if (indexes.size() == arrayInfo.num().size())
            {
                // Check if the indexes point outside the whole array..
                // char a[10][10];
                // a[0][20]  <-- ok.
                // a[9][20]  <-- error.

                // total number of elements of array..
                MathLib::bigint totalElements = 1;

                // total index..
                MathLib::bigint totalIndex = 0;

                // calculate the totalElements and totalIndex..
                for (unsigned int i = 0; i < indexes.size(); ++i)
                {
                    std::size_t ri = indexes.size() - 1 - i;
                    totalIndex += indexes[ri] * totalElements;
                    totalElements *= arrayInfo.num(ri);
                }

                // totalElements == 0 => Unknown size
                if (totalElements == 0)
                    continue;

                const Token *tok2 = tok->previous();
                while (tok2 && Token::Match(tok2->tokAt(-1), "%var% ."))
                    tok2 = tok2->tokAt(-2);

                // just taking the address?
                const bool addr(Token::Match(tok2, "&") ||
                                Token::simpleMatch(tok2->tokAt(-1), "& ("));

                // taking address of 1 past end?
                if (addr && totalIndex == totalElements)
                    continue;

                // Is totalIndex in bounds?
                if (totalIndex > totalElements)
                {
                    arrayIndexOutOfBoundsError(tok, arrayInfo, indexes);
                }
                // Is any array index out of bounds?
                else
                {
                    // check each index for overflow
                    for (unsigned int i = 0; i < indexes.size(); ++i)
                    {
                        if (indexes[i] >= arrayInfo.num(i))
                        {
                            // The access is still within the memory range for the array
                            // so it may be intentional.
                            if (_settings->inconclusive)
                            {
                                arrayIndexOutOfBoundsError(tok, arrayInfo, indexes);
                                break; // only warn about the first one
                            }
                        }
                    }
                }
            }
        }

        // Loop..
        else if (Token::simpleMatch(tok, "for ("))
        {
            bool bailout = false;
            checkScopeForBody(tok, arrayInfo, bailout);
            if (bailout)
                break;
            continue;
        }


        // Check function call..
        if (Token::Match(tok, "%var% ("))
        {
            checkFunctionCall(tok, arrayInfo);
        }

        if (Token::Match(tok, "strncpy|memcpy|memmove ( %varid% , %str% , %num% )", arrayInfo.varid()))
        {
            unsigned int num = (unsigned int)MathLib::toLongNumber(tok->strAt(6));
            if (Token::getStrLength(tok->tokAt(4)) >= (unsigned int)total_size && (unsigned int)total_size == num)
            {
                if (_settings->inconclusive)
                    bufferNotZeroTerminatedError(tok, tok->strAt(2), tok->str());
            }
        }

        if ((Token::Match(tok, "strncpy|strncat ( %varid% , %var% , %num% )", arrayInfo.varid())) ||
            (Token::Match(tok, "strncpy|strncat ( %varid% , %var% [ %any% ] , %num% )", arrayInfo.varid())))
        {
            const int offset = tok->strAt(5) == "[" ? 3 : 0;

            // check for strncpy which is not terminated
            if (tok->str() == "strncpy")
            {
                // strncpy takes entire variable length as input size
                unsigned int num = (unsigned int)MathLib::toLongNumber(tok->strAt(6 + offset));

                if (num >= total_size)
                {
                    const Token *tok2 = tok->next()->link()->next();
                    for (; tok2; tok2 = tok2->next())
                    {
                        if (tok2->varId() == tok->tokAt(2)->varId())
                        {
                            if (!Token::Match(tok2, "%varid% [ %any% ]  = 0 ;", tok->tokAt(2)->varId()))
                            {
                                // this is currently 'inconclusive'. See TestBufferOverrun::terminateStrncpy3
                                if (_settings->isEnabled("style") && _settings->inconclusive)
                                    terminateStrncpyError(tok, tok->strAt(2));
                            }

                            break;
                        }
                    }
                }
            }

            // Dangerous usage of strncat..
            if (tok->str() == "strncat")
            {
                const MathLib::bigint n = MathLib::toLongNumber(tok->strAt(6 + offset));
                if (n >= total_size)
                    strncatUsageError(tok);
            }

            // Dangerous usage of strncpy + strncat..
            if (Token::Match(tok->tokAt(8 + offset), "; strncat ( %varid% , %any% , %num% )", arrayInfo.varid()))
            {
                const MathLib::bigint n = MathLib::toLongNumber(tok->strAt(6 + offset)) + MathLib::toLongNumber(tok->strAt(15 + offset));
                if (n > total_size)
                    strncatUsageError(tok->tokAt(9 + offset));
            }
        }

        // Writing data into array..
        if (Token::Match(tok, "strcpy|strcat ( %varid% , %str% )", arrayInfo.varid()))
        {
            const std::size_t len = Token::getStrLength(tok->tokAt(4));
            if (total_size > 0 && len >= (unsigned int)total_size)
            {
                bufferOverrunError(tok, arrayInfo.varname());
                continue;
            }
        }

        // Detect few strcat() calls
        if (total_size > 0 && Token::Match(tok, "strcat ( %varid% , %str% ) ;", arrayInfo.varid()))
        {
            std::size_t charactersAppend = 0;
            const Token *tok2 = tok;

            while (tok2 && Token::Match(tok2, "strcat ( %varid% , %str% ) ;", arrayInfo.varid()))
            {
                charactersAppend += Token::getStrLength(tok2->tokAt(4));
                if (charactersAppend >= (unsigned int)total_size)
                {
                    bufferOverrunError(tok2, arrayInfo.varname());
                    break;
                }
                tok2 = tok2->tokAt(7);
            }
        }


        if (Token::Match(tok, "sprintf ( %varid% , %str% [,)]", arrayInfo.varid()))
        {
            checkSprintfCall(tok, total_size);
        }

        // snprintf..
        if (total_size > 0 && Token::Match(tok, "snprintf ( %varid% , %num% ,", arrayInfo.varid()))
        {
            const MathLib::bigint n = MathLib::toLongNumber(tok->strAt(4));
            if (n > total_size)
                outOfBoundsError(tok->tokAt(4), "snprintf size");
        }

        // undefined behaviour: result of pointer arithmetic is out of bounds
        if (_settings->isEnabled("portability") && Token::Match(tok, "= %varid% + %num% ;", arrayInfo.varid()))
        {
            const MathLib::bigint index = MathLib::toLongNumber(tok->strAt(3));
            if (index < 0 || index > arrayInfo.num(0))
            {
                pointerOutOfBoundsError(tok->next(), "array");
            }
        }
    }
}


//---------------------------------------------------------------------------
// Checking local variables in a scope
//---------------------------------------------------------------------------

void CheckBufferOverrun::checkGlobalAndLocalVariable()
{
    // check all known fixed size arrays first by just looking them up
    for (size_t i = 1; i <= _tokenizer->varIdCount(); i++)
    {
        const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(i);
        if (var && var->isArray() && var->dimension(0) > 0)
        {
            ArrayInfo arrayInfo(var, _tokenizer);
            const Token *tok = var->nameToken();
            while (tok && tok->str() != ";")
            {
                if (tok->str() == "{")
                {
                    if (Token::simpleMatch(tok->previous(), "= {"))
                        tok = tok->link();
                    else
                        break;
                }
                tok = tok->next();
            }
            if (!tok)
                break;
            if (tok->str() == "{")
                tok = tok->next();
            checkScope(tok, arrayInfo);
        }
    }

    // find all dynamically allocated arrays next by parsing the token stream
    // Count { and } when parsing all tokens
    int indentlevel = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
            --indentlevel;

        // size : Max array index
        MathLib::bigint size = 0;

        // type : The type of a array element
        std::string type;

        // varid : The variable id for the array
        unsigned int varid = 0;

        // nextTok : number of tokens used in variable declaration - used to skip to next statement.
        int nextTok = 0;

        // if the previous token exists, it must be either a variable name or "[;{}]"
        if (tok->previous() && (!tok->previous()->isName() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        _errorLogger->reportProgress(_tokenizer->getFiles()->front(),
                                     "Check (BufferOverrun::checkGlobalAndLocalVariable)",
                                     tok->progressValue());

        if (indentlevel > 0 && Token::Match(tok, "[*;{}] %var% = new %type% [ %num% ]"))
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
        else if (indentlevel > 0 &&
                 Token::Match(tok, "[;{}] %var% = %str% ;") &&
                 tok->next()->varId() > 0 &&
                 NULL != Token::findmatch(_tokenizer->tokens(), "[;{}] const| %type% * %varid% ;", tok->next()->varId()))
        {
            size = 1 + int(tok->tokAt(3)->strValue().size());
            type = "char";
            varid = tok->next()->varId();
            nextTok = 4;
        }
        else if (indentlevel > 0 && Token::Match(tok, "[*;{}] %var% = malloc|alloca ( %num% ) ;"))
        {
            size = MathLib::toLongNumber(tok->strAt(5));
            type = "char";   // minimum type, typesize=1
            varid = tok->tokAt(1)->varId();
            nextTok = 7;

            if (varid > 0)
            {
                // get type of variable
                const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(varid);
                /** @todo false negatives: this may be too conservative */
                if (!var || var->typeEndToken()->str() != "*" || var->typeStartToken()->next() != var->typeEndToken())
                    continue;

                // get name of variable
                type = var->typeStartToken()->str();

                // malloc() gets count of bytes and not count of
                // elements, so we should calculate count of elements
                // manually
                unsigned int sizeOfType = _tokenizer->sizeOfType(var->typeStartToken());
                if (sizeOfType > 0)
                    size /= static_cast<int>(sizeOfType);
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
        const MathLib::bigint total_size = size * static_cast<int>(_tokenizer->sizeOfType(&sizeTok));
        if (total_size == 0)
            continue;

        std::vector<std::string> v;
        ArrayInfo temp(varid, tok->next()->str(), total_size / size, size);
        checkScope(tok->tokAt(nextTok), v, temp);
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checking member variables of structs..
//---------------------------------------------------------------------------

void CheckBufferOverrun::checkStructVariable()
{
    const SymbolDatabase * symbolDatabase = _tokenizer->getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    // find every class and struct
    for (scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope)
    {
        // only check classes and structures
        if (!scope->isClassOrStruct())
            continue;

        // check all variables to see if they are arrays
        std::list<Variable>::const_iterator var;
        for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var)
        {
            // find all array variables
            if (var->isArray())
            {
                // create ArrayInfo from the array variable
                ArrayInfo arrayInfo(&*var, _tokenizer);

                std::list<Scope>::const_iterator func_scope;

                // find every function
                for (func_scope = symbolDatabase->scopeList.begin(); func_scope != symbolDatabase->scopeList.end(); ++func_scope)
                {
                    // only check functions
                    if (func_scope->type != Scope::eFunction)
                        continue;

                    // check for member variables
                    if (func_scope->functionOf == &*scope)
                    {
                        // only check non-empty function
                        if (func_scope->function->start->next() != func_scope->function->start->link())
                        {
                            // start checking after the {
                            const Token *tok = func_scope->function->start->next();
                            checkScope(tok, arrayInfo);
                        }
                    }

                    // skip inner scopes..
                    /** @todo false negatives: handle inner scopes someday */
                    if (scope->nestedIn->isClassOrStruct())
                        continue;

                    std::vector<std::string> varname;
                    varname.push_back("");
                    varname.push_back(arrayInfo.varname());

                    // search the function and it's parameters
                    for (const Token *tok3 = func_scope->classDef; tok3 && tok3 != func_scope->classEnd; tok3 = tok3->next())
                    {
                        // search for the class/struct name
                        if (tok3->str() != scope->className)
                            continue;

                        // Declare variable: Fred fred1;
                        if (Token::Match(tok3->next(), "%var% ;"))
                            varname[0] = tok3->strAt(1);

                        // Declare pointer or reference: Fred *fred1
                        else if (Token::Match(tok3->next(), "*|& %var% [,);=]"))
                            varname[0] = tok3->strAt(2);

                        else
                            continue;

                        // check for variable sized structure
                        if (scope->type == Scope::eStruct && var->isPublic())
                        {
                            // last member of a struct with array size of 0 or 1 could be a variable sized structure
                            if (var->dimensions().size() == 1 && var->dimension(0) < 2 &&
                                var->index() == (scope->varlist.size() - 1))
                            {
                                // dynamically allocated so could be variable sized structure
                                if (tok3->next()->str() == "*")
                                {
                                    // check for allocation
                                    if ((Token::Match(tok3->tokAt(3), "; %var% = malloc ( %num% ) ;") ||
                                         (Token::Match(tok3->tokAt(3), "; %var% = (") &&
                                          Token::Match(tok3->tokAt(6)->link(), ") malloc ( %num% ) ;"))) &&
                                        (tok3->strAt(4) == tok3->strAt(2)))
                                    {
                                        MathLib::bigint size;

                                        // find size of allocation
                                        if (tok3->strAt(3) == "(") // has cast
                                            size = MathLib::toLongNumber(tok3->tokAt(6)->link()->strAt(3));
                                        else
                                            size = MathLib::toLongNumber(tok3->strAt(8));

                                        // We don't calculate the size of a structure even when we know
                                        // the size of the members.  We just assign a length of 100 for
                                        // any struct.  If the size is less than 100, we assume the
                                        // programmer knew the size and specified it rather than using
                                        // sizeof(struct). If the size is greater than 100, we assume
                                        // the programmer specified the size as sizeof(struct) + number.
                                        // Either way, this is just a guess and could be wrong.  The
                                        // information to make the right decision has been simplified
                                        // away by the time we get here.
                                        if (size != 100) // magic number for size of struct
                                        {
                                            // check if a real size was specified and give up
                                            // malloc(10) rather than malloc(sizeof(struct))
                                            if (size < 100)
                                                continue;

                                            // calculate real array size based on allocated size
                                            MathLib::bigint elements = (size - 100) / arrayInfo.element_size();
                                            arrayInfo.num(0, arrayInfo.num(0) + elements);
                                        }
                                    }

                                    // size unknown so assume it is a variable sized structure
                                    else
                                        continue;
                                }
                            }
                        }

                        // Goto end of statement.
                        const Token *CheckTok = NULL;
                        while (tok3 && tok3 != func_scope->classEnd)
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
                        ArrayInfo temp = arrayInfo;
                        temp.varid(0); // do variable lookup by variable and member names rather than varid
                        std::string varnames; // use class and member name for messages
                        for (unsigned int i = 0; i < varname.size(); ++i)
                            varnames += (i == 0 ? "" : ".") + varname[i];
                        temp.varname(varnames);
                        checkScope(CheckTok, varname, temp);
                    }
                }
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


MathLib::bigint CheckBufferOverrun::countSprintfLength(const std::string &input_string, const std::list<const Token*> &parameters)
{
    bool percentCharFound = false;
    std::size_t input_string_size = 1;
    bool handleNextParameter = false;
    std::string digits_string = "";
    bool i_d_x_f_found = false;
    std::list<const Token*>::const_iterator paramIter = parameters.begin();
    std::size_t parameterLength = 0;
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
            unsigned int tempDigits = static_cast<unsigned int>(std::abs(std::atoi(digits_string.c_str())));
            if (i_d_x_f_found)
                tempDigits = std::max(static_cast<unsigned int>(tempDigits), 1U);

            if (digits_string.find('.') != std::string::npos)
            {
                const std::string endStr = digits_string.substr(digits_string.find('.') + 1);
                unsigned int maxLen = std::max(static_cast<unsigned int>(std::abs(std::atoi(endStr.c_str()))), 1U);

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

    return (MathLib::bigint)input_string_size;
}

void CheckBufferOverrun::checkSprintfCall(const Token *tok, const MathLib::bigint size)
{
    if (size == 0)
        return;

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

                // count parentheses for tok3
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

    MathLib::bigint len = countSprintfLength(tok->tokAt(4 + varc)->strValue(), parameters);
    if (len > size)
    {
        bufferOverrunError(tok);
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

        // count { and } for tok
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
                bufferOverrunError(tok);
            }
            else if (Token::Match(tok, "sprintf ( %varid% , %str% , %var% )", dstVarId) &&
                     tok->tokAt(6)->varId() == srcVarId &&
                     tok->tokAt(4)->str().find("%s") != std::string::npos)
            {
                bufferOverrunError(tok);
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


void CheckBufferOverrun::negativeIndexError(const Token *tok, MathLib::bigint index)
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
        const MathLib::bigint index = MathLib::toLongNumber(tok->next()->str());
        if (index < 0)
        {
            // Negative index. Check if it's an array.
            const Token *tok2 = tok;
            while (Token::simpleMatch(tok2->previous(), "]"))
                tok2 = tok2->previous()->link();

            if (tok2->previous() && tok2->previous()->varId())
            {
                const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok2->previous()->varId());
                if (var && var->isArray())
                    negativeIndexError(tok, index);
            }
        }
    }
}




#include "executionpath.h"

/// @addtogroup Checks
/// @{



CheckBufferOverrun::ArrayInfo::ArrayInfo()
{
    _element_size = 0;
    _varid = 0;
}

CheckBufferOverrun::ArrayInfo::ArrayInfo(const CheckBufferOverrun::ArrayInfo &ai)
{
    *this = ai;
}

CheckBufferOverrun::ArrayInfo::ArrayInfo(const Variable *var, const Tokenizer *tokenizer)
{
    _varid = var->varId();
    _varname = var->name();
    for (size_t i = 0; i < var->dimensions().size(); i++)
        _num.push_back(var->dimension(i));
    if (var->typeEndToken()->str() == "*")
        _element_size = tokenizer->sizeOfType(var->typeEndToken());
    else if (var->typeStartToken()->str() == "struct")
        _element_size = 100;
    else
        _element_size = tokenizer->sizeOfType(var->typeEndToken());
}

CheckBufferOverrun::ArrayInfo & CheckBufferOverrun::ArrayInfo::operator=(const CheckBufferOverrun::ArrayInfo &ai)
{
    if (&ai != this)
    {
        _element_size = ai._element_size;
        _num = ai._num;
        _varid = ai._varid;
        _varname = ai._varname;
    }
    return *this;
}

/**
 * Create array info with specified data
 * The intention is that this is only a temporary solution.. all
 * checking should be based on ArrayInfo from the start and then
 * this will not be needed as the declare can be used instead.
 */
CheckBufferOverrun::ArrayInfo::ArrayInfo(unsigned int id, const std::string &name, MathLib::bigint size1, MathLib::bigint n)
{
    _element_size = size1;
    _num.push_back(n);
    _varid = id;
    _varname = name;
}

CheckBufferOverrun::ArrayInfo CheckBufferOverrun::ArrayInfo::limit(MathLib::bigint value) const
{
    MathLib::bigint uvalue = std::max(MathLib::bigint(0), value);
    MathLib::bigint n = 1;
    for (unsigned int i = 0; i < _num.size(); ++i)
        n *= _num[i];
    if (uvalue > n)
        n = uvalue;
    return ArrayInfo(_varid, _varname, _element_size, n - uvalue);
}

bool CheckBufferOverrun::ArrayInfo::declare(const Token *tok, const Tokenizer &tokenizer)
{
    _num.clear();
    _element_size = 0;
    _varname.clear();

    if (!tok)
        return false;

    if (!tok->isName() || tok->str() == "return")
        return false;

    while (tok && (tok->str() == "static" ||
                   tok->str() == "const" ||
                   tok->str() == "extern"))
        tok = tok->next();

    // ivar : number of type tokens
    int ivar = 0;
    if (Token::Match(tok, "%type% *| %var% ["))
        ivar = 1;
    else if (Token::Match(tok, "%type% %type% *| %var% ["))
        ivar = 2;
    else
        return false;

    if (tok->str().find(":") != std::string::npos)
        return false;

    // Goto variable name token, get element size..
    const Token *vartok = tok->tokAt(ivar);
    if (vartok->str() == "*")
    {
        _element_size = tokenizer.sizeOfType(vartok);
        vartok = vartok->next();
    }
    else if (tok->str() == "struct")
    {
        _element_size = 100;
    }
    else
    {
        _element_size = tokenizer.sizeOfType(tok);
    }

    _varname = vartok->str();
    _varid = vartok->varId();
    if (!_varid)
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

    if (Token::Match(atok, "] = !!{"))
        return false;

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
        : ExecutionPath(c, 0), arrayInfo(arrayinfo), value(0)
    {
    }

private:
    /** @brief Copy this check. Called from the ExecutionPath baseclass. */
    ExecutionPath *copy()
    {
        return new ExecutionPathBufferOverrun(*this);
    }

    /** @brief is other execution path equal? */
    bool is_equal(const ExecutionPath *e) const
    {
        const ExecutionPathBufferOverrun *c = static_cast<const ExecutionPathBufferOverrun *>(e);
        return (value == c->value);
    }

    /** @brief Buffer information */
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

    /** @brief Variable value. */
    MathLib::bigint value;

    /**
     * @brief Assign value to a variable
     * @param checks the execution paths
     * @param varid the variable id
     * @param value the assigned value
     */
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

    /**
     * @brief Found array usage, analyse the array usage
     * @param tok token where usage occurs (only used when reporting the error)
     * @param checks The execution paths
     * @param varid1 variable id for the array
     * @param varid2 variable id for the index
     */
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
            if (c && c->varId == varid2 && c->value >= ai.num(0))
            {
                // variable value is out of bounds, report error
                CheckBufferOverrun *checkBufferOverrun = dynamic_cast<CheckBufferOverrun *>(c->owner);
                if (checkBufferOverrun)
                {
                    std::vector<MathLib::bigint> index;
                    index.push_back(c->value);
                    checkBufferOverrun->arrayIndexOutOfBoundsError(tok, ai, index);
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

        // Assign variable (unknown value = 0)..
        if (Token::Match(tok.tokAt(-2), "(|, & %var% ,|)"))
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
    // Parse all variables and extract array info..
    std::map<unsigned int, ArrayInfo> arrayInfo;
    for (size_t i = 1; i <= _tokenizer->varIdCount(); i++)
    {
        const Variable *var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(i);
        if (var && var->isArray() && var->dimension(0) > 0)
            arrayInfo[i] = ArrayInfo(var, _tokenizer);
    }

    // Perform checking - check how the arrayInfo arrays are used
    ExecutionPathBufferOverrun c(this, arrayInfo);
    checkExecutionPaths(_tokenizer->tokens(), &c);
}




void CheckBufferOverrun::arrayIndexThenCheck()
{
    if (!_settings->isEnabled("style"))
        return;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "%var% [ %var% ]"))
        {
            const std::string arrayName(tok->str());
            const std::string indexName(tok->strAt(2));

            // skip array index..
            tok = tok->tokAt(4);
            while (tok && tok->str() == "[")
                tok = tok->link()->next();

            // syntax error
            if (!tok)
                return;

            // skip comparison
            if (Token::Match(tok, "==|!=|<|<=|>|>= %any% &&"))
                tok = tok->tokAt(2);

            // check if array index is ok
            if (Token::Match(tok, ("&& " + indexName + " <|<=").c_str()))
                arrayIndexThenCheckError(tok, indexName);
        }
    }
}

void CheckBufferOverrun::arrayIndexThenCheckError(const Token *tok, const std::string &indexName)
{
    reportError(tok, Severity::style, "arrayIndexThenCheck",
                "Array index " + indexName + " is used before limits check\n"
                "Defensive programming: The variable " + indexName + " is used as array index and then there is a check that it is within limits. This can "
                "mean that the array might be accessed out-of-bounds. Reorder conditions such as '(a[i] && i < 10)' to '(i < 10 && a[i])'. That way the "
                "array will not be accessed when the index is out of limits.");
}

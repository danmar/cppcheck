/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include <cassert>     // <- assert
#include <cstdlib>

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    CheckBufferOverrun instance;
}

//---------------------------------------------------------------------------

static void makeArrayIndexOutOfBoundsError(std::ostream& oss, const CheckBufferOverrun::ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index)
{
    oss << "Array '" << arrayInfo.varname();
    for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
        oss << "[" << arrayInfo.num(i) << "]";
    if (index.size() == 1)
        oss << "' accessed at index " << index[0] << ", which is";
    else {
        oss << "' index " << arrayInfo.varname();
        for (unsigned int i = 0; i < index.size(); ++i)
            oss << "[" << index[i] << "]";
    }
    oss << " out of bounds.";
}
void CheckBufferOverrun::arrayIndexOutOfBoundsError(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index)
{
    std::ostringstream oss;
    makeArrayIndexOutOfBoundsError(oss, arrayInfo, index);
    reportError(tok, Severity::error, "arrayIndexOutOfBounds", oss.str());
}

void CheckBufferOverrun::arrayIndexOutOfBoundsError(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<ValueFlow::Value> &index)
{
    std::ostringstream errmsg;

    errmsg << "Array '" << arrayInfo.varname();
    for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
        errmsg << "[" << arrayInfo.num(i) << "]";
    if (index.size() == 1)
        errmsg << "' accessed at index " << index[0].intvalue << ", which is out of bounds.";
    else {
        errmsg << "' index " << arrayInfo.varname();
        for (unsigned int i = 0; i < index.size(); ++i)
            errmsg << "[" << index[i].intvalue << "]";
        errmsg << " out of bounds.";
    }

    const Token *condition = 0;
    for (unsigned int i = 0; i < index.size(); ++i) {
        if (condition == NULL)
            condition = index[i].condition;
    }

    if (condition != NULL) {
        errmsg << " Otherwise condition '" << condition->expressionString() << "' is redundant.";
        std::list<const Token *> callstack;
        callstack.push_back(tok);
        callstack.push_back(condition);
        reportError(callstack, Severity::warning, "arrayIndexOutOfBoundsCond", errmsg.str());
    } else {
        reportError(tok, Severity::error, "arrayIndexOutOfBounds", errmsg.str());
    }
}

void CheckBufferOverrun::arrayIndexOutOfBoundsError(const std::list<const Token *> &callstack, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index)
{
    std::ostringstream oss;
    makeArrayIndexOutOfBoundsError(oss, arrayInfo, index);
    reportError(callstack, Severity::error, "arrayIndexOutOfBounds", oss.str());
}

static std::string bufferOverrunMessage(std::string varnames)
{
    varnames.erase(std::remove(varnames.begin(), varnames.end(), ' '), varnames.end());

    std::string errmsg("Buffer is accessed out of bounds");
    if (!varnames.empty())
        errmsg += ": " + varnames;
    else
        errmsg += ".";

    return errmsg;
}

void CheckBufferOverrun::bufferOverrunError(const Token *tok, const std::string &varnames)
{
    reportError(tok, Severity::error, "bufferAccessOutOfBounds", bufferOverrunMessage(varnames));
}


void CheckBufferOverrun::bufferOverrunError(const std::list<const Token *> &callstack, const std::string &varnames)
{
    reportError(callstack, Severity::error, "bufferAccessOutOfBounds", bufferOverrunMessage(varnames));
}

void CheckBufferOverrun::possibleBufferOverrunError(const Token *tok, const std::string &src, const std::string &dst, bool cat)
{
    if (cat)
        reportError(tok, Severity::warning, "possibleBufferAccessOutOfBounds",
                    "Possible buffer overflow if strlen(" + src + ") is larger than sizeof(" + dst + ")-strlen(" + dst +").\n"
                    "Possible buffer overflow if strlen(" + src + ") is larger than sizeof(" + dst + ")-strlen(" + dst +"). "
                    "The source buffer is larger than the destination buffer so there is the potential for overflowing the destination buffer.");
    else
        reportError(tok, Severity::warning, "possibleBufferAccessOutOfBounds",
                    "Possible buffer overflow if strlen(" + src + ") is larger than or equal to sizeof(" + dst + ").\n"
                    "Possible buffer overflow if strlen(" + src + ") is larger than or equal to sizeof(" + dst + "). "
                    "The source buffer is larger than the destination buffer so there is the potential for overflowing the destination buffer.");
}

void CheckBufferOverrun::possibleReadlinkBufferOverrunError(const Token* tok, const std::string &funcname, const std::string &varname)
{
    const std::string errmsg = funcname + "() might return the full size of '" + varname + "'. Lower the supplied size by one.\n" +
                               funcname + "() might return the full size of '" + varname + "'. "
                               "If a " + varname + "[len] = '\\0'; statement follows, it will overrun the buffer. Lower the supplied size by one.";

    reportError(tok, Severity::warning, "possibleReadlinkBufferOverrun", errmsg, true);
}

void CheckBufferOverrun::strncatUsageError(const Token *tok)
{
    if (_settings && !_settings->isEnabled("warning"))
        return;

    reportError(tok, Severity::warning, "strncatUsage",
                "Dangerous usage of strncat - 3rd parameter is the maximum number of characters to append.\n"
                "strncat appends at max its 3rd parameter's amount of characters. The safe way to use "
                "strncat is to calculate remaining space in the buffer and use it as 3rd parameter.");
}

void CheckBufferOverrun::outOfBoundsError(const Token *tok, const std::string &what, const bool show_size_info, const MathLib::bigint &supplied_size, const MathLib::bigint &actual_size)
{
    std::ostringstream oss;

    oss << what << " is out of bounds";
    if (show_size_info)
        oss << ": Supplied size " << supplied_size << " is larger than actual size " << actual_size;
    oss << '.';
    reportError(tok, Severity::error, "outOfBounds", oss.str());
}

void CheckBufferOverrun::pointerOutOfBoundsError(const Token *tok, const std::string &object)
{
    reportError(tok, Severity::portability, "pointerOutOfBounds", "Undefined behaviour: Pointer arithmetic result does not point into or just past the end of the " + object + ".\n"
                "Undefined behaviour: The result of this pointer arithmetic does not point into or just one element past the end of the " + object + ". Further information: https://www.securecoding.cert.org/confluence/display/seccode/ARR30-C.+Do+not+form+or+use+out+of+bounds+pointers+or+array+subscripts");
}

void CheckBufferOverrun::sizeArgumentAsCharError(const Token *tok)
{
    if (_settings && !_settings->isEnabled("warning"))
        return;
    reportError(tok, Severity::warning, "sizeArgumentAsChar", "The size argument is given as a char constant.");
}


void CheckBufferOverrun::terminateStrncpyError(const Token *tok, const std::string &varname)
{
    reportError(tok, Severity::warning, "terminateStrncpy",
                "The buffer '" + varname + "' may not be null-terminated after the call to strncpy().\n"
                "If the source string's size fits or exceeds the given size, strncpy() does not add a "
                "zero at the end of the buffer. This causes bugs later in the code if the code "
                "assumes buffer is null-terminated.", true);
}

void CheckBufferOverrun::cmdLineArgsError(const Token *tok)
{
    reportError(tok, Severity::error, "insecureCmdLineArgs", "Buffer overrun possible for long command line arguments.");
}

void CheckBufferOverrun::bufferNotZeroTerminatedError(const Token *tok, const std::string &varname, const std::string &function)
{
    const std::string errmsg = "The buffer '" + varname + "' is not null-terminated after the call to " + function + "().\n"
                               "The buffer '" + varname + "' is not null-terminated after the call to " + function + "(). "
                               "This will cause bugs later in the code if the code assumes the buffer is null-terminated.";

    reportError(tok, Severity::warning, "bufferNotZeroTerminated", errmsg, true);
}

void CheckBufferOverrun::argumentSizeError(const Token *tok, const std::string &functionName, const std::string &varname)
{
    reportError(tok, Severity::warning, "argumentSize", "The array '" + varname + "' is too small, the function '" + functionName + "' expects a bigger one.");
}

void CheckBufferOverrun::negativeMemoryAllocationSizeError(const Token *tok)
{
    reportError(tok, Severity::error, "negativeMemoryAllocationSize",
                "Memory allocation size have to be greater or equal to 0.\n"
                "Memory allocation size have to be greater or equal to 0."
                "The allocation size of memory have to be greater or equal to 0 because"
                "negative size have no speficied behaviour.");
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Check array usage..
//---------------------------------------------------------------------------

/**
 * @brief This is a helper class to be used with std::find_if
 */
class TokenStrEquals {
public:
    /**
     * @param str Token::str() is compared against this.
     */
    explicit TokenStrEquals(const std::string &str)
        : value(str) {
    }

    /**
     * Called automatically by std::find_if
     * @param tok Token inside the list
     */
    bool operator()(const Token *tok) const {
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
    const bool is_if = tok->str() == "if";

    const Token* end = tok->linkAt(1)->linkAt(1);
    if (Token::simpleMatch(end, "} else {")) // scan the else-block
        end = end->linkAt(2);
    if (Token::simpleMatch(end, "{")) // Ticket #5203: Invalid code, bailout
        return true;
    for (; tok != end; tok = tok->next()) {
        // If scanning a "if" block then bailout for "break"
        if (is_if && (tok->str() == "break" || tok->str() == "continue"))
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
 * \param varname [out] name of counter variable
 * \param init_value [out] init value of counter variable
 * \return success => pointer to the for loop condition. fail => 0. If 0 is returned and varname has been set then there is
 * a missing varid for the counter variable
 */
static const Token *for_init(const Token *tok, unsigned int &varid, std::string &varname, std::string &init_value)
{
    if (Token::Match(tok, "%var% = %any% ;")) {
        if (tok->tokAt(2)->isNumber()) {
            init_value = tok->strAt(2);
        }

        varid = tok->varId();
        varname = tok->str();
        tok = tok->tokAt(4);

        if (varid == 0)
            return 0;  // failed
    } else if (Token::Match(tok, "%type% %var% = %any% ;")) {
        if (tok->tokAt(3)->isNumber()) {
            init_value = tok->strAt(3);
        }

        varid = tok->next()->varId();
        varname = tok->next()->str();
        tok = tok->tokAt(5);
    } else if (Token::Match(tok, "%type% %type% %var% = %any% ;")) {
        if (tok->tokAt(4)->isNumber()) {
            init_value = tok->strAt(4);
        }

        varid = tok->tokAt(2)->varId();
        varname = tok->strAt(2);
        tok = tok->tokAt(6);
    } else
        return 0;

    if (!init_value.empty() && (Token::Match(tok, "-- %varid%", varid) || Token::Match(tok, "%varid% --", varid))) {
        init_value = MathLib::subtract(init_value, "1");
    }

    return tok;
}


/** Parse for condition */
static bool for_condition(const Token *tok2, unsigned int varid, std::string &min_value, std::string &max_value, bool &maxMinFlipped)
{
    if (Token::Match(tok2, "%varid% < %num% ;|&&|%oror%", varid) ||
        Token::Match(tok2, "%varid% != %num% ; ++ %varid%", varid) ||
        Token::Match(tok2, "%varid% != %num% ; %varid% ++", varid)) {
        maxMinFlipped = false;
        const MathLib::bigint value = MathLib::toLongNumber(tok2->strAt(2));
        max_value = MathLib::toString(value - 1);
    } else if (Token::Match(tok2, "%varid% <= %num% ;|&&|%oror%", varid)) {
        maxMinFlipped = false;
        max_value = tok2->strAt(2);
    } else if (Token::Match(tok2, "%num% < %varid% ;|&&|%oror%", varid) ||
               Token::Match(tok2, "%num% != %varid% ; ++ %varid%", varid) ||
               Token::Match(tok2, "%num% != %varid% ; %varid% ++", varid)) {
        maxMinFlipped = true;
        const MathLib::bigint value = MathLib::toLongNumber(tok2->str());
        max_value = min_value;
        min_value = MathLib::toString(value + 1);
    } else if (Token::Match(tok2, "%num% <= %varid% ;|&&|%oror%", varid)) {
        maxMinFlipped = true;
        max_value = min_value;
        min_value = tok2->str();
    }  else if (Token::Match(tok2, "%varid% -- ; )", varid) ||
                Token::Match(tok2, "-- %varid% ; )", varid)) {
        maxMinFlipped = true;
        max_value = min_value;
        min_value = (tok2->str() == "--") ? "1" : "0";
    } else {
        // parse condition
        while (tok2 && tok2->str() != ";") {
            if (tok2->str() == "(")
                tok2 = tok2->link();
            else if (tok2->str() == ")")    // unexpected ")" => break
                break;
            if (tok2->str() == "&&" || tok2->str() == "||") {
                if (for_condition(tok2->next(), varid, min_value, max_value, maxMinFlipped))
                    return true;
            }
            tok2 = tok2->next();
        }
        return false;
    }

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
    max_value = MathLib::toString(max);
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
    if (Token::Match(tok, "%varid%  = %num% + %varid% )", varid)) {
        if (!for_maxvalue(tok->tokAt(2), min_value, max_value))
            return false;
    } else if (Token::Match(tok, "%varid% = %varid% + %num% )", varid)) {
        if (!for_maxvalue(tok->tokAt(4), min_value, max_value))
            return false;
    } else if (Token::Match(tok, "%varid% = %num% - %varid% )", varid)) {
        if (!for_maxvalue(tok->tokAt(2), min_value, max_value))
            return false;
    } else if (Token::Match(tok, "%varid% = %varid% - %num% )", varid)) {
        if (!for_maxvalue(tok->tokAt(4), min_value, max_value))
            return false;
    } else if (Token::Match(tok, "--| %varid% --| )", varid)) {
        if (!maxMinFlipped && MathLib::toLongNumber(min_value) < MathLib::toLongNumber(max_value)) {
            // Code relies on the fact that integer will overflow:
            // for (unsigned int i = 3; i < 5; --i)

            // Set min value in this case to zero.
            max_value = min_value;
            min_value = "0";
        }
    } else if (! Token::Match(tok, "++| %varid% ++| )", varid)) {
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
    for (const Token *loopTok = tok1; loopTok && loopTok != tok1->link(); loopTok = loopTok->next()) {
        if (loopTok->varId() == varid) {
            // Counter variable used inside loop
            if (Token::Match(loopTok->next(), "++|--|=") ||
                (loopTok->previous()->type() == Token::eIncDecOp)) {
                return true;
            }
        }
    }
    return false;
}


void CheckBufferOverrun::parse_for_body(const Token *tok, const ArrayInfo &arrayInfo, const std::string &strindex, bool condition_out_of_bounds, unsigned int counter_varid, const std::string &min_counter_value, const std::string &max_counter_value)
{
    const std::string pattern = (arrayInfo.declarationId() ? std::string("%varid%") : arrayInfo.varname()) + " [ " + strindex + " ]";

    for (const Token* tok2 = tok; tok2 && tok2 != tok->link(); tok2 = tok2->next()) {
        // TestBufferOverrun::array_index_for_question
        if (tok2->str() == "?") {
            // does condition check counter variable?
            bool usesCounter = false;
            const Token *tok3 = tok2->previous();
            while (Token::Match(tok3, "%comp%|%num%|%var%|)")) {
                if (tok3->str() == strindex) {
                    usesCounter = true;
                    break;
                }
                tok3 = tok3->previous();
            }

            // If strindex is used in the condition then skip the
            // conditional expressions
            if (usesCounter) {
                while (tok2 && !Token::Match(tok2, "[)],;]")) {
                    if (tok2->str() == "(" || tok2->str() == "[")
                        tok2 = tok2->link();
                    tok2 = tok2->next();
                }
                if (!tok2)
                    break;
                continue;
            }
        }

        if (Token::simpleMatch(tok2, "for (") && Token::simpleMatch(tok2->next()->link(), ") {")) {
            const Token *endpar = tok2->next()->link();
            const Token *startbody = endpar->next();
            const Token *endbody = startbody->link();
            tok2 = endbody;
            continue;
        }

        if (Token::Match(tok2, "if|switch")) {
            if (bailoutIfSwitch(tok2, arrayInfo.declarationId()))
                break;
        }

        if (condition_out_of_bounds && Token::Match(tok2, pattern.c_str(), arrayInfo.declarationId())) {
            bufferOverrunError(tok2, arrayInfo.varname());
            break;
        }

        else if (arrayInfo.declarationId() && tok2->varId() && counter_varid > 0 && !min_counter_value.empty() && !max_counter_value.empty()) {
            // Is the loop variable used to calculate the array index?
            // In this scope it is determined if such calculated
            // array indexes are out of bounds.
            // Only the minimum and maximum results of the calculation is
            // determined

            // Minimum calculated array index
            int min_index = 0;

            // Maximum calculated array index
            int max_index = 0;

            if (Token::Match(tok2, "%varid% [ %var% +|-|*|/ %num% ]", arrayInfo.declarationId()) &&
                tok2->tokAt(2)->varId() == counter_varid) {
                // operator: +-*/
                const char action = tok2->strAt(3)[0];

                // second operator
                const std::string &second(tok2->strAt(4));

                //printf("min_index: %s %c %s\n", min_counter_value.c_str(), action, second.c_str());
                //printf("max_index: %s %c %s\n", max_counter_value.c_str(), action, second.c_str());
                min_index = std::atoi(MathLib::calculate(min_counter_value, second, action).c_str());
                max_index = std::atoi(MathLib::calculate(max_counter_value, second, action).c_str());
            } else if (Token::Match(tok2, "%varid% [ %num% +|-|*|/ %var% ]", arrayInfo.declarationId()) &&
                       tok2->tokAt(4)->varId() == counter_varid) {
                // operator: +-*/
                const char action = tok2->strAt(3)[0];

                // first operand
                const std::string &first(tok2->strAt(2));

                //printf("min_index: %s %c %s\n", first.c_str(), action, min_counter_value.c_str());
                //printf("max_index: %s %c %s\n", first.c_str(), action, max_counter_value.c_str());

                min_index = std::atoi(MathLib::calculate(first, min_counter_value, action).c_str());
                max_index = std::atoi(MathLib::calculate(first, max_counter_value, action).c_str());
            }

            else {
                continue;
            }

            //printf("min_index = %d, max_index = %d, size = %d\n", min_index, max_index, size);
            if (min_index < 0 || max_index < 0) {
                std::vector<MathLib::bigint> indexes;
                indexes.push_back(std::min(min_index, max_index));
                arrayIndexOutOfBoundsError(tok2, arrayInfo, indexes);
            }

            // skip 0 length arrays
            if (arrayInfo.num(0) == 0)
                ;

            // taking address.
            else if (tok2->previous()->str() == "&" && max_index == arrayInfo.num(0))
                ;

            else if (arrayInfo.num(0) && (min_index >= arrayInfo.num(0) || max_index >= arrayInfo.num(0))) {
                std::vector<MathLib::bigint> indexes;
                indexes.push_back(std::max(min_index, max_index));
                arrayIndexOutOfBoundsError(tok2, arrayInfo, indexes);
            }
        }
    }
}


void CheckBufferOverrun::checkFunctionParameter(const Token &tok, unsigned int par, const ArrayInfo &arrayInfo, std::list<const Token *> callstack)
{
    // total_size : which parameter in function call takes the total size?
    std::map<std::string, unsigned int> total_size;

    total_size["fgets"] = 2; // The second argument for fgets can't exceed the total size of the array
    total_size["memcmp"] = 3;
    total_size["memcpy"] = 3;
    total_size["memmove"] = 3;
    total_size["memchr"] = 3;

    if (par == 1) {
        // reading from array
        // if it is zero terminated properly there won't be buffer overruns
        total_size["strncat"] = 3;
        total_size["strncpy"] = 3;
        total_size["memset"] = 3;
        total_size["fread"] = 1001;     // parameter 2 * parameter 3
        total_size["fwrite"] = 1001;    // parameter 2 * parameter 3
    }

    else if (par == 2) {
        if (_settings->standards.posix) {
            total_size["read"] = 3;
            total_size["pread"] = 3;
            total_size["write"] = 3;
            total_size["recv"] = 3;
            total_size["recvfrom"] = 3;
            total_size["send"] = 3;
            total_size["sendto"] = 3;
        }
    }

    if (Token::Match(tok.previous(), ".") || Token::Match(tok.tokAt(-2), "!!std ::"))
        total_size.clear();

    std::map<std::string, unsigned int>::const_iterator it = total_size.find(tok.str());
    if (it != total_size.end()) {
        if (arrayInfo.element_size() == 0)
            return;

        // arg : the index of the "wanted" argument in the function call.
        const unsigned int arg = it->second;

        // Parse function call. When a ',' is seen, arg is decremented.
        // if arg becomes 1 then the current function parameter is the wanted parameter.
        // if arg becomes 1001 then multiply current and next argument.
        const Token *tok2 = tok.tokAt(2)->nextArgument();
        if (arg == 3)
            tok2 = tok2->nextArgument();
        if ((arg == 2 || arg == 3) && tok2) {
            if (Token::Match(tok2, "%num% ,|)")) {
                const MathLib::bigint sz = MathLib::toLongNumber(tok2->str());
                MathLib::bigint elements = 1;
                for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
                    elements *= arrayInfo.num(i);
                if (sz < 0 || sz > int(elements * arrayInfo.element_size())) {
                    bufferOverrunError(callstack, arrayInfo.varname());
                }
            }

            else if (Token::Match(tok2->next(), ",|)") && tok2->type() == Token::eChar) {
                sizeArgumentAsCharError(tok2);
            }
        } else if (arg == 1001) { // special code. This parameter multiplied with the next must not exceed total_size
            if (Token::Match(tok2, "%num% , %num% ,|)")) {
                const MathLib::bigint sz = MathLib::toLongNumber(MathLib::multiply(tok2->str(), tok2->strAt(2)));
                MathLib::bigint elements = 1;
                for (unsigned int i = 0; i < arrayInfo.num().size(); ++i)
                    elements *= arrayInfo.num(i);
                if (sz < 0 || sz > int(elements * arrayInfo.element_size())) {
                    bufferOverrunError(&tok, arrayInfo.varname());
                }
            }
        }
    }

    // Calling a user function?
    // only 1-dimensional arrays can be checked currently
    else if (arrayInfo.num().size() == 1) {
        const Function* const func = tok.function();

        if (func && func->hasBody) {
            // Get corresponding parameter..
            const Variable* const parameter = func->getArgumentVar(par-1);

            // Ensure that it has a compatible size..
            if (!parameter || _tokenizer->sizeOfType(parameter->typeStartToken()) != arrayInfo.element_size())
                return;

            // No variable id occur for instance when:
            // - Variable function arguments: "void f(...)"
            // - Unnamed parameter: "void f(char *)"
            if (parameter->declarationId() == 0)
                return;

            // Check the parameter usage in the function scope..
            for (const Token* ftok = func->functionScope->classStart; ftok != func->functionScope->classEnd; ftok = ftok->next()) {
                if (Token::Match(ftok, "if|for|while (")) {
                    // bailout if there is buffer usage..
                    if (bailoutIfSwitch(ftok, parameter->declarationId())) {
                        break;
                    }

                    // no bailout is needed. skip the if-block
                    else {
                        // goto end of if block..
                        ftok = ftok->next()->link()->next()->link();
                        if (Token::simpleMatch(ftok, "} else {"))
                            ftok = ftok->linkAt(2);
                        if (!ftok)
                            break;
                        continue;
                    }
                }

                if (ftok->str() == "}")
                    break;

                if (ftok->varId() == parameter->declarationId()) {
                    if (Token::Match(ftok->previous(), "-- %var%") ||
                        Token::Match(ftok, "%var% --"))
                        break;

                    if (Token::Match(ftok->previous(), ";|{|}|%op% %var% [ %num% ]")) {
                        const MathLib::bigint index = MathLib::toLongNumber(ftok->strAt(2));
                        if (index >= 0 && arrayInfo.num(0) > 0 && index >= arrayInfo.num(0)) {
                            std::list<const Token *> callstack2(callstack);
                            callstack2.push_back(ftok);

                            std::vector<MathLib::bigint> indexes;
                            indexes.push_back(index);

                            arrayIndexOutOfBoundsError(callstack2, arrayInfo, indexes);
                        }
                    }
                }

                // Calling function..
                if (Token::Match(ftok, "%var% (")) {
                    ArrayInfo ai(arrayInfo);
                    ai.declarationId(parameter->declarationId());
                    checkFunctionCall(ftok, ai, callstack);
                }
            }
        }
    }

    // Check 'float x[10]' arguments in declaration
    if (_settings->isEnabled("warning")) {
        const Function* const func = tok.function();

        // If argument is '%type% a[num]' then check bounds against num
        if (func) {
            const Variable* const argument = func->getArgumentVar(par-1);
            const Token *nameToken;
            if (argument && Token::Match(argument->typeStartToken(), "%type% %var% [ %num% ] [,)[]")
                && (nameToken = argument->nameToken()) != NULL) {
                const Token *tok2 = nameToken->next();

                MathLib::bigint argsize = _tokenizer->sizeOfType(argument->typeStartToken());
                if (argsize == 100) // unknown size
                    argsize = 0;
                while (Token::Match(tok2, "[ %num% ] [,)[]")) {
                    argsize *= MathLib::toLongNumber(tok2->strAt(1));
                    tok2 = tok2->tokAt(3);
                }

                MathLib::bigint arraysize = arrayInfo.element_size();
                if (arraysize == 100) // unknown size
                    arraysize = 0;
                for (unsigned int i = 0; i < arrayInfo.num().size(); i++)
                    arraysize *= arrayInfo.num(i);

                if (Token::Match(tok2, "[,)]") && arraysize > 0 && argsize > arraysize)
                    argumentSizeError(&tok, tok.str(), arrayInfo.varname());
            }
        }
    }
}


void CheckBufferOverrun::checkFunctionCall(const Token *tok, const ArrayInfo &arrayInfo, std::list<const Token *> callstack)
{
    // Don't go deeper than 2 levels, the checking can get very slow
    // when there is no limit
    if (callstack.size() >= 2)
        return;

    // Prevent recursion
    for (std::list<const Token*>::const_iterator it = callstack.begin(); it != callstack.end(); ++it) {
        // Same function name => bail out
        if (tok->str() == (*it)->str())
            return;
    }
    callstack.push_back(tok);

    const Token *tok2 = tok->tokAt(2);
    // 1st parameter..
    if (Token::Match(tok2, "%varid% ,|)", arrayInfo.declarationId()))
        checkFunctionParameter(*tok, 1, arrayInfo, callstack);
    else if (Token::Match(tok2, "%varid% + %num% ,|)", arrayInfo.declarationId())) {
        const ArrayInfo ai(arrayInfo.limit(MathLib::toLongNumber(tok2->strAt(2))));
        checkFunctionParameter(*tok, 1, ai, callstack);
    }

    // goto 2nd parameter and check it..
    tok2 = tok2->nextArgument();
    if (Token::Match(tok2, "%varid% ,|)", arrayInfo.declarationId()))
        checkFunctionParameter(*tok, 2, arrayInfo, callstack);
    else if (Token::Match(tok2, "%varid% + %num% ,|)", arrayInfo.declarationId())) {
        const ArrayInfo ai(arrayInfo.limit(MathLib::toLongNumber(tok2->strAt(2))));
        checkFunctionParameter(*tok, 2, ai, callstack);
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
        if (Token::findsimplematch(bodyStart, "break ;", bodyEnd))
            return;
    }

    std::string counter_name;
    unsigned int counter_varid = 0;
    std::string counter_init_value;

    tok2 = for_init(tok2, counter_varid, counter_name, counter_init_value);
    if (tok2 == 0 && !counter_name.empty())
        _tokenizer->getSymbolDatabase()->debugMessage(tok, "for loop variable \'" + counter_name + "\' has varid 0.");
    if (tok2 == 0 || counter_varid == 0)
        return;

    bool maxMinFlipped = false;
    std::string min_counter_value = counter_init_value;
    std::string max_counter_value;
    if (!for_condition(tok2, counter_varid, min_counter_value, max_counter_value, maxMinFlipped)) {
        // Can't understand the condition. Check that the start value
        // is used correctly
        const Token * const startForScope = tok->next()->link()->next();
        if (!for_bailout(startForScope, counter_varid)) {
            // Get index variable and stopsize.
            bool condition_out_of_bounds = bool(size > 0);
            if (MathLib::toLongNumber(counter_init_value) < size)
                condition_out_of_bounds = false;

            parse_for_body(startForScope, arrayInfo, counter_name, condition_out_of_bounds, counter_varid, counter_init_value, counter_init_value);
        }
        return;
    }

    // Get index variable and stopsize.
    bool condition_out_of_bounds = bool(size > 0);
    if (MathLib::toLongNumber(max_counter_value) < size)
        condition_out_of_bounds = false;

    // Goto the end of the condition
    while (tok2 && tok2->str() != ";") {
        if (tok2->str() == "(")
            tok2 = tok2->link();
        else if (tok2->str() == ")")  // unexpected ")" => break
            break;
        tok2 = tok2->next();
    }
    if (!tok2 || tok2->str() != ";")
        return;
    const bool hasFor3 = tok2->next()->str() != ")";
    if (hasFor3 && !for3(tok2->next(), counter_varid, min_counter_value, max_counter_value, maxMinFlipped))
        return;

    if (Token::Match(tok2->next(), "%var% =") && MathLib::toLongNumber(max_counter_value) < size)
        condition_out_of_bounds = false;

    // Goto the end parentheses of the for-statement: "for (x; y; z)" ..
    tok2 = tok->next()->link();
    if (!tok2 || !tok2->tokAt(5)) {
        bailout = true;
        return;
    }

    // Check is the counter variable increased elsewhere inside the loop or used
    // for anything else except reading
    if (for_bailout(tok2->next(), counter_varid)) {
        bailout = true;
        return;
    }

    parse_for_body(tok2->next(), arrayInfo, counter_name, condition_out_of_bounds, counter_varid, min_counter_value, max_counter_value);
}

void CheckBufferOverrun::arrayIndexInForLoop(const Token *tok, const ArrayInfo &arrayInfo)
{
    const MathLib::bigint size = arrayInfo.num(0);
    const Token *tok3 = tok->tokAt(2);
    std::string counter_name;
    unsigned int counter_varid = 0;
    std::string counter_init_value;

    tok3 = for_init(tok3, counter_varid, counter_name, counter_init_value);
    if (tok3 == 0 && !counter_name.empty())
        _tokenizer->getSymbolDatabase()->debugMessage(tok, "for loop variable \'" + counter_name + "\' has varid 0.");
    if (tok3 == 0 || counter_varid == 0)
        return;

    bool maxMinFlipped = false;
    std::string min_counter_value = counter_init_value;
    std::string max_counter_value;

    if (!for_condition(tok3, counter_varid, min_counter_value, max_counter_value, maxMinFlipped))
        return;

    const MathLib::bigint max_value = MathLib::toLongNumber(max_counter_value);

    // Skip condition
    while (tok3 && tok3->str() != ";")
        tok3 = tok3->next();

    if (max_value > size && Token::simpleMatch(tok3, "; ) {")) {
        const Token * const endToken = tok3->linkAt(2);
        const Token *useToken = NULL;
        bool incrementInLoop = false;
        for (const Token *loopTok = tok3->tokAt(3); loopTok != endToken; loopTok = loopTok->next()) {
            if (Token::Match(loopTok, "%varid% [ %var% ++| ]", arrayInfo.declarationId()) && loopTok->tokAt(2)->varId() == counter_varid)
                useToken = loopTok;
            if (Token::Match(loopTok, "%varid% ++", counter_varid))
                incrementInLoop = true;
        }

        if ((useToken != NULL) && incrementInLoop)
            bufferOverrunError(useToken, arrayInfo.varname());
    }
}

void CheckBufferOverrun::checkScope(const Token *tok, const std::vector<std::string> &varname, const ArrayInfo &arrayInfo)
{
    const MathLib::bigint size = arrayInfo.num(0);
    if (size == 0)  // unknown size
        return;

    const MathLib::bigint total_size = arrayInfo.element_size() * arrayInfo.num(0);
    const unsigned int declarationId = arrayInfo.declarationId();

    std::string varnames;
    for (unsigned int i = 0; i < varname.size(); ++i)
        varnames += (i == 0 ? "" : " . ") + varname[i];

    const unsigned char varc = static_cast<unsigned char>(varname.empty() ? 0U : (varname.size() - 1) * 2U);

    if (tok->str() == "return") {
        tok = tok->next();
        if (!tok)
            return;
    }

    // Array index..
    if ((declarationId > 0 && Token::Match(tok, "%varid% [ %num% ]", declarationId)) ||
        (declarationId == 0 && Token::Match(tok, (varnames + " [ %num% ]").c_str()))) {
        const MathLib::bigint index = MathLib::toLongNumber(tok->strAt(2 + varc));
        if (index >= size) {
            std::vector<MathLib::bigint> indexes;
            indexes.push_back(index);
            arrayIndexOutOfBoundsError(tok->tokAt(varc), arrayInfo, indexes);
        }
    }

    // If the result of pointer arithmetic means that the pointer is
    // out of bounds then this flag will be set.
    bool pointerIsOutOfBounds = false;

    for (const Token* const end = tok->scope()->classEnd; tok != end; tok = tok->next()) {
        if (declarationId != 0 && Token::Match(tok, "%varid% = new|malloc|realloc", declarationId)) {
            // Abort
            break;
        }

        // reassign buffer
        if (declarationId > 0 && Token::Match(tok, "[;{}] %varid% = %any%", declarationId)) {
            // using varid .. bailout
            if (tok->tokAt(3)->varId() != declarationId)
                break;
            pointerIsOutOfBounds = false;
        }

        // Array index..
        if ((declarationId > 0 && ((tok->str() == "return" || (!tok->isName() && !Token::Match(tok, "[.&]"))) && Token::Match(tok->next(), "%varid% [ %num% ]", declarationId))) ||
            (declarationId == 0 && ((tok->str() == "return" || (!tok->isName() && !Token::Match(tok, "[.&]"))) && (Token::Match(tok->next(), (varnames + " [ %num% ]").c_str()) || Token::Match(tok->next(), (varname[0] +" [ %num% ] . " + varname[1] + " [ %num% ]").c_str()))))) {
            std::vector<MathLib::bigint> indexes;
            const Token *tok2 = tok->tokAt(2 + varc);
            for (; Token::Match(tok2, "[ %num% ]"); tok2 = tok2->tokAt(3)) {
                const MathLib::bigint index = MathLib::toLongNumber(tok2->strAt(1));
                indexes.push_back(index);
            }
            for (; Token::Match(tok2->tokAt(3), "[ %num% ]"); tok2 = tok2->tokAt(3)) {
                const MathLib::bigint index = MathLib::toLongNumber(tok2->strAt(4));
                indexes.push_back(index);
            }

            if (indexes.size() == arrayInfo.num().size()) {
                // Check if the indexes point outside the whole array..
                // char a[10][10];
                // a[0][20]  <-- ok.
                // a[9][20]  <-- error.

                // total number of elements of array..
                MathLib::bigint totalElements = 1;

                // total index..
                MathLib::bigint totalIndex = 0;

                // calculate the totalElements and totalIndex..
                for (unsigned int i = 0; i < indexes.size(); ++i) {
                    std::size_t ri = indexes.size() - 1 - i;
                    totalIndex += indexes[ri] * totalElements;
                    totalElements *= arrayInfo.num(ri);
                }

                // totalElements == 0 => Unknown size
                if (totalElements == 0)
                    continue;

                const Token *tok3 = tok->previous();
                while (tok3 && Token::Match(tok3->previous(), "%var% ."))
                    tok3 = tok3->tokAt(-2);

                // just taking the address?
                const bool addr(tok3 && (tok3->str() == "&" ||
                                         Token::simpleMatch(tok3->previous(), "& (")));

                // taking address of 1 past end?
                if (addr && totalIndex == totalElements)
                    continue;

                // Is totalIndex in bounds?
                if (totalIndex > totalElements || totalIndex < 0) {
                    arrayIndexOutOfBoundsError(tok->tokAt(1 + varc), arrayInfo, indexes);
                }
                // Is any array index out of bounds?
                else {
                    // check each index for overflow
                    for (unsigned int i = 0; i < indexes.size(); ++i) {
                        if (indexes[i] >= arrayInfo.num(i)) {
                            if (indexes.size() == 1U) {
                                arrayIndexOutOfBoundsError(tok->tokAt(1 + varc), arrayInfo, indexes);
                                break; // only warn about the first one
                            }

                            // The access is still within the memory range for the array
                            // so it may be intentional.
                            else if (_settings->inconclusive) {
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
        if (declarationId == 0 && size > 0) {
            std::list<const Token *> callstack;
            callstack.push_back(tok);
            if (Token::Match(tok, ("%var% ( " + varnames + " ,").c_str()))
                checkFunctionParameter(*tok, 1, arrayInfo, callstack);
            if (Token::Match(tok, ("%var% ( %var% , " + varnames + " ,").c_str()))
                checkFunctionParameter(*tok, 2, arrayInfo, callstack);
        }

        // Loop..
        if (Token::simpleMatch(tok, "for (")) {
            const ArrayInfo arrayInfo1(declarationId, varnames, (unsigned int)size, (unsigned int)total_size);
            bool bailout = false;
            checkScopeForBody(tok, arrayInfo1, bailout);
            if (bailout)
                break;
            continue;
        }

        // Writing data into array..
        if ((declarationId > 0 && Token::Match(tok, "strcpy|strcat ( %varid% , %str% )", declarationId)) ||
            (declarationId == 0 && Token::Match(tok, ("strcpy|strcat ( " + varnames + " , %str% )").c_str()))) {
            const std::size_t len = Token::getStrLength(tok->tokAt(varc + 4));
            if (total_size > 0 && len >= (unsigned int)total_size) {
                bufferOverrunError(tok, declarationId > 0 ? std::string() : varnames);
                continue;
            }
        } else if ((declarationId > 0 && Token::Match(tok, "strcpy|strcat ( %varid% , %var% )", declarationId)) ||
                   (declarationId == 0 && Token::Match(tok, ("strcpy|strcat ( " + varnames + " , %var% )").c_str()))) {
            const Variable *var = tok->tokAt(4)->variable();
            if (var && var->isArray() && var->dimensions().size() == 1) {
                const std::size_t len = (std::size_t)var->dimension(0);
                if (total_size > 0 && len > (unsigned int)total_size) {
                    if (_settings->inconclusive)
                        possibleBufferOverrunError(tok, tok->strAt(4), tok->strAt(2), tok->str() == "strcat");
                    continue;
                }
            }
        }

        // Detect few strcat() calls
        const std::string strcatPattern = declarationId > 0 ? std::string("strcat ( %varid% , %str% ) ;") : ("strcat ( " + varnames + " , %str% ) ;");
        if (Token::Match(tok, strcatPattern.c_str(), declarationId)) {
            std::size_t charactersAppend = 0;
            const Token *tok2 = tok;

            while (Token::Match(tok2, strcatPattern.c_str(), declarationId)) {
                charactersAppend += Token::getStrLength(tok2->tokAt(4 + varc));
                if (charactersAppend >= static_cast<std::size_t>(total_size)) {
                    bufferOverrunError(tok2);
                    break;
                }
                tok2 = tok2->tokAt(7 + varc);
            }
        }

        // sprintf..
        // TODO: change total_size to an unsigned value and remove the "&& total_size > 0" check.
        const std::string sprintfPattern = declarationId > 0 ? std::string("sprintf ( %varid% , %str% [,)]") : ("sprintf ( " + varnames + " , %str% [,)]");
        if (Token::Match(tok, sprintfPattern.c_str(), declarationId) && total_size > 0) {
            checkSprintfCall(tok, static_cast<unsigned int>(total_size));
        }

        // snprintf..
        const std::string snprintfPattern = declarationId > 0 ? std::string("snprintf ( %varid% , %num% ,") : ("snprintf ( " + varnames + " , %num% ,");
        if (Token::Match(tok, snprintfPattern.c_str(), declarationId)) {
            const MathLib::bigint n = MathLib::toLongNumber(tok->strAt(4 + varc));
            if (n > total_size)
                outOfBoundsError(tok->tokAt(4 + varc), "snprintf size", true, n, total_size);
        }

        // Check function call..
        if (Token::Match(tok, "%var% (") && total_size > 0) {
            // No varid => function calls are not handled
            if (declarationId == 0)
                continue;

            const ArrayInfo arrayInfo1(declarationId, varnames, total_size / size, size);
            const std::list<const Token *> callstack;
            checkFunctionCall(tok, arrayInfo1, callstack);
        }

        // undefined behaviour: result of pointer arithmetic is out of bounds
        else if (declarationId && Token::Match(tok, "= %varid% + %num% ;", declarationId)) {
            const MathLib::bigint index = MathLib::toLongNumber(tok->strAt(3));
            if (index > size && _settings->isEnabled("portability"))
                pointerOutOfBoundsError(tok->next(), "buffer");
            if (index >= size && Token::Match(tok->tokAt(-2), "[;{}] %varid% =", declarationId))
                pointerIsOutOfBounds = true;
        }

        else if (pointerIsOutOfBounds && Token::Match(tok, "[;{}=] * %varid% [;=]", declarationId)) {
            outOfBoundsError(tok->tokAt(2), tok->strAt(2), false, 0, 0);
        }
    }
}


void CheckBufferOverrun::checkScope(const Token *tok, const ArrayInfo &arrayInfo)
{
    const MathLib::bigint total_size = arrayInfo.num(0) * arrayInfo.element_size();

    const Token *scope_begin = tok->previous();
    assert(scope_begin != 0);

    for (const Token* const end = tok->scope()->classEnd; tok != end; tok = tok->next()) {
        // Skip array declarations
        if (Token::Match(tok, "[;{}] %type% *| %var% [") && tok->strAt(1) != "return") {
            tok = tok->tokAt(3);
            continue;
        }

        else if (Token::Match(tok, "%varid% [", arrayInfo.declarationId())) {
            // Look for errors first
            for (int warn = 0; warn == 0 || warn == 1; ++warn) {
                std::vector<ValueFlow::Value> indexes;
                unsigned int valuevarid = 0;
                for (const Token *tok2 = tok->next(); indexes.size() < arrayInfo.num().size() && Token::Match(tok2, "["); tok2 = tok2->link()->next()) {
                    if (!tok2->astOperand2()) {
                        indexes.clear();
                        break;
                    }
                    const ValueFlow::Value *value = tok2->astOperand2()->getMaxValue(warn == 1);
                    if (!value) {
                        indexes.clear();
                        break;
                    }
                    if (valuevarid == 0U)
                        valuevarid = value->varId;
                    if (value->varId > 0 && valuevarid != value->varId) {
                        indexes.clear();
                        break;
                    }
                    if (value->intvalue < 0) {
                        indexes.clear();
                        break;
                    }
                    indexes.push_back(*value);
                }
                if (indexes.size() == arrayInfo.num().size()) {
                    // Check if the indexes point outside the whole array..
                    // char a[10][10];
                    // a[0][20]  <-- ok.
                    // a[9][20]  <-- error.

                    // total number of elements of array..
                    MathLib::bigint totalElements = 1;

                    // total index..
                    MathLib::bigint totalIndex = 0;

                    // calculate the totalElements and totalIndex..
                    for (unsigned int i = 0; i < indexes.size(); ++i) {
                        std::size_t ri = indexes.size() - 1 - i;
                        totalIndex += indexes[ri].intvalue * totalElements;
                        totalElements *= arrayInfo.num(ri);
                    }

                    // totalElements == 0 => Unknown size
                    if (totalElements == 0)
                        continue;

                    const Token *tok2 = tok->previous();
                    while (tok2 && Token::Match(tok2->previous(), "%var% ."))
                        tok2 = tok2->tokAt(-2);

                    // just taking the address?
                    const bool addr(tok2 && (tok2->str() == "&" ||
                                             Token::simpleMatch(tok2->previous(), "& (")));

                    // taking address of 1 past end?
                    if (addr && totalIndex == totalElements)
                        continue;

                    // Is totalIndex in bounds?
                    if (totalIndex >= totalElements) {
                        arrayIndexOutOfBoundsError(tok, arrayInfo, indexes);
                        break;
                    }

                    // Is any array index out of bounds?
                    else {
                        // check each index for overflow
                        for (unsigned int i = 0; i < indexes.size(); ++i) {
                            if (indexes[i].intvalue >= arrayInfo.num(i)) {
                                // The access is still within the memory range for the array
                                // so it may be intentional.
                                if (_settings->inconclusive) {
                                    arrayIndexOutOfBoundsError(tok, arrayInfo, indexes);
                                    break; // only warn about the first one
                                }
                            }
                        }
                    }
                }
            }
        }

        // Loop..
        else if (Token::simpleMatch(tok, "for (")) {
            bool bailout = false;
            arrayIndexInForLoop(tok, arrayInfo);
            checkScopeForBody(tok, arrayInfo, bailout);
            if (bailout)
                break;
            continue;
        }


        // Check function call..
        if (Token::Match(tok, "%var% (")) {
            const std::list<const Token *> callstack;
            checkFunctionCall(tok, arrayInfo, callstack);
        }

        if (Token::Match(tok, "strncpy|memcpy|memmove ( %varid% , %str% , %num% )", arrayInfo.declarationId())) {
            const unsigned int num = (unsigned int)MathLib::toLongNumber(tok->strAt(6));
            if (Token::getStrLength(tok->tokAt(4)) >= (unsigned int)total_size && (unsigned int)total_size == num) {
                if (_settings->inconclusive)
                    bufferNotZeroTerminatedError(tok, tok->strAt(2), tok->str());
            }
        }

        if ((Token::Match(tok, "strncpy|strncat ( %varid% ,", arrayInfo.declarationId()) && Token::Match(tok->linkAt(1)->tokAt(-2), ", %num% )"))) {
            const Token* param3 = tok->linkAt(1)->previous();

            // check for strncpy which is not terminated
            if (tok->str() == "strncpy") {
                // strncpy takes entire variable length as input size
                unsigned int num = (unsigned int)MathLib::toLongNumber(param3->str());

                // this is currently 'inconclusive'. See TestBufferOverrun::terminateStrncpy3
                if (num >= total_size && _settings->isEnabled("warning") && _settings->inconclusive) {
                    const Token *tok2 = tok->next()->link()->next();
                    for (; tok2; tok2 = tok2->next()) {
                        if (tok2->varId() == tok->tokAt(2)->varId()) {
                            if (!Token::Match(tok2, "%varid% [ %any% ]  = 0 ;", tok->tokAt(2)->varId())) {
                                terminateStrncpyError(tok, tok->strAt(2));
                            }

                            break;
                        }
                    }
                }
            }

            // Dangerous usage of strncat..
            else if (tok->str() == "strncat") {
                const MathLib::bigint n = MathLib::toLongNumber(param3->str());
                if (n >= total_size)
                    strncatUsageError(tok);
            }

            // Dangerous usage of strncpy + strncat..
            if (Token::Match(param3->tokAt(2), "; strncat ( %varid% ,", arrayInfo.declarationId()) && Token::Match(param3->linkAt(4)->tokAt(-2), ", %num% )")) {
                const MathLib::bigint n = MathLib::toLongNumber(param3->str()) + MathLib::toLongNumber(param3->linkAt(4)->strAt(-1));
                if (n > total_size)
                    strncatUsageError(param3->tokAt(3));
            }
        }

        // Writing data into array..
        if (Token::Match(tok, "strcpy|strcat ( %varid% , %str% )", arrayInfo.declarationId())) {
            const std::size_t len = Token::getStrLength(tok->tokAt(4));
            if (total_size > 0 && len >= (unsigned int)total_size) {
                bufferOverrunError(tok, arrayInfo.varname());
                continue;
            }
        }

        // Detect few strcat() calls
        if (total_size > 0 && Token::Match(tok, "strcat ( %varid% , %str% ) ;", arrayInfo.declarationId())) {
            std::size_t charactersAppend = 0;
            const Token *tok2 = tok;

            while (tok2 && Token::Match(tok2, "strcat ( %varid% , %str% ) ;", arrayInfo.declarationId())) {
                charactersAppend += Token::getStrLength(tok2->tokAt(4));
                if (charactersAppend >= (unsigned int)total_size) {
                    bufferOverrunError(tok2, arrayInfo.varname());
                    break;
                }
                tok2 = tok2->tokAt(7);
            }
        }


        if (Token::Match(tok, "sprintf ( %varid% , %str% [,)]", arrayInfo.declarationId())) {
            checkSprintfCall(tok, total_size);
        }

        // snprintf..
        if (total_size > 0 && Token::Match(tok, "snprintf ( %varid% , %num% ,", arrayInfo.declarationId())) {
            const MathLib::bigint n = MathLib::toLongNumber(tok->strAt(4));
            if (n > total_size)
                outOfBoundsError(tok->tokAt(4), "snprintf size", true, n, total_size);
        }

        // readlink() / readlinkat() buffer usage
        if (_settings->standards.posix && Token::Match(tok, "readlink|readlinkat ("))
            checkReadlinkBufferUsage(tok, scope_begin, arrayInfo.declarationId(), total_size);

        // undefined behaviour: result of pointer arithmetic is out of bounds
        if (_settings->isEnabled("portability") && Token::Match(tok, "= %varid% + %num% ;", arrayInfo.declarationId())) {
            const MathLib::bigint index = MathLib::toLongNumber(tok->strAt(3));
            if (index < 0 || index > arrayInfo.num(0)) {
                pointerOutOfBoundsError(tok->next(), "array");
            }
        }
    }
}

//---------------------------------------------------------------------------
// Checking member variables of structs..
//---------------------------------------------------------------------------
bool CheckBufferOverrun::isArrayOfStruct(const Token* tok, int &position)
{
    if (Token::Match(tok->next(), "%var% [ %num% ] ")) {
        tok = tok->tokAt(4);
        int i = 1;
        for (;;) {
            if (Token::Match(tok->next(), "[ %num% ] ")) {
                i++;
                tok = tok->tokAt(4);
            } else
                break;
        }
        if (Token::Match(tok->next(),";")) {
            position = i;
            return true;
        }
    }
    return false;
}

void CheckBufferOverrun::checkReadlinkBufferUsage(const Token* ftok, const Token *scope_begin, const unsigned int varid, const MathLib::bigint total_size)
{
    const std::string funcname = ftok->str();

    const Token* bufParam = ftok->tokAt(2)->nextArgument();
    if (funcname == "readlinkat")
        bufParam = bufParam ? bufParam->nextArgument() : NULL;
    if (!Token::Match(bufParam, "%varid% , %num% )", varid))
        return;

    const MathLib::bigint n = MathLib::toLongNumber(bufParam->strAt(2));
    if (total_size > 0 && n > total_size)
        outOfBoundsError(bufParam, funcname + "() buf size", true, n, total_size);

    if (!_settings->inconclusive)
        return;

    // only writing a part of the buffer
    if (n < total_size)
        return;

    // readlink()/readlinkat() never terminates the buffer, check the end of the scope for buffer termination.
    bool found_termination = false;
    const Token *scope_end = scope_begin->link();
    for (const Token *tok2 = bufParam->tokAt(4); tok2 && tok2 != scope_end; tok2 = tok2->next()) {
        if (Token::Match(tok2, "%varid% [ %any% ] = 0 ;", bufParam->varId())) {
            found_termination = true;
            break;
        }
    }

    if (!found_termination) {
        bufferNotZeroTerminatedError(ftok, bufParam->str(), funcname);
    } else if (n == total_size) {
        possibleReadlinkBufferOverrunError(ftok, funcname, bufParam->str());
    }
}

//---------------------------------------------------------------------------
// Checking local variables in a scope
//---------------------------------------------------------------------------

void CheckBufferOverrun::checkGlobalAndLocalVariable()
{
    // check string literals
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%str% [ %num% ]")) {
            std::string str = tok->strValue();
            std::size_t index = (std::size_t)std::atoi(tok->strAt(2).c_str());
            if (index > str.length()) {
                bufferOverrunError(tok, tok->str());
            }
        }
    }

    // check all known fixed size arrays first by just looking them up
    const SymbolDatabase* symbolDatabase = _tokenizer->getSymbolDatabase();
    for (unsigned int i = 1; i <= _tokenizer->varIdCount(); i++) {
        const Variable *var = symbolDatabase->getVariableFromVarId(i);
        if (var && var->isArray() && var->dimension(0) > 0) {
            const ArrayInfo arrayInfo(var, _tokenizer, i);
            const Token *tok = var->nameToken();
            while (tok && tok->str() != ";") {
                if (tok->str() == "{") {
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

    // find all dynamically allocated arrays next
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];

        for (const Token *tok = scope->classStart; tok != scope->classEnd; tok = tok->next()) {
            // if the previous token exists, it must be either a variable name or "[;{}]"
            if (tok->previous() && (!tok->previous()->isName() && !Token::Match(tok->previous(), "[;{}]")))
                continue;

            // size : Max array index
            MathLib::bigint size = 0;

            // type : The type of a array element
            std::string type;

            // varid : The variable id for the array
            const Variable *var = 0;

            // nextTok : number of tokens used in variable declaration - used to skip to next statement.
            int nextTok = 0;

            _errorLogger->reportProgress(_tokenizer->getSourceFilePath(),
                                         "Check (BufferOverrun::checkGlobalAndLocalVariable)",
                                         tok->progressValue());

            if (Token::Match(tok, "[*;{}] %var% = new %type% [ %num% ]")) {
                size = MathLib::toLongNumber(tok->strAt(6));
                type = tok->strAt(4);
                var = tok->next()->variable();
                nextTok = 8;
                if (size < 0) {
                    negativeMemoryAllocationSizeError(tok->next()->next());
                }
            } else if (Token::Match(tok, "[*;{}] %var% = new %type% ( %num% )")) {
                size = 1;
                type = tok->strAt(4);
                var = tok->next()->variable();
                nextTok = 8;
            } else if (Token::Match(tok, "[;{}] %var% = %str% ;") &&
                       tok->next()->varId() > 0 &&
                       NULL != Token::findmatch(_tokenizer->tokens(), "[;{}] const| %type% * %varid% ;", tok->next()->varId())) {
                size = 1 + int(tok->tokAt(3)->strValue().size());
                type = "char";
                var = tok->next()->variable();
                nextTok = 4;
            } else if (Token::Match(tok, "[*;{}] %var% = malloc|alloca ( %num% ) ;")) {
                size = MathLib::toLongNumber(tok->strAt(5));
                type = "char";   // minimum type, typesize=1
                var = tok->next()->variable();
                nextTok = 7;

                if (size < 0) {
                    negativeMemoryAllocationSizeError(tok->next()->next());
                }

                /** @todo false negatives: this may be too conservative */
                if (!var || var->typeEndToken()->str() != "*" || var->typeStartToken()->next() != var->typeEndToken())
                    continue;

                // get name of variable
                type = var->typeStartToken()->str();

                // malloc() gets count of bytes and not count of
                // elements, so we should calculate count of elements
                // manually
                const unsigned int sizeOfType = _tokenizer->sizeOfType(var->typeStartToken());
                if (sizeOfType > 0) {
                    size /= static_cast<int>(sizeOfType);
                }
                if (size < 0) {
                    negativeMemoryAllocationSizeError(tok->next()->next());
                }
            } else {
                continue;
            }

            if (var == 0)
                continue;

            Token sizeTok(0);
            sizeTok.str(type);
            const MathLib::bigint total_size = size * static_cast<int>(_tokenizer->sizeOfType(&sizeTok));
            if (total_size == 0)
                continue;

            std::vector<std::string> v;
            ArrayInfo temp(var->declarationId(), tok->next()->str(), total_size / size, size);
            checkScope(tok->tokAt(nextTok), v, temp);
        }
    }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checking member variables of structs..
//---------------------------------------------------------------------------

void CheckBufferOverrun::checkStructVariable()
{
    const SymbolDatabase * symbolDatabase = _tokenizer->getSymbolDatabase();

    // find every class and struct
    const std::size_t classes = symbolDatabase->classAndStructScopes.size();
    for (std::size_t i = 0; i < classes; ++i) {
        const Scope * scope = symbolDatabase->classAndStructScopes[i];

        // check all variables to see if they are arrays
        std::list<Variable>::const_iterator var;
        for (var = scope->varlist.begin(); var != scope->varlist.end(); ++var) {
            if (var->isArray()) {
                // create ArrayInfo from the array variable
                ArrayInfo arrayInfo(&*var, _tokenizer);

                // find every function
                const std::size_t functions = symbolDatabase->functionScopes.size();
                for (std::size_t j = 0; j < functions; ++j) {
                    const Scope * func_scope = symbolDatabase->functionScopes[j];

                    // If struct is declared in a function then check
                    // if scope_func matches
                    if (scope->nestedIn->type == Scope::eFunction &&
                        scope->nestedIn != &*func_scope) {
                        continue;
                    }

                    // check for member variables
                    if (func_scope->functionOf == &*scope) {
                        // only check non-empty function
                        if (func_scope->classStart->next() != func_scope->classEnd) {
                            // start checking after the {
                            const Token *tok = func_scope->classStart->next();
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
                    for (const Token *tok3 = func_scope->classDef; tok3 && tok3 != func_scope->classEnd; tok3 = tok3->next()) {
                        // search for the class/struct name
                        if (tok3->str() != scope->className)
                            continue;

                        // find all array variables
                        int posOfSemicolon = -1;

                        // Declare variable: Fred fred1;
                        if (Token::Match(tok3->next(), "%var% ;"))
                            varname[0] = tok3->strAt(1);

                        else if (isArrayOfStruct(tok3,posOfSemicolon)) {
                            varname[0] = tok3->strAt(1);

                            int pos = 2;
                            for (int k = 0 ; k < posOfSemicolon; k++) {
                                for (int index = pos; index < (pos + 3); index++)
                                    tok3->strAt(index);
                                pos += 3;
                            }
                        }

                        // Declare pointer or reference: Fred *fred1
                        else if (Token::Match(tok3->next(), "*|& %var% [,);=]"))
                            varname[0] = tok3->strAt(2);

                        else
                            continue;

                        // check for variable sized structure
                        if (scope->type == Scope::eStruct && var->isPublic()) {
                            // last member of a struct with array size of 0 or 1 could be a variable sized structure
                            if (var->dimensions().size() == 1 && var->dimension(0) < 2 &&
                                var->index() == (scope->varlist.size() - 1)) {
                                // dynamically allocated so could be variable sized structure
                                if (tok3->next()->str() == "*") {
                                    // check for allocation
                                    if ((Token::Match(tok3->tokAt(3), "; %var% = malloc ( %num% ) ;") ||
                                         (Token::Match(tok3->tokAt(3), "; %var% = (") &&
                                          Token::Match(tok3->linkAt(6), ") malloc ( %num% ) ;"))) &&
                                        (tok3->strAt(4) == tok3->strAt(2))) {
                                        MathLib::bigint size;

                                        // find size of allocation
                                        if (tok3->strAt(3) == "(") // has cast
                                            size = MathLib::toLongNumber(tok3->linkAt(6)->strAt(3));
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
                                        if (size != 100) { // magic number for size of struct
                                            // check if a real size was specified and give up
                                            // malloc(10) rather than malloc(sizeof(struct))
                                            if (size < 100 || arrayInfo.element_size() == 0)
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
                        while (tok3 && tok3 != func_scope->classEnd) {
                            // End of statement.
                            if (tok3->str() == ";") {
                                CheckTok = tok3;
                                break;
                            }

                            // End of function declaration..
                            if (Token::simpleMatch(tok3, ") ;"))
                                break;

                            // Function implementation..
                            if (Token::simpleMatch(tok3, ") {")) {
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
                        temp.declarationId(0); // do variable lookup by variable and member names rather than varid
                        std::string varnames; // use class and member name for messages
                        for (unsigned int k = 0; k < varname.size(); ++k)
                            varnames += (k == 0 ? "" : ".") + varname[k];

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
    for (std::string::size_type i = 0; i < input_string.length(); ++i) {
        if (input_string[i] == '\\') {
            if (input_string[i+1] == '0')
                break;

            ++input_string_size;
            ++i;
            continue;
        }

        if (percentCharFound) {
            switch (input_string[i]) {
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
                if (paramIter != parameters.end() && *paramIter && (*paramIter)->type() != Token::eString)
                    parameterLength = (*paramIter)->str().length();

                handleNextParameter = true;
                break;
            case 's':
                if (paramIter != parameters.end() && *paramIter && (*paramIter)->type() == Token::eString)
                    parameterLength = Token::getStrLength(*paramIter);

                handleNextParameter = true;
                break;
            }
        }

        if (input_string[i] == '%')
            percentCharFound = !percentCharFound;
        else if (percentCharFound) {
            digits_string.append(1, input_string[i]);
        }

        if (!percentCharFound)
            input_string_size++;

        if (handleNextParameter) {
            unsigned int tempDigits = static_cast<unsigned int>(std::abs(std::atoi(digits_string.c_str())));
            if (i_d_x_f_found)
                tempDigits = std::max(static_cast<unsigned int>(tempDigits), 1U);

            if (digits_string.find('.') != std::string::npos) {
                const std::string endStr = digits_string.substr(digits_string.find('.') + 1);
                unsigned int maxLen = std::max(static_cast<unsigned int>(std::abs(std::atoi(endStr.c_str()))), 1U);

                if (input_string[i] == 's') {
                    // For strings, the length after the dot "%.2s" will limit
                    // the length of the string.
                    if (parameterLength > maxLen)
                        parameterLength = maxLen;
                } else {
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

    std::list<const Token*> parameters;
    const Token* vaArg = tok->tokAt(2)->nextArgument()->nextArgument();
    while (vaArg) {
        if (Token::Match(vaArg->next(), "[,)]")) {
            if (vaArg->type() == Token::eString)
                parameters.push_back(vaArg);

            else if (vaArg->isNumber())
                parameters.push_back(vaArg);

            else
                parameters.push_back(0);
        } else // Parameter is more complex than just a value or variable. Ignore it for now and skip to next token.
            parameters.push_back(0);

        vaArg = vaArg->nextArgument();
    }

    MathLib::bigint len = countSprintfLength(tok->tokAt(2)->nextArgument()->strValue(), parameters);
    if (len > size) {
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
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart->next(); tok && tok != scope->classEnd; tok = tok->next()) {
            unsigned int dstVarId;
            unsigned int srcVarId;

            // Look for allocation of a buffer based on the size of a string
            if (Token::Match(tok, "%var% = malloc|g_malloc|g_try_malloc ( strlen ( %var% ) )")) {
                dstVarId = tok->varId();
                srcVarId = tok->tokAt(6)->varId();
                tok      = tok->tokAt(8);
            } else if (Token::Match(tok, "%var% = new char [ strlen ( %var% ) ]")) {
                dstVarId = tok->varId();
                srcVarId = tok->tokAt(7)->varId();
                tok      = tok->tokAt(9);
            } else if (Token::Match(tok, "%var% = realloc|g_realloc|g_try_realloc ( %var% , strlen ( %var% ) )")) {
                dstVarId = tok->varId();
                srcVarId = tok->tokAt(8)->varId();
                tok      = tok->tokAt(10);
            } else
                continue;

            // To avoid false positives and added complexity, we will only look for
            // improper usage of the buffer within the block that it was allocated
            for (const Token* const end = tok->scope()->classEnd; tok && tok->next() && tok != end; tok = tok->next()) {
                // If the buffers are modified, we can't be sure of their sizes
                if (tok->varId() == srcVarId || tok->varId() == dstVarId)
                    break;

                if (Token::Match(tok, "strcpy ( %varid% , %var% )", dstVarId) &&
                    tok->tokAt(4)->varId() == srcVarId) {
                    bufferOverrunError(tok);
                } else if (Token::Match(tok, "sprintf ( %varid% , %str% , %var% )", dstVarId) &&
                           tok->tokAt(6)->varId() == srcVarId &&
                           tok->strAt(4).find("%s") != std::string::npos) {
                    bufferOverrunError(tok);
                }
            }
            if (!tok)
                return;
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
    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();

    std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * scope = symbolDatabase->functionScopes[i];
        Function * j = scope->function;
        if (j) {
            const Token* tok = j->token;

            // Get the name of the argv variable
            unsigned int varid = 0;
            if (Token::Match(tok, "main ( int %var% , char * %var% [ ] ,|)")) {
                varid = tok->tokAt(7)->varId();

            } else if (Token::Match(tok, "main ( int %var% , char * * %var% ,|)")) {
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
            for (const Token* end = tok->link(); tok != end; tok = tok->next()) {
                // If argv is modified or tested, its size may be being limited properly
                if (tok->varId() == varid)
                    break;

                // Match common patterns that can result in a buffer overrun
                // e.g. strcpy(buffer, argv[0])
                if (Token::Match(tok, "strcpy|strcat ( %var% , * %varid%", varid) ||
                    Token::Match(tok, "strcpy|strcat ( %var% , %varid% [", varid)) {
                    cmdLineArgsError(tok);
                } else if (Token::Match(tok, "sprintf ( %var% , %str% , %varid% [", varid) &&
                           tok->strAt(4).find("%s") != std::string::npos) {
                    cmdLineArgsError(tok);
                } else if (Token::Match(tok, "sprintf ( %var% , %str% , * %varid%", varid) &&
                           tok->strAt(4).find("%s") != std::string::npos) {
                    cmdLineArgsError(tok);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------


void CheckBufferOverrun::negativeIndexError(const Token *tok, MathLib::bigint index)
{
    std::ostringstream ostr;
    ostr << "Array index " << index << " is out of bounds.";
    reportError(tok, Severity::error, "negativeIndex", ostr.str());
}

void CheckBufferOverrun::negativeIndex()
{
    const char pattern[] = "[ %num% ]";
    for (const Token *tok = Token::findmatch(_tokenizer->tokens(), pattern); tok; tok = Token::findmatch(tok->next(),pattern)) {
        const MathLib::bigint index = MathLib::toLongNumber(tok->next()->str());
        if (index < 0) {
            // Negative index. Check if it's an array.
            const Token *tok2 = tok;
            while (tok2->strAt(-1) == "]")
                tok2 = tok2->previous()->link();

            if (tok2->previous() && tok2->previous()->varId()) {
                const Variable *var = tok2->previous()->variable();
                if (var && var->isArray())
                    negativeIndexError(tok, index);
            }
        }
    }
}


/// @addtogroup Checks
/// @{



CheckBufferOverrun::ArrayInfo::ArrayInfo()
    : _element_size(0), _declarationId(0)
{
}

CheckBufferOverrun::ArrayInfo::ArrayInfo(const Variable *var, const Tokenizer *tokenizer, const unsigned int forcedeclid)
    : _varname(var->name()), _declarationId((forcedeclid == 0U) ? var->declarationId() : forcedeclid)
{
    for (std::size_t i = 0; i < var->dimensions().size(); i++)
        _num.push_back(var->dimension(i));
    if (var->typeEndToken()->str() == "*")
        _element_size = tokenizer->sizeOfType(var->typeEndToken());
    else if (var->typeStartToken()->str() == "struct")
        _element_size = 100;
    else
        _element_size = tokenizer->sizeOfType(var->typeEndToken());
}

/**
 * Create array info with specified data
 * The intention is that this is only a temporary solution.. all
 * checking should be based on ArrayInfo from the start and then
 * this will not be needed as the declare can be used instead.
 */
CheckBufferOverrun::ArrayInfo::ArrayInfo(unsigned int id, const std::string &name, MathLib::bigint size1, MathLib::bigint n)
    : _varname(name), _element_size(size1), _declarationId(id)
{
    _num.push_back(n);
}

CheckBufferOverrun::ArrayInfo CheckBufferOverrun::ArrayInfo::limit(MathLib::bigint value) const
{
    MathLib::bigint uvalue = std::max(MathLib::bigint(0), value);
    MathLib::bigint n = 1;
    for (unsigned int i = 0; i < _num.size(); ++i)
        n *= _num[i];
    if (uvalue > n)
        n = uvalue;
    return ArrayInfo(_declarationId, _varname, _element_size, n - uvalue);
}



void CheckBufferOverrun::arrayIndexThenCheck()
{
    if (!_settings->isEnabled("style"))
        return;

    const SymbolDatabase * const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * const scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "%var% [ %var% ]")) {
                const std::string& indexName(tok->strAt(2));

                // skip array index..
                tok = tok->tokAt(4);
                while (tok && tok->str() == "[")
                    tok = tok->link()->next();

                // syntax error
                if (!tok)
                    return;

                // skip comparison
                if (tok->type() == Token::eComparisonOp)
                    tok = tok->tokAt(2);

                // skip close parenthesis
                if (tok->str() == ")") {
                    tok = tok->next();
                }

                // check if array index is ok
                // statement can be closed in parentheses, so "(| " is using
                if (Token::Match(tok, ("&& (| " + indexName + " <|<=").c_str()))
                    arrayIndexThenCheckError(tok, indexName);
            }
        }
    }
}

void CheckBufferOverrun::arrayIndexThenCheckError(const Token *tok, const std::string &indexName)
{
    reportError(tok, Severity::style, "arrayIndexThenCheck",
                "Array index '" + indexName + "' is used before limits check.\n"
                "Defensive programming: The variable '" + indexName + "' is used as an array index before it "
                "is checked that is within limits. This can mean that the array might be accessed out of bounds. "
                "Reorder conditions such as '(a[i] && i < 10)' to '(i < 10 && a[i])'. That way the array will "
                "not be accessed if the index is out of limits.");
}

// -------------------------------------------------------------------------------------
// Check the second and the third parameter of the POSIX function write and validate
// their values.
// The parameters have the following meaning:
// - 1.parameter: file descripter (not required for this check)
// - 2.parameter: is a null terminated character string of the content to write.
// - 3.parameter: the number of bytes to write.
//
// This check is triggered if the size of the string ( 2. parameter) is lower than
// the number of bytes provided at the 3. parameter.
//
// References:
//  - http://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html
//  - http://gd.tuwien.ac.at/languages/c/programming-bbrown/c_075.htm
//  - http://codewiki.wikidot.com/c:system-calls:write
// -------------------------------------------------------------------------------------
void CheckBufferOverrun::writeOutsideBufferSize()
{
    if (!_settings->standards.posix)
        return;

    const SymbolDatabase* const symbolDatabase = _tokenizer->getSymbolDatabase();
    const std::size_t functions = symbolDatabase->functionScopes.size();
    for (std::size_t i = 0; i < functions; ++i) {
        const Scope * const scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
            if (Token::Match(tok, "pwrite|write (") && Token::Match(tok->tokAt(2)->nextArgument(), "%str% , %num%")) {
                const std::string & functionName(tok->str());
                tok = tok->tokAt(2)->nextArgument(); // set tokenptr to %str% parameter
                const std::size_t stringLength = Token::getStrLength(tok)+1; // zero-terminated string!
                tok = tok->tokAt(2); // set tokenptr to %num% parameter
                const MathLib::bigint writeLength = MathLib::toLongNumber(tok->str());
                if (static_cast<std::size_t>(writeLength) > stringLength)
                    writeOutsideBufferSizeError(tok, stringLength, writeLength, functionName);
            }
        }
    }
}

void CheckBufferOverrun::writeOutsideBufferSizeError(const Token *tok, const std::size_t stringLength, const MathLib::bigint writeLength, const std::string &strFunctionName)
{
    reportError(tok, Severity::error, "writeOutsideBufferSize",
                "Writing " + MathLib::toString(writeLength-stringLength) + " bytes outside buffer size.\n"
                "The number of bytes to write (" + MathLib::toString(writeLength) + " bytes) are bigger than the source buffer (" +MathLib::toString(stringLength)+ " bytes)."
                " Please check the second and the third parameter of the function '"+strFunctionName+"'.");
}
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------

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
#ifndef CheckBufferOverrunH
#define CheckBufferOverrunH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"
#include "mathlib.h"
#include <list>
#include <vector>
#include <string>

class ErrorLogger;
class Token;
class Tokenizer;
class Variable;

/// @addtogroup Checks
/// @{

/**
 * @brief buffer overruns and array index out of bounds
 *
 * Buffer overrun and array index out of bounds are pretty much the same.
 * But I generally use 'array index' if the code contains []. And the given
 * index is out of bounds.
 * I generally use 'buffer overrun' if you for example call a strcpy or
 * other function and pass a buffer and reads or writes too much data.
 */
class CheckBufferOverrun : public Check
{
public:

    /** This constructor is used when registering the CheckClass */
    CheckBufferOverrun() : Check(myName())
    { }

    /** This constructor is used when running checks. */
    CheckBufferOverrun(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckBufferOverrun checkBufferOverrun(tokenizer, settings, errorLogger);
        checkBufferOverrun.bufferOverrun();
        checkBufferOverrun.negativeIndex();
        checkBufferOverrun.arrayIndexThenCheck();

        /** ExecutionPath checking.. */
        checkBufferOverrun.executionPaths();
    }

    /** @brief %Check for buffer overruns */
    void bufferOverrun();

    /** @brief Using array index before bounds check */
    void arrayIndexThenCheck();

    /** @brief %Check for buffer overruns by inspecting execution paths */
    void executionPaths();

    /**
     * @brief Get minimum length of format string result
     * @param input_string format string
     * @param parameters given parameters to sprintf
     * @return minimum length of resulting string
     */
    static MathLib::bigint countSprintfLength(const std::string &input_string, const std::list<const Token*> &parameters);

    /**
     * @brief %Check code that matches: "sprintf ( %varid% , %str% [,)]" when varid is not 0,
     * and report found errors.
     * @param tok The "sprintf" token.
     * @param size The size of the buffer where sprintf is writing.
     */
    void checkSprintfCall(const Token *tok, const MathLib::bigint size);

    /** Check for buffer overruns - locate struct variables and check them with the .._CheckScope function */
    void checkStructVariable();

    /** Check for buffer overruns - locate global variables and local function variables and check them with the checkScope function */
    void checkGlobalAndLocalVariable();

    /** Check for buffer overruns due to allocating strlen(src) bytes instead of (strlen(src)+1) bytes before copying a string */
    void checkBufferAllocatedWithStrlen();

    /** Check for buffer overruns due to copying command-line args to fixed-sized buffers without bounds checking */
    void checkInsecureCmdLineArgs();

    /** Check for negative index */
    void negativeIndex();

    /** Check for buffer overruns */
    void checkScope(const Token *tok, const std::vector<std::string> &varname, const MathLib::bigint size, const MathLib::bigint total_size, unsigned int varid);

    /** Information about N-dimensional array */
    class ArrayInfo
    {
    private:
        /** number of elements of array */
        std::vector<MathLib::bigint> _num;

        /** size of each element in array */
        MathLib::bigint _element_size;

        /** variable id */
        unsigned int _varid;

        /** full name of variable as pattern */
        std::string _varname;

    public:
        ArrayInfo();
        ArrayInfo(const ArrayInfo &);
        ArrayInfo(const Variable *var, const Tokenizer *tokenizer);
        ArrayInfo & operator=(const ArrayInfo &ai);

        /**
         * Create array info with specified data
         * The intention is that this is only a temporary solution.. all
         * checking should be based on ArrayInfo from the start and then
         * this will not be needed as the declare can be used instead.
         */
        ArrayInfo(unsigned int id, const std::string &name, MathLib::bigint size1, MathLib::bigint n);

        /** Create a copy ArrayInfo where the number of elements have been limited by a value */
        ArrayInfo limit(MathLib::bigint value) const;

        /**
         * Declare array - set info
         * \param tok first token in array declaration
         * \param tokenizer The tokenizer (for type size)
         * \return success => true
         */
        bool declare(const Token *tok, const Tokenizer &tokenizer);

        /** array sizes */
        const std::vector<MathLib::bigint> &num() const
        {
            return _num;
        }

        /** array size */
        MathLib::bigint num(size_t index) const
        {
            return _num[index];
        }

        /** size of each element */
        MathLib::bigint element_size() const
        {
            return _element_size;
        }

        /** Variable name */
        unsigned int varid() const
        {
            return _varid;
        }

        /** Variable name */
        const std::string &varname() const
        {
            return _varname;
        }
    };

    /** Check for buffer overruns (based on ArrayInfo) */
    void checkScope(const Token *tok, const ArrayInfo &arrayInfo);

    /** Check scope helper function - parse for body */
    void checkScopeForBody(const Token *tok, const ArrayInfo &arrayInfo, bool &bailout);

    /** Helper function used when parsing for-loops */
    void parse_for_body(const Token *tok2, const ArrayInfo &arrayInfo, const std::string &strindex, bool condition_out_of_bounds, unsigned int counter_varid, const std::string &min_counter_value, const std::string &max_counter_value);

    /**
     * Helper function for checkFunctionCall - check a function parameter
     * \param tok token for the function name
     * \param par on what parameter is the array used
     * \param arrayInfo the array information
     */
    void checkFunctionParameter(const Token &tok, const unsigned int par, const ArrayInfo &arrayInfo);

    /**
     * Helper function that checks if the array is used and if so calls the checkFunctionCall
     * @param tok token that matches "%var% ("
     * @param arrayInfo the array information
     */
    void checkFunctionCall(const Token *tok, const ArrayInfo &arrayInfo);

    void arrayIndexOutOfBoundsError(const Token *tok, MathLib::bigint size, MathLib::bigint index);
    void arrayIndexOutOfBoundsError(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index);
    void arrayIndexOutOfBoundsError(const std::list<const Token *> &callstack, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index);
    void bufferOverrunError(const Token *tok, const std::string &varnames = "");
    void strncatUsageError(const Token *tok);
    void outOfBoundsError(const Token *tok, const std::string &what);
    void sizeArgumentAsCharError(const Token *tok);
    void terminateStrncpyError(const Token *tok, const std::string &varname);
    void bufferNotZeroTerminatedError(const Token *tok, const std::string &varname, const std::string &function);
    void negativeIndexError(const Token *tok, MathLib::bigint index);
    void cmdLineArgsError(const Token *tok);
    void pointerOutOfBoundsError(const Token *tok, const std::string &object);	// UB when result of calculation is out of bounds
    void arrayIndexThenCheckError(const Token *tok, const std::string &indexName);
    void possibleBufferOverrunError(const Token *tok, const std::string &src, const std::string &dst, bool cat);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
    {
        CheckBufferOverrun c(0, settings, errorLogger);
        c.arrayIndexOutOfBoundsError(0, 2, 2);
        c.bufferOverrunError(0, std::string("buffer"));
        c.strncatUsageError(0);
        c.outOfBoundsError(0, "index");
        c.sizeArgumentAsCharError(0);
        c.terminateStrncpyError(0, "buffer");
        c.bufferNotZeroTerminatedError(0, "buffer", "strncpy");
        c.negativeIndexError(0, -1);
        c.cmdLineArgsError(0);
        c.pointerOutOfBoundsError(0, "array");
        c.arrayIndexThenCheckError(0, "index");
        c.possibleBufferOverrunError(0, "source", "destination", false);
    }

    std::string myName() const
    {
        return "Bounds checking";
    }

    std::string classInfo() const
    {
        return "out of bounds checking";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif




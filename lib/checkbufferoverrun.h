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
#ifndef CheckBufferOverrunH
#define CheckBufferOverrunH
//---------------------------------------------------------------------------

#include "check.h"
#include "settings.h"
#include <list>
#include <vector>
#include <string>

class ErrorLogger;
class Token;
class Tokenizer;

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
    CheckBufferOverrun() : Check()
    { }

    /** This constructor is used when running checks. */
    CheckBufferOverrun(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(tokenizer, settings, errorLogger)
    { }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
    {
        CheckBufferOverrun checkBufferOverrun(tokenizer, settings, errorLogger);
        checkBufferOverrun.bufferOverrun();
        checkBufferOverrun.negativeIndex();

        /** ExecutionPath checking.. */
        checkBufferOverrun.executionPaths();
    }

    /** @brief %Check for buffer overruns */
    void bufferOverrun();

    /** @brief %Check for buffer overruns by inspecting execution paths */
    void executionPaths();

    /**
     * @brief Get minimum length of format string result
     * @param input_string format string
     * @param parameters given parameters to sprintf
     * @return minimum length of resulting string
     */
    static unsigned int countSprintfLength(const std::string &input_string, const std::list<const Token*> &parameters);

    /**
     * @brief %Check code that matches: "sprintf ( %varid% , %str% [,)]" when varid is not 0,
     * and report found errors.
     * @param tok The "sprintf" token.
     * @param size The size of the buffer where sprintf is writing.
     */
    void checkSprintfCall(const Token *tok, const unsigned int size);

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
    void checkScope(const Token *tok, const std::vector<std::string> &varname, const int size, const int total_size, unsigned int varid);

    /** Information about N-dimensional array */
    class ArrayInfo
    {
    private:
        /** number of elements of array */
        std::vector<unsigned int> _num;

        /** size of each element in array */
        unsigned int _element_size;

        /** variable id */
        unsigned int _varid;

        /** full name of variable as pattern */
        std::string _varname;

    public:
        ArrayInfo();
        ArrayInfo(const ArrayInfo &);
        ArrayInfo & operator=(const ArrayInfo &ai);

        /**
         * Create array info with specified data
         * The intention is that this is only a temporary solution.. all
         * checking should be based on ArrayInfo from the start and then
         * this will not be needed as the declare can be used instead.
         */
        ArrayInfo(unsigned int id, const std::string &name, unsigned int size1, unsigned int n);

        /** Create a copy ArrayInfo where the number of elements have been limited by a value */
        ArrayInfo limit(long value) const;

        /**
         * Declare array - set info
         * \param tok first token in array declaration
         * \param tokenizer The tokenizer (for type size)
         * \return success => true
         */
        bool declare(const Token *tok, const Tokenizer &tokenizer);

        /** array size */
        const std::vector<unsigned int> &num;

        /** size of each element */
        const unsigned int &element_size;

        /** Variable name */
        const unsigned int &varid;

        /** Variable name */
        const std::string &varname;
    };

    /** Check for buffer overruns (based on ArrayInfo) */
    void checkScope(const Token *tok, const ArrayInfo &arrayInfo);


    /** Helper function used when parsing for-loops */
    void parse_for_body(const Token *tok2, const ArrayInfo &arrayInfo, const std::string &strindex, bool condition_out_of_bounds, unsigned int counter_varid, const std::string &min_counter_value, const std::string &max_counter_value);

    /**
     * Helper function for checkScope - check a function call
     * \param tok token for the function name
     * \param par on what parameter is the array used
     * \param arrayInfo the array information
     */
    void checkFunctionCall(const Token &tok, const unsigned int par, const ArrayInfo &arrayInfo);

    void arrayIndexOutOfBounds(const Token *tok, int size, int index);
    void arrayIndexOutOfBounds(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<unsigned int> &index);
    void arrayIndexOutOfBounds(const std::list<const Token *> &callstack, const ArrayInfo &arrayInfo, const std::vector<unsigned int> &index);
    void bufferOverrun(const Token *tok, const std::string &varnames = "");
    void strncatUsage(const Token *tok);
    void outOfBounds(const Token *tok, const std::string &what);
    void sizeArgumentAsChar(const Token *tok);
    void terminateStrncpyError(const Token *tok);
    void negativeIndexError(const Token *tok, long index);
    void cmdLineArgsError(const Token *tok);

    void getErrorMessages()
    {
        arrayIndexOutOfBounds(0, 2, 2);
        bufferOverrun(0, std::string("buffer"));
        strncatUsage(0);
        outOfBounds(0, "index");
        sizeArgumentAsChar(0);
        terminateStrncpyError(0);
        negativeIndexError(0, -1);
        cmdLineArgsError(0);
    }

    std::string name() const
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




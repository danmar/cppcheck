/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#ifndef checkbufferoverrunH
#define checkbufferoverrunH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include "mathlib.h"
#include <list>
#include <vector>
#include <string>

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
class CPPCHECKLIB CheckBufferOverrun : public Check {
public:

    /** This constructor is used when registering the CheckClass */
    CheckBufferOverrun() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckBufferOverrun(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckBufferOverrun checkBufferOverrun(tokenizer, settings, errorLogger);
        checkBufferOverrun.bufferOverrun();
        checkBufferOverrun.bufferOverrun2();
        checkBufferOverrun.arrayIndexThenCheck();
    }

    /** @brief %Check for buffer overruns */
    void bufferOverrun();

    /** @brief %Check for buffer overruns #2 (single pass, use ast and valueflow) */
    void bufferOverrun2();

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
    static MathLib::biguint countSprintfLength(const std::string &input_string, const std::list<const Token*> &parameters);

    /** Check for buffer overruns - locate struct variables and check them with the .._CheckScope function */
    void checkStructVariable();

    /** Check for buffer overruns - locate global variables and local function variables and check them with the checkScope function */
    void checkGlobalAndLocalVariable();

    /** Check for buffer overruns due to allocating strlen(src) bytes instead of (strlen(src)+1) bytes before copying a string */
    void checkBufferAllocatedWithStrlen();

    /** Check string argument buffer overruns */
    void checkStringArgument();

    /** Check for buffer overruns due to copying command-line args to fixed-sized buffers without bounds checking */
    void checkInsecureCmdLineArgs();

    /** Information about N-dimensional array */
    class CPPCHECKLIB ArrayInfo {
    private:
        /** number of elements of array */
        std::vector<MathLib::bigint> _num;

        /** full name of variable as pattern */
        std::string _varname;

        /** size of each element in array */
        MathLib::bigint _element_size;

        /** declaration id */
        unsigned int _declarationId;

    public:
        ArrayInfo();
        ArrayInfo(const Variable *var, const Tokenizer *tokenizer, const Library *library, const unsigned int forcedeclid = 0);

        /**
         * Create array info with specified data
         * The intention is that this is only a temporary solution.. all
         * checking should be based on ArrayInfo from the start and then
         * this will not be needed as the declare can be used instead.
         */
        ArrayInfo(unsigned int id, const std::string &name, MathLib::bigint size1, MathLib::bigint n);

        /** Create a copy ArrayInfo where the number of elements have been limited by a value */
        ArrayInfo limit(MathLib::bigint value) const;

        /** array sizes */
        const std::vector<MathLib::bigint> &num() const {
            return _num;
        }

        /** array size */
        MathLib::bigint num(std::size_t index) const {
            return _num[index];
        }
        void num(std::size_t index, MathLib::bigint number) {
            _num[index] = number;
        }

        /** size of each element */
        MathLib::bigint element_size() const {
            return _element_size;
        }

        /** Variable name */
        unsigned int declarationId() const {
            return _declarationId;
        }
        void declarationId(unsigned int id) {
            _declarationId = id;
        }

        /** Variable name */
        const std::string &varname() const {
            return _varname;
        }
        void varname(const std::string &name) {
            _varname = name;
        }
    };

    /** Check for buffer overruns (based on ArrayInfo) */
    void checkScope(const Token *tok, const ArrayInfo &arrayInfo);

    /** Check for buffer overruns */
    void checkScope(const Token *tok, const std::vector<std::string> &varname, const ArrayInfo &arrayInfo);

    /**
     * Helper function for checkFunctionCall - check a function parameter
     * \param tok token for the function name
     * \param par on what parameter is the array used
     * \param arrayInfo the array information
     * \param callstack call stack. This is used to prevent recursion and to provide better error messages. Pass a empty list from checkScope etc.
     */
    void checkFunctionParameter(const Token &tok, const unsigned int par, const ArrayInfo &arrayInfo, const std::list<const Token *>& callstack);

    /**
     * Helper function that checks if the array is used and if so calls the checkFunctionCall
     * @param tok token that matches "%name% ("
     * @param arrayInfo the array information
     * \param callstack call stack. This is used to prevent recursion and to provide better error messages. Pass a empty list from checkScope etc.
     */
    void checkFunctionCall(const Token *tok, const ArrayInfo &arrayInfo, std::list<const Token *> callstack);

    void arrayIndexOutOfBoundsError(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index);
    void arrayIndexOutOfBoundsError(const Token *tok, const ArrayInfo &arrayInfo, const std::vector<ValueFlow::Value> &index);

    /* data for multifile checking */
    class MyFileInfo : public Check::FileInfo {
    public:
        struct ArrayUsage {
            MathLib::bigint   index;
            std::string       fileName;
            unsigned int      linenr;
        };

        /* key:arrayName */
        std::map<std::string, struct ArrayUsage> arrayUsage;

        /* key:arrayName, data:arraySize */
        std::map<std::string, MathLib::bigint>  arraySize;
    };

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const;

    /** @brief Analyse all file infos for all TU */
    void analyseWholeProgram(const std::list<Check::FileInfo*> &fileInfo, ErrorLogger &errorLogger);

private:

    static bool isArrayOfStruct(const Token* tok, int &position);
    void arrayIndexOutOfBoundsError(const std::list<const Token *> &callstack, const ArrayInfo &arrayInfo, const std::vector<MathLib::bigint> &index);
    void bufferOverrunError(const Token *tok, const std::string &varnames = emptyString);
    void bufferOverrunError(const std::list<const Token *> &callstack, const std::string &varnames = emptyString);
    void strncatUsageError(const Token *tok);
    void negativeMemoryAllocationSizeError(const Token *tok); // provide a negative value to memory allocation function
    void outOfBoundsError(const Token *tok, const std::string &what, const bool show_size_info, const MathLib::bigint &supplied_size, const MathLib::bigint &actual_size);
    void sizeArgumentAsCharError(const Token *tok);
    void terminateStrncpyError(const Token *tok, const std::string &varname);
    void bufferNotZeroTerminatedError(const Token *tok, const std::string &varname, const std::string &function);
    void negativeIndexError(const Token *tok, MathLib::bigint index);
    void negativeIndexError(const Token *tok, const ValueFlow::Value &index);
    void cmdLineArgsError(const Token *tok);
    void pointerOutOfBoundsError(const Token *tok, const Token *index=nullptr, const MathLib::bigint indexvalue=0);
    void arrayIndexThenCheckError(const Token *tok, const std::string &indexName);
    void possibleBufferOverrunError(const Token *tok, const std::string &src, const std::string &dst, bool cat);
    void argumentSizeError(const Token *tok, const std::string &functionName, const std::string &varname);

    void valueFlowCheckArrayIndex(const Token * const tok, const ArrayInfo &arrayInfo);

public:
    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckBufferOverrun c(0, settings, errorLogger);
        std::vector<MathLib::bigint> indexes;
        indexes.push_back(2);
        c.arrayIndexOutOfBoundsError(0, ArrayInfo(0, "array", 1, 2), indexes);
        c.bufferOverrunError(0, std::string("buffer"));
        c.strncatUsageError(0);
        c.outOfBoundsError(0, "index", true, 2, 1);
        c.sizeArgumentAsCharError(0);
        c.terminateStrncpyError(0, "buffer");
        c.bufferNotZeroTerminatedError(0, "buffer", "strncpy");
        c.negativeIndexError(0, -1);
        c.cmdLineArgsError(0);
        c.pointerOutOfBoundsError(nullptr, nullptr, 0);
        c.arrayIndexThenCheckError(0, "index");
        c.possibleBufferOverrunError(0, "source", "destination", false);
        c.argumentSizeError(0, "function", "array");
        c.negativeMemoryAllocationSizeError(0);
        c.reportError(nullptr, Severity::warning, "arrayIndexOutOfBoundsCond", "Array 'x[10]' accessed at index 20, which is out of bounds. Otherwise condition 'y==20' is redundant.");
    }
private:

    static std::string myName() {
        return "Bounds checking";
    }

    std::string classInfo() const {
        return "Out of bounds checking:\n"
               "- Array index out of bounds detection by value flow analysis\n"
               "- Dangerous usage of strncat()\n"
               "- char constant passed as size to function like memset()\n"
               "- strncpy() leaving string unterminated\n"
               "- Accessing array with negative index\n"
               "- Unsafe usage of main(argv, argc) arguments\n"
               "- Accessing array with index variable before checking its value\n"
               "- Check for large enough arrays being passed to functions\n"
               "- Allocating memory with a negative size\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkbufferoverrunH

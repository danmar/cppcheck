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
#ifndef checkioH
#define checkioH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

/// @addtogroup Checks
/// @{

/** @brief %Check input output operations. */
class CPPCHECKLIB CheckIO : public Check {
public:
    /** @brief This constructor is used when registering CheckIO */
    CheckIO() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckIO(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** @brief Run checks on the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckIO checkIO(tokenizer, settings, errorLogger);

        checkIO.checkWrongPrintfScanfArguments();
    }

    /** @brief Run checks on the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        CheckIO checkIO(tokenizer, settings, errorLogger);

        checkIO.checkCoutCerrMisusage();
        checkIO.checkFileUsage();
        checkIO.invalidScanf();
    }

    /** @brief %Check for missusage of std::cout */
    void checkCoutCerrMisusage();

    /** @brief %Check usage of files*/
    void checkFileUsage();

    /** @brief scanf can crash if width specifiers are not used */
    void invalidScanf();

    /** @brief %Checks type and number of arguments given to functions like printf or scanf*/
    void checkWrongPrintfScanfArguments();

private:
    class ArgumentInfo {
    public:
        ArgumentInfo(const Token *arg, const Settings *settings);
        ~ArgumentInfo();

        bool isArrayOrPointer() const;
        bool isComplexType() const;
        bool isKnownType() const;
        bool isStdVectorOrString();
        bool isStdContainer(const Token *tok);
        bool isLibraryType(const Settings *settings) const;

        const Variable *variableInfo;
        const Token *typeToken;
        const Function *functionInfo;
        bool element;
        bool _template;
        bool address;
        Token *tempToken;

    private:
        ArgumentInfo(const ArgumentInfo &); // not implemented
        ArgumentInfo operator = (const ArgumentInfo &); // not implemented
    };

    // Reporting errors..
    void coutCerrMisusageError(const Token* tok, const std::string& streamName);
    void fflushOnInputStreamError(const Token *tok, const std::string &varname);
    void ioWithoutPositioningError(const Token *tok);
    void readWriteOnlyFileError(const Token *tok);
    void writeReadOnlyFileError(const Token *tok);
    void useClosedFileError(const Token *tok);
    void seekOnAppendedFileError(const Token *tok);
    void invalidScanfError(const Token *tok, bool portability);
    void wrongPrintfScanfArgumentsError(const Token* tok,
                                        const std::string &function,
                                        unsigned int numFormat,
                                        unsigned int numFunction);
    void wrongPrintfScanfPosixParameterPositionError(const Token* tok, const std::string& functionName,
            unsigned int index, unsigned int numFunction);
    void invalidScanfArgTypeError_s(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidScanfArgTypeError_int(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo, bool isUnsigned);
    void invalidScanfArgTypeError_float(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_s(const Token* tok, unsigned int numFormat, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_n(const Token* tok, unsigned int numFormat, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_p(const Token* tok, unsigned int numFormat, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_int(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_uint(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_sint(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidPrintfArgTypeError_float(const Token* tok, unsigned int numFormat, const std::string& specifier, const ArgumentInfo* argInfo);
    void invalidLengthModifierError(const Token* tok, unsigned int numFormat, const std::string& modifier);
    void invalidScanfFormatWidthError(const Token* tok, unsigned int numFormat, int width, const Variable *var);
    static void argumentType(std::ostream & s, const ArgumentInfo * argInfo);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckIO c(0, settings, errorLogger);

        c.coutCerrMisusageError(0, "cout");
        c.fflushOnInputStreamError(0, "stdin");
        c.ioWithoutPositioningError(0);
        c.readWriteOnlyFileError(0);
        c.writeReadOnlyFileError(0);
        c.useClosedFileError(0);
        c.seekOnAppendedFileError(0);
        c.invalidScanfError(0, false);
        c.invalidScanfError(0, true);
        c.wrongPrintfScanfArgumentsError(0,"printf",3,2);
        c.invalidScanfArgTypeError_s(0, 1, "s", NULL);
        c.invalidScanfArgTypeError_int(0, 1, "d", NULL, false);
        c.invalidScanfArgTypeError_float(0, 1, "f", NULL);
        c.invalidPrintfArgTypeError_s(0, 1, NULL);
        c.invalidPrintfArgTypeError_n(0, 1, NULL);
        c.invalidPrintfArgTypeError_p(0, 1, NULL);
        c.invalidPrintfArgTypeError_int(0, 1, "X", NULL);
        c.invalidPrintfArgTypeError_uint(0, 1, "u", NULL);
        c.invalidPrintfArgTypeError_sint(0, 1, "i", NULL);
        c.invalidPrintfArgTypeError_float(0, 1, "f", NULL);
        c.invalidLengthModifierError(0, 1, "I");
        c.invalidScanfFormatWidthError(0, 10, 5, NULL);
        c.wrongPrintfScanfPosixParameterPositionError(0, "printf", 2, 1);
    }

    static std::string myName() {
        return "IO using format string";
    }

    std::string classInfo() const {
        return "Check format string input/output operations.\n"
               "- Bad usage of the function 'sprintf' (overlapping data)\n"
               "- Missing or wrong width specifiers in 'scanf' format string\n"
               "- Use a file that has been closed\n"
               "- File input/output without positioning results in undefined behaviour\n"
               "- Read to a file that has only been opened for writing (or vice versa)\n"
               "- Repositioning operation on a file opened in append mode\n"
               "- Using fflush() on an input stream\n"
               "- Invalid usage of output stream. For example: 'std::cout << std::cout;'\n"
               "- Wrong number of arguments given to 'printf' or 'scanf;'\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkioH

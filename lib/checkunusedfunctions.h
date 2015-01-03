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
#ifndef checkunusedfunctionsH
#define checkunusedfunctionsH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

/// @addtogroup Checks
/** @brief Check for functions never called */
/// @{

class CPPCHECKLIB CheckUnusedFunctions : public Check {
public:
    /** @brief This constructor is used when registering the CheckUnusedFunctions */
    CheckUnusedFunctions() : Check(myName()) {
    }

    /** @brief This constructor is used when running checks. */
    CheckUnusedFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    // Parse current tokens and determine..
    // * Check what functions are used
    // * What functions are declared
    void parseTokens(const Tokenizer &tokenizer, const char FileName[], const Settings *settings);

    void check(ErrorLogger * const errorLogger);

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const;

    /** @brief Analyse all file infos for all TU */
    void analyseWholeProgram(const std::list<Check::FileInfo*> &fileInfo, ErrorLogger &errorLogger);

    static CheckUnusedFunctions instance;

private:

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckUnusedFunctions c(0, settings, errorLogger);
        c.unusedFunctionError(errorLogger, "", 0, "funcName");
    }

    /**
     * Dummy implementation, just to provide error for --errorlist
     */
    static void unusedFunctionError(ErrorLogger * const errorLogger,
                                    const std::string &filename, unsigned int lineNumber,
                                    const std::string &funcname);

    /**
     * Dummy implementation, just to provide error for --errorlist
     */
    void runSimplifiedChecks(const Tokenizer *, const Settings *, ErrorLogger *) {

    }

    static std::string myName() {
        return "Unused functions";
    }

    std::string classInfo() const {
        return "Check for functions that are never called\n";
    }

    class CPPCHECKLIB FunctionUsage {
    public:
        FunctionUsage() : lineNumber(0), usedSameFile(false), usedOtherFile(false) {
        }

        std::string filename;
        unsigned int lineNumber;
        bool   usedSameFile;
        bool   usedOtherFile;
    };

    std::map<std::string, FunctionUsage> _functions;
};
/// @}
//---------------------------------------------------------------------------
#endif // checkunusedfunctionsH

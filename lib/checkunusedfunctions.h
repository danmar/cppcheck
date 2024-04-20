/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include <list>
#include <set>
#include <string>
#include <unordered_map>

class ErrorLogger;
class Function;
class Settings;
class Tokenizer;

/** @brief Check for functions never called */
/// @{

class CPPCHECKLIB CheckUnusedFunctions {
    friend class TestSuppressions;
    friend class TestSingleExecutorBase;
    friend class TestProcessExecutorBase;
    friend class TestThreadExecutorBase;
    friend class TestUnusedFunctions;

public:
    CheckUnusedFunctions() = default;

    // Parse current tokens and determine..
    // * Check what functions are used
    // * What functions are declared
    void parseTokens(const Tokenizer &tokenizer, const Settings &settings);

    std::string analyzerInfo() const;

    static void analyseWholeProgram(const Settings &settings, ErrorLogger& errorLogger, const std::string &buildDir);

    static void getErrorMessages(ErrorLogger &errorLogger) {
        unusedFunctionError(errorLogger, emptyString, 0, 0, "funcName");
    }

    // Return true if an error is reported.
    bool check(const Settings& settings, ErrorLogger& errorLogger) const;

    void updateFunctionData(const CheckUnusedFunctions& check);

private:
    static void unusedFunctionError(ErrorLogger& errorLogger,
                                    const std::string &filename, unsigned int fileIndex, unsigned int lineNumber,
                                    const std::string &funcname);

    struct CPPCHECKLIB FunctionUsage {
        std::string filename;
        unsigned int lineNumber{};
        unsigned int fileIndex{};
        bool usedSameFile{};
        bool usedOtherFile{};
    };

    std::unordered_map<std::string, FunctionUsage> mFunctions;

    class CPPCHECKLIB FunctionDecl {
    public:
        explicit FunctionDecl(const Function *f);
        std::string functionName;
        unsigned int lineNumber;
    };
    std::list<FunctionDecl> mFunctionDecl;
    std::set<std::string> mFunctionCalls;
};
/// @}
//---------------------------------------------------------------------------
#endif // checkunusedfunctionsH

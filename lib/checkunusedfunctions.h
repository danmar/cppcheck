/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "check.h"
#include "config.h"

#include <list>
#include <set>
#include <string>
#include <unordered_map>

class ErrorLogger;
class Function;
class Settings;
class Tokenizer;

namespace CTU {
    class FileInfo;
}

/// @addtogroup Checks
/** @brief Check for functions never called */
/// @{

class CPPCHECKLIB CheckUnusedFunctions : public Check {
public:
    /** @brief This constructor is used when registering the CheckUnusedFunctions */
    CheckUnusedFunctions() : Check(myName()) {}

    /** @brief This constructor is used when running checks. */
    CheckUnusedFunctions(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {}

    static void clear() {
        instance.mFunctions.clear();
        instance.mFunctionCalls.clear();
    }

    // Parse current tokens and determine..
    // * Check what functions are used
    // * What functions are declared
    void parseTokens(const Tokenizer &tokenizer, const char FileName[], const Settings *settings);

    // Return true if an error is reported.
    bool check(ErrorLogger * const errorLogger, const Settings& settings) const;

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    static CheckUnusedFunctions instance;

    std::string analyzerInfo() const;

    /** @brief Combine and analyze all analyzerInfos for all TUs */
    static void analyseWholeProgram(const Settings &settings, ErrorLogger * const errorLogger, const std::string &buildDir);

private:

    void getErrorMessages(ErrorLogger *errorLogger, const Settings * /*settings*/) const override {
        CheckUnusedFunctions::unusedFunctionError(errorLogger, emptyString, 0, "funcName");
    }

    void runChecks(const Tokenizer & /*tokenizer*/, ErrorLogger * /*errorLogger*/) override {}

    /**
     * Dummy implementation, just to provide error for --errorlist
     */
    static void unusedFunctionError(ErrorLogger * const errorLogger,
                                    const std::string &filename, unsigned int lineNumber,
                                    const std::string &funcname);

    static std::string myName() {
        return "Unused functions";
    }

    std::string classInfo() const override {
        return "Check for functions that are never called\n";
    }

    struct CPPCHECKLIB FunctionUsage {
        std::string filename;
        unsigned int lineNumber{};
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

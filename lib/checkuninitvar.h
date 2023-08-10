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
#ifndef checkuninitvarH
#define checkuninitvarH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "errortypes.h"

#include <list>
#include <string>

class Scope;
class Token;
class ErrorLogger;
class Settings;
class Library;
class Tokenizer;

namespace CTU {
    class FileInfo;
}

namespace tinyxml2 {
    class XMLElement;
}


/// @addtogroup Checks
/// @{


/** @brief Checking for uninitialized variables */

class CPPCHECKLIB CheckUninitVar : public Check {
    friend class TestUninitVar;

public:
    /** @brief This constructor is used when registering the CheckUninitVar */
    CheckUninitVar() : Check("Uninitialized variables") {}

    enum Alloc { NO_ALLOC, NO_CTOR_CALL, CTOR_CALL, ARRAY };

    static const Token *isVariableUsage(const Token *vartok, const Library &library, bool pointer, Alloc alloc, int indirect = 0);

private:
    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const override;

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    void getErrorMessages(ErrorLogger* errorLogger, const Settings* settings) const override;

    std::string classInfo() const override {
        return "Uninitialized variables\n"
               "- using uninitialized local variables\n"
               "- using allocated data before it has been initialized\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkuninitvarH

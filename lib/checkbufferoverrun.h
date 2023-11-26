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
#ifndef checkbufferoverrunH
#define checkbufferoverrunH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"
#include "ctu.h"
#include "errortypes.h"
#include "mathlib.h"
#include "symboldatabase.h"
#include "vfvalue.h"

#include <list>
#include <map>
#include <string>
#include <vector>

namespace tinyxml2 {
    class XMLElement;
}

class ErrorLogger;
class Settings;
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
class CPPCHECKLIB CheckBufferOverrun : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckBufferOverrun() : Check("Bounds checking") {}

private:
    void runChecks(const Tokenizer &tokenizer, ErrorLogger *errorLogger) override;

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const override;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) override;

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const override;

    static bool isCtuUnsafeBufferUsage(const Settings *settings, const Token *argtok, MathLib::bigint *offset, int type);
    static bool isCtuUnsafeArrayIndex(const Settings *settings, const Token *argtok, MathLib::bigint *offset);
    static bool isCtuUnsafePointerArith(const Settings *settings, const Token *argtok, MathLib::bigint *offset);

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const override;
    static bool analyseWholeProgram1(const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap, const CTU::FileInfo::UnsafeUsage &unsafeUsage, int type, ErrorLogger &errorLogger);

    std::string classInfo() const override {
        return "Out of bounds checking:\n"
               "- Array index out of bounds\n"
               "- Pointer arithmetic overflow\n"
               "- Buffer overflow\n"
               "- Dangerous usage of strncat()\n"
               "- Using array index before checking it\n"
               "- Partial string write that leads to buffer that is not zero terminated.\n"
               "- Check for large enough arrays being passed to functions\n"
               "- Allocating memory with a negative size\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkbufferoverrunH

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#include "mathlib.h"
#include "symboldatabase.h"
#include "valueflow.h"

#include <cstddef>
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
    CheckBufferOverrun() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckBufferOverrun(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) OVERRIDE {
        CheckBufferOverrun checkBufferOverrun(tokenizer, settings, errorLogger);
        checkBufferOverrun.arrayIndex();
        checkBufferOverrun.pointerArithmetic();
        checkBufferOverrun.bufferOverflow();
        checkBufferOverrun.arrayIndexThenCheck();
        checkBufferOverrun.stringNotZeroTerminated();
        checkBufferOverrun.objectIndex();
    }

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const OVERRIDE {
        CheckBufferOverrun c(nullptr, settings, errorLogger);
        c.arrayIndexError(nullptr, std::vector<Dimension>(), std::vector<const ValueFlow::Value *>());
        c.pointerArithmeticError(nullptr, nullptr, nullptr);
        c.negativeIndexError(nullptr, std::vector<Dimension>(), std::vector<const ValueFlow::Value *>());
        c.arrayIndexThenCheckError(nullptr, "i");
        c.bufferOverflowError(nullptr, nullptr);
        c.objectIndexError(nullptr, nullptr, true);
    }

    /** @brief Parse current TU and extract file info */
    Check::FileInfo *getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const OVERRIDE;

    /** @brief Analyse all file infos for all TU */
    bool analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<Check::FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) OVERRIDE;

private:

    void arrayIndex();
    void arrayIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const std::vector<const ValueFlow::Value *> &indexes);
    void negativeIndexError(const Token *tok, const std::vector<Dimension> &dimensions, const std::vector<const ValueFlow::Value *> &indexes);

    void pointerArithmetic();
    void pointerArithmeticError(const Token *tok, const Token *indexToken, const ValueFlow::Value *indexValue);

    void bufferOverflow();
    void bufferOverflowError(const Token *tok, const ValueFlow::Value *value);

    void arrayIndexThenCheck();
    void arrayIndexThenCheckError(const Token *tok, const std::string &indexName);

    void stringNotZeroTerminated();
    void terminateStrncpyError(const Token *tok, const std::string &varname);

    void objectIndex();
    void objectIndexError(const Token *tok, const ValueFlow::Value *v, bool known);

    ValueFlow::Value getBufferSize(const Token *bufTok) const;

    // CTU

    /** data for multifile checking */
    class MyFileInfo : public Check::FileInfo {
    public:
        /** unsafe array index usage */
        std::list<CTU::FileInfo::UnsafeUsage> unsafeArrayIndex;

        /** unsafe pointer arithmetics */
        std::list<CTU::FileInfo::UnsafeUsage> unsafePointerArith;

        /** Convert MyFileInfo data into xml string */
        std::string toString() const OVERRIDE;
    };

    static bool isCtuUnsafeBufferUsage(const Check *check, const Token *argtok, MathLib::bigint *offset, int type);
    static bool isCtuUnsafeArrayIndex(const Check *check, const Token *argtok, MathLib::bigint *offset);
    static bool isCtuUnsafePointerArith(const Check *check, const Token *argtok, MathLib::bigint *offset);

    Check::FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const OVERRIDE;
    bool analyseWholeProgram1(const CTU::FileInfo *ctu, const std::map<std::string, std::list<const CTU::FileInfo::CallBase *>> &callsMap, const CTU::FileInfo::UnsafeUsage &unsafeUsage, int type, ErrorLogger &errorLogger);


    static std::string myName() {
        return "Bounds checking";
    }

    std::string classInfo() const OVERRIDE {
        return "Out of bounds checking:\n"
               "- Array index out of bounds\n"
               "- Pointer arithmetic overflow\n"
               "- Buffer overflow\n"
               "- Dangerous usage of strncat()\n"
               "- Using array index before checking it\n"
               "- Partial string write that leads to buffer that is not zero terminated.\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkbufferoverrunH

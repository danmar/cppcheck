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
#ifndef checkH
#define checkH
//---------------------------------------------------------------------------

#include "config.h"
#include "errortypes.h"

#include <list>
#include <string>
#include <utility>

namespace tinyxml2 {
    class XMLElement;
}

namespace CTU {
    class FileInfo;
}

namespace ValueFlow {
    class Value;
}

class Settings;
class Token;
class ErrorLogger;
class ErrorMessage;
class Tokenizer;

/** Use WRONG_DATA in checkers to mark conditions that check that data is correct */
#define WRONG_DATA(COND, TOK)  ((COND) && wrongData((TOK), #COND))

/// @addtogroup Core
/// @{

/**
 * @brief Interface class that cppcheck uses to communicate with the checks.
 * All checking classes must inherit from this class
 */
class CPPCHECKLIB Check {
public:
    /** This constructor is used when registering the CheckClass */
    explicit Check(const std::string &aname);

    /** This constructor is used when running checks. */
    Check(std::string aname, const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : mTokenizer(tokenizer), mSettings(settings), mErrorLogger(errorLogger), mName(std::move(aname)) {}

    virtual ~Check() {
        if (!mTokenizer)
            instances().remove(this);
    }

    Check(const Check &) = delete;
    Check& operator=(const Check &) = delete;

    /** List of registered check classes. This is used by Cppcheck to run checks and generate documentation */
    static std::list<Check *> &instances();

    /** run checks, the token list is not simplified */
    virtual void runChecks(const Tokenizer &, ErrorLogger *) = 0;

    /** get error messages */
    virtual void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const = 0;

    /** class name, used to generate documentation */
    const std::string& name() const {
        return mName;
    }

    /** get information about this class, used to generate documentation */
    virtual std::string classInfo() const = 0;

    /**
     * Write given error to stdout in xml format.
     * This is for for printout out the error list with --errorlist
     * @param errmsg Error message to write
     */
    static void writeToErrorList(const ErrorMessage &errmsg);

    /** Base class used for whole-program analysis */
    class CPPCHECKLIB FileInfo {
    public:
        FileInfo() = default;
        virtual ~FileInfo() = default;
        virtual std::string toString() const {
            return std::string();
        }
    };

    virtual FileInfo * getFileInfo(const Tokenizer *tokenizer, const Settings *settings) const {
        (void)tokenizer;
        (void)settings;
        return nullptr;
    }

    virtual FileInfo * loadFileInfoFromXml(const tinyxml2::XMLElement *xmlElement) const {
        (void)xmlElement;
        return nullptr;
    }

    // Return true if an error is reported.
    virtual bool analyseWholeProgram(const CTU::FileInfo *ctu, const std::list<FileInfo*> &fileInfo, const Settings& /*settings*/, ErrorLogger & /*errorLogger*/) {
        (void)ctu;
        (void)fileInfo;
        //(void)settings;
        //(void)errorLogger;
        return false;
    }

    static std::string getMessageId(const ValueFlow::Value &value, const char id[]);

protected:
    const Tokenizer* const mTokenizer{};
    const Settings* const mSettings{};
    ErrorLogger* const mErrorLogger{};

    /** report an error */
    void reportError(const Token *tok, const Severity::SeverityType severity, const std::string &id, const std::string &msg) {
        reportError(tok, severity, id, msg, CWE(0U), Certainty::normal);
    }

    /** report an error */
    void reportError(const Token *tok, const Severity::SeverityType severity, const std::string &id, const std::string &msg, const CWE &cwe, Certainty certainty) {
        const std::list<const Token *> callstack(1, tok);
        reportError(callstack, severity, id, msg, cwe, certainty);
    }

    /** report an error */
    void reportError(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg) {
        reportError(callstack, severity, id, msg, CWE(0U), Certainty::normal);
    }

    /** report an error */
    void reportError(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg, const CWE &cwe, Certainty certainty);

    void reportError(const ErrorPath &errorPath, Severity::SeverityType severity, const char id[], const std::string &msg, const CWE &cwe, Certainty certainty);

    /** log checker */
    void logChecker(const char id[]);

    ErrorPath getErrorPath(const Token* errtok, const ValueFlow::Value* value, std::string bug) const;

    /**
     * Use WRONG_DATA in checkers when you check for wrong data. That
     * will call this method
     */
    bool wrongData(const Token *tok, const char *str);

private:
    const std::string mName;
};

/// @}
//---------------------------------------------------------------------------
#endif //  checkH

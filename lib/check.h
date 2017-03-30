/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "token.h"
#include "tokenize.h"
#include "settings.h"
#include "errorlogger.h"

#include <list>
#include <set>

/**
 * Use this macro Cppcheck data can be wrong and you need a to check if that happens to avoid crash/hang
 * Using this macro we can make sure that released binaries don't crash/hang but the problem is not hidden
 * in debug builds.
 */
#define CHECK_WRONG_DATA(X)   (X)

namespace tinyxml2 {
    class XMLElement;
}

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
    Check(const std::string &aname, const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : _tokenizer(tokenizer), _settings(settings), _errorLogger(errorLogger), _name(aname) {
    }

    virtual ~Check() {
        if (!_tokenizer)
            instances().remove(this);
    }

    /** List of registered check classes. This is used by Cppcheck to run checks and generate documentation */
    static std::list<Check *> &instances();

    /** run checks, the token list is not simplified */
    virtual void runChecks(const Tokenizer *, const Settings *, ErrorLogger *) {
    }

    /** run checks, the token list is simplified */
    virtual void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) = 0;

    /** get error messages */
    virtual void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const = 0;

    /** class name, used to generate documentation */
    const std::string& name() const {
        return _name;
    }

    /** get information about this class, used to generate documentation */
    virtual std::string classInfo() const = 0;

    /**
     * Write given error to errorlogger or to out stream in xml format.
     * This is for for printout out the error list with --errorlist
     * @param errmsg Error message to write
     */
    static void reportError(const ErrorLogger::ErrorMessage &errmsg);

    bool inconclusiveFlag() const {
        return _settings && _settings->inconclusive;
    }

    /** Base class used for whole-program analysis */
    class FileInfo {
    public:
        FileInfo() {}
        virtual ~FileInfo() {}
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

    virtual void analyseWholeProgram(const std::list<FileInfo*> &fileInfo, const Settings& settings, ErrorLogger &errorLogger) {
        (void)fileInfo;
        (void)settings;
        (void)errorLogger;
    }

protected:
    const Tokenizer * const _tokenizer;
    const Settings * const _settings;
    ErrorLogger * const _errorLogger;

    /** report an error */
    template<typename T, typename U>
    void reportError(const Token *tok, const Severity::SeverityType severity, const T id, const U msg) {
        reportError(tok, severity, id, msg, CWE(0U), false);
    }

    /** report an error */
    template<typename T, typename U>
    void reportError(const Token *tok, const Severity::SeverityType severity, const T id, const U msg, const CWE &cwe, bool inconclusive) {
        const std::list<const Token *> callstack(1, tok);
        reportError(callstack, severity, id, msg, cwe, inconclusive);
    }

    /** report an error */
    template<typename T, typename U>
    void reportError(const std::list<const Token *> &callstack, Severity::SeverityType severity, const T id, const U msg) {
        reportError(callstack, severity, id, msg, CWE(0U), false);
    }

    /** report an error */
    template<typename T, typename U>
    void reportError(const std::list<const Token *> &callstack, Severity::SeverityType severity, const T id, const U msg, const CWE &cwe, bool inconclusive) {
        const ErrorLogger::ErrorMessage errmsg(callstack, _tokenizer?&_tokenizer->list:0, severity, id, msg, cwe, inconclusive);
        if (_errorLogger)
            _errorLogger->reportErr(errmsg);
        else
            reportError(errmsg);
    }

private:
    const std::string _name;

    /** disabled assignment operator and copy constructor */
    void operator=(const Check &);
    explicit Check(const Check &);
};

/// @}
//---------------------------------------------------------------------------
#endif //  checkH

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "cppcheck.h"

#include "preprocessor.h" // Preprocessor
#include "tokenize.h" // Tokenizer

#include "check.h"
#include "path.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "timer.h"
#include "version.h"

#ifdef HAVE_RULES
#define PCRE_STATIC
#include <pcre.h>
#endif

static const char Version[] = CPPCHECK_VERSION_STRING;
static const char ExtraVersion[] = "";

static TimerResults S_timerResults;

CppCheck::CppCheck(ErrorLogger &errorLogger, bool useGlobalSuppressions)
    : _errorLogger(errorLogger), exitcode(0), _useGlobalSuppressions(useGlobalSuppressions), tooManyConfigs(false), _simplify(true)
{
}

CppCheck::~CppCheck()
{
    S_timerResults.ShowResults(_settings._showtime);
}

const char * CppCheck::version()
{
    return Version;
}

const char * CppCheck::extraVersion()
{
    return ExtraVersion;
}

unsigned int CppCheck::check(const std::string &path)
{
    return processFile(path, "");
}

unsigned int CppCheck::check(const std::string &path, const std::string &content)
{
    return processFile(path, content);
}

void CppCheck::replaceAll(std::string& code, const std::string &from, const std::string &to)
{
    std::size_t pos = 0;
    while ((pos = code.find(from, pos)) != std::string::npos) {
        code.replace(pos, from.length(), to);
        pos += to.length();
    }
}

bool CppCheck::findError(std::string code, const char FileName[])
{
    // First make sure that error occurs with the original code
    checkFile(code, FileName);
    if (_errorList.empty()) {
        // Error does not occur with this code
        return false;
    }

    std::string previousCode = code;
    std::string error = _errorList.front();
    for (;;) {

        // Try to remove included files from the source
        std::size_t found = previousCode.rfind("\n#endfile");
        if (found == std::string::npos) {
            // No modifications can be done to the code
        } else {
            // Modify code and re-check it to see if error
            // is still there.
            code = previousCode.substr(found+9);
            _errorList.clear();
            checkFile(code, FileName);
        }

        if (_errorList.empty()) {
            // Latest code didn't fail anymore. Fall back
            // to previous code
            code = previousCode;
        } else {
            error = _errorList.front();
        }

        // Add '\n' so that "\n#file" on first line would be found
        code = "// " + error + "\n" + code;
        replaceAll(code, "\n#file", "\n// #file");
        replaceAll(code, "\n#endfile", "\n// #endfile");

        // We have reduced the code as much as we can. Print out
        // the code and quit.
        _errorLogger.reportOut(code);
        break;
    }

    return true;
}

unsigned int CppCheck::processFile(const std::string& filename, const std::string& fileContent)
{
    exitcode = 0;

    // only show debug warnings for accepted C/C++ source files
    if (!Path::acceptFile(filename))
        _settings.debugwarnings = false;

    if (_settings.terminated())
        return exitcode;

    if (_settings._errorsOnly == false) {
        std::string fixedpath = Path::simplifyPath(filename.c_str());
        fixedpath = Path::toNativeSeparators(fixedpath);
        _errorLogger.reportOut(std::string("Checking ") + fixedpath + std::string("..."));
    }

    try {
        Preprocessor preprocessor(&_settings, this);
        std::list<std::string> configurations;
        std::string filedata = "";

        if (!fileContent.empty()) {
            // File content was given as a string (democlient)
            std::istringstream iss(fileContent);
            preprocessor.preprocess(iss, filedata, configurations, filename, _settings._includePaths);
        } else {
            // Only file name was given, read the content from file
            std::ifstream fin(filename.c_str());
            Timer t("Preprocessor::preprocess", _settings._showtime, &S_timerResults);
            preprocessor.preprocess(fin, filedata, configurations, filename, _settings._includePaths);
        }

        if (_settings.checkConfiguration) {
            return 0;
        }

        // Run rules on this code
        for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
            if (it->tokenlist == "define") {
                Tokenizer tokenizer2(&_settings, this);
                std::istringstream istr2(filedata);
                tokenizer2.list.createTokens(istr2, filename);

                for (const Token *tok = tokenizer2.list.front(); tok; tok = tok->next()) {
                    if (tok->str() == "#define") {
                        std::string code = std::string(tok->linenr()-1U, '\n');
                        for (const Token *tok2 = tok; tok2 && tok2->linenr() == tok->linenr(); tok2 = tok2->next())
                            code += " " + tok2->str();
                        Tokenizer tokenizer3(&_settings, this);
                        std::istringstream istr3(code);
                        tokenizer3.list.createTokens(istr3, tokenizer2.list.file(tok));
                        executeRules("define", tokenizer3);
                    }
                }
                break;
            }
        }

        if (!_settings.userDefines.empty() && _settings._maxConfigs==1U) {
            configurations.clear();
            configurations.push_back(_settings.userDefines);
        }

        if (!_settings._force && configurations.size() > _settings._maxConfigs) {
            if (_settings.isEnabled("information")) {
                tooManyConfigsError(Path::toNativeSeparators(filename),configurations.size());
            } else {
                tooManyConfigs = true;
            }
        }

        unsigned int checkCount = 0;
        for (std::list<std::string>::const_iterator it = configurations.begin(); it != configurations.end(); ++it) {
            // Check only a few configurations (default 12), after that bail out, unless --force
            // was used.
            if (!_settings._force && ++checkCount > _settings._maxConfigs)
                break;

            cfg = *it;

            // If only errors are printed, print filename after the check
            if (_settings._errorsOnly == false && it != configurations.begin()) {
                std::string fixedpath = Path::simplifyPath(filename.c_str());
                fixedpath = Path::toNativeSeparators(fixedpath);
                _errorLogger.reportOut(std::string("Checking ") + fixedpath + ": " + cfg + std::string("..."));
            }

            if (!_settings.userDefines.empty()) {
                if (!cfg.empty())
                    cfg = ";" + cfg;
                cfg = _settings.userDefines + cfg;
            }

            Timer t("Preprocessor::getcode", _settings._showtime, &S_timerResults);
            const std::string codeWithoutCfg = preprocessor.getcode(filedata, cfg, filename);
            t.Stop();

            const std::string &appendCode = _settings.append();

            if (_settings.debugFalsePositive) {
                if (findError(codeWithoutCfg + appendCode, filename.c_str())) {
                    return exitcode;
                }
            } else {
                checkFile(codeWithoutCfg + appendCode, filename.c_str());
            }
        }
    } catch (const std::runtime_error &e) {
        internalError(filename, e.what());
    } catch (const InternalError &e) {
        internalError(filename, e.errorMessage);
    }

    if (_settings.isEnabled("information") || _settings.checkConfiguration)
        reportUnmatchedSuppressions(_settings.nomsg.getUnmatchedLocalSuppressions(filename));

    _errorList.clear();
    return exitcode;
}

void CppCheck::internalError(const std::string &filename, const std::string &msg)
{
    const std::string fixedpath = Path::toNativeSeparators(filename);
    const std::string fullmsg("Bailing out from checking " + fixedpath + " since there was a internal error: " + msg);

    if (_settings.isEnabled("information")) {
        const ErrorLogger::ErrorMessage::FileLocation loc1(filename, 0);
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        callstack.push_back(loc1);

        ErrorLogger::ErrorMessage errmsg(callstack,
                                         Severity::information,
                                         fullmsg,
                                         "internalError",
                                         false);

        _errorLogger.reportErr(errmsg);

    } else {
        // Report on stdout
        _errorLogger.reportOut(fullmsg);
    }
}


void CppCheck::checkFunctionUsage()
{
    // This generates false positives - especially for libraries
    if (_settings.isEnabled("unusedFunction") && _settings._jobs == 1) {
        const bool verbose_orig = _settings._verbose;
        _settings._verbose = false;

        if (_settings._errorsOnly == false)
            _errorLogger.reportOut("Checking usage of global functions..");

        _checkUnusedFunctions.check(this);

        _settings._verbose = verbose_orig;
    }
}

void CppCheck::analyseFile(std::istream &fin, const std::string &filename)
{
    // Preprocess file..
    Preprocessor preprocessor(&_settings, this);
    std::list<std::string> configurations;
    std::string filedata = "";
    preprocessor.preprocess(fin, filedata, configurations, filename, _settings._includePaths);
    const std::string code = preprocessor.getcode(filedata, "", filename);

    if (_settings.checkConfiguration) {
        return;
    }

    // Tokenize..
    Tokenizer tokenizer(&_settings, this);
    std::istringstream istr(code);
    tokenizer.tokenize(istr, filename.c_str(), "");
    tokenizer.simplifyTokenList2();

    // Analyse the tokens..
    std::set<std::string> data;
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        (*it)->analyse(tokenizer.tokens(), data);
    }

    // Save analysis results..
    // TODO: This loop should be protected by a mutex or something like that
    //       The saveAnalysisData must _not_ be called from many threads at the same time.
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        (*it)->saveAnalysisData(data);
    }
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a specified file
//---------------------------------------------------------------------------

void CppCheck::checkFile(const std::string &code, const char FileName[])
{
    if (_settings.terminated() || _settings.checkConfiguration)
        return;

    Tokenizer _tokenizer(&_settings, this);
    if (_settings._showtime != SHOWTIME_NONE)
        _tokenizer.setTimerResults(&S_timerResults);
    try {
        bool result;

        // Execute rules for "raw" code
        for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
            if (it->tokenlist == "raw") {
                Tokenizer tokenizer2(&_settings, this);
                std::istringstream istr(code);
                tokenizer2.list.createTokens(istr, FileName);
                executeRules("raw", tokenizer2);
                break;
            }
        }

        // Tokenize the file
        std::istringstream istr(code);

        Timer timer("Tokenizer::tokenize", _settings._showtime, &S_timerResults);
        result = _tokenizer.tokenize(istr, FileName, cfg);
        timer.Stop();
        if (!result) {
            // File had syntax errors, abort
            return;
        }

        // call all "runChecks" in all registered Check classes
        for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
            if (_settings.terminated())
                return;

            Timer timerRunChecks((*it)->name() + "::runChecks", _settings._showtime, &S_timerResults);
            (*it)->runChecks(&_tokenizer, &_settings, this);
        }

        if (_settings.isEnabled("unusedFunction") && _settings._jobs == 1)
            _checkUnusedFunctions.parseTokens(_tokenizer, FileName, &_settings);

        executeRules("normal", _tokenizer);

        if (!_simplify)
            return;

        Timer timer3("Tokenizer::simplifyTokenList", _settings._showtime, &S_timerResults);
        result = _tokenizer.simplifyTokenList2();
        timer3.Stop();
        if (!result)
            return;

        // call all "runSimplifiedChecks" in all registered Check classes
        for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
            if (_settings.terminated())
                return;

            Timer timerSimpleChecks((*it)->name() + "::runSimplifiedChecks", _settings._showtime, &S_timerResults);
            (*it)->runSimplifiedChecks(&_tokenizer, &_settings, this);
        }

        if (_settings.terminated())
            return;

        executeRules("simple", _tokenizer);

        if (_settings.terminated())
            return;
    } catch (const InternalError &e) {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        ErrorLogger::ErrorMessage::FileLocation loc2;
        loc2.setfile(Path::toNativeSeparators(FileName));
        locationList.push_back(loc2);
        ErrorLogger::ErrorMessage::FileLocation loc;
        if (e.token) {
            loc.line = e.token->linenr();
            const std::string fixedpath = Path::toNativeSeparators(_tokenizer.list.file(e.token));
            loc.setfile(fixedpath);
        } else {
            loc.setfile(_tokenizer.getSourceFilePath());
        }
        locationList.push_back(loc);
        const ErrorLogger::ErrorMessage errmsg(locationList,
                                               Severity::error,
                                               e.errorMessage,
                                               "cppcheckError",
                                               false);

        _errorLogger.reportErr(errmsg);
    }
}

void CppCheck::executeRules(const std::string &tokenlist, const Tokenizer &tokenizer)
{
    (void)tokenlist;
    (void)tokenizer;

#ifdef HAVE_RULES
    // Are there rules to execute?
    bool isrule = false;
    for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
        if (it->tokenlist == tokenlist)
            isrule = true;
    }

    // There is no rule to execute
    if (isrule == false)
        return;

    // Write all tokens in a string that can be parsed by pcre
    std::ostringstream ostr;
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        ostr << " " << tok->str();
    const std::string str(ostr.str());

    for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
        const Settings::Rule &rule = *it;
        if (rule.pattern.empty() || rule.id.empty() || rule.severity.empty() || rule.tokenlist != tokenlist)
            continue;

        const char *error = 0;
        int erroffset = 0;
        pcre *re = pcre_compile(rule.pattern.c_str(),0,&error,&erroffset,NULL);
        if (!re) {
            if (error) {
                ErrorLogger::ErrorMessage errmsg(std::list<ErrorLogger::ErrorMessage::FileLocation>(),
                                                 Severity::error,
                                                 error,
                                                 "pcre_compile",
                                                 false);

                reportErr(errmsg);
            }
            continue;
        }

        int pos = 0;
        int ovector[30];
        while (pos < (int)str.size() && 0 <= pcre_exec(re, NULL, str.c_str(), (int)str.size(), pos, 0, ovector, 30)) {
            unsigned int pos1 = (unsigned int)ovector[0];
            unsigned int pos2 = (unsigned int)ovector[1];

            // jump to the end of the match for the next pcre_exec
            pos = (int)pos2;

            // determine location..
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.setfile(tokenizer.getSourceFilePath());
            loc.line = 0;

            std::size_t len = 0;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
                len = len + 1U + tok->str().size();
                if (len > pos1) {
                    loc.setfile(tokenizer.list.getFiles().at(tok->fileIndex()));
                    loc.line = tok->linenr();
                    break;
                }
            }

            const std::list<ErrorLogger::ErrorMessage::FileLocation> callStack(1, loc);

            // Create error message
            std::string summary;
            if (rule.summary.empty())
                summary = "found '" + str.substr(pos1, pos2 - pos1) + "'";
            else
                summary = rule.summary;
            const ErrorLogger::ErrorMessage errmsg(callStack, Severity::fromString(rule.severity), summary, rule.id, false);

            // Report error
            reportErr(errmsg);
        }

        pcre_free(re);
    }
#endif
}

Settings &CppCheck::settings()
{
    return _settings;
}

void CppCheck::tooManyConfigsError(const std::string &file, const std::size_t numberOfConfigurations)
{
    if (!_settings.isEnabled("information") && !tooManyConfigs)
        return;

    tooManyConfigs = false;

    if (_settings.isEnabled("information") && file.empty())
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        ErrorLogger::ErrorMessage::FileLocation location;
        location.setfile(file);
        loclist.push_back(location);
    }

    std::ostringstream msg;
    msg << "Too many #ifdef configurations - cppcheck only checks " << _settings._maxConfigs;
    if (numberOfConfigurations > _settings._maxConfigs)
        msg << " of " << numberOfConfigurations << " configurations. Use --force to check all configurations.\n";
    if (file.empty())
        msg << " configurations. Use --force to check all configurations. For more details, use --enable=information.\n";
    msg << "The checking of the file will be interrupted because there are too many "
        "#ifdef configurations. Checking of all #ifdef configurations can be forced "
        "by --force command line option or from GUI preferences. However that may "
        "increase the checking time.";
    if (file.empty())
        msg << " For more details, use --enable=information.";


    ErrorLogger::ErrorMessage errmsg(loclist,
                                     Severity::information,
                                     msg.str(),
                                     "toomanyconfigs",
                                     false);

    reportErr(errmsg);
}

//---------------------------------------------------------------------------

void CppCheck::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    if (!_settings.library.reportErrors(msg.file0))
        return;

    std::string errmsg = msg.toString(_settings._verbose);
    if (errmsg.empty())
        return;

    // Alert only about unique errors
    if (std::find(_errorList.begin(), _errorList.end(), errmsg) != _errorList.end())
        return;

    if (_settings.debugFalsePositive) {
        // Don't print out error
        _errorList.push_back(errmsg);
        return;
    }

    std::string file;
    unsigned int line(0);
    if (!msg._callStack.empty()) {
        file = msg._callStack.back().getfile(false);
        line = msg._callStack.back().line;
    }

    if (_useGlobalSuppressions) {
        if (_settings.nomsg.isSuppressed(msg._id, file, line))
            return;
    } else {
        if (_settings.nomsg.isSuppressedLocal(msg._id, file, line))
            return;
    }

    if (!_settings.nofail.isSuppressed(msg._id, file, line))
        exitcode = 1;

    _errorList.push_back(errmsg);

    _errorLogger.reportErr(msg);
}

void CppCheck::reportOut(const std::string &outmsg)
{
    _errorLogger.reportOut(outmsg);
}

void CppCheck::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    _errorLogger.reportProgress(filename, stage, value);
}

void CppCheck::reportInfo(const ErrorLogger::ErrorMessage &msg)
{
    // Suppressing info message?
    std::string file;
    unsigned int line(0);
    if (!msg._callStack.empty()) {
        file = msg._callStack.back().getfile(false);
        line = msg._callStack.back().line;
    }
    if (_useGlobalSuppressions) {
        if (_settings.nomsg.isSuppressed(msg._id, file, line))
            return;
    } else {
        if (_settings.nomsg.isSuppressedLocal(msg._id, file, line))
            return;
    }

    _errorLogger.reportInfo(msg);
}

void CppCheck::reportStatus(unsigned int /*fileindex*/, unsigned int /*filecount*/, std::size_t /*sizedone*/, std::size_t /*sizetotal*/)
{

}

void CppCheck::getErrorMessages()
{
    tooManyConfigs = true;
    tooManyConfigsError("",0U);

    // call all "getErrorMessages" in all registered Check classes
    for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
        (*it)->getErrorMessages(this, &_settings);

    Tokenizer::getErrorMessages(this, &_settings);
    Preprocessor::getErrorMessages(this, &_settings);
}

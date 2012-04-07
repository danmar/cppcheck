/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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
#include <stdexcept>
#include "timer.h"

#ifdef HAVE_RULES
#define PCRE_STATIC
#include <pcre.h>
#endif

static const char Version[] = "1.53";
static const char ExtraVersion[] = "";

static TimerResults S_timerResults;

CppCheck::CppCheck(ErrorLogger &errorLogger, bool useGlobalSuppressions)
    : exitcode(0), _useGlobalSuppressions(useGlobalSuppressions), _errorLogger(errorLogger)
{
}

CppCheck::~CppCheck()
{
    if (_settings._showtime != SHOWTIME_NONE)
        S_timerResults.ShowResults();
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
    return processFile(path);
}

unsigned int CppCheck::check(const std::string &path, const std::string &content)
{
    _fileContent = content;
    const unsigned int retval = processFile(path);
    _fileContent.clear();
    return retval;
}

void CppCheck::replaceAll(std::string& code, const std::string &from, const std::string &to)
{
    size_t pos = 0;
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
        size_t found=previousCode.rfind("\n#endfile");
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

unsigned int CppCheck::processFile(const std::string& filename)
{
    exitcode = 0;

    // only show debug warnings for C/C++ source files (don't fix
    // debug warnings for java/c#/etc files)
    if (!Path::acceptFile(filename))
        _settings.debugwarnings = false;

    // TODO: Should this be moved out to its own function so all the files can be
    // analysed before any files are checked?
    if (_settings.test_2_pass && _settings._jobs == 1) {
        const std::string printname = Path::toNativeSeparators(filename);
        reportOut("Analysing " + printname + "...");

        std::ifstream f(filename.c_str());
        analyseFile(f, filename);
    }

    _errout.str("");

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

        if (!_fileContent.empty()) {
            // File content was given as a string
            std::istringstream iss(_fileContent);
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

        if (!_settings.userDefines.empty()) {
            configurations.clear();
            configurations.push_back(_settings.userDefines);
        }

        int checkCount = 0;
        for (std::list<std::string>::const_iterator it = configurations.begin(); it != configurations.end(); ++it) {
            // Check only a few configurations (default 12), after that bail out, unless --force
            // was used.
            if (!_settings._force && checkCount >= _settings._maxConfigs) {

                const std::string fixedpath = Path::toNativeSeparators(filename);
                ErrorLogger::ErrorMessage::FileLocation location;
                location.setfile(fixedpath);
                std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
                loclist.push_back(location);
                const std::string msg("Interrupted checking because of too many #ifdef configurations.\n"
                                      "The checking of the file was interrupted because there were too many "
                                      "#ifdef configurations. Checking of all #ifdef configurations can be forced "
                                      "by --force command line option or from GUI preferences. However that may "
                                      "increase the checking time.");
                ErrorLogger::ErrorMessage errmsg(loclist,
                                                 Severity::information,
                                                 msg,
                                                 "toomanyconfigs",
                                                 false);

                if (!_settings.nomsg.isSuppressedLocal(errmsg._id, fixedpath, location.line)) {
                    reportErr(errmsg);
                }

                break;
            }

            cfg = *it;
            Timer t("Preprocessor::getcode", _settings._showtime, &S_timerResults);
            const std::string codeWithoutCfg = preprocessor.getcode(filedata, *it, filename);
            t.Stop();

            // If only errors are printed, print filename after the check
            if (_settings._errorsOnly == false && it != configurations.begin()) {
                std::string fixedpath = Path::simplifyPath(filename.c_str());
                fixedpath = Path::toNativeSeparators(fixedpath);
                _errorLogger.reportOut(std::string("Checking ") + fixedpath + ": " + cfg + std::string("..."));
            }

            const std::string &appendCode = _settings.append();

            if (_settings.debugFalsePositive) {
                if (findError(codeWithoutCfg + appendCode, filename.c_str())) {
                    return exitcode;
                }
            } else {
                checkFile(codeWithoutCfg + appendCode, filename.c_str());
            }

            ++checkCount;
        }
    } catch (std::runtime_error &e) {
        // Exception was thrown when checking this file..
        const std::string fixedpath = Path::toNativeSeparators(filename);
        _errorLogger.reportOut("Bailing out from checking " + fixedpath + ": " + e.what());
    }

    if (!_settings._errorsOnly)
        reportUnmatchedSuppressions(_settings.nomsg.getUnmatchedLocalSuppressions(filename));

    _errorList.clear();
    return exitcode;
}



void CppCheck::checkFunctionUsage()
{
    // This generates false positives - especially for libraries
    if (_settings.isEnabled("unusedFunction") && _settings._jobs == 1) {
        const bool verbose_orig = _settings._verbose;
        _settings._verbose = false;

        _errout.str("");
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
    tokenizer.simplifyTokenList();

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
    try {
        bool result;

        // Tokenize the file
        std::istringstream istr(code);

        Timer timer("Tokenizer::tokenize", _settings._showtime, &S_timerResults);
        result = _tokenizer.tokenize(istr, FileName, cfg);
        timer.Stop();
        if (!result) {
            // File had syntax errors, abort
            return;
        }

        // Update the _dependencies..
        if (_tokenizer.getFiles().size() >= 2)
            _dependencies.insert(_tokenizer.getFiles().begin()+1, _tokenizer.getFiles().end());

        // call all "runChecks" in all registered Check classes
        for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
            if (_settings.terminated())
                return;

            Timer timerRunChecks((*it)->name() + "::runChecks", _settings._showtime, &S_timerResults);
            (*it)->runChecks(&_tokenizer, &_settings, this);
        }

        if (_settings.isEnabled("unusedFunction") && _settings._jobs == 1)
            _checkUnusedFunctions.parseTokens(_tokenizer);

        Timer timer3("Tokenizer::simplifyTokenList", _settings._showtime, &S_timerResults);
        result = _tokenizer.simplifyTokenList();
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

#ifdef HAVE_RULES
        // Are there extra rules?
        if (!_settings.rules.empty()) {
            std::ostringstream ostr;
            for (const Token *tok = _tokenizer.tokens(); tok; tok = tok->next())
                ostr << " " << tok->str();
            const std::string str(ostr.str());
            for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
                const Settings::Rule &rule = *it;
                if (rule.pattern.empty() || rule.id.empty() || rule.severity.empty())
                    continue;

                const char *error = 0;
                int erroffset = 0;
                pcre *re = pcre_compile(rule.pattern.c_str(),0,&error,&erroffset,NULL);
                if (!re && error) {
                    ErrorLogger::ErrorMessage errmsg(std::list<ErrorLogger::ErrorMessage::FileLocation>(),
                                                     Severity::error,
                                                     error,
                                                     "pcre_compile",
                                                     false);

                    reportErr(errmsg);
                }
                if (!re)
                    continue;

                int pos = 0;
                int ovector[30];
                while (0 <= pcre_exec(re, NULL, str.c_str(), (int)str.size(), pos, 0, ovector, 30)) {
                    unsigned int pos1 = (unsigned int)ovector[0];
                    unsigned int pos2 = (unsigned int)ovector[1];

                    // jump to the end of the match for the next pcre_exec
                    pos = (int)pos2;

                    // determine location..
                    ErrorLogger::ErrorMessage::FileLocation loc;
                    loc.setfile(_tokenizer.getFiles().front());
                    loc.line = 0;

                    unsigned int len = 0;
                    for (const Token *tok = _tokenizer.tokens(); tok; tok = tok->next()) {
                        len = len + 1 + tok->str().size();
                        if (len > pos1) {
                            loc.setfile(_tokenizer.getFiles().at(tok->fileIndex()));
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
        }
#endif
    } catch (const InternalError &e) {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        ErrorLogger::ErrorMessage::FileLocation loc2;
        loc2.setfile(Path::toNativeSeparators(FileName));
        locationList.push_back(loc2);
        ErrorLogger::ErrorMessage::FileLocation loc;
        if (e.token) {
            loc.line = e.token->linenr();
            const std::string fixedpath = Path::toNativeSeparators(_tokenizer.file(e.token));
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

Settings &CppCheck::settings()
{
    return _settings;
}

//---------------------------------------------------------------------------

void CppCheck::reportErr(const ErrorLogger::ErrorMessage &msg)
{
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

    if (_settings._verbose) {
        errmsg += "\n    Defines=\'" + cfg + "\'\n";
    }

    _errorLogger.reportErr(msg);

    _errout << errmsg << std::endl;
}

void CppCheck::reportOut(const std::string &outmsg)
{
    _errorLogger.reportOut(outmsg);
}

void CppCheck::reportProgress(const std::string &filename, const char stage[], const unsigned int value)
{
    _errorLogger.reportProgress(filename, stage, value);
}

void CppCheck::reportStatus(unsigned int /*fileindex*/, unsigned int /*filecount*/, size_t /*sizedone*/, size_t /*sizetotal*/)
{

}

void CppCheck::getErrorMessages()
{
    // call all "getErrorMessages" in all registered Check classes
    for (std::list<Check *>::iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
        (*it)->getErrorMessages(this, &_settings);

    Tokenizer tokenizer(&_settings, 0);
    tokenizer.getErrorMessages(this, &_settings);

    Preprocessor::getErrorMessages(this, &_settings);
}

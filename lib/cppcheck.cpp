/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "check.h"
#include "checkunusedfunctions.h"
#include "library.h"
#include "mathlib.h"
#include "path.h"
#include "platform.h"
#include "preprocessor.h" // Preprocessor
#include "suppressions.h"
#include "timer.h"
#include "token.h"
#include "tokenize.h" // Tokenizer
#include "tokenlist.h"
#include "version.h"

#include <simplecpp.h>
#include <tinyxml2.h>
#include <algorithm>
#include <cstring>
#include <new>
#include <set>
#include <stdexcept>
#include <vector>

#ifdef HAVE_RULES
#define PCRE_STATIC
#include <pcre.h>
#endif

static const char Version[] = CPPCHECK_VERSION_STRING;
static const char ExtraVersion[] = "";

static TimerResults S_timerResults;

// CWE ids used
static const CWE CWE398(398U);  // Indicator of Poor Code Quality

CppCheck::CppCheck(ErrorLogger &errorLogger, bool useGlobalSuppressions)
    : _errorLogger(errorLogger), exitcode(0), _useGlobalSuppressions(useGlobalSuppressions), tooManyConfigs(false), _simplify(true)
{
}

CppCheck::~CppCheck()
{
    while (!fileInfo.empty()) {
        delete fileInfo.back();
        fileInfo.pop_back();
    }
    S_timerResults.ShowResults(_settings.showtime);
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
    std::ifstream fin(path.c_str());
    return processFile(Path::simplifyPath(path), emptyString, fin);
}

unsigned int CppCheck::check(const std::string &path, const std::string &content)
{
    std::istringstream iss(content);
    return processFile(Path::simplifyPath(path), emptyString, iss);
}

unsigned int CppCheck::check(const ImportProject::FileSettings &fs)
{
    CppCheck temp(_errorLogger, _useGlobalSuppressions);
    temp._settings = _settings;
    if (!temp._settings.userDefines.empty())
        temp._settings.userDefines += ';';
    temp._settings.userDefines += fs.cppcheckDefines();
    temp._settings.includePaths = fs.includePaths;
    // TODO: temp._settings.userUndefs = fs.undefs;
    if (fs.platformType != Settings::Unspecified) {
        temp._settings.platform(fs.platformType);
    }
    std::ifstream fin(fs.filename.c_str());
    return temp.processFile(Path::simplifyPath(fs.filename), fs.cfg, fin);
}

unsigned int CppCheck::processFile(const std::string& filename, const std::string &cfgname, std::istream& fileStream)
{
    exitcode = 0;

    // only show debug warnings for accepted C/C++ source files
    if (!Path::acceptFile(filename))
        _settings.debugwarnings = false;

    if (_settings.terminated())
        return exitcode;

    if (!_settings.quiet) {
        std::string fixedpath = Path::simplifyPath(filename);
        fixedpath = Path::toNativeSeparators(fixedpath);
        _errorLogger.reportOut(std::string("Checking ") + fixedpath + ' ' + cfgname + std::string("..."));

        if (_settings.verbose) {
            _errorLogger.reportOut("Defines: " + _settings.userDefines);
            std::string includePaths;
            for (std::list<std::string>::const_iterator I = _settings.includePaths.begin(); I != _settings.includePaths.end(); ++I)
                includePaths += " -I" + *I;
            _errorLogger.reportOut("Includes:" + includePaths);
            _errorLogger.reportOut(std::string("Platform:") + _settings.platformString());
        }
    }

    if (plistFile.is_open()) {
        plistFile << ErrorLogger::plistFooter();
        plistFile.close();
    }

    CheckUnusedFunctions checkUnusedFunctions(nullptr, nullptr, nullptr);

    bool internalErrorFound(false);
    try {
        Preprocessor preprocessor(_settings, this);
        std::set<std::string> configurations;

        simplecpp::OutputList outputList;
        std::vector<std::string> files;
        simplecpp::TokenList tokens1(fileStream, files, filename, &outputList);

        // If there is a syntax error, report it and stop
        for (simplecpp::OutputList::const_iterator it = outputList.begin(); it != outputList.end(); ++it) {
            bool err;
            switch (it->type) {
            case simplecpp::Output::ERROR:
            case simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY:
            case simplecpp::Output::SYNTAX_ERROR:
            case simplecpp::Output::UNHANDLED_CHAR_ERROR:
                err = true;
                break;
            case simplecpp::Output::WARNING:
            case simplecpp::Output::MISSING_HEADER:
            case simplecpp::Output::PORTABILITY_BACKSLASH:
                err = false;
                break;
            };

            if (err) {
                const ErrorLogger::ErrorMessage::FileLocation loc1(it->location.file(), it->location.line);
                std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
                callstack.push_back(loc1);

                ErrorLogger::ErrorMessage errmsg(callstack,
                                                 "",
                                                 Severity::error,
                                                 it->msg,
                                                 "syntaxError",
                                                 false);
                _errorLogger.reportErr(errmsg);
                return 1;
            }
        }

        preprocessor.loadFiles(tokens1, files);

        if (!_settings.plistOutput.empty()) {
            std::string filename2;
            if (filename.find('/') != std::string::npos)
                filename2 = filename.substr(filename.rfind('/') + 1);
            else
                filename2 = filename;
            filename2 = _settings.plistOutput + filename2.substr(0, filename2.find('.')) + ".plist";
            plistFile.open(filename2);
            plistFile << ErrorLogger::plistHeader(version(), files);
        }

        // write dump file xml prolog
        std::ofstream fdump;
        if (_settings.dump) {
            const std::string dumpfile(_settings.dumpFile.empty() ? (filename + ".dump") : _settings.dumpFile);
            fdump.open(dumpfile.c_str());
            if (fdump.is_open()) {
                fdump << "<?xml version=\"1.0\"?>" << std::endl;
                fdump << "<dumps>" << std::endl;
                fdump << "  <platform"
                      << " name=\"" << _settings.platformString() << '\"'
                      << " char_bit=\"" << _settings.char_bit << '\"'
                      << " short_bit=\"" << _settings.short_bit << '\"'
                      << " int_bit=\"" << _settings.int_bit << '\"'
                      << " long_bit=\"" << _settings.long_bit << '\"'
                      << " long_long_bit=\"" << _settings.long_long_bit << '\"'
                      << " pointer_bit=\"" << (_settings.sizeof_pointer * _settings.char_bit) << '\"'
                      << "/>\n";
                fdump << "  <rawtokens>" << std::endl;
                for (unsigned int i = 0; i < files.size(); ++i)
                    fdump << "    <file index=\"" << i << "\" name=\"" << ErrorLogger::toxml(files[i]) << "\"/>" << std::endl;
                for (const simplecpp::Token *tok = tokens1.cfront(); tok; tok = tok->next) {
                    fdump << "    <tok "
                          << "fileIndex=\"" << tok->location.fileIndex << "\" "
                          << "linenr=\"" << tok->location.line << "\" "
                          << "str=\"" << ErrorLogger::toxml(tok->str) << "\""
                          << "/>" << std::endl;
                }
                fdump << "  </rawtokens>" << std::endl;
            }
        }

        // Parse comments and then remove them
        preprocessor.inlineSuppressions(tokens1);
        tokens1.removeComments();
        preprocessor.removeComments();

        if (!_settings.buildDir.empty()) {
            // Get toolinfo
            std::string toolinfo;
            toolinfo += CPPCHECK_VERSION_STRING;
            toolinfo += _settings.isEnabled(Settings::WARNING) ? 'w' : ' ';
            toolinfo += _settings.isEnabled(Settings::STYLE) ? 's' : ' ';
            toolinfo += _settings.isEnabled(Settings::PERFORMANCE) ? 'p' : ' ';
            toolinfo += _settings.isEnabled(Settings::PORTABILITY) ? 'p' : ' ';
            toolinfo += _settings.isEnabled(Settings::INFORMATION) ? 'i' : ' ';
            toolinfo += _settings.userDefines;

            // Calculate checksum so it can be compared with old checksum / future checksums
            const unsigned int checksum = preprocessor.calculateChecksum(tokens1, toolinfo);
            std::list<ErrorLogger::ErrorMessage> errors;
            if (!analyzerInformation.analyzeFile(_settings.buildDir, filename, cfgname, checksum, &errors)) {
                while (!errors.empty()) {
                    reportErr(errors.front());
                    errors.pop_front();
                }
                return exitcode;  // known results => no need to reanalyze file
            }
        }

        // Get directives
        preprocessor.setDirectives(tokens1);
        preprocessor.simplifyPragmaAsm(&tokens1);

        preprocessor.setPlatformInfo(&tokens1);

        // Get configurations..
        if (_settings.userDefines.empty() || _settings.force) {
            Timer t("Preprocessor::getConfigs", _settings.showtime, &S_timerResults);
            configurations = preprocessor.getConfigs(tokens1);
        } else {
            configurations.insert(_settings.userDefines);
        }

        if (_settings.checkConfiguration) {
            for (std::set<std::string>::const_iterator it = configurations.begin(); it != configurations.end(); ++it)
                (void)preprocessor.getcode(tokens1, *it, files, true);

            return 0;
        }

        // Run define rules on raw code
        for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
            if (it->tokenlist != "define")
                continue;

            std::string code;
            const std::list<Directive> &directives = preprocessor.getDirectives();
            for (std::list<Directive>::const_iterator dir = directives.begin(); dir != directives.end(); ++dir) {
                if (dir->str.compare(0,8,"#define ") == 0)
                    code += "#line " + MathLib::toString(dir->linenr) + " \"" + dir->file + "\"\n" + dir->str + '\n';
            }
            Tokenizer tokenizer2(&_settings, this);
            std::istringstream istr2(code);
            tokenizer2.list.createTokens(istr2);
            executeRules("define", tokenizer2);
            break;
        }

        if (!_settings.force && configurations.size() > _settings.maxConfigs) {
            if (_settings.isEnabled(Settings::INFORMATION)) {
                tooManyConfigsError(Path::toNativeSeparators(filename),configurations.size());
            } else {
                tooManyConfigs = true;
            }
        }

        std::set<unsigned long long> checksums;
        unsigned int checkCount = 0;
        for (std::set<std::string>::const_iterator it = configurations.begin(); it != configurations.end(); ++it) {
            // bail out if terminated
            if (_settings.terminated())
                break;

            // Check only a few configurations (default 12), after that bail out, unless --force
            // was used.
            if (!_settings.force && ++checkCount > _settings.maxConfigs)
                break;

            cfg = *it;

            // If only errors are printed, print filename after the check
            if (!_settings.quiet && (!cfg.empty() || it != configurations.begin())) {
                std::string fixedpath = Path::simplifyPath(filename);
                fixedpath = Path::toNativeSeparators(fixedpath);
                _errorLogger.reportOut("Checking " + fixedpath + ": " + cfg + "...");
            }

            if (!_settings.userDefines.empty()) {
                if (!cfg.empty())
                    cfg = ";" + cfg;
                cfg = _settings.userDefines + cfg;
            }

            if (_settings.preprocessOnly) {
                Timer t("Preprocessor::getcode", _settings.showtime, &S_timerResults);
                std::string codeWithoutCfg = preprocessor.getcode(tokens1, cfg, files, true);
                t.Stop();

                if (codeWithoutCfg.compare(0,5,"#file") == 0)
                    codeWithoutCfg.insert(0U, "//");
                std::string::size_type pos = 0;
                while ((pos = codeWithoutCfg.find("\n#file",pos)) != std::string::npos)
                    codeWithoutCfg.insert(pos+1U, "//");
                pos = 0;
                while ((pos = codeWithoutCfg.find("\n#endfile",pos)) != std::string::npos)
                    codeWithoutCfg.insert(pos+1U, "//");
                pos = 0;
                while ((pos = codeWithoutCfg.find(Preprocessor::macroChar,pos)) != std::string::npos)
                    codeWithoutCfg[pos] = ' ';
                reportOut(codeWithoutCfg);
                continue;
            }

            Tokenizer _tokenizer(&_settings, this);
            if (_settings.showtime != SHOWTIME_NONE)
                _tokenizer.setTimerResults(&S_timerResults);

            try {
                bool result;

                // Create tokens, skip rest of iteration if failed
                Timer timer("Tokenizer::createTokens", _settings.showtime, &S_timerResults);
                const simplecpp::TokenList &tokensP = preprocessor.preprocess(tokens1, cfg, files);
                _tokenizer.createTokens(&tokensP);
                timer.Stop();
                if (tokensP.empty())
                    continue;

                // skip rest of iteration if just checking configuration
                if (_settings.checkConfiguration)
                    continue;

                // Check raw tokens
                checkRawTokens(_tokenizer);

                // Simplify tokens into normal form, skip rest of iteration if failed
                Timer timer2("Tokenizer::simplifyTokens1", _settings.showtime, &S_timerResults);
                result = _tokenizer.simplifyTokens1(cfg);
                timer2.Stop();
                if (!result)
                    continue;

                // dump xml if --dump
                if (_settings.dump && fdump.is_open()) {
                    fdump << "<dump cfg=\"" << ErrorLogger::toxml(cfg) << "\">" << std::endl;
                    preprocessor.dump(fdump);
                    _tokenizer.dump(fdump);
                    fdump << "</dump>" << std::endl;
                }

                // Skip if we already met the same simplified token list
                if (_settings.force || _settings.maxConfigs > 1) {
                    const unsigned long long checksum = _tokenizer.list.calculateChecksum();
                    if (checksums.find(checksum) != checksums.end()) {
                        if (_settings.isEnabled(Settings::INFORMATION) && (_settings.debug || _settings.verbose))
                            purgedConfigurationMessage(filename, cfg);
                        continue;
                    }
                    checksums.insert(checksum);
                }

                // Check normal tokens
                checkNormalTokens(_tokenizer);

                // Analyze info..
                if (!_settings.buildDir.empty())
                    checkUnusedFunctions.parseTokens(_tokenizer, filename.c_str(), &_settings, false);

                // simplify more if required, skip rest of iteration if failed
                if (_simplify) {
                    // if further simplification fails then skip rest of iteration
                    Timer timer3("Tokenizer::simplifyTokenList2", _settings.showtime, &S_timerResults);
                    result = _tokenizer.simplifyTokenList2();
                    timer3.Stop();
                    if (!result)
                        continue;

                    // Check simplified tokens
                    checkSimplifiedTokens(_tokenizer);
                }

            } catch (const InternalError &e) {
                internalErrorFound=true;
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                if (e.token) {
                    loc.line = e.token->linenr();
                    const std::string fixedpath = Path::toNativeSeparators(_tokenizer.list.file(e.token));
                    loc.setfile(fixedpath);
                } else {
                    ErrorLogger::ErrorMessage::FileLocation loc2;
                    loc2.setfile(Path::toNativeSeparators(filename));
                    locationList.push_back(loc2);
                    loc.setfile(_tokenizer.list.getSourceFilePath());
                }
                locationList.push_back(loc);
                ErrorLogger::ErrorMessage errmsg(locationList,
                                                 _tokenizer.list.getSourceFilePath(),
                                                 Severity::error,
                                                 e.errorMessage,
                                                 e.id,
                                                 false);

                reportErr(errmsg);
            }
        }

        // dumped all configs, close root </dumps> element now
        if (_settings.dump && fdump.is_open())
            fdump << "</dumps>" << std::endl;

    } catch (const std::runtime_error &e) {
        internalError(filename, e.what());
    } catch (const std::bad_alloc &e) {
        internalError(filename, e.what());
    } catch (const InternalError &e) {
        internalError(filename, e.errorMessage);
        exitcode=1; // e.g. reflect a syntax error
    }

    analyzerInformation.setFileInfo("CheckUnusedFunctions", checkUnusedFunctions.analyzerInfo());
    analyzerInformation.close();

    // In jointSuppressionReport mode, unmatched suppressions are
    // collected after all files are processed
    if (!_settings.jointSuppressionReport && (_settings.isEnabled(Settings::INFORMATION) || _settings.checkConfiguration)) {
        reportUnmatchedSuppressions(_settings.nomsg.getUnmatchedLocalSuppressions(filename, isUnusedFunctionCheckEnabled()));
    }

    _errorList.clear();
    if (internalErrorFound && (exitcode==0)) {
        exitcode=1;
    }
    return exitcode;
}

void CppCheck::internalError(const std::string &filename, const std::string &msg)
{
    const std::string fixedpath = Path::toNativeSeparators(filename);
    const std::string fullmsg("Bailing out from checking " + fixedpath + " since there was an internal error: " + msg);

    if (_settings.isEnabled(Settings::INFORMATION)) {
        const ErrorLogger::ErrorMessage::FileLocation loc1(filename, 0);
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        callstack.push_back(loc1);

        ErrorLogger::ErrorMessage errmsg(callstack,
                                         emptyString,
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

//---------------------------------------------------------------------------
// CppCheck - A function that checks a raw token list
//---------------------------------------------------------------------------
void CppCheck::checkRawTokens(const Tokenizer &tokenizer)
{
    // Execute rules for "raw" code
    executeRules("raw", tokenizer);
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a normal token list
//---------------------------------------------------------------------------

void CppCheck::checkNormalTokens(const Tokenizer &tokenizer)
{
    // call all "runChecks" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        if (_settings.terminated())
            return;

        if (tokenizer.isMaxTime())
            return;

        Timer timerRunChecks((*it)->name() + "::runChecks", _settings.showtime, &S_timerResults);
        (*it)->runChecks(&tokenizer, &_settings, this);
    }

    // Analyse the tokens..
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        Check::FileInfo *fi = (*it)->getFileInfo(&tokenizer, &_settings);
        if (fi != nullptr) {
            fileInfo.push_back(fi);
            analyzerInformation.setFileInfo((*it)->name(), fi->toString());
        }
    }

    executeRules("normal", tokenizer);
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a simplified token list
//---------------------------------------------------------------------------

void CppCheck::checkSimplifiedTokens(const Tokenizer &tokenizer)
{
    // call all "runSimplifiedChecks" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        if (_settings.terminated())
            return;

        if (tokenizer.isMaxTime())
            return;

        Timer timerSimpleChecks((*it)->name() + "::runSimplifiedChecks", _settings.showtime, &S_timerResults);
        (*it)->runSimplifiedChecks(&tokenizer, &_settings, this);
        timerSimpleChecks.Stop();
    }

    if (!_settings.terminated())
        executeRules("simple", tokenizer);
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
        if (rule.pattern.empty() || rule.id.empty() || rule.severity == Severity::none || rule.tokenlist != tokenlist)
            continue;

        const char *error = nullptr;
        int erroffset = 0;
        pcre *re = pcre_compile(rule.pattern.c_str(),0,&error,&erroffset,nullptr);
        if (!re) {
            if (error) {
                ErrorLogger::ErrorMessage errmsg(std::list<ErrorLogger::ErrorMessage::FileLocation>(),
                                                 emptyString,
                                                 Severity::error,
                                                 error,
                                                 "pcre_compile",
                                                 false);

                reportErr(errmsg);
            }
            continue;
        }

        int pos = 0;
        int ovector[30]= {0};
        while (pos < (int)str.size() && 0 <= pcre_exec(re, nullptr, str.c_str(), (int)str.size(), pos, 0, ovector, 30)) {
            const unsigned int pos1 = (unsigned int)ovector[0];
            const unsigned int pos2 = (unsigned int)ovector[1];

            // jump to the end of the match for the next pcre_exec
            pos = (int)pos2;

            // determine location..
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.setfile(tokenizer.list.getSourceFilePath());
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
            const ErrorLogger::ErrorMessage errmsg(callStack, tokenizer.list.getSourceFilePath(), rule.severity, summary, rule.id, false);

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
    if (!_settings.isEnabled(Settings::INFORMATION) && !tooManyConfigs)
        return;

    tooManyConfigs = false;

    if (_settings.isEnabled(Settings::INFORMATION) && file.empty())
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        ErrorLogger::ErrorMessage::FileLocation location;
        location.setfile(file);
        loclist.push_back(location);
    }

    std::ostringstream msg;
    msg << "Too many #ifdef configurations - cppcheck only checks " << _settings.maxConfigs;
    if (numberOfConfigurations > _settings.maxConfigs)
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
                                     emptyString,
                                     Severity::information,
                                     msg.str(),
                                     "toomanyconfigs", CWE398,
                                     false);

    reportErr(errmsg);
}

void CppCheck::purgedConfigurationMessage(const std::string &file, const std::string& configuration)
{
    tooManyConfigs = false;

    if (_settings.isEnabled(Settings::INFORMATION) && file.empty())
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        ErrorLogger::ErrorMessage::FileLocation location;
        location.setfile(file);
        loclist.push_back(location);
    }

    ErrorLogger::ErrorMessage errmsg(loclist,
                                     emptyString,
                                     Severity::information,
                                     "The configuration '" + configuration + "' was not checked because its code equals another one.",
                                     "purgedConfiguration",
                                     false);

    reportErr(errmsg);
}

//---------------------------------------------------------------------------

void CppCheck::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    if (!_settings.library.reportErrors(msg.file0))
        return;

    const std::string errmsg = msg.toString(_settings.verbose);
    if (errmsg.empty())
        return;

    // Alert only about unique errors
    if (std::find(_errorList.begin(), _errorList.end(), errmsg) != _errorList.end())
        return;

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

    if (!_settings.nofail.isSuppressed(msg._id, file, line) && !_settings.nomsg.isSuppressed(msg._id, file, line))
        exitcode = 1;

    _errorList.push_back(errmsg);

    _errorLogger.reportErr(msg);
    analyzerInformation.reportErr(msg, _settings.verbose);
    if (!_settings.plistOutput.empty() && plistFile.is_open()) {
        plistFile << ErrorLogger::plistData(msg);
    }
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
    Settings s(_settings);
    s.addEnabled("warning");
    s.addEnabled("style");
    s.addEnabled("portability");
    s.addEnabled("performance");
    s.addEnabled("information");

    tooManyConfigs = true;
    tooManyConfigsError("",0U);

    // call all "getErrorMessages" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
        (*it)->getErrorMessages(this, &s);

    Preprocessor::getErrorMessages(this, &s);
}

bool CppCheck::analyseWholeProgram()
{
    bool errors = false;
    // Analyse the tokens
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
        errors |= (*it)->analyseWholeProgram(fileInfo, _settings, *this);
    return errors;
}

void CppCheck::analyseWholeProgram(const std::string &buildDir, const std::map<std::string, std::size_t> &files)
{
    (void)files;
    if (buildDir.empty())
        return;
    if (_settings.isEnabled(Settings::UNUSED_FUNCTION))
        CheckUnusedFunctions::analyseWholeProgram(this, buildDir);
    std::list<Check::FileInfo*> fileInfoList;

    // Load all analyzer info data..
    const std::string filesTxt(buildDir + "/files.txt");
    std::ifstream fin(filesTxt.c_str());
    std::string filesTxtLine;
    while (std::getline(fin, filesTxtLine)) {
        const std::string::size_type firstColon = filesTxtLine.find(':');
        if (firstColon == std::string::npos)
            continue;
        const std::string::size_type lastColon = filesTxtLine.rfind(':');
        if (firstColon == lastColon)
            continue;
        const std::string xmlfile = buildDir + '/' + filesTxtLine.substr(0,firstColon);
        //const std::string sourcefile = filesTxtLine.substr(lastColon+1);

        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError error = doc.LoadFile(xmlfile.c_str());
        if (error != tinyxml2::XML_SUCCESS)
            continue;

        const tinyxml2::XMLElement * const rootNode = doc.FirstChildElement();
        if (rootNode == nullptr)
            continue;

        for (const tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e; e = e->NextSiblingElement()) {
            if (std::strcmp(e->Name(), "FileInfo") != 0)
                continue;
            const char *checkClassAttr = e->Attribute("check");
            if (!checkClassAttr)
                continue;
            for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
                if (checkClassAttr == (*it)->name())
                    fileInfoList.push_back((*it)->loadFileInfoFromXml(e));
            }
        }
    }

    // Analyse the tokens
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
        (*it)->analyseWholeProgram(fileInfoList, _settings, *this);

    for (std::list<Check::FileInfo*>::iterator fi = fileInfoList.begin(); fi != fileInfoList.end(); ++fi)
        delete (*fi);
}

bool CppCheck::isUnusedFunctionCheckEnabled() const
{
    return (_settings.jobs == 1 && _settings.isEnabled(Settings::UNUSED_FUNCTION));
}

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
    : mErrorLogger(errorLogger), mExitCode(0), mSuppressInternalErrorFound(false), mUseGlobalSuppressions(useGlobalSuppressions), mTooManyConfigs(false), mSimplify(true)
{
}

CppCheck::~CppCheck()
{
    while (!mFileInfo.empty()) {
        delete mFileInfo.back();
        mFileInfo.pop_back();
    }
    S_timerResults.ShowResults(mSettings.showtime);
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
    std::ifstream fin(path);
    return checkFile(Path::simplifyPath(path), emptyString, fin);
}

unsigned int CppCheck::check(const std::string &path, const std::string &content)
{
    std::istringstream iss(content);
    return checkFile(Path::simplifyPath(path), emptyString, iss);
}

unsigned int CppCheck::check(const ImportProject::FileSettings &fs)
{
    CppCheck temp(mErrorLogger, mUseGlobalSuppressions);
    temp.mSettings = mSettings;
    if (!temp.mSettings.userDefines.empty())
        temp.mSettings.userDefines += ';';
    temp.mSettings.userDefines += fs.cppcheckDefines();
    temp.mSettings.includePaths = fs.includePaths;
    temp.mSettings.userUndefs = fs.undefs;
    if (fs.platformType != Settings::Unspecified) {
        temp.mSettings.platform(fs.platformType);
    }
    std::ifstream fin(fs.filename);
    return temp.checkFile(Path::simplifyPath(fs.filename), fs.cfg, fin);
}

unsigned int CppCheck::checkFile(const std::string& filename, const std::string &cfgname, std::istream& fileStream)
{
    mExitCode = 0;
    mSuppressInternalErrorFound = false;

    // only show debug warnings for accepted C/C++ source files
    if (!Path::acceptFile(filename))
        mSettings.debugwarnings = false;

    if (mSettings.terminated())
        return mExitCode;

    if (!mSettings.quiet) {
        std::string fixedpath = Path::simplifyPath(filename);
        fixedpath = Path::toNativeSeparators(fixedpath);
        mErrorLogger.reportOut(std::string("Checking ") + fixedpath + ' ' + cfgname + std::string("..."));

        if (mSettings.verbose) {
            mErrorLogger.reportOut("Defines:" + mSettings.userDefines);
            std::string undefs;
            for (const std::string& U : mSettings.userUndefs) {
                if (!undefs.empty())
                    undefs += ';';
                undefs += ' ' + U;
            }
            mErrorLogger.reportOut("Undefines:" + undefs);
            std::string includePaths;
            for (const std::string &I : mSettings.includePaths)
                includePaths += " -I" + I;
            mErrorLogger.reportOut("Includes:" + includePaths);
            mErrorLogger.reportOut(std::string("Platform:") + mSettings.platformString());
        }
    }

    if (plistFile.is_open()) {
        plistFile << ErrorLogger::plistFooter();
        plistFile.close();
    }

    CheckUnusedFunctions checkUnusedFunctions(nullptr, nullptr, nullptr);

    bool internalErrorFound(false);
    try {
        Preprocessor preprocessor(mSettings, this);
        std::set<std::string> configurations;

        simplecpp::OutputList outputList;
        std::vector<std::string> files;
        simplecpp::TokenList tokens1(fileStream, files, filename, &outputList);

        // If there is a syntax error, report it and stop
        for (const simplecpp::Output &output : outputList) {
            bool err;
            switch (output.type) {
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
                const ErrorLogger::ErrorMessage::FileLocation loc1(output.location.file(), output.location.line);
                std::list<ErrorLogger::ErrorMessage::FileLocation> callstack(1, loc1);

                ErrorLogger::ErrorMessage errmsg(callstack,
                                                 "",
                                                 Severity::error,
                                                 output.msg,
                                                 "syntaxError",
                                                 false);
                reportErr(errmsg);
                return mExitCode;
            }
        }

        preprocessor.loadFiles(tokens1, files);

        if (!mSettings.plistOutput.empty()) {
            std::string filename2;
            if (filename.find('/') != std::string::npos)
                filename2 = filename.substr(filename.rfind('/') + 1);
            else
                filename2 = filename;
            filename2 = mSettings.plistOutput + filename2.substr(0, filename2.find('.')) + ".plist";
            plistFile.open(filename2);
            plistFile << ErrorLogger::plistHeader(version(), files);
        }

        // write dump file xml prolog
        std::ofstream fdump;
        if (mSettings.dump) {
            const std::string dumpfile(mSettings.dumpFile.empty() ? (filename + ".dump") : mSettings.dumpFile);
            fdump.open(dumpfile);
            if (fdump.is_open()) {
                fdump << "<?xml version=\"1.0\"?>" << std::endl;
                fdump << "<dumps>" << std::endl;
                fdump << "  <platform"
                      << " name=\"" << mSettings.platformString() << '\"'
                      << " char_bit=\"" << mSettings.char_bit << '\"'
                      << " short_bit=\"" << mSettings.short_bit << '\"'
                      << " int_bit=\"" << mSettings.int_bit << '\"'
                      << " long_bit=\"" << mSettings.long_bit << '\"'
                      << " long_long_bit=\"" << mSettings.long_long_bit << '\"'
                      << " pointer_bit=\"" << (mSettings.sizeof_pointer * mSettings.char_bit) << '\"'
                      << "/>\n";
                fdump << "  <rawtokens>" << std::endl;
                for (unsigned int i = 0; i < files.size(); ++i)
                    fdump << "    <file index=\"" << i << "\" name=\"" << ErrorLogger::toxml(files[i]) << "\"/>" << std::endl;
                for (const simplecpp::Token *tok = tokens1.cfront(); tok; tok = tok->next) {
                    fdump << "    <tok "
                          << "fileIndex=\"" << tok->location.fileIndex << "\" "
                          << "linenr=\"" << tok->location.line << "\" "
                          << "str=\"" << ErrorLogger::toxml(tok->str()) << "\""
                          << "/>" << std::endl;
                }
                fdump << "  </rawtokens>" << std::endl;
            }
        }

        // Parse comments and then remove them
        preprocessor.inlineSuppressions(tokens1);
        if (mSettings.dump && fdump.is_open()) {
            mSettings.nomsg.dump(fdump);
        }
        tokens1.removeComments();
        preprocessor.removeComments();

        if (!mSettings.buildDir.empty()) {
            // Get toolinfo
            std::ostringstream toolinfo;
            toolinfo << CPPCHECK_VERSION_STRING;
            toolinfo << (mSettings.isEnabled(Settings::WARNING) ? 'w' : ' ');
            toolinfo << (mSettings.isEnabled(Settings::STYLE) ? 's' : ' ');
            toolinfo << (mSettings.isEnabled(Settings::PERFORMANCE) ? 'p' : ' ');
            toolinfo << (mSettings.isEnabled(Settings::PORTABILITY) ? 'p' : ' ');
            toolinfo << (mSettings.isEnabled(Settings::INFORMATION) ? 'i' : ' ');
            toolinfo << mSettings.userDefines;
            mSettings.nomsg.dump(toolinfo);

            // Calculate checksum so it can be compared with old checksum / future checksums
            const unsigned int checksum = preprocessor.calculateChecksum(tokens1, toolinfo.str());
            std::list<ErrorLogger::ErrorMessage> errors;
            if (!mAnalyzerInformation.analyzeFile(mSettings.buildDir, filename, cfgname, checksum, &errors)) {
                while (!errors.empty()) {
                    reportErr(errors.front());
                    errors.pop_front();
                }
                return mExitCode;  // known results => no need to reanalyze file
            }
        }

        // Get directives
        preprocessor.setDirectives(tokens1);
        preprocessor.simplifyPragmaAsm(&tokens1);

        preprocessor.setPlatformInfo(&tokens1);

        // Get configurations..
        if (mSettings.userDefines.empty() || mSettings.force) {
            Timer t("Preprocessor::getConfigs", mSettings.showtime, &S_timerResults);
            configurations = preprocessor.getConfigs(tokens1);
        } else {
            configurations.insert(mSettings.userDefines);
        }

        if (mSettings.checkConfiguration) {
            for (const std::string &config : configurations)
                (void)preprocessor.getcode(tokens1, config, files, true);

            return 0;
        }

        // Run define rules on raw code
        for (const Settings::Rule &rule : mSettings.rules) {
            if (rule.tokenlist != "define")
                continue;

            std::string code;
            const std::list<Directive> &directives = preprocessor.getDirectives();
            for (const Directive &dir : directives) {
                if (dir.str.compare(0,8,"#define ") == 0)
                    code += "#line " + MathLib::toString(dir.linenr) + " \"" + dir.file + "\"\n" + dir.str + '\n';
            }
            Tokenizer tokenizer2(&mSettings, this);
            std::istringstream istr2(code);
            tokenizer2.list.createTokens(istr2);
            executeRules("define", tokenizer2);
            break;
        }

        if (!mSettings.force && configurations.size() > mSettings.maxConfigs) {
            if (mSettings.isEnabled(Settings::INFORMATION)) {
                tooManyConfigsError(Path::toNativeSeparators(filename),configurations.size());
            } else {
                mTooManyConfigs = true;
            }
        }

        std::set<unsigned long long> checksums;
        unsigned int checkCount = 0;
        bool hasValidConfig = false;
        std::list<std::string> configurationError;
        for (const std::string &currCfg : configurations) {
            // bail out if terminated
            if (mSettings.terminated())
                break;

            // Check only a few configurations (default 12), after that bail out, unless --force
            // was used.
            if (!mSettings.force && ++checkCount > mSettings.maxConfigs)
                break;

            mCurrentConfig = currCfg;

            if (!mSettings.userDefines.empty()) {
                if (!mCurrentConfig.empty())
                    mCurrentConfig = ";" + mCurrentConfig;
                mCurrentConfig = mSettings.userDefines + mCurrentConfig;
            }

            if (mSettings.preprocessOnly) {
                Timer t("Preprocessor::getcode", mSettings.showtime, &S_timerResults);
                std::string codeWithoutCfg = preprocessor.getcode(tokens1, mCurrentConfig, files, true);
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

            Tokenizer mTokenizer(&mSettings, this);
            if (mSettings.showtime != SHOWTIME_NONE)
                mTokenizer.setTimerResults(&S_timerResults);

            try {
                bool result;

                // Create tokens, skip rest of iteration if failed
                Timer timer("Tokenizer::createTokens", mSettings.showtime, &S_timerResults);
                const simplecpp::TokenList &tokensP = preprocessor.preprocess(tokens1, mCurrentConfig, files, true);
                mTokenizer.createTokens(&tokensP);
                timer.Stop();
                hasValidConfig = true;

                // If only errors are printed, print filename after the check
                if (!mSettings.quiet && (!mCurrentConfig.empty() || checkCount > 1)) {
                    std::string fixedpath = Path::simplifyPath(filename);
                    fixedpath = Path::toNativeSeparators(fixedpath);
                    mErrorLogger.reportOut("Checking " + fixedpath + ": " + mCurrentConfig + "...");
                }

                if (tokensP.empty())
                    continue;

                // skip rest of iteration if just checking configuration
                if (mSettings.checkConfiguration)
                    continue;

                // Check raw tokens
                checkRawTokens(mTokenizer);

                // Simplify tokens into normal form, skip rest of iteration if failed
                Timer timer2("Tokenizer::simplifyTokens1", mSettings.showtime, &S_timerResults);
                result = mTokenizer.simplifyTokens1(mCurrentConfig);
                timer2.Stop();
                if (!result)
                    continue;

                // dump xml if --dump
                if (mSettings.dump && fdump.is_open()) {
                    fdump << "<dump cfg=\"" << ErrorLogger::toxml(mCurrentConfig) << "\">" << std::endl;
                    preprocessor.dump(fdump);
                    mTokenizer.dump(fdump);
                    fdump << "</dump>" << std::endl;
                }

                // Skip if we already met the same simplified token list
                if (mSettings.force || mSettings.maxConfigs > 1) {
                    const unsigned long long checksum = mTokenizer.list.calculateChecksum();
                    if (checksums.find(checksum) != checksums.end()) {
                        if (mSettings.debugwarnings)
                            purgedConfigurationMessage(filename, mCurrentConfig);
                        continue;
                    }
                    checksums.insert(checksum);
                }

                // Check normal tokens
                checkNormalTokens(mTokenizer);

                // Analyze info..
                if (!mSettings.buildDir.empty())
                    checkUnusedFunctions.parseTokens(mTokenizer, filename.c_str(), &mSettings);

                // simplify more if required, skip rest of iteration if failed
                if (mSimplify) {
                    // if further simplification fails then skip rest of iteration
                    Timer timer3("Tokenizer::simplifyTokenList2", mSettings.showtime, &S_timerResults);
                    result = mTokenizer.simplifyTokenList2();
                    timer3.Stop();
                    if (!result)
                        continue;

                    // Check simplified tokens
                    checkSimplifiedTokens(mTokenizer);
                }

            } catch (const simplecpp::Output &o) {
                // #error etc during preprocessing
                configurationError.push_back((mCurrentConfig.empty() ? "\'\'" : mCurrentConfig) + " : [" + o.location.file() + ':' + MathLib::toString(o.location.line) + "] " + o.msg);
                --checkCount; // don't count invalid configurations
                continue;

            } catch (const InternalError &e) {
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                if (e.token) {
                    loc.line = e.token->linenr();
                    const std::string fixedpath = Path::toNativeSeparators(mTokenizer.list.file(e.token));
                    loc.setfile(fixedpath);
                } else {
                    ErrorLogger::ErrorMessage::FileLocation loc2;
                    loc2.setfile(Path::toNativeSeparators(filename));
                    locationList.push_back(loc2);
                    loc.setfile(mTokenizer.list.getSourceFilePath());
                }
                locationList.push_back(loc);
                ErrorLogger::ErrorMessage errmsg(locationList,
                                                 mTokenizer.list.getSourceFilePath(),
                                                 Severity::error,
                                                 e.errorMessage,
                                                 e.id,
                                                 false);

                if (errmsg._severity == Severity::error || mSettings.isEnabled(errmsg._severity)) {
                    reportErr(errmsg);
                    if (!mSuppressInternalErrorFound)
                        internalErrorFound = true;
                }
            }
        }

        if (!hasValidConfig && configurations.size() > 1 && mSettings.isEnabled(Settings::INFORMATION)) {
            std::string msg;
            msg = "This file is not analyzed. Cppcheck failed to extract a valid configuration. Use -v for more details.";
            msg += "\nThis file is not analyzed. Cppcheck failed to extract a valid configuration. The tested configurations have these preprocessor errors:";
            for (const std::string &s : configurationError)
                msg += '\n' + s;

            std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.setfile(Path::toNativeSeparators(filename));
            locationList.push_back(loc);
            ErrorLogger::ErrorMessage errmsg(locationList,
                                             loc.getfile(),
                                             Severity::information,
                                             msg,
                                             "noValidConfiguration",
                                             false);
            reportErr(errmsg);
        }

        // dumped all configs, close root </dumps> element now
        if (mSettings.dump && fdump.is_open())
            fdump << "</dumps>" << std::endl;

    } catch (const std::runtime_error &e) {
        internalError(filename, e.what());
    } catch (const std::bad_alloc &e) {
        internalError(filename, e.what());
    } catch (const InternalError &e) {
        internalError(filename, e.errorMessage);
        mExitCode=1; // e.g. reflect a syntax error
    }

    mAnalyzerInformation.setFileInfo("CheckUnusedFunctions", checkUnusedFunctions.analyzerInfo());
    mAnalyzerInformation.close();

    // In jointSuppressionReport mode, unmatched suppressions are
    // collected after all files are processed
    if (!mSettings.jointSuppressionReport && (mSettings.isEnabled(Settings::INFORMATION) || mSettings.checkConfiguration)) {
        reportUnmatchedSuppressions(mSettings.nomsg.getUnmatchedLocalSuppressions(filename, isUnusedFunctionCheckEnabled()));
    }

    mErrorList.clear();
    if (internalErrorFound && (mExitCode==0)) {
        mExitCode = 1;
    }

    return mExitCode;
}

void CppCheck::internalError(const std::string &filename, const std::string &msg)
{
    const std::string fixedpath = Path::toNativeSeparators(filename);
    const std::string fullmsg("Bailing out from checking " + fixedpath + " since there was an internal error: " + msg);

    if (mSettings.isEnabled(Settings::INFORMATION)) {
        const ErrorLogger::ErrorMessage::FileLocation loc1(filename, 0);
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack(1, loc1);

        ErrorLogger::ErrorMessage errmsg(callstack,
                                         emptyString,
                                         Severity::information,
                                         fullmsg,
                                         "internalError",
                                         false);

        mErrorLogger.reportErr(errmsg);
    } else {
        // Report on stdout
        mErrorLogger.reportOut(fullmsg);
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
        if (mSettings.terminated())
            return;

        if (tokenizer.isMaxTime())
            return;

        Timer timerRunChecks((*it)->name() + "::runChecks", mSettings.showtime, &S_timerResults);
        (*it)->runChecks(&tokenizer, &mSettings, this);
    }

    // Analyse the tokens..
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        Check::FileInfo *fi = (*it)->getFileInfo(&tokenizer, &mSettings);
        if (fi != nullptr) {
            mFileInfo.push_back(fi);
            mAnalyzerInformation.setFileInfo((*it)->name(), fi->toString());
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
        if (mSettings.terminated())
            return;

        if (tokenizer.isMaxTime())
            return;

        Timer timerSimpleChecks((*it)->name() + "::runSimplifiedChecks", mSettings.showtime, &S_timerResults);
        (*it)->runSimplifiedChecks(&tokenizer, &mSettings, this);
        timerSimpleChecks.Stop();
    }

    if (!mSettings.terminated())
        executeRules("simple", tokenizer);
}

#ifdef HAVE_RULES

static const char * pcreErrorCodeToString(const int pcreExecRet)
{
    switch (pcreExecRet) {
    case PCRE_ERROR_NULL:
        return "Either code or subject was passed as NULL, or ovector was NULL "
               "and ovecsize was not zero (PCRE_ERROR_NULL)";
    case PCRE_ERROR_BADOPTION:
        return "An unrecognized bit was set in the options argument (PCRE_ERROR_BADOPTION)";
    case PCRE_ERROR_BADMAGIC:
        return "PCRE stores a 4-byte \"magic number\" at the start of the compiled code, "
               "to catch the case when it is passed a junk pointer and to detect when a "
               "pattern that was compiled in an environment of one endianness is run in "
               "an environment with the other endianness. This is the error that PCRE "
               "gives when the magic number is not present (PCRE_ERROR_BADMAGIC)";
    case PCRE_ERROR_UNKNOWN_NODE:
        return "While running the pattern match, an unknown item was encountered in the "
               "compiled pattern. This error could be caused by a bug in PCRE or by "
               "overwriting of the compiled pattern (PCRE_ERROR_UNKNOWN_NODE)";
    case PCRE_ERROR_NOMEMORY:
        return "If a pattern contains back references, but the ovector that is passed "
               "to pcre_exec() is not big enough to remember the referenced substrings, "
               "PCRE gets a block of memory at the start of matching to use for this purpose. "
               "If the call via pcre_malloc() fails, this error is given. The memory is "
               "automatically freed at the end of matching. This error is also given if "
               "pcre_stack_malloc() fails in pcre_exec(). "
               "This can happen only when PCRE has been compiled with "
               "--disable-stack-for-recursion (PCRE_ERROR_NOMEMORY)";
    case PCRE_ERROR_NOSUBSTRING:
        return "This error is used by the pcre_copy_substring(), pcre_get_substring(), "
               "and pcre_get_substring_list() functions (see below). "
               "It is never returned by pcre_exec() (PCRE_ERROR_NOSUBSTRING)";
    case PCRE_ERROR_MATCHLIMIT:
        return "The backtracking limit, as specified by the match_limit field in a pcre_extra "
               "structure (or defaulted) was reached. "
               "See the description above (PCRE_ERROR_MATCHLIMIT)";
    case PCRE_ERROR_CALLOUT:
        return "This error is never generated by pcre_exec() itself. "
               "It is provided for use by callout functions that want to yield a distinctive "
               "error code. See the pcrecallout documentation for details (PCRE_ERROR_CALLOUT)";
    case PCRE_ERROR_BADUTF8:
        return "A string that contains an invalid UTF-8 byte sequence was passed as a subject, "
               "and the PCRE_NO_UTF8_CHECK option was not set. If the size of the output vector "
               "(ovecsize) is at least 2, the byte offset to the start of the the invalid UTF-8 "
               "character is placed in the first element, and a reason code is placed in the "
               "second element. The reason codes are listed in the following section. For "
               "backward compatibility, if PCRE_PARTIAL_HARD is set and the problem is a truncated "
               "UTF-8 character at the end of the subject (reason codes 1 to 5), "
               "PCRE_ERROR_SHORTUTF8 is returned instead of PCRE_ERROR_BADUTF8";
    case PCRE_ERROR_BADUTF8_OFFSET:
        return "The UTF-8 byte sequence that was passed as a subject was checked and found to "
               "be valid (the PCRE_NO_UTF8_CHECK option was not set), but the value of "
               "startoffset did not point to the beginning of a UTF-8 character or the end of "
               "the subject (PCRE_ERROR_BADUTF8_OFFSET)";
    case PCRE_ERROR_PARTIAL:
        return "The subject string did not match, but it did match partially. See the "
               "pcrepartial documentation for details of partial matching (PCRE_ERROR_PARTIAL)";
    case PCRE_ERROR_BADPARTIAL:
        return "This code is no longer in use. It was formerly returned when the PCRE_PARTIAL "
               "option was used with a compiled pattern containing items that were not supported "
               "for partial matching. From release 8.00 onwards, there are no restrictions on "
               "partial matching (PCRE_ERROR_BADPARTIAL)";
    case PCRE_ERROR_INTERNAL:
        return "An unexpected internal error has occurred. This error could be caused by a bug "
               "in PCRE or by overwriting of the compiled pattern (PCRE_ERROR_INTERNAL)";
    case PCRE_ERROR_BADCOUNT:
        return"This error is given if the value of the ovecsize argument is negative "
              "(PCRE_ERROR_BADCOUNT)";
    case PCRE_ERROR_RECURSIONLIMIT :
        return "The internal recursion limit, as specified by the match_limit_recursion "
               "field in a pcre_extra structure (or defaulted) was reached. "
               "See the description above (PCRE_ERROR_RECURSIONLIMIT)";
    case PCRE_ERROR_DFA_UITEM:
        return "PCRE_ERROR_DFA_UITEM";
    case PCRE_ERROR_DFA_UCOND:
        return "PCRE_ERROR_DFA_UCOND";
    case PCRE_ERROR_DFA_WSSIZE:
        return "PCRE_ERROR_DFA_WSSIZE";
    case PCRE_ERROR_DFA_RECURSE:
        return "PCRE_ERROR_DFA_RECURSE";
    case PCRE_ERROR_NULLWSLIMIT:
        return "PCRE_ERROR_NULLWSLIMIT";
    case PCRE_ERROR_BADNEWLINE:
        return "An invalid combination of PCRE_NEWLINE_xxx options was "
               "given (PCRE_ERROR_BADNEWLINE)";
    case PCRE_ERROR_BADOFFSET:
        return "The value of startoffset was negative or greater than the length "
               "of the subject, that is, the value in length (PCRE_ERROR_BADOFFSET)";
    case PCRE_ERROR_SHORTUTF8:
        return "This error is returned instead of PCRE_ERROR_BADUTF8 when the subject "
               "string ends with a truncated UTF-8 character and the PCRE_PARTIAL_HARD option is set. "
               "Information about the failure is returned as for PCRE_ERROR_BADUTF8. "
               "It is in fact sufficient to detect this case, but this special error code for "
               "PCRE_PARTIAL_HARD precedes the implementation of returned information; "
               "it is retained for backwards compatibility (PCRE_ERROR_SHORTUTF8)";
    case PCRE_ERROR_RECURSELOOP:
        return "This error is returned when pcre_exec() detects a recursion loop "
               "within the pattern. Specifically, it means that either the whole pattern "
               "or a subpattern has been called recursively for the second time at the same "
               "position in the subject string. Some simple patterns that might do this "
               "are detected and faulted at compile time, but more complicated cases, "
               "in particular mutual recursions between two different subpatterns, "
               "cannot be detected until run time (PCRE_ERROR_RECURSELOOP)";
    case PCRE_ERROR_JIT_STACKLIMIT:
        return "This error is returned when a pattern that was successfully studied "
               "using a JIT compile option is being matched, but the memory available "
               "for the just-in-time processing stack is not large enough. See the pcrejit "
               "documentation for more details (PCRE_ERROR_JIT_STACKLIMIT)";
    case PCRE_ERROR_BADMODE:
        return "This error is given if a pattern that was compiled by the 8-bit library "
               "is passed to a 16-bit or 32-bit library function, or vice versa (PCRE_ERROR_BADMODE)";
    case PCRE_ERROR_BADENDIANNESS:
        return "This error is given if a pattern that was compiled and saved is reloaded on a "
               "host with different endianness. The utility function pcre_pattern_to_host_byte_order() "
               "can be used to convert such a pattern so that it runs on the new host (PCRE_ERROR_BADENDIANNESS)";
    case PCRE_ERROR_DFA_BADRESTART:
        return "PCRE_ERROR_DFA_BADRESTART";
#if PCRE_MAJOR >= 8 && PCRE_MINOR >= 32
    case PCRE_ERROR_BADLENGTH:
        return "This error is given if pcre_exec() is called with a negative value for the length argument (PCRE_ERROR_BADLENGTH)";
    case PCRE_ERROR_JIT_BADOPTION:
        return "This error is returned when a pattern that was successfully studied using a JIT compile "
               "option is being matched, but the matching mode (partial or complete match) does not correspond "
               "to any JIT compilation mode. When the JIT fast path function is used, this error may be "
               "also given for invalid options. See the pcrejit documentation for more details (PCRE_ERROR_JIT_BADOPTION)";
#endif
    }
    return "";
}

#endif // HAVE_RULES


void CppCheck::executeRules(const std::string &tokenlist, const Tokenizer &tokenizer)
{
    (void)tokenlist;
    (void)tokenizer;

#ifdef HAVE_RULES
    // Are there rules to execute?
    bool isrule = false;
    for (std::list<Settings::Rule>::const_iterator it = mSettings.rules.begin(); it != mSettings.rules.end(); ++it) {
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

    for (std::list<Settings::Rule>::const_iterator it = mSettings.rules.begin(); it != mSettings.rules.end(); ++it) {
        const Settings::Rule &rule = *it;
        if (rule.pattern.empty() || rule.id.empty() || rule.severity == Severity::none || rule.tokenlist != tokenlist)
            continue;

        const char *pcreCompileErrorStr = nullptr;
        int erroffset = 0;
        pcre * const re = pcre_compile(rule.pattern.c_str(),0,&pcreCompileErrorStr,&erroffset,nullptr);
        if (!re) {
            if (pcreCompileErrorStr) {
                const std::string msg = "pcre_compile failed: " + std::string(pcreCompileErrorStr);
                const ErrorLogger::ErrorMessage errmsg(std::list<ErrorLogger::ErrorMessage::FileLocation>(),
                                                       emptyString,
                                                       Severity::error,
                                                       msg,
                                                       "pcre_compile",
                                                       false);

                reportErr(errmsg);
            }
            continue;
        }

        // Optimize the regex, but only if PCRE_CONFIG_JIT is available
#ifdef PCRE_CONFIG_JIT
        const char *pcreStudyErrorStr = nullptr;
        pcre_extra * const pcreExtra = pcre_study(re, PCRE_STUDY_JIT_COMPILE, &pcreStudyErrorStr);
        // pcre_study() returns NULL for both errors and when it can not optimize the regex.
        // The last argument is how one checks for errors.
        // It is NULL if everything works, and points to an error string otherwise.
        if (pcreStudyErrorStr) {
            const std::string msg = "pcre_study failed: " + std::string(pcreStudyErrorStr);
            const ErrorLogger::ErrorMessage errmsg(std::list<ErrorLogger::ErrorMessage::FileLocation>(),
                                                   emptyString,
                                                   Severity::error,
                                                   msg,
                                                   "pcre_study",
                                                   false);

            reportErr(errmsg);
            // pcre_compile() worked, but pcre_study() returned an error. Free the resources allocated by pcre_compile().
            pcre_free(re);
            continue;
        }
#else
        const pcre_extra * const pcreExtra = nullptr;
#endif

        int pos = 0;
        int ovector[30]= {0};
        while (pos < (int)str.size()) {
            const int pcreExecRet = pcre_exec(re, pcreExtra, str.c_str(), (int)str.size(), pos, 0, ovector, 30);
            if (pcreExecRet < 0) {
                const std::string errorMessage = pcreErrorCodeToString(pcreExecRet);
                if (!errorMessage.empty()) {
                    const ErrorLogger::ErrorMessage errmsg(std::list<ErrorLogger::ErrorMessage::FileLocation>(),
                                                           emptyString,
                                                           Severity::error,
                                                           std::string("pcre_exec failed: ") + errorMessage,
                                                           "pcre_exec",
                                                           false);

                    reportErr(errmsg);
                }
                break;
            }
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
#ifdef PCRE_CONFIG_JIT
        // Free up the EXTRA PCRE value (may be NULL at this point)
        if (pcreExtra) {
            pcre_free_study(pcreExtra);
        }
#endif
    }
#endif
}

Settings &CppCheck::settings()
{
    return mSettings;
}

void CppCheck::tooManyConfigsError(const std::string &file, const std::size_t numberOfConfigurations)
{
    if (!mSettings.isEnabled(Settings::INFORMATION) && !mTooManyConfigs)
        return;

    mTooManyConfigs = false;

    if (mSettings.isEnabled(Settings::INFORMATION) && file.empty())
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        ErrorLogger::ErrorMessage::FileLocation location;
        location.setfile(file);
        loclist.push_back(location);
    }

    std::ostringstream msg;
    msg << "Too many #ifdef configurations - cppcheck only checks " << mSettings.maxConfigs;
    if (numberOfConfigurations > mSettings.maxConfigs)
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
    mTooManyConfigs = false;

    if (mSettings.isEnabled(Settings::INFORMATION) && file.empty())
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
    mSuppressInternalErrorFound = false;

    if (!mSettings.library.reportErrors(msg.file0))
        return;

    const std::string errmsg = msg.toString(mSettings.verbose);
    if (errmsg.empty())
        return;

    // Alert only about unique errors
    if (std::find(mErrorList.begin(), mErrorList.end(), errmsg) != mErrorList.end())
        return;

    const Suppressions::ErrorMessage errorMessage = msg.toSuppressionsErrorMessage();

    if (mUseGlobalSuppressions) {
        if (mSettings.nomsg.isSuppressed(errorMessage)) {
            mSuppressInternalErrorFound = true;
            return;
        }
    } else {
        if (mSettings.nomsg.isSuppressedLocal(errorMessage)) {
            mSuppressInternalErrorFound = true;
            return;
        }
    }

    if (!mSettings.nofail.isSuppressed(errorMessage) && (mUseGlobalSuppressions || !mSettings.nomsg.isSuppressed(errorMessage)))
        mExitCode = 1;

    mErrorList.push_back(errmsg);

    mErrorLogger.reportErr(msg);
    mAnalyzerInformation.reportErr(msg, mSettings.verbose);
    if (!mSettings.plistOutput.empty() && plistFile.is_open()) {
        plistFile << ErrorLogger::plistData(msg);
    }
}

void CppCheck::reportOut(const std::string &outmsg)
{
    mErrorLogger.reportOut(outmsg);
}

void CppCheck::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    mErrorLogger.reportProgress(filename, stage, value);
}

void CppCheck::reportInfo(const ErrorLogger::ErrorMessage &msg)
{
    const Suppressions::ErrorMessage &errorMessage = msg.toSuppressionsErrorMessage();
    if (!mSettings.nomsg.isSuppressed(errorMessage))
        mErrorLogger.reportInfo(msg);
}

void CppCheck::reportStatus(unsigned int /*fileindex*/, unsigned int /*filecount*/, std::size_t /*sizedone*/, std::size_t /*sizetotal*/)
{

}

void CppCheck::getErrorMessages()
{
    Settings s(mSettings);
    s.addEnabled("warning");
    s.addEnabled("style");
    s.addEnabled("portability");
    s.addEnabled("performance");
    s.addEnabled("information");

    purgedConfigurationMessage("","");

    mTooManyConfigs = true;
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
        errors |= (*it)->analyseWholeProgram(mFileInfo, mSettings, *this);
    return errors && (mExitCode > 0);
}

void CppCheck::analyseWholeProgram(const std::string &buildDir, const std::map<std::string, std::size_t> &files)
{
    (void)files;
    if (buildDir.empty())
        return;
    if (mSettings.isEnabled(Settings::UNUSED_FUNCTION))
        CheckUnusedFunctions::analyseWholeProgram(this, buildDir);
    std::list<Check::FileInfo*> fileInfoList;

    // Load all analyzer info data..
    const std::string filesTxt(buildDir + "/files.txt");
    std::ifstream fin(filesTxt);
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
        const tinyxml2::XMLError error = doc.LoadFile(xmlfile.c_str());
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
        (*it)->analyseWholeProgram(fileInfoList, mSettings, *this);

    for (std::list<Check::FileInfo*>::iterator fi = fileInfoList.begin(); fi != fileInfoList.end(); ++fi)
        delete (*fi);
}

bool CppCheck::isUnusedFunctionCheckEnabled() const
{
    return (mSettings.jobs == 1 && mSettings.isEnabled(Settings::UNUSED_FUNCTION));
}

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

#include "cppcheckexecutor.h"

#include "analyzerinfo.h"
#include "checkers.h"
#include "checkersreport.h"
#include "cmdlineparser.h"
#include "color.h"
#include "config.h"
#include "cppcheck.h"
#include "errortypes.h"
#include "filelister.h"
#include "importproject.h"
#include "library.h"
#include "path.h"
#include "pathmatch.h"
#include "settings.h"
#include "singleexecutor.h"
#include "suppressions.h"
#include "utils.h"

#include "checkunusedfunctions.h"

#if defined(THREADING_MODEL_THREAD)
#include "threadexecutor.h"
#elif defined(THREADING_MODEL_FORK)
#include "processexecutor.h"
#endif

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <sstream> // IWYU pragma: keep
#include <utility>
#include <vector>

#ifdef USE_UNIX_SIGNAL_HANDLING
#include "cppcheckexecutorsig.h"
#endif

#ifdef USE_WINDOWS_SEH
#include "cppcheckexecutorseh.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

class XMLErrorMessagesLogger : public ErrorLogger
{
    void reportOut(const std::string & outmsg, Color /*c*/ = Color::Reset) override
    {
        std::cout << outmsg << std::endl;
    }

    void reportErr(const ErrorMessage &msg) override
    {
        reportOut(msg.toXML());
    }

    void reportProgress(const std::string & /*filename*/, const char /*stage*/[], const std::size_t /*value*/) override
    {}
};

// TODO: do not directly write to stdout

/*static*/ FILE* CppCheckExecutor::mExceptionOutput = stdout;

CppCheckExecutor::~CppCheckExecutor()
{
    delete mErrorOutput;
}

bool CppCheckExecutor::parseFromArgs(Settings &settings, int argc, const char* const argv[])
{
    CmdLineParser parser(settings, settings.nomsg, settings.nofail);
    const bool success = parser.parseFromArgs(argc, argv);

    if (success) {
        if (parser.getShowVersion() && !parser.getShowErrorMessages()) {
            if (!settings.cppcheckCfgProductName.empty()) {
                std::cout << settings.cppcheckCfgProductName << std::endl;
            } else {
                const char * const extraVersion = CppCheck::extraVersion();
                if (*extraVersion != 0)
                    std::cout << "Cppcheck " << CppCheck::version() << " ("
                              << extraVersion << ')' << std::endl;
                else
                    std::cout << "Cppcheck " << CppCheck::version() << std::endl;
            }
        }

        if (parser.getShowErrorMessages()) {
            XMLErrorMessagesLogger xmlLogger;
            std::cout << ErrorMessage::getXMLHeader(settings.cppcheckCfgProductName);
            CppCheck::getErrorMessages(xmlLogger);
            std::cout << ErrorMessage::getXMLFooter() << std::endl;
        }

        if (parser.exitAfterPrinting()) {
            Settings::terminate();
            return true;
        }
    } else {
        return false;
    }

    // Libraries must be loaded before FileLister is executed to ensure markup files will be
    // listed properly.
    if (!loadLibraries(settings))
        return false;

    // Check that all include paths exist
    {
        for (std::list<std::string>::iterator iter = settings.includePaths.begin();
             iter != settings.includePaths.end();
             ) {
            const std::string path(Path::toNativeSeparators(*iter));
            if (Path::isDirectory(path))
                ++iter;
            else {
                // If the include path is not found, warn user and remove the non-existing path from the list.
                if (settings.severity.isEnabled(Severity::information))
                    std::cout << "(information) Couldn't find path given by -I '" << path << '\'' << std::endl;
                iter = settings.includePaths.erase(iter);
            }
        }
    }

    // Output a warning for the user if he tries to exclude headers
    const std::vector<std::string>& ignored = parser.getIgnoredPaths();
    const bool warn = std::any_of(ignored.cbegin(), ignored.cend(), [](const std::string& i) {
        return Path::isHeader(i);
    });
    if (warn) {
        std::cout << "cppcheck: filename exclusion does not apply to header (.h and .hpp) files." << std::endl;
        std::cout << "cppcheck: Please use --suppress for ignoring results from the header files." << std::endl;
    }

    const std::vector<std::string>& pathnames = parser.getPathNames();

#if defined(_WIN32)
    // For Windows we want case-insensitive path matching
    const bool caseSensitive = false;
#else
    const bool caseSensitive = true;
#endif
    if (!settings.project.fileSettings.empty() && !settings.fileFilters.empty()) {
        // filter only for the selected filenames from all project files
        std::list<ImportProject::FileSettings> newList;

        const std::list<ImportProject::FileSettings>& fileSettings = settings.project.fileSettings;
        std::copy_if(fileSettings.cbegin(), fileSettings.cend(), std::back_inserter(newList), [&](const ImportProject::FileSettings& fs) {
            return matchglobs(settings.fileFilters, fs.filename);
        });
        if (!newList.empty())
            settings.project.fileSettings = newList;
        else {
            std::cout << "cppcheck: error: could not find any files matching the filter." << std::endl;
            return false;
        }
    } else if (!pathnames.empty()) {
        // Execute recursiveAddFiles() to each given file parameter
        const PathMatch matcher(ignored, caseSensitive);
        for (const std::string &pathname : pathnames) {
            std::string err = FileLister::recursiveAddFiles(mFiles, Path::toNativeSeparators(pathname), settings.library.markupExtensions(), matcher);
            if (!err.empty()) {
                std::cout << "cppcheck: " << err << std::endl;
            }
        }
    }

    if (mFiles.empty() && settings.project.fileSettings.empty()) {
        std::cout << "cppcheck: error: could not find or open any of the paths given." << std::endl;
        if (!ignored.empty())
            std::cout << "cppcheck: Maybe all paths were ignored?" << std::endl;
        return false;
    }
    if (!settings.fileFilters.empty() && settings.project.fileSettings.empty()) {
        std::map<std::string, std::size_t> newMap;
        for (std::map<std::string, std::size_t>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i)
            if (matchglobs(settings.fileFilters, i->first)) {
                newMap[i->first] = i->second;
            }
        mFiles = newMap;
        if (mFiles.empty()) {
            std::cout << "cppcheck: error: could not find any files matching the filter." << std::endl;
            return false;
        }

    }

    return true;
}

int CppCheckExecutor::check(int argc, const char* const argv[])
{
    CheckUnusedFunctions::clear();

    Settings settings;
    if (!parseFromArgs(settings, argc, argv)) {
        return EXIT_FAILURE;
    }
    if (Settings::terminated()) {
        return EXIT_SUCCESS;
    }

    CppCheck cppCheck(*this, true, executeCommand);
    cppCheck.settings() = settings;
    mSettings = &settings;

    int ret;
    if (settings.exceptionHandling)
        ret = check_wrapper(cppCheck);
    else
        ret = check_internal(cppCheck);

    mSettings = nullptr;
    return ret;
}

int CppCheckExecutor::check_wrapper(CppCheck& cppcheck)
{
#ifdef USE_WINDOWS_SEH
    return check_wrapper_seh(*this, &CppCheckExecutor::check_internal, cppcheck);
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    return check_wrapper_sig(*this, &CppCheckExecutor::check_internal, cppcheck);
#else
    return check_internal(cppcheck);
#endif
}

bool CppCheckExecutor::reportSuppressions(const Settings &settings, bool unusedFunctionCheckEnabled, const std::map<std::string, std::size_t> &files, ErrorLogger& errorLogger) {
    const auto& suppressions = settings.nomsg.getSuppressions();
    if (std::any_of(suppressions.begin(), suppressions.end(), [](const Suppressions::Suppression& s) {
        return s.errorId == "unmatchedSuppression" && s.fileName.empty() && s.lineNumber == Suppressions::Suppression::NO_LINE;
    }))
        return false;

    bool err = false;
    if (settings.useSingleJob()) {
        for (std::map<std::string, std::size_t>::const_iterator i = files.cbegin(); i != files.cend(); ++i) {
            err |= Suppressions::reportUnmatchedSuppressions(
                settings.nomsg.getUnmatchedLocalSuppressions(i->first, unusedFunctionCheckEnabled), errorLogger);
        }
    }
    err |= Suppressions::reportUnmatchedSuppressions(settings.nomsg.getUnmatchedGlobalSuppressions(unusedFunctionCheckEnabled), errorLogger);
    return err;
}

/*
 * That is a method which gets called from check_wrapper
 * */
int CppCheckExecutor::check_internal(CppCheck& cppcheck)
{
    Settings& settings = cppcheck.settings();

    if (settings.reportProgress >= 0)
        mLatestProgressOutputTime = std::time(nullptr);

    if (!settings.outputFile.empty()) {
        mErrorOutput = new std::ofstream(settings.outputFile);
    }

    if (settings.xml) {
        reportErr(ErrorMessage::getXMLHeader(settings.cppcheckCfgProductName));
    }

    if (!settings.buildDir.empty()) {
        settings.loadSummaries();

        std::list<std::string> fileNames;
        for (std::map<std::string, std::size_t>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i)
            fileNames.emplace_back(i->first);
        AnalyzerInformation::writeFilesTxt(settings.buildDir, fileNames, settings.userDefines, settings.project.fileSettings);
    }

    if (!settings.checkersReportFilename.empty())
        std::remove(settings.checkersReportFilename.c_str());

    unsigned int returnValue = 0;
    if (settings.useSingleJob()) {
        // Single process
        SingleExecutor executor(cppcheck, mFiles, settings, settings.nomsg, *this);
        returnValue = executor.check();
    } else {
#if defined(THREADING_MODEL_THREAD)
        ThreadExecutor executor(mFiles, settings, settings.nomsg, *this);
#elif defined(THREADING_MODEL_FORK)
        ProcessExecutor executor(mFiles, settings, settings.nomsg, *this);
#endif
        returnValue = executor.check();
    }

    cppcheck.analyseWholeProgram(settings.buildDir, mFiles);

    if (settings.severity.isEnabled(Severity::information) || settings.checkConfiguration) {
        const bool err = reportSuppressions(settings, cppcheck.isUnusedFunctionCheckEnabled(), mFiles, *this);
        if (err && returnValue == 0)
            returnValue = settings.exitCode;
    }

    if (!settings.checkConfiguration) {
        cppcheck.tooManyConfigsError(emptyString,0U);
    }

    if (settings.xml) {
        reportErr(ErrorMessage::getXMLFooter());
    }

    writeCheckersReport(settings);

    if (returnValue)
        return settings.exitCode;
    return 0;
}

void CppCheckExecutor::writeCheckersReport(const Settings& settings) const
{
    CheckersReport checkersReport(settings, mActiveCheckers);

    if (!settings.quiet) {
        const int activeCheckers = checkersReport.getActiveCheckersCount();
        const int totalCheckers = checkersReport.getAllCheckersCount();

        const std::string extra = settings.verbose ? " (use --checkers-report=<filename> to see details)" : "";
        if (mCriticalErrors.empty())
            std::cout << "Active checkers: " << activeCheckers << "/" << totalCheckers << extra << std::endl;
        else
            std::cout << "Active checkers: There was critical errors" << extra << std::endl;
    }

    if (settings.checkersReportFilename.empty())
        return;

    std::ofstream fout(settings.checkersReportFilename);
    if (fout.is_open())
        fout << checkersReport.getReport(mCriticalErrors);

}

bool CppCheckExecutor::loadLibraries(Settings& settings)
{
    if (!tryLoadLibrary(settings.library, settings.exename, "std.cfg")) {
        const std::string msg("Failed to load std.cfg. Your Cppcheck installation is broken, please re-install.");
#ifdef FILESDIR
        const std::string details("The Cppcheck binary was compiled with FILESDIR set to \""
                                  FILESDIR "\" and will therefore search for "
                                  "std.cfg in " FILESDIR "/cfg.");
#else
        const std::string cfgfolder(Path::fromNativeSeparators(Path::getPathFromFilename(settings.exename)) + "cfg");
        const std::string details("The Cppcheck binary was compiled without FILESDIR set. Either the "
                                  "std.cfg should be available in " + cfgfolder + " or the FILESDIR "
                                  "should be configured.");
#endif
        std::cout << msg << " " << details << std::endl;
        return false;
    }

    bool result = true;
    for (const auto& lib : settings.libraries) {
        if (!tryLoadLibrary(settings.library, settings.exename, lib.c_str())) {
            result = false;
        }
    }
    return result;
}

#ifdef _WIN32
// fix trac ticket #439 'Cppcheck reports wrong filename for filenames containing 8-bit ASCII'
static inline std::string ansiToOEM(const std::string &msg, bool doConvert)
{
    if (doConvert) {
        const unsigned msglength = msg.length();
        // convert ANSI strings to OEM strings in two steps
        std::vector<WCHAR> wcContainer(msglength);
        std::string result(msglength, '\0');

        // ansi code page characters to wide characters
        MultiByteToWideChar(CP_ACP, 0, msg.data(), msglength, wcContainer.data(), msglength);
        // wide characters to oem codepage characters
        WideCharToMultiByte(CP_OEMCP, 0, wcContainer.data(), msglength, const_cast<char *>(result.data()), msglength, nullptr, nullptr);

        return result; // hope for return value optimization
    }
    return msg;
}
#else
// no performance regression on non-windows systems
#define ansiToOEM(msg, doConvert) (msg)
#endif

void CppCheckExecutor::reportErr(const std::string &errmsg)
{
    if (mErrorOutput)
        *mErrorOutput << errmsg << std::endl;
    else {
        std::cerr << ansiToOEM(errmsg, (mSettings == nullptr) ? true : !mSettings->xml) << std::endl;
    }
}

void CppCheckExecutor::reportOut(const std::string &outmsg, Color c)
{
    if (c == Color::Reset)
        std::cout << ansiToOEM(outmsg, true) << std::endl;
    else
        std::cout << c << ansiToOEM(outmsg, true) << Color::Reset << std::endl;
}

// TODO: remove filename parameter?
void CppCheckExecutor::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    (void)filename;

    if (!mLatestProgressOutputTime)
        return;

    // Report progress messages every x seconds
    const std::time_t currentTime = std::time(nullptr);
    if (currentTime >= (mLatestProgressOutputTime + mSettings->reportProgress))
    {
        mLatestProgressOutputTime = currentTime;

        // format a progress message
        std::ostringstream ostr;
        ostr << "progress: "
             << stage
             << ' ' << value << '%';

        // Report progress message
        reportOut(ostr.str());
    }
}

void CppCheckExecutor::reportErr(const ErrorMessage &msg)
{
    assert(mSettings != nullptr);

    if (msg.severity == Severity::none && (msg.id == "logChecker" || endsWith(msg.id, "-logChecker"))) {
        const std::string& checker = msg.shortMessage();
        mActiveCheckers.emplace(checker);
        return;
    }

    // Alert only about unique errors
    if (!mShownErrors.insert(msg.toString(mSettings->verbose)).second)
        return;

    if (ErrorLogger::isCriticalErrorId(msg.id) && mCriticalErrors.find(msg.id) == std::string::npos) {
        if (!mCriticalErrors.empty())
            mCriticalErrors += ", ";
        mCriticalErrors += msg.id;
    }

    if (mSettings->xml)
        reportErr(msg.toXML());
    else
        reportErr(msg.toString(mSettings->verbose, mSettings->templateFormat, mSettings->templateLocation));
}

void CppCheckExecutor::setExceptionOutput(FILE* exceptionOutput)
{
    mExceptionOutput = exceptionOutput;
}

FILE* CppCheckExecutor::getExceptionOutput()
{
    return mExceptionOutput;
}

bool CppCheckExecutor::tryLoadLibrary(Library& destination, const std::string& basepath, const char* filename)
{
    const Library::Error err = destination.load(basepath.c_str(), filename);

    if (err.errorcode == Library::ErrorCode::UNKNOWN_ELEMENT)
        std::cout << "cppcheck: Found unknown elements in configuration file '" << filename << "': " << err.reason << std::endl;
    else if (err.errorcode != Library::ErrorCode::OK) {
        std::cout << "cppcheck: Failed to load library configuration file '" << filename << "'. ";
        switch (err.errorcode) {
        case Library::ErrorCode::OK:
            break;
        case Library::ErrorCode::FILE_NOT_FOUND:
            std::cout << "File not found";
            break;
        case Library::ErrorCode::BAD_XML:
            std::cout << "Bad XML";
            break;
        case Library::ErrorCode::UNKNOWN_ELEMENT:
            std::cout << "Unexpected element";
            break;
        case Library::ErrorCode::MISSING_ATTRIBUTE:
            std::cout << "Missing attribute";
            break;
        case Library::ErrorCode::BAD_ATTRIBUTE_VALUE:
            std::cout << "Bad attribute value";
            break;
        case Library::ErrorCode::UNSUPPORTED_FORMAT:
            std::cout << "File is of unsupported format version";
            break;
        case Library::ErrorCode::DUPLICATE_PLATFORM_TYPE:
            std::cout << "Duplicate platform type";
            break;
        case Library::ErrorCode::PLATFORM_TYPE_REDEFINED:
            std::cout << "Platform type redefined";
            break;
        }
        if (!err.reason.empty())
            std::cout << " '" + err.reason + "'";
        std::cout << std::endl;
        return false;
    }
    return true;
}

/**
 * Execute a shell command and read the output from it. Returns true if command terminated successfully.
 */
// cppcheck-suppress passedByValue - used as callback so we need to preserve the signature
// NOLINTNEXTLINE(performance-unnecessary-value-param) - used as callback so we need to preserve the signature
bool CppCheckExecutor::executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string &output_)
{
    output_.clear();

#ifdef _WIN32
    // Extra quoutes are needed in windows if filename has space
    if (exe.find(" ") != std::string::npos)
        exe = "\"" + exe + "\"";
#endif

    std::string joinedArgs;
    for (const std::string &arg : args) {
        if (!joinedArgs.empty())
            joinedArgs += " ";
        if (arg.find(' ') != std::string::npos)
            joinedArgs += '"' + arg + '"';
        else
            joinedArgs += arg;
    }

    const std::string cmd = exe + " " + joinedArgs + " " + redirect;

#ifdef _WIN32
    FILE* p = _popen(cmd.c_str(), "r");
#else
    FILE *p = popen(cmd.c_str(), "r");
#endif
    if (!p) {
        // TODO: read errno
        return false;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), p) != nullptr)
        output_ += buffer;

#ifdef _WIN32
    const int res = _pclose(p);
#else
    const int res = pclose(p);
#endif
    if (res == -1) { // error occured
        // TODO: read errno
        return false;
    }
    if (res != 0) { // process failed
        // TODO: need to get error details
        return false;
    }
    return true;
}


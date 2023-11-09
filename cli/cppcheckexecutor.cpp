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

#include "addoninfo.h"
#include "analyzerinfo.h"
#include "checkersreport.h"
#include "cmdlinelogger.h"
#include "cmdlineparser.h"
#include "color.h"
#include "config.h"
#include "cppcheck.h"
#include "errortypes.h"
#include "filelister.h"
#include "filesettings.h"
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
#include <iostream>
#include <iterator>
#include <list>
#include <sstream> // IWYU pragma: keep
#include <unordered_set>
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

class CmdLineLoggerStd : public CmdLineLogger
{
public:
    CmdLineLoggerStd() = default;

    void printMessage(const std::string &message) override
    {
        printRaw("cppcheck: " + message);
    }

    void printError(const std::string &message) override
    {
        printMessage("error: " + message);
    }

    void printRaw(const std::string &message) override
    {
        std::cout << message << std::endl;
    }
};

// TODO: do not directly write to stdout

#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
/*static*/ FILE* CppCheckExecutor::mExceptionOutput = stdout;
#endif

CppCheckExecutor::~CppCheckExecutor()
{
    delete mErrorOutput;
}

bool CppCheckExecutor::parseFromArgs(Settings &settings, int argc, const char* const argv[])
{
    CmdLineLoggerStd logger;
    CmdLineParser parser(logger, settings, settings.nomsg, settings.nofail);
    const bool success = parser.parseFromArgs(argc, argv);

    if (success) {
        if (parser.getShowVersion() && !parser.getShowErrorMessages()) {
            if (!settings.cppcheckCfgProductName.empty()) {
                logger.printRaw(settings.cppcheckCfgProductName);
            } else {
                const char * const extraVersion = CppCheck::extraVersion();
                if (*extraVersion != 0)
                    logger.printRaw(std::string("Cppcheck ") + CppCheck::version() + " ("+ extraVersion + ')');
                else
                    logger.printRaw(std::string("Cppcheck ") + CppCheck::version());
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

    if (!loadAddons(settings))
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
        logger.printMessage("filename exclusion does not apply to header (.h and .hpp) files.");
        logger.printMessage("Please use --suppress for ignoring results from the header files.");
    }

    const std::vector<std::string>& pathnamesRef = parser.getPathNames();
    const std::list<FileSettings>& fileSettingsRef = parser.getFileSettings();

    // the inputs can only be used exclusively - CmdLineParser should already handle this
    assert(!(!pathnamesRef.empty() && !fileSettingsRef.empty()));

    if (!fileSettingsRef.empty()) {
        std::list<FileSettings> fileSettings;
        if (!settings.fileFilters.empty()) {
            // filter only for the selected filenames from all project files
            std::copy_if(fileSettingsRef.cbegin(), fileSettingsRef.cend(), std::back_inserter(fileSettings), [&](const FileSettings &fs) {
                return matchglobs(settings.fileFilters, fs.filename);
            });
            if (fileSettings.empty()) {
                logger.printError("could not find any files matching the filter.");
                return false;
            }
        }
        else {
            fileSettings = fileSettingsRef;
        }

        // sort the markup last
        std::copy_if(fileSettings.cbegin(), fileSettings.cend(), std::back_inserter(mFileSettings), [&](const FileSettings &fs) {
            return !settings.library.markupFile(fs.filename) || !settings.library.processMarkupAfterCode(fs.filename);
        });

        std::copy_if(fileSettings.cbegin(), fileSettings.cend(), std::back_inserter(mFileSettings), [&](const FileSettings &fs) {
            return settings.library.markupFile(fs.filename) && settings.library.processMarkupAfterCode(fs.filename);
        });
    }

    if (!pathnamesRef.empty()) {
        std::list<std::pair<std::string, std::size_t>> filesResolved;
        // TODO: this needs to be inlined into PathMatch as it depends on the underlying filesystem
#if defined(_WIN32)
        // For Windows we want case-insensitive path matching
        const bool caseSensitive = false;
#else
        const bool caseSensitive = true;
#endif
        // Execute recursiveAddFiles() to each given file parameter
        const PathMatch matcher(ignored, caseSensitive);
        for (const std::string &pathname : pathnamesRef) {
            const std::string err = FileLister::recursiveAddFiles(filesResolved, Path::toNativeSeparators(pathname), settings.library.markupExtensions(), matcher);
            if (!err.empty()) {
                // TODO: bail out?
                logger.printMessage(err);
            }
        }

        std::list<std::pair<std::string, std::size_t>> files;
        if (!settings.fileFilters.empty()) {
            std::copy_if(filesResolved.cbegin(), filesResolved.cend(), std::inserter(files, files.end()), [&](const decltype(filesResolved)::value_type& entry) {
                return matchglobs(settings.fileFilters, entry.first);
            });
            if (files.empty()) {
                logger.printError("could not find any files matching the filter.");
                return false;
            }
        }
        else {
            files = std::move(filesResolved);
        }

        // sort the markup last
        std::copy_if(files.cbegin(), files.cend(), std::inserter(mFiles, mFiles.end()), [&](const decltype(files)::value_type& entry) {
            return !settings.library.markupFile(entry.first) || !settings.library.processMarkupAfterCode(entry.first);
        });

        std::copy_if(files.cbegin(), files.cend(), std::inserter(mFiles, mFiles.end()), [&](const decltype(files)::value_type& entry) {
            return settings.library.markupFile(entry.first) && settings.library.processMarkupAfterCode(entry.first);
        });
    }

    if (mFiles.empty() && mFileSettings.empty()) {
        logger.printError("could not find or open any of the paths given.");
        if (!ignored.empty())
            logger.printMessage("Maybe all paths were ignored?");
        return false;
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

    const int ret = check_wrapper(cppCheck);

    mSettings = nullptr;
    return ret;
}

int CppCheckExecutor::check_wrapper(CppCheck& cppcheck)
{
#ifdef USE_WINDOWS_SEH
    if (cppcheck.settings().exceptionHandling)
        return check_wrapper_seh(*this, &CppCheckExecutor::check_internal, cppcheck);
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    if (cppcheck.settings().exceptionHandling)
        return check_wrapper_sig(*this, &CppCheckExecutor::check_internal, cppcheck);
#endif
    return check_internal(cppcheck);
}

bool CppCheckExecutor::reportSuppressions(const Settings &settings, bool unusedFunctionCheckEnabled, const std::list<std::pair<std::string, std::size_t>> &files, ErrorLogger& errorLogger) {
    const auto& suppressions = settings.nomsg.getSuppressions();
    if (std::any_of(suppressions.begin(), suppressions.end(), [](const Suppressions::Suppression& s) {
        return s.errorId == "unmatchedSuppression" && s.fileName.empty() && s.lineNumber == Suppressions::Suppression::NO_LINE;
    }))
        return false;

    bool err = false;
    if (settings.useSingleJob()) {
        for (std::list<std::pair<std::string, std::size_t>>::const_iterator i = files.cbegin(); i != files.cend(); ++i) {
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
        for (std::list<std::pair<std::string, std::size_t>>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i)
            fileNames.emplace_back(i->first);
        AnalyzerInformation::writeFilesTxt(settings.buildDir, fileNames, settings.userDefines, mFileSettings);
    }

    if (!settings.checkersReportFilename.empty())
        std::remove(settings.checkersReportFilename.c_str());

    unsigned int returnValue = 0;
    if (settings.useSingleJob()) {
        // Single process
        SingleExecutor executor(cppcheck, mFiles, mFileSettings, settings, settings.nomsg, *this);
        returnValue = executor.check();
    } else {
#if defined(THREADING_MODEL_THREAD)
        ThreadExecutor executor(mFiles, mFileSettings, settings, settings.nomsg, *this, CppCheckExecutor::executeCommand);
#elif defined(THREADING_MODEL_FORK)
        ProcessExecutor executor(mFiles, mFileSettings, settings, settings.nomsg, *this, CppCheckExecutor::executeCommand);
#endif
        returnValue = executor.check();
    }

    cppcheck.analyseWholeProgram(settings.buildDir, mFiles, mFileSettings);

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

bool CppCheckExecutor::loadAddons(Settings& settings)
{
    bool result = true;
    for (const std::string &addon: settings.addons) {
        AddonInfo addonInfo;
        const std::string failedToGetAddonInfo = addonInfo.getAddonInfo(addon, settings.exename);
        if (!failedToGetAddonInfo.empty()) {
            std::cout << failedToGetAddonInfo << std::endl;
            result = false;
            continue;
        }
        settings.addonInfos.emplace_back(std::move(addonInfo));
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
        WideCharToMultiByte(CP_OEMCP, 0, wcContainer.data(), msglength, &result[0], msglength, nullptr, nullptr);

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

#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
void CppCheckExecutor::setExceptionOutput(FILE* exceptionOutput)
{
    mExceptionOutput = exceptionOutput;
}

FILE* CppCheckExecutor::getExceptionOutput()
{
    return mExceptionOutput;
}
#endif

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
// cppcheck-suppress passedByValueCallback - used as callback so we need to preserve the signature
// NOLINTNEXTLINE(performance-unnecessary-value-param) - used as callback so we need to preserve the signature
int CppCheckExecutor::executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string &output_)
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
    //std::cout << "invoking command '" << cmd << "'" << std::endl;
    if (!p) {
        // TODO: how to provide to caller?
        //const int err = errno;
        //std::cout << "popen() errno " << std::to_string(err) << std::endl;
        return -1;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), p) != nullptr)
        output_ += buffer;

#ifdef _WIN32
    const int res = _pclose(p);
#else
    const int res = pclose(p);
#endif
    if (res == -1) { // error occurred
        // TODO: how to provide to caller?
        //const int err = errno;
        //std::cout << "pclose() errno " << std::to_string(err) << std::endl;
        return res;
    }
#if !defined(WIN32) && !defined(__MINGW32__)
    if (WIFEXITED(res)) {
        return WEXITSTATUS(res);
    }
    if (WIFSIGNALED(res)) {
        return WTERMSIG(res);
    }
#endif
    return res;
}


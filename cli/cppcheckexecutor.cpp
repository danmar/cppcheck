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
#include "preprocessor.h"
#include "settings.h"
#include "suppressions.h"
#include "utils.h"
#include "checkunusedfunctions.h"

#if defined(THREADING_MODEL_THREAD)
#include "threadexecutor.h"
#elif defined(THREADING_MODEL_FORK)
#include "processexecutor.h"
#endif

#include <atomic>
#include <cstdio>
#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <sstream> // IWYU pragma: keep
#include <utility>
#include <vector>
#include <numeric>

#ifdef USE_UNIX_SIGNAL_HANDLING
#include "cppcheckexecutorsig.h"
#endif

#ifdef USE_WINDOWS_SEH
#include "cppcheckexecutorseh.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif


/*static*/ FILE* CppCheckExecutor::mExceptionOutput = stdout;

CppCheckExecutor::CppCheckExecutor()
    : mSettings(nullptr), mLatestProgressOutputTime(0), mErrorOutput(nullptr), mShowAllErrors(false)
{}

CppCheckExecutor::~CppCheckExecutor()
{
    delete mErrorOutput;
}

bool CppCheckExecutor::parseFromArgs(CppCheck *cppcheck, int argc, const char* const argv[])
{
    Settings& settings = cppcheck->settings();
    CmdLineParser parser(&settings);
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
            mShowAllErrors = true;
            std::cout << ErrorMessage::getXMLHeader(settings.cppcheckCfgProductName);
            cppcheck->getErrorMessages();
            std::cout << ErrorMessage::getXMLFooter() << std::endl;
        }

        if (parser.exitAfterPrinting()) {
            Settings::terminate();
            return true;
        }
    } else {
        return false;
    }

    // Check that all include paths exist
    {
        for (std::list<std::string>::iterator iter = settings.includePaths.begin();
             iter != settings.includePaths.end();
             ) {
            const std::string path(Path::toNativeSeparators(*iter));
            if (FileLister::isDirectory(path))
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
    if (!mSettings->project.fileSettings.empty() && !mSettings->fileFilters.empty()) {
        // filter only for the selected filenames from all project files
        std::list<ImportProject::FileSettings> newList;

        const std::list<ImportProject::FileSettings>& fileSettings = settings.project.fileSettings;
        std::copy_if(fileSettings.cbegin(), fileSettings.cend(), std::back_inserter(newList), [&](const ImportProject::FileSettings& fs) {
            return matchglobs(mSettings->fileFilters, fs.filename);
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
            std::string err = FileLister::recursiveAddFiles(mFiles, Path::toNativeSeparators(pathname), mSettings->library.markupExtensions(), matcher);
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
    } else if (!mSettings->fileFilters.empty() && settings.project.fileSettings.empty()) {
        std::map<std::string, std::size_t> newMap;
        for (std::map<std::string, std::size_t>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i)
            if (matchglobs(mSettings->fileFilters, i->first)) {
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
    Preprocessor::missingIncludeFlag = false;
    Preprocessor::missingSystemIncludeFlag = false;

    CheckUnusedFunctions::clear();

    CppCheck cppCheck(*this, true, executeCommand);

    const Settings& settings = cppCheck.settings();
    mSettings = &settings;

    if (!parseFromArgs(&cppCheck, argc, argv)) {
        mSettings = nullptr;
        return EXIT_FAILURE;
    }
    if (Settings::terminated()) {
        mSettings = nullptr;
        return EXIT_SUCCESS;
    }

    int ret;

    if (cppCheck.settings().exceptionHandling)
        ret = check_wrapper(cppCheck);
    else
        ret = check_internal(cppCheck);

    mSettings = nullptr;
    return ret;
}

void CppCheckExecutor::setSettings(const Settings &settings)
{
    mSettings = &settings;
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
    if (settings.jointSuppressionReport) {
        for (std::map<std::string, std::size_t>::const_iterator i = files.cbegin(); i != files.cend(); ++i) {
            err |= errorLogger.reportUnmatchedSuppressions(
                settings.nomsg.getUnmatchedLocalSuppressions(i->first, unusedFunctionCheckEnabled));
        }
    }
    err |= errorLogger.reportUnmatchedSuppressions(settings.nomsg.getUnmatchedGlobalSuppressions(unusedFunctionCheckEnabled));
    return err;
}

/*
 * That is a method which gets called from check_wrapper
 * */
int CppCheckExecutor::check_internal(CppCheck& cppcheck)
{
    Settings& settings = cppcheck.settings();
    mSettings = &settings;
    const bool std = tryLoadLibrary(settings.library, settings.exename, "std.cfg");

    for (const std::string &lib : settings.libraries) {
        if (!tryLoadLibrary(settings.library, settings.exename, lib.c_str())) {
            const std::string msg("Failed to load the library " + lib);
            const std::list<ErrorMessage::FileLocation> callstack;
            ErrorMessage errmsg(callstack, emptyString, Severity::information, msg, "failedToLoadCfg", Certainty::normal);
            reportErr(errmsg);
            return EXIT_FAILURE;
        }
    }

    if (!std) {
        const std::list<ErrorMessage::FileLocation> callstack;
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
        ErrorMessage errmsg(callstack, emptyString, Severity::information, msg+" "+details, "failedToLoadCfg", Certainty::normal);
        reportErr(errmsg);
        return EXIT_FAILURE;
    }

    if (settings.reportProgress)
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

    unsigned int returnValue = 0;
    if (settings.jobs == 1) {
        // Single process
        settings.jointSuppressionReport = true;

        const std::size_t totalfilesize = std::accumulate(mFiles.cbegin(), mFiles.cend(), std::size_t(0), [](std::size_t v, const std::pair<std::string, std::size_t>& f) {
            return v + f.second;
        });

        std::size_t processedsize = 0;
        unsigned int c = 0;
        if (settings.project.fileSettings.empty()) {
            for (std::map<std::string, std::size_t>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i) {
                if (!mSettings->library.markupFile(i->first)
                    || !mSettings->library.processMarkupAfterCode(i->first)) {
                    returnValue += cppcheck.check(i->first);
                    processedsize += i->second;
                    if (!settings.quiet)
                        reportStatus(c + 1, mFiles.size(), processedsize, totalfilesize);
                    c++;
                }
            }
        } else {
            // filesettings
            // check all files of the project
            for (const ImportProject::FileSettings &fs : settings.project.fileSettings) {
                returnValue += cppcheck.check(fs);
                ++c;
                if (!settings.quiet)
                    reportStatus(c, settings.project.fileSettings.size(), c, settings.project.fileSettings.size());
                if (settings.clangTidy)
                    cppcheck.analyseClangTidy(fs);
            }
        }

        // second loop to parse all markup files which may not work until all
        // c/cpp files have been parsed and checked
        for (std::map<std::string, std::size_t>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i) {
            if (mSettings->library.markupFile(i->first) && mSettings->library.processMarkupAfterCode(i->first)) {
                returnValue += cppcheck.check(i->first);
                processedsize += i->second;
                if (!settings.quiet)
                    reportStatus(c + 1, mFiles.size(), processedsize, totalfilesize);
                c++;
            }
        }
        if (cppcheck.analyseWholeProgram())
            returnValue++;
    } else {
#if defined(THREADING_MODEL_THREAD)
        ThreadExecutor executor(mFiles, settings, *this);
#elif defined(THREADING_MODEL_FORK)
        ProcessExecutor executor(mFiles, settings, *this);
#endif
        returnValue = executor.check();
    }

    cppcheck.analyseWholeProgram(mSettings->buildDir, mFiles);

    if (settings.severity.isEnabled(Severity::information) || settings.checkConfiguration) {
        const bool err = reportSuppressions(settings, cppcheck.isUnusedFunctionCheckEnabled(), mFiles, *this);
        if (err && returnValue == 0)
            returnValue = settings.exitCode;
    }

    if (!settings.checkConfiguration) {
        cppcheck.tooManyConfigsError(emptyString,0U);

        if (settings.checks.isEnabled(Checks::missingInclude) && (Preprocessor::missingIncludeFlag || Preprocessor::missingSystemIncludeFlag)) {
            const std::list<ErrorMessage::FileLocation> callStack;
            ErrorMessage msg(callStack,
                             emptyString,
                             Severity::information,
                             "Cppcheck cannot find all the include files (use --check-config for details)\n"
                             "Cppcheck cannot find all the include files. Cppcheck can check the code without the "
                             "include files found. But the results will probably be more accurate if all the include "
                             "files are found. Please check your project's include directories and add all of them "
                             "as include directories for Cppcheck. To see what files Cppcheck cannot find use "
                             "--check-config.",
                             "",
                             Certainty::normal);
            if (Preprocessor::missingIncludeFlag) {
                msg.id = "missingInclude";
                reportInfo(msg);
            }
            if (Preprocessor::missingSystemIncludeFlag) {
                msg.id = "missingIncludeSystem";
                reportInfo(msg);
            }
        }
    }

    if (settings.xml) {
        reportErr(ErrorMessage::getXMLFooter());
    }

    mSettings = nullptr;
    if (returnValue)
        return settings.exitCode;
    return 0;
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
    std::cout << c << ansiToOEM(outmsg, true) << Color::Reset << std::endl;
}

void CppCheckExecutor::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    (void)filename;

    if (!mLatestProgressOutputTime)
        return;

    // Report progress messages every 10 seconds
    const std::time_t currentTime = std::time(nullptr);
    if (currentTime >= (mLatestProgressOutputTime + 10)) {
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

void CppCheckExecutor::reportInfo(const ErrorMessage &msg)
{
    reportErr(msg);
}

void CppCheckExecutor::reportStatus(std::size_t fileindex, std::size_t filecount, std::size_t sizedone, std::size_t sizetotal)
{
    if (filecount > 1) {
        std::ostringstream oss;
        const long percentDone = (sizetotal > 0) ? static_cast<long>(static_cast<long double>(sizedone) / sizetotal * 100) : 0;
        oss << fileindex << '/' << filecount
            << " files checked " << percentDone
            << "% done";
        std::cout << Color::FgBlue << oss.str() << Color::Reset << std::endl;
    }
}

void CppCheckExecutor::reportErr(const ErrorMessage &msg)
{
    if (mShowAllErrors) {
        reportOut(msg.toXML());
        return;
    }

    // Alert only about unique errors
    if (!mShownErrors.insert(msg.toString(mSettings->verbose)).second)
        return;

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
bool CppCheckExecutor::executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string *output_)
{
    output_->clear();

    std::string joinedArgs;
    for (const std::string &arg : args) {
        if (!joinedArgs.empty())
            joinedArgs += " ";
        if (arg.find(" ") != std::string::npos)
            joinedArgs += '"' + arg + '"';
        else
            joinedArgs += arg;
    }

#ifdef _WIN32
    // Extra quoutes are needed in windows if filename has space
    if (exe.find(" ") != std::string::npos)
        exe = "\"" + exe + "\"";
    const std::string cmd = exe + " " + joinedArgs + " " + redirect;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
    const std::string cmd = exe + " " + joinedArgs + " " + redirect;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
#endif
    if (!pipe)
        return false;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
        *output_ += buffer;
    return true;
}


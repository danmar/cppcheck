/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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
#include "checkersreport.h"
#include "cmdlinelogger.h"
#include "cmdlineparser.h"
#include "color.h"
#include "config.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "settings.h"
#include "singleexecutor.h"
#include "suppressions.h"
#include "utils.h"

#if defined(HAS_THREADING_MODEL_THREAD)
#include "threadexecutor.h"
#endif
#if defined(HAS_THREADING_MODEL_FORK)
#include "processexecutor.h"
#endif

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef USE_UNIX_SIGNAL_HANDLING
#include "signalhandler.h"
#endif

#ifdef USE_WINDOWS_SEH
#include "cppcheckexecutorseh.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#if !defined(WIN32) && !defined(__MINGW32__)
#include <sys/wait.h> // WIFEXITED and friends
#endif

namespace {
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

    class StdLogger : public ErrorLogger
    {
    public:
        explicit StdLogger(const Settings& settings)
            : mSettings(settings)
        {
            if (!mSettings.outputFile.empty()) {
                mErrorOutput = new std::ofstream(settings.outputFile);
            }
        }

        ~StdLogger() override {
            delete mErrorOutput;
        }

        StdLogger(const StdLogger&) = delete;
        StdLogger& operator=(const SingleExecutor &) = delete;

        void resetLatestProgressOutputTime() {
            mLatestProgressOutputTime = std::time(nullptr);
        }

        /**
         * Helper function to print out errors. Appends a line change.
         * @param errmsg String printed to error stream
         */
        void reportErr(const std::string &errmsg);

        /**
         * @brief Write the checkers report
         */
        void writeCheckersReport();

        bool hasCriticalErrors() const {
            return !mCriticalErrors.empty();
        }

    private:
        /**
         * Information about progress is directed here. This should be
         * called by the CppCheck class only.
         *
         * @param outmsg Progress message e.g. "Checking main.cpp..."
         */
        void reportOut(const std::string &outmsg, Color c = Color::Reset) override;

        /** xml output of errors */
        void reportErr(const ErrorMessage &msg) override;

        void reportProgress(const std::string &filename, const char stage[], const std::size_t value) override;

        /**
         * Pointer to current settings; set while check() is running for reportError().
         */
        const Settings& mSettings;

        /**
         * Used to filter out duplicate error messages.
         */
        // TODO: store hashes instead of the full messages
        std::unordered_set<std::string> mShownErrors;

        /**
         * Report progress time
         */
        std::time_t mLatestProgressOutputTime{};

        /**
         * Error output
         */
        std::ofstream* mErrorOutput{};

        /**
         * Checkers that has been executed
         */
        std::set<std::string> mActiveCheckers;

        /**
         * True if there are critical errors
         */
        std::string mCriticalErrors;
    };
}

// TODO: do not directly write to stdout

#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
/*static*/ FILE* CppCheckExecutor::mExceptionOutput = stdout;
#endif

int CppCheckExecutor::check(int argc, const char* const argv[])
{
    Settings settings;
    CmdLineLoggerStd logger;
    CmdLineParser parser(logger, settings, settings.supprs);
    if (!parser.fillSettingsFromArgs(argc, argv)) {
        return EXIT_FAILURE;
    }
    if (Settings::terminated()) {
        return EXIT_SUCCESS;
    }

    settings.loadSummaries();

    mFiles = parser.getFiles();
    mFileSettings = parser.getFileSettings();

    settings.setMisraRuleTexts(executeCommand);

    const int ret = check_wrapper(settings);

    return ret;
}

int CppCheckExecutor::check_wrapper(const Settings& settings)
{
#ifdef USE_WINDOWS_SEH
    if (settings.exceptionHandling)
        return check_wrapper_seh(*this, &CppCheckExecutor::check_internal, settings);
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    if (settings.exceptionHandling)
        register_signal_handler();
#endif
    return check_internal(settings);
}

bool CppCheckExecutor::reportSuppressions(const Settings &settings, const SuppressionList& suppressions, bool unusedFunctionCheckEnabled, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, ErrorLogger& errorLogger) {
    const auto& suppr = suppressions.getSuppressions();
    if (std::any_of(suppr.begin(), suppr.end(), [](const SuppressionList::Suppression& s) {
        return s.errorId == "unmatchedSuppression" && s.fileName.empty() && s.lineNumber == SuppressionList::Suppression::NO_LINE;
    }))
        return false;

    bool err = false;
    if (settings.useSingleJob()) {
        // the two inputs may only be used exclusively
        assert(!(!files.empty() && !fileSettings.empty()));

        for (std::list<FileWithDetails>::const_iterator i = files.cbegin(); i != files.cend(); ++i) {
            err |= SuppressionList::reportUnmatchedSuppressions(
                suppressions.getUnmatchedLocalSuppressions(*i, unusedFunctionCheckEnabled), errorLogger);
        }

        for (std::list<FileSettings>::const_iterator i = fileSettings.cbegin(); i != fileSettings.cend(); ++i) {
            err |= SuppressionList::reportUnmatchedSuppressions(
                suppressions.getUnmatchedLocalSuppressions(i->file, unusedFunctionCheckEnabled), errorLogger);
        }
    }
    err |= SuppressionList::reportUnmatchedSuppressions(suppressions.getUnmatchedGlobalSuppressions(unusedFunctionCheckEnabled), errorLogger);
    return err;
}

/*
 * That is a method which gets called from check_wrapper
 * */
int CppCheckExecutor::check_internal(const Settings& settings) const
{
    StdLogger stdLogger(settings);

    if (settings.reportProgress >= 0)
        stdLogger.resetLatestProgressOutputTime();

    if (settings.xml) {
        stdLogger.reportErr(ErrorMessage::getXMLHeader(settings.cppcheckCfgProductName));
    }

    if (!settings.buildDir.empty()) {
        std::list<std::string> fileNames;
        for (std::list<FileWithDetails>::const_iterator i = mFiles.cbegin(); i != mFiles.cend(); ++i)
            fileNames.emplace_back(i->path());
        AnalyzerInformation::writeFilesTxt(settings.buildDir, fileNames, settings.userDefines, mFileSettings);
    }

    if (!settings.checkersReportFilename.empty())
        std::remove(settings.checkersReportFilename.c_str());

    CppCheck cppcheck(stdLogger, true, executeCommand);
    cppcheck.settings() = settings; // this is a copy
    auto& suppressions = cppcheck.settings().supprs.nomsg;

    unsigned int returnValue = 0;
    if (settings.useSingleJob()) {
        // Single process
        SingleExecutor executor(cppcheck, mFiles, mFileSettings, settings, suppressions, stdLogger);
        returnValue = executor.check();
    } else {
#if defined(HAS_THREADING_MODEL_THREAD)
        if (settings.executor == Settings::ExecutorType::Thread) {
            ThreadExecutor executor(mFiles, mFileSettings, settings, suppressions, stdLogger, CppCheckExecutor::executeCommand);
            returnValue = executor.check();
        }
#endif
#if defined(HAS_THREADING_MODEL_FORK)
        if (settings.executor == Settings::ExecutorType::Process) {
            ProcessExecutor executor(mFiles, mFileSettings, settings, suppressions, stdLogger, CppCheckExecutor::executeCommand);
            returnValue = executor.check();
        }
#endif
    }

    returnValue |= cppcheck.analyseWholeProgram(settings.buildDir, mFiles, mFileSettings);

    if (settings.severity.isEnabled(Severity::information) || settings.checkConfiguration) {
        const bool err = reportSuppressions(settings, suppressions, settings.checks.isEnabled(Checks::unusedFunction), mFiles, mFileSettings, stdLogger);
        if (err && returnValue == 0)
            returnValue = settings.exitCode;
    }

    if (!settings.checkConfiguration) {
        cppcheck.tooManyConfigsError(emptyString,0U);
    }

    if (settings.safety || settings.severity.isEnabled(Severity::information) || !settings.checkersReportFilename.empty())
        stdLogger.writeCheckersReport();

    if (settings.xml) {
        stdLogger.reportErr(ErrorMessage::getXMLFooter());
    }

    if (settings.safety && (stdLogger.hasCriticalErrors() || returnValue != 0))
        return EXIT_FAILURE;

    if (returnValue)
        return settings.exitCode;
    return EXIT_SUCCESS;
}

void StdLogger::writeCheckersReport()
{
    CheckersReport checkersReport(mSettings, mActiveCheckers);

    bool suppressed = false;
    for (const SuppressionList::Suppression& s : mSettings.supprs.nomsg.getSuppressions()) {
        if (s.errorId == "checkersReport")
            suppressed = true;
    }

    if (!suppressed) {
        ErrorMessage msg;
        msg.severity = Severity::information;
        msg.id = "checkersReport";

        const int activeCheckers = checkersReport.getActiveCheckersCount();
        const int totalCheckers = checkersReport.getAllCheckersCount();

        std::string what;
        if (mCriticalErrors.empty())
            what = std::to_string(activeCheckers) + "/" + std::to_string(totalCheckers);
        else
            what = "There was critical errors";
        if (mSettings.checkersReportFilename.empty())
            what += " (use --checkers-report=<filename> to see details)";
        msg.setmsg("Active checkers: " + what);

        reportErr(msg);
    }

    if (!mSettings.checkersReportFilename.empty()) {
        std::ofstream fout(mSettings.checkersReportFilename);
        if (fout.is_open())
            fout << checkersReport.getReport(mCriticalErrors);
    }
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

void StdLogger::reportErr(const std::string &errmsg)
{
    if (mErrorOutput)
        *mErrorOutput << errmsg << std::endl;
    else {
        std::cerr << ansiToOEM(errmsg, !mSettings.xml) << std::endl;
    }
}

void StdLogger::reportOut(const std::string &outmsg, Color c)
{
    if (c == Color::Reset)
        std::cout << ansiToOEM(outmsg, true) << std::endl;
    else
        std::cout << c << ansiToOEM(outmsg, true) << Color::Reset << std::endl;
}

// TODO: remove filename parameter?
void StdLogger::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    (void)filename;

    if (!mLatestProgressOutputTime)
        return;

    // Report progress messages every x seconds
    const std::time_t currentTime = std::time(nullptr);
    if (currentTime >= (mLatestProgressOutputTime + mSettings.reportProgress))
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

void StdLogger::reportErr(const ErrorMessage &msg)
{
    if (msg.severity == Severity::internal && (msg.id == "logChecker" || endsWith(msg.id, "-logChecker"))) {
        const std::string& checker = msg.shortMessage();
        mActiveCheckers.emplace(checker);
        return;
    }

    if (ErrorLogger::isCriticalErrorId(msg.id) && mCriticalErrors.find(msg.id) == std::string::npos) {
        if (!mCriticalErrors.empty())
            mCriticalErrors += ", ";
        mCriticalErrors += msg.id;
        if (msg.severity == Severity::internal)
            mCriticalErrors += " (suppressed)";
    }

    if (msg.severity == Severity::internal)
        return;

    // TODO: we generate a different message here then we log below
    // TODO: there should be no need for verbose and default messages here
    // Alert only about unique errors
    if (!mShownErrors.insert(msg.toString(mSettings.verbose)).second)
        return;

    if (mSettings.xml)
        reportErr(msg.toXML());
    else
        reportErr(msg.toString(mSettings.verbose, mSettings.templateFormat, mSettings.templateLocation));
}

#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
void CppCheckExecutor::setExceptionOutput(FILE* exceptionOutput)
{
    mExceptionOutput = exceptionOutput;
#if defined(USE_UNIX_SIGNAL_HANDLING)
    set_signal_handler_output(mExceptionOutput);
#endif
}

// cppcheck-suppress unusedFunction - only used by USE_WINDOWS_SEH code
FILE* CppCheckExecutor::getExceptionOutput()
{
    return mExceptionOutput;
}
#endif

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


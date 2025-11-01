/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#if defined(__CYGWIN__)
#define _BSD_SOURCE // required to have popen() and pclose()
#endif

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
#include "json.h"
#include "path.h"
#include "sarifreport.h"
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
#include <cstdint>
#include <cstdio>
#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef USE_UNIX_SIGNAL_HANDLING
#include "signalhandler.h"
#endif

#ifdef USE_WINDOWS_SEH
#include "sehwrapper.h"
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
            std::cout << message << std::endl; // TODO: should not append newline
        }
    };

    class StdLogger : public ErrorLogger
    {
    public:
        explicit StdLogger(const Settings& settings)
            : mSettings(settings)
            , mGuidelineMapping(createGuidelineMapping(settings.reportType))
        {
            if (!mSettings.outputFile.empty()) {
                mErrorOutput = new std::ofstream(settings.outputFile);
            }
            if (!mSettings.buildDir.empty()) {
                mCheckersFile = Path::join(settings.buildDir, "checkers.txt");
            }
        }

        ~StdLogger() override {
            if (mSettings.outputFormat == Settings::OutputFormat::sarif) {
                reportErr(mSarifReport.serialize(mSettings.cppcheckCfgProductName));
            }
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

        void reportMetric(const std::string &metric) override
        {
            mFileMetrics.push_back(metric);
        }

        void reportMetrics()
        {
            if (!mFileMetrics.empty()) {
                auto &out = mErrorOutput ? *mErrorOutput : std::cerr;
                out << "    <metrics>" << std::endl;
                for (const auto &metric : mFileMetrics) {
                    out << "        " << metric << std::endl;
                }
                out << "    </metrics>" << std::endl;
            }
        }

        /**
         * @brief Write the checkers report
         */
        void writeCheckersReport(const Suppressions& supprs);

        bool hasCriticalErrors() const {
            return !mCriticalErrors.empty();
        }

        const std::string& getCtuInfo() const {
            return mCtuInfo;
        }

        void readActiveCheckers() {
            if (mCheckersFile.empty())
                return;

            std::ifstream fin(mCheckersFile);
            if (fin.is_open())
            {
                std::set<std::string> activeCheckers;
                std::string line;
                // cppcheck-suppress accessMoved - FP
                while (std::getline(fin, line))
                {
                    // cppcheck-suppress accessMoved - FP
                    activeCheckers.emplace(std::move(line));
                }
                mActiveCheckers = std::move(activeCheckers);
            }
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

        void reportProgress(const std::string &filename, const char stage[], std::size_t value) override;

        /**
         * Reference to current settings; set while check() is running for reportError().
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
         * List of critical errors
         */
        std::string mCriticalErrors;

        /**
         * CTU information
         */
        std::string mCtuInfo;

        /**
         * SARIF report generator
         */
        SarifReport mSarifReport;

        /**
         * Coding standard guideline mapping
         */
        std::map<std::string, std::string> mGuidelineMapping;

        /**
         * File metrics
         */
        std::vector<std::string> mFileMetrics;

        /**
         * The file the cached active checkers are stored in
         */
        std::string mCheckersFile;
    };
}

int CppCheckExecutor::check(int argc, const char* const argv[])
{
    Settings settings;
    CmdLineLoggerStd logger;
    Suppressions supprs;
    CmdLineParser parser(logger, settings, supprs);
    if (!parser.fillSettingsFromArgs(argc, argv)) {
        return EXIT_FAILURE;
    }
    if (Settings::terminated()) {
        return EXIT_SUCCESS;
    }

    settings.loadSummaries();

    mFiles = parser.getFiles();
    mFileSettings = parser.getFileSettings();

    const int ret = check_wrapper(settings, supprs);

    return ret;
}

int CppCheckExecutor::check_wrapper(const Settings& settings, Suppressions& supprs)
{
#ifdef USE_WINDOWS_SEH
    if (settings.exceptionHandling) {
        CALL_WITH_SEH_WRAPPER(check_internal(settings, supprs));
    }
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    if (settings.exceptionHandling)
        register_signal_handler(settings.exceptionOutput);
#endif
    return check_internal(settings, supprs);
}

/**
 * Report unmatched suppressions
 * @param unmatched list of unmatched suppressions (from Settings::Suppressions::getUnmatched(Local|Global)Suppressions)
 * @return true is returned if errors are reported
 */
static bool reportUnmatchedSuppressions(const std::list<SuppressionList::Suppression> &unmatched, ErrorLogger &errorLogger, const std::vector<std::string>& filters)
{
    bool err = false;
    // Report unmatched suppressions
    for (const SuppressionList::Suppression &s : unmatched) {
        // check if this unmatched suppression is suppressed
        bool suppressed = false;
        for (const SuppressionList::Suppression &s2 : unmatched) {
            if (s2.errorId == "unmatchedSuppression") {
                if ((s2.fileName.empty() || s2.fileName == "*" || s2.fileName == s.fileName) &&
                    (s2.lineNumber == SuppressionList::Suppression::NO_LINE || s2.lineNumber == s.lineNumber)) {
                    suppressed = true;
                    break;
                }
            }
        }

        if (suppressed)
            continue;

        const bool skip = std::any_of(filters.cbegin(), filters.cend(), [&s](const std::string& filter) {
            return matchglob(filter, s.errorId);
        });
        if (skip)
            continue;

        std::list<::ErrorMessage::FileLocation> callStack;
        if (!s.fileName.empty()) {
            callStack.emplace_back(s.fileName, s.lineNumber, 0);
        }
        errorLogger.reportErr(::ErrorMessage(std::move(callStack), "", Severity::information, "Unmatched suppression: " + s.errorId, "unmatchedSuppression", Certainty::normal));
        err = true;
    }
    return err;
}

bool CppCheckExecutor::reportUnmatchedSuppressions(const Settings &settings, const SuppressionList& suppressions, const std::list<FileWithDetails> &files, const std::list<FileSettings>& fileSettings, ErrorLogger& errorLogger) {
    // the two inputs may only be used exclusively
    assert(!(!files.empty() && !fileSettings.empty()));

    // bail out if there is a suppression of unmatchedSuppression which matches any file
    auto suppr = suppressions.getSuppressions();
    if (std::any_of(suppr.cbegin(), suppr.cend(), [](const SuppressionList::Suppression& s) {
        return s.errorId == "unmatchedSuppression" && (s.fileName.empty() || s.fileName == "*") && s.lineNumber == SuppressionList::Suppression::NO_LINE;
    }))
        return false;

    SuppressionList supprlist;

    const char* unusedFunctionOnly = std::getenv("UNUSEDFUNCTION_ONLY");
    const bool doUnusedFunctionOnly = unusedFunctionOnly && (std::strcmp(unusedFunctionOnly, "1") == 0);
    // ignore all other suppressions if we use the unusedFunction hack
    for (auto&& s : suppr)
    {
        // TODO: checkersReport should not be reported - see #13387
        if (doUnusedFunctionOnly && s.errorId != "unusedFunction" && s.errorId != "checkersReport")
            continue;
        supprlist.addSuppression(std::move(s));
    }

    bool err = false;

    for (auto i = files.cbegin(); i != files.cend(); ++i) {
        err |= ::reportUnmatchedSuppressions(supprlist.getUnmatchedLocalSuppressions(*i), errorLogger, settings.unmatchedSuppressionFilters);
    }

    for (auto i = fileSettings.cbegin(); i != fileSettings.cend(); ++i) {
        err |= ::reportUnmatchedSuppressions(supprlist.getUnmatchedLocalSuppressions(i->file), errorLogger, settings.unmatchedSuppressionFilters);
    }

    if (settings.inlineSuppressions) {
        err |= ::reportUnmatchedSuppressions(supprlist.getUnmatchedInlineSuppressions(), errorLogger, settings.unmatchedSuppressionFilters);
    }

    err |= ::reportUnmatchedSuppressions(supprlist.getUnmatchedGlobalSuppressions(), errorLogger, settings.unmatchedSuppressionFilters);
    return err;
}

/*
 * That is a method which gets called from check_wrapper
 * */
int CppCheckExecutor::check_internal(const Settings& settings, Suppressions& supprs) const
{
    StdLogger stdLogger(settings);

    if (settings.reportProgress >= 0)
        stdLogger.resetLatestProgressOutputTime();

    if (settings.outputFormat == Settings::OutputFormat::xml) {
        stdLogger.reportErr(ErrorMessage::getXMLHeader(settings.cppcheckCfgProductName, settings.xml_version));
    }

    if (!settings.buildDir.empty()) {
        std::list<std::string> fileNames;
        for (auto i = mFiles.cbegin(); i != mFiles.cend(); ++i)
            fileNames.emplace_back(i->path());
        AnalyzerInformation::writeFilesTxt(settings.buildDir, fileNames, settings.userDefines, mFileSettings);

        stdLogger.readActiveCheckers();
    }

    if (!settings.checkersReportFilename.empty())
        std::remove(settings.checkersReportFilename.c_str());

    CppCheck cppcheck(settings, supprs, stdLogger, true, executeCommand);

    unsigned int returnValue = 0;
    if (settings.useSingleJob()) {
        // Single process
        SingleExecutor executor(cppcheck, mFiles, mFileSettings, settings, supprs, stdLogger);
        returnValue = executor.check();
    } else {
#if defined(HAS_THREADING_MODEL_THREAD)
        if (settings.executor == Settings::ExecutorType::Thread) {
            ThreadExecutor executor(mFiles, mFileSettings, settings, supprs, stdLogger, CppCheckExecutor::executeCommand);
            returnValue = executor.check();
        }
#endif
#if defined(HAS_THREADING_MODEL_FORK)
        if (settings.executor == Settings::ExecutorType::Process) {
            ProcessExecutor executor(mFiles, mFileSettings, settings, supprs, stdLogger, CppCheckExecutor::executeCommand);
            returnValue = executor.check();
        }
#endif
    }

    returnValue |= cppcheck.analyseWholeProgram(settings.buildDir, mFiles, mFileSettings, stdLogger.getCtuInfo());

    if (settings.severity.isEnabled(Severity::information) || settings.checkConfiguration) {
        const bool err = reportUnmatchedSuppressions(settings, supprs.nomsg, mFiles, mFileSettings, stdLogger);
        if (err && returnValue == 0)
            returnValue = settings.exitCode;
    }

    if (!settings.checkConfiguration) {
        cppcheck.tooManyConfigsError("",0U);
    }

    stdLogger.writeCheckersReport(supprs);

    if (settings.outputFormat == Settings::OutputFormat::xml) {
        if (settings.xml_version == 3)
            stdLogger.reportMetrics();
        stdLogger.reportErr(ErrorMessage::getXMLFooter(settings.xml_version));
    }

    if (settings.safety && stdLogger.hasCriticalErrors())
        return EXIT_FAILURE;

    if (returnValue)
        return settings.exitCode;
    return EXIT_SUCCESS;
}

void StdLogger::writeCheckersReport(const Suppressions& supprs)
{
    if (!mCheckersFile.empty())
    {
        std::ofstream fout(mCheckersFile);
        for (const auto& c : mActiveCheckers)
        {
            fout << c << std::endl;
        }
    }

    const bool summary = mSettings.safety || mSettings.severity.isEnabled(Severity::information);
    const bool xmlReport = mSettings.outputFormat == Settings::OutputFormat::xml && mSettings.xml_version == 3;
    const bool textReport = !mSettings.checkersReportFilename.empty();

    if (!summary && !xmlReport && !textReport)
        return;

    CheckersReport checkersReport(mSettings, mActiveCheckers);

    const auto& suppressions = supprs.nomsg.getSuppressions();
    const bool summarySuppressed = std::any_of(suppressions.cbegin(), suppressions.cend(), [](const SuppressionList::Suppression& s) {
        return s.errorId == "checkersReport";
    });

    if (summary && !summarySuppressed) {
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
        if (!xmlReport && !textReport)
            what += " (use --checkers-report=<filename> to see details)";
        msg.setmsg("Active checkers: " + what);

        reportErr(msg);
    }

    if (textReport) {
        std::ofstream fout(mSettings.checkersReportFilename);
        if (fout.is_open())
            fout << checkersReport.getReport(mCriticalErrors);
    }

    if (xmlReport) {
        reportErr("    </errors>\n");
        if (mSettings.safety)
            reportErr("    <safety/>\n");
        if (mSettings.inlineSuppressions)
            reportErr("    <inline-suppr/>\n");
        if (!suppressions.empty()) {
            std::ostringstream suppressionsXml;
            supprs.nomsg.dump(suppressionsXml);
            reportErr(suppressionsXml.str());
        }
        reportErr(checkersReport.getXmlReport(mCriticalErrors));
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
        std::cerr << ansiToOEM(errmsg, mSettings.outputFormat != Settings::OutputFormat::xml) << std::endl;
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

    if (msg.severity == Severity::internal && msg.id == "ctuinfo") {
        mCtuInfo += msg.shortMessage() + "\n";
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

    ErrorMessage msgCopy = msg;
    msgCopy.guideline = getGuideline(msgCopy.id, mSettings.reportType,
                                     mGuidelineMapping, msgCopy.severity);
    msgCopy.classification = getClassification(msgCopy.guideline, mSettings.reportType);

    // TODO: there should be no need for verbose and default messages here
    const std::string msgStr =
        msgCopy.toString(mSettings.verbose, mSettings.templateFormat, mSettings.templateLocation);

    // Alert only about unique errors
    if (!mSettings.emitDuplicates && !mShownErrors.insert(msgStr).second)
        return;

    if (mSettings.outputFormat == Settings::OutputFormat::sarif) {
        mSarifReport.addFinding(std::move(msgCopy));
    } else if (mSettings.outputFormat == Settings::OutputFormat::xml) {
        reportErr(msgCopy.toXML());
    } else {
        reportErr(msgStr);
    }
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

    std::string cmd = exe + " " + joinedArgs + " " + redirect;

#ifdef _WIN32
    cmd = "\"" + cmd + "\"";
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
#elif defined(__APPLE__) && defined(__MACH__)
    // the W* macros cast to int* on macOS
    int res = pclose(p);
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


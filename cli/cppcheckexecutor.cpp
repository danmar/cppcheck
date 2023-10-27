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
    class SarifReport {
    public:
        void addFinding(ErrorMessage msg) {
            mFindings.push_back(std::move(msg));
        }

        picojson::array serializeRules() const {
            picojson::array ret;
            std::set<std::string> ruleIds;
            for (const auto& finding : mFindings) {
                // github only supports findings with locations
                if (finding.callStack.empty())
                    continue;
                if (ruleIds.insert(finding.id).second) {
                    picojson::object rule;
                    rule["id"] = picojson::value(finding.id);
                    // rule.shortDescription.text
                    picojson::object shortDescription;
                    shortDescription["text"] = picojson::value(finding.shortMessage());
                    rule["shortDescription"] = picojson::value(shortDescription);
                    // rule.fullDescription.text
                    picojson::object fullDescription;
                    fullDescription["text"] = picojson::value(finding.verboseMessage());
                    rule["fullDescription"] = picojson::value(fullDescription);
                    // rule.help.text
                    picojson::object help;
                    help["text"] = picojson::value(finding.verboseMessage()); // FIXME provide proper help text
                    rule["help"] = picojson::value(help);
                    // rule.properties.precision, rule.properties.problem.severity
                    picojson::object properties;
                    properties["precision"] = picojson::value(sarifPrecision(finding));
                    double securitySeverity = 0;
                    if (finding.severity == Severity::error && !ErrorLogger::isCriticalErrorId(finding.id))
                        securitySeverity = 9.9; // We see undefined behavior
                    //else if (finding.severity == Severity::warning)
                    //    securitySeverity = 5.1; // We see potential undefined behavior
                    if (securitySeverity > 0.5) {
                        properties["security-severity"] = picojson::value(securitySeverity);
                        const picojson::array tags{picojson::value("security")};
                        properties["tags"] = picojson::value(tags);
                    }
                    rule["properties"] = picojson::value(properties);

                    ret.emplace_back(rule);
                }
            }
            return ret;
        }

        static picojson::array serializeLocations(const ErrorMessage& finding) {
            picojson::array ret;
            for (const auto& location : finding.callStack) {
                picojson::object physicalLocation;
                picojson::object artifactLocation;
                artifactLocation["uri"] = picojson::value(location.getfile(false));
                physicalLocation["artifactLocation"] = picojson::value(artifactLocation);
                picojson::object region;
                region["startLine"] = picojson::value(static_cast<int64_t>(location.line));
                region["startColumn"] = picojson::value(static_cast<int64_t>(location.column));
                region["endLine"] = region["startLine"];
                region["endColumn"] = region["startColumn"];
                physicalLocation["region"] = picojson::value(region);
                picojson::object loc;
                loc["physicalLocation"] = picojson::value(physicalLocation);
                ret.emplace_back(loc);
            }
            return ret;
        }

        picojson::array serializeResults() const {
            picojson::array results;
            for (const auto& finding : mFindings) {
                // github only supports findings with locations
                if (finding.callStack.empty())
                    continue;
                picojson::object res;
                res["level"] = picojson::value(sarifSeverity(finding));
                res["locations"] = picojson::value(serializeLocations(finding));
                picojson::object message;
                message["text"] = picojson::value(finding.shortMessage());
                res["message"] = picojson::value(message);
                res["ruleId"] = picojson::value(finding.id);
                results.emplace_back(res);
            }
            return results;
        }

        picojson::value serializeRuns(const std::string& productName, const std::string& version) const {
            picojson::object driver;
            driver["name"] = picojson::value(productName);
            driver["semanticVersion"] = picojson::value(version);
            driver["informationUri"] = picojson::value("https://cppcheck.sourceforge.io");
            driver["rules"] = picojson::value(serializeRules());
            picojson::object tool;
            tool["driver"] = picojson::value(driver);
            picojson::object run;
            run["tool"] = picojson::value(tool);
            run["results"] = picojson::value(serializeResults());
            picojson::array runs{picojson::value(run)};
            return picojson::value(runs);
        }

        std::string serialize(std::string productName) const {
            const auto nameAndVersion = Settings::getNameAndVersion(productName);
            productName = nameAndVersion.first.empty() ? "Cppcheck" : nameAndVersion.first;
            std::string version = nameAndVersion.first.empty() ? CppCheck::version() : nameAndVersion.second;
            if (version.find(' ') != std::string::npos)
                version.erase(version.find(' '), std::string::npos);

            picojson::object doc;
            doc["version"] = picojson::value("2.1.0");
            doc["$schema"] = picojson::value("https://docs.oasis-open.org/sarif/sarif/v2.1.0/errata01/os/schemas/sarif-schema-2.1.0.json");
            doc["runs"] = serializeRuns(productName, version);

            return picojson::value(doc).serialize(true);
        }
    private:

        static std::string sarifSeverity(const ErrorMessage& errmsg) {
            if (ErrorLogger::isCriticalErrorId(errmsg.id))
                return "error";
            switch (errmsg.severity) {
            case Severity::error:
            case Severity::warning:
            case Severity::style:
            case Severity::portability:
            case Severity::performance:
                return "warning";
            case Severity::information:
            case Severity::internal:
            case Severity::debug:
            case Severity::none:
                return "note";
            }
            return "note";
        }

        static std::string sarifPrecision(const ErrorMessage& errmsg) {
            if (errmsg.certainty == Certainty::inconclusive)
                return "medium";
            return "high";
        }

        std::vector<ErrorMessage> mFindings;
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

        /**
         * @brief Write the checkers report
         */
        void writeCheckersReport();

        bool hasCriticalErrors() const {
            return !mCriticalErrors.empty();
        }

        const std::string& getCtuInfo() const {
            return mCtuInfo;
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
    };
}

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
    if (settings.exceptionHandling) {
        CALL_WITH_SEH_WRAPPER(check_internal(settings));
    }
#elif defined(USE_UNIX_SIGNAL_HANDLING)
    if (settings.exceptionHandling)
        register_signal_handler(settings.exceptionOutput);
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

        for (auto i = files.cbegin(); i != files.cend(); ++i) {
            err |= SuppressionList::reportUnmatchedSuppressions(
                suppressions.getUnmatchedLocalSuppressions(*i, unusedFunctionCheckEnabled), errorLogger);
        }

        for (auto i = fileSettings.cbegin(); i != fileSettings.cend(); ++i) {
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
        stdLogger.reportErr(ErrorMessage::getXMLHeader(settings.cppcheckCfgProductName, settings.xml_version));
    }

    if (!settings.buildDir.empty()) {
        std::list<std::string> fileNames;
        for (auto i = mFiles.cbegin(); i != mFiles.cend(); ++i)
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

    returnValue |= cppcheck.analyseWholeProgram(settings.buildDir, mFiles, mFileSettings, stdLogger.getCtuInfo());

    if (settings.severity.isEnabled(Severity::information) || settings.checkConfiguration) {
        const bool err = reportSuppressions(settings, suppressions, settings.checks.isEnabled(Checks::unusedFunction), mFiles, mFileSettings, stdLogger);
        if (err && returnValue == 0)
            returnValue = settings.exitCode;
    }

    if (!settings.checkConfiguration) {
        cppcheck.tooManyConfigsError(emptyString,0U);
    }

    stdLogger.writeCheckersReport();

    if (settings.xml) {
        stdLogger.reportErr(ErrorMessage::getXMLFooter(settings.xml_version));
    }

    if (settings.safety && stdLogger.hasCriticalErrors())
        return EXIT_FAILURE;

    if (returnValue)
        return settings.exitCode;
    return EXIT_SUCCESS;
}

void StdLogger::writeCheckersReport()
{
    const bool summary = mSettings.safety || mSettings.severity.isEnabled(Severity::information);
    const bool xmlReport = mSettings.xml && mSettings.xml_version == 3;
    const bool textReport = !mSettings.checkersReportFilename.empty();

    if (!summary && !xmlReport && !textReport)
        return;

    CheckersReport checkersReport(mSettings, mActiveCheckers);

    const auto& suppressions = mSettings.supprs.nomsg.getSuppressions();
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

    // TODO: we generate a different message here then we log below
    // TODO: there should be no need for verbose and default messages here
    // Alert only about unique errors
    if (!mShownErrors.insert(msg.toString(mSettings.verbose)).second)
        return;

    if (mSettings.outputFormat == Settings::OutputFormat::sarif)
        mSarifReport.addFinding(msg);
    else if (mSettings.xml)
        reportErr(msg.toXML());
    else
        reportErr(msg.toString(mSettings.verbose, mSettings.templateFormat, mSettings.templateLocation));
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


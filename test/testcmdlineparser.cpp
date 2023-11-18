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

#include "cmdlinelogger.h"
#include "cmdlineparser.h"
#include "config.h"
#include "cppcheckexecutor.h"
#include "errortypes.h"
#include "helpers.h"
#include "platform.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "fixture.h"
#include "timer.h"
#include "utils.h"

#include <cstdint>
#include <cstdio>
#include <list>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

class TestCmdlineParser : public TestFixture {
public:
    TestCmdlineParser() : TestFixture("TestCmdlineParser")
    {}

private:
    class CmdLineLoggerTest : public CmdLineLogger
    {
    public:
        CmdLineLoggerTest() = default;

        void printMessage(const std::string &message) override
        {
            printInternal("cppcheck: " + message + '\n');
        }

        void printError(const std::string &message) override
        {
            printMessage("error: " + message);
        }

        void printRaw(const std::string &message) override
        {
            printInternal(message + '\n');
        }

        std::string str()
        {
            std::string s;
            std::swap(buf, s);
            return s;
        }

        void destroy()
        {
            if (!buf.empty())
                throw std::runtime_error("unconsumed messages");
        }

    private:
        void printInternal(const std::string &message)
        {
            buf += message;
        }

        std::string buf;
    };

    std::unique_ptr<CmdLineLoggerTest> logger;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<CmdLineParser> parser;

    void prepareTestInternal() override {
        logger.reset(new CmdLineLoggerTest());
        settings.reset(new Settings());
        parser.reset(new CmdLineParser(*logger, *settings, settings->nomsg, settings->nofail));
    }

    void teardownTestInternal() override {
        logger->destroy();
        // TODO: verify that the redirect output is empty
    }

    void run() override {
        TEST_CASE(nooptions);
        TEST_CASE(helpshort);
        TEST_CASE(helplong);
        TEST_CASE(showversion);
        TEST_CASE(onefile);
        TEST_CASE(onepath);
        TEST_CASE(optionwithoutfile);
        TEST_CASE(verboseshort);
        TEST_CASE(verboselong);
        TEST_CASE(debugSimplified);
        TEST_CASE(debugwarnings);
        TEST_CASE(forceshort);
        TEST_CASE(forcelong);
        TEST_CASE(relativePaths1);
        TEST_CASE(relativePaths2);
        TEST_CASE(relativePaths3);
        TEST_CASE(relativePaths4);
        TEST_CASE(quietshort);
        TEST_CASE(quietlong);
        TEST_CASE(defines_noarg);
        TEST_CASE(defines_noarg2);
        TEST_CASE(defines_noarg3);
        TEST_CASE(defines);
        TEST_CASE(defines2);
        TEST_CASE(defines3);
        TEST_CASE(defines4);
        TEST_CASE(enforceLanguage1);
        TEST_CASE(enforceLanguage2);
        TEST_CASE(enforceLanguage3);
        TEST_CASE(enforceLanguage4);
        TEST_CASE(enforceLanguage5);
        TEST_CASE(enforceLanguage6);
        TEST_CASE(enforceLanguage7);
        TEST_CASE(includesnopath);
        TEST_CASE(includes);
        TEST_CASE(includesslash);
        TEST_CASE(includesbackslash);
        TEST_CASE(includesnospace);
        TEST_CASE(includes2);
        TEST_CASE(includesFile);
        TEST_CASE(includesFileNoFile);
        TEST_CASE(configExcludesFile);
        TEST_CASE(configExcludesFileNoFile);
        TEST_CASE(enabledAll);
        TEST_CASE(enabledStyle);
        TEST_CASE(enabledPerformance);
        TEST_CASE(enabledPortability);
        TEST_CASE(enabledInformation);
        TEST_CASE(enabledUnusedFunction);
        TEST_CASE(enabledMissingInclude);
#ifdef CHECK_INTERNAL
        TEST_CASE(enabledInternal);
#endif
        TEST_CASE(enabledMultiple);
        TEST_CASE(enabledInvalid);
        TEST_CASE(enabledError);
        TEST_CASE(enabledEmpty);
        TEST_CASE(disableAll);
        TEST_CASE(disableMultiple);
        TEST_CASE(disableStylePartial);
        TEST_CASE(disableInformationPartial);
        TEST_CASE(disableInformationPartial2);
        TEST_CASE(disableInvalid);
        TEST_CASE(disableError);
        TEST_CASE(disableEmpty);
        TEST_CASE(inconclusive);
        TEST_CASE(errorExitcode);
        TEST_CASE(errorExitcodeMissing);
        TEST_CASE(errorExitcodeStr);
        TEST_CASE(exitcodeSuppressionsOld);
        TEST_CASE(exitcodeSuppressions);
        TEST_CASE(exitcodeSuppressionsNoFile);
        TEST_CASE(fileList);
        TEST_CASE(fileListNoFile);
        // TEST_CASE(fileListStdin);  // Disabled since hangs the test run
        TEST_CASE(fileListInvalid);
        TEST_CASE(inlineSuppr);
        TEST_CASE(jobs);
        TEST_CASE(jobs2);
        TEST_CASE(jobsMissingCount);
        TEST_CASE(jobsInvalid);
        TEST_CASE(jobsNoJobs);
        TEST_CASE(jobsTooBig);
        TEST_CASE(maxConfigs);
        TEST_CASE(maxConfigsMissingCount);
        TEST_CASE(maxConfigsInvalid);
        TEST_CASE(maxConfigsTooSmall);
        TEST_CASE(reportProgress1);
        TEST_CASE(reportProgress2);
        TEST_CASE(reportProgress3);
        TEST_CASE(reportProgress4);
        TEST_CASE(reportProgress5);
        TEST_CASE(stdc99);
        TEST_CASE(stdcpp11);
        TEST_CASE(stdunknown1);
        TEST_CASE(stdunknown2);
        TEST_CASE(platformWin64);
        TEST_CASE(platformWin32A);
        TEST_CASE(platformWin32W);
        TEST_CASE(platformUnix32);
        TEST_CASE(platformUnix32Unsigned);
        TEST_CASE(platformUnix64);
        TEST_CASE(platformUnix64Unsigned);
        TEST_CASE(platformNative);
        TEST_CASE(platformUnspecified);
        TEST_CASE(platformPlatformFile);
        TEST_CASE(platformUnknown);
        TEST_CASE(plistEmpty);
        TEST_CASE(plistDoesNotExist);
        TEST_CASE(suppressionsOld);
        TEST_CASE(suppressions);
        TEST_CASE(suppressionsNoFile1);
        TEST_CASE(suppressionsNoFile2);
        TEST_CASE(suppressionsNoFile3);
        TEST_CASE(suppressionSingle);
        TEST_CASE(suppressionSingleFile);
        TEST_CASE(suppressionTwo);
        TEST_CASE(suppressionTwoSeparate);
        TEST_CASE(templates);
        TEST_CASE(templatesGcc);
        TEST_CASE(templatesVs);
        TEST_CASE(templatesEdit);
        TEST_CASE(templatesCppcheck1);
        TEST_CASE(templatesDaca2);
        TEST_CASE(templatesSelfcheck);
        TEST_CASE(templatesNoPlaceholder);
        TEST_CASE(templateFormatInvalid);
        TEST_CASE(templateFormatEmpty);
        TEST_CASE(templateLocationInvalid);
        TEST_CASE(templateLocationEmpty);
        TEST_CASE(xml);
        TEST_CASE(xmlver2);
        TEST_CASE(xmlver2both);
        TEST_CASE(xmlver2both2);
        TEST_CASE(xmlverunknown);
        TEST_CASE(xmlverinvalid);
        TEST_CASE(doc);
        TEST_CASE(showtimeFile);
        TEST_CASE(showtimeFileTotal);
        TEST_CASE(showtimeTop5);
        TEST_CASE(showtimeTop5File);
        TEST_CASE(showtimeTop5Summary);
        TEST_CASE(showtimeNone);
        TEST_CASE(showtimeEmpty);
        TEST_CASE(showtimeInvalid);
        TEST_CASE(errorlist1);
        TEST_CASE(errorlistverbose1);
        TEST_CASE(errorlistverbose2);
        TEST_CASE(ignorepathsnopath);
#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
        TEST_CASE(exceptionhandling);
        TEST_CASE(exceptionhandling2);
        TEST_CASE(exceptionhandling3);
        TEST_CASE(exceptionhandlingInvalid);
        TEST_CASE(exceptionhandlingInvalid2);
#else
        TEST_CASE(exceptionhandlingNotSupported);
        TEST_CASE(exceptionhandlingNotSupported2);
#endif
        TEST_CASE(clang);
        TEST_CASE(clang2);
        TEST_CASE(clangInvalid);
        TEST_CASE(valueFlowMaxIterations);
        TEST_CASE(valueFlowMaxIterations2);
        TEST_CASE(valueFlowMaxIterationsInvalid);
        TEST_CASE(valueFlowMaxIterationsInvalid2);
        TEST_CASE(valueFlowMaxIterationsInvalid3);
        TEST_CASE(checksMaxTime);
        TEST_CASE(checksMaxTime2);
        TEST_CASE(checksMaxTimeInvalid);
#ifdef THREADING_MODEL_FORK
        TEST_CASE(loadAverage);
        TEST_CASE(loadAverage2);
        TEST_CASE(loadAverageInvalid);
#else
        TEST_CASE(loadAverageNotSupported);
#endif
        TEST_CASE(maxCtuDepth);
        TEST_CASE(maxCtuDepthInvalid);
        TEST_CASE(performanceValueflowMaxTime);
        TEST_CASE(performanceValueflowMaxTimeInvalid);
        TEST_CASE(performanceValueFlowMaxIfCount);
        TEST_CASE(performanceValueFlowMaxIfCountInvalid);
        TEST_CASE(templateMaxTime);
        TEST_CASE(templateMaxTimeInvalid);
        TEST_CASE(templateMaxTimeInvalid2);
        TEST_CASE(typedefMaxTime);
        TEST_CASE(typedefMaxTimeInvalid);
        TEST_CASE(typedefMaxTimeInvalid2);
        TEST_CASE(templateMaxTime);
        TEST_CASE(templateMaxTime);
        TEST_CASE(project);
        TEST_CASE(projectMultiple);
        TEST_CASE(projectAndSource);
        TEST_CASE(projectEmpty);
        TEST_CASE(projectMissing);
        TEST_CASE(projectNoPaths);
        TEST_CASE(addon);
#ifdef HAVE_RULES
        TEST_CASE(rule);
#else
        TEST_CASE(ruleNotSupported);
#endif
#ifdef HAVE_RULES
        TEST_CASE(ruleFile);
        TEST_CASE(ruleFileEmpty);
        TEST_CASE(ruleFileMissing);
        TEST_CASE(ruleFileInvalid);
#else
        TEST_CASE(ruleFileNotSupported);
#endif
        TEST_CASE(signedChar);
        TEST_CASE(signedChar2);
        TEST_CASE(unsignedChar);
        TEST_CASE(unsignedChar2);
        TEST_CASE(signedCharUnsignedChar);

        TEST_CASE(ignorepaths1);
        TEST_CASE(ignorepaths2);
        TEST_CASE(ignorepaths3);
        TEST_CASE(ignorepaths4);
        TEST_CASE(ignorefilepaths1);
        TEST_CASE(ignorefilepaths2);

        TEST_CASE(checkconfig);
        TEST_CASE(unknownParam);

        TEST_CASE(undefs_noarg);
        TEST_CASE(undefs_noarg2);
        TEST_CASE(undefs_noarg3);
        TEST_CASE(undefs);
        TEST_CASE(undefs2);

        TEST_CASE(cppcheckBuildDirExistent);
        TEST_CASE(cppcheckBuildDirNonExistent);
        TEST_CASE(cppcheckBuildDirEmpty);
    }


    void nooptions() {
        REDIRECT;
        const char * const argv[] = {"cppcheck"};
        ASSERT(parser->parseFromArgs(1, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void helpshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-h"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void helplong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--help"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void showversion() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--version"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser->getShowVersion());
        ASSERT_EQUALS("", logger->str()); // version is not actually shown
    }

    void onefile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser->getPathNames().size());
        ASSERT_EQUALS("file.cpp", parser->getPathNames().at(0));
        ASSERT_EQUALS("", logger->str());
    }

    void onepath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "src"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser->getPathNames().size());
        ASSERT_EQUALS("src", parser->getPathNames().at(0));
        ASSERT_EQUALS("", logger->str());
    }

    void optionwithoutfile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v"};
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(0, (int)parser->getPathNames().size());
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", logger->str());
    }

    void verboseshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v", "file.cpp"};
        settings->verbose = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->verbose);
        ASSERT_EQUALS("", logger->str());
    }

    void verboselong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "file.cpp"};
        settings->verbose = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->verbose);
        ASSERT_EQUALS("", logger->str());
    }

    void debugSimplified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-simplified", "file.cpp"};
        settings->debugSimplified = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->debugSimplified);
        ASSERT_EQUALS("", logger->str());
    }

    void debugwarnings() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-warnings", "file.cpp"};
        settings->debugwarnings = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->debugwarnings);
        ASSERT_EQUALS("", logger->str());
    }

    void forceshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "file.cpp"};
        settings->force = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->force);
        ASSERT_EQUALS("", logger->str());
    }

    void forcelong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--force", "file.cpp"};
        settings->force = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->force);
        ASSERT_EQUALS("", logger->str());
    }

    void relativePaths1() {
        REDIRECT;
        settings->relativePaths = false;
        const char * const argv[] = {"cppcheck", "-rp", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS("", logger->str());
    }

    void relativePaths2() {
        REDIRECT;
        settings->relativePaths = false;
        const char * const argv[] = {"cppcheck", "--relative-paths", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS("", logger->str());
    }

    void relativePaths3() {
        REDIRECT;
        settings->relativePaths = false;
        settings->basePaths.clear();
        const char * const argv[] = {"cppcheck", "-rp=C:/foo;C:\\bar", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS(2, settings->basePaths.size());
        ASSERT_EQUALS("C:/foo", settings->basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings->basePaths[1]);
        ASSERT_EQUALS("", logger->str());
    }

    void relativePaths4() {
        REDIRECT;
        settings->relativePaths = false;
        settings->basePaths.clear();

        const char * const argv[] = {"cppcheck", "--relative-paths=C:/foo;C:\\bar", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS(2, settings->basePaths.size());
        ASSERT_EQUALS("C:/foo", settings->basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings->basePaths[1]);
        ASSERT_EQUALS("", logger->str());
    }

    void quietshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-q", "file.cpp"};
        settings->quiet = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->quiet);
        ASSERT_EQUALS("", logger->str());
    }

    void quietlong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--quiet", "file.cpp"};
        settings->quiet = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->quiet);
        ASSERT_EQUALS("", logger->str());
    }

    void defines_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "-v", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "--quiet", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "file.cpp"};
        settings->userDefines.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("_WIN32=1", settings->userDefines);
        ASSERT_EQUALS("", logger->str());
    }

    void defines2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "-DNODEBUG", "file.cpp"};
        settings->userDefines.clear();
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("_WIN32=1;NODEBUG=1", settings->userDefines);
        ASSERT_EQUALS("", logger->str());
    }

    void defines3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "DEBUG", "file.cpp"};
        settings->userDefines.clear();
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("DEBUG=1", settings->userDefines);
        ASSERT_EQUALS("", logger->str());
    }

    void defines4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-DDEBUG=", "file.cpp"}; // #5137 - defining empty macro
        settings->userDefines.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("DEBUG=", settings->userDefines);
        ASSERT_EQUALS("", logger->str());
    }

    void enforceLanguage1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(Settings::Language::None, settings->enforcedLang);
        ASSERT_EQUALS("", logger->str());
    }

    void enforceLanguage2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x", "c++", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(Settings::Language::CPP, settings->enforcedLang);
        ASSERT_EQUALS("", logger->str());
    }

    void enforceLanguage3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x"};
        ASSERT(!parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", logger->str());
    }

    void enforceLanguage4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x", "--inconclusive", "file.cpp"};
        ASSERT(!parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", logger->str());
    }

    void enforceLanguage5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=c++", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Language::CPP, settings->enforcedLang);
        ASSERT_EQUALS("", logger->str());
    }

    void enforceLanguage6() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=c", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Language::C, settings->enforcedLang);
        ASSERT_EQUALS("", logger->str());
    }

    void enforceLanguage7() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=unknownLanguage", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unknown language 'unknownLanguage' enforced.\n", logger->str());
    }

    void includesnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I"};
        // Fails since -I has no param
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-I' is missing.\n", logger->str());
    }

    void includes() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include", "file.cpp"};
        settings->includePaths.clear();
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
        ASSERT_EQUALS("", logger->str());
    }

    void includesslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "file.cpp"};
        settings->includePaths.clear();
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
        ASSERT_EQUALS("", logger->str());
    }

    void includesbackslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include\\", "file.cpp"};
        settings->includePaths.clear();
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
        ASSERT_EQUALS("", logger->str());
    }

    void includesnospace() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-Iinclude", "file.cpp"};
        settings->includePaths.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
        ASSERT_EQUALS("", logger->str());
    }

    void includes2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "-I", "framework/", "file.cpp"};
        settings->includePaths.clear();
        ASSERT(parser->parseFromArgs(6, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
        settings->includePaths.pop_front();
        ASSERT_EQUALS("framework/", settings->includePaths.front());
        ASSERT_EQUALS("", logger->str());
    }

    void includesFile() {
        REDIRECT;
        ScopedFile file("includes.txt",
                        "path/sub\n"
                        "path2/sub1\n");
        const char * const argv[] = {"cppcheck", "--includes-file=includes.txt", "file.cpp"};
        ASSERT_EQUALS(true, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->includePaths.size());
        auto it = settings->includePaths.cbegin();
        ASSERT_EQUALS("path/sub/", *it++);
        ASSERT_EQUALS("path2/sub1/", *it);
        ASSERT_EQUALS("", logger->str());
    }

    void includesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--includes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to open includes file at 'fileThatDoesNotExist.txt'\n", logger->str());
    }

    void configExcludesFile() {
        REDIRECT;
        ScopedFile file("excludes.txt",
                        "path/sub\n"
                        "path2/sub1\n");
        const char * const argv[] = {"cppcheck", "--config-excludes-file=excludes.txt", "file.cpp"};
        settings->configExcludePaths.clear();
        ASSERT_EQUALS(true, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->configExcludePaths.size());
        auto it = settings->configExcludePaths.cbegin();
        ASSERT_EQUALS("path/sub/", *it++);
        ASSERT_EQUALS("path2/sub1/", *it);
        ASSERT_EQUALS("", logger->str());
    }

    void configExcludesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--config-excludes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS( false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to open config excludes file at 'fileThatDoesNotExist.txt'\n", logger->str());
    }

    void enabledAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT(!settings->checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledStyle() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledPerformance() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=performance", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(!settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->severity.isEnabled(Severity::performance));
        ASSERT(!settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledPortability() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=portability", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(!settings->severity.isEnabled(Severity::warning));
        ASSERT(!settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledInformation() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("cppcheck: '--enable=information' will no longer implicitly enable 'missingInclude' starting with 2.16. Please enable it explicitly if you require it.\n", logger->str());
    }

    void enabledUnusedFunction() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=unusedFunction", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledMissingInclude() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

#ifdef CHECK_INTERNAL
    void enabledInternal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=internal", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", logger->str());
    }
#endif

    void enabledMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude,portability,warning", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(!settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=warning,missingIncludeSystem,style", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'missingIncludeSystem'\n", logger->str());
    }

    void enabledError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=error", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'error'\n", logger->str());
    }

    void enabledEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter is empty\n", logger->str());
    }


    void disableAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=all", "file.cpp"};
        settings->severity.clear();
        settings->checks.clear();
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", logger->str());
    }

    void disableMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=style", "--disable=unusedFunction", "file.cpp"};
        settings->severity.clear();
        settings->checks.clear();
        ASSERT(parser->parseFromArgs(5, argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", logger->str());
    }

    // make sure the implied "style" checks are not added when "--enable=style" is specified
    void disableStylePartial() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "--disable=performance", "--enable=unusedFunction", "file.cpp"};
        settings->severity.clear();
        settings->checks.clear();
        ASSERT(parser->parseFromArgs(5, argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(true, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", logger->str());
    }

    void disableInformationPartial() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "--disable=missingInclude", "file.cpp"};
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("cppcheck: '--enable=information' will no longer implicitly enable 'missingInclude' starting with 2.16. Please enable it explicitly if you require it.\n", logger->str());
    }

    void disableInformationPartial2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "--disable=information", "file.cpp"};
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT(!settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void disableInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=leaks", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'leaks'\n", logger->str());
    }

    void disableError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=error", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'error'\n", logger->str());
    }

    void disableEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter is empty\n", logger->str());
    }

    void inconclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inconclusive", "file.cpp"};
        settings->certainty.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->certainty.isEnabled(Certainty::inconclusive));
        ASSERT_EQUALS("", logger->str());
    }

    void errorExitcode() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=5", "file.cpp"};
        settings->exitCode = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(5, settings->exitCode);
        ASSERT_EQUALS("", logger->str());
    }

    void errorExitcodeMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=", "file.cpp"};
        // Fails since exit code not given
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--error-exitcode=' is not valid - not an integer.\n", logger->str());
    }

    void errorExitcodeStr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=foo", "file.cpp"};
        // Fails since invalid exit code
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--error-exitcode=' is not valid - not an integer.\n", logger->str());
    }

    void exitcodeSuppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "suppr.txt", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", logger->str());
    }

    void exitcodeSuppressions() {
        REDIRECT;
        ScopedFile file("suppr.txt",
                        "uninitvar\n"
                        "unusedFunction\n");
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions=suppr.txt", "file.cpp"};
        ASSERT_EQUALS(true, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->nofail.getSuppressions().size());
        auto it = settings->nofail.getSuppressions().cbegin();
        ASSERT_EQUALS("uninitvar", (it++)->errorId);
        ASSERT_EQUALS("unusedFunction", it->errorId);
        ASSERT_EQUALS("", logger->str());
    }

    void exitcodeSuppressionsNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", logger->str());
    }

    void fileList() {
        REDIRECT;
        ScopedFile file("files.txt",
                        "file1.c\n"
                        "file2.cpp\n");
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        ASSERT_EQUALS(true, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(3, parser->getPathNames().size());
        auto it = parser->getPathNames().cbegin();
        ASSERT_EQUALS("file1.c", *it++);
        ASSERT_EQUALS("file2.cpp", *it++);
        ASSERT_EQUALS("file.cpp", *it);
        ASSERT_EQUALS("", logger->str());
    }

    void fileListNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: couldn't open the file: \"files.txt\".\n", logger->str());
    }

    /*    void fileListStdin() {
            // TODO: Give it some stdin to read from, fails because the list of
            // files in stdin (_pathnames) is empty
            REDIRECT;
            const char * const argv[] = {"cppcheck", "--file-list=-", "file.cpp"};
            TODO_ASSERT_EQUALS(true, false, parser->parseFromArgs(3, argv));
            TODO_ASSERT_EQUALS("", "", logger->str());
        } */

    void fileListInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list", "files.txt", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--file-list\".\n", logger->str());
    }

    void inlineSuppr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inline-suppr", "file.cpp"};
        settings->inlineSuppressions = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->inlineSuppressions);
        ASSERT_EQUALS("", logger->str());
    }

    void jobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "3", "file.cpp"};
        settings->jobs = 0;
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(3, settings->jobs);
        ASSERT_EQUALS("", logger->str());
    }

    void jobs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j3", "file.cpp"};
        settings->jobs = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(3, settings->jobs);
        ASSERT_EQUALS("", logger->str());
    }

    void jobsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "file.cpp"};
        // Fails since -j is missing thread count
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not valid - not an integer.\n", logger->str());
    }

    void jobsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "e", "file.cpp"};
        // Fails since invalid count given for -j
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not valid - not an integer.\n", logger->str());
    }

    void jobsNoJobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j0", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument for '-j' must be greater than 0.\n", logger->str());
    }

    void jobsTooBig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j1025", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument for '-j' is allowed to be 1024 at max.\n", logger->str());
    }

    void maxConfigs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "--max-configs=12", "file.cpp"};
        settings->force = false;
        settings->maxConfigs = 0;
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(12, settings->maxConfigs);
        ASSERT_EQUALS(false, settings->force);
        ASSERT_EQUALS("", logger->str());
    }

    void maxConfigsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=", "file.cpp"};
        // Fails since --max-configs= is missing limit
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not valid - not an integer.\n", logger->str());
    }

    void maxConfigsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=e", "file.cpp"};
        // Fails since invalid count given for --max-configs=
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not valid - not an integer.\n", logger->str());
    }

    void maxConfigsTooSmall() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=0", "file.cpp"};
        // Fails since limit must be greater than 0
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' must be greater than 0.\n", logger->str());
    }

    void reportProgress1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(10, settings->reportProgress);
        ASSERT_EQUALS("", logger->str());
    }

    void reportProgress2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--report-progress=' is not valid - not an integer.\n", logger->str());
    }

    void reportProgress3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=-1", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--report-progress=' needs to be a positive integer.\n", logger->str());
    }

    void reportProgress4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=0", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(0, settings->reportProgress);
        ASSERT_EQUALS("", logger->str());
    }

    void reportProgress5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=1", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->reportProgress);
        ASSERT_EQUALS("", logger->str());
    }

    void stdc99() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c99", "file.cpp"};
        settings->standards.c = Standards::C89;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->standards.c == Standards::C99);
        ASSERT_EQUALS("", logger->str());
    }

    void stdcpp11() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c++11", "file.cpp"};
        settings->standards.cpp = Standards::CPP03;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->standards.cpp == Standards::CPP11);
        ASSERT_EQUALS("", logger->str());
    }

    void stdunknown1() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=d++11", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unknown --std value 'd++11'\n", logger->str());
    }

    void stdunknown2() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=cplusplus11", "file.cpp"};
        TODO_ASSERT(!parser->parseFromArgs(3, argv));
        TODO_ASSERT_EQUALS("cppcheck: error: unknown --std value 'cplusplus11'\n", "", logger->str());
    }

    void platformWin64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win64", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Win64, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformWin32A() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32A", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Win32A, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformWin32W() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32W", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Win32W, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformUnix32() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix32, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformUnix32Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32-unsigned", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix32, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformUnix64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix64, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformUnix64Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64-unsigned", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix64, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformNative() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=native", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Native, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformUnspecified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unspecified", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Native));
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unspecified, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformPlatformFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=avr8", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(true, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::File, settings->platform.type);
        ASSERT_EQUALS("", logger->str());
    }

    void platformUnknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win128", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized platform: 'win128'.\n", logger->str());
    }

    void plistEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=", "file.cpp"};
        settings->plistOutput = "";
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->plistOutput == "./");
        ASSERT_EQUALS("", logger->str());
    }

    void plistDoesNotExist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=./cppcheck_reports", "file.cpp"};
        // Fails since folder pointed by --plist-output= does not exist
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        // TODO: output contains non-native separator
        //ASSERT_EQUALS("cppcheck: error: plist folder does not exist: \"cppcheck_reports/\".\n", logger->str());
    }

    void suppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions", "suppr.txt", "file.cpp"};
        ASSERT(!parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--suppressions\".\n", logger->str());
    }

    void suppressions() {
        REDIRECT;
        ScopedFile file("suppr.txt",
                        "uninitvar\n"
                        "unusedFunction\n");
        const char * const argv[] = {"cppcheck", "--suppressions-list=suppr.txt", "file.cpp"};
        ASSERT_EQUALS(true, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->nomsg.getSuppressions().size());
        auto it = settings->nomsg.getSuppressions().cbegin();
        ASSERT_EQUALS("uninitvar", (it++)->errorId);
        ASSERT_EQUALS("unusedFunction", it->errorId);
        ASSERT_EQUALS("", logger->str());
    }

    void suppressionsNoFile1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(false, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    void suppressionsNoFile2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr,b.suppr", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    void suppressionsNoFile3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr b.suppr", "file.cpp"};
        ASSERT_EQUALS(false, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    static Suppressions::ErrorMessage errorMessage(const std::string &errorId, const std::string &fileName, int lineNumber) {
        Suppressions::ErrorMessage e;
        e.errorId = errorId;
        e.setFileName(fileName);
        e.lineNumber = lineNumber;
        return e;
    }

    void suppressionSingle() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1)));
        ASSERT_EQUALS("", logger->str());
    }

    void suppressionSingleFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar:file.cpp", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        ASSERT_EQUALS("", logger->str());
    }

    void suppressionTwo() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar,noConstructor", "file.cpp"};
        TODO_ASSERT_EQUALS(true, false, parser->parseFromArgs(3, argv));
        TODO_ASSERT_EQUALS(true, false, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        TODO_ASSERT_EQUALS(true, false, settings->nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
        TODO_ASSERT_EQUALS("", "cppcheck: error: Failed to add suppression. Invalid id \"uninitvar,noConstructor\"\n", logger->str());
    }

    void suppressionTwoSeparate() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar", "--suppress=noConstructor", "file.cpp"};
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
        ASSERT_EQUALS("", logger->str());
    }

    void templates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template={file}:{line},{severity},{id},{message}", "--template-location={file}:{line}:{column} {info}", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("{file}:{line},{severity},{id},{message}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column} {info}", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void templatesGcc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=gcc", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: warning: {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void templatesVs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=vs", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}({line}): {severity}: {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void templatesEdit() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=edit", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file} +{line}: {severity}: {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void templatesCppcheck1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=cppcheck1", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{callstack}: ({severity}{inconclusive:, inconclusive}) {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void templatesDaca2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=daca2", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}", settings->templateLocation);
        ASSERT_EQUALS(true, settings->daca);
        ASSERT_EQUALS("", logger->str());
    }

    void templatesSelfcheck() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=selfcheck", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    // TODO: we should bail out on this
    void templatesNoPlaceholder() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=selfchek", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        TODO_ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("selfchek", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void templateFormatInvalid() {
        REDIRECT;
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        const char* const argv[] = { "cppcheck", "--template", "file.cpp" };
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--template\".\n", logger->str());
    }

    // will use the default
    // TODO: bail out on empty?
    void templateFormatEmpty() {
        REDIRECT;
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        const char* const argv[] = { "cppcheck", "--template=", "file.cpp" };
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {inconclusive:}{severity}:{inconclusive: inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void templateLocationInvalid() {
        REDIRECT;
        const char* const argv[] = { "cppcheck", "--template-location", "--template={file}", "file.cpp" };
        ASSERT(!parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--template-location\".\n", logger->str());
    }

    // will use the default
    // TODO: bail out on empty?
    void templateLocationEmpty() {
        REDIRECT;
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        const char* const argv[] = { "cppcheck", "--template-location=", "file.cpp" };
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {inconclusive:}{severity}:{inconclusive: inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
        ASSERT_EQUALS("", logger->str());
    }

    void xml() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(1, settings->xml_version);
        ASSERT_EQUALS("", logger->str());
    }

    void xmlver2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(2, settings->xml_version);
        ASSERT_EQUALS("", logger->str());
    }

    void xmlver2both() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=2", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(2, settings->xml_version);
        ASSERT_EQUALS("", logger->str());
    }

    void xmlver2both2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "--xml", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(2, settings->xml_version);
        ASSERT_EQUALS("", logger->str());
    }

    void xmlverunknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=3", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: '--xml-version' can only be 2.\n", logger->str());
    }

    void xmlverinvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=a", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--xml-version=' is not valid - not an integer.\n", logger->str());
    }

    void doc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--doc"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT(parser->exitAfterPrinting());
        ASSERT(startsWith(logger->str(), "## "));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void showtimeSummary() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=summary", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_SUMMARY);
        ASSERT_EQUALS("", logger->str());
    }

    void showtimeFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=file", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_FILE);
        ASSERT_EQUALS("", logger->str());
    }

    void showtimeFileTotal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=file-total", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_FILE_TOTAL);
        ASSERT_EQUALS("", logger->str());
    }

    void showtimeTop5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_TOP5_FILE);
        ASSERT_EQUALS("cppcheck: --showtime=top5 is deprecated and will be removed in Cppcheck 2.14. Please use --showtime=top5_file or --showtime=top5_summary instead.\n", logger->str());
    }

    void showtimeTop5File() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5_file", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_TOP5_FILE);
        ASSERT_EQUALS("", logger->str());
    }

    void showtimeTop5Summary() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5_summary", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_TOP5_SUMMARY);
        ASSERT_EQUALS("", logger->str());
    }

    void showtimeNone() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=none", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_FILE;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_NONE);
        ASSERT_EQUALS("", logger->str());
    }

    void showtimeEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: no mode provided for --showtime\n", logger->str());
    }

    void showtimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top10", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized --showtime mode: 'top10'. Supported modes: file, file-total, summary, top5, top5_file, top5_summary.\n", logger->str());
    }

    void errorlist1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT(parser->getShowErrorMessages());
        ASSERT_EQUALS("", logger->str());
    }

    void errorlistverbose1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "--errorlist"};
        settings->verbose = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->verbose);
        ASSERT_EQUALS("", logger->str());
    }

    void errorlistverbose2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist", "--verbose"};
        settings->verbose = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->verbose);
        ASSERT_EQUALS("", logger->str());
    }

    void ignorepathsnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i"};
        // Fails since no ignored path given
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-i' is missing.\n", logger->str());
    }

#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
    void exceptionhandling() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling", "file.cpp"};
        settings->exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stderr);
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stderr, CppCheckExecutor::getExceptionOutput());
        ASSERT_EQUALS("", logger->str());
    }

    void exceptionhandling2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stderr", "file.cpp"};
        settings->exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stdout);
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stderr, CppCheckExecutor::getExceptionOutput());
        ASSERT_EQUALS("", logger->str());
    }

    void exceptionhandling3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stdout", "file.cpp"};
        settings->exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stderr);
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stdout, CppCheckExecutor::getExceptionOutput());
        ASSERT_EQUALS("", logger->str());
    }

    void exceptionhandlingInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=exfile"};
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: invalid '--exception-handling' argument\n", logger->str());
    }

    void exceptionhandlingInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling-foo"};
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exception-handling-foo\".\n", logger->str());
    }
#else
    void exceptionhandlingNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.\n", logger->str());
    }

    void exceptionhandlingNotSupported2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stderr", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.\n", logger->str());
    }
#endif

    void clang() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang", "file.cpp"};
        settings->clang = false;
        settings->clangExecutable = "exe";
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->clang);
        ASSERT_EQUALS("exe", settings->clangExecutable);
        ASSERT_EQUALS("", logger->str());
    }

    void clang2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang=clang-14", "file.cpp"};
        settings->clang = false;
        settings->clangExecutable = "";
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT(settings->clang);
        ASSERT_EQUALS("clang-14", settings->clangExecutable);
        ASSERT_EQUALS("", logger->str());
    }

    void clangInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang-foo"};
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--clang-foo\".\n", logger->str());
    }

    void valueFlowMaxIterations() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=0", "file.cpp"};
        settings->valueFlowMaxIterations = SIZE_MAX;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(0, settings->valueFlowMaxIterations);
        ASSERT_EQUALS("", logger->str());
    }

    void valueFlowMaxIterations2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=11", "file.cpp"};
        settings->valueFlowMaxIterations = SIZE_MAX;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(11, settings->valueFlowMaxIterations);
        ASSERT_EQUALS("", logger->str());
    }

    void valueFlowMaxIterationsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations"};
        ASSERT(!parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--valueflow-max-iterations\".\n", logger->str());
    }

    void valueFlowMaxIterationsInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=seven"};
        ASSERT(!parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iterations=' is not valid - not an integer.\n", logger->str());
    }

    void valueFlowMaxIterationsInvalid3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=-1"};
        ASSERT(!parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iterations=' is not valid - needs to be positive.\n", logger->str());
    }

    void checksMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=12", "file.cpp"};
        settings->checksMaxTime = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->checksMaxTime);
        ASSERT_EQUALS("", logger->str());
    }

    void checksMaxTime2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=-1", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--checks-max-time=' needs to be a positive integer.\n", logger->str());
    }

    void checksMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=one", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--checks-max-time=' is not valid - not an integer.\n", logger->str());
    }

#ifdef THREADING_MODEL_FORK
    void loadAverage() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "12", "file.cpp"};
        settings->loadAverage = 0;
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(12, settings->loadAverage);
        ASSERT_EQUALS("", logger->str());
    }

    void loadAverage2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l12", "file.cpp"};
        settings->loadAverage = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->loadAverage);
        ASSERT_EQUALS("", logger->str());
    }

    void loadAverageInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "one", "file.cpp"};
        ASSERT(!parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-l' is not valid - not an integer.\n", logger->str());
    }
#else
    void loadAverageNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "12", "file.cpp"};
        ASSERT(!parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: Option -l cannot be used as Cppcheck has not been built with fork threading model.\n", logger->str());
    }
#endif

    void maxCtuDepth() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=12", "file.cpp"};
        settings->maxCtuDepth = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->maxCtuDepth);
        ASSERT_EQUALS("", logger->str());
    }

    void maxCtuDepthInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=one", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-ctu-depth=' is not valid - not an integer.\n", logger->str());
    }

    void performanceValueflowMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-time=12", "file.cpp"};
        settings->performanceValueFlowMaxTime = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->performanceValueFlowMaxTime);
        ASSERT_EQUALS("", logger->str());
    }

    void performanceValueflowMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-time=one", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--performance-valueflow-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void performanceValueFlowMaxIfCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-if-count=12", "file.cpp"};
        settings->performanceValueFlowMaxIfCount = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->performanceValueFlowMaxIfCount);
        ASSERT_EQUALS("", logger->str());
    }

    void performanceValueFlowMaxIfCountInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-if-count=one", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--performance-valueflow-max-if-count=' is not valid - not an integer.\n", logger->str());
    }

    void templateMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=12", "file.cpp"};
        settings->templateMaxTime = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->templateMaxTime);
        ASSERT_EQUALS("", logger->str());
    }

    void templateMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=one", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--template-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void templateMaxTimeInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=-1", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--template-max-time=' is not valid - needs to be positive.\n", logger->str());
    }

    void typedefMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=12", "file.cpp"};
        settings->typedefMaxTime = 0;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->typedefMaxTime);
        ASSERT_EQUALS("", logger->str());
    }

    void typedefMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=one", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--typedef-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void typedefMaxTimeInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=-1", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--typedef-max-time=' is not valid - needs to be positive.\n", logger->str());
    }

    void project() {
        REDIRECT;
        ScopedFile file("project.cppcheck",
                        "<project>\n"
                        "<paths>\n"
                        "<dir name=\"dir\"/>\n"
                        "</paths>\n"
                        "</project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck"};
        ASSERT(parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(1, parser->getPathNames().size());
        auto it = parser->getPathNames().cbegin();
        ASSERT_EQUALS("dir", *it);
        ASSERT_EQUALS("", logger->str());
    }

    void projectMultiple() {
        REDIRECT;
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT(!parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: multiple --project options are not supported.\n", logger->str());
    }

    void projectAndSource() {
        REDIRECT;
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --project cannot be used in conjunction with source files.\n", logger->str());
    }

    void projectEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project=", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: failed to open project ''. The file does not exist.\n", logger->str());
    }

    void projectMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: failed to open project 'project.cppcheck'. The file does not exist.\n", logger->str());
    }

    void projectNoPaths() {
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck"};
        ASSERT(!parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", logger->str());
    }

    void addon() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--addon=misra", "file.cpp"};
        settings->addons.clear();
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->addons.size());
        ASSERT_EQUALS("misra", *settings->addons.cbegin());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void signedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--fsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS('s', settings->platform.defaultSign);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void signedChar2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=avr8", "--fsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS('s', settings->platform.defaultSign);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void unsignedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--funsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void unsignedChar2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=mips32", "--funsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void signedCharUnsignedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--fsigned-char", "--funsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

#ifdef HAVE_RULES
    void rule() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=.+", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->rules.size());
        auto it = settings->rules.cbegin();
        ASSERT_EQUALS(".+", it->pattern);
        ASSERT_EQUALS("", logger->str());
    }
#else
    void ruleNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=.+", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: Option --rule cannot be used as Cppcheck has not been built with rules support.\n", logger->str());
    }
#endif

#ifdef HAVE_RULES
    void ruleFile() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rules>\n"
                        "<rule>\n"
                        "<pattern>.+</pattern>\n"
                        "</rule>\n"
                        "</rules>");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->rules.size());
        auto it = settings->rules.cbegin();
        ASSERT_EQUALS(".+", it->pattern);
        ASSERT_EQUALS("", logger->str());
    }

    void ruleFileEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file '' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void ruleFileMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void ruleFileInvalid() {
        REDIRECT;
        ScopedFile file("rule.xml", "");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' (XML_ERROR_EMPTY_DOCUMENT).\n", logger->str());
    }
#else
    void ruleFileNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: Option --rule-file cannot be used as Cppcheck has not been built with rules support.\n", logger->str());
    }
#endif

    void ignorepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("", logger->str());
    }

    void ignorepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "file.cpp"};
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("", logger->str());
    }

    void ignorepaths3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "-imodule", "file.cpp"};
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(2, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("module", parser->getIgnoredPaths()[1]);
        ASSERT_EQUALS("", logger->str());
    }

    void ignorepaths4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "-i", "module", "file.cpp"};
        ASSERT(parser->parseFromArgs(6, argv));
        ASSERT_EQUALS(2, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("module", parser->getIgnoredPaths()[1]);
        ASSERT_EQUALS("", logger->str());
    }

    void ignorefilepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-ifoo.cpp", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("foo.cpp", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("", logger->str());
    }

    void ignorefilepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc/foo.cpp", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src/foo.cpp", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("", logger->str());
    }

    void checkconfig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-config", "file.cpp"};
        settings->checkConfiguration = false;
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->checkConfiguration);
        ASSERT_EQUALS("", logger->str());
    }

    void unknownParam() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--foo", "file.cpp"};
        ASSERT(!parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--foo\".\n", logger->str());
    }

    void undefs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "file.cpp"};
        ASSERT(parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->userUndefs.size());
        ASSERT(settings->userUndefs.find("_WIN32") != settings->userUndefs.end());
        ASSERT_EQUALS("", logger->str());
    }

    void undefs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "-UNODEBUG", "file.cpp"};
        ASSERT(parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(2, settings->userUndefs.size());
        ASSERT(settings->userUndefs.find("_WIN32") != settings->userUndefs.end());
        ASSERT(settings->userUndefs.find("NODEBUG") != settings->userUndefs.end());
        ASSERT_EQUALS("", logger->str());
    }

    void undefs_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void undefs_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "-v", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void undefs_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "--quiet", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void cppcheckBuildDirExistent() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=.", "file.cpp"};
        ASSERT_EQUALS(true, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("", logger->str());
    }

    void cppcheckBuildDirNonExistent() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=non-existent-path"};
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: Directory 'non-existent-path' specified by --cppcheck-build-dir argument has to be existent.\n", logger->str());
    }

    void cppcheckBuildDirEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir="};
        ASSERT_EQUALS(false, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: Directory '' specified by --cppcheck-build-dir argument has to be existent.\n", logger->str());
    }
};

REGISTER_TEST(TestCmdlineParser)

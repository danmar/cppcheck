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
#include "path.h"
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
                throw std::runtime_error("unconsumed messages: " + buf);
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
    }

    // add overload so the enums can be compared without a cast
    template<typename T, typename U>
    bool assertEquals(const char* const filename, const unsigned int linenr, const T& expected, const U& actual) const
    {
        return TestFixture::assertEquals(filename, linenr, expected, actual);
    }

    bool assertEquals(const char* const filename, const unsigned int linenr, const CmdLineParser::Result& expected, const CmdLineParser::Result& actual) const
    {
        return TestFixture::assertEquals(filename, linenr, static_cast<int>(expected), static_cast<int>(actual));
    }

    void run() override {
        TEST_CASE(nooptions);
        TEST_CASE(helpshort);
        TEST_CASE(helpshortExclusive);
        TEST_CASE(helplong);
        TEST_CASE(helplongExclusive);
        TEST_CASE(version);
        TEST_CASE(versionWithCfg);
        TEST_CASE(versionExclusive);
        TEST_CASE(versionWithInvalidCfg);
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
        TEST_CASE(disabledMissingIncludeWithInformation);
        TEST_CASE(enabledMissingIncludeWithInformation);
        TEST_CASE(enabledMissingIncludeWithInformationReverseOrder);
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
        TEST_CASE(docExclusive);
        TEST_CASE(showtimeFile);
        TEST_CASE(showtimeFileTotal);
        TEST_CASE(showtimeTop5);
        TEST_CASE(showtimeTop5File);
        TEST_CASE(showtimeTop5Summary);
        TEST_CASE(showtimeNone);
        TEST_CASE(showtimeEmpty);
        TEST_CASE(showtimeInvalid);
        TEST_CASE(errorlist);
        TEST_CASE(errorlistWithCfg);
        TEST_CASE(errorlistExclusive);
        TEST_CASE(errorlistWithInvalidCfg);
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
        TEST_CASE(addonMissing);
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
        TEST_CASE(library);
        TEST_CASE(libraryMissing);

        TEST_CASE(ignorepaths1);
        TEST_CASE(ignorepaths2);
        TEST_CASE(ignorepaths3);
        TEST_CASE(ignorepaths4);
        TEST_CASE(ignorefilepaths1);
        TEST_CASE(ignorefilepaths2);
        TEST_CASE(ignorefilepaths3);

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

        TEST_CASE(invalidCppcheckCfg);
    }

    void nooptions() {
        REDIRECT;
        const char * const argv[] = {"cppcheck"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(1, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helpshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-h"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(2, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helpshortExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "-h"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(3, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helplong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--help"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(2, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helplongExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--help"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(3, argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void version() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--version"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(2, argv));
        ASSERT(logger->str().compare(0, 11, "Cppcheck 2.") == 0);
    }

    void versionWithCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n"
                        "\"productName\": \"The Product\""
                        "}\n");
        const char * const argv[] = {"cppcheck", "--version"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(2, argv));
        // TODO: somehow the config is not loaded on some systems
        (void)logger->str(); //ASSERT_EQUALS("The Product\n", logger->str()); // TODO: include version?
    }

    // TODO: test --version with extraVersion

    void versionExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--version"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(3, argv));
        ASSERT(logger->str().compare(0, 11, "Cppcheck 2.") == 0);
    }

    void versionWithInvalidCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n");
        const char * const argv[] = {"cppcheck", "--version"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 2 near: \n", logger->str());
    }

    void onefile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser->getPathNames().size());
        ASSERT_EQUALS("file.cpp", parser->getPathNames().at(0));
    }

    void onepath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "src"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser->getPathNames().size());
        ASSERT_EQUALS("src", parser->getPathNames().at(0));
    }

    void optionwithoutfile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(0, (int)parser->getPathNames().size());
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", logger->str());
    }

    void verboseshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v", "file.cpp"};
        settings->verbose = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->verbose);
    }

    void verboselong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "file.cpp"};
        settings->verbose = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->verbose);
    }

    void debugSimplified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-simplified", "file.cpp"};
        settings->debugSimplified = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->debugSimplified);
    }

    void debugwarnings() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-warnings", "file.cpp"};
        settings->debugwarnings = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->debugwarnings);
    }

    void forceshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "file.cpp"};
        settings->force = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->force);
    }

    void forcelong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--force", "file.cpp"};
        settings->force = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->force);
    }

    void relativePaths1() {
        REDIRECT;
        settings->relativePaths = false;
        const char * const argv[] = {"cppcheck", "-rp", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
    }

    void relativePaths2() {
        REDIRECT;
        settings->relativePaths = false;
        const char * const argv[] = {"cppcheck", "--relative-paths", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
    }

    void relativePaths3() {
        REDIRECT;
        settings->relativePaths = false;
        settings->basePaths.clear();
        const char * const argv[] = {"cppcheck", "-rp=C:/foo;C:\\bar", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS(2, settings->basePaths.size());
        ASSERT_EQUALS("C:/foo", settings->basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings->basePaths[1]);
    }

    void relativePaths4() {
        REDIRECT;
        settings->relativePaths = false;
        settings->basePaths.clear();

        const char * const argv[] = {"cppcheck", "--relative-paths=C:/foo;C:\\bar", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS(2, settings->basePaths.size());
        ASSERT_EQUALS("C:/foo", settings->basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings->basePaths[1]);
    }

    void quietshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-q", "file.cpp"};
        settings->quiet = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->quiet);
    }

    void quietlong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--quiet", "file.cpp"};
        settings->quiet = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->quiet);
    }

    void defines_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D"};
        // Fails since -D has no param
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "-v", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "--quiet", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "file.cpp"};
        settings->userDefines.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("_WIN32=1", settings->userDefines);
    }

    void defines2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "-DNODEBUG", "file.cpp"};
        settings->userDefines.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("_WIN32=1;NODEBUG=1", settings->userDefines);
    }

    void defines3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "DEBUG", "file.cpp"};
        settings->userDefines.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("DEBUG=1", settings->userDefines);
    }

    void defines4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-DDEBUG=", "file.cpp"}; // #5137 - defining empty macro
        settings->userDefines.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("DEBUG=", settings->userDefines);
    }

    void enforceLanguage1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(Settings::Language::None, settings->enforcedLang);
    }

    void enforceLanguage2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x", "c++", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(Settings::Language::CPP, settings->enforcedLang);
    }

    void enforceLanguage3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", logger->str());
    }

    void enforceLanguage4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x", "--inconclusive", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", logger->str());
    }

    void enforceLanguage5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=c++", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Language::CPP, settings->enforcedLang);
    }

    void enforceLanguage6() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=c", "file.cpp"};
        settings->enforcedLang = Settings::Language::None;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Language::C, settings->enforcedLang);
    }

    void enforceLanguage7() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=unknownLanguage", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unknown language 'unknownLanguage' enforced.\n", logger->str());
    }

    void includesnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I"};
        // Fails since -I has no param
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-I' is missing.\n", logger->str());
    }

    void includes() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include", "file.cpp"};
        settings->includePaths.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includesslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "file.cpp"};
        settings->includePaths.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includesbackslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include\\", "file.cpp"};
        settings->includePaths.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includesnospace() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-Iinclude", "file.cpp"};
        settings->includePaths.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includes2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "-I", "framework/", "file.cpp"};
        settings->includePaths.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(6, argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
        settings->includePaths.pop_front();
        ASSERT_EQUALS("framework/", settings->includePaths.front());
    }

    void includesFile() {
        REDIRECT;
        ScopedFile file("includes.txt",
                        "path/sub\n"
                        "path2/sub1\n");
        const char * const argv[] = {"cppcheck", "--includes-file=includes.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->includePaths.size());
        auto it = settings->includePaths.cbegin();
        ASSERT_EQUALS("path/sub/", *it++);
        ASSERT_EQUALS("path2/sub1/", *it);
    }

    void includesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--includes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to open includes file at 'fileThatDoesNotExist.txt'\n", logger->str());
    }

    void configExcludesFile() {
        REDIRECT;
        ScopedFile file("excludes.txt",
                        "path/sub\n"
                        "path2/sub1\n");
        const char * const argv[] = {"cppcheck", "--config-excludes-file=excludes.txt", "file.cpp"};
        settings->configExcludePaths.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->configExcludePaths.size());
        auto it = settings->configExcludePaths.cbegin();
        ASSERT_EQUALS("path/sub/", *it++);
        ASSERT_EQUALS("path2/sub1/", *it);
    }

    void configExcludesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--config-excludes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to open config excludes file at 'fileThatDoesNotExist.txt'\n", logger->str());
    }

    void enabledAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT(!settings->checks.isEnabled(Checks::internalCheck));
    }

    void enabledStyle() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::internalCheck));
    }

    void enabledPerformance() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=performance", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(!settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->severity.isEnabled(Severity::performance));
        ASSERT(!settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
    }

    void enabledPortability() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=portability", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(!settings->severity.isEnabled(Severity::warning));
        ASSERT(!settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
    }

    void enabledInformation() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("cppcheck: '--enable=information' will no longer implicitly enable 'missingInclude' starting with 2.16. Please enable it explicitly if you require it.\n", logger->str());
    }

    void enabledUnusedFunction() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=unusedFunction", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->checks.isEnabled(Checks::unusedFunction));
    }

    void enabledMissingInclude() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
    }

    void disabledMissingIncludeWithInformation() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=missingInclude", "--enable=information", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledMissingIncludeWithInformation() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "--enable=missingInclude", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledMissingIncludeWithInformationReverseOrder() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "--enable=information", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

#ifdef CHECK_INTERNAL
    void enabledInternal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=internal", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->checks.isEnabled(Checks::internalCheck));
    }
#endif

    void enabledMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude,portability,warning", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(!settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
    }

    void enabledInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=warning,missingIncludeSystem,style", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'missingIncludeSystem'\n", logger->str());
    }

    void enabledError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=error", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'error'\n", logger->str());
    }

    void enabledEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter is empty\n", logger->str());
    }


    void disableAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=all", "file.cpp"};
        settings->severity.clear();
        settings->checks.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
    }

    void disableMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=style", "--disable=unusedFunction", "file.cpp"};
        settings->severity.clear();
        settings->checks.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(5, argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
    }

    // make sure the implied "style" checks are not added when "--enable=style" is specified
    void disableStylePartial() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "--disable=performance", "--enable=unusedFunction", "file.cpp"};
        settings->severity.clear();
        settings->checks.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(5, argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(true, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
    }

    void disableInformationPartial() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "--disable=missingInclude", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void disableInformationPartial2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "--disable=information", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT(!settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
    }

    void disableInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=leaks", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'leaks'\n", logger->str());
    }

    void disableError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=error", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'error'\n", logger->str());
    }

    void disableEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter is empty\n", logger->str());
    }

    void inconclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inconclusive", "file.cpp"};
        settings->certainty.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->certainty.isEnabled(Certainty::inconclusive));
    }

    void errorExitcode() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=5", "file.cpp"};
        settings->exitCode = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(5, settings->exitCode);
    }

    void errorExitcodeMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=", "file.cpp"};
        // Fails since exit code not given
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--error-exitcode=' is not valid - not an integer.\n", logger->str());
    }

    void errorExitcodeStr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=foo", "file.cpp"};
        // Fails since invalid exit code
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--error-exitcode=' is not valid - not an integer.\n", logger->str());
    }

    void exitcodeSuppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "suppr.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", logger->str());
    }

    void exitcodeSuppressions() {
        REDIRECT;
        ScopedFile file("suppr.txt",
                        "uninitvar\n"
                        "unusedFunction\n");
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions=suppr.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->nofail.getSuppressions().size());
        auto it = settings->nofail.getSuppressions().cbegin();
        ASSERT_EQUALS("uninitvar", (it++)->errorId);
        ASSERT_EQUALS("unusedFunction", it->errorId);
    }

    void exitcodeSuppressionsNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", logger->str());
    }

    void fileList() {
        REDIRECT;
        ScopedFile file("files.txt",
                        "file1.c\n"
                        "file2.cpp\n");
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(3, parser->getPathNames().size());
        auto it = parser->getPathNames().cbegin();
        ASSERT_EQUALS("file1.c", *it++);
        ASSERT_EQUALS("file2.cpp", *it++);
        ASSERT_EQUALS("file.cpp", *it);
    }

    void fileListNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: couldn't open the file: \"files.txt\".\n", logger->str());
    }

    /*    void fileListStdin() {
            // TODO: Give it some stdin to read from, fails because the list of
            // files in stdin (_pathnames) is empty
            REDIRECT;
            const char * const argv[] = {"cppcheck", "--file-list=-", "file.cpp"};
            TODO_ASSERT_EQUALS(true, false, parser->parseFromArgs(3, argv));
        } */

    void fileListInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list", "files.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--file-list\".\n", logger->str());
    }

    void inlineSuppr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inline-suppr", "file.cpp"};
        settings->inlineSuppressions = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->inlineSuppressions);
    }

    void jobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "3", "file.cpp"};
        settings->jobs = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(3, settings->jobs);
    }

    void jobs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j3", "file.cpp"};
        settings->jobs = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(3, settings->jobs);
    }

    void jobsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "file.cpp"};
        // Fails since -j is missing thread count
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not valid - not an integer.\n", logger->str());
    }

    void jobsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "e", "file.cpp"};
        // Fails since invalid count given for -j
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not valid - not an integer.\n", logger->str());
    }

    void jobsNoJobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j0", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument for '-j' must be greater than 0.\n", logger->str());
    }

    void jobsTooBig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j1025", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument for '-j' is allowed to be 1024 at max.\n", logger->str());
    }

    void maxConfigs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "--max-configs=12", "file.cpp"};
        settings->force = false;
        settings->maxConfigs = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(12, settings->maxConfigs);
        ASSERT_EQUALS(false, settings->force);
    }

    void maxConfigsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=", "file.cpp"};
        // Fails since --max-configs= is missing limit
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not valid - not an integer.\n", logger->str());
    }

    void maxConfigsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=e", "file.cpp"};
        // Fails since invalid count given for --max-configs=
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not valid - not an integer.\n", logger->str());
    }

    void maxConfigsTooSmall() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=0", "file.cpp"};
        // Fails since limit must be greater than 0
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' must be greater than 0.\n", logger->str());
    }

    void reportProgress1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(10, settings->reportProgress);
    }

    void reportProgress2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--report-progress=' is not valid - not an integer.\n", logger->str());
    }

    void reportProgress3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=-1", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--report-progress=' needs to be a positive integer.\n", logger->str());
    }

    void reportProgress4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=0", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(0, settings->reportProgress);
    }

    void reportProgress5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=1", "file.cpp"};
        settings->reportProgress = -1;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->reportProgress);
    }

    void stdc99() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c99", "file.cpp"};
        settings->standards.c = Standards::C89;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->standards.c == Standards::C99);
    }

    void stdcpp11() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c++11", "file.cpp"};
        settings->standards.cpp = Standards::CPP03;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->standards.cpp == Standards::CPP11);
    }

    void stdunknown1() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=d++11", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unknown --std value 'd++11'\n", logger->str());
    }

    void stdunknown2() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=cplusplus11", "file.cpp"};
        TODO_ASSERT_EQUALS(static_cast<int>(CmdLineParser::Result::Fail), static_cast<int>(CmdLineParser::Result::Success), static_cast<int>(parser->parseFromArgs(3, argv)));
        TODO_ASSERT_EQUALS("cppcheck: error: unknown --std value 'cplusplus11'\n", "", logger->str());
    }

    void platformWin64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win64", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Win64, settings->platform.type);
    }

    void platformWin32A() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32A", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Win32A, settings->platform.type);
    }

    void platformWin32W() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32W", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Win32W, settings->platform.type);
    }

    void platformUnix32() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix32, settings->platform.type);
    }

    void platformUnix32Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32-unsigned", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix32, settings->platform.type);
    }

    void platformUnix64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix64, settings->platform.type);
    }

    void platformUnix64Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64-unsigned", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unix64, settings->platform.type);
    }

    void platformNative() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=native", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Native, settings->platform.type);
    }

    void platformUnspecified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unspecified", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Native));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::Unspecified, settings->platform.type);
    }

    void platformPlatformFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=avr8", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(Platform::Type::File, settings->platform.type);
    }

    void platformUnknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win128", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized platform: 'win128'.\n", logger->str());
    }

    void plistEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=", "file.cpp"};
        settings->plistOutput = "";
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->plistOutput == "./");
    }

    void plistDoesNotExist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=./cppcheck_reports", "file.cpp"};
        // Fails since folder pointed by --plist-output= does not exist
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: plist folder does not exist: 'cppcheck_reports'.\n", logger->str());
    }

    void suppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions", "suppr.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--suppressions\".\n", logger->str());
    }

    void suppressions() {
        REDIRECT;
        ScopedFile file("suppr.txt",
                        "uninitvar\n"
                        "unusedFunction\n");
        const char * const argv[] = {"cppcheck", "--suppressions-list=suppr.txt", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(2, settings->nomsg.getSuppressions().size());
        auto it = settings->nomsg.getSuppressions().cbegin();
        ASSERT_EQUALS("uninitvar", (it++)->errorId);
        ASSERT_EQUALS("unusedFunction", it->errorId);
    }

    void suppressionsNoFile1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(false, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    void suppressionsNoFile2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr,b.suppr", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    void suppressionsNoFile3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr b.suppr", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
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
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1)));
    }

    void suppressionSingleFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar:file.cpp", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
    }

    void suppressionTwo() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar,noConstructor", "file.cpp"};
        TODO_ASSERT_EQUALS(static_cast<int>(CmdLineParser::Result::Success), static_cast<int>(CmdLineParser::Result::Fail), static_cast<int>(parser->parseFromArgs(3, argv)));
        TODO_ASSERT_EQUALS(true, false, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        TODO_ASSERT_EQUALS(true, false, settings->nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
        TODO_ASSERT_EQUALS("", "cppcheck: error: Failed to add suppression. Invalid id \"uninitvar,noConstructor\"\n", logger->str());
    }

    void suppressionTwoSeparate() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar", "--suppress=noConstructor", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        ASSERT_EQUALS(true, settings->nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
    }

    void templates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template={file}:{line},{severity},{id},{message}", "--template-location={file}:{line}:{column} {info}", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("{file}:{line},{severity},{id},{message}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column} {info}", settings->templateLocation);
    }

    void templatesGcc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=gcc", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: warning: {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    void templatesVs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=vs", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}({line}): {severity}: {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templatesEdit() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=edit", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file} +{line}: {severity}: {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templatesCppcheck1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=cppcheck1", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{callstack}: ({severity}{inconclusive:, inconclusive}) {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templatesDaca2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=daca2", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}", settings->templateLocation);
        ASSERT_EQUALS(true, settings->daca);
    }

    void templatesSelfcheck() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=selfcheck", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    // TODO: we should bail out on this
    void templatesNoPlaceholder() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=selfchek", "file.cpp"};
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        TODO_ASSERT_EQUALS(static_cast<int>(CmdLineParser::Result::Fail), static_cast<int>(CmdLineParser::Result::Success), static_cast<int>(parser->parseFromArgs(3, argv)));
        ASSERT_EQUALS("selfchek", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templateFormatInvalid() {
        REDIRECT;
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        const char* const argv[] = { "cppcheck", "--template", "file.cpp" };
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--template\".\n", logger->str());
    }

    // will use the default
    // TODO: bail out on empty?
    void templateFormatEmpty() {
        REDIRECT;
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        const char* const argv[] = { "cppcheck", "--template=", "file.cpp" };
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {inconclusive:}{severity}:{inconclusive: inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    void templateLocationInvalid() {
        REDIRECT;
        const char* const argv[] = { "cppcheck", "--template-location", "--template={file}", "file.cpp" };
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--template-location\".\n", logger->str());
    }

    // will use the default
    // TODO: bail out on empty?
    void templateLocationEmpty() {
        REDIRECT;
        settings->templateFormat.clear();
        settings->templateLocation.clear();
        const char* const argv[] = { "cppcheck", "--template-location=", "file.cpp" };
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {inconclusive:}{severity}:{inconclusive: inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    void xml() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(1, settings->xml_version);
    }

    void xmlver2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void xmlver2both() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=2", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void xmlver2both2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "--xml", "file.cpp"};
        settings->xml_version = 1;
        settings->xml = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT(settings->xml);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void xmlverunknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=3", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: '--xml-version' can only be 2.\n", logger->str());
    }

    void xmlverinvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=a", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--xml-version=' is not valid - not an integer.\n", logger->str());
    }

    void doc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--doc"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(2, argv));
        ASSERT(startsWith(logger->str(), "## "));
    }

    void docExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--doc"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(3, argv));
        ASSERT(startsWith(logger->str(), "## "));
    }

    void showtimeSummary() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=summary", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_SUMMARY);
    }

    void showtimeFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=file", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_FILE);
    }

    void showtimeFileTotal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=file-total", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_FILE_TOTAL);
    }

    void showtimeTop5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_TOP5_FILE);
        ASSERT_EQUALS("cppcheck: --showtime=top5 is deprecated and will be removed in Cppcheck 2.14. Please use --showtime=top5_file or --showtime=top5_summary instead.\n", logger->str());
    }

    void showtimeTop5File() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5_file", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_TOP5_FILE);
    }

    void showtimeTop5Summary() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5_summary", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_TOP5_SUMMARY);
    }

    void showtimeNone() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=none", "file.cpp"};
        settings->showtime = SHOWTIME_MODES::SHOWTIME_FILE;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->showtime == SHOWTIME_MODES::SHOWTIME_NONE);
    }

    void showtimeEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: no mode provided for --showtime\n", logger->str());
    }

    void showtimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top10", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized --showtime mode: 'top10'. Supported modes: file, file-total, summary, top5, top5_file, top5_summary.\n", logger->str());
    }

    void errorlist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("", logger->str()); // empty since it is logged via ErrorLogger
        const std::string errout_s = GET_REDIRECT_OUTPUT;
        ASSERT(startsWith(errout_s, ErrorMessage::getXMLHeader("")));
        ASSERT(endsWith(errout_s, "</results>\n"));
    }

    void errorlistWithCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        R"({"productName": "The Product"}\n)");
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("", logger->str()); // empty since it is logged via ErrorLogger
        ASSERT(startsWith(GET_REDIRECT_OUTPUT, ErrorMessage::getXMLHeader("The Product")));
    }

    void errorlistExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--errorlist"};
        ASSERT_EQUALS(CmdLineParser::Result::Exit, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("", logger->str()); // empty since it is logged via ErrorLogger
        const std::string errout_s = GET_REDIRECT_OUTPUT;
        ASSERT(startsWith(errout_s, ErrorMessage::getXMLHeader("")));
        ASSERT(endsWith(errout_s, "</results>\n"));
    }

    void errorlistWithInvalidCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n");
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 2 near: \n", logger->str());
    }

    void ignorepathsnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i"};
        // Fails since no ignored path given
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-i' is missing.\n", logger->str());
    }

#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
    void exceptionhandling() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling", "file.cpp"};
        settings->exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stderr);
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stderr, CppCheckExecutor::getExceptionOutput());
    }

    void exceptionhandling2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stderr", "file.cpp"};
        settings->exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stdout);
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stderr, CppCheckExecutor::getExceptionOutput());
    }

    void exceptionhandling3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stdout", "file.cpp"};
        settings->exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stderr);
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stdout, CppCheckExecutor::getExceptionOutput());
    }

    void exceptionhandlingInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=exfile"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: invalid '--exception-handling' argument\n", logger->str());
    }

    void exceptionhandlingInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling-foo"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exception-handling-foo\".\n", logger->str());
    }
#else
    void exceptionhandlingNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.\n", logger->str());
    }

    void exceptionhandlingNotSupported2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stderr", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.\n", logger->str());
    }
#endif

    void clang() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang", "file.cpp"};
        settings->clang = false;
        settings->clangExecutable = "exe";
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->clang);
        ASSERT_EQUALS("exe", settings->clangExecutable);
    }

    void clang2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang=clang-14", "file.cpp"};
        settings->clang = false;
        settings->clangExecutable = "";
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT(settings->clang);
        ASSERT_EQUALS("clang-14", settings->clangExecutable);
    }

    void clangInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang-foo"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--clang-foo\".\n", logger->str());
    }

    void valueFlowMaxIterations() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=0", "file.cpp"};
        settings->valueFlowMaxIterations = SIZE_MAX;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(0, settings->valueFlowMaxIterations);
    }

    void valueFlowMaxIterations2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=11", "file.cpp"};
        settings->valueFlowMaxIterations = SIZE_MAX;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(11, settings->valueFlowMaxIterations);
    }

    void valueFlowMaxIterationsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--valueflow-max-iterations\".\n", logger->str());
    }

    void valueFlowMaxIterationsInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=seven"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iterations=' is not valid - not an integer.\n", logger->str());
    }

    void valueFlowMaxIterationsInvalid3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=-1"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iterations=' is not valid - needs to be positive.\n", logger->str());
    }

    void checksMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=12", "file.cpp"};
        settings->checksMaxTime = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->checksMaxTime);
    }

    void checksMaxTime2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=-1", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--checks-max-time=' needs to be a positive integer.\n", logger->str());
    }

    void checksMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=one", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--checks-max-time=' is not valid - not an integer.\n", logger->str());
    }

#ifdef THREADING_MODEL_FORK
    void loadAverage() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "12", "file.cpp"};
        settings->loadAverage = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(12, settings->loadAverage);
    }

    void loadAverage2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l12", "file.cpp"};
        settings->loadAverage = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->loadAverage);
    }

    void loadAverageInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "one", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-l' is not valid - not an integer.\n", logger->str());
    }
#else
    void loadAverageNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "12", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: Option -l cannot be used as Cppcheck has not been built with fork threading model.\n", logger->str());
    }
#endif

    void maxCtuDepth() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=12", "file.cpp"};
        settings->maxCtuDepth = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->maxCtuDepth);
    }

    void maxCtuDepthInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=one", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-ctu-depth=' is not valid - not an integer.\n", logger->str());
    }

    void performanceValueflowMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-time=12", "file.cpp"};
        settings->performanceValueFlowMaxTime = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->performanceValueFlowMaxTime);
    }

    void performanceValueflowMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-time=one", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--performance-valueflow-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void performanceValueFlowMaxIfCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-if-count=12", "file.cpp"};
        settings->performanceValueFlowMaxIfCount = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->performanceValueFlowMaxIfCount);
    }

    void performanceValueFlowMaxIfCountInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-if-count=one", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--performance-valueflow-max-if-count=' is not valid - not an integer.\n", logger->str());
    }

    void templateMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=12", "file.cpp"};
        settings->templateMaxTime = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->templateMaxTime);
    }

    void templateMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=one", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--template-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void templateMaxTimeInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=-1", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--template-max-time=' is not valid - needs to be positive.\n", logger->str());
    }

    void typedefMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=12", "file.cpp"};
        settings->typedefMaxTime = 0;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(12, settings->typedefMaxTime);
    }

    void typedefMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=one", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--typedef-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void typedefMaxTimeInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=-1", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
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
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS(1, parser->getPathNames().size());
        auto it = parser->getPathNames().cbegin();
        ASSERT_EQUALS("dir", *it);
    }

    void projectMultiple() {
        REDIRECT;
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: multiple --project options are not supported.\n", logger->str());
    }

    void projectAndSource() {
        REDIRECT;
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --project cannot be used in conjunction with source files.\n", logger->str());
    }

    void projectEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project=", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: failed to open project ''. The file does not exist.\n", logger->str());
    }

    void projectMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: failed to open project 'project.cppcheck'. The file does not exist.\n", logger->str());
    }

    void projectNoPaths() {
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", logger->str());
    }

    void addon() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--addon=misra", "file.cpp"};
        settings->addons.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->addons.size());
        ASSERT_EQUALS("misra", *settings->addons.cbegin());
    }

    void addonMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--addon=misra2", "file.cpp"};
        settings->addons.clear();
        ASSERT(!parser->fillSettingsFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->addons.size());
        ASSERT_EQUALS("misra2", *settings->addons.cbegin());
        ASSERT_EQUALS("Did not find addon misra2.py\n", logger->str());
    }

    void signedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--fsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS('s', settings->platform.defaultSign);
    }

    void signedChar2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=avr8", "--fsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS('s', settings->platform.defaultSign);
    }

    void unsignedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--funsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
    }

    void unsignedChar2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=mips32", "--funsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
    }

    void signedCharUnsignedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--fsigned-char", "--funsigned-char", "file.cpp"};
        settings->platform.defaultSign = '\0';
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
    }

#ifdef HAVE_RULES
    void rule() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=.+", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->rules.size());
        auto it = settings->rules.cbegin();
        ASSERT_EQUALS(".+", it->pattern);
    }
#else
    void ruleNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=.+", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
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
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->rules.size());
        auto it = settings->rules.cbegin();
        ASSERT_EQUALS(".+", it->pattern);
    }

    void ruleFileEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file '' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void ruleFileMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void ruleFileInvalid() {
        REDIRECT;
        ScopedFile file("rule.xml", "");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' (XML_ERROR_EMPTY_DOCUMENT).\n", logger->str());
    }
#else
    void ruleFileNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: Option --rule-file cannot be used as Cppcheck has not been built with rules support.\n", logger->str());
    }
#endif

    void library() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=posix", "file.cpp"};
        settings->libraries.clear();
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->libraries.size());
        ASSERT_EQUALS("posix", *settings->libraries.cbegin());
    }

    void libraryMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=posix2", "file.cpp"};
        settings->libraries.clear();
        ASSERT_EQUALS(false, parser->fillSettingsFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->libraries.size());
        ASSERT_EQUALS("posix2", *settings->libraries.cbegin());
        ASSERT_EQUALS("cppcheck: Failed to load library configuration file 'posix2'. File not found\n", logger->str());
    }

    void ignorepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
    }

    void ignorepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
    }

    void ignorepaths3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "-imodule", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(2, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("module", parser->getIgnoredPaths()[1]);
    }

    void ignorepaths4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "-i", "module", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(6, argv));
        ASSERT_EQUALS(2, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser->getIgnoredPaths()[0]);
        ASSERT_EQUALS("module", parser->getIgnoredPaths()[1]);
    }

    void ignorefilepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-ifoo.cpp", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("foo.cpp", parser->getIgnoredPaths()[0]);
    }

    void ignorefilepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc/foo.cpp", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("src/foo.cpp", parser->getIgnoredPaths()[0]);
    }

    void ignorefilepaths3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "foo.cpp", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(1, parser->getIgnoredPaths().size());
        ASSERT_EQUALS("foo.cpp", parser->getIgnoredPaths()[0]);
    }

    void checkconfig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-config", "file.cpp"};
        settings->checkConfiguration = false;
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings->checkConfiguration);
    }

    void unknownParam() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--foo", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--foo\".\n", logger->str());
    }

    void undefs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings->userUndefs.size());
        ASSERT(settings->userUndefs.find("_WIN32") != settings->userUndefs.end());
    }

    void undefs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "-UNODEBUG", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS(2, settings->userUndefs.size());
        ASSERT(settings->userUndefs.find("_WIN32") != settings->userUndefs.end());
        ASSERT(settings->userUndefs.find("NODEBUG") != settings->userUndefs.end());
    }

    void undefs_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U"};
        // Fails since -U has no param
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void undefs_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "-v", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void undefs_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "--quiet", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void cppcheckBuildDirExistent() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=.", "file.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Success, parser->parseFromArgs(3, argv));
    }

    void cppcheckBuildDirNonExistent() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=non-existent-path"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: Directory 'non-existent-path' specified by --cppcheck-build-dir argument has to be existent.\n", logger->str());
    }

    void cppcheckBuildDirEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir="};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: Directory '' specified by --cppcheck-build-dir argument has to be existent.\n", logger->str());
    }

    void invalidCppcheckCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n");
        const char * const argv[] = {"cppcheck", "test.cpp"};
        ASSERT_EQUALS(CmdLineParser::Result::Fail, parser->parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 2 near: \n", logger->str());
    }
};

REGISTER_TEST(TestCmdlineParser)

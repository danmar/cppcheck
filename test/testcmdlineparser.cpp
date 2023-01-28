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

#include "cmdlineparser.h"
#include "cppcheckexecutor.h"
#include "errortypes.h"
#include "platform.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "fixture.h"
#include "timer.h"

#include <cstdio>
#include <list>
#include <set>
#include <string>
#include <vector>

class TestCmdlineParser : public TestFixture {
public:
    TestCmdlineParser()
        : TestFixture("TestCmdlineParser")
        , defParser(&settings) {
#if defined(_WIN64) || defined(_WIN32)
        CmdLineParser::SHOW_DEF_PLATFORM_MSG = false;
#endif
    }

    ~TestCmdlineParser() override {
#if defined(_WIN64) || defined(_WIN32)
        CmdLineParser::SHOW_DEF_PLATFORM_MSG = true;
#endif
    }

private:
    Settings settings; // TODO: reset after each test
    CmdLineParser defParser; // TODO: reset after each test

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
        TEST_CASE(relativePaths);
        TEST_CASE(quietshort);
        TEST_CASE(quietlong);
        TEST_CASE(defines_noarg);
        TEST_CASE(defines_noarg2);
        TEST_CASE(defines_noarg3);
        TEST_CASE(defines);
        TEST_CASE(defines2);
        TEST_CASE(defines3);
        TEST_CASE(defines4);
        TEST_CASE(enforceLanguage);
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
        TEST_CASE(jobsMissingCount);
        TEST_CASE(jobsInvalid);
        TEST_CASE(maxConfigs);
        TEST_CASE(maxConfigsMissingCount);
        TEST_CASE(maxConfigsInvalid);
        TEST_CASE(maxConfigsTooSmall);
        TEST_CASE(reportProgressTest); // "Test" suffix to avoid hiding the parent's reportProgress
        TEST_CASE(stdc99);
        TEST_CASE(stdcpp11);
        TEST_CASE(stdunknown);
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
#if defined(_WIN64) || defined(_WIN32)
        TEST_CASE(platformDefault);
        TEST_CASE(platformDefault2);
#endif
        TEST_CASE(plistEmpty);
        TEST_CASE(plistDoesNotExist);
        TEST_CASE(suppressionsOld);
        TEST_CASE(suppressions);
        TEST_CASE(suppressionsNoFile);
        TEST_CASE(suppressionSingle);
        TEST_CASE(suppressionSingleFile);
        TEST_CASE(suppressionTwo);
        TEST_CASE(suppressionTwoSeparate);
        TEST_CASE(templates);
        TEST_CASE(templatesGcc);
        TEST_CASE(templatesVs);
        TEST_CASE(templatesEdit);
        TEST_CASE(xml);
        TEST_CASE(xmlver2);
        TEST_CASE(xmlver2both);
        TEST_CASE(xmlver2both2);
        TEST_CASE(xmlverunknown);
        TEST_CASE(xmlverinvalid);
        TEST_CASE(doc);
        TEST_CASE(showtime);
        TEST_CASE(errorlist1);
        TEST_CASE(errorlistverbose1);
        TEST_CASE(errorlistverbose2);
        TEST_CASE(ignorepathsnopath);
        TEST_CASE(exceptionhandling);
        TEST_CASE(exceptionhandling2);
        TEST_CASE(exceptionhandling3);
        TEST_CASE(exceptionhandlingInvalid);
        TEST_CASE(exceptionhandlingInvalid2);
        TEST_CASE(clang);
        TEST_CASE(clang2);
        TEST_CASE(clangInvalid);
        TEST_CASE(valueFlowMaxIterations);
        TEST_CASE(valueFlowMaxIterations2);
        TEST_CASE(valueFlowMaxIterationsInvalid);
        TEST_CASE(valueFlowMaxIterationsInvalid2);
        TEST_CASE(valueFlowMaxIterationsInvalid3);

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
    }


    void nooptions() {
        REDIRECT;
        const char * const argv[] = {"cppcheck"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(1, argv));
        ASSERT_EQUALS(true, parser.getShowHelp());
        ASSERT(GET_REDIRECT_OUTPUT.find("Cppcheck - A tool for static C/C++ code analysis") == 0);
    }

    void helpshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-h"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.getShowHelp());
        ASSERT(GET_REDIRECT_OUTPUT.find("Cppcheck - A tool for static C/C++ code analysis") == 0);
    }

    void helplong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--help"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.getShowHelp());
        ASSERT(GET_REDIRECT_OUTPUT.find("Cppcheck - A tool for static C/C++ code analysis") == 0);
    }

    void showversion() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--version"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.getShowVersion());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT); // version is not actually shown
    }

    void onefile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser.getPathNames().size());
        ASSERT_EQUALS("file.cpp", parser.getPathNames().at(0));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void onepath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "src"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser.getPathNames().size());
        ASSERT_EQUALS("src", parser.getPathNames().at(0));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void optionwithoutfile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v"};
        CmdLineParser parser(&settings);
        ASSERT_EQUALS(false, parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(0, (int)parser.getPathNames().size());
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", GET_REDIRECT_OUTPUT);
    }

    void verboseshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v", "file.cpp"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.verbose);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void verboselong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "file.cpp"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.verbose);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void debugSimplified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-simplified", "file.cpp"};
        settings.debugSimplified = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.debugSimplified);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void debugwarnings() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-warnings", "file.cpp"};
        settings.debugwarnings = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.debugwarnings);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void forceshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "file.cpp"};
        settings.force = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.force);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void forcelong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--force", "file.cpp"};
        settings.force = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.force);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void relativePaths() {
        REDIRECT;
        settings.relativePaths = false;

        const char * const argvs[] = {"cppcheck", "-rp", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvs));
        ASSERT_EQUALS(true, settings.relativePaths);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);

        CLEAR_REDIRECT_OUTPUT;
        settings.relativePaths = false;

        const char * const argvl[] = {"cppcheck", "--relative-paths", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvl));
        ASSERT_EQUALS(true, settings.relativePaths);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);

        CLEAR_REDIRECT_OUTPUT;
        settings.relativePaths = false;
        settings.basePaths.clear();

        const char * const argvsp[] = {"cppcheck", "-rp=C:/foo;C:\\bar", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvsp));
        ASSERT_EQUALS(true, settings.relativePaths);
        ASSERT_EQUALS(2, settings.basePaths.size());
        ASSERT_EQUALS("C:/foo", settings.basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings.basePaths[1]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);

        CLEAR_REDIRECT_OUTPUT;
        settings.relativePaths = false;
        settings.basePaths.clear();

        const char * const argvlp[] = {"cppcheck", "--relative-paths=C:/foo;C:\\bar", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvlp));
        ASSERT_EQUALS(true, settings.relativePaths);
        ASSERT_EQUALS(2, settings.basePaths.size());
        ASSERT_EQUALS("C:/foo", settings.basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings.basePaths[1]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void quietshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-q", "file.cpp"};
        settings.quiet = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.quiet);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void quietlong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--quiet", "file.cpp"};
        settings.quiet = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.quiet);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void defines_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", GET_REDIRECT_OUTPUT);
    }

    void defines_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "-v", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", GET_REDIRECT_OUTPUT);
    }

    void defines_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "--quiet", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", GET_REDIRECT_OUTPUT);
    }

    void defines() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "file.cpp"};
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("_WIN32=1", settings.userDefines);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void defines2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "-DNODEBUG", "file.cpp"};
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("_WIN32=1;NODEBUG=1", settings.userDefines);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void defines3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "DEBUG", "file.cpp"};
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("DEBUG=1", settings.userDefines);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void defines4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-DDEBUG=", "file.cpp"}; // #5137 - defining empty macro
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("DEBUG=", settings.userDefines);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void enforceLanguage() {
        REDIRECT;
        {
            const char * const argv[] = {"cppcheck", "file.cpp"};
            settings.enforcedLang = Settings::Language::None;
            ASSERT(defParser.parseFromArgs(2, argv));
            ASSERT_EQUALS(Settings::Language::None, settings.enforcedLang);
            ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
        }
        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "-x", "c++", "file.cpp"};
            settings.enforcedLang = Settings::Language::None;
            ASSERT(defParser.parseFromArgs(4, argv));
            ASSERT_EQUALS(Settings::Language::CPP, settings.enforcedLang);
            ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
        }
        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "-x"};
            ASSERT(!defParser.parseFromArgs(2, argv));
            ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", GET_REDIRECT_OUTPUT);
        }
        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "-x", "--inconclusive", "file.cpp"};
            ASSERT(!defParser.parseFromArgs(4, argv));
            ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", GET_REDIRECT_OUTPUT);
        }
        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "--language=c++", "file.cpp"};
            settings.enforcedLang = Settings::Language::None;
            ASSERT(defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(Settings::Language::CPP, settings.enforcedLang);
            ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
        }
        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "--language=c", "file.cpp"};
            settings.enforcedLang = Settings::Language::None;
            ASSERT(defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(Settings::Language::C, settings.enforcedLang);
            ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
        }
        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "--language=unknownLanguage", "file.cpp"};
            ASSERT(!defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS("cppcheck: error: unknown language 'unknownLanguage' enforced.\n", GET_REDIRECT_OUTPUT);
        }
    }

    void includesnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I"};
        // Fails since -I has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-I' is missing.\n", GET_REDIRECT_OUTPUT);
    }

    void includes() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void includesslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void includesbackslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include\\", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void includesnospace() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-Iinclude", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void includes2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "-I", "framework/", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(6, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
        settings.includePaths.pop_front();
        ASSERT_EQUALS("framework/", settings.includePaths.front());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    // TODO: fails since the file is not found
    void includesFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--includes-file=fileThatDoesNotExist.txt", "file.cpp"};
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        TODO_ASSERT_EQUALS("", "cppcheck: error: unable to open includes file at 'fileThatDoesNotExist.txt'\n", GET_REDIRECT_OUTPUT);
    }

    void includesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--includes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to open includes file at 'fileThatDoesNotExist.txt'\n", GET_REDIRECT_OUTPUT);
    }

    // TODO: fails since the file is not found
    void configExcludesFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--config-excludes-file=fileThatDoesNotExist.txt", "file.cpp"};
        settings.includePaths.clear();
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        // TODO: add checks
        TODO_ASSERT_EQUALS("", "cppcheck: error: unable to open config excludes file at 'fileThatDoesNotExist.txt'\n", GET_REDIRECT_OUTPUT);
    }

    void configExcludesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--config-excludes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS( false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to open config excludes file at 'fileThatDoesNotExist.txt'\n", GET_REDIRECT_OUTPUT);
    }

    void enabledAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.severity.isEnabled(Severity::style));
        ASSERT(settings.severity.isEnabled(Severity::warning));
        ASSERT(settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings.checks.isEnabled(Checks::missingInclude));
        ASSERT(!settings.checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void enabledStyle() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.severity.isEnabled(Severity::style));
        ASSERT(settings.severity.isEnabled(Severity::warning));
        ASSERT(settings.severity.isEnabled(Severity::performance));
        ASSERT(settings.severity.isEnabled(Severity::portability));
        ASSERT(!settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings.checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void enabledPerformance() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=performance", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(!settings.severity.isEnabled(Severity::style));
        ASSERT(!settings.severity.isEnabled(Severity::warning));
        ASSERT(settings.severity.isEnabled(Severity::performance));
        ASSERT(!settings.severity.isEnabled(Severity::portability));
        ASSERT(!settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings.checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void enabledPortability() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=portability", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(!settings.severity.isEnabled(Severity::style));
        ASSERT(!settings.severity.isEnabled(Severity::warning));
        ASSERT(!settings.severity.isEnabled(Severity::performance));
        ASSERT(settings.severity.isEnabled(Severity::portability));
        ASSERT(!settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings.checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void enabledUnusedFunction() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=unusedFunction", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void enabledMissingInclude() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

#ifdef CHECK_INTERNAL
    void enabledInternal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=internal", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }
#endif

    void enabledMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude,portability,warning", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(!settings.severity.isEnabled(Severity::style));
        ASSERT(settings.severity.isEnabled(Severity::warning));
        ASSERT(!settings.severity.isEnabled(Severity::performance));
        ASSERT(settings.severity.isEnabled(Severity::portability));
        ASSERT(!settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings.checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void enabledInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=warning,missingIncludeSystem,style", "file.cpp"};
        settings = Settings();
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'missingIncludeSystem'\n", GET_REDIRECT_OUTPUT);
    }

    void enabledError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=error", "file.cpp"};
        settings = Settings();
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'error'\n", GET_REDIRECT_OUTPUT);
    }

    void enabledEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=", "file.cpp"};
        settings = Settings();
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter is empty\n", GET_REDIRECT_OUTPUT);
    }

    void disableAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=all"};
        settings.severity.clear();
        settings.checks.clear();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::error));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings.checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings.checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void disableMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=style", "--disable=unusedFunction"};
        settings.severity.clear();
        settings.checks.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::style));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, settings.checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings.checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    // make sure the implied "style" checks are not added when "--enable=style" is specified
    void disableStylePartial() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "--disable=performance", "--enable=unusedFunction"};
        settings.severity.clear();
        settings.checks.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings.severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings.severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(true, settings.checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings.checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings.checks.isEnabled(Checks::internalCheck));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void disableInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=leaks", "file.cpp"};
        settings = Settings();
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'leaks'\n", GET_REDIRECT_OUTPUT);
    }

    void disableError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=error", "file.cpp"};
        settings = Settings();
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'error'\n", GET_REDIRECT_OUTPUT);
    }

    void disableEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=", "file.cpp"};
        settings = Settings();
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter is empty\n", GET_REDIRECT_OUTPUT);
    }

    void inconclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inconclusive"};
        settings.certainty.clear();
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, settings.certainty.isEnabled(Certainty::inconclusive));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void errorExitcode() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=5", "file.cpp"};
        settings.exitCode = 0;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(5, settings.exitCode);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void errorExitcodeMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=", "file.cpp"};
        // Fails since exit code not given
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument must be an integer. Try something like '--error-exitcode=1'.\n", GET_REDIRECT_OUTPUT);
    }

    void errorExitcodeStr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=foo", "file.cpp"};
        // Fails since invalid exit code
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument must be an integer. Try something like '--error-exitcode=1'.\n", GET_REDIRECT_OUTPUT);
    }

    void exitcodeSuppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "suppr.txt", "file.cpp"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", GET_REDIRECT_OUTPUT);
    }

    void exitcodeSuppressions() {
        // TODO: Fails since cannot open the file
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions=suppr.txt", "file.cpp"};
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        TODO_ASSERT_EQUALS("", "cppcheck: error: couldn't open the file: \"suppr.txt\".\n", GET_REDIRECT_OUTPUT);
    }

    void exitcodeSuppressionsNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "file.cpp"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", GET_REDIRECT_OUTPUT);
    }

    // TODO: file does not exist
    void fileList() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        // TODO: settings are not being reset after each test
        //TODO_ASSERT_EQUALS(4, 1, defParser.getPathNames().size());
        TODO_ASSERT_EQUALS("", "cppcheck: error: couldn't open the file: \"files.txt\".\n", GET_REDIRECT_OUTPUT);
    }

    void fileListNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: couldn't open the file: \"files.txt\".\n", GET_REDIRECT_OUTPUT);
    }

    /*    void fileListStdin() {
            // TODO: Give it some stdin to read from, fails because the list of
            // files in stdin (_pathnames) is empty
            REDIRECT;
            const char * const argv[] = {"cppcheck", "--file-list=-", "file.cpp"};
            TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
            TODO_ASSERT_EQUALS("", "", GET_REDIRECT_OUTPUT);
        } */

    void fileListInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list", "files.txt", "file.cpp"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--file-list\".\n", GET_REDIRECT_OUTPUT);
    }

    void inlineSuppr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inline-suppr", "file.cpp"};
        settings.inlineSuppressions = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.inlineSuppressions);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void jobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "3", "file.cpp"};
        settings.jobs = 0;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(3, settings.jobs);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void jobsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "file.cpp"};
        // Fails since -j is missing thread count
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not a number.\n", GET_REDIRECT_OUTPUT);
    }

    void jobsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "e", "file.cpp"};
        // Fails since invalid count given for -j
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not a number.\n", GET_REDIRECT_OUTPUT);
    }

    void maxConfigs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "--max-configs=12", "file.cpp"};
        settings.force = false;
        settings.maxConfigs = 0;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(12, settings.maxConfigs);
        ASSERT_EQUALS(false, settings.force);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void maxConfigsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=", "file.cpp"};
        // Fails since --max-configs= is missing limit
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not a number.\n", GET_REDIRECT_OUTPUT);
    }

    void maxConfigsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=e", "file.cpp"};
        // Fails since invalid count given for --max-configs=
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not a number.\n", GET_REDIRECT_OUTPUT);
    }

    void maxConfigsTooSmall() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=0", "file.cpp"};
        // Fails since limit must be greater than 0
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' must be greater than 0.\n", GET_REDIRECT_OUTPUT);
    }

    void reportProgressTest() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress", "file.cpp"};
        settings.reportProgress = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.reportProgress);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void stdc99() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c99", "file.cpp"};
        settings.standards.c = Standards::C89;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.standards.c == Standards::C99);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void stdcpp11() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c++11", "file.cpp"};
        settings.standards.cpp = Standards::CPP03;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.standards.cpp == Standards::CPP11);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void stdunknown() {
        REDIRECT;
        {
            const char *const argv[] = {"cppcheck", "--std=d++11", "file.cpp"};
            ASSERT(!defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS("cppcheck: error: unknown --std value 'd++11'\n", GET_REDIRECT_OUTPUT);
        }
        CLEAR_REDIRECT_OUTPUT;
        {
            const char *const argv[] = {"cppcheck", "--std=cplusplus11", "file.cpp"};
            TODO_ASSERT(!defParser.parseFromArgs(3, argv));
            TODO_ASSERT_EQUALS("cppcheck: error: unknown --std value 'cplusplus11'\n", "", GET_REDIRECT_OUTPUT);
        }
    }

    void platformWin64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win64", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Win64, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformWin32A() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32A", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Win32A, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformWin32W() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32W", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Win32W, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformUnix32() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Unix32, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformUnix32Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32-unsigned", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.platformType == Settings::Unix32);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformUnix64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Unix64, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformUnix64Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64-unsigned", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.platformType == Settings::Unix64);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformNative() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=native", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Native, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformUnspecified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unspecified", "file.cpp"};
        ASSERT(settings.platform(Settings::Native));
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Unspecified, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformPlatformFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=avr8", "file.cpp"};
        ASSERT(settings.platform(Settings::Unspecified));
        ASSERT_EQUALS(true, defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::PlatformFile, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void platformUnknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win128", "file.cpp"};
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized platform: \"win128\".\n", GET_REDIRECT_OUTPUT);
    }

#if defined(_WIN64) || defined(_WIN32)
    void platformDefault() {
        REDIRECT;

        CmdLineParser::SHOW_DEF_PLATFORM_MSG = true;

        const char * const argv[] = {"cppcheck", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(2, argv));
#if defined(_WIN64)
        ASSERT_EQUALS(Settings::Win64, settings.platformType);
        ASSERT_EQUALS("cppcheck: Windows 64-bit binaries currently default to the 'win64' platform. Starting with Cppcheck 2.13 they will default to 'native' instead. Please specify '--platform=win64' explicitly if you rely on this.\n", GET_REDIRECT_OUTPUT);
#elif defined(_WIN32)
        ASSERT_EQUALS(Settings::Win32A, settings.platformType);
        ASSERT_EQUALS("cppcheck: Windows 32-bit binaries currently default to the 'win32A' platform. Starting with Cppcheck 2.13 they will default to 'native' instead. Please specify '--platform=win32A' explicitly if you rely on this.\n", GET_REDIRECT_OUTPUT);
#endif

        CmdLineParser::SHOW_DEF_PLATFORM_MSG = false;
    }

    void platformDefault2() {
        REDIRECT;

        CmdLineParser::SHOW_DEF_PLATFORM_MSG = true;

        const char * const argv[] = {"cppcheck", "--platform=unix64", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(Settings::Unix64, settings.platformType);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);

        CmdLineParser::SHOW_DEF_PLATFORM_MSG = false;
    }
#endif

    void plistEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=", "file.cpp"};
        settings.plistOutput = "";
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.plistOutput == "./");
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void plistDoesNotExist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=./cppcheck_reports", "file.cpp"};
        // Fails since folder pointed by --plist-output= does not exist
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
        // TODO: output contains non-native separator
        //ASSERT_EQUALS("cppcheck: error: plist folder does not exist: \"cppcheck_reports/\".\n", GET_REDIRECT_OUTPUT);
    }

    void suppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions", "suppr.txt", "file.cpp"};
        ASSERT(!defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--suppressions\".\n", GET_REDIRECT_OUTPUT);
    }

    void suppressions() {
        // TODO: Fails because there is no suppr.txt file!
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=suppr.txt", "file.cpp"};
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        TODO_ASSERT_EQUALS("", "cppcheck: error: couldn't open the file: \"suppr.txt\".\n", GET_REDIRECT_OUTPUT);
    }

    void suppressionsNoFile() {
        REDIRECT;
        {
            const char * const argv[] = {"cppcheck", "--suppressions-list=", "file.cpp"};
            ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(false, GET_REDIRECT_OUTPUT.find("If you want to pass two files") != std::string::npos);
        }

        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr,b.suppr", "file.cpp"};
            ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(true, GET_REDIRECT_OUTPUT.find("If you want to pass two files") != std::string::npos);
        }

        CLEAR_REDIRECT_OUTPUT;
        {
            const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr b.suppr", "file.cpp"};
            ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(true, GET_REDIRECT_OUTPUT.find("If you want to pass two files") != std::string::npos);
        }
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
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1)));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void suppressionSingleFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar:file.cpp", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void suppressionTwo() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar,noConstructor", "file.cpp"};
        settings = Settings();
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        TODO_ASSERT_EQUALS(true, false, settings.nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        TODO_ASSERT_EQUALS(true, false, settings.nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
        TODO_ASSERT_EQUALS("", "cppcheck: error: Failed to add suppression. Invalid id \"uninitvar,noConstructor\"\n", GET_REDIRECT_OUTPUT);
    }

    void suppressionTwoSeparate() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar", "--suppress=noConstructor", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings.nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        ASSERT_EQUALS(true, settings.nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void templates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "{file}:{line},{severity},{id},{message}", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{file}:{line},{severity},{id},{message}", settings.templateFormat);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void templatesGcc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "gcc", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{bold}{file}:{line}:{column}: {magenta}warning:{default} {message} [{id}]{reset}\\n{code}", settings.templateFormat);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void templatesVs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "vs", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{file}({line}): {severity}: {message}", settings.templateFormat);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void templatesEdit() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "edit", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{file} +{line}: {severity}: {message}", settings.templateFormat);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void xml() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(1, settings.xml_version);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void xmlver2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(2, settings.xml_version);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void xmlver2both() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=2", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(2, settings.xml_version);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void xmlver2both2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "--xml", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(2, settings.xml_version);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void xmlverunknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=3", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: '--xml-version' can only be 2.\n", GET_REDIRECT_OUTPUT);
    }

    void xmlverinvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=a", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--xml-version' is not a number.\n", GET_REDIRECT_OUTPUT);
    }

    void doc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--doc"};
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(defParser.exitAfterPrinting());
        ASSERT(GET_REDIRECT_OUTPUT.find("## ") == 0);
    }

    void showtime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=summary"};
        settings.showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(settings.showtime == SHOWTIME_MODES::SHOWTIME_SUMMARY);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void errorlist1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(defParser.getShowErrorMessages());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void errorlistverbose1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "--errorlist"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.verbose);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void errorlistverbose2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist", "--verbose"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.verbose);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void ignorepathsnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i"};
        CmdLineParser parser(&settings);
        // Fails since no ignored path given
        ASSERT_EQUALS(false, parser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-i' is missing.\n", GET_REDIRECT_OUTPUT);
    }

    void exceptionhandling() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling"};
        settings.exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stderr);
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(settings.exceptionHandling);
        ASSERT_EQUALS(stderr, CppCheckExecutor::getExceptionOutput());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void exceptionhandling2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stderr"};
        settings.exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stdout);
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(settings.exceptionHandling);
        ASSERT_EQUALS(stderr, CppCheckExecutor::getExceptionOutput());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void exceptionhandling3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stdout"};
        settings.exceptionHandling = false;
        CppCheckExecutor::setExceptionOutput(stderr);
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(settings.exceptionHandling);
        ASSERT_EQUALS(stdout, CppCheckExecutor::getExceptionOutput());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void exceptionhandlingInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=exfile"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: invalid '--exception-handling' argument\n", GET_REDIRECT_OUTPUT);
    }

    void exceptionhandlingInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling-foo"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exception-handling-foo\".\n", GET_REDIRECT_OUTPUT);
    }

    void clang() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang"};
        settings.clang = false;
        settings.clangExecutable = "exe";
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(settings.clang);
        ASSERT_EQUALS("exe", settings.clangExecutable);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void clang2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang=clang-14"};
        settings.clang = false;
        settings.clangExecutable = "";
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(settings.clang);
        ASSERT_EQUALS("clang-14", settings.clangExecutable);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void clangInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang-foo"};
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--clang-foo\".\n", GET_REDIRECT_OUTPUT);
    }

    void valueFlowMaxIterations() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=0"};
        settings.valueFlowMaxIterations = SIZE_MAX;
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS(0, settings.valueFlowMaxIterations);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void valueFlowMaxIterations2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=11"};
        settings.valueFlowMaxIterations = SIZE_MAX;
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS(11, settings.valueFlowMaxIterations);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void valueFlowMaxIterationsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations"};
        ASSERT(!defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--valueflow-max-iterations\".\n", GET_REDIRECT_OUTPUT);
    }

    void valueFlowMaxIterationsInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=seven"};
        ASSERT(!defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iteration' is invalid.\n", GET_REDIRECT_OUTPUT);
    }

    void valueFlowMaxIterationsInvalid3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=-1"};
        ASSERT(!defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iteration' needs to be at least 0.\n", GET_REDIRECT_OUTPUT);
    }
    void ignorepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser.getIgnoredPaths()[0]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void ignorepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(4, argv));
        ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser.getIgnoredPaths()[0]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void ignorepaths3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "-imodule", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(4, argv));
        ASSERT_EQUALS(2, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser.getIgnoredPaths()[0]);
        ASSERT_EQUALS("module", parser.getIgnoredPaths()[1]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void ignorepaths4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "-i", "module", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(6, argv));
        ASSERT_EQUALS(2, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("src", parser.getIgnoredPaths()[0]);
        ASSERT_EQUALS("module", parser.getIgnoredPaths()[1]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void ignorefilepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-ifoo.cpp", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("foo.cpp", parser.getIgnoredPaths()[0]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void ignorefilepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc/foo.cpp", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("src/foo.cpp", parser.getIgnoredPaths()[0]);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void checkconfig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-config", "file.cpp"};
        settings.checkConfiguration = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.checkConfiguration);
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void unknownParam() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--foo", "file.cpp"};
        ASSERT(!defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--foo\".\n", GET_REDIRECT_OUTPUT);
    }

    void undefs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings.userUndefs.size());
        ASSERT(settings.userUndefs.find("_WIN32") != settings.userUndefs.end());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void undefs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "-UNODEBUG", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(2, settings.userUndefs.size());
        ASSERT(settings.userUndefs.find("_WIN32") != settings.userUndefs.end());
        ASSERT(settings.userUndefs.find("NODEBUG") != settings.userUndefs.end());
        ASSERT_EQUALS("", GET_REDIRECT_OUTPUT);
    }

    void undefs_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", GET_REDIRECT_OUTPUT);
    }

    void undefs_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "-v", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", GET_REDIRECT_OUTPUT);
    }

    void undefs_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "--quiet", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", GET_REDIRECT_OUTPUT);
    }
};

REGISTER_TEST(TestCmdlineParser)

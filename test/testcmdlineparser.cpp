/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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
#include "errortypes.h"
#include "platform.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "testsuite.h"
#include "timer.h"

#include <list>
#include <set>
#include <string>
#include <vector>

class TestCmdlineParser : public TestFixture {
public:
    TestCmdlineParser()
        : TestFixture("TestCmdlineParser")
        , defParser(&settings) {}

private:
    Settings settings;
    CmdLineParser defParser;

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
        TEST_CASE(configExcludesFile);
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
        TEST_CASE(inconclusive);
        TEST_CASE(errorExitcode);
        TEST_CASE(errorExitcodeMissing);
        TEST_CASE(errorExitcodeStr);
        TEST_CASE(exitcodeSuppressionsOld); // TODO: Create and test real suppression file
        TEST_CASE(exitcodeSuppressions);
        TEST_CASE(exitcodeSuppressionsNoFile);
        TEST_CASE(fileList); // TODO: Create and test real file listing file
        // TEST_CASE(fileListStdin);  // Disabled since hangs the test run
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
        TEST_CASE(platform);
        TEST_CASE(plistEmpty);
        TEST_CASE(plistDoesNotExist);
        TEST_CASE(suppressionsOld); // TODO: Create and test real suppression file
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

        // TODO
        // Disabling these tests since they use relative paths to the
        // testrunner executable.
        //TEST_CASE(ignorepaths1);
        //TEST_CASE(ignorepaths2);
        //TEST_CASE(ignorepaths3);
        //TEST_CASE(ignorepaths4);
        //TEST_CASE(ignorefilepaths1);
        //TEST_CASE(ignorefilepaths2);

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
    }

    void helpshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-h"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.getShowHelp());
    }

    void helplong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--help"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.getShowHelp());
    }

    void showversion() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--version"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, parser.getShowVersion());
    }

    void onefile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser.getPathNames().size());
        ASSERT_EQUALS("file.cpp", parser.getPathNames().at(0));
    }

    void onepath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "src"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(1, (int)parser.getPathNames().size());
        ASSERT_EQUALS("src", parser.getPathNames().at(0));
    }

    void optionwithoutfile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v"};
        CmdLineParser parser(&settings);
        ASSERT_EQUALS(false, parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(0, (int)parser.getPathNames().size());
    }

    void verboseshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v", "file.cpp"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.verbose);
    }

    void verboselong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "file.cpp"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.verbose);
    }

    void debugSimplified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-simplified", "file.cpp"};
        settings.debugSimplified = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.debugSimplified);
    }

    void debugwarnings() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-warnings", "file.cpp"};
        settings.debugwarnings = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.debugwarnings);
    }

    void forceshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "file.cpp"};
        settings.force = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.force);
    }

    void forcelong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--force", "file.cpp"};
        settings.force = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.force);
    }

    void relativePaths() {
        REDIRECT;
        settings.relativePaths = false;

        const char * const argvs[] = {"cppcheck", "-rp", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvs));
        ASSERT_EQUALS(true, settings.relativePaths);

        settings.relativePaths = false;

        const char * const argvl[] = {"cppcheck", "--relative-paths", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvl));
        ASSERT_EQUALS(true, settings.relativePaths);

        settings.relativePaths = false;
        settings.basePaths.clear();

        const char * const argvsp[] = {"cppcheck", "-rp=C:/foo;C:\\bar", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvsp));
        ASSERT_EQUALS(true, settings.relativePaths);
        ASSERT_EQUALS(2, settings.basePaths.size());
        ASSERT_EQUALS("C:/foo", settings.basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings.basePaths[1]);

        settings.relativePaths = false;
        settings.basePaths.clear();

        const char * const argvlp[] = {"cppcheck", "--relative-paths=C:/foo;C:\\bar", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argvlp));
        ASSERT_EQUALS(true, settings.relativePaths);
        ASSERT_EQUALS(2, settings.basePaths.size());
        ASSERT_EQUALS("C:/foo", settings.basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings.basePaths[1]);
    }

    void quietshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-q", "file.cpp"};
        settings.quiet = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.quiet);
    }

    void quietlong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--quiet", "file.cpp"};
        settings.quiet = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.quiet);
    }

    void defines_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
    }

    void defines_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "-v", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
    }

    void defines_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "--quiet", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
    }

    void defines() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "file.cpp"};
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("_WIN32=1", settings.userDefines);
    }

    void defines2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "-DNODEBUG", "file.cpp"};
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("_WIN32=1;NODEBUG=1", settings.userDefines);
    }

    void defines3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "DEBUG", "file.cpp"};
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("DEBUG=1", settings.userDefines);
    }

    void defines4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-DDEBUG=", "file.cpp"}; // #5137 - defining empty macro
        settings.userDefines.clear();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("DEBUG=", settings.userDefines);
    }

    void enforceLanguage() {
        REDIRECT;
        {
            const char * const argv[] = {"cppcheck", "file.cpp"};
            settings.enforcedLang = Settings::None;
            ASSERT(defParser.parseFromArgs(2, argv));
            ASSERT_EQUALS(Settings::None, settings.enforcedLang);
        }
        {
            const char * const argv[] = {"cppcheck", "-x", "c++", "file.cpp"};
            settings.enforcedLang = Settings::None;
            ASSERT(defParser.parseFromArgs(4, argv));
            ASSERT_EQUALS(Settings::CPP, settings.enforcedLang);
        }
        {
            const char * const argv[] = {"cppcheck", "-x"};
            ASSERT(!defParser.parseFromArgs(2, argv));
        }
        {
            const char * const argv[] = {"cppcheck", "-x", "--inconclusive", "file.cpp"};
            ASSERT(!defParser.parseFromArgs(4, argv));
        }
        {
            const char * const argv[] = {"cppcheck", "--language=c++", "file.cpp"};
            settings.enforcedLang = Settings::None;
            ASSERT(defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(Settings::CPP, settings.enforcedLang);
        }
        {
            const char * const argv[] = {"cppcheck", "--language=c", "file.cpp"};
            settings.enforcedLang = Settings::None;
            ASSERT(defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(Settings::C, settings.enforcedLang);
        }
        {
            const char * const argv[] = {"cppcheck", "--language=unknownLanguage", "file.cpp"};
            ASSERT(!defParser.parseFromArgs(3, argv));
        }
    }

    void includesnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I"};
        // Fails since -I has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
    }

    void includes() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
    }

    void includesslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
    }

    void includesbackslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include\\", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
    }

    void includesnospace() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-Iinclude", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
    }

    void includes2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "-I", "framework/", "file.cpp"};
        settings.includePaths.clear();
        ASSERT(defParser.parseFromArgs(6, argv));
        ASSERT_EQUALS("include/", settings.includePaths.front());
        settings.includePaths.pop_front();
        ASSERT_EQUALS("framework/", settings.includePaths.front());
    }

    void includesFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--includes-file=fileThatDoesNotExist.txt", "file.cpp"};
        settings.includePaths.clear();
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void configExcludesFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--config-excludes-file=fileThatDoesNotExist.txt", "file.cpp"};
        settings.includePaths.clear();
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
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
    }

    void enabledUnusedFunction() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=unusedFunction", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.checks.isEnabled(Checks::unusedFunction));
    }

    void enabledMissingInclude() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.checks.isEnabled(Checks::missingInclude));
    }

#ifdef CHECK_INTERNAL
    void enabledInternal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=internal", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.checks.isEnabled(Checks::internalCheck));
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
    }

    void inconclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inconclusive"};
        settings.certainty.clear();
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT_EQUALS(true, settings.certainty.isEnabled(Certainty::inconclusive));
    }

    void errorExitcode() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=5", "file.cpp"};
        settings.exitCode = 0;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(5, settings.exitCode);
    }

    void errorExitcodeMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=", "file.cpp"};
        settings.exitCode = 0;
        // Fails since exit code not given
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void errorExitcodeStr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=foo", "file.cpp"};
        settings.exitCode = 0;
        // Fails since invalid exit code
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void exitcodeSuppressionsOld() {
        // TODO: Fails since cannot open the file
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "suppr.txt", "file.cpp"};
        settings.exitCode = 0;
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(4, argv));
    }

    void exitcodeSuppressions() {
        // TODO: Fails since cannot open the file
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions=suppr.txt", "file.cpp"};
        settings.exitCode = 0;
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
    }

    void exitcodeSuppressionsNoFile() {
        // TODO: Fails since cannot open the file
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "file.cpp"};
        settings.exitCode = 0;
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
    }

    void fileList() {
        // TODO: Fails since cannot open the file
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list", "files.txt", "file.cpp"};
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(4, argv));
    }

    /*    void fileListStdin() {
            // TODO: Give it some stdin to read from, fails because the list of
            // files in stdin (_pathnames) is empty
            REDIRECT;
            const char * const argv[] = {"cppcheck", "--file-list=-", "file.cpp"};
            TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        } */

    void inlineSuppr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inline-suppr", "file.cpp"};
        ASSERT(defParser.parseFromArgs(3, argv));
    }

    void jobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "3", "file.cpp"};
        settings.jobs = 0;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(3, settings.jobs);
    }

    void jobsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "file.cpp"};
        settings.jobs = 0;
        // Fails since -j is missing thread count
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void jobsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "e", "file.cpp"};
        settings.jobs = 0;
        // Fails since invalid count given for -j
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
    }

    void maxConfigs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "--max-configs=12", "file.cpp"};
        settings.force = false;
        settings.maxConfigs = 12;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(12, settings.maxConfigs);
        ASSERT_EQUALS(false, settings.force);
    }

    void maxConfigsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=", "file.cpp"};
        // Fails since --max-configs= is missing limit
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void maxConfigsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=e", "file.cpp"};
        // Fails since invalid count given for --max-configs=
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void maxConfigsTooSmall() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=0", "file.cpp"};
        // Fails since limit must be greater than 0
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void reportProgressTest() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress", "file.cpp"};
        settings.reportProgress = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.reportProgress);
    }

    void stdc99() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c99", "file.cpp"};
        settings.standards.c = Standards::C89;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.standards.c == Standards::C99);
    }

    void stdcpp11() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c++11", "file.cpp"};
        settings.standards.cpp = Standards::CPP03;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.standards.cpp == Standards::CPP11);
    }

    void stdunknown() {
        REDIRECT;
        {
            CLEAR_REDIRECT_OUTPUT;
            const char *const argv[] = {"cppcheck", "--std=d++11", "file.cpp"};
            ASSERT(!defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS("cppcheck: error: unknown --std value 'd++11'\n", GET_REDIRECT_OUTPUT);
        }
        {
            CLEAR_REDIRECT_OUTPUT;
            const char *const argv[] = {"cppcheck", "--std=cplusplus11", "file.cpp"};
            TODO_ASSERT(!defParser.parseFromArgs(3, argv));
            TODO_ASSERT_EQUALS("cppcheck: error: unknown --std value 'cplusplus11'\n", "", GET_REDIRECT_OUTPUT);
        }
    }

    void platform() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win64", "file.cpp"};
        settings.platform(Settings::Unspecified);
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.platformType == Settings::Win64);
    }

    void plistEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=", "file.cpp"};
        settings.plistOutput = "";
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.plistOutput == "./");
    }

    void plistDoesNotExist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=./cppcheck_reports", "file.cpp"};
        settings.plistOutput = "";
        // Fails since folder pointed by --plist-output= does not exist
        ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
    }

    void suppressionsOld() {
        // TODO: Fails because there is no suppr.txt file!
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions", "suppr.txt", "file.cpp"};
        ASSERT(!defParser.parseFromArgs(4, argv));
    }

    void suppressions() {
        // TODO: Fails because there is no suppr.txt file!
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=suppr.txt", "file.cpp"};
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
    }

    void suppressionsNoFile() {
        REDIRECT;
        {
            CLEAR_REDIRECT_OUTPUT;
            const char * const argv[] = {"cppcheck", "--suppressions-list=", "file.cpp"};
            ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(false, GET_REDIRECT_OUTPUT.find("If you want to pass two files") != std::string::npos);
        }

        {
            CLEAR_REDIRECT_OUTPUT;
            const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr,b.suppr", "file.cpp"};
            ASSERT_EQUALS(false, defParser.parseFromArgs(3, argv));
            ASSERT_EQUALS(true, GET_REDIRECT_OUTPUT.find("If you want to pass two files") != std::string::npos);
        }

        {
            CLEAR_REDIRECT_OUTPUT;
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
    }

    void suppressionSingleFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar:file.cpp", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
    }

    void suppressionTwo() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar,noConstructor", "file.cpp"};
        settings = Settings();
        TODO_ASSERT_EQUALS(true, false, defParser.parseFromArgs(3, argv));
        TODO_ASSERT_EQUALS(true, false, settings.nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        TODO_ASSERT_EQUALS(true, false, settings.nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
    }

    void suppressionTwoSeparate() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar", "--suppress=noConstructor", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(true, settings.nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        ASSERT_EQUALS(true, settings.nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
    }

    void templates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "{file}:{line},{severity},{id},{message}", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{file}:{line},{severity},{id},{message}", settings.templateFormat);
    }

    void templatesGcc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "gcc", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{bold}{file}:{line}:{column}: {magenta}warning:{default} {message} [{id}]{reset}\\n{code}", settings.templateFormat);
    }

    void templatesVs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "vs", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{file}({line}): {severity}: {message}", settings.templateFormat);
    }

    void templatesEdit() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template", "edit", "file.cpp"};
        settings.templateFormat.clear();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS("{file} +{line}: {severity}: {message}", settings.templateFormat);
    }

    void xml() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(1, settings.xml_version);
    }

    void xmlver2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(2, settings.xml_version);
    }

    void xmlver2both() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=2", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(2, settings.xml_version);
    }

    void xmlver2both2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "--xml", "file.cpp"};
        settings.xml_version = 1;
        settings.xml = false;
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT(settings.xml);
        ASSERT_EQUALS(2, settings.xml_version);
    }

    void xmlverunknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=3", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
    }

    void xmlverinvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=a", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
    }

    void doc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--doc"};
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(defParser.exitAfterPrinting());
    }

    void showtime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=summary"};
        settings.showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(settings.showtime == SHOWTIME_MODES::SHOWTIME_SUMMARY);
    }

    void errorlist1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT(defParser.parseFromArgs(2, argv));
        ASSERT(defParser.getShowErrorMessages());
    }

    void errorlistverbose1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "--errorlist"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.verbose);
    }

    void errorlistverbose2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist", "--verbose"};
        settings.verbose = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT(settings.verbose);
    }

    void ignorepathsnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i"};
        CmdLineParser parser(&settings);
        // Fails since no ignored path given
        ASSERT_EQUALS(false, parser.parseFromArgs(2, argv));
        ASSERT_EQUALS(0, parser.getIgnoredPaths().size());
    }

    /*
        void ignorepaths1() {
            REDIRECT;
            const char * const argv[] = {"cppcheck", "-isrc", "file.cpp"};
            CmdLineParser parser(&settings);
            ASSERT(parser.parseFromArgs(3, argv));
            ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
            ASSERT_EQUALS("src/", parser.getIgnoredPaths()[0]);
        }

        void ignorepaths2() {
            REDIRECT;
            const char * const argv[] = {"cppcheck", "-i", "src", "file.cpp"};
            CmdLineParser parser(&settings);
            ASSERT(parser.parseFromArgs(4, argv));
            ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
            ASSERT_EQUALS("src/", parser.getIgnoredPaths()[0]);
        }

        void ignorepaths3() {
            REDIRECT;
            const char * const argv[] = {"cppcheck", "-isrc", "-imodule", "file.cpp"};
            CmdLineParser parser(&settings);
            ASSERT(parser.parseFromArgs(4, argv));
            ASSERT_EQUALS(2, parser.getIgnoredPaths().size());
            ASSERT_EQUALS("src/", parser.getIgnoredPaths()[0]);
            ASSERT_EQUALS("module/", parser.getIgnoredPaths()[1]);
        }

       void ignorepaths4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "-i", "module", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(6, argv));
        ASSERT_EQUALS(2, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("src/", parser.getIgnoredPaths()[0]);
        ASSERT_EQUALS("module/", parser.getIgnoredPaths()[1]);
       }

        void ignorefilepaths1() {
            REDIRECT;
            const char * const argv[] = {"cppcheck", "-ifoo.cpp", "file.cpp"};
            CmdLineParser parser(&settings);
            ASSERT(parser.parseFromArgs(3, argv));
            ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
            ASSERT_EQUALS("foo.cpp", parser.getIgnoredPaths()[0]);
        }

       void ignorefilepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc/foo.cpp", "file.cpp"};
        CmdLineParser parser(&settings);
        ASSERT(parser.parseFromArgs(3, argv));
        ASSERT_EQUALS(1, parser.getIgnoredPaths().size());
        ASSERT_EQUALS("src/foo.cpp", parser.getIgnoredPaths()[0]);
       }
     */

    void checkconfig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-config", "file.cpp"};
        settings.checkConfiguration = false;
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(true, settings.checkConfiguration);
    }

    void unknownParam() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--foo", "file.cpp"};
        ASSERT(!defParser.parseFromArgs(3, argv));
    }

    void undefs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(3, argv));
        ASSERT_EQUALS(1, settings.userUndefs.size());
        ASSERT(settings.userUndefs.find("_WIN32") != settings.userUndefs.end());
    }

    void undefs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "-UNODEBUG", "file.cpp"};
        settings = Settings();
        ASSERT(defParser.parseFromArgs(4, argv));
        ASSERT_EQUALS(2, settings.userUndefs.size());
        ASSERT(settings.userUndefs.find("_WIN32") != settings.userUndefs.end());
        ASSERT(settings.userUndefs.find("NODEBUG") != settings.userUndefs.end());
    }

    void undefs_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(2, argv));
    }

    void undefs_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "-v", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
    }

    void undefs_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "--quiet", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS(false, defParser.parseFromArgs(4, argv));
    }
};

REGISTER_TEST(TestCmdlineParser)

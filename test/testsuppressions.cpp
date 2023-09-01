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

#include "config.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errortypes.h"
#include "processexecutor.h"
#include "settings.h"
#include "suppressions.h"
#include "fixture.h"
#include "helpers.h"
#include "threadexecutor.h"
#include "singleexecutor.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <sstream> // IWYU pragma: keep
#include <string>
#include <utility>
#include <vector>

class TestSuppressions : public TestFixture {
public:
    TestSuppressions() : TestFixture("TestSuppressions") {}

private:

    void run() override {
        TEST_CASE(suppressionsBadId1);
        TEST_CASE(suppressionsDosFormat);     // Ticket #1836
        TEST_CASE(suppressionsFileNameWithColon);    // Ticket #1919 - filename includes colon
        TEST_CASE(suppressionsGlob);
        TEST_CASE(suppressionsFileNameWithExtraPath);
        TEST_CASE(suppressionsSettings);
        TEST_CASE(suppressionsSettingsThreads);
#if !defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
        TEST_CASE(suppressionsSettingsProcesses);
#endif
        TEST_CASE(suppressionsMultiFile);
        TEST_CASE(suppressionsPathSeparator);
        TEST_CASE(suppressionsLine0);
        TEST_CASE(suppressionsFileComment);

        TEST_CASE(inlinesuppress);
        TEST_CASE(inlinesuppress_symbolname);
        TEST_CASE(inlinesuppress_comment);

        TEST_CASE(multi_inlinesuppress);
        TEST_CASE(multi_inlinesuppress_comment);

        TEST_CASE(globalSuppressions); // Testing that global suppressions work (#8515)

        TEST_CASE(inlinesuppress_unusedFunction); // #4210 - unusedFunction
        TEST_CASE(globalsuppress_unusedFunction); // #4946
        TEST_CASE(suppressionWithRelativePaths); // #4733
        TEST_CASE(suppressingSyntaxErrors); // #7076
        TEST_CASE(suppressingSyntaxErrorsInline); // #5917
        TEST_CASE(suppressingSyntaxErrorsWhileFileRead); // PR #1333
        TEST_CASE(symbol);

        TEST_CASE(unusedFunction);

        TEST_CASE(suppressingSyntaxErrorAndExitCode);
        TEST_CASE(suppressLocal);

        TEST_CASE(suppressUnmatchedSuppressions);
    }

    void suppressionsBadId1() const {
        Suppressions suppressions;
        std::istringstream s1("123");
        ASSERT_EQUALS("Failed to add suppression. Invalid id \"123\"", suppressions.parseFile(s1));

        std::istringstream s2("obsoleteFunctionsrand_r");
        ASSERT_EQUALS("", suppressions.parseFile(s2));
    }

    static Suppressions::ErrorMessage errorMessage(const std::string &errorId) {
        Suppressions::ErrorMessage ret;
        ret.errorId = errorId;
        ret.hash = 0;
        ret.lineNumber = 0;
        ret.certainty = Certainty::normal;
        return ret;
    }

    static Suppressions::ErrorMessage errorMessage(const std::string &errorId, const std::string &file, int line) {
        Suppressions::ErrorMessage ret;
        ret.errorId = errorId;
        ret.setFileName(file);
        ret.lineNumber = line;
        return ret;
    }

    void suppressionsDosFormat() const {
        Suppressions suppressions;
        std::istringstream s("abc\r\n"
                             "def\r\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("abc")));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("def")));
    }

    void suppressionsFileNameWithColon() const {
        Suppressions suppressions;
        std::istringstream s("errorid:c:\\foo.cpp\n"
                             "errorid:c:\\bar.cpp:12");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "c:/foo.cpp", 1111)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "c:/bar.cpp", 10)));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "c:/bar.cpp", 12)));
    }

    void suppressionsGlob() const {
        // Check for syntax errors in glob
        {
            Suppressions suppressions;
            std::istringstream s("errorid:**.cpp\n");
            ASSERT_EQUALS("Failed to add suppression. Invalid glob pattern '**.cpp'.", suppressions.parseFile(s));
        }

        // Check that globbing works
        {
            Suppressions suppressions;
            std::istringstream s("errorid:x*.cpp\n"
                                 "errorid:y?.cpp\n"
                                 "errorid:test.c*");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp.cpp", 1)));
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "abc.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "ya.cpp", 1)));
            ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "y.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.c", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.cpp", 1)));
        }

        // Check that both a filename match and a glob match apply
        {
            Suppressions suppressions;
            std::istringstream s("errorid:x*.cpp\n"
                                 "errorid:xyz.cpp:1\n"
                                 "errorid:a*.cpp:1\n"
                                 "errorid:abc.cpp:2");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "xyz.cpp", 2)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "abc.cpp", 1)));
            ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "abc.cpp", 2)));
        }
    }

    void suppressionsFileNameWithExtraPath() const {
        // Ticket #2797
        Suppressions suppressions;
        suppressions.addSuppressionLine("errorid:./a.c:123");
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "a.c", 123)));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "x/../a.c", 123)));
    }

    // Check the suppression
    unsigned int checkSuppression(const char code[], const std::string &suppression = emptyString) {
        std::map<std::string, std::string> files;
        files["test.cpp"] = code;

        return checkSuppression(files, suppression);
    }

    // Check the suppression for multiple files
    unsigned int checkSuppression(std::map<std::string, std::string> &f, const std::string &suppression = emptyString) {
        // Clear the error log
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> files;
        for (std::map<std::string, std::string>::const_iterator i = f.cbegin(); i != f.cend(); ++i) {
            files[i->first] = i->second.size();
        }

        CppCheck cppCheck(*this, true, nullptr);
        Settings& settings = cppCheck.settings();
        settings.jobs = 1;
        settings.inlineSuppressions = true;
        settings.severity.enable(Severity::information);
        if (suppression == "unusedFunction")
            settings.checks.setEnabled(Checks::unusedFunction, true);
        if (!suppression.empty()) {
            EXPECT_EQ("", settings.nomsg.addSuppressionLine(suppression));
        }
        SingleExecutor executor(cppCheck, files, settings, settings.nomsg, *this);
        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(files.size());
        for (std::map<std::string, std::string>::const_iterator i = f.cbegin(); i != f.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, i->second));

        const unsigned int exitCode = executor.check();

        CppCheckExecutor::reportSuppressions(settings, false, files, *this);

        return exitCode;
    }

    unsigned int checkSuppressionThreads(const char code[], const std::string &suppression = emptyString) {
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> files;
        files["test.cpp"] = strlen(code);

        Settings settings;
        settings.jobs = 2;
        settings.inlineSuppressions = true;
        settings.severity.enable(Severity::information);
        if (!suppression.empty()) {
            EXPECT_EQ("", settings.nomsg.addSuppressionLine(suppression));
        }
        ThreadExecutor executor(files, settings, settings.nomsg, *this);
        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(files.size());
        for (std::map<std::string, std::size_t>::const_iterator i = files.cbegin(); i != files.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, code));

        const unsigned int exitCode = executor.check();

        CppCheckExecutor::reportSuppressions(settings, false, files, *this);

        return exitCode;
    }

#if !defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
    unsigned int checkSuppressionProcesses(const char code[], const std::string &suppression = emptyString) {
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> files;
        files["test.cpp"] = strlen(code);

        Settings settings;
        settings.jobs = 2;
        settings.inlineSuppressions = true;
        settings.severity.enable(Severity::information);
        if (!suppression.empty()) {
            EXPECT_EQ("", settings.nomsg.addSuppressionLine(suppression));
        }
        ProcessExecutor executor(files, settings, settings.nomsg, *this);
        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(files.size());
        for (std::map<std::string, std::size_t>::const_iterator i = files.cbegin(); i != files.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, code));

        const unsigned int exitCode = executor.check();

        CppCheckExecutor::reportSuppressions(settings, false, files, *this);

        return exitCode;
    }
#endif

    void runChecks(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        // check to make sure the appropriate error is present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        // suppress uninitvar globally
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar"));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar globally, without error present
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    b++;\n"
                                        "}\n",
                                        "uninitvar"));
        ASSERT_EQUALS("(information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress uninitvar for this file only
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp"));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress all for this file only
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "*:test.cpp"));
        ASSERT_EQUALS("", errout.str());

        // suppress all for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "*:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: *\n", errout.str());

        // suppress uninitvar for this file and line
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp:3"));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar for this file and line, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp:3");
        ASSERT_EQUALS("[test.cpp:3]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;// cppcheck-suppress uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar */\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar */\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;/* cppcheck-suppress uninitvar */\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress[uninitvar]\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress[uninitvar]\n"
                                        "    a++;\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;// cppcheck-suppress[uninitvar]\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress[uninitvar]*/\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress[uninitvar]*/\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;/* cppcheck-suppress[uninitvar]*/\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline, with asm before (#6813)
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    __asm {\n"
                                        "        foo\n"
                                        "    }"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "}",
                                        ""));
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    b++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:4]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // #5746 - exitcode
        ASSERT_EQUALS(1U,
                      (this->*check)("int f() {\n"
                                     "  int a; return a;\n"
                                     "}\n",
                                     ""));
        ASSERT_EQUALS(0U,
                      (this->*check)("int f() {\n"
                                     "  int a; return a;\n"
                                     "}\n",
                                     "uninitvar"));
    }

    void suppressionsSettings() {
        runChecks(&TestSuppressions::checkSuppression);
    }

    void suppressionsSettingsThreads() {
        runChecks(&TestSuppressions::checkSuppressionThreads);
    }

#if !defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
    void suppressionsSettingsProcesses() {
        runChecks(&TestSuppressions::checkSuppressionProcesses);
    }
#endif

    void suppressionsMultiFile() {
        std::map<std::string, std::string> files;
        files["abc.cpp"] = "void f() {\n"
                           "}\n";
        files["xyz.cpp"] = "void f() {\n"
                           "    int a;\n"
                           "    a++;\n"
                           "}\n";

        // suppress uninitvar for this file and line
        ASSERT_EQUALS(0, checkSuppression(files, "uninitvar:xyz.cpp:3"));
        ASSERT_EQUALS("", errout.str());
    }

    void suppressionsPathSeparator() const {
        const Suppressions::Suppression s1("*", "test/foo/*");
        ASSERT_EQUALS(true, s1.isSuppressed(errorMessage("someid", "test/foo/bar.cpp", 142)));

        const Suppressions::Suppression s2("abc", "include/1.h");
        ASSERT_EQUALS(true, s2.isSuppressed(errorMessage("abc", "include/1.h", 142)));
    }

    void suppressionsLine0() const {
        Suppressions suppressions;
        suppressions.addSuppressionLine("syntaxError:*:0");
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("syntaxError", "test.cpp", 0)));
    }

    void suppressionsFileComment() const {
        std::istringstream file1("# comment\n"
                                 "abc");
        Suppressions suppressions1;
        suppressions1.parseFile(file1);
        ASSERT_EQUALS(true, suppressions1.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file2("// comment\n"
                                 "abc");
        Suppressions suppressions2;
        suppressions2.parseFile(file2);
        ASSERT_EQUALS(true, suppressions2.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file3("abc // comment");
        Suppressions suppressions3;
        suppressions3.parseFile(file3);
        ASSERT_EQUALS(true, suppressions3.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file4("abc\t\t # comment");
        Suppressions suppressions4;
        suppressions4.parseFile(file4);
        ASSERT_EQUALS(true, suppressions4.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file5("abc:test.cpp\t\t # comment");
        Suppressions suppressions5;
        suppressions5.parseFile(file5);
        ASSERT_EQUALS(true, suppressions5.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file6("abc:test.cpp:123\t\t # comment with . inside");
        Suppressions suppressions6;
        suppressions6.parseFile(file6);
        ASSERT_EQUALS(true, suppressions6.isSuppressed(errorMessage("abc", "test.cpp", 123)));
    }

    void inlinesuppress() const {
        Suppressions::Suppression s;
        std::string msg;
        ASSERT_EQUALS(false, s.parseComment("/* some text */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress */", &msg));

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id */", &msg));
        ASSERT_EQUALS("", msg);

        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id some text */", &msg));
        ASSERT_EQUALS("Bad suppression attribute 'some'. You can write comments in the comment after a ; or //. Valid suppression attributes; symbolName=sym", msg);
    }

    void inlinesuppress_symbolname() {
        ASSERT_EQUALS(0, checkSuppression("void f() {\n"
                                          "    int a;\n"
                                          "    /* cppcheck-suppress uninitvar symbolName=a */\n"
                                          "    a++;\n"
                                          "}\n",
                                          ""));
        ASSERT_EQUALS("", errout.str());

        ASSERT_EQUALS(1, checkSuppression("void f() {\n"
                                          "    int a,b;\n"
                                          "    /* cppcheck-suppress uninitvar symbolName=b */\n"
                                          "    a++; b++;\n"
                                          "}\n",
                                          ""));
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());
    }

    void inlinesuppress_comment() const {
        Suppressions::Suppression s;
        std::string errMsg;
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc ; some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc // some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc -- some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
    }

    void multi_inlinesuppress() const {
        std::vector<Suppressions::Suppression> suppressions;
        std::string errMsg;

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId symbolName=arr]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("arr", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId symbolName=]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS("errorId1", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("errorId2", suppressions[1].errorId);
        ASSERT_EQUALS("arr", suppressions[1].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress errorId", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId1 errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbol=arr]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbolName]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());
    }

    void multi_inlinesuppress_comment() const {
        std::vector<Suppressions::Suppression> suppressions;
        std::string errMsg;

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("//cppcheck-suppress[errorId1, errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("//cppcheck-suppress[errorId1, errorId2 symbolName=arr] some text", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=Suppressions::parseMultiSuppressComment("/*cppcheck-suppress[errorId1, errorId2 symbolName=arr]*/", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());
    }

    void globalSuppressions() { // Testing that Cppcheck::useGlobalSuppressions works (#8515)
        errout.str("");

        CppCheck cppCheck(*this, false, nullptr); // <- do not "use global suppressions". pretend this is a thread that just checks a file.
        Settings& settings = cppCheck.settings();
        settings.nomsg.addSuppressionLine("uninitvar");
        settings.exitCode = 1;

        const char code[] = "int f() { int a; return a; }";
        ASSERT_EQUALS(0, cppCheck.check("test.c", code)); // <- no unsuppressed error is seen
        ASSERT_EQUALS("[test.c:1]: (error) Uninitialized variable: a\n", errout.str()); // <- report error so ThreadExecutor can suppress it and make sure the global suppression is matched.
    }

    void inlinesuppress_unusedFunction() const { // #4210, #4946 - wrong report of "unmatchedSuppression" for "unusedFunction"
        Suppressions suppressions;
        Suppressions::Suppression suppression("unusedFunction", "test.c", 3);
        suppression.checked = true; // have to do this because fixes for #5704
        suppressions.addSuppression(std::move(suppression));
        ASSERT_EQUALS(true, !suppressions.getUnmatchedLocalSuppressions("test.c", true).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedGlobalSuppressions(true).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedLocalSuppressions("test.c", false).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedGlobalSuppressions(false).empty());
    }

    void globalsuppress_unusedFunction() const { // #4946 - wrong report of "unmatchedSuppression" for "unusedFunction"
        Suppressions suppressions;
        suppressions.addSuppressionLine("unusedFunction:*");
        ASSERT_EQUALS(false, !suppressions.getUnmatchedLocalSuppressions("test.c", true).empty());
        ASSERT_EQUALS(true, !suppressions.getUnmatchedGlobalSuppressions(true).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedLocalSuppressions("test.c", false).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedGlobalSuppressions(false).empty());
    }

    void suppressionWithRelativePaths() {
        // Clear the error log
        errout.str("");

        CppCheck cppCheck(*this, true, nullptr);
        Settings& settings = cppCheck.settings();
        settings.severity.enable(Severity::style);
        settings.inlineSuppressions = true;
        settings.relativePaths = true;
        settings.basePaths.emplace_back("/somewhere");
        const char code[] =
            "struct Point\n"
            "{\n"
            "    // cppcheck-suppress unusedStructMember\n"
            "    int x;\n"
            "    // cppcheck-suppress unusedStructMember\n"
            "    int y;\n"
            "};";
        cppCheck.check("/somewhere/test.cpp", code);
        ASSERT_EQUALS("",errout.str());
    }

    void suppressingSyntaxErrors() { // syntaxErrors should be suppressible (#7076)
        const char code[] = "if if\n";

        ASSERT_EQUALS(0, checkSuppression(code, "syntaxError:test.cpp:1"));
        ASSERT_EQUALS("", errout.str());
    }

    void suppressingSyntaxErrorsInline() { // syntaxErrors should be suppressible (#5917)
        const char code[] = "double result(0.0);\n"
                            "_asm\n"
                            "{\n"
                            "   // cppcheck-suppress syntaxError\n"
                            "   push  EAX               ; save EAX for callers\n"
                            "   mov   EAX,Real10        ; get the address pointed to by Real10\n"
                            "   fld   TBYTE PTR [EAX]   ; load an extended real (10 bytes)\n"
                            "   fstp  QWORD PTR result  ; store a double (8 bytes)\n"
                            "   pop   EAX               ; restore EAX\n"
                            "}";
        ASSERT_EQUALS(0, checkSuppression(code, ""));
        ASSERT_EQUALS("", errout.str());
    }

    void suppressingSyntaxErrorsWhileFileRead() { // syntaxError while file read should be suppressible (PR #1333)
        const char code[] = "CONST (genType, KS_CONST) genService[KS_CFG_NR_OF_NVM_BLOCKS] =\n"
                            "{\n"
                            "[!VAR \"BC\" = \"$BC + 1\"!][!//\n"
                            "[!IF \"(as:modconf('Ks')[1]/KsGeneral/KsType = 'KS_CFG_TYPE_KS_MASTER') and\n"
                            "      (as:modconf('Ks')[1]/KsGeneral/KsUseShe = 'true')\"!][!//\n"
                            "  {\n"
                            "      &varNB_GetErrorStatus,\n"
                            "      &varNB_WriteBlock,\n"
                            "      &varNB_ReadBlock\n"
                            "  },\n"
                            "[!VAR \"BC\" = \"$BC + 1\"!][!//\n"
                            "[!ENDIF!][!//\n"
                            "};";
        ASSERT_EQUALS(0, checkSuppression(code, "syntaxError:test.cpp:4"));
        ASSERT_EQUALS("", errout.str());
    }

    void symbol() const {
        Suppressions::Suppression s;
        s.errorId = "foo";
        s.symbolName = "array*";

        Suppressions::ErrorMessage errorMsg;
        errorMsg.errorId = "foo";
        errorMsg.setFileName("test.cpp");
        errorMsg.lineNumber = 123;
        errorMsg.symbolNames = "";
        ASSERT_EQUALS(false, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "x\n";
        ASSERT_EQUALS(false, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "array1\n";
        ASSERT_EQUALS(true, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "x\n"
                               "array2\n";
        ASSERT_EQUALS(true, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "array3\n"
                               "x\n";
        ASSERT_EQUALS(true, s.isSuppressed(errorMsg));
    }

    void unusedFunction() {
        ASSERT_EQUALS(0, checkSuppression("void f() {}", "unusedFunction"));
    }

    void suppressingSyntaxErrorAndExitCode() {
        const char code[] = "fi if;";

        ASSERT_EQUALS(0, checkSuppression(code, "*:test.cpp"));
        ASSERT_EQUALS("", errout.str());

        // multi files, but only suppression one
        std::map<std::string, std::string> mfiles;
        mfiles["test.cpp"] = "fi if;";
        mfiles["test2.cpp"] = "fi if";
        ASSERT_EQUALS(2, checkSuppression(mfiles, "*:test.cpp"));
        ASSERT_EQUALS("[test2.cpp:1]: (error) syntax error\n", errout.str());

        // multi error in file, but only suppression one error
        const char code2[] = "fi fi\n"
                             "if if;";
        ASSERT_EQUALS(2, checkSuppression(code2, "*:test.cpp:1"));  // suppress all error at line 1 of test.cpp
        ASSERT_EQUALS("[test.cpp:2]: (error) syntax error\n", errout.str());

        // multi error in file, but only suppression one error (2)
        const char code3[] = "void f(int x, int y){\n"
                             "    int a = x/0;\n"
                             "    int b = y/0;\n"
                             "}\n"
                             "f(0, 1);\n";
        ASSERT_EQUALS(2, checkSuppression(code3, "zerodiv:test.cpp:3"));  // suppress 'errordiv' at line 3 of test.cpp
    }

    void suppressLocal() const {
        Suppressions suppressions;
        std::istringstream s("errorid:test.cpp\n"
                             "errorid2");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.cpp", 1)));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "test.cpp", 1), false));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "test2.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid", "test2.cpp", 1), false));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid2", "test.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid2", "test.cpp", 1), false));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid2", "test2.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("errorid2", "test2.cpp", 1), false));
    }

    void suppressUnmatchedSuppressions() {
        std::list<Suppressions::Suppression> suppressions;

        // No unmatched suppression
        errout.str("");
        suppressions.clear();
        Suppressions::reportUnmatchedSuppressions(suppressions, *this);
        ASSERT_EQUALS("", errout.str());

        // suppress all unmatchedSuppression
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "*", Suppressions::Suppression::NO_LINE);
        Suppressions::reportUnmatchedSuppressions(suppressions, *this);
        ASSERT_EQUALS("", errout.str());

        // suppress all unmatchedSuppression (corresponds to "--suppress=unmatchedSuppression")
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "", Suppressions::Suppression::NO_LINE);
        Suppressions::reportUnmatchedSuppressions(suppressions, *this);
        ASSERT_EQUALS("", errout.str());

        // suppress all unmatchedSuppression in a.c
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", Suppressions::Suppression::NO_LINE);
        Suppressions::reportUnmatchedSuppressions(suppressions, *this);
        ASSERT_EQUALS("", errout.str());

        // suppress unmatchedSuppression in a.c at line 10
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", 10U);
        Suppressions::reportUnmatchedSuppressions(suppressions, *this);
        ASSERT_EQUALS("", errout.str());

        // don't suppress unmatchedSuppression when file is mismatching
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "b.c", Suppressions::Suppression::NO_LINE);
        Suppressions::reportUnmatchedSuppressions(suppressions, *this);
        ASSERT_EQUALS("[a.c:10]: (information) Unmatched suppression: abc\n", errout.str());

        // don't suppress unmatchedSuppression when line is mismatching
        errout.str("");
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", 1U);
        Suppressions::reportUnmatchedSuppressions(suppressions, *this);
        ASSERT_EQUALS("[a.c:10]: (information) Unmatched suppression: abc\n", errout.str());
    }
};

REGISTER_TEST(TestSuppressions)

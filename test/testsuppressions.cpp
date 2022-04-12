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

#include "config.h"
#include "cppcheck.h"
#include "errortypes.h"
#include "settings.h"
#include "suppressions.h"
#include "testsuite.h"
#include "testutils.h"
#include "threadexecutor.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
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
        ret.certainty = Certainty::CertaintyLevel::normal;
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
        std::istringstream s("abc\r\ndef\r\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("abc")));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("def")));
    }

    void suppressionsFileNameWithColon() const {
        Suppressions suppressions;
        std::istringstream s("errorid:c:\\foo.cpp\nerrorid:c:\\bar.cpp:12");
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
            std::istringstream s("errorid:x*.cpp\nerrorid:y?.cpp\nerrorid:test.c*");
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
            std::istringstream s("errorid:x*.cpp\nerrorid:xyz.cpp:1\nerrorid:a*.cpp:1\nerrorid:abc.cpp:2");
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

    void reportSuppressions(const Settings &settings, const std::map<std::string, std::string> &files) {
        // make it verbose that this check is disabled
        const bool unusedFunctionCheck = false;

        if (settings.jointSuppressionReport) {
            for (std::map<std::string, std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
                reportUnmatchedSuppressions(settings.nomsg.getUnmatchedLocalSuppressions(i->first, unusedFunctionCheck));
            }
        }

        reportUnmatchedSuppressions(settings.nomsg.getUnmatchedGlobalSuppressions(unusedFunctionCheck));
    }

    // Check the suppression
    unsigned int checkSuppression(const char code[], const std::string &suppression = emptyString) {
        std::map<std::string, std::string> files;
        files["test.cpp"] = code;

        return checkSuppression(files, suppression);
    }

    // Check the suppression for multiple files
    unsigned int checkSuppression(std::map<std::string, std::string> &files, const std::string &suppression = emptyString) {
        // Clear the error log
        errout.str("");

        CppCheck cppCheck(*this, true, nullptr);
        Settings& settings = cppCheck.settings();
        settings.exitCode = 1;
        settings.inlineSuppressions = true;
        if (suppression == "unusedFunction")
            settings.checks.setEnabled(Checks::unusedFunction, true);
        settings.severity.enable(Severity::information);
        settings.jointSuppressionReport = true;
        if (!suppression.empty()) {
            std::string r = settings.nomsg.addSuppressionLine(suppression);
            EXPECT_EQ("", r);
        }

        unsigned int exitCode = 0;
        for (std::map<std::string, std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
            exitCode |= cppCheck.check(file->first, file->second);
        }
        if (cppCheck.analyseWholeProgram())
            exitCode |= settings.exitCode;

        reportSuppressions(settings, files);

        return exitCode;
    }

    unsigned int checkSuppressionThreads(const char code[], const std::string &suppression = emptyString) {
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> files;
        files["test.cpp"] = strlen(code);

        Settings settings;
        settings.jobs = 1;
        settings.inlineSuppressions = true;
        settings.severity.enable(Severity::information);
        if (!suppression.empty()) {
            EXPECT_EQ("", settings.nomsg.addSuppressionLine(suppression));
        }
        ThreadExecutor executor(files, settings, *this);
        std::vector<ScopedFile> scopedfiles;
        scopedfiles.reserve(files.size());
        for (std::map<std::string, std::size_t>::const_iterator i = files.begin(); i != files.end(); ++i)
            scopedfiles.emplace_back(i->first, code);

        const unsigned int exitCode = executor.check();

        std::map<std::string, std::string> files_for_report;
        for (std::map<std::string, std::size_t>::const_iterator file = files.begin(); file != files.end(); ++file)
            files_for_report[file->first] = "";

        reportSuppressions(settings, files_for_report);

        return exitCode;
    }

    void runChecks(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        // check to make sure the appropriate error is present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout.str());

        // suppress uninitvar globally
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "uninitvar");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar globally, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar");
        ASSERT_EQUALS("(information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress uninitvar for this file only
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "uninitvar:test.cpp");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress all for this file only
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "*:test.cpp");
        ASSERT_EQUALS("", errout.str());

        // suppress all for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "*:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: *\n", errout.str());

        // suppress uninitvar for this file and line
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "uninitvar:test.cpp:3");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar for this file and line, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp:3");
        ASSERT_EQUALS("[test.cpp:3]: (information) Unmatched suppression: uninitvar\n", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;// cppcheck-suppress uninitvar\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    /* cppcheck-suppress uninitvar */\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    /* cppcheck-suppress uninitvar */\n"
                       "\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;/* cppcheck-suppress uninitvar */\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress[uninitvar]\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress[uninitvar]\n"
                       "    a++;\n"
                       "\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;// cppcheck-suppress[uninitvar]\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    /* cppcheck-suppress[uninitvar]*/\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    /* cppcheck-suppress[uninitvar]*/\n"
                       "\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;/* cppcheck-suppress[uninitvar]*/\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout.str());

        // suppress uninitvar inline, with asm before (#6813)
        (this->*check)("void f() {\n"
                       "    __asm {\n"
                       "        foo\n"
                       "    }"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    a++;\n"
                       "}",
                       "");
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
        if (ThreadExecutor::isEnabled())
            runChecks(&TestSuppressions::checkSuppressionThreads);
    }

    void suppressionsMultiFile() {
        std::map<std::string, std::string> files;
        files["abc.cpp"] = "void f() {\n"
                           "}\n";
        files["xyz.cpp"] = "void f() {\n"
                           "    int a;\n"
                           "    a++;\n"
                           "}\n";

        // suppress uninitvar for this file and line
        checkSuppression(files, "uninitvar:xyz.cpp:3");
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
        std::istringstream file1("# comment\nabc");
        Suppressions suppressions1;
        suppressions1.parseFile(file1);
        ASSERT_EQUALS(true, suppressions1.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file2("// comment\nabc");
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

    void inlinesuppress() {
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
        checkSuppression("void f() {\n"
                         "    int a;\n"
                         "    /* cppcheck-suppress uninitvar symbolName=a */\n"
                         "    a++;\n"
                         "}\n",
                         "");
        ASSERT_EQUALS("", errout.str());

        checkSuppression("void f() {\n"
                         "    int a,b;\n"
                         "    /* cppcheck-suppress uninitvar symbolName=b */\n"
                         "    a++; b++;\n"
                         "}\n",
                         "");
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout.str());
    }

    void inlinesuppress_comment() {
        Suppressions::Suppression s;
        std::string errMsg;
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc ; some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc // some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc -- some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
    }

    void multi_inlinesuppress() {
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

    void multi_inlinesuppress_comment() {
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
        auto suppression = Suppressions::Suppression("unusedFunction", "test.c", 3);
        suppression.checked = true; // have to do this because fixes for #5704
        suppressions.addSuppression(suppression);
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
        std::map<std::string, std::string> files;
        files["test.cpp"] = "if if\n";

        checkSuppression(files, "syntaxError:test.cpp:1");
        ASSERT_EQUALS("", errout.str());
    }

    void suppressingSyntaxErrorsInline() { // syntaxErrors should be suppressible (#5917)
        std::map<std::string, std::string> files;
        files["test.cpp"] = "double result(0.0);\n"
                            "_asm\n"
                            "{\n"
                            "   // cppcheck-suppress syntaxError\n"
                            "   push  EAX               ; save EAX for callers\n"
                            "   mov   EAX,Real10        ; get the address pointed to by Real10\n"
                            "   fld   TBYTE PTR [EAX]   ; load an extended real (10 bytes)\n"
                            "   fstp  QWORD PTR result  ; store a double (8 bytes)\n"
                            "   pop   EAX               ; restore EAX\n"
                            "}";
        checkSuppression(files, "");
        ASSERT_EQUALS("", errout.str());
    }

    void suppressingSyntaxErrorsWhileFileRead() { // syntaxError while file read should be suppressible (PR #1333)
        std::map<std::string, std::string> files;
        files["test.cpp"] = "CONST (genType, KS_CONST) genService[KS_CFG_NR_OF_NVM_BLOCKS] =\n"
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
        checkSuppression(files, "syntaxError:test.cpp:4");
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
        errorMsg.symbolNames = "x\narray2\n";
        ASSERT_EQUALS(true, s.isSuppressed(errorMsg));
        errorMsg.symbolNames = "array3\nx\n";
        ASSERT_EQUALS(true, s.isSuppressed(errorMsg));
    }

    void unusedFunction() {
        ASSERT_EQUALS(0, checkSuppression("void f() {}", "unusedFunction"));
    }

    void suppressingSyntaxErrorAndExitCode() {
        std::map<std::string, std::string> files;
        files["test.cpp"] = "fi if;";

        ASSERT_EQUALS(0, checkSuppression(files, "*:test.cpp"));
        ASSERT_EQUALS("", errout.str());

        // multi files, but only suppression one
        std::map<std::string, std::string> mfiles;
        mfiles["test.cpp"] = "fi if;";
        mfiles["test2.cpp"] = "fi if";
        ASSERT_EQUALS(1, checkSuppression(mfiles, "*:test.cpp"));
        ASSERT_EQUALS("[test2.cpp:1]: (error) syntax error\n", errout.str());

        // multi error in file, but only suppression one error
        std::map<std::string, std::string> file2;
        file2["test.cpp"] = "fi fi\n"
                            "if if;";
        ASSERT_EQUALS(1, checkSuppression(file2, "*:test.cpp:1"));  // suppress all error at line 1 of test.cpp
        ASSERT_EQUALS("[test.cpp:2]: (error) syntax error\n", errout.str());

        // multi error in file, but only suppression one error (2)
        std::map<std::string, std::string> file3;
        file3["test.cpp"] = "void f(int x, int y){\n"
                            "    int a = x/0;\n"
                            "    int b = y/0;\n"
                            "}\n"
                            "f(0, 1);\n";
        ASSERT_EQUALS(1, checkSuppression(file3, "zerodiv:test.cpp:3"));  // suppress 'errordiv' at line 3 of test.cpp
    }

};

REGISTER_TEST(TestSuppressions)

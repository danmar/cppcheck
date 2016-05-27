/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

#include "cppcheck.h"
#include "settings.h"
#include "testsuite.h"
#include "cppcheckexecutor.h"
#include "threadexecutor.h"

#include <string>
#include <map>


class TestSuppressions : public TestFixture {
public:
    TestSuppressions() : TestFixture("TestSuppressions") {
    }

private:

    void run() {
        TEST_CASE(suppressionsBadId1);
        TEST_CASE(suppressionsDosFormat);     // Ticket #1836
        TEST_CASE(suppressionsFileNameWithColon);    // Ticket #1919 - filename includes colon
        TEST_CASE(suppressionsGlob);
        TEST_CASE(suppressionsFileNameWithExtraPath);
        TEST_CASE(suppressionsSettings);
        TEST_CASE(suppressionsMultiFile);
        TEST_CASE(suppressionsPathSeparator);

        TEST_CASE(inlinesuppress_unusedFunction); // #4210 - unusedFunction
        TEST_CASE(globalsuppress_unusedFunction); // #4946
        TEST_CASE(suppressionWithRelativePaths); // #4733
        TEST_CASE(suppressingSyntaxErrors); // #7076
        TEST_CASE(suppressingSyntaxErrorsInline); // #5917
    }

    void suppressionsBadId1() const {
        Suppressions suppressions;
        std::istringstream s1("123");
        ASSERT_EQUALS("Failed to add suppression. Invalid id \"123\"", suppressions.parseFile(s1));

        std::istringstream s2("obsoleteFunctionsrand_r");
        ASSERT_EQUALS("", suppressions.parseFile(s2));
    }

    void suppressionsDosFormat() const {
        Suppressions suppressions;
        std::istringstream s("abc\r\ndef\r\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed("abc", "test.cpp", 1));
        ASSERT_EQUALS(true, suppressions.isSuppressed("def", "test.cpp", 1));
    }

    void suppressionsFileNameWithColon() const {
        Suppressions suppressions;
        std::istringstream s("errorid:c:\\foo.cpp\nerrorid:c:\\bar.cpp:12");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "c:/foo.cpp", 1111));
        ASSERT_EQUALS(false, suppressions.isSuppressed("errorid", "c:/bar.cpp", 10));
        ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "c:/bar.cpp", 12));
    }

    void suppressionsGlob() const {
        // Check for syntax errors in glob
        {
            Suppressions suppressions;
            std::istringstream s("errorid:**.cpp\n");
            ASSERT_EQUALS("Failed to add suppression. Syntax error in glob.", suppressions.parseFile(s));
        }

        // Check that globbing works
        {
            Suppressions suppressions;
            std::istringstream s("errorid:x*.cpp\nerrorid:y?.cpp\nerrorid:test.c*");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp.cpp", 1));
            ASSERT_EQUALS(false, suppressions.isSuppressed("errorid", "abc.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "ya.cpp", 1));
            ASSERT_EQUALS(false, suppressions.isSuppressed("errorid", "y.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "test.c", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "test.cpp", 1));
        }

        // Check that both a filename match and a glob match apply
        {
            Suppressions suppressions;
            std::istringstream s("errorid:x*.cpp\nerrorid:xyz.cpp:1\nerrorid:a*.cpp:1\nerrorid:abc.cpp:2");
            ASSERT_EQUALS("", suppressions.parseFile(s));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "xyz.cpp", 2));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "abc.cpp", 1));
            ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "abc.cpp", 2));
        }
    }

    void suppressionsFileNameWithExtraPath() const {
        // Ticket #2797
        Suppressions suppressions;
        suppressions.addSuppression("errorid", "./a.c", 123);
        ASSERT_EQUALS(true, suppressions.isSuppressed("errorid", "a.c", 123));
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

        CppCheck cppCheck(*this, true);
        Settings& settings = cppCheck.settings();
        settings.inlineSuppressions = true;
        settings.addEnabled("information");
        settings.jointSuppressionReport = true;
        if (!suppression.empty()) {
            std::string r = settings.nomsg.addSuppressionLine(suppression);
            ASSERT_EQUALS("", r);
        }

        unsigned int exitCode = 0;
        for (std::map<std::string, std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
            exitCode |= cppCheck.check(file->first, file->second);
        }
        cppCheck.analyseWholeProgram();

        reportSuppressions(settings, files);

        return exitCode;
    }

    unsigned int checkSuppressionThreads(const char code[], const std::string &suppression = emptyString) {
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> files;
        files["test.cpp"] = 1;

        Settings settings;
        settings.jobs = 1;
        settings.inlineSuppressions = true;
        settings.addEnabled("information");
        if (!suppression.empty()) {
            ASSERT_EQUALS("", settings.nomsg.addSuppressionLine(suppression));
        }
        ThreadExecutor executor(files, settings, *this);
        for (std::map<std::string, std::size_t>::const_iterator i = files.begin(); i != files.end(); ++i)
            executor.addFileContent(i->first, code);

        unsigned int exitCode = executor.check();

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
        ASSERT_EQUALS("[*]: (information) Unmatched suppression: uninitvar\n", errout.str());

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
        Suppressions suppressions;
        suppressions.addSuppressionLine("*:test\\*");
        ASSERT_EQUALS(true, suppressions.isSuppressed("someid", "test/foo/bar.cpp", 142));

        suppressions.addSuppressionLine("abc:include/1.h");
        ASSERT_EQUALS(true, suppressions.isSuppressed("abc", "include\\1.h", 142));
    }

    void inlinesuppress_unusedFunction() const { // #4210, #4946 - wrong report of "unmatchedSuppression" for "unusedFunction"
        Suppressions suppressions;
        suppressions.addSuppression("unusedFunction", "test.c", 3U);
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

        CppCheck cppCheck(*this, true);
        Settings& settings = cppCheck.settings();
        settings.addEnabled("style");
        settings.inlineSuppressions = true;
        settings.relativePaths = true;
        settings.basePaths.push_back("/somewhere");
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

    void suppressingSyntaxErrors() { // syntaxErrors should be suppressable (#7076)
        std::map<std::string, std::string> files;
        files["test.cpp"] = "if if\n";

        checkSuppression(files, "syntaxError:test.cpp:1");
        ASSERT_EQUALS("", errout.str());
    }

    void suppressingSyntaxErrorsInline() { // syntaxErrors should be suppressable (#5917)
        std::map<std::string, std::string> files;
        files["test.cpp"] = "double result(0.0);\n"
                            "_asm\n"
                            "{\n"
                            "   // cppcheck-suppress syntaxError\n"
                            "   push  EAX               ; save EAX for callers \n"
                            "   mov   EAX,Real10        ; get the address pointed to by Real10\n"
                            "   fld   TBYTE PTR [EAX]   ; load an extended real (10 bytes)\n"
                            "   fstp  QWORD PTR result  ; store a double (8 bytes)\n"
                            "   pop   EAX               ; restore EAX\n"
                            "}";
        checkSuppression(files, "");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestSuppressions)

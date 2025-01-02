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

#include "config.h"
#include "cppcheck.h"
#include "cppcheckexecutor.h"
#include "errortypes.h"
#include "filesettings.h"
#include "processexecutor.h"
#include "settings.h"
#include "suppressions.h"
#include "fixture.h"
#include "helpers.h"
#include "threadexecutor.h"
#include "singleexecutor.h"

#include <cstring>
#include <list>
#include <map>
#include <memory>
#include <sstream>
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
        TEST_CASE(suppressionsGlobId);
        TEST_CASE(suppressionsFileNameWithExtraPath);
        TEST_CASE(suppressionsSettingsFiles);
        TEST_CASE(suppressionsSettingsFS);
        TEST_CASE(suppressionsSettingsThreadsFiles);
        TEST_CASE(suppressionsSettingsThreadsFS);
#if !defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
        TEST_CASE(suppressionsSettingsProcessesFiles);
        TEST_CASE(suppressionsSettingsProcessesFS);
#endif
        TEST_CASE(suppressionsMultiFileFiles);
        TEST_CASE(suppressionsMultiFileFS);
        TEST_CASE(suppressionsPathSeparator);
        TEST_CASE(suppressionsLine0);
        TEST_CASE(suppressionsFileComment);

        TEST_CASE(inlinesuppress);
        TEST_CASE(inlinesuppress_symbolname_Files);
        TEST_CASE(inlinesuppress_symbolname_FS);
        TEST_CASE(inlinesuppress_comment);

        TEST_CASE(multi_inlinesuppress);
        TEST_CASE(multi_inlinesuppress_comment);

        TEST_CASE(globalSuppressions); // Testing that global suppressions work (#8515)

        TEST_CASE(inlinesuppress_unusedFunction); // #4210 - unusedFunction
        TEST_CASE(globalsuppress_unusedFunction); // #4946
        TEST_CASE(suppressionWithRelativePaths); // #4733
        TEST_CASE(suppressingSyntaxErrorsFiles); // #7076
        TEST_CASE(suppressingSyntaxErrorsFS); // #7076
        TEST_CASE(suppressingSyntaxErrorsInlineFiles); // #5917
        TEST_CASE(suppressingSyntaxErrorsInlineFS); // #5917
        TEST_CASE(suppressingSyntaxErrorsWhileFileReadFiles); // PR #1333
        TEST_CASE(suppressingSyntaxErrorsWhileFileReadFS); // PR #1333
        TEST_CASE(symbol);

        TEST_CASE(unusedFunctionFiles);
        TEST_CASE(unusedFunctionFS);

        TEST_CASE(suppressingSyntaxErrorAndExitCodeFiles);
        TEST_CASE(suppressingSyntaxErrorAndExitCodeFS);
        TEST_CASE(suppressingSyntaxErrorAndExitCodeMultiFileFiles);
        TEST_CASE(suppressingSyntaxErrorAndExitCodeMultiFileFS);
        TEST_CASE(suppressLocal);

        TEST_CASE(suppressUnmatchedSuppressions);
        TEST_CASE(addSuppressionDuplicate);
        TEST_CASE(updateSuppressionState);
        TEST_CASE(addSuppressionLineMultiple);

        TEST_CASE(suppressionsParseXmlFile);

        TEST_CASE(toString);
    }

    void suppressionsBadId1() const {
        SuppressionList suppressions;
        std::istringstream s1("123");
        ASSERT_EQUALS("Failed to add suppression. Invalid id \"123\"", suppressions.parseFile(s1));

        std::istringstream s2("obsoleteFunctionsrand_r");
        ASSERT_EQUALS("", suppressions.parseFile(s2));
    }

    static SuppressionList::ErrorMessage errorMessage(const std::string &errorId) {
        SuppressionList::ErrorMessage ret;
        ret.errorId = errorId;
        ret.hash = 0;
        ret.lineNumber = 0;
        ret.certainty = Certainty::normal;
        return ret;
    }

    static SuppressionList::ErrorMessage errorMessage(const std::string &errorId, const std::string &file, int line) {
        SuppressionList::ErrorMessage ret;
        ret.errorId = errorId;
        ret.setFileName(file);
        ret.lineNumber = line;
        return ret;
    }

    void suppressionsDosFormat() const {
        SuppressionList suppressions;
        std::istringstream s("abc\r\n"
                             "def\r\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("abc")));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("def")));
    }

    void suppressionsFileNameWithColon() const {
        SuppressionList suppressions;
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
            SuppressionList suppressions;
            std::istringstream s("errorid:**.cpp\n");
            ASSERT_EQUALS("Failed to add suppression. Invalid glob pattern '**.cpp'.", suppressions.parseFile(s));
        }

        // Check that globbing works
        {
            SuppressionList suppressions;
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
            SuppressionList suppressions;
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

    void suppressionsGlobId() const {
        SuppressionList suppressions;
        std::istringstream s("a*\n");
        ASSERT_EQUALS("", suppressions.parseFile(s));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("abc", "xyz.cpp", 1)));
        ASSERT_EQUALS(false, suppressions.isSuppressed(errorMessage("def", "xyz.cpp", 1)));
    }

    void suppressionsFileNameWithExtraPath() const {
        // Ticket #2797
        SuppressionList suppressions;
        ASSERT_EQUALS("", suppressions.addSuppressionLine("errorid:./a.c:123"));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "a.c", 123)));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("errorid", "x/../a.c", 123)));
    }

    unsigned int checkSuppressionFiles(const char code[], const std::string &suppression = emptyString) {
        return _checkSuppression(code, false, suppression);
    }

    unsigned int checkSuppressionFS(const char code[], const std::string &suppression = emptyString) {
        return _checkSuppression(code, true, suppression);
    }

    // Check the suppression
    unsigned int _checkSuppression(const char code[], bool useFS, const std::string &suppression = emptyString) {
        std::map<std::string, std::string> files;
        files["test.cpp"] = code;

        return _checkSuppression(files, useFS, suppression);
    }

    unsigned int checkSuppressionFiles(std::map<std::string, std::string> &f, const std::string &suppression = emptyString) {
        return _checkSuppression(f, false, suppression);
    }

    unsigned int checkSuppressionFS(std::map<std::string, std::string> &f, const std::string &suppression = emptyString) {
        return _checkSuppression(f, true, suppression);
    }

    // Check the suppression for multiple files
    unsigned int _checkSuppression(std::map<std::string, std::string> &f, bool useFS, const std::string &suppression = emptyString) {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        for (auto i = f.cbegin(); i != f.cend(); ++i) {
            filelist.emplace_back(i->first, Standards::Language::CPP, i->second.size());
            if (useFS) {
                fileSettings.emplace_back(i->first, Standards::Language::CPP, i->second.size());
            }
        }

        CppCheck cppCheck(*this, true, nullptr);
        Settings& settings = cppCheck.settings();
        settings.jobs = 1;
        settings.quiet = true;
        settings.inlineSuppressions = true;
        settings.severity.enable(Severity::information);
        if (suppression == "unusedFunction")
            settings.checks.setEnabled(Checks::unusedFunction, true);
        if (!suppression.empty()) {
            ASSERT_EQUALS("", settings.supprs.nomsg.addSuppressionLine(suppression));
        }

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = f.cbegin(); i != f.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, i->second));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        SingleExecutor executor(cppCheck, filelist, fileSettings, settings, settings.supprs.nomsg, *this);
        const unsigned int exitCode = executor.check();

        CppCheckExecutor::reportSuppressions(settings, settings.supprs.nomsg, false, filelist, fileSettings, *this); // TODO: check result

        return exitCode;
    }

    unsigned int checkSuppressionThreadsFiles(const char code[], const std::string &suppression = emptyString) {
        return _checkSuppressionThreads(code, false, suppression);
    }

    unsigned int checkSuppressionThreadsFS(const char code[], const std::string &suppression = emptyString) {
        return _checkSuppressionThreads(code, true, suppression);
    }

    unsigned int _checkSuppressionThreads(const char code[], bool useFS, const std::string &suppression = emptyString) {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        filelist.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        if (useFS) {
            fileSettings.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        }

        /*const*/ auto settings = dinit(Settings,
                                        $.jobs = 2,
                                            $.quiet = true,
                                            $.inlineSuppressions = true);
        settings.severity.enable(Severity::information);
        if (!suppression.empty()) {
            ASSERT_EQUALS("", settings.supprs.nomsg.addSuppressionLine(suppression));
        }

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = filelist.cbegin(); i != filelist.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->path(), code));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        ThreadExecutor executor(filelist, fileSettings, settings, settings.supprs.nomsg, *this, CppCheckExecutor::executeCommand);
        const unsigned int exitCode = executor.check();

        CppCheckExecutor::reportSuppressions(settings, settings.supprs.nomsg, false, filelist, fileSettings, *this); // TODO: check result

        return exitCode;
    }

#if !defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
    unsigned int checkSuppressionProcessesFiles(const char code[], const std::string &suppression = emptyString) {
        return _checkSuppressionProcesses(code, false, suppression);
    }

    unsigned int checkSuppressionProcessesFS(const char code[], const std::string &suppression = emptyString) {
        return _checkSuppressionProcesses(code, true, suppression);
    }

    unsigned int _checkSuppressionProcesses(const char code[], bool useFS, const std::string &suppression = emptyString) {
        std::list<FileSettings> fileSettings;

        std::list<FileWithDetails> filelist;
        filelist.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        if (useFS) {
            fileSettings.emplace_back("test.cpp", Standards::Language::CPP, strlen(code));
        }

        /*const*/ auto settings = dinit(Settings,
                                        $.jobs = 2,
                                            $.quiet = true,
                                            $.inlineSuppressions = true);
        settings.severity.enable(Severity::information);
        if (!suppression.empty()) {
            ASSERT_EQUALS("", settings.supprs.nomsg.addSuppressionLine(suppression));
        }

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filelist.size());
        for (auto i = filelist.cbegin(); i != filelist.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->path(), code));

        // clear files list so only fileSettings are used
        if (useFS)
            filelist.clear();

        ProcessExecutor executor(filelist, fileSettings, settings, settings.supprs.nomsg, *this, CppCheckExecutor::executeCommand);
        const unsigned int exitCode = executor.check();

        CppCheckExecutor::reportSuppressions(settings, settings.supprs.nomsg, false, filelist, fileSettings, *this); // TODO: check result

        return exitCode;
    }
#endif

    void runChecks(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        // check to make sure the appropriate errors are present
        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:3]: (error) Uninitialized variable: a\n"
                      "[test.cpp:5]: (error) Uninitialized variable: b\n", errout_str());

        // suppress uninitvar globally
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar"));
        ASSERT_EQUALS("", errout_str());

        (this->*check)("void f() {\n"
                       "    // cppcheck-suppress-file uninitvar\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:2]: (error) File suppression should be at the top of the file\n"
                      "[test.cpp:4]: (error) Uninitialized variable: a\n", errout_str());

        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n"
                       "// cppcheck-suppress-file uninitvar\n",
                       "");
        ASSERT_EQUALS("[test.cpp:5]: (error) File suppression should be at the top of the file\n"
                      "[test.cpp:3]: (error) Uninitialized variable: a\n", errout_str());

        ASSERT_EQUALS(0, (this->*check)("// cppcheck-suppress-file uninitvar\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(0, (this->*check)("/* Fake file description\n"
                                        " * End\n"
                                        " */\n"
                                        "\n"
                                        "// cppcheck-suppress-file uninitvar\n"
                                        "\n"
                                        "void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        (this->*check)("// cppcheck-suppress-file uninitvar\n"
                       "void f() {\n"
                       "    int a;\n"
                       "    a++;\n"
                       "    int b;\n"
                       "    b++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar globally, without error present
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    b++;\n"
                                        "}\n",
                                        "uninitvar"));
        ASSERT_EQUALS("(information) Unmatched suppression: uninitvar\n", errout_str());

        (this->*check)("// cppcheck-suppress-file uninitvar\n"
                       "void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:1]: (information) Unmatched suppression: uninitvar\n", errout_str());

        // suppress uninitvar for this file only
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp"));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: uninitvar\n", errout_str());

        // suppress all for this file only
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "*:test.cpp"));
        ASSERT_EQUALS("", errout_str());

        // suppress all for this file only, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "*:test.cpp");
        ASSERT_EQUALS("[test.cpp]: (information) Unmatched suppression: *\n", errout_str());

        // suppress uninitvar for this file and line
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "}\n",
                                        "uninitvar:test.cpp:3"));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar for this file and line, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    b++;\n"
                       "}\n",
                       "uninitvar:test.cpp:3");
        ASSERT_EQUALS("[test.cpp:3]: (information) Unmatched suppression: uninitvar\n", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress uninitvar\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;// cppcheck-suppress uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar */\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar */\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;/* cppcheck-suppress uninitvar */\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress[uninitvar]\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress[uninitvar]\n"
                                        "    a++;\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;// cppcheck-suppress[uninitvar]\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress[uninitvar]*/\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress[uninitvar]*/\n"
                                        "\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;/* cppcheck-suppress[uninitvar]*/\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

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
        ASSERT_EQUALS("", errout_str());

        // suppress uninitvar inline, without error present
        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    b++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:4]: (information) Unmatched suppression: uninitvar\n", errout_str());

        // suppress block inline checks
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:2]: (error) Suppress Begin: No matching end\n"
                      "[test.cpp:4]: (error) Uninitialized variable: a\n"
                      "[test.cpp:6]: (error) Uninitialized variable: b\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    a++;\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:6]: (error) Suppress End: No matching begin\n"
                      "[test.cpp:3]: (error) Uninitialized variable: a\n"
                      "[test.cpp:5]: (error) Uninitialized variable: b\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7]: (error) Uninitialized variable: b\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin uninitvar\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end uninitvar\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7]: (error) Uninitialized variable: b\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin[uninitvar]\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end[uninitvar]\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7]: (error) Uninitialized variable: b\n", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    // cppcheck-suppress-begin [uninitvar]\n"
                                        "    a++;\n"
                                        "    // cppcheck-suppress-end [uninitvar]\n"
                                        "    int b;\n"
                                        "    b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:7]: (error) Uninitialized variable: b\n", errout_str());

        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    int b;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    b++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    int b;\n"
                       "    b++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("void f() {\n"
                       "    // cppcheck-suppress-begin [uninitvar]\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    int b;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    b++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    // cppcheck-suppress-end [uninitvar]\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("void f() {\n"
                       "    // cppcheck-suppress-begin [uninitvar, syntaxError]\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    int b;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    b++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    // cppcheck-suppress-end [uninitvar, syntaxError]\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:2]: (information) Unmatched suppression: syntaxError\n", errout_str());

        (this->*check)("// cppcheck-suppress-begin [uninitvar, syntaxError]\n"
                       "void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    int b;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    b++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "}\n"
                       "// cppcheck-suppress-end [uninitvar, syntaxError]\n",
                       "");
        ASSERT_EQUALS("[test.cpp:1]: (information) Unmatched suppression: syntaxError\n", errout_str());

        (this->*check)("// cppcheck-suppress-begin [uninitvar, syntaxError]\n"
                       "void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "    int b;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    b++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "}\n"
                       "// cppcheck-suppress-end [uninitvar, syntaxError]",
                       "");
        ASSERT_EQUALS("[test.cpp:1]: (information) Unmatched suppression: syntaxError\n", errout_str());

        // test of multiple suppression types
        (this->*check)("// cppcheck-suppress-file uninitvar\n"
                       "void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("// cppcheck-suppress-file uninitvar\n"
                       "void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("// cppcheck-suppress-file uninitvar\n"
                       "void f() {\n"
                       "    int a;\n"
                       "    // cppcheck-suppress-begin uninitvar\n"
                       "    a++;\n"
                       "    // cppcheck-suppress-end uninitvar\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("// cppcheck-suppress-file uninitvar\n"
                       "void f() {\n"
                       "    // cppcheck-suppress uninitvar\n"
                       "    int a;\n"
                       "    a++;\n"
                       "}\n",
                       "");
        ASSERT_EQUALS("[test.cpp:4]: (information) Unmatched suppression: uninitvar\n", errout_str());

        // #5746 - exitcode
        ASSERT_EQUALS(1U,
                      (this->*check)("int f() {\n"
                                     "  int a; return a;\n"
                                     "}\n",
                                     ""));
        ASSERT_EQUALS("[test.cpp:2]: (error) Uninitialized variable: a\n", errout_str());

        ASSERT_EQUALS(0U,
                      (this->*check)("int f() {\n"
                                     "  int a; return a;\n"
                                     "}\n",
                                     "uninitvar"));
        ASSERT_EQUALS("", errout_str());

        // cppcheck-suppress-macro
        (this->*check)("// cppcheck-suppress-macro zerodiv\n"
                       "#define DIV(A,B) A/B\n"
                       "a = DIV(10,0);\n",
                       "");
        ASSERT_EQUALS("", errout_str());

        (this->*check)("// cppcheck-suppress-macro abc\n"
                       "#define DIV(A,B) A/B\n"
                       "a = DIV(10,1);\n",
                       "");
        ASSERT_EQUALS("", errout_str()); // <- no unmatched suppression reported for macro suppression
    }

    void suppressionsSettingsFiles() {
        runChecks(&TestSuppressions::checkSuppressionFiles);
    }

    static void suppressionsSettingsFS() {
        // TODO
        // runChecks(&TestSuppressions::checkSuppressionFS);
    }

    void suppressionsSettingsThreadsFiles() {
        runChecks(&TestSuppressions::checkSuppressionThreadsFiles);
    }

    void suppressionsSettingsThreadsFS() {
        runChecks(&TestSuppressions::checkSuppressionThreadsFS);
    }

#if !defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
    void suppressionsSettingsProcessesFiles() {
        runChecks(&TestSuppressions::checkSuppressionProcessesFiles);
    }

    void suppressionsSettingsProcessesFS() {
        runChecks(&TestSuppressions::checkSuppressionProcessesFS);
    }
#endif

    void suppressionsMultiFileInternal(unsigned int (TestSuppressions::*check)(std::map<std::string, std::string> &f, const std::string &)) {
        std::map<std::string, std::string> files;
        files["abc.cpp"] = "void f() {\n"
                           "}\n";
        files["xyz.cpp"] = "void f() {\n"
                           "    int a;\n"
                           "    a++;\n"
                           "}\n";

        // suppress uninitvar for this file and line
        ASSERT_EQUALS(0, (this->*check)(files, "uninitvar:xyz.cpp:3"));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressionsMultiFileFiles() {
        suppressionsMultiFileInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressionsMultiFileFS() {
        suppressionsMultiFileInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressionsPathSeparator() const {
        const SuppressionList::Suppression s1("*", "test/foo/*");
        ASSERT_EQUALS(true, s1.isSuppressed(errorMessage("someid", "test/foo/bar.cpp", 142)));

        const SuppressionList::Suppression s2("abc", "include/1.h");
        ASSERT_EQUALS(true, s2.isSuppressed(errorMessage("abc", "include/1.h", 142)));
    }

    void suppressionsLine0() const {
        SuppressionList suppressions;
        ASSERT_EQUALS("", suppressions.addSuppressionLine("syntaxError:*:0"));
        ASSERT_EQUALS(true, suppressions.isSuppressed(errorMessage("syntaxError", "test.cpp", 0)));
    }

    void suppressionsFileComment() const {
        std::istringstream file1("# comment\n"
                                 "abc");
        SuppressionList suppressions1;
        ASSERT_EQUALS("", suppressions1.parseFile(file1));
        ASSERT_EQUALS(true, suppressions1.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file2("// comment\n"
                                 "abc");
        SuppressionList suppressions2;
        ASSERT_EQUALS("", suppressions2.parseFile(file2));
        ASSERT_EQUALS(true, suppressions2.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file3("abc // comment");
        SuppressionList suppressions3;
        ASSERT_EQUALS("", suppressions3.parseFile(file3));
        ASSERT_EQUALS(true, suppressions3.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file4("abc\t\t # comment");
        SuppressionList suppressions4;
        ASSERT_EQUALS("", suppressions4.parseFile(file4));
        ASSERT_EQUALS(true, suppressions4.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file5("abc:test.cpp\t\t # comment");
        SuppressionList suppressions5;
        ASSERT_EQUALS("", suppressions5.parseFile(file5));
        ASSERT_EQUALS(true, suppressions5.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file6("abc:test.cpp:123\t\t # comment with . inside");
        SuppressionList suppressions6;
        ASSERT_EQUALS("", suppressions6.parseFile(file6));
        ASSERT_EQUALS(true, suppressions6.isSuppressed(errorMessage("abc", "test.cpp", 123)));

        std::istringstream file7(" // comment\n" // #11450
                                 "abc");
        SuppressionList suppressions7;
        ASSERT_EQUALS("", suppressions7.parseFile(file7));
        ASSERT_EQUALS(true, suppressions7.isSuppressed(errorMessage("abc", "test.cpp", 123)));
    }

    void inlinesuppress() const {
        SuppressionList::Suppression s;
        std::string msg;

        // Suppress without attribute
        ASSERT_EQUALS(false, s.parseComment("/* some text */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-file  */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-begin */", &msg));
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-end */", &msg));

        // Correct suppress
        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id */", &msg));
        ASSERT_EQUALS("", msg);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-file id */", &msg));
        ASSERT_EQUALS("", msg);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-begin id */", &msg));
        ASSERT_EQUALS("", msg);

        msg.clear();
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-end id */", &msg));
        ASSERT_EQUALS("", msg);

        // Bad cppcheck-suppress comment
        ASSERT_EQUALS(false, s.parseComment("/* cppcheck-suppress-beggin id */", &msg));

        // Bad attribute construction
        const std::string badSuppressionAttribute = "Bad suppression attribute 'some'. You can write comments in the comment after a ; or //. Valid suppression attributes; symbolName=sym";

        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-file id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-begin id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
        ASSERT_EQUALS(true, s.parseComment("/* cppcheck-suppress-end id some text */", &msg));
        ASSERT_EQUALS(badSuppressionAttribute, msg);
    }

    void inlinesuppress_symbolname_Internal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        ASSERT_EQUALS(0, (this->*check)("void f() {\n"
                                        "    int a;\n"
                                        "    /* cppcheck-suppress uninitvar symbolName=a */\n"
                                        "    a++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS(1, (this->*check)("void f() {\n"
                                        "    int a,b;\n"
                                        "    /* cppcheck-suppress uninitvar symbolName=b */\n"
                                        "    a++; b++;\n"
                                        "}\n",
                                        ""));
        ASSERT_EQUALS("[test.cpp:4]: (error) Uninitialized variable: a\n", errout_str());
    }

    void inlinesuppress_symbolname_Files() {
        inlinesuppress_symbolname_Internal(&TestSuppressions::checkSuppressionFiles);
    }

    void inlinesuppress_symbolname_FS() {
        inlinesuppress_symbolname_Internal(&TestSuppressions::checkSuppressionFS);
    }

    void inlinesuppress_comment() const {
        SuppressionList::Suppression s;
        std::string errMsg;
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc ; some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc // some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
        ASSERT_EQUALS(true, s.parseComment("// cppcheck-suppress abc -- some comment", &errMsg));
        ASSERT_EQUALS("", errMsg);
    }

    void multi_inlinesuppress() const {
        std::vector<SuppressionList::Suppression> suppressions;
        std::string errMsg;

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-begin[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-begin [errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-end[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-end [errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-file[errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress-file [errorId]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId symbolName=arr]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("arr", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId symbolName=]", &errMsg);
        ASSERT_EQUALS(1, suppressions.size());
        ASSERT_EQUALS("errorId", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS("errorId1", suppressions[0].errorId);
        ASSERT_EQUALS("", suppressions[0].symbolName);
        ASSERT_EQUALS("errorId2", suppressions[1].errorId);
        ASSERT_EQUALS("arr", suppressions[1].symbolName);
        ASSERT_EQUALS("", errMsg);

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress errorId", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1 errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbol=arr]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("// cppcheck-suppress[errorId1, errorId2 symbolName]", &errMsg);
        ASSERT_EQUALS(0, suppressions.size());
        ASSERT_EQUALS(false, errMsg.empty());
    }

    void multi_inlinesuppress_comment() const {
        std::vector<SuppressionList::Suppression> suppressions;
        std::string errMsg;

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("//cppcheck-suppress[errorId1, errorId2 symbolName=arr]", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("//cppcheck-suppress[errorId1, errorId2 symbolName=arr] some text", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());

        errMsg = "";
        suppressions=SuppressionList::parseMultiSuppressComment("/*cppcheck-suppress[errorId1, errorId2 symbolName=arr]*/", &errMsg);
        ASSERT_EQUALS(2, suppressions.size());
        ASSERT_EQUALS(true, errMsg.empty());
    }

    void globalSuppressions() { // Testing that Cppcheck::useGlobalSuppressions works (#8515)
        CppCheck cppCheck(*this, false, nullptr); // <- do not "use global suppressions". pretend this is a thread that just checks a file.
        Settings& settings = cppCheck.settings();
        settings.quiet = true;
        ASSERT_EQUALS("", settings.supprs.nomsg.addSuppressionLine("uninitvar"));
        settings.exitCode = 1;

        const char code[] = "int f() { int a; return a; }";
        ASSERT_EQUALS(0, cppCheck.check(FileWithDetails("test.c"), code)); // <- no unsuppressed error is seen
        ASSERT_EQUALS("[test.c:1]: (error) Uninitialized variable: a\n", errout_str()); // <- report error so ThreadExecutor can suppress it and make sure the global suppression is matched.
    }

    void inlinesuppress_unusedFunction() const { // #4210, #4946 - wrong report of "unmatchedSuppression" for "unusedFunction"
        SuppressionList suppressions;
        SuppressionList::Suppression suppression("unusedFunction", "test.c", 3);
        suppression.checked = true; // have to do this because fixes for #5704
        ASSERT_EQUALS("", suppressions.addSuppression(std::move(suppression)));
        ASSERT_EQUALS(true, !suppressions.getUnmatchedLocalSuppressions(FileWithDetails("test.c"), true).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedGlobalSuppressions(true).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedLocalSuppressions(FileWithDetails("test.c"), false).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedGlobalSuppressions(false).empty());
    }

    void globalsuppress_unusedFunction() const { // #4946 - wrong report of "unmatchedSuppression" for "unusedFunction"
        SuppressionList suppressions;
        ASSERT_EQUALS("", suppressions.addSuppressionLine("unusedFunction:*"));
        ASSERT_EQUALS(false, !suppressions.getUnmatchedLocalSuppressions(FileWithDetails("test.c"), true).empty());
        ASSERT_EQUALS(true, !suppressions.getUnmatchedGlobalSuppressions(true).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedLocalSuppressions(FileWithDetails("test.c"), false).empty());
        ASSERT_EQUALS(false, !suppressions.getUnmatchedGlobalSuppressions(false).empty());
    }

    void suppressionWithRelativePaths() {
        CppCheck cppCheck(*this, true, nullptr);
        Settings& settings = cppCheck.settings();
        settings.quiet = true;
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
        ASSERT_EQUALS(0, cppCheck.check(FileWithDetails("/somewhere/test.cpp"), code));
        ASSERT_EQUALS("",errout_str());
    }

    void suppressingSyntaxErrorsInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) { // syntaxErrors should be suppressible (#7076)
        const char code[] = "if if\n";

        ASSERT_EQUALS(0, (this->*check)(code, "syntaxError:test.cpp:1"));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressingSyntaxErrorsFiles() {
        suppressingSyntaxErrorsInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorsFS() {
        suppressingSyntaxErrorsInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorsInlineInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) { // syntaxErrors should be suppressible (#5917)
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
        ASSERT_EQUALS(0, (this->*check)(code, ""));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressingSyntaxErrorsInlineFiles() {
        suppressingSyntaxErrorsInlineInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorsInlineFS() {
        suppressingSyntaxErrorsInlineInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressingSyntaxErrorsWhileFileReadInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) { // syntaxError while file read should be suppressible (PR #1333)
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
        ASSERT_EQUALS(0, (this->*check)(code, "syntaxError:test.cpp:4"));
        ASSERT_EQUALS("", errout_str());
    }

    void suppressingSyntaxErrorsWhileFileReadFiles() {
        suppressingSyntaxErrorsWhileFileReadInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void suppressingSyntaxErrorsWhileFileReadFS() {
        suppressingSyntaxErrorsWhileFileReadInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void symbol() const {
        SuppressionList::Suppression s;
        s.errorId = "foo";
        s.symbolName = "array*";

        SuppressionList::ErrorMessage errorMsg;
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

    void unusedFunctionInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        ASSERT_EQUALS(0, (this->*check)("void f() {}", "unusedFunction"));
    }

    void unusedFunctionFiles() {
        unusedFunctionInternal(&TestSuppressions::checkSuppressionFiles);
    }

    void unusedFunctionFS() {
        unusedFunctionInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressingSyntaxErrorAndExitCodeInternal(unsigned int (TestSuppressions::*check)(const char[], const std::string &)) {
        const char code[] = "fi if;";

        ASSERT_EQUALS(0, (this->*check)(code, "*:test.cpp"));
        ASSERT_EQUALS("", errout_str());

        // multi error in file, but only suppression one error
        const char code2[] = "fi fi\n"
                             "if if;";
        ASSERT_EQUALS(1, (this->*check)(code2, "*:test.cpp:1"));  // suppress all error at line 1 of test.cpp
        ASSERT_EQUALS("[test.cpp:2]: (error) syntax error\n", errout_str());

        // multi error in file, but only suppression one error (2)
        const char code3[] = "void f(int x, int y){\n"
                             "    int a = x/0;\n"
                             "    int b = y/0;\n"
                             "}\n"
                             "f(0, 1);\n";
        ASSERT_EQUALS(1, (this->*check)(code3, "zerodiv:test.cpp:3"));  // suppress 'zerodiv' at line 3 of test.cpp
        ASSERT_EQUALS("[test.cpp:2]: (error) Division by zero.\n", errout_str());
    }

    void suppressingSyntaxErrorAndExitCodeFiles() {
        suppressingSyntaxErrorAndExitCodeInternal(&TestSuppressions::checkSuppressionFiles);
    }

    static void suppressingSyntaxErrorAndExitCodeFS() {
        // TODO
        // suppressingSyntaxErrorAndExitCodeInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressingSyntaxErrorAndExitCodeMultiFileInternal(unsigned int (TestSuppressions::*check)(std::map<std::string, std::string> &f, const std::string &)) {
        // multi files, but only suppression one
        std::map<std::string, std::string> mfiles;
        mfiles["test.cpp"] = "fi if;";
        mfiles["test2.cpp"] = "fi if";
        ASSERT_EQUALS(1, (this->*check)(mfiles, "*:test.cpp"));
        ASSERT_EQUALS("[test2.cpp:1]: (error) syntax error\n", errout_str());
    }

    void suppressingSyntaxErrorAndExitCodeMultiFileFiles() {
        suppressingSyntaxErrorAndExitCodeMultiFileInternal(&TestSuppressions::checkSuppressionFiles);
    }

    static void suppressingSyntaxErrorAndExitCodeMultiFileFS() {
        // TODO
        // suppressingSyntaxErrorAndExitCodeMultiFileInternal(&TestSuppressions::checkSuppressionFS);
    }

    void suppressLocal() const {
        SuppressionList suppressions;
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
        std::list<SuppressionList::Suppression> suppressions;

        // No unmatched suppression
        suppressions.clear();
        ASSERT_EQUALS(false, SuppressionList::reportUnmatchedSuppressions(suppressions, *this));
        ASSERT_EQUALS("", errout_str());

        // suppress all unmatchedSuppression
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "*", SuppressionList::Suppression::NO_LINE);
        ASSERT_EQUALS(false, SuppressionList::reportUnmatchedSuppressions(suppressions, *this));
        ASSERT_EQUALS("", errout_str());

        // suppress all unmatchedSuppression (corresponds to "--suppress=unmatchedSuppression")
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "", SuppressionList::Suppression::NO_LINE);
        ASSERT_EQUALS(false, SuppressionList::reportUnmatchedSuppressions(suppressions, *this));
        ASSERT_EQUALS("", errout_str());

        // suppress all unmatchedSuppression in a.c
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", SuppressionList::Suppression::NO_LINE);
        ASSERT_EQUALS(false, SuppressionList::reportUnmatchedSuppressions(suppressions, *this));
        ASSERT_EQUALS("", errout_str());

        // suppress unmatchedSuppression in a.c at line 10
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", 10U);
        ASSERT_EQUALS(false, SuppressionList::reportUnmatchedSuppressions(suppressions, *this));
        ASSERT_EQUALS("", errout_str());

        // don't suppress unmatchedSuppression when file is mismatching
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "b.c", SuppressionList::Suppression::NO_LINE);
        ASSERT_EQUALS(true, SuppressionList::reportUnmatchedSuppressions(suppressions, *this));
        ASSERT_EQUALS("[a.c:10]: (information) Unmatched suppression: abc\n", errout_str());

        // don't suppress unmatchedSuppression when line is mismatching
        suppressions.clear();
        suppressions.emplace_back("abc", "a.c", 10U);
        suppressions.emplace_back("unmatchedSuppression", "a.c", 1U);
        ASSERT_EQUALS(true, SuppressionList::reportUnmatchedSuppressions(suppressions, *this));
        ASSERT_EQUALS("[a.c:10]: (information) Unmatched suppression: abc\n", errout_str());
    }

    void suppressionsParseXmlFile() const {
        {
            ScopedFile file("suppressparsexml.xml",
                            "<suppressions>\n"
                            "<suppress>\n"
                            "<id>uninitvar</id>\n"
                            "<fileName>file.c</fileName>\n"
                            "<lineNumber>10</lineNumber>\n"
                            "<symbolName>sym</symbolName>\n"
                            "</suppress>\n"
                            "</suppressions>");

            SuppressionList supprList;
            ASSERT_EQUALS("", supprList.parseXmlFile(file.path().c_str()));
            const auto& supprs = supprList.getSuppressions();
            ASSERT_EQUALS(1, supprs.size());
            const auto& suppr = *supprs.cbegin();
            ASSERT_EQUALS("uninitvar", suppr.errorId);
            ASSERT_EQUALS("file.c", suppr.fileName);
            ASSERT_EQUALS(10, suppr.lineNumber);
            ASSERT_EQUALS("sym", suppr.symbolName);
        }

        // no file specified
        {
            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML '' (XML_ERROR_FILE_NOT_FOUND).", supprList.parseXmlFile(""));
        }

        // missing file
        {
            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML 'suppressparsexml.xml' (XML_ERROR_FILE_NOT_FOUND).", supprList.parseXmlFile("suppressparsexml.xml"));
        }

        // empty file
        {
            ScopedFile file("suppressparsexml.xml",
                            "");

            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML 'suppressparsexml.xml' (XML_ERROR_EMPTY_DOCUMENT).", supprList.parseXmlFile(file.path().c_str()));
        }

        // wrong root node
        {
            ScopedFile file("suppressparsexml.xml",
                            "<suppress/>\n");

            SuppressionList supprList;
            ASSERT_EQUALS("", supprList.parseXmlFile(file.path().c_str()));
        }

        // no root node
        {
            ScopedFile file("suppressparsexml.xml",
                            "<?xml version=\"1.0\"?>\n");

            SuppressionList supprList;
            ASSERT_EQUALS("failed to load suppressions XML 'suppressparsexml.xml' (no root node found).", supprList.parseXmlFile(file.path().c_str()));
        }

        // unknown element
        {
            ScopedFile file("suppressparsexml.xml",
                            "<suppressions>\n"
                            "<suppress>\n"
                            "<eid>uninitvar</eid>\n"
                            "</suppress>\n"
                            "</suppressions>");

            SuppressionList supprList;
            ASSERT_EQUALS("unknown element 'eid' in suppressions XML 'suppressparsexml.xml', expected id/fileName/lineNumber/symbolName/hash.", supprList.parseXmlFile(file.path().c_str()));
        }
    }

    void addSuppressionDuplicate() const {
        SuppressionList supprs;

        SuppressionList::Suppression s;
        s.errorId = "uninitvar";

        ASSERT_EQUALS("", supprs.addSuppression(s));
        ASSERT_EQUALS("suppression 'uninitvar' already exists", supprs.addSuppression(s));
    }

    void updateSuppressionState() const {
        {
            SuppressionList supprs;

            SuppressionList::Suppression s;
            s.errorId = "uninitVar";
            ASSERT_EQUALS(false, supprs.updateSuppressionState(s));
        }
        {
            SuppressionList supprs;

            SuppressionList::Suppression s;
            s.errorId = "uninitVar";

            ASSERT_EQUALS("", supprs.addSuppression(s));

            ASSERT_EQUALS(true, supprs.updateSuppressionState(s));

            const std::list<SuppressionList::Suppression> l = supprs.getUnmatchedGlobalSuppressions(false);
            ASSERT_EQUALS(1, l.size());
        }
        {
            SuppressionList supprs;

            SuppressionList::Suppression s;
            s.errorId = "uninitVar";
            s.matched = false;

            ASSERT_EQUALS("", supprs.addSuppression(s));

            s.matched = true;
            ASSERT_EQUALS(true, supprs.updateSuppressionState(s));

            const std::list<SuppressionList::Suppression> l = supprs.getUnmatchedGlobalSuppressions(false);
            ASSERT_EQUALS(0, l.size());
        }
    }

    void addSuppressionLineMultiple() {
        SuppressionList supprlist;

        ASSERT_EQUALS("", supprlist.addSuppressionLine("syntaxError"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("uninitvar:1.c"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("memleak:1.c"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("uninitvar:2.c"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("memleak:3.c:12 # first"));
        ASSERT_EQUALS("", supprlist.addSuppressionLine("memleak:3.c:22 // second"));

        const auto& supprs = supprlist.getSuppressions();
        ASSERT_EQUALS(6, supprs.size());

        auto it = supprs.cbegin();

        ASSERT_EQUALS("syntaxError", it->errorId);
        ASSERT_EQUALS("", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("uninitvar", it->errorId);
        ASSERT_EQUALS("1.c", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("memleak", it->errorId);
        ASSERT_EQUALS("1.c", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("uninitvar", it->errorId);
        ASSERT_EQUALS("2.c", it->fileName);
        ASSERT_EQUALS(SuppressionList::Suppression::NO_LINE, it->lineNumber);
        ++it;

        ASSERT_EQUALS("memleak", it->errorId);
        ASSERT_EQUALS("3.c", it->fileName);
        ASSERT_EQUALS(12, it->lineNumber);
        ++it;

        ASSERT_EQUALS("memleak", it->errorId);
        ASSERT_EQUALS("3.c", it->fileName);
        ASSERT_EQUALS(22, it->lineNumber);
    }

    void toString() const
    {
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            ASSERT_EQUALS("unitvar", s.toString());
        }
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            s.fileName = "test.cpp";
            ASSERT_EQUALS("unitvar:test.cpp", s.toString());
        }
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            s.fileName = "test.cpp";
            s.lineNumber = 12;
            ASSERT_EQUALS("unitvar:test.cpp:12", s.toString());
        }
        {
            SuppressionList::Suppression s;
            s.errorId = "unitvar";
            s.symbolName = "sym";
            ASSERT_EQUALS("unitvar:sym", s.toString());
        }
    }
};

REGISTER_TEST(TestSuppressions)

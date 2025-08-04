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

#include "addoninfo.h"
#include "color.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "path.h"
#include "preprocessor.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <list>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <simplecpp.h>

class TestCppcheck : public TestFixture {
public:
    TestCppcheck() : TestFixture("TestCppcheck") {}

private:
    const std::string templateFormat{"{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]"};

    class ErrorLogger2 : public ErrorLogger {
    public:
        std::list<std::string> ids;
        std::list<ErrorMessage> errmsgs;

    private:
        void reportOut(const std::string & /*outmsg*/, Color /*c*/ = Color::Reset) override {}

        void reportErr(const ErrorMessage &msg) override {
            ids.push_back(msg.id);
            errmsgs.push_back(msg);
        }

        void reportMetric(const std::string &metric) override {
            (void) metric;
        }
    };

    void run() override {
        TEST_CASE(getErrorMessages);
        TEST_CASE(checkWithFile);
        TEST_CASE(checkWithFileWithTools);
        TEST_CASE(checkWithFileWithToolsNoCommand);
        TEST_CASE(checkWithFS);
        TEST_CASE(checkWithFSWithTools);
        TEST_CASE(checkWithFSWithToolsNoCommand);
        TEST_CASE(suppress_error_library);
        TEST_CASE(unique_errors);
        TEST_CASE(unique_errors_2);
        TEST_CASE(isPremiumCodingStandardId);
        TEST_CASE(getDumpFileContentsRawTokens);
        TEST_CASE(getDumpFileContentsLibrary);
        TEST_CASE(premiumResultsCache);
    }

    void getErrorMessages() const {
        ErrorLogger2 errorLogger;
        CppCheck::getErrorMessages(errorLogger);
        ASSERT(!errorLogger.ids.empty());

        // Check if there are duplicate error ids in errorLogger.id
        std::string duplicate;
        for (auto it = errorLogger.ids.cbegin();
             it != errorLogger.ids.cend();
             ++it) {
            if (std::find(errorLogger.ids.cbegin(), it, *it) != it) {
                duplicate = "Duplicate ID: " + *it;
                break;
            }
        }
        ASSERT_EQUALS("", duplicate);

        // Check for error ids from this class.
        bool foundPurgedConfiguration = false;
        bool foundTooManyConfigs = false;
        bool foundMissingInclude = false; // #11984
        bool foundMissingIncludeSystem = false; // #11984
        for (const std::string & it : errorLogger.ids) {
            if (it == "purgedConfiguration")
                foundPurgedConfiguration = true;
            else if (it == "toomanyconfigs")
                foundTooManyConfigs = true;
            else if (it == "missingInclude")
                foundMissingInclude = true;
            else if (it == "missingIncludeSystem")
                foundMissingIncludeSystem = true;
        }
        ASSERT(foundPurgedConfiguration);
        ASSERT(foundTooManyConfigs);
        ASSERT(foundMissingInclude);
        ASSERT(foundMissingIncludeSystem);
    }

    static std::string exename_(const std::string& exe)
    {
#ifdef _WIN32
        return exe + ".exe";
#else
        return exe;
#endif
    }

    CppCheck::ExecuteCmdFn getExecuteCommand(int& called) const
    {
        // cppcheck-suppress passedByValue - used as callback so we need to preserve the signature
        // NOLINTNEXTLINE(performance-unnecessary-value-param) - used as callback so we need to preserve the signature
        return [&](std::string exe, std::vector<std::string> args, std::string redirect, std::string& /*output*/) -> int {
            ++called;
            if (exe == exename_("clang-tidy"))
            {
                ASSERT_EQUALS(4, args.size());
                ASSERT_EQUALS("-quiet", args[0]);
                ASSERT_EQUALS("-checks=*,-clang-analyzer-*,-llvm*", args[1]);
                ASSERT_EQUALS("test.cpp", args[2]);
                ASSERT_EQUALS("--", args[3]);
                ASSERT_EQUALS("2>&1", redirect);
                return EXIT_SUCCESS;
            }
            if (exe == exename_("python3"))
            {
                ASSERT_EQUALS(1, args.size());
                ASSERT_EQUALS("--version", args[0]);
                ASSERT_EQUALS("2>&1", redirect);
                return EXIT_SUCCESS;
            }
            if (exe == exename_("python"))
            {
                ASSERT_EQUALS(1, args.size());
                ASSERT_EQUALS("--version", args[0]);
                ASSERT_EQUALS("2>&1", redirect);
                return EXIT_SUCCESS;
            }
            ASSERT_MSG(false, "unhandled exe: " + exe);
            return EXIT_FAILURE;
        };
    }

    void checkWithFileInternal(bool tools, bool nocmd = false) const
    {
        REDIRECT;
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        int called = 0;
        std::unordered_set<std::string> addons;
        std::vector<AddonInfo> addonInfo;
        if (tools)
        {
            addons.emplace("testcppcheck");
            addonInfo.emplace_back(/*AddonInfo()*/);
        }
        const auto s = dinit(Settings,
                             $.templateFormat = templateFormat,
                                 $.clangTidy = tools,
                                 $.addons = std::move (addons),
                                 $.addonInfos = std::move (addonInfo));
        Suppressions supprs;
        ErrorLogger2 errorLogger;
        CppCheck::ExecuteCmdFn f;
        if (tools && !nocmd) {
            f = getExecuteCommand(called);
        }
        CppCheck cppcheck(s, supprs, errorLogger, false, f);
        ASSERT_EQUALS(1, cppcheck.check(FileWithDetails(file.path(), Path::identify(file.path(), false), 0)));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        errorLogger.errmsgs.erase(std::remove_if(errorLogger.errmsgs.begin(), errorLogger.errmsgs.end(), [](const ErrorMessage& msg) {
            return msg.id == "logChecker";
        }), errorLogger.errmsgs.end());
        if (tools)
        {
            ASSERT_EQUALS(2, errorLogger.ids.size());
            auto it = errorLogger.errmsgs.cbegin();
            ASSERT_EQUALS("nullPointer", it->id);
            ++it;

            if (nocmd)
            {
                ASSERT_EQUALS("internalError", it->id);
                ASSERT_EQUALS("Bailing out from analysis: Checking file failed: Failed to execute addon - no command callback provided", it->shortMessage()); // TODO: add addon name

                // TODO: clang-tidy is currently not invoked for file inputs - see #12053
                // TODO: needs to become a proper error
                TODO_ASSERT_EQUALS("Failed to execute '" + exename_("clang-tidy") + "' (no command callback provided)\n", "", GET_REDIRECT_ERROUT);

                ASSERT_EQUALS(0, called); // not called because we check if the callback exists
            }
            else
            {
                ASSERT_EQUALS("internalError", it->id);
                ASSERT_EQUALS("Bailing out from analysis: Checking file failed: Failed to auto detect python", it->shortMessage()); // TODO: clarify what python is used for

                // TODO: we cannot check this because the python detection is cached globally so this result will different dependent on how the test is called
                //ASSERT_EQUALS(2, called);
            }
        }
        else
        {
            ASSERT_EQUALS(0, called);
            ASSERT_EQUALS(1, errorLogger.ids.size());
            ASSERT_EQUALS("nullPointer", *errorLogger.ids.cbegin());
        }
    }

    void checkWithFile() const {
        checkWithFileInternal(false);
    }

    void checkWithFileWithTools() const {
        checkWithFileInternal(true);
    }

    void checkWithFileWithToolsNoCommand() const {
        checkWithFileInternal(true, true);
    }

    void checkWithFSInternal(bool tools, bool nocmd = false) const
    {
        REDIRECT;
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        int called = 0;
        std::unordered_set<std::string> addons;
        std::vector<AddonInfo> addonInfo;
        if (tools)
        {
            addons.emplace("testcppcheck");
            addonInfo.emplace_back(/*AddonInfo()*/);
        }
        const auto s = dinit(Settings,
                             $.templateFormat = templateFormat,
                                 $.clangTidy = tools,
                                 $.addons = std::move (addons),
                                 $.addonInfos = std::move (addonInfo));
        Suppressions supprs;
        ErrorLogger2 errorLogger;
        CppCheck::ExecuteCmdFn f;
        if (tools && !nocmd) {
            f = getExecuteCommand(called);
        }
        CppCheck cppcheck(s, supprs, errorLogger, false, f);
        FileSettings fs{file.path(), Path::identify(file.path(), false), 0};
        ASSERT_EQUALS(1, cppcheck.check(fs));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        errorLogger.errmsgs.erase(std::remove_if(errorLogger.errmsgs.begin(), errorLogger.errmsgs.end(), [](const ErrorMessage& msg) {
            return msg.id == "logChecker";
        }), errorLogger.errmsgs.end());
        if (tools)
        {
            ASSERT_EQUALS(2, errorLogger.ids.size());
            auto it = errorLogger.errmsgs.cbegin();
            ASSERT_EQUALS("nullPointer", it->id);
            ++it;

            if (nocmd)
            {
                ASSERT_EQUALS("internalError", it->id);
                ASSERT_EQUALS("Bailing out from analysis: Checking file failed: Failed to execute addon - no command callback provided", it->shortMessage()); // TODO: add addon name

                // TODO: needs to become a proper error
                ASSERT_EQUALS("Failed to execute '" + exename_("clang-tidy") + "' (no command callback provided)\n", GET_REDIRECT_ERROUT);

                ASSERT_EQUALS(0, called); // not called because we check if the callback exists
            }
            else
            {
                ASSERT_EQUALS("internalError", it->id);
                ASSERT_EQUALS("Bailing out from analysis: Checking file failed: Failed to auto detect python", it->shortMessage()); // TODO: clarify what python is used for

                // TODO: we cannot check this because the python detection is cached globally so this result will different dependent on how the test is called
                //ASSERT_EQUALS(3, called);
            }
        }
        else
        {
            ASSERT_EQUALS(0, called);
            ASSERT_EQUALS(1, errorLogger.ids.size());
            ASSERT_EQUALS("nullPointer", *errorLogger.ids.cbegin());
        }
    }

    void checkWithFS() const {
        checkWithFSInternal(false);
    }

    void checkWithFSWithTools() const {
        checkWithFSInternal(true);
    }

    void checkWithFSWithToolsNoCommand() const {
        checkWithFSInternal(true, true);
    }

    void suppress_error_library() const
    {
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        const char xmldata[] = R"(<def format="2"><markup ext=".cpp" reporterrors="false"/></def>)";
        const Settings s = settingsBuilder().libraryxml(xmldata).build();
        Suppressions supprs;
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(s, supprs, errorLogger, false, {});
        ASSERT_EQUALS(0, cppcheck.check(FileWithDetails(file.path(), Path::identify(file.path(), false), 0)));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        ASSERT_EQUALS(0, errorLogger.ids.size());
    }

    // TODO: how to actually get duplicated findings
    void unique_errors() const
    {
        ScopedFile file("inc.h",
                        "inline void f()\n"
                        "{\n"
                        "  (void)*((int*)0);\n"
                        "}");
        ScopedFile test_file_a("a.cpp",
                               "#include \"inc.h\"");
        ScopedFile test_file_b("b.cpp",
                               "#include \"inc.h\"");

        // this is the "simple" format
        const auto s = dinit(Settings, $.templateFormat = templateFormat); // TODO: remove when we only longer rely on toString() in unique message handling
        Suppressions supprs;
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(s, supprs, errorLogger, false, {});
        ASSERT_EQUALS(1, cppcheck.check(FileWithDetails(test_file_a.path(), Path::identify(test_file_a.path(), false), 0)));
        ASSERT_EQUALS(1, cppcheck.check(FileWithDetails(test_file_b.path(), Path::identify(test_file_b.path(), false), 0)));
        // TODO: how to properly disable these warnings?
        errorLogger.errmsgs.erase(std::remove_if(errorLogger.errmsgs.begin(), errorLogger.errmsgs.end(), [](const ErrorMessage& msg) {
            return msg.id == "logChecker";
        }), errorLogger.errmsgs.end());
        // the internal errorlist is cleared after each check() call
        ASSERT_EQUALS(2, errorLogger.errmsgs.size());
        auto it = errorLogger.errmsgs.cbegin();
        ASSERT_EQUALS("a.cpp", it->file0);
        ASSERT_EQUALS("nullPointer", it->id);
        ++it;
        ASSERT_EQUALS("b.cpp", it->file0);
        ASSERT_EQUALS("nullPointer", it->id);
    }

    void unique_errors_2() const
    {
        ScopedFile test_file("c.cpp",
                             "void f()\n"
                             "{\n"
                             "const long m[9] = {};\n"
                             "long a=m[9], b=m[9];\n"
                             "(void)a;\n"
                             "(void)b;\n"
                             "}");

        // this is the "simple" format
        const auto s = dinit(Settings, $.templateFormat = templateFormat); // TODO: remove when we only longer rely on toString() in unique message handling?
        Suppressions supprs;
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(s, supprs, errorLogger, false, {});
        ASSERT_EQUALS(1, cppcheck.check(FileWithDetails(test_file.path(), Path::identify(test_file.path(), false), 0)));
        // TODO: how to properly disable these warnings?
        errorLogger.errmsgs.erase(std::remove_if(errorLogger.errmsgs.begin(), errorLogger.errmsgs.end(), [](const ErrorMessage& msg) {
            return msg.id == "logChecker";
        }), errorLogger.errmsgs.end());
        // the internal errorlist is cleared after each check() call
        ASSERT_EQUALS(2, errorLogger.errmsgs.size());
        auto it = errorLogger.errmsgs.cbegin();
        ASSERT_EQUALS("c.cpp", it->file0);
        ASSERT_EQUALS(1, it->callStack.size());
        {
            auto stack = it->callStack.cbegin();
            ASSERT_EQUALS(4, stack->line);
            ASSERT_EQUALS(9, stack->column);
        }
        ASSERT_EQUALS("arrayIndexOutOfBounds", it->id);
        ++it;
        ASSERT_EQUALS("c.cpp", it->file0);
        ASSERT_EQUALS(1, it->callStack.size());
        {
            auto stack = it->callStack.cbegin();
            ASSERT_EQUALS(4, stack->line);
            ASSERT_EQUALS(17, stack->column);
        }
        ASSERT_EQUALS("arrayIndexOutOfBounds", it->id);
    }

    void isPremiumCodingStandardId() const {
        Suppressions supprs;
        ErrorLogger2 errorLogger;

        {
            const auto s = dinit(Settings, $.premiumArgs = "");
            CppCheck cppcheck(s, supprs, errorLogger, false, {});

            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("misra-c2012-0.0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("misra-c2023-0.0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c-2012-0.0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c-2023-0.0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c-2025-0.0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c-2025-dir-0.0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c++-2008-0-0-0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c++-2023-0.0.0"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-cert-int50-cpp"));
            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-autosar-0-0-0"));
        }

        {
            const auto s = dinit(Settings, $.premiumArgs = "--misra-c-2012 --cert-c++-2016 --autosar");

            CppCheck cppcheck(s, supprs, errorLogger, false, {});

            ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("misra-c2012-0.0"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c-2012-0.0"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c-2023-0.0"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c-2025-0.0"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c-2025-dir-0.0"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c++-2008-0-0-0"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c++-2023-0.0.0"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-cert-int50-cpp"));
            ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-autosar-0-0-0"));
        }
    }

    void getDumpFileContentsRawTokens() const {
        Settings s;
        s.relativePaths = true;
        s.basePaths.emplace_back("/some/path");
        Suppressions supprs;
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(s, supprs, errorLogger, false, {});
        std::vector<std::string> files{"/some/path/test.cpp"};
        simplecpp::TokenList tokens1(files);
        const std::string expected = "  <rawtokens>\n"
                                     "    <file index=\"0\" name=\"test.cpp\"/>\n"
                                     "  </rawtokens>\n";
        ASSERT_EQUALS(expected, cppcheck.getDumpFileContentsRawTokens(files, tokens1));
    }

    void getDumpFileContentsLibrary() const {
        Suppressions supprs;
        ErrorLogger2 errorLogger;

        {
            Settings s;
            s.libraries.emplace_back("std.cfg");
            CppCheck cppcheck(s, supprs, errorLogger, false, {});
            //std::vector<std::string> files{ "/some/path/test.cpp" };
            const std::string expected = "  <library lib=\"std.cfg\"/>\n";
            ASSERT_EQUALS(expected, cppcheck.getLibraryDumpData());
        }

        {
            Settings s;
            s.libraries.emplace_back("std.cfg");
            s.libraries.emplace_back("posix.cfg");
            CppCheck cppcheck(s, supprs, errorLogger, false, {});
            const std::string expected = "  <library lib=\"std.cfg\"/>\n  <library lib=\"posix.cfg\"/>\n";
            ASSERT_EQUALS(expected, cppcheck.getLibraryDumpData());
        }
    }

    void premiumResultsCache() const {
        // Trac #13889 - cached misra results are shown after removing --premium=misra-c-2012 option

        Settings settings;
        Suppressions supprs;
        ErrorLogger2 errorLogger;

        std::vector<std::string> files;

        std::istringstream istr("void f();\nint x;\n");
        const simplecpp::TokenList tokens(istr, files, "m1.c");

        Preprocessor preprocessor(settings, errorLogger, Standards::Language::C);
        ASSERT(preprocessor.loadFiles(tokens, files));

        AddonInfo premiumaddon;
        premiumaddon.name = "premiumaddon.json";
        premiumaddon.executable = "premiumaddon";

        settings.cppcheckCfgProductName = "Cppcheck Premium 0.0.0";
        settings.addons.insert(premiumaddon.name);
        settings.addonInfos.push_back(premiumaddon);

        settings.premiumArgs = "misra-c-2012";
        CppCheck check(settings, supprs, errorLogger, false, {});
        const size_t hash1 = check.calculateHash(preprocessor, tokens);

        settings.premiumArgs = "";
        const size_t hash2 = check.calculateHash(preprocessor, tokens);

        // cppcheck-suppress knownConditionTrueFalse
        ASSERT(hash1 != hash2);
    }

    // TODO: test suppressions
    // TODO: test all with FS
};

REGISTER_TEST(TestCppcheck)

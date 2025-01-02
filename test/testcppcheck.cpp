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

#include "color.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

#include "simplecpp.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

class TestCppcheck : public TestFixture {
public:
    TestCppcheck() : TestFixture("TestCppcheck") {}

private:

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
    };

    void run() override {
        TEST_CASE(getErrorMessages);
        TEST_CASE(checkWithFile);
        TEST_CASE(checkWithFS);
        TEST_CASE(suppress_error_library);
        TEST_CASE(unique_errors);
        TEST_CASE(isPremiumCodingStandardId);
        TEST_CASE(getDumpFileContentsRawTokens);
        TEST_CASE(getDumpFileContentsLibrary);
        TEST_CASE(getClangFlagsIncludeFile);
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

    void checkWithFile() const
    {
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        ASSERT_EQUALS(1, cppcheck.check(FileWithDetails(file.path())));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        ASSERT_EQUALS(1, errorLogger.ids.size());
        ASSERT_EQUALS("nullPointer", *errorLogger.ids.cbegin());
    }

    void checkWithFS() const
    {
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        FileSettings fs{file.path()};
        ASSERT_EQUALS(1, cppcheck.check(fs));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        ASSERT_EQUALS(1, errorLogger.ids.size());
        ASSERT_EQUALS("nullPointer", *errorLogger.ids.cbegin());
    }

    void suppress_error_library() const
    {
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        const char xmldata[] = R"(<def format="2"><markup ext=".cpp" reporterrors="false"/></def>)";
        const Settings s = settingsBuilder().libraryxml(xmldata, sizeof(xmldata)).build();
        cppcheck.settings() = s;
        ASSERT_EQUALS(0, cppcheck.check(FileWithDetails(file.path())));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        ASSERT_EQUALS(0, errorLogger.ids.size());
    }

    // TODO: hwo to actually get duplicated findings
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

        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        ASSERT_EQUALS(1, cppcheck.check(FileWithDetails(test_file_a.path())));
        ASSERT_EQUALS(1, cppcheck.check(FileWithDetails(test_file_b.path())));
        // TODO: how to properly disable these warnings?
        errorLogger.errmsgs.erase(std::remove_if(errorLogger.errmsgs.begin(), errorLogger.errmsgs.end(), [](const ErrorMessage& msg) {
            return msg.id == "logChecker";
        }), errorLogger.errmsgs.end());
        // the internal errorlist is cleared after each check() call
        ASSERT_EQUALS(2, errorLogger.errmsgs.size());
        auto it = errorLogger.errmsgs.cbegin();
        ASSERT_EQUALS("nullPointer", it->id);
        ++it;
        ASSERT_EQUALS("nullPointer", it->id);
    }

    void isPremiumCodingStandardId() const {
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});

        cppcheck.settings().premiumArgs = "";
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("misra-c2012-0.0"));
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("misra-c2023-0.0"));
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c2012-0.0"));
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c2023-0.0"));
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c++2008-0.0.0"));
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-misra-c++2023-0.0.0"));
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-cert-int50-cpp"));
        ASSERT_EQUALS(false, cppcheck.isPremiumCodingStandardId("premium-autosar-0-0-0"));

        cppcheck.settings().premiumArgs = "--misra-c-2012 --cert-c++-2016 --autosar";
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("misra-c2012-0.0"));
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("misra-c2023-0.0"));
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c2012-0.0"));
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c2023-0.0"));
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c++2008-0.0.0"));
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-misra-c++2023-0.0.0"));
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-cert-int50-cpp"));
        ASSERT_EQUALS(true, cppcheck.isPremiumCodingStandardId("premium-autosar-0-0-0"));
    }

    void getDumpFileContentsRawTokens() const {
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        cppcheck.settings() = settingsBuilder().build();
        cppcheck.settings().relativePaths = true;
        cppcheck.settings().basePaths.emplace_back("/some/path");
        std::vector<std::string> files{"/some/path/test.cpp"};
        simplecpp::TokenList tokens1(files);
        const std::string expected = "  <rawtokens>\n"
                                     "    <file index=\"0\" name=\"test.cpp\"/>\n"
                                     "  </rawtokens>\n";
        ASSERT_EQUALS(expected, cppcheck.getDumpFileContentsRawTokens(files, tokens1));
    }

    void getDumpFileContentsLibrary() const {
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        cppcheck.settings().libraries.emplace_back("std.cfg");
        std::vector<std::string> files{ "/some/path/test.cpp" };
        const std::string expected1 = "  <library lib=\"std.cfg\"/>\n";
        ASSERT_EQUALS(expected1, cppcheck.getLibraryDumpData());
        cppcheck.settings().libraries.emplace_back("posix.cfg");
        const std::string expected2 = "  <library lib=\"std.cfg\"/>\n  <library lib=\"posix.cfg\"/>\n";
        ASSERT_EQUALS(expected2, cppcheck.getLibraryDumpData());
    }

    void getClangFlagsIncludeFile() const {
        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        cppcheck.settings().userIncludes.emplace_back("1.h");
        ASSERT_EQUALS("-x c --include 1.h ", cppcheck.getClangFlags(Standards::Language::C));
    }

    // TODO: test suppressions
    // TODO: test all with FS
};

REGISTER_TEST(TestCppcheck)

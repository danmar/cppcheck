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

#include "color.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"

#include <algorithm>
#include <list>
#include <string>


class TestCppcheck : public TestFixture {
public:
    TestCppcheck() : TestFixture("TestCppcheck") {}

private:

    class ErrorLogger2 : public ErrorLogger {
    public:
        std::list<std::string> ids;

    private:
        void reportOut(const std::string & /*outmsg*/, Color /*c*/ = Color::Reset) override {}

        void reportErr(const ErrorMessage &msg) override {
            ids.push_back(msg.id);
        }
    };

    void run() override {
        TEST_CASE(getErrorMessages);
        TEST_CASE(checkWithFile);
        TEST_CASE(checkWithFS);
        TEST_CASE(suppress_error_library);
    }

    void getErrorMessages() const {
        ErrorLogger2 errorLogger;
        CppCheck::getErrorMessages(errorLogger);
        ASSERT(!errorLogger.ids.empty());

        // Check if there are duplicate error ids in errorLogger.id
        std::string duplicate;
        for (std::list<std::string>::const_iterator it = errorLogger.ids.cbegin();
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
        for (const std::string & it : errorLogger.ids) {
            if (it == "purgedConfiguration")
                foundPurgedConfiguration = true;
            else if (it == "toomanyconfigs")
                foundTooManyConfigs = true;
        }
        ASSERT(foundPurgedConfiguration);
        ASSERT(foundTooManyConfigs);
    }

    void checkWithFile()
    {
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        ASSERT_EQUALS(1, cppcheck.check(file.path()));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        ASSERT_EQUALS(1, errorLogger.ids.size());
        ASSERT_EQUALS("nullPointer", *errorLogger.ids.cbegin());
    }

    void checkWithFS()
    {
        ScopedFile file("test.cpp",
                        "int main()\n"
                        "{\n"
                        "  int i = *((int*)0);\n"
                        "  return 0;\n"
                        "}");

        ErrorLogger2 errorLogger;
        CppCheck cppcheck(errorLogger, false, {});
        FileSettings fs;
        fs.filename = file.path();
        ASSERT_EQUALS(1, cppcheck.check(fs));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        ASSERT_EQUALS(1, errorLogger.ids.size());
        ASSERT_EQUALS("nullPointer", *errorLogger.ids.cbegin());
    }

    void suppress_error_library()
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
        ASSERT_EQUALS(0, cppcheck.check(file.path()));
        // TODO: how to properly disable these warnings?
        errorLogger.ids.erase(std::remove_if(errorLogger.ids.begin(), errorLogger.ids.end(), [](const std::string& id) {
            return id == "logChecker";
        }), errorLogger.ids.end());
        ASSERT_EQUALS(0, errorLogger.ids.size());
    }

    // TODO: test suppressions
    // TODO: test unique errors
    // TODO: test all with FS
};

REGISTER_TEST(TestCppcheck)

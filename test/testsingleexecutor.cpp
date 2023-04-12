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
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cppcheck.h"
#include "fixture.h"
#include "helpers.h"
#include "redirect.h"
#include "library.h"
#include "settings.h"
#include "singleexecutor.h"
#include "timer.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class TestSingleExecutor : public TestFixture {
public:
    TestSingleExecutor() : TestFixture("TestSingleExecutor") {}

private:
    Settings settings;

    static std::string zpad3(int i)
    {
        if (i < 10)
            return "00" + std::to_string(i);
        if (i < 100)
            return "0" + std::to_string(i);
        return std::to_string(i);
    }

    void check(int files, int result, const std::string &data, SHOWTIME_MODES showtime = SHOWTIME_MODES::SHOWTIME_NONE, const char* const plistOutput = nullptr, const std::vector<std::string>& filesList = {}) {
        errout.str("");
        output.str("");

        std::map<std::string, std::size_t> filemap;
        if (filesList.empty()) {
            for (int i = 1; i <= files; ++i) {
                const std::string s = "file_" + zpad3(i) + ".cpp";
                filemap[s] = data.size();
            }
        }
        else {
            for (const auto& f : filesList)
            {
                filemap[f] = data.size();
            }
        }

        settings.showtime = showtime;
        if (plistOutput)
            settings.plistOutput = plistOutput;
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        CppCheck cppcheck(*this, true, [](std::string,std::vector<std::string>,std::string,std::string&){
            return false;
        });
        cppcheck.settings() = settings;
        // TODO: test with settings.project.fileSettings;
        SingleExecutor executor(cppcheck, filemap, settings, *this);
        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filemap.size());
        for (std::map<std::string, std::size_t>::const_iterator i = filemap.cbegin(); i != filemap.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, data));

        ASSERT_EQUALS(result, executor.check());
    }

    void run() override {
        LOAD_LIB_2(settings.library, "std.cfg");

        TEST_CASE(many_files);
        TEST_CASE(many_files_showtime);
        TEST_CASE(many_files_plist);
        TEST_CASE(no_errors_more_files);
        TEST_CASE(no_errors_less_files);
        TEST_CASE(no_errors_equal_amount_files);
        TEST_CASE(one_error_less_files);
        TEST_CASE(one_error_several_files);
        TEST_CASE(markup);
    }

    void many_files() {
        const Settings settingsOld = settings;
        settings.quiet = false;

        check(100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}");
        std::string expected;
        for (int i = 1; i <= 100; ++i) {
            expected += "Checking file_" + zpad3(i) + ".cpp ...\n";
            expected += std::to_string(i) + "/100 files checked " + std::to_string(i) + "% done\n";
        }
        ASSERT_EQUALS(expected, output.str());

        settings = settingsOld;
    }

    void many_files_showtime() {
        SUPPRESS;
        check(100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}", SHOWTIME_MODES::SHOWTIME_SUMMARY);
    }

    void many_files_plist() {
        const char plistOutput[] = "plist";
        ScopedFile plistFile("dummy", plistOutput);

        check(100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}", SHOWTIME_MODES::SHOWTIME_NONE, plistOutput);
    }

    void no_errors_more_files() {
        check(3, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_less_files() {
        check(1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void no_errors_equal_amount_files() {
        check(2, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}");
    }

    void one_error_less_files() {
        check(1, 1,
              "int main()\n"
              "{\n"
              "  {char *a = malloc(10);}\n"
              "  return 0;\n"
              "}");
    }

    void one_error_several_files() {
        check(20, 20,
              "int main()\n"
              "{\n"
              "  {char *a = malloc(10);}\n"
              "  return 0;\n"
              "}");
    }

    void markup() {
        const Settings settingsOld = settings;
        settings.library.mMarkupExtensions.emplace(".cp1");
        settings.library.mProcessAfterCode.emplace(".cp1", true);

        const std::vector<std::string> files = {
            "file_1.cp1", "file_2.cpp", "file_3.cp1", "file_4.cpp"
        };

        check(4, 4,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}",
              SHOWTIME_MODES::SHOWTIME_NONE, nullptr, files);
        // TODO: filter out the "files checked" messages
        ASSERT_EQUALS("Checking file_2.cpp ...\n"
                      "1/4 files checked 25% done\n"
                      "Checking file_4.cpp ...\n"
                      "2/4 files checked 50% done\n"
                      "Checking file_1.cp1 ...\n"
                      "3/4 files checked 75% done\n"
                      "Checking file_3.cp1 ...\n"
                      "4/4 files checked 100% done\n", output.str());
        settings = settingsOld;
    }

    // TODO: test clang-tidy
    // TODO: test whole program analysis
};

REGISTER_TEST(TestSingleExecutor)

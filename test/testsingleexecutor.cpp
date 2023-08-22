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
#include "importproject.h"
#include "redirect.h"
#include "library.h"
#include "settings.h"
#include "singleexecutor.h"
#include "timer.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

class TestSingleExecutorBase : public TestFixture {
protected:
    TestSingleExecutorBase(const char * const name, bool useFS) : TestFixture(name), useFS(useFS) {}

private:
    Settings settings = settingsBuilder().library("std.cfg").build();
    bool useFS;

    std::string fprefix() const
    {
        if (useFS)
            return "singlefs";
        return "single";
    }

    static std::string zpad3(int i)
    {
        if (i < 10)
            return "00" + std::to_string(i);
        if (i < 100)
            return "0" + std::to_string(i);
        return std::to_string(i);
    }

    struct CheckOptions
    {
        CheckOptions() = default;
        SHOWTIME_MODES showtime = SHOWTIME_MODES::SHOWTIME_NONE;
        const char* plistOutput = nullptr;
        std::vector<std::string> filesList;
        bool executeCommandCalled = false;
        std::string exe;
        std::vector<std::string> args;
    };

    void check(int files, int result, const std::string &data, const CheckOptions& opt = make_default_obj{}) {
        errout.str("");
        output.str("");
        settings.project.fileSettings.clear();

        std::map<std::string, std::size_t> filemap;
        if (opt.filesList.empty()) {
            for (int i = 1; i <= files; ++i) {
                const std::string s = fprefix() + "_" + zpad3(i) + ".cpp";
                filemap[s] = data.size();
                if (useFS) {
                    ImportProject::FileSettings fs;
                    fs.filename = s;
                    settings.project.fileSettings.emplace_back(std::move(fs));
                }
            }
        }
        else {
            for (const auto& f : opt.filesList)
            {
                filemap[f] = data.size();
                if (useFS) {
                    ImportProject::FileSettings fs;
                    fs.filename = f;
                    settings.project.fileSettings.emplace_back(std::move(fs));
                }
            }
        }

        settings.showtime = opt.showtime;
        if (opt.plistOutput)
            settings.plistOutput = opt.plistOutput;

        bool executeCommandCalled = false;
        std::string exe;
        std::vector<std::string> args;
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        CppCheck cppcheck(*this, true, [&executeCommandCalled, &exe, &args](std::string e,std::vector<std::string> a,std::string,std::string&){
            executeCommandCalled = true;
            exe = std::move(e);
            args = std::move(a);
            return true;
        });
        cppcheck.settings() = settings;

        std::vector<std::unique_ptr<ScopedFile>> scopedfiles;
        scopedfiles.reserve(filemap.size());
        for (std::map<std::string, std::size_t>::const_iterator i = filemap.cbegin(); i != filemap.cend(); ++i)
            scopedfiles.emplace_back(new ScopedFile(i->first, data));

        // clear files list so only fileSettings are used
        if (useFS)
            filemap.clear();

        // TODO: test with settings.project.fileSettings;
        SingleExecutor executor(cppcheck, filemap, settings, settings.nomsg, *this);
        ASSERT_EQUALS(result, executor.check());
        ASSERT_EQUALS(opt.executeCommandCalled, executeCommandCalled);
        ASSERT_EQUALS(opt.exe, exe);
        ASSERT_EQUALS(opt.args.size(), args.size());
        for (int i = 0; i < args.size(); ++i)
        {
            ASSERT_EQUALS(opt.args[i], args[i]);
        }
    }

    void run() override {
        TEST_CASE(many_files);
        TEST_CASE(many_files_showtime);
        TEST_CASE(many_files_plist);
        TEST_CASE(no_errors_more_files);
        TEST_CASE(no_errors_less_files);
        TEST_CASE(no_errors_equal_amount_files);
        TEST_CASE(one_error_less_files);
        TEST_CASE(one_error_several_files);
        TEST_CASE(markup);
        TEST_CASE(clangTidy);
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
            expected += "Checking " + fprefix() + "_" + zpad3(i) + ".cpp ...\n";
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
              "}", dinit(CheckOptions, $.showtime = SHOWTIME_MODES::SHOWTIME_SUMMARY));
    }

    void many_files_plist() {
        const std::string plistOutput = "plist_" + fprefix() + "/";
        ScopedFile plistFile("dummy", "", plistOutput);

        check(100, 100,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}", dinit(CheckOptions, $.plistOutput = plistOutput.c_str()));
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
            fprefix() + "_1.cp1", fprefix() + "_2.cpp", fprefix() + "_3.cp1", fprefix() + "_4.cpp"
        };

        // checks are not executed on markup files => expected result is 2
        check(4, 2,
              "int main()\n"
              "{\n"
              "  char *a = malloc(10);\n"
              "  return 0;\n"
              "}",
              dinit(CheckOptions, $.filesList = files));
        // TODO: filter out the "files checked" messages
        ASSERT_EQUALS("Checking " + fprefix() + "_2.cpp ...\n"
                      "1/4 files checked 25% done\n"
                      "Checking " + fprefix() + "_4.cpp ...\n"
                      "2/4 files checked 50% done\n"
                      "Checking " + fprefix() + "_1.cp1 ...\n"
                      "3/4 files checked 75% done\n"
                      "Checking " + fprefix() + "_3.cp1 ...\n"
                      "4/4 files checked 100% done\n", output.str());
        settings = settingsOld;
    }

    void clangTidy() {
        // TODO: we currently only invoke it with ImportProject::FileSettings
        if (!useFS)
            return;

        const Settings settingsOld = settings;
        settings.clangTidy = true;

#ifdef _WIN32
        const char exe[] = "clang-tidy.exe";
#else
        const char exe[] = "clang-tidy";
#endif

        const std::string file = fprefix() + "_001.cpp";
        check(1, 0,
              "int main()\n"
              "{\n"
              "  return 0;\n"
              "}",
              dinit(CheckOptions,
                    $.executeCommandCalled = true,
                        $.exe = exe,
                        $.args = {"-quiet", "-checks=*,-clang-analyzer-*,-llvm*", file, "--"}));
        ASSERT_EQUALS("Checking " + file + " ...\n", output.str());
        settings = settingsOld;
    }

    // TODO: test whole program analysis
};

class TestSingleExecutorFiles : public TestSingleExecutorBase {
public:
    TestSingleExecutorFiles() : TestSingleExecutorBase("TestSingleExecutorFiles", false) {}
};

class TestSingleExecutorFS : public TestSingleExecutorBase {
public:
    TestSingleExecutorFS() : TestSingleExecutorBase("TestSingleExecutorFS", true) {}
};

REGISTER_TEST(TestSingleExecutorFiles)
REGISTER_TEST(TestSingleExecutorFS)

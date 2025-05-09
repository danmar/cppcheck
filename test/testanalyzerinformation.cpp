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


#include "analyzerinfo.h"
#include "filesettings.h"
#include "fixture.h"

#include <sstream>

class TestAnalyzerInformation : public TestFixture, private AnalyzerInformation {
public:
    TestAnalyzerInformation() : TestFixture("TestAnalyzerInformation") {}

private:

    void run() override {
        TEST_CASE(getAnalyzerInfoFile);
        TEST_CASE(duplicateFile);
        TEST_CASE(filesTextDuplicateFile);
        TEST_CASE(parse);
    }

    void getAnalyzerInfoFile() const {
        constexpr char filesTxt[] = "file1.a4:::file1.c\n";
        std::istringstream f1(filesTxt);
        ASSERT_EQUALS("file1.a4", getAnalyzerInfoFileFromFilesTxt(f1, "file1.c", "", 0));
        std::istringstream f2(filesTxt);
        ASSERT_EQUALS("file1.a4", getAnalyzerInfoFileFromFilesTxt(f2, "./file1.c", "", 0));
        ASSERT_EQUALS("builddir/file1.c.analyzerinfo", AnalyzerInformation::getAnalyzerInfoFile("builddir", "file1.c", "", 0));
        ASSERT_EQUALS("builddir/file1.c.analyzerinfo", AnalyzerInformation::getAnalyzerInfoFile("builddir", "some/path/file1.c", "", 0));
    }

    void filesTextDuplicateFile() const {
        std::list<FileSettings> fileSettings;
        fileSettings.emplace_back("a.c", Standards::Language::C, 10);
        fileSettings.back().fileIndex = 0;
        fileSettings.emplace_back("a.c", Standards::Language::C, 10);
        fileSettings.back().fileIndex = 1;

        const char expected[] = "a.a1:::a.c\n"
                                "a.a2::1:a.c\n";

        ASSERT_EQUALS(expected, getFilesTxt({}, "", fileSettings));
    }

    void duplicateFile() const {
        // same file duplicated
        constexpr char filesTxt[] = "file1.a1::1:file1.c\n"
                                    "file1.a2::2:file1.c\n";
        std::istringstream f1(filesTxt);
        ASSERT_EQUALS("file1.a1", getAnalyzerInfoFileFromFilesTxt(f1, "file1.c", "", 1));
        std::istringstream f2(filesTxt);
        ASSERT_EQUALS("file1.a2", getAnalyzerInfoFileFromFilesTxt(f2, "file1.c", "", 2));
    }

    void parse() const {
        AnalyzerInformation::Info info;

        ASSERT_EQUALS(false, info.parse("a"));
        ASSERT_EQUALS(false, info.parse("a:b"));
        ASSERT_EQUALS(false, info.parse("a:b:c"));
        ASSERT_EQUALS(false, info.parse("a:b:c:d"));

        ASSERT_EQUALS(true, info.parse("a:b::d"));
        ASSERT_EQUALS("a", info.afile);
        ASSERT_EQUALS("b", info.cfg);
        ASSERT_EQUALS(0, info.fileIndex);
        ASSERT_EQUALS("d", info.sourceFile);

        ASSERT_EQUALS(true, info.parse("e:f:12:g"));
        ASSERT_EQUALS("e", info.afile);
        ASSERT_EQUALS("f", info.cfg);
        ASSERT_EQUALS(12, info.fileIndex);
        ASSERT_EQUALS("g", info.sourceFile);

        ASSERT_EQUALS(true, info.parse("odr1.a1:::C:/dm/cppcheck-fix-13333/test/cli/whole-program/odr1.cpp"));
        ASSERT_EQUALS("odr1.a1", info.afile);
        ASSERT_EQUALS("", info.cfg);
        ASSERT_EQUALS(0, info.fileIndex);
        ASSERT_EQUALS("C:/dm/cppcheck-fix-13333/test/cli/whole-program/odr1.cpp", info.sourceFile);
    }
};

REGISTER_TEST(TestAnalyzerInformation)

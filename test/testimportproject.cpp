/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "importproject.h"
#include "testsuite.h"

#include <list>
#include <map>
#include <string>

class TestImporter : public ImportProject {
public:
    void importCompileCommands(std::istream &istr) {
        ImportProject::importCompileCommands(istr);
    }
};


class TestImportProject : public TestFixture {
public:
    TestImportProject() : TestFixture("TestImportProject") {
    }

private:

    void run() override {
        TEST_CASE(setDefines);
        TEST_CASE(setIncludePaths1);
        TEST_CASE(setIncludePaths2);
        TEST_CASE(setIncludePaths3); // macro names are case insensitive
        TEST_CASE(importCompileCommands);
    }

    void setDefines() const {
        ImportProject::FileSettings fs;

        fs.setDefines("A");
        ASSERT_EQUALS("A=1", fs.defines);

        fs.setDefines("A;B;");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        fs.setDefines("A;;B;");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        fs.setDefines("A;;B");
        ASSERT_EQUALS("A=1;B=1", fs.defines);
    }

    void setIncludePaths1() const {
        ImportProject::FileSettings fs;
        std::list<std::string> in(1, "../include");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        fs.setIncludePaths("abc/def/", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("abc/include/", fs.includePaths.front());
    }

    void setIncludePaths2() const {
        ImportProject::FileSettings fs;
        std::list<std::string> in(1, "$(SolutionDir)other");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        variables["SolutionDir"] = "c:/abc/";
        fs.setIncludePaths("/home/fred", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void setIncludePaths3() const { // macro names are case insensitive
        ImportProject::FileSettings fs;
        std::list<std::string> in(1, "$(SOLUTIONDIR)other");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        variables["SolutionDir"] = "c:/abc/";
        fs.setIncludePaths("/home/fred", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void importCompileCommands() const {

        const char json[] = "[ { \"directory\": \"/tmp\","
                            "\"command\": \"gcc -I/tmp -DCFGDIR=\\\\\\\"/usr/local/share/Cppcheck\\\\\\\" -DTEST1 -DTEST2=2 -o /tmp/src.o -c /tmp/src.c\","
                            "\"file\": \"/tmp/src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        importer.importCompileCommands(istr);
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("CFGDIR=\"/usr/local/share/Cppcheck\";TEST1=1;TEST2=2", importer.fileSettings.begin()->defines);
    }
};

REGISTER_TEST(TestImportProject)

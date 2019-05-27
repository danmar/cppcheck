/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#include "settings.h"
#include "testsuite.h"

#include <list>
#include <map>
#include <string>

class TestImporter : public ImportProject {
public:
    using ImportProject::importCompileCommands;
    using ImportProject::importCppcheckGuiProject;
};


class TestImportProject : public TestFixture {
public:
    TestImportProject() : TestFixture("TestImportProject") {
    }

private:

    void run() OVERRIDE {
        TEST_CASE(setDefines);
        TEST_CASE(setIncludePaths1);
        TEST_CASE(setIncludePaths2);
        TEST_CASE(setIncludePaths3); // macro names are case insensitive
        TEST_CASE(importCompileCommands1);
        TEST_CASE(importCompileCommands2); // #8563
        TEST_CASE(importCompileCommands3); // check with existing trailing / in directory
        TEST_CASE(importCompileCommands4); // only accept certain file types
        TEST_CASE(importCompileCommandsArgumentsSection); // Handle arguments section
        TEST_CASE(importCompileCommandsNoCommandSection); // gracefully handles malformed json
        TEST_CASE(importCppcheckGuiProject);
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

    void importCompileCommands1() const {
        const char json[] = "[ { \"directory\": \"/tmp\","
                            "\"command\": \"gcc -I/tmp -DCFGDIR=\\\\\\\"/usr/local/share/Cppcheck\\\\\\\" -DTEST1 -DTEST2=2 -o /tmp/src.o -c /tmp/src.c\","
                            "\"file\": \"/tmp/src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        importer.importCompileCommands(istr);
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("CFGDIR=\"/usr/local/share/Cppcheck\";TEST1=1;TEST2=2", importer.fileSettings.begin()->defines);
    }

    void importCompileCommands2() const {
        const char json[] = "[ { \"directory\": \"/tmp\","
                            "\"command\": \"gcc -c src.c\","
                            "\"file\": \"src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        importer.importCompileCommands(istr);
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.begin()->filename);
    }

    void importCompileCommands3() const {
        const char json[] = "[ { \"directory\": \"/tmp/\","
                            "\"command\": \"gcc -c src.c\","
                            "\"file\": \"src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        importer.importCompileCommands(istr);
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.begin()->filename);
    }

    void importCompileCommands4() const {
        const char json[] = "[ { \"directory\": \"/tmp/\","
                            "\"command\": \"gcc -c src.mm\","
                            "\"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        importer.importCompileCommands(istr);
        ASSERT_EQUALS(0, importer.fileSettings.size());
    }

    void importCompileCommandsArgumentsSection() const {
        const char json[] = "[ { \"directory\": \"/tmp/\","
                            "\"arguments\": [\"gcc\", \"-c\", \"src.c\"],"
                            "\"file\": \"src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        importer.importCompileCommands(istr);
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.begin()->filename);
    }

    void importCompileCommandsNoCommandSection() const {
        const char json[] = "[ { \"directory\": \"/tmp/\","
                            "\"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        importer.importCompileCommands(istr);
        ASSERT_EQUALS(0, importer.fileSettings.size());
    }

    void importCppcheckGuiProject() const {
        const char xml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<project version=\"1\">\n"
                           "    <root name=\".\"/>\n"
                           "    <builddir>out1</builddir>\n"
                           "    <analyze-all-vs-configs>true</analyze-all-vs-configs>\n"
                           "    <includedir>\n"
                           "        <dir name=\"lib/\"/>\n"
                           "    </includedir>\n"
                           "    <paths>\n"
                           "        <dir name=\"cli/\"/>\n"
                           "    </paths>\n"
                           "    <exclude>\n"
                           "        <path name=\"gui/temp/\"/>\n"
                           "    </exclude>\n"
                           "</project>\n";
        std::istringstream istr(xml);
        Settings s;
        TestImporter project;
        ASSERT_EQUALS(true, project.importCppcheckGuiProject(istr, &s));
        ASSERT_EQUALS(1, project.guiProject.pathNames.size());
        ASSERT_EQUALS("cli/", project.guiProject.pathNames[0]);
        ASSERT_EQUALS(1, s.includePaths.size());
        ASSERT_EQUALS("lib/", s.includePaths.front());
    }
};

REGISTER_TEST(TestImportProject)

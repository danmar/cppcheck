/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include "testprojectfile.h"

#include "importproject.h"
#include "library.h"
#include "platform.h"
#include "projectfile.h"
#include "settings.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <QList>
#include <QStringList>
#include <QtTest>

// Mock...
const char Settings::SafeChecks::XmlRootName[] = "safe-checks";
const char Settings::SafeChecks::XmlClasses[] = "class-public";
const char Settings::SafeChecks::XmlExternalFunctions[] = "external-functions";
const char Settings::SafeChecks::XmlInternalFunctions[] = "internal-functions";
const char Settings::SafeChecks::XmlExternalVariables[] = "external-variables";
Settings::Settings() : maxCtuDepth(10) {}
cppcheck::Platform::Platform() = default;
ImportProject::ImportProject() = default;
bool ImportProject::sourceFileExists(const std::string & /*file*/) {
    return true;
}

void TestProjectFile::loadInexisting() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/foo.cppcheck");
    ProjectFile pfile(filepath);
    QCOMPARE(pfile.read(), false);
}

void TestProjectFile::loadSimple() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/simple.cppcheck");
    ProjectFile pfile(filepath);
    QVERIFY(pfile.read());
    QCOMPARE(pfile.getRootPath(), QString("../.."));
    QStringList includes = pfile.getIncludeDirs();
    QCOMPARE(includes.size(), 2);
    QCOMPARE(includes[0], QString("lib/"));
    QCOMPARE(includes[1], QString("cli/"));
    QStringList paths = pfile.getCheckPaths();
    QCOMPARE(paths.size(), 2);
    QCOMPARE(paths[0], QString("gui/"));
    QCOMPARE(paths[1], QString("test/"));
    QStringList excludes = pfile.getExcludedPaths();
    QCOMPARE(excludes.size(), 1);
    QCOMPARE(excludes[0], QString("gui/temp/"));
    QStringList defines = pfile.getDefines();
    QCOMPARE(defines.size(), 1);
    QCOMPARE(defines[0], QString("FOO"));
}

// Test that project file with old 'ignore' element works
void TestProjectFile::loadSimpleWithIgnore() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/simple_ignore.cppcheck");
    ProjectFile pfile(filepath);
    QVERIFY(pfile.read());
    QCOMPARE(pfile.getRootPath(), QString("../.."));
    QStringList includes = pfile.getIncludeDirs();
    QCOMPARE(includes.size(), 2);
    QCOMPARE(includes[0], QString("lib/"));
    QCOMPARE(includes[1], QString("cli/"));
    QStringList paths = pfile.getCheckPaths();
    QCOMPARE(paths.size(), 2);
    QCOMPARE(paths[0], QString("gui/"));
    QCOMPARE(paths[1], QString("test/"));
    QStringList excludes = pfile.getExcludedPaths();
    QCOMPARE(excludes.size(), 1);
    QCOMPARE(excludes[0], QString("gui/temp/"));
    QStringList defines = pfile.getDefines();
    QCOMPARE(defines.size(), 1);
    QCOMPARE(defines[0], QString("FOO"));
}

void TestProjectFile::loadSimpleNoroot() const
{
    const QString filepath(QString(SRCDIR) + "/../data/projectfiles/simple_noroot.cppcheck");
    ProjectFile pfile(filepath);
    QVERIFY(pfile.read());
    QCOMPARE(pfile.getRootPath(), QString());
    QStringList includes = pfile.getIncludeDirs();
    QCOMPARE(includes.size(), 2);
    QCOMPARE(includes[0], QString("lib/"));
    QCOMPARE(includes[1], QString("cli/"));
    QStringList paths = pfile.getCheckPaths();
    QCOMPARE(paths.size(), 2);
    QCOMPARE(paths[0], QString("gui/"));
    QCOMPARE(paths[1], QString("test/"));
    QStringList excludes = pfile.getExcludedPaths();
    QCOMPARE(excludes.size(), 1);
    QCOMPARE(excludes[0], QString("gui/temp/"));
    QStringList defines = pfile.getDefines();
    QCOMPARE(defines.size(), 1);
    QCOMPARE(defines[0], QString("FOO"));
}

QTEST_MAIN(TestProjectFile)

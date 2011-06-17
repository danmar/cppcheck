/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#include <QtTest>
#include <QObject>
#include "testfilelist.h"
#include "filelist.h"

void TestFileList::addFile()
{
    // Accepted extensions: *.cpp, *.cxx, *.cc, *.c, *.c++, *.txx, *.tpp"
    FileList list;
    list.AddFile(QString(SRCDIR) + "/../data/files/foo1.cpp");
    list.AddFile(QString(SRCDIR) + "/../data/files/foo2.cxx");
    list.AddFile(QString(SRCDIR) + "/../data/files/foo3.cc");
    list.AddFile(QString(SRCDIR) + "/../data/files/foo4.c");
    list.AddFile(QString(SRCDIR) + "/../data/files/foo5.c++");
    list.AddFile(QString(SRCDIR) + "/../data/files/foo6.txx");
    list.AddFile(QString(SRCDIR) + "/../data/files/foo7.tpp");
    QStringList files = list.GetFileList();
    QCOMPARE(files.size(), 7);
}

void TestFileList::addFile_notexist()
{
    FileList list;
    list.AddFile(QString(SRCDIR) + "/../data/files/bar1.cpp");
    QStringList files = list.GetFileList();
    QCOMPARE(files.size(), 0);
}

void TestFileList::addFile_unknown()
{
    FileList list;
    list.AddFile(QString(SRCDIR) + "/../data/files/bar1");
    list.AddFile(QString(SRCDIR) + "/../data/files/bar1.foo");
    QStringList files = list.GetFileList();
    QCOMPARE(files.size(), 0);
}


QTEST_MAIN(TestFileList)

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
#include "testxmlreportv2.h"
#include "xmlreportv2.h"
#include "erroritem.h"
#include "errorlogger.h"

void TestXmlReportV2::readXml()
{
    const QString filepath(QString(SRCDIR) + "/../data/xmlfiles/xmlreport_v2.xml");
    XmlReportV2 report(filepath);
    QVERIFY(report.Open());
    QList<ErrorItem> errors = report.Read();
    QCOMPARE(errors.size(), 6);

    ErrorItem item = errors[0];
    QCOMPARE(item.file, QString("test.cxx"));
    QCOMPARE(item.files.size(), 1);
    QCOMPARE(item.lines.size(), 1);
    QCOMPARE(item.files[0], QString("test.cxx"));
    QCOMPARE(item.lines[0], (unsigned int)11);
    QCOMPARE(item.errorId, QString("unreadVariable"));
    QCOMPARE(GuiSeverity::toString(item.severity), QString("style"));
    QCOMPARE(item.summary, QString("Variable 'a' is assigned a value that is never used"));
    QCOMPARE(item.message, QString("Variable 'a' is assigned a value that is never used"));

    ErrorItem item2 = errors[3];
    QCOMPARE(item2.file, QString("test.cxx"));
    QCOMPARE(item2.files.size(), 2);
    QCOMPARE(item2.lines.size(), 2);
    QCOMPARE(item2.files[0], QString("test.cxx"));
    QCOMPARE(item2.lines[0], (unsigned int)32);
    QCOMPARE(item2.files[1], QString("test.cxx"));
    QCOMPARE(item2.lines[1], (unsigned int)16);
    QCOMPARE(item2.errorId, QString("mismatchAllocDealloc"));
    QCOMPARE(GuiSeverity::toString(item2.severity), QString("error"));
    QCOMPARE(item2.summary, QString("Mismatching allocation and deallocation: k"));
    QCOMPARE(item2.message, QString("Mismatching allocation and deallocation: k"));
}

QTEST_MAIN(TestXmlReportV2)

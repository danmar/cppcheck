/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "testxmlreportv1.h"
#include "xmlreportv1.h"
#include "erroritem.h"
#include "errorlogger.h"

void TestXmlReportV1::readXml()
{
    const QString filepath(QString(SRCDIR) + "/../data/xmlfiles/xmlreport_v1.xml");
    XmlReportV1 report(filepath);
    QVERIFY(report.Open());
    QList<ErrorItem> errors = report.Read();
    QCOMPARE(errors.size(), 6);

    ErrorItem item = errors[0];
    QCOMPARE(item.file, QString("test.cxx"));
    QCOMPARE(item.lines[0], (unsigned int)11);
    QCOMPARE(item.errorId, QString("unreadVariable"));
    QCOMPARE(GuiSeverity::toString(item.severity), QString("style"));
    QCOMPARE(item.summary, QString("Variable 'a' is assigned a value that is never used"));
    QCOMPARE(item.message, QString("Variable 'a' is assigned a value that is never used"));
}

QTEST_MAIN(TestXmlReportV1)

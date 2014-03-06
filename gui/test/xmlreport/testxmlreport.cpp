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
#include "testxmlreport.h"
#include "xmlreport.h"

void TestXmlReport::testQuoteMessage()
{
    const QString toQuote("abcdefgh&\"'<>12345");
    const QString quoted("abcdefgh&amp;&quot;&#039;&lt;&gt;12345");
    QCOMPARE(XmlReport::quoteMessage(toQuote), quoted);
}

void TestXmlReport::testUnquoteMessage()
{
    const QString toQuote("abcdefgh&\"'<>12345");
    const QString quoted("abcdefgh&amp;&quot;&#039;&lt;&gt;12345");
    QCOMPARE(XmlReport::unquoteMessage(quoted), toQuote);
}

void TestXmlReport::testGetVersion1()
{
    const QString filepath(QString(SRCDIR) + "/../data/xmlfiles/xmlreport_v1.xml");
    QCOMPARE(XmlReport::determineVersion(filepath), 1);
}

void TestXmlReport::testGetVersion2()
{
    const QString filepath(QString(SRCDIR) + "/../data/xmlfiles/xmlreport_v2.xml");
    QCOMPARE(XmlReport::determineVersion(filepath), 2);
}

QTEST_MAIN(TestXmlReport)

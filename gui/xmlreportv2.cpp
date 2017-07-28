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

#include <QObject>
#include <QString>
#include <QList>
#include <QDir>
#include <QXmlStreamWriter>
#include <QDebug>
#include "report.h"
#include "erroritem.h"
#include "xmlreport.h"
#include "xmlreportv2.h"
#include "cppcheck.h"

static const char ResultElementName[] = "results";
static const char CppcheckElementName[] = "cppcheck";
static const char ErrorElementName[] = "error";
static const char ErrorsElementName[] = "errors";
static const char LocationElementName[] = "location";
static const char ColAttribute[] = "col";
static const char CWEAttribute[] = "cwe";
static const char FilenameAttribute[] = "file";
static const char IncludedFromFilenameAttribute[] = "file0";
static const char InconclusiveAttribute[] = "inconclusive";
static const char InfoAttribute[] = "info";
static const char LineAttribute[] = "line";
static const char IdAttribute[] = "id";
static const char SeverityAttribute[] = "severity";
static const char MsgAttribute[] = "msg";
static const char VersionAttribute[] = "version";
static const char VerboseAttribute[] = "verbose";

XmlReportV2::XmlReportV2(const QString &filename) :
    XmlReport(filename),
    mXmlReader(NULL),
    mXmlWriter(NULL)
{
}

XmlReportV2::~XmlReportV2()
{
    delete mXmlReader;
    delete mXmlWriter;
}

bool XmlReportV2::create()
{
    if (Report::create()) {
        mXmlWriter = new QXmlStreamWriter(Report::getFile());
        return true;
    }
    return false;
}

bool XmlReportV2::open()
{
    if (Report::open()) {
        mXmlReader = new QXmlStreamReader(Report::getFile());
        return true;
    }
    return false;
}

void XmlReportV2::writeHeader()
{
    mXmlWriter->setAutoFormatting(true);
    mXmlWriter->writeStartDocument();
    mXmlWriter->writeStartElement(ResultElementName);
    mXmlWriter->writeAttribute(VersionAttribute, QString::number(2));
    mXmlWriter->writeStartElement(CppcheckElementName);
    mXmlWriter->writeAttribute(VersionAttribute, QString(CppCheck::version()));
    mXmlWriter->writeEndElement();
    mXmlWriter->writeStartElement(ErrorsElementName);
}

void XmlReportV2::writeFooter()
{
    mXmlWriter->writeEndElement(); // errors
    mXmlWriter->writeEndElement(); // results
    mXmlWriter->writeEndDocument();
}

void XmlReportV2::writeError(const ErrorItem &error)
{
    /*
    Error example from the core program in xml
    <error id="mismatchAllocDealloc" severity="error" msg="Mismatching allocation and deallocation: k"
              verbose="Mismatching allocation and deallocation: k">
      <location file="..\..\test\test.cxx" line="16"/>
      <location file="..\..\test\test.cxx" line="32"/>
    </error>
    */

    mXmlWriter->writeStartElement(ErrorElementName);
    mXmlWriter->writeAttribute(IdAttribute, error.errorId);

    // Don't localize severity so we can read these files
    mXmlWriter->writeAttribute(SeverityAttribute, GuiSeverity::toString(error.severity));
    const QString summary = XmlReport::quoteMessage(error.summary);
    mXmlWriter->writeAttribute(MsgAttribute, summary);
    const QString message = XmlReport::quoteMessage(error.message);
    mXmlWriter->writeAttribute(VerboseAttribute, message);
    if (error.inconclusive)
        mXmlWriter->writeAttribute(InconclusiveAttribute, "true");
    if (error.cwe > 0)
        mXmlWriter->writeAttribute(CWEAttribute, QString::number(error.cwe));

    for (int i = error.errorPath.count() - 1; i >= 0; i--) {
        mXmlWriter->writeStartElement(LocationElementName);

        QString file = QDir::toNativeSeparators(error.errorPath[i].file);
        if (!error.file0.isEmpty() && file != error.file0) {
            mXmlWriter->writeAttribute(IncludedFromFilenameAttribute, quoteMessage(error.file0));
        }
        mXmlWriter->writeAttribute(FilenameAttribute, XmlReport::quoteMessage(file));
        mXmlWriter->writeAttribute(LineAttribute, QString::number(error.errorPath[i].line));
        if (error.errorPath[i].col > 0)
            mXmlWriter->writeAttribute(ColAttribute, QString::number(error.errorPath[i].col));
        if (error.errorPath.count() > 1)
            mXmlWriter->writeAttribute(InfoAttribute, XmlReport::quoteMessage(error.errorPath[i].info));

        mXmlWriter->writeEndElement();
    }

    mXmlWriter->writeEndElement();
}

QList<ErrorItem> XmlReportV2::read()
{
    QList<ErrorItem> errors;
    bool insideResults = false;
    if (!mXmlReader) {
        qDebug() << "You must Open() the file before reading it!";
        return errors;
    }
    while (!mXmlReader->atEnd()) {
        switch (mXmlReader->readNext()) {
        case QXmlStreamReader::StartElement:
            if (mXmlReader->name() == ResultElementName)
                insideResults = true;

            // Read error element from inside result element
            if (insideResults && mXmlReader->name() == ErrorElementName) {
                ErrorItem item = readError(mXmlReader);
                errors.append(item);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (mXmlReader->name() == ResultElementName)
                insideResults = false;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    }
    return errors;
}

ErrorItem XmlReportV2::readError(QXmlStreamReader *reader)
{
    /*
    Error example from the core program in xml
    <error id="mismatchAllocDealloc" severity="error" msg="Mismatching allocation and deallocation: k"
              verbose="Mismatching allocation and deallocation: k">
      <location file="..\..\test\test.cxx" line="16"/>
      <location file="..\..\test\test.cxx" line="32"/>
    </error>
    */

    ErrorItem item;

    // Read error element from inside errors element
    if (mXmlReader->name() == ErrorElementName) {
        QXmlStreamAttributes attribs = reader->attributes();
        item.errorId = attribs.value("", IdAttribute).toString();
        item.severity = GuiSeverity::fromString(attribs.value("", SeverityAttribute).toString());
        const QString summary = attribs.value("", MsgAttribute).toString();
        item.summary = XmlReport::unquoteMessage(summary);
        const QString message = attribs.value("", VerboseAttribute).toString();
        item.message = XmlReport::unquoteMessage(message);
        if (attribs.hasAttribute("", InconclusiveAttribute))
            item.inconclusive = true;
        if (attribs.hasAttribute("", CWEAttribute))
            item.cwe = attribs.value("", CWEAttribute).toString().toInt();
    }

    bool errorRead = false;
    while (!errorRead && !mXmlReader->atEnd()) {
        switch (mXmlReader->readNext()) {
        case QXmlStreamReader::StartElement:

            // Read location element from inside error element
            if (mXmlReader->name() == LocationElementName) {
                QXmlStreamAttributes attribs = mXmlReader->attributes();
                QString file0 = attribs.value("", IncludedFromFilenameAttribute).toString();
                if (!file0.isEmpty())
                    item.file0 = XmlReport::unquoteMessage(file0);
                QErrorPathItem loc;
                loc.file = XmlReport::unquoteMessage(attribs.value("", FilenameAttribute).toString());
                loc.line = attribs.value("", LineAttribute).toString().toUInt();
                if (attribs.hasAttribute("", ColAttribute))
                    loc.col = attribs.value("", ColAttribute).toString().toInt();
                if (attribs.hasAttribute("", InfoAttribute))
                    loc.info = XmlReport::unquoteMessage(attribs.value("", InfoAttribute).toString());
                item.errorPath.push_front(loc);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (mXmlReader->name() == ErrorElementName)
                errorRead = true;
            break;

        // Not handled
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Characters:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    }
    return item;
}

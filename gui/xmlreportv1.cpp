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
#include "xmlreportv1.h"

static const char ResultElementName[] = "results";
static const char ErrorElementName[] = "error";
static const char FilenameAttribute[] = "file";
static const char LineAttribute[] = "line";
static const char IdAttribute[] = "id";
static const char SeverityAttribute[] = "severity";
static const char MsgAttribute[] = "msg";

XmlReportV1::XmlReportV1(const QString &filename) :
    XmlReport(filename),
    mXmlReader(NULL),
    mXmlWriter(NULL)
{
}

XmlReportV1::~XmlReportV1()
{
    delete mXmlReader;
    delete mXmlWriter;
}

bool XmlReportV1::Create()
{
    if (Report::Create()) {
        mXmlWriter = new QXmlStreamWriter(Report::GetFile());
        return true;
    }
    return false;
}

bool XmlReportV1::Open()
{
    if (Report::Open()) {
        mXmlReader = new QXmlStreamReader(Report::GetFile());
        return true;
    }
    return false;
}

void XmlReportV1::WriteHeader()
{
    mXmlWriter->setAutoFormatting(true);
    mXmlWriter->writeStartDocument();
    mXmlWriter->writeStartElement(ResultElementName);
}

void XmlReportV1::WriteFooter()
{
    mXmlWriter->writeEndElement();
    mXmlWriter->writeEndDocument();
}

void XmlReportV1::WriteError(const ErrorItem &error)
{
    /*
    Error example from the core program in xml
    <error file="gui/test.cpp" line="14" id="mismatchAllocDealloc" severity="error" msg="Mismatching allocation and deallocation: k"/>
    The callstack seems to be ignored here as well, instead last item of the stack is used
    */

    // Don't write inconclusive errors to XML V1
    if (error.inconclusive)
        return;

    mXmlWriter->writeStartElement(ErrorElementName);
    QString file = QDir::toNativeSeparators(error.files[error.files.size() - 1]);
    file = XmlReport::quoteMessage(file);
    mXmlWriter->writeAttribute(FilenameAttribute, file);
    const QString line = QString::number(error.lines[error.lines.size() - 1]);
    mXmlWriter->writeAttribute(LineAttribute, line);
    mXmlWriter->writeAttribute(IdAttribute, error.errorId);

    // Don't localize severity so we can read these files
    mXmlWriter->writeAttribute(SeverityAttribute, GuiSeverity::toString(error.severity));
    const QString message = XmlReport::quoteMessage(error.message);
    mXmlWriter->writeAttribute(MsgAttribute, message);
    mXmlWriter->writeEndElement();
}

QList<ErrorItem> XmlReportV1::Read()
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
                ErrorItem item = ReadError(mXmlReader);
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

ErrorItem XmlReportV1::ReadError(QXmlStreamReader *reader)
{
    ErrorItem item;
    if (reader->name().toString() == ErrorElementName) {
        QXmlStreamAttributes attribs = reader->attributes();
        QString file = attribs.value("", FilenameAttribute).toString();
        file = XmlReport::unquoteMessage(file);
        item.file = file;
        item.files.push_back(file);
        const int line = attribs.value("", LineAttribute).toString().toUInt();
        item.lines.push_back(line);
        item.errorId = attribs.value("", IdAttribute).toString();
        item.severity = GuiSeverity::fromString(attribs.value("", SeverityAttribute).toString());

        // NOTE: This duplicates the message to Summary-field. But since
        // old XML format doesn't have separate summary and verbose messages
        // we must add same message to both data so it shows up in GUI.
        // Check if there is full stop and cut the summary to it.
        QString summary = attribs.value("", MsgAttribute).toString();
        const int ind = summary.indexOf('.');
        if (ind != -1)
            summary = summary.left(ind + 1);
        item.summary = XmlReport::unquoteMessage(summary);
        QString message = attribs.value("", MsgAttribute).toString();
        item.message = XmlReport::unquoteMessage(message);
    }
    return item;
}

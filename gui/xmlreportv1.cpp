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

#include <QObject>
#include <QString>
#include <QList>
#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>
#include "report.h"
#include "erroritem.h"
#include "xmlreportv1.h"

static const char ResultElementName[] = "results";
static const char ErrorElementName[] = "error";
static const char FilenameAttribute[] = "file";
static const char LineAttribute[] = "line";
static const char IdAttribute[] = "id";
static const char SeverityAttribute[] = "severity";
static const char MsgAttribute[] = "msg";

XmlReportV1::XmlReportV1(const QString &filename, QObject * parent) :
    XmlReport(filename, parent),
    mXmlReader(NULL),
    mXmlWriter(NULL)
{
}

XmlReportV1::~XmlReportV1()
{
    delete mXmlReader;
    delete mXmlWriter;
    Close();
}

bool XmlReportV1::Create()
{
    bool success = false;
    if (Report::Create())
    {
        mXmlWriter = new QXmlStreamWriter(Report::GetFile());
        success = true;
    }
    return success;
}

bool XmlReportV1::Open()
{
    bool success = false;
    if (Report::Open())
    {
        mXmlReader = new QXmlStreamReader(Report::GetFile());
        success = true;
    }
    return success;
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

    mXmlWriter->writeStartElement(ErrorElementName);
    const QString file = QDir::toNativeSeparators(error.files[error.files.size() - 1]);
    mXmlWriter->writeAttribute(FilenameAttribute, file);
    const QString line = QString::number(error.lines[error.lines.size() - 1]);
    mXmlWriter->writeAttribute(LineAttribute, line);
    mXmlWriter->writeAttribute(IdAttribute, error.id);
    mXmlWriter->writeAttribute(SeverityAttribute, error.severity);
    mXmlWriter->writeAttribute(MsgAttribute, error.message);
    mXmlWriter->writeEndElement();
}

QList<ErrorLine> XmlReportV1::Read()
{
    QList<ErrorLine> errors;
    bool insideResults = false;
    if (!mXmlReader)
    {
        qDebug() << "You must Open() the file before reading it!";
        return errors;
    }
    while (!mXmlReader->atEnd())
    {
        switch (mXmlReader->readNext())
        {
        case QXmlStreamReader::StartElement:
            if (mXmlReader->name() == ResultElementName)
                insideResults = true;

            // Read error element from inside result element
            if (insideResults && mXmlReader->name() == ErrorElementName)
            {
                ErrorLine line = ReadError(mXmlReader);
                errors.append(line);
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

ErrorLine XmlReportV1::ReadError(QXmlStreamReader *reader)
{
    ErrorLine line;
    if (reader->name().toString() == ErrorElementName)
    {
        QXmlStreamAttributes attribs = reader->attributes();
        line.file = attribs.value("", FilenameAttribute).toString();
        line.line = attribs.value("", LineAttribute).toString().toUInt();
        line.id = attribs.value("", IdAttribute).toString();
        line.severity = attribs.value("", SeverityAttribute).toString();

        // NOTE: This dublicates the message to Summary-field. But since
        // old XML format doesn't have separate summary and verbose messages
        // we must add same message to both data so it shows up in GUI.
        // Check if there is full stop and cut the summary to it.
        QString summary = attribs.value("", MsgAttribute).toString();
        const int ind = summary.indexOf('.');
        if (ind != -1)
            summary = summary.left(ind + 1);
        line.summary = summary;
        line.message = attribs.value("", MsgAttribute).toString();
    }
    return line;
}

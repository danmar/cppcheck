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
#include "xmlreportv2.h"
#include "cppcheck.h"

static const char ResultElementName[] = "results";
static const char CppcheckElementName[] = "cppcheck";
static const char ErrorElementName[] = "error";
static const char ErrorsElementName[] = "errors";
static const char LocationElementName[] = "location";
static const char FilenameAttribute[] = "file";
static const char LineAttribute[] = "line";
static const char IdAttribute[] = "id";
static const char SeverityAttribute[] = "severity";
static const char MsgAttribute[] = "msg";
static const char VersionAttribute[] = "version";
static const char VerboseAttribute[] = "verbose";

XmlReportV2::XmlReportV2(const QString &filename, QObject * parent) :
    Report(filename, parent),
    mXmlReader(NULL),
    mXmlWriter(NULL)
{
}

XmlReportV2::~XmlReportV2()
{
    delete mXmlReader;
    delete mXmlWriter;
    Close();
}

bool XmlReportV2::Create()
{
    bool success = false;
    if (Report::Create())
    {
        mXmlWriter = new QXmlStreamWriter(Report::GetFile());
        success = true;
    }
    return success;
}

bool XmlReportV2::Open()
{
    bool success = false;
    if (Report::Open())
    {
        mXmlReader = new QXmlStreamReader(Report::GetFile());
        success = true;
    }
    return success;
}

void XmlReportV2::WriteHeader()
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

void XmlReportV2::WriteFooter()
{
    mXmlWriter->writeEndElement(); // errors
    mXmlWriter->writeEndElement(); // results
    mXmlWriter->writeEndDocument();
}

void XmlReportV2::WriteError(const ErrorItem &error)
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
    mXmlWriter->writeAttribute(IdAttribute, error.id);
    mXmlWriter->writeAttribute(SeverityAttribute, error.severity);
    mXmlWriter->writeAttribute(MsgAttribute, error.summary);
    mXmlWriter->writeAttribute(VerboseAttribute, error.message);

    for (int i = 0; i < error.files.count(); i++)
    {
        mXmlWriter->writeStartElement(LocationElementName);

        const QString file = QDir::toNativeSeparators(error.files[i]);
        mXmlWriter->writeAttribute(FilenameAttribute, file);
        const QString line = QString::number(error.lines[i]);
        mXmlWriter->writeAttribute(LineAttribute, line);

        mXmlWriter->writeEndElement();
    }

    mXmlWriter->writeEndElement();
}

QList<ErrorLine> XmlReportV2::Read()
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

ErrorLine XmlReportV2::ReadError(QXmlStreamReader *reader)
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

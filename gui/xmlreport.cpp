/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

#include <QFile>
#include <QXmlStreamWriter>
#include "xmlreport.h"

XmlReport::XmlReport(const QString &filename, QObject * parent) :
    Report(filename, parent),
    mXmlWriter(NULL)
{
}

XmlReport::~XmlReport()
{
    delete mXmlWriter;
    Close();
}

bool XmlReport::Create()
{
    bool success = false;
    if (Report::Create())
    {
        mXmlWriter = new QXmlStreamWriter(Report::GetFile());
        success = true;
    }
    return success;
}

void XmlReport::WriteHeader()
{
    mXmlWriter->setAutoFormatting(true);
    mXmlWriter->writeStartDocument();
    mXmlWriter->writeStartElement("results");
}

void XmlReport::WriteFooter()
{
    mXmlWriter->writeEndElement();
    mXmlWriter->writeEndDocument();
}

void XmlReport::WriteError(const QStringList &files, const QStringList &lines,
                           const QString &id, const QString &severity, const QString &msg)
{
    /*
    Error example from the core program in xml
    <error file="gui/test.cpp" line="14" id="mismatchAllocDealloc" severity="error" msg="Mismatching allocation and deallocation: k"/>
    The callstack seems to be ignored here aswell, instead last item of the stack is used
    */

    mXmlWriter->writeStartElement("error");
    mXmlWriter->writeAttribute("file", files[files.size() - 1]);
    mXmlWriter->writeAttribute("line", lines[lines.size() - 1]);
    mXmlWriter->writeAttribute("id", id);
    mXmlWriter->writeAttribute("severity", severity);
    mXmlWriter->writeAttribute("msg", msg);
    mXmlWriter->writeEndElement();
}

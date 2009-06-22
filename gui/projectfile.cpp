/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QFile>
#include "projectfile.h"

static const char ProjectElementName[] = "project";
static const char AllocElementName[] = "autodealloc";
static const char ClassElementName[] = "class";
static const char ClassNameAttrib[] = "name";

ProjectFile::ProjectFile(QObject *parent) :
        QObject(parent)
{
}

ProjectFile::ProjectFile(const QString &filename, QObject *parent) :
        QObject(parent),
        mFilename(filename)
{
}

bool ProjectFile::Read(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    QFile file(mFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QXmlStreamReader xmlReader(&file);
    bool insideProject = false;
    while (!xmlReader.atEnd())
    {
        switch (xmlReader.readNext())
        {
        case QXmlStreamReader::StartElement:
            if (xmlReader.name() == ProjectElementName)
                insideProject = true;
            if (insideProject && xmlReader.name() == AllocElementName)
                ReadAutoAllocClasses(xmlReader);
            break;

        case QXmlStreamReader::EndElement:
            if (xmlReader.name() == ProjectElementName)
                insideProject = false;
            break;
        }
    }

    file.close();
    return true;
}

QStringList ProjectFile::GetDeAllocatedClasses() const
{
    return mDeAllocatedClasses;
}

void ProjectFile::ReadAutoAllocClasses(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do
    {
        type = reader.readNext();
        switch (type)
        {
        case QXmlStreamReader::StartElement:
            if (reader.name().toString() == ClassElementName)
            {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value("", ClassNameAttrib).toString();
                if (!name.isEmpty())
                    mDeAllocatedClasses << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == AllocElementName)
                allRead = true;
            break;
        }

    }
    while (!allRead);
}

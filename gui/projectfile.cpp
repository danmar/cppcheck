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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include "projectfile.h"

static const char ProjectElementName[] = "project";
static const char ProjectVersionAttrib[] = "version";
static const char ProjectFileVersion[] = "1";
static const char BuildDirElementName[] = "builddir";
static const char ImportProjectElementName[] = "importproject";
static const char IncludeDirElementName[] = "includedir";
static const char DirElementName[] = "dir";
static const char DirNameAttrib[] = "name";
static const char DefinesElementName[] = "defines";
static const char DefineName[] = "define";
static const char DefineNameAttrib[] = "name";
static const char PathsElementName[] = "paths";
static const char PathName[] = "dir";
static const char PathNameAttrib[] = "name";
static const char RootPathName[] = "root";
static const char RootPathNameAttrib[] = "name";
static const char IgnoreElementName[] = "ignore";
static const char IgnorePathName[] = "path";
static const char IgnorePathNameAttrib[] = "name";
static const char ExcludeElementName[] = "exclude";
static const char ExcludePathName[] = "path";
static const char ExcludePathNameAttrib[] = "name";
static const char LibrariesElementName[] = "libraries";
static const char LibraryElementName[] = "library";
static const char SuppressionsElementName[] = "suppressions";
static const char SuppressionElementName[] = "suppression";

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
    bool projectTagFound = false;
    while (!xmlReader.atEnd()) {
        switch (xmlReader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xmlReader.name() == ProjectElementName) {
                insideProject = true;
                projectTagFound = true;
            }
            // Read root path from inside project element
            if (insideProject && xmlReader.name() == RootPathName)
                ReadRootPath(xmlReader);

            // Read root path from inside project element
            if (insideProject && xmlReader.name() == BuildDirElementName)
                ReadBuildDir(xmlReader);

            // Find paths to check from inside project element
            if (insideProject && xmlReader.name() == PathsElementName)
                ReadCheckPaths(xmlReader);

            if (insideProject && xmlReader.name() == ImportProjectElementName)
                ReadImportProject(xmlReader);

            // Find include directory from inside project element
            if (insideProject && xmlReader.name() == IncludeDirElementName)
                ReadIncludeDirs(xmlReader);

            // Find preprocessor define from inside project element
            if (insideProject && xmlReader.name() == DefinesElementName)
                ReadDefines(xmlReader);

            // Find exclude list from inside project element
            if (insideProject && xmlReader.name() == ExcludeElementName)
                ReadExcludes(xmlReader);

            // Find ignore list from inside project element
            // These are read for compatibility
            if (insideProject && xmlReader.name() == IgnoreElementName)
                ReadExcludes(xmlReader);

            // Find libraries list from inside project element
            if (insideProject && xmlReader.name() == LibrariesElementName)
                ReadStringList(mLibraries, xmlReader,LibraryElementName);

            // Find suppressions list from inside project element
            if (insideProject && xmlReader.name() == SuppressionsElementName)
                ReadStringList(mSuppressions, xmlReader,SuppressionElementName);

            break;

        case QXmlStreamReader::EndElement:
            if (xmlReader.name() == ProjectElementName)
                insideProject = false;
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

    file.close();
    if (projectTagFound)
        return true;
    else
        return false;
}

void ProjectFile::ReadRootPath(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attribs = reader.attributes();
    QString name = attribs.value("", RootPathNameAttrib).toString();
    if (!name.isEmpty())
        mRootPath = name;
}

void ProjectFile::ReadBuildDir(QXmlStreamReader &reader)
{
    mBuildDir.clear();
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mBuildDir = reader.text().toString();
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (1);
}

void ProjectFile::ReadImportProject(QXmlStreamReader &reader)
{
    mImportProject.clear();
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mImportProject = reader.text().toString();
        case QXmlStreamReader::EndElement:
            return;
        // Not handled
        case QXmlStreamReader::StartElement:
        case QXmlStreamReader::NoToken:
        case QXmlStreamReader::Invalid:
        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    } while (1);
}

void ProjectFile::ReadIncludeDirs(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == DirElementName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value("", DirNameAttrib).toString();
                if (!name.isEmpty())
                    mIncludeDirs << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == IncludeDirElementName)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::ReadDefines(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read define-elements
            if (reader.name().toString() == DefineName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value("", DefineNameAttrib).toString();
                if (!name.isEmpty())
                    mDefines << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == DefinesElementName)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::ReadCheckPaths(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == PathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value("", PathNameAttrib).toString();
                if (!name.isEmpty())
                    mPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == PathsElementName)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::ReadExcludes(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read exclude-elements
            if (reader.name().toString() == ExcludePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value("", ExcludePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            // Read ignore-elements - deprecated but support reading them
            else if (reader.name().toString() == IgnorePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value("", IgnorePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == IgnoreElementName)
                allRead = true;
            if (reader.name().toString() == ExcludeElementName)
                allRead = true;
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
    } while (!allRead);
}


void ProjectFile::ReadStringList(QStringList &stringlist, QXmlStreamReader &reader, const char elementname[])
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == elementname) {
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    QString text = reader.text().toString();
                    stringlist << text;
                }
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != elementname)
                allRead = true;
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
    } while (!allRead);
}

void ProjectFile::SetIncludes(const QStringList &includes)
{
    mIncludeDirs = includes;
}

void ProjectFile::SetDefines(const QStringList &defines)
{
    mDefines = defines;
}

void ProjectFile::SetCheckPaths(const QStringList &paths)
{
    mPaths = paths;
}

void ProjectFile::SetExcludedPaths(const QStringList &paths)
{
    mExcludedPaths = paths;
}

void ProjectFile::SetLibraries(const QStringList &libraries)
{
    mLibraries = libraries;
}

void ProjectFile::SetSuppressions(const QStringList &suppressions)
{
    mSuppressions = suppressions;
}

bool ProjectFile::Write(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    QFile file(mFilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument("1.0");
    xmlWriter.writeStartElement(ProjectElementName);
    xmlWriter.writeAttribute(ProjectVersionAttrib, ProjectFileVersion);

    if (!mRootPath.isEmpty()) {
        xmlWriter.writeStartElement(RootPathName);
        xmlWriter.writeAttribute(RootPathNameAttrib, mRootPath);
        xmlWriter.writeEndElement();
    }

    if (!mBuildDir.isEmpty()) {
        xmlWriter.writeStartElement(BuildDirElementName);
        xmlWriter.writeCharacters(mBuildDir);
        xmlWriter.writeEndElement();
    }

    if (!mImportProject.isEmpty()) {
        xmlWriter.writeStartElement(ImportProjectElementName);
        xmlWriter.writeCharacters(mImportProject);
        xmlWriter.writeEndElement();
    }

    if (!mIncludeDirs.isEmpty()) {
        xmlWriter.writeStartElement(IncludeDirElementName);
        foreach (QString incdir, mIncludeDirs) {
            xmlWriter.writeStartElement(DirElementName);
            xmlWriter.writeAttribute(DirNameAttrib, incdir);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mDefines.isEmpty()) {
        xmlWriter.writeStartElement(DefinesElementName);
        foreach (QString define, mDefines) {
            xmlWriter.writeStartElement(DefineName);
            xmlWriter.writeAttribute(DefineNameAttrib, define);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mPaths.isEmpty()) {
        xmlWriter.writeStartElement(PathsElementName);
        foreach (QString path, mPaths) {
            xmlWriter.writeStartElement(PathName);
            xmlWriter.writeAttribute(PathNameAttrib, path);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mExcludedPaths.isEmpty()) {
        xmlWriter.writeStartElement(ExcludeElementName);
        foreach (QString path, mExcludedPaths) {
            xmlWriter.writeStartElement(ExcludePathName);
            xmlWriter.writeAttribute(ExcludePathNameAttrib, path);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    WriteStringList(xmlWriter,
                    mLibraries,
                    LibrariesElementName,
                    LibraryElementName);

    WriteStringList(xmlWriter,
                    mSuppressions,
                    SuppressionsElementName,
                    SuppressionElementName);

    xmlWriter.writeEndDocument();
    file.close();
    return true;
}

void ProjectFile::WriteStringList(QXmlStreamWriter &xmlWriter, const QStringList &stringlist, const char startelementname[], const char stringelementname[])
{
    if (stringlist.isEmpty())
        return;

    xmlWriter.writeStartElement(startelementname);
    foreach (QString str, stringlist) {
        xmlWriter.writeStartElement(stringelementname);
        xmlWriter.writeCharacters(str);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

QStringList ProjectFile::fromNativeSeparators(const QStringList &paths)
{
    QStringList ret;
    foreach (const QString &path, paths)
        ret << QDir::fromNativeSeparators(path);
    return ret;
}

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#include "common.h"
#include "importproject.h"

#include "settings.h"

ProjectFile *ProjectFile::mActiveProject;

ProjectFile::ProjectFile(QObject *parent) :
    QObject(parent)
{
    clear();
}

ProjectFile::ProjectFile(const QString &filename, QObject *parent) :
    QObject(parent),
    mFilename(filename)
{
    clear();
    read();
}

void ProjectFile::clear()
{
    const Settings settings;
    clangParser = false;
    bugHunting = false;
    mRootPath.clear();
    mBuildDir.clear();
    mImportProject.clear();
    mAnalyzeAllVsConfigs = true;
    mIncludeDirs.clear();
    mDefines.clear();
    mUndefines.clear();
    mPaths.clear();
    mExcludedPaths.clear();
    mFunctionContracts.clear();
    mVariableContracts.clear();
    mLibraries.clear();
    mPlatform.clear();
    mSuppressions.clear();
    mAddons.clear();
    mClangAnalyzer = mClangTidy = false;
    mAnalyzeAllVsConfigs = false;
    mCheckHeaders = true;
    mCheckUnusedTemplates = true;
    mMaxCtuDepth = settings.maxCtuDepth;
    mMaxTemplateRecursion = settings.maxTemplateRecursion;
    mCheckUnknownFunctionReturn.clear();
    safeChecks.clear();
    mVsConfigurations.clear();
    mTags.clear();
    mWarningTags.clear();
}

bool ProjectFile::read(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    QFile file(mFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    clear();

    QXmlStreamReader xmlReader(&file);
    bool insideProject = false;
    bool projectTagFound = false;
    while (!xmlReader.atEnd()) {
        switch (xmlReader.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xmlReader.name() == CppcheckXml::ProjectElementName) {
                insideProject = true;
                projectTagFound = true;
                break;
            }
            if (!insideProject)
                break;

            // Read root path from inside project element
            if (xmlReader.name() == CppcheckXml::RootPathName)
                readRootPath(xmlReader);

            // Read root path from inside project element
            if (xmlReader.name() == CppcheckXml::BuildDirElementName)
                readBuildDir(xmlReader);

            // Find paths to check from inside project element
            if (xmlReader.name() == CppcheckXml::PathsElementName)
                readCheckPaths(xmlReader);

            if (xmlReader.name() == CppcheckXml::ImportProjectElementName)
                readImportProject(xmlReader);

            if (xmlReader.name() == CppcheckXml::AnalyzeAllVsConfigsElementName)
                mAnalyzeAllVsConfigs = readBool(xmlReader);

            if (xmlReader.name() == CppcheckXml::Parser)
                clangParser = true;

            if (xmlReader.name() == CppcheckXml::BugHunting)
                bugHunting = true;

            if (xmlReader.name() == CppcheckXml::CheckHeadersElementName)
                mCheckHeaders = readBool(xmlReader);

            if (xmlReader.name() == CppcheckXml::CheckUnusedTemplatesElementName)
                mCheckUnusedTemplates = readBool(xmlReader);

            // Find include directory from inside project element
            if (xmlReader.name() == CppcheckXml::IncludeDirElementName)
                readIncludeDirs(xmlReader);

            // Find preprocessor define from inside project element
            if (xmlReader.name() == CppcheckXml::DefinesElementName)
                readDefines(xmlReader);

            // Find preprocessor define from inside project element
            if (xmlReader.name() == CppcheckXml::UndefinesElementName)
                readStringList(mUndefines, xmlReader, CppcheckXml::UndefineName);

            // Find exclude list from inside project element
            if (xmlReader.name() == CppcheckXml::ExcludeElementName)
                readExcludes(xmlReader);

            // Find ignore list from inside project element
            // These are read for compatibility
            if (xmlReader.name() == CppcheckXml::IgnoreElementName)
                readExcludes(xmlReader);

            // Function contracts
            if (xmlReader.name() == CppcheckXml::FunctionContracts)
                readFunctionContracts(xmlReader);

            // Variable constraints
            if (xmlReader.name() == CppcheckXml::VariableContractsElementName)
                readVariableContracts(xmlReader);

            // Find libraries list from inside project element
            if (xmlReader.name() == CppcheckXml::LibrariesElementName)
                readStringList(mLibraries, xmlReader, CppcheckXml::LibraryElementName);

            if (xmlReader.name() == CppcheckXml::PlatformElementName)
                readPlatform(xmlReader);

            // Find suppressions list from inside project element
            if (xmlReader.name() == CppcheckXml::SuppressionsElementName)
                readSuppressions(xmlReader);

            // Unknown function return values
            if (xmlReader.name() == CppcheckXml::CheckUnknownFunctionReturn)
                readStringList(mCheckUnknownFunctionReturn, xmlReader, CppcheckXml::Name);

            // check all function parameter values
            if (xmlReader.name() == Settings::SafeChecks::XmlRootName)
                safeChecks.loadFromXml(xmlReader);

            // Addons
            if (xmlReader.name() == CppcheckXml::AddonsElementName)
                readStringList(mAddons, xmlReader, CppcheckXml::AddonElementName);

            // Tools
            if (xmlReader.name() == CppcheckXml::ToolsElementName) {
                QStringList tools;
                readStringList(tools, xmlReader, CppcheckXml::ToolElementName);
                mClangAnalyzer = tools.contains(CLANG_ANALYZER);
                mClangTidy = tools.contains(CLANG_TIDY);
            }

            if (xmlReader.name() == CppcheckXml::TagsElementName)
                readStringList(mTags, xmlReader, CppcheckXml::TagElementName);

            if (xmlReader.name() == CppcheckXml::TagWarningsElementName)
                readTagWarnings(xmlReader, xmlReader.attributes().value(QString(), CppcheckXml::TagAttributeName).toString());

            if (xmlReader.name() == CppcheckXml::MaxCtuDepthElementName)
                mMaxCtuDepth = readInt(xmlReader, mMaxCtuDepth);

            if (xmlReader.name() == CppcheckXml::MaxTemplateRecursionElementName)
                mMaxTemplateRecursion = readInt(xmlReader, mMaxTemplateRecursion);

            // VSConfiguration
            if (xmlReader.name() == CppcheckXml::VSConfigurationElementName)
                readVsConfigurations(xmlReader);
            break;

        case QXmlStreamReader::EndElement:
            if (xmlReader.name() == CppcheckXml::ProjectElementName)
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
    return projectTagFound;
}

void ProjectFile::readRootPath(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attribs = reader.attributes();
    QString name = attribs.value(QString(), CppcheckXml::RootPathNameAttrib).toString();
    if (!name.isEmpty())
        mRootPath = name;
}

void ProjectFile::readBuildDir(QXmlStreamReader &reader)
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
    } while (true);
}

void ProjectFile::readImportProject(QXmlStreamReader &reader)
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
    } while (true);
}

bool ProjectFile::readBool(QXmlStreamReader &reader)
{
    bool ret = false;
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            ret = (reader.text().toString() == "true");
        case QXmlStreamReader::EndElement:
            return ret;
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
    } while (true);
}

int ProjectFile::readInt(QXmlStreamReader &reader, int defaultValue)
{
    int ret = defaultValue;
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            ret = reader.text().toString().toInt();
        case QXmlStreamReader::EndElement:
            return ret;
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
    } while (true);
}

void ProjectFile::readIncludeDirs(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == CppcheckXml::DirElementName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::DirNameAttrib).toString();
                if (!name.isEmpty())
                    mIncludeDirs << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::IncludeDirElementName)
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

void ProjectFile::readDefines(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read define-elements
            if (reader.name().toString() == CppcheckXml::DefineName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::DefineNameAttrib).toString();
                if (!name.isEmpty())
                    mDefines << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::DefinesElementName)
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

void ProjectFile::readCheckPaths(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:

            // Read dir-elements
            if (reader.name().toString() == CppcheckXml::PathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::PathNameAttrib).toString();
                if (!name.isEmpty())
                    mPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::PathsElementName)
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

void ProjectFile::readExcludes(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read exclude-elements
            if (reader.name().toString() == CppcheckXml::ExcludePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::ExcludePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            // Read ignore-elements - deprecated but support reading them
            else if (reader.name().toString() == CppcheckXml::IgnorePathName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString name = attribs.value(QString(), CppcheckXml::IgnorePathNameAttrib).toString();
                if (!name.isEmpty())
                    mExcludedPaths << name;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::IgnoreElementName)
                allRead = true;
            if (reader.name().toString() == CppcheckXml::ExcludeElementName)
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

void ProjectFile::readFunctionContracts(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    bool allRead = false;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            if (reader.name().toString() == CppcheckXml::FunctionContract) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString function = attribs.value(QString(), CppcheckXml::ContractFunction).toString();
                QString expects = attribs.value(QString(), CppcheckXml::ContractExpects).toString();
                if (!function.isEmpty() && !expects.isEmpty())
                    mFunctionContracts[function.toStdString()] = expects.toStdString();
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::FunctionContracts)
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

void ProjectFile::readVariableContracts(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    while (true) {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            if (reader.name().toString() == CppcheckXml::VariableContractItemElementName) {
                QXmlStreamAttributes attribs = reader.attributes();
                QString varname = attribs.value(QString(), CppcheckXml::VariableContractVarName).toString();
                QString minValue = attribs.value(QString(), CppcheckXml::VariableContractMin).toString();
                QString maxValue = attribs.value(QString(), CppcheckXml::VariableContractMax).toString();
                setVariableContracts(varname, minValue, maxValue);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() == CppcheckXml::VariableContractsElementName)
                return;
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
}

void ProjectFile::readVsConfigurations(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == CppcheckXml::VSConfigurationName) {
                QString config;
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    config = reader.text().toString();
                }
                mVsConfigurations << config;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != CppcheckXml::VSConfigurationName)
                return;
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
    } while (true);
}

void ProjectFile::readPlatform(QXmlStreamReader &reader)
{
    do {
        const QXmlStreamReader::TokenType type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::Characters:
            mPlatform = reader.text().toString();
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
    } while (true);
}


void ProjectFile::readSuppressions(QXmlStreamReader &reader)
{
    QXmlStreamReader::TokenType type;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == CppcheckXml::SuppressionElementName) {
                Suppressions::Suppression suppression;
                if (reader.attributes().hasAttribute(QString(),"fileName"))
                    suppression.fileName = reader.attributes().value(QString(),"fileName").toString().toStdString();
                if (reader.attributes().hasAttribute(QString(),"lineNumber"))
                    suppression.lineNumber = reader.attributes().value(QString(),"lineNumber").toInt();
                if (reader.attributes().hasAttribute(QString(),"symbolName"))
                    suppression.symbolName = reader.attributes().value(QString(),"symbolName").toString().toStdString();
                if (reader.attributes().hasAttribute(QString(),"hash"))
                    suppression.hash = reader.attributes().value(QString(),"hash").toULongLong();
                type = reader.readNext();
                if (type == QXmlStreamReader::Characters) {
                    suppression.errorId = reader.text().toString().toStdString();
                }
                mSuppressions << suppression;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != CppcheckXml::SuppressionElementName)
                return;
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
    } while (true);
}


void ProjectFile::readTagWarnings(QXmlStreamReader &reader, const QString &tag)
{
    QXmlStreamReader::TokenType type;
    do {
        type = reader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            // Read library-elements
            if (reader.name().toString() == CppcheckXml::WarningElementName) {
                std::size_t hash = reader.attributes().value(QString(), CppcheckXml::HashAttributeName).toULongLong();
                mWarningTags[hash] = tag;
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name().toString() != CppcheckXml::WarningElementName)
                return;
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
    } while (true);
}


void ProjectFile::readStringList(QStringList &stringlist, QXmlStreamReader &reader, const char elementname[])
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

void ProjectFile::setIncludes(const QStringList &includes)
{
    mIncludeDirs = includes;
}

void ProjectFile::setDefines(const QStringList &defines)
{
    mDefines = defines;
}

void ProjectFile::setUndefines(const QStringList &undefines)
{
    mUndefines = undefines;
}

void ProjectFile::setCheckPaths(const QStringList &paths)
{
    mPaths = paths;
}

void ProjectFile::setExcludedPaths(const QStringList &paths)
{
    mExcludedPaths = paths;
}

void ProjectFile::setLibraries(const QStringList &libraries)
{
    mLibraries = libraries;
}

void ProjectFile::setFunctionContract(QString function, QString expects)
{
    mFunctionContracts[function.toStdString()] = expects.toStdString();
}

void ProjectFile::setPlatform(const QString &platform)
{
    mPlatform = platform;
}

void ProjectFile::setSuppressions(const QList<Suppressions::Suppression> &suppressions)
{
    mSuppressions = suppressions;
}

void ProjectFile::addSuppression(const Suppressions::Suppression &suppression)
{
    mSuppressions.append(suppression);
}

void ProjectFile::setAddons(const QStringList &addons)
{
    mAddons = addons;
}

void ProjectFile::setVSConfigurations(const QStringList &vsConfigs)
{
    mVsConfigurations = vsConfigs;
}

void ProjectFile::setWarningTags(std::size_t hash, QString tags)
{
    if (tags.isEmpty())
        mWarningTags.erase(hash);
    else if (hash > 0)
        mWarningTags[hash] = tags;
}

QString ProjectFile::getWarningTags(std::size_t hash) const
{
    auto it = mWarningTags.find(hash);
    return (it != mWarningTags.end()) ? it->second : QString();
}

bool ProjectFile::write(const QString &filename)
{
    if (!filename.isEmpty())
        mFilename = filename;

    QFile file(mFilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument("1.0");
    xmlWriter.writeStartElement(CppcheckXml::ProjectElementName);
    xmlWriter.writeAttribute(CppcheckXml::ProjectVersionAttrib, CppcheckXml::ProjectFileVersion);

    if (!mRootPath.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::RootPathName);
        xmlWriter.writeAttribute(CppcheckXml::RootPathNameAttrib, mRootPath);
        xmlWriter.writeEndElement();
    }

    if (!mBuildDir.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::BuildDirElementName);
        xmlWriter.writeCharacters(mBuildDir);
        xmlWriter.writeEndElement();
    }

    if (!mPlatform.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::PlatformElementName);
        xmlWriter.writeCharacters(mPlatform);
        xmlWriter.writeEndElement();
    }

    if (!mImportProject.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::ImportProjectElementName);
        xmlWriter.writeCharacters(mImportProject);
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeStartElement(CppcheckXml::AnalyzeAllVsConfigsElementName);
    xmlWriter.writeCharacters(mAnalyzeAllVsConfigs ? "true" : "false");
    xmlWriter.writeEndElement();

    if (clangParser) {
        xmlWriter.writeStartElement(CppcheckXml::Parser);
        xmlWriter.writeCharacters("clang");
        xmlWriter.writeEndElement();
    }

    if (bugHunting) {
        xmlWriter.writeStartElement(CppcheckXml::BugHunting);
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeStartElement(CppcheckXml::CheckHeadersElementName);
    xmlWriter.writeCharacters(mCheckHeaders ? "true" : "false");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(CppcheckXml::CheckUnusedTemplatesElementName);
    xmlWriter.writeCharacters(mCheckUnusedTemplates ? "true" : "false");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(CppcheckXml::MaxCtuDepthElementName);
    xmlWriter.writeCharacters(QString::number(mMaxCtuDepth));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(CppcheckXml::MaxTemplateRecursionElementName);
    xmlWriter.writeCharacters(QString::number(mMaxTemplateRecursion));
    xmlWriter.writeEndElement();

    if (!mIncludeDirs.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::IncludeDirElementName);
        foreach (QString incdir, mIncludeDirs) {
            xmlWriter.writeStartElement(CppcheckXml::DirElementName);
            xmlWriter.writeAttribute(CppcheckXml::DirNameAttrib, incdir);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mDefines.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::DefinesElementName);
        foreach (QString define, mDefines) {
            xmlWriter.writeStartElement(CppcheckXml::DefineName);
            xmlWriter.writeAttribute(CppcheckXml::DefineNameAttrib, define);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mVsConfigurations.isEmpty()) {
        writeStringList(xmlWriter,
                        mVsConfigurations,
                        CppcheckXml::VSConfigurationElementName,
                        CppcheckXml::VSConfigurationName);
    }

    writeStringList(xmlWriter,
                    mUndefines,
                    CppcheckXml::UndefinesElementName,
                    CppcheckXml::UndefineName);

    if (!mPaths.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::PathsElementName);
        foreach (QString path, mPaths) {
            xmlWriter.writeStartElement(CppcheckXml::PathName);
            xmlWriter.writeAttribute(CppcheckXml::PathNameAttrib, path);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mExcludedPaths.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::ExcludeElementName);
        foreach (QString path, mExcludedPaths) {
            xmlWriter.writeStartElement(CppcheckXml::ExcludePathName);
            xmlWriter.writeAttribute(CppcheckXml::ExcludePathNameAttrib, path);
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    writeStringList(xmlWriter,
                    mLibraries,
                    CppcheckXml::LibrariesElementName,
                    CppcheckXml::LibraryElementName);

    if (!mFunctionContracts.empty()) {
        xmlWriter.writeStartElement(CppcheckXml::FunctionContracts);
        for (const auto& contract: mFunctionContracts) {
            xmlWriter.writeStartElement(CppcheckXml::FunctionContract);
            xmlWriter.writeAttribute(CppcheckXml::ContractFunction, QString::fromStdString(contract.first));
            xmlWriter.writeAttribute(CppcheckXml::ContractExpects, QString::fromStdString(contract.second));
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    if (!mVariableContracts.empty()) {
        xmlWriter.writeStartElement(CppcheckXml::VariableContractsElementName);

        for (auto vc: mVariableContracts) {
            xmlWriter.writeStartElement(CppcheckXml::VariableContractItemElementName);
            xmlWriter.writeAttribute(CppcheckXml::VariableContractVarName, vc.first);
            xmlWriter.writeAttribute(CppcheckXml::VariableContractMin, QString::fromStdString(vc.second.minValue));
            xmlWriter.writeAttribute(CppcheckXml::VariableContractMax, QString::fromStdString(vc.second.maxValue));
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
    }

    if (!mSuppressions.isEmpty()) {
        xmlWriter.writeStartElement(CppcheckXml::SuppressionsElementName);
        foreach (const Suppressions::Suppression &suppression, mSuppressions) {
            xmlWriter.writeStartElement(CppcheckXml::SuppressionElementName);
            if (!suppression.fileName.empty())
                xmlWriter.writeAttribute("fileName", QString::fromStdString(suppression.fileName));
            if (suppression.lineNumber > 0)
                xmlWriter.writeAttribute("lineNumber", QString::number(suppression.lineNumber));
            if (!suppression.symbolName.empty())
                xmlWriter.writeAttribute("symbolName", QString::fromStdString(suppression.symbolName));
            if (suppression.hash > 0)
                xmlWriter.writeAttribute(CppcheckXml::HashAttributeName, QString::number(suppression.hash));
            if (!suppression.errorId.empty())
                xmlWriter.writeCharacters(QString::fromStdString(suppression.errorId));
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement();
    }

    writeStringList(xmlWriter,
                    mCheckUnknownFunctionReturn,
                    CppcheckXml::CheckUnknownFunctionReturn,
                    CppcheckXml::Name);

    safeChecks.saveToXml(xmlWriter);

    writeStringList(xmlWriter,
                    mAddons,
                    CppcheckXml::AddonsElementName,
                    CppcheckXml::AddonElementName);

    QStringList tools;
    if (mClangAnalyzer)
        tools << CLANG_ANALYZER;
    if (mClangTidy)
        tools << CLANG_TIDY;
    writeStringList(xmlWriter,
                    tools,
                    CppcheckXml::ToolsElementName,
                    CppcheckXml::ToolElementName);

    writeStringList(xmlWriter, mTags, CppcheckXml::TagsElementName, CppcheckXml::TagElementName);
    if (!mWarningTags.empty()) {
        QStringList tags;
        for (const auto& wt: mWarningTags) {
            if (!tags.contains(wt.second))
                tags.append(wt.second);
        }
        for (const QString &tag: tags) {
            xmlWriter.writeStartElement(CppcheckXml::TagWarningsElementName);
            xmlWriter.writeAttribute(CppcheckXml::TagAttributeName, tag);
            QStringList warnings;
            for (const auto& wt: mWarningTags) {
                if (wt.second == tag) {
                    xmlWriter.writeStartElement(CppcheckXml::WarningElementName);
                    xmlWriter.writeAttribute(CppcheckXml::HashAttributeName, QString::number(wt.first));
                    xmlWriter.writeEndElement();
                }
            }
            xmlWriter.writeEndElement();
        }
    }

    xmlWriter.writeEndDocument();
    file.close();
    return true;
}

void ProjectFile::writeStringList(QXmlStreamWriter &xmlWriter, const QStringList &stringlist, const char startelementname[], const char stringelementname[])
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

QStringList ProjectFile::getAddonsAndTools() const
{
    QStringList ret(mAddons);
    if (mClangAnalyzer)
        ret << CLANG_ANALYZER;
    if (mClangTidy)
        ret << CLANG_TIDY;
    return ret;
}

void ProjectFile::SafeChecks::loadFromXml(QXmlStreamReader &xmlReader)
{
    classes = externalFunctions = internalFunctions = externalVariables = false;

    int level = 0;

    do {
        const QXmlStreamReader::TokenType type = xmlReader.readNext();
        switch (type) {
        case QXmlStreamReader::StartElement:
            ++level;
            if (xmlReader.name() == Settings::SafeChecks::XmlClasses)
                classes = true;
            else if (xmlReader.name() == Settings::SafeChecks::XmlExternalFunctions)
                externalFunctions = true;
            else if (xmlReader.name() == Settings::SafeChecks::XmlInternalFunctions)
                internalFunctions = true;
            else if (xmlReader.name() == Settings::SafeChecks::XmlExternalVariables)
                externalVariables = true;
            break;
        case QXmlStreamReader::EndElement:
            if (level <= 0)
                return;
            level--;
            break;
        // Not handled
        case QXmlStreamReader::Characters:
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
    } while (true);
}

void ProjectFile::SafeChecks::saveToXml(QXmlStreamWriter &xmlWriter) const
{
    if (!classes && !externalFunctions && !internalFunctions && !externalVariables)
        return;
    xmlWriter.writeStartElement(Settings::SafeChecks::XmlRootName);
    if (classes) {
        xmlWriter.writeStartElement(Settings::SafeChecks::XmlClasses);
        xmlWriter.writeEndElement();
    }
    if (externalFunctions) {
        xmlWriter.writeStartElement(Settings::SafeChecks::XmlExternalFunctions);
        xmlWriter.writeEndElement();
    }
    if (internalFunctions) {
        xmlWriter.writeStartElement(Settings::SafeChecks::XmlInternalFunctions);
        xmlWriter.writeEndElement();
    }
    if (externalVariables) {
        xmlWriter.writeStartElement(Settings::SafeChecks::XmlExternalVariables);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

QString ProjectFile::getAddonFilePath(QString filesDir, const QString &addon)
{
    if (!filesDir.endsWith("/"))
        filesDir += "/";

    QStringList searchPaths;
    searchPaths << filesDir << (filesDir + "addons/") << (filesDir + "../addons/")
#ifdef FILESDIR
                << (QLatin1String(FILESDIR) + "/addons/")
#endif
                ;

    foreach (QString path, searchPaths) {
        QString f = path + addon + ".py";
        if (QFile(f).exists())
            return f;
    }

    return QString();
}

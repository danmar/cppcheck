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

#include "cppchecklibrarydata.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

const unsigned int CppcheckLibraryData::Function::Arg::ANY = ~0U;

CppcheckLibraryData::CppcheckLibraryData()
{
}

static CppcheckLibraryData::Container loadContainer(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Container container;
    container.id           = xmlReader.attributes().value("id").toString();
    container.inherits     = xmlReader.attributes().value("inherits").toString();
    container.startPattern = xmlReader.attributes().value("startPattern").toString();
    container.endPattern   = xmlReader.attributes().value("endPattern").toString();

    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "container") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "type") {
            container.type.templateParameter = xmlReader.attributes().value("templateParameter").toString();
            container.type.string            = xmlReader.attributes().value("string").toString();
        } else if (elementName == "size" || elementName == "access" || elementName == "other") {
            const QString indexOperator = xmlReader.attributes().value("indexOperator").toString();
            if (elementName == "access" && indexOperator == "array-like")
                container.access_arrayLike = true;
            const QString templateParameter = xmlReader.attributes().value("templateParameter").toString();
            if (elementName == "size" && !templateParameter.isEmpty())
                container.size_templateParameter = templateParameter.toInt();
            for (;;) {
                type = xmlReader.readNext();
                if (xmlReader.name().toString() == elementName)
                    break;
                if (type != QXmlStreamReader::StartElement)
                    continue;
                struct CppcheckLibraryData::Container::Function function;
                function.name   = xmlReader.attributes().value("name").toString();
                function.action = xmlReader.attributes().value("action").toString();
                function.yields = xmlReader.attributes().value("yields").toString();
                if (elementName == "size")
                    container.sizeFunctions.append(function);
                else if (elementName == "access")
                    container.accessFunctions.append(function);
                else
                    container.otherFunctions.append(function);
            };
        }
    }
    return container;
}

static CppcheckLibraryData::Define loadDefine(const QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Define define;
    define.name = xmlReader.attributes().value("name").toString();
    define.value = xmlReader.attributes().value("value").toString();
    return define;
}

static CppcheckLibraryData::Function::Arg loadFunctionArg(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Function::Arg arg;
    QString argnr = xmlReader.attributes().value("nr").toString();
    if (argnr == "any")
        arg.nr = CppcheckLibraryData::Function::Arg::ANY;
    else
        arg.nr = argnr.toUInt();

    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "arg") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "not-bool")
            arg.notbool = true;
        else if (elementName == "not-null")
            arg.notnull = true;
        else if (elementName == "not-uninit")
            arg.notuninit = true;
        else if (elementName == "strz")
            arg.strz = true;
        else if (elementName == "formatstr")
            arg.formatstr = true;
        else if (elementName == "valid")
            arg.valid = xmlReader.readElementText();
        else if (elementName == "minsize") {
            CppcheckLibraryData::Function::Arg::MinSize minsize;
            minsize.type = xmlReader.attributes().value("type").toString();
            minsize.arg  = xmlReader.attributes().value("arg").toString();
            minsize.arg2 = xmlReader.attributes().value("arg2").toString();
            arg.minsizes.append(minsize);
        }
    }
    return arg;
}

static CppcheckLibraryData::Function loadFunction(QXmlStreamReader &xmlReader, const QString comments)
{
    CppcheckLibraryData::Function function;
    function.comments = comments;
    function.name = xmlReader.attributes().value("name").toString();
    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "function") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "noreturn")
            function.noreturn = (xmlReader.readElementText() == "true") ? CppcheckLibraryData::Function::True : CppcheckLibraryData::Function::False;
        else if (elementName == "pure")
            function.gccPure = true;
        else if (elementName == "const")
            function.gccConst = true;
        else if (elementName == "leak-ignore")
            function.leakignore = true;
        else if (elementName == "use-retval")
            function.useretval = true;
        else if (elementName == "formatstr") {
            function.formatstr.scan   = xmlReader.attributes().value("scan").toString();
            function.formatstr.secure = xmlReader.attributes().value("secure").toString();
        } else if (elementName == "arg")
            function.args.append(loadFunctionArg(xmlReader));
        else if (elementName == "warn") {
            function.warn.severity     = xmlReader.attributes().value("severity").toString();
            function.warn.reason       = xmlReader.attributes().value("reason").toString();
            function.warn.alternatives = xmlReader.attributes().value("alternatives").toString();
            function.warn.msg          = xmlReader.readElementText();
        }
    }
    return function;
}

static CppcheckLibraryData::MemoryResource loadMemoryResource(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::MemoryResource memoryresource;
    memoryresource.type = xmlReader.name().toString();
    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != memoryresource.type) {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "alloc") {
            CppcheckLibraryData::MemoryResource::Alloc alloc;
            alloc.init = (xmlReader.attributes().value("init").toString() == "true");
            alloc.name = xmlReader.readElementText();
            memoryresource.alloc.append(alloc);
        } else if (elementName == "dealloc")
            memoryresource.dealloc.append(xmlReader.readElementText());
        else if (elementName == "use")
            memoryresource.use.append(xmlReader.readElementText());
    }
    return memoryresource;
}

static CppcheckLibraryData::PodType loadPodType(const QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::PodType podtype;
    podtype.name = xmlReader.attributes().value("name").toString();
    podtype.size = xmlReader.attributes().value("size").toString();
    podtype.sign = xmlReader.attributes().value("sign").toString();
    return podtype;
}

bool CppcheckLibraryData::open(QIODevice &file)
{
    clear();
    QString comments;
    QXmlStreamReader xmlReader(&file);
    while (!xmlReader.atEnd()) {
        const QXmlStreamReader::TokenType t = xmlReader.readNext();
        switch (t) {
        case QXmlStreamReader::Comment:
            if (!comments.isEmpty())
                comments += "\n";
            comments += xmlReader.text().toString();
            break;
        case QXmlStreamReader::StartElement:
            if (xmlReader.name() == "container")
                containers.append(loadContainer(xmlReader));
            if (xmlReader.name() == "define")
                defines.append(loadDefine(xmlReader));
            else if (xmlReader.name() == "function")
                functions.append(loadFunction(xmlReader, comments));
            else if (xmlReader.name() == "memory" || xmlReader.name() == "resource")
                memoryresource.append(loadMemoryResource(xmlReader));
            else if (xmlReader.name() == "podtype")
                podtypes.append(loadPodType(xmlReader));
            comments.clear();
            break;
        default:
            break;
        }
    }

    return true;
}

static void writeContainerFunctions(QXmlStreamWriter &xmlWriter, const QString name, int extra, const QList<struct CppcheckLibraryData::Container::Function> &functions)
{
    if (functions.isEmpty() && extra <= 0)
        return;
    xmlWriter.writeStartElement(name);
    if (extra > 0) {
        if (name == "access")
            xmlWriter.writeAttribute("indexOperator", "array-like");
        else if (name == "size")
            xmlWriter.writeAttribute("templateParameter", QString::number(extra));
    }
    foreach (const CppcheckLibraryData::Container::Function &function, functions) {
        xmlWriter.writeStartElement("function");
        xmlWriter.writeAttribute("name", function.name);
        if (!function.action.isEmpty())
            xmlWriter.writeAttribute("action", function.action);
        if (!function.yields.isEmpty())
            xmlWriter.writeAttribute("yields", function.yields);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
}

static void writeContainer(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::Container &container)
{
    xmlWriter.writeStartElement("container");
    xmlWriter.writeAttribute("id", container.id);
    if (!container.startPattern.isEmpty())
        xmlWriter.writeAttribute("startPattern", container.startPattern);
    if (!container.endPattern.isNull())
        xmlWriter.writeAttribute("endPattern", container.endPattern);
    if (!container.inherits.isEmpty())
        xmlWriter.writeAttribute("inherits", container.inherits);
    if (!container.type.templateParameter.isEmpty() || !container.type.string.isEmpty()) {
        xmlWriter.writeStartElement("type");
        if (!container.type.templateParameter.isEmpty())
            xmlWriter.writeAttribute("templateParameter", container.type.templateParameter);
        if (!container.type.string.isEmpty())
            xmlWriter.writeAttribute("string", container.type.string);
        xmlWriter.writeEndElement();
    }
    writeContainerFunctions(xmlWriter, "size", container.size_templateParameter, container.sizeFunctions);
    writeContainerFunctions(xmlWriter, "access", container.access_arrayLike, container.accessFunctions);
    writeContainerFunctions(xmlWriter, "other", 0, container.otherFunctions);
    xmlWriter.writeEndElement();
}

static void writeFunction(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::Function &function)
{
    QString comments = function.comments;
    while (comments.startsWith("\n"))
        comments = comments.mid(1);
    while (comments.endsWith("\n"))
        comments.chop(1);
    foreach (const QString &comment, comments.split('\n')) {
        if (comment.length() >= 1)
            xmlWriter.writeComment(comment);
    }

    xmlWriter.writeStartElement("function");
    xmlWriter.writeAttribute("name", function.name);

    if (function.useretval)
        xmlWriter.writeEmptyElement("use-retval");
    if (function.gccConst)
        xmlWriter.writeEmptyElement("const");
    if (function.gccPure)
        xmlWriter.writeEmptyElement("pure");
    if (function.noreturn != CppcheckLibraryData::Function::Unknown)
        xmlWriter.writeTextElement("noreturn", (function.noreturn == CppcheckLibraryData::Function::True) ? "true" : "false");
    if (function.leakignore)
        xmlWriter.writeEmptyElement("leak-ignore");
    // Argument info..
    foreach (const CppcheckLibraryData::Function::Arg &arg, function.args) {
        if (arg.formatstr) {
            xmlWriter.writeStartElement("formatstr");
            if (!function.formatstr.scan.isNull())
                xmlWriter.writeAttribute("scan", function.formatstr.scan);
            if (!function.formatstr.secure.isNull())
                xmlWriter.writeAttribute("secure", function.formatstr.secure);
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeStartElement("arg");
        if (arg.nr == CppcheckLibraryData::Function::Arg::ANY)
            xmlWriter.writeAttribute("nr", "any");
        else
            xmlWriter.writeAttribute("nr", QString::number(arg.nr));
        if (arg.formatstr)
            xmlWriter.writeEmptyElement("formatstr");
        if (arg.notnull)
            xmlWriter.writeEmptyElement("not-null");
        if (arg.notuninit)
            xmlWriter.writeEmptyElement("not-uninit");
        if (arg.notbool)
            xmlWriter.writeEmptyElement("not-bool");
        if (arg.strz)
            xmlWriter.writeEmptyElement("strz");

        if (!arg.valid.isEmpty())
            xmlWriter.writeTextElement("valid",arg.valid);

        foreach (const CppcheckLibraryData::Function::Arg::MinSize &minsize, arg.minsizes) {
            xmlWriter.writeStartElement("minsize");
            xmlWriter.writeAttribute("type", minsize.type);
            xmlWriter.writeAttribute("arg", minsize.arg);
            if (!minsize.arg2.isEmpty())
                xmlWriter.writeAttribute("arg2", minsize.arg2);
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
    }

    if (!function.warn.isEmpty()) {
        xmlWriter.writeStartElement("warn");

        if (!function.warn.severity.isEmpty())
            xmlWriter.writeAttribute("severity", function.warn.severity);

        if (!function.warn.reason.isEmpty())
            xmlWriter.writeAttribute("reason", function.warn.reason);

        if (!function.warn.alternatives.isEmpty())
            xmlWriter.writeAttribute("alternatives", function.warn.alternatives);

        if (!function.warn.msg.isEmpty())
            xmlWriter.writeCharacters(function.warn.msg);

        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndElement();
}

static void writeMemoryResource(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::MemoryResource &mr)
{
    xmlWriter.writeStartElement(mr.type);
    foreach (const CppcheckLibraryData::MemoryResource::Alloc &alloc, mr.alloc) {
        xmlWriter.writeStartElement("alloc");
        xmlWriter.writeAttribute("init", alloc.init ? "true" : "false");
        xmlWriter.writeCharacters(alloc.name);
        xmlWriter.writeEndElement();
    }
    foreach (const QString &dealloc, mr.dealloc) {
        xmlWriter.writeTextElement("dealloc", dealloc);
    }
    foreach (const QString &use, mr.use) {
        xmlWriter.writeTextElement("use", use);
    }
    xmlWriter.writeEndElement();
}

QString CppcheckLibraryData::toString() const
{
    QString outputString;
    QXmlStreamWriter xmlWriter(&outputString);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.setAutoFormattingIndent(2);
    xmlWriter.writeStartDocument("1.0");
    xmlWriter.writeStartElement("def");
    xmlWriter.writeAttribute("format","2");

    foreach (const Define &define, defines) {
        xmlWriter.writeStartElement("define");
        xmlWriter.writeAttribute("name", define.name);
        xmlWriter.writeAttribute("value", define.value);
        xmlWriter.writeEndElement();
    }

    foreach (const Function &function, functions) {
        writeFunction(xmlWriter, function);
    }

    foreach (const MemoryResource &mr, memoryresource) {
        writeMemoryResource(xmlWriter, mr);
    }

    foreach (const Container &container, containers) {
        writeContainer(xmlWriter, container);
    }

    foreach (const PodType &podtype, podtypes) {
        xmlWriter.writeStartElement("podtype");
        xmlWriter.writeAttribute("name", podtype.name);
        if (!podtype.sign.isEmpty())
            xmlWriter.writeAttribute("sign", podtype.sign);
        if (!podtype.size.isEmpty())
            xmlWriter.writeAttribute("size", podtype.size);
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndElement();

    return outputString;
}

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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

#include <stdexcept>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

const unsigned int CppcheckLibraryData::Function::Arg::ANY = ~0U;
const unsigned int CppcheckLibraryData::Function::Arg::VARIADIC = ~1U;

CppcheckLibraryData::CppcheckLibraryData()
{
}

static std::string unhandledElement(const QXmlStreamReader &xmlReader)
{
    throw std::runtime_error(QObject::tr("line %1: Unhandled element %2").arg(xmlReader.lineNumber()).arg(xmlReader.name().toString()).toStdString());
}

static std::string mandatoryAttibuteMissing(const QXmlStreamReader &xmlReader, QString attributeName)
{
    throw std::runtime_error(QObject::tr("line %1: Mandatory attribute '%2' missing in '%3'")
                             .arg(xmlReader.lineNumber())
                             .arg(attributeName)
                             .arg(xmlReader.name().toString()).toStdString());
}

static CppcheckLibraryData::Container loadContainer(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Container container;
    container.id            = xmlReader.attributes().value("id").toString();
    container.inherits      = xmlReader.attributes().value("inherits").toString();
    container.startPattern  = xmlReader.attributes().value("startPattern").toString();
    container.endPattern    = xmlReader.attributes().value("endPattern").toString();
    container.opLessAllowed = xmlReader.attributes().value("opLessAllowed").toString();
    container.itEndPattern  = xmlReader.attributes().value("itEndPattern").toString();

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
            }
        } else {
            unhandledElement(xmlReader);
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

static QString loadUndefine(const QXmlStreamReader &xmlReader)
{
    return xmlReader.attributes().value("name").toString();
}

static QString loadSmartPointer(const QXmlStreamReader &xmlReader)
{
    return xmlReader.attributes().value("class-name").toString();
}

static CppcheckLibraryData::TypeChecks loadTypeChecks(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::TypeChecks typeChecks;
    QXmlStreamReader::TokenType type;
    while ((type = xmlReader.readNext()) != QXmlStreamReader::EndElement ||
           xmlReader.name().toString() != "type-checks") {
        if (type != QXmlStreamReader::StartElement)
            continue;
        const QString elementName = xmlReader.name().toString();
        if (elementName == "suppress" || elementName == "check") {
            QPair<QString, QString> entry(elementName, xmlReader.readElementText());
            typeChecks.append(entry);
        }
    }
    return typeChecks;
}

static CppcheckLibraryData::Function::Arg loadFunctionArg(QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::Function::Arg arg;
    QString argnr = xmlReader.attributes().value("nr").toString();
    if (argnr == "any")
        arg.nr = CppcheckLibraryData::Function::Arg::ANY;
    else if (argnr == "variadic")
        arg.nr = CppcheckLibraryData::Function::Arg::VARIADIC;
    else
        arg.nr = argnr.toUInt();
    arg.defaultValue = xmlReader.attributes().value("default").toString();

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
        } else if (elementName == "iterator") {
            arg.iterator.container = xmlReader.attributes().value("container").toInt();
            arg.iterator.type = xmlReader.attributes().value("type").toString();
        } else {
            unhandledElement(xmlReader);
        }
    }
    return arg;
}

static CppcheckLibraryData::Function loadFunction(QXmlStreamReader &xmlReader, const QString &comments)
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
        else if (elementName == "returnValue") {
            const QString container = xmlReader.attributes().value("container").toString();
            function.returnValue.container = container.isNull() ? -1 : container.toInt();
            function.returnValue.type = xmlReader.attributes().value("type").toString();
            function.returnValue.value = xmlReader.readElementText();
        } else if (elementName == "formatstr") {
            function.formatstr.scan   = xmlReader.attributes().value("scan").toString();
            function.formatstr.secure = xmlReader.attributes().value("secure").toString();
        } else if (elementName == "arg")
            function.args.append(loadFunctionArg(xmlReader));
        else if (elementName == "warn") {
            function.warn.severity     = xmlReader.attributes().value("severity").toString();
            function.warn.cstd         = xmlReader.attributes().value("cstd").toString();
            function.warn.reason       = xmlReader.attributes().value("reason").toString();
            function.warn.alternatives = xmlReader.attributes().value("alternatives").toString();
            function.warn.msg          = xmlReader.readElementText();
        } else {
            unhandledElement(xmlReader);
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
        if (elementName == "alloc" || elementName == "realloc") {
            CppcheckLibraryData::MemoryResource::Alloc alloc;
            alloc.isRealloc = (elementName == "realloc");
            alloc.init = (xmlReader.attributes().value("init").toString() == "true");
            if (xmlReader.attributes().hasAttribute("arg")) {
                alloc.arg = xmlReader.attributes().value("arg").toInt();
            }
            if (alloc.isRealloc && xmlReader.attributes().hasAttribute("realloc-arg")) {
                alloc.reallocArg = xmlReader.attributes().value("realloc-arg").toInt();
            }
            if (memoryresource.type == "memory") {
                alloc.bufferSize = xmlReader.attributes().value("buffer-size").toString();
            }
            alloc.name = xmlReader.readElementText();
            memoryresource.alloc.append(alloc);
        } else if (elementName == "dealloc") {
            CppcheckLibraryData::MemoryResource::Dealloc dealloc;
            if (xmlReader.attributes().hasAttribute("arg")) {
                dealloc.arg = xmlReader.attributes().value("arg").toInt();
            }
            dealloc.name = xmlReader.readElementText();
            memoryresource.dealloc.append(dealloc);
        } else if (elementName == "use")
            memoryresource.use.append(xmlReader.readElementText());
        else
            unhandledElement(xmlReader);
    }
    return memoryresource;
}

static CppcheckLibraryData::PodType loadPodType(const QXmlStreamReader &xmlReader)
{
    CppcheckLibraryData::PodType podtype;
    podtype.name = xmlReader.attributes().value("name").toString();
    if (podtype.name.isEmpty()) {
        mandatoryAttibuteMissing(xmlReader, "name");
    }
    podtype.stdtype = xmlReader.attributes().value("stdtype").toString();
    podtype.size = xmlReader.attributes().value("size").toString();
    podtype.sign = xmlReader.attributes().value("sign").toString();
    return podtype;
}

QString CppcheckLibraryData::open(QIODevice &file)
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
            try {
                const QString elementName(xmlReader.name().toString());
                if (elementName == "def")
                    ;
                else if (elementName == "container")
                    containers.append(loadContainer(xmlReader));
                else if (elementName == "define")
                    defines.append(loadDefine(xmlReader));
                else if (elementName == "undefine")
                    undefines.append(loadUndefine(xmlReader));
                else if (elementName == "function")
                    functions.append(loadFunction(xmlReader, comments));
                else if (elementName == "memory" || elementName == "resource")
                    memoryresource.append(loadMemoryResource(xmlReader));
                else if (elementName == "podtype")
                    podtypes.append(loadPodType(xmlReader));
                else if (elementName == "smart-pointer")
                    smartPointers.append(loadSmartPointer(xmlReader));
                else if (elementName == "type-checks")
                    typeChecks.append(loadTypeChecks(xmlReader));
                else
                    unhandledElement(xmlReader);
            } catch (std::runtime_error &e) {
                return e.what();
            }
            comments.clear();
            break;
        default:
            break;
        }
    }
    if (xmlReader.hasError()) {
        return xmlReader.errorString();
    } else {
        return QString();
    }
}

static void writeContainerFunctions(QXmlStreamWriter &xmlWriter, const QString &name, int extra, const QList<struct CppcheckLibraryData::Container::Function> &functions)
{
    if (functions.isEmpty() && extra < 0)
        return;
    xmlWriter.writeStartElement(name);
    if (extra >= 0) {
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
    if (!container.opLessAllowed.isEmpty())
        xmlWriter.writeAttribute("opLessAllowed", container.opLessAllowed);
    if (!container.itEndPattern.isEmpty())
        xmlWriter.writeAttribute("itEndPattern", container.itEndPattern);

    if (!container.type.templateParameter.isEmpty() || !container.type.string.isEmpty()) {
        xmlWriter.writeStartElement("type");
        if (!container.type.templateParameter.isEmpty())
            xmlWriter.writeAttribute("templateParameter", container.type.templateParameter);
        if (!container.type.string.isEmpty())
            xmlWriter.writeAttribute("string", container.type.string);
        xmlWriter.writeEndElement();
    }
    writeContainerFunctions(xmlWriter, "size", container.size_templateParameter, container.sizeFunctions);
    writeContainerFunctions(xmlWriter, "access", container.access_arrayLike?1:-1, container.accessFunctions);
    writeContainerFunctions(xmlWriter, "other", -1, container.otherFunctions);
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
    if (!function.returnValue.empty()) {
        xmlWriter.writeStartElement("returnValue");
        if (!function.returnValue.type.isNull())
            xmlWriter.writeAttribute("type", function.returnValue.type);
        if (function.returnValue.container >= 0)
            xmlWriter.writeAttribute("container", QString::number(function.returnValue.container));
        if (!function.returnValue.value.isNull())
            xmlWriter.writeCharacters(function.returnValue.value);
        xmlWriter.writeEndElement();
    }
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
        else if (arg.nr == CppcheckLibraryData::Function::Arg::VARIADIC)
            xmlWriter.writeAttribute("nr", "variadic");
        else
            xmlWriter.writeAttribute("nr", QString::number(arg.nr));
        if (!arg.defaultValue.isNull())
            xmlWriter.writeAttribute("default", arg.defaultValue);
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

        if (arg.iterator.container >= 0 || !arg.iterator.type.isNull()) {
            xmlWriter.writeStartElement("iterator");
            if (arg.iterator.container >= 0)
                xmlWriter.writeAttribute("container", QString::number(arg.iterator.container));
            if (!arg.iterator.type.isNull())
                xmlWriter.writeAttribute("type", arg.iterator.type);
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeEndElement();
    }

    if (!function.warn.isEmpty()) {
        xmlWriter.writeStartElement("warn");

        if (!function.warn.severity.isEmpty())
            xmlWriter.writeAttribute("severity", function.warn.severity);

        if (!function.warn.cstd.isEmpty())
            xmlWriter.writeAttribute("cstd", function.warn.cstd);

        if (!function.warn.alternatives.isEmpty())
            xmlWriter.writeAttribute("alternatives", function.warn.alternatives);

        if (!function.warn.reason.isEmpty())
            xmlWriter.writeAttribute("reason", function.warn.reason);

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
        if (alloc.isRealloc) {
            xmlWriter.writeStartElement("realloc");
        } else {
            xmlWriter.writeStartElement("alloc");
        }
        xmlWriter.writeAttribute("init", alloc.init ? "true" : "false");
        if (alloc.arg != -1) {
            xmlWriter.writeAttribute("arg", QString("%1").arg(alloc.arg));
        }
        if (alloc.isRealloc && alloc.reallocArg != -1) {
            xmlWriter.writeAttribute("realloc-arg", QString("%1").arg(alloc.reallocArg));
        }
        if (mr.type == "memory" && !alloc.bufferSize.isEmpty()) {
            xmlWriter.writeAttribute("buffer-size", alloc.bufferSize);
        }
        xmlWriter.writeCharacters(alloc.name);
        xmlWriter.writeEndElement();
    }

    foreach (const CppcheckLibraryData::MemoryResource::Dealloc &dealloc, mr.dealloc) {
        xmlWriter.writeStartElement("dealloc");
        if (dealloc.arg != -1) {
            xmlWriter.writeAttribute("arg", QString("%1").arg(dealloc.arg));
        }
        xmlWriter.writeCharacters(dealloc.name);
        xmlWriter.writeEndElement();
    }

    foreach (const QString &use, mr.use) {
        xmlWriter.writeTextElement("use", use);
    }
    xmlWriter.writeEndElement();
}

static void writeTypeChecks(QXmlStreamWriter &xmlWriter, const CppcheckLibraryData::TypeChecks &typeChecks)
{
    QPair<QString, QString> check;
    xmlWriter.writeStartElement("type-checks");
    if (!typeChecks.isEmpty()) {
        xmlWriter.writeStartElement("unusedvar");
    }
    foreach (check, typeChecks) {
        xmlWriter.writeStartElement(check.first);
        xmlWriter.writeCharacters(check.second);
        xmlWriter.writeEndElement();
    }
    if (!typeChecks.isEmpty()) {
        xmlWriter.writeEndElement();
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

    foreach (const QString &undef, undefines) {
        xmlWriter.writeStartElement("undefine");
        xmlWriter.writeAttribute("name", undef);
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
        if (!podtype.stdtype.isEmpty())
            xmlWriter.writeAttribute("stdtype", podtype.stdtype);
        if (!podtype.sign.isEmpty())
            xmlWriter.writeAttribute("sign", podtype.sign);
        if (!podtype.size.isEmpty())
            xmlWriter.writeAttribute("size", podtype.size);
        xmlWriter.writeEndElement();
    }

    foreach (const TypeChecks check, typeChecks) {
        writeTypeChecks(xmlWriter, check);
    }

    foreach (const QString &smartPtr, smartPointers) {
        xmlWriter.writeStartElement("smart-pointer");
        xmlWriter.writeAttribute("class-name", smartPtr);
        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndElement();

    return outputString;
}

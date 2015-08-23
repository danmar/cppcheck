/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include "librarydialog.h"
#include "ui_librarydialog.h"

#include <QFile>
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>

const unsigned int LibraryDialog::Function::Arg::ANY = ~0U;

// TODO: get/compare functions from header

LibraryDialog::LibraryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibraryDialog)
{
    ui->setupUi(this);
    ui->buttonSave->setEnabled(false);
}

LibraryDialog::~LibraryDialog()
{
    delete ui;
}

static LibraryDialog::Function::Arg loadFunctionArg(const QDomElement &functionArgElement)
{
    LibraryDialog::Function::Arg arg;
    if (functionArgElement.attribute("nr") == "any")
        arg.nr = LibraryDialog::Function::Arg::ANY;
    else
        arg.nr = functionArgElement.attribute("nr").toUInt();
    for (QDomElement childElement = functionArgElement.firstChildElement(); !childElement.isNull(); childElement = childElement.nextSiblingElement()) {
        if (childElement.tagName() == "not-bool")
            arg.notbool = true;
        else if (childElement.tagName() == "not-null")
            arg.notnull = true;
        else if (childElement.tagName() == "not-uninit")
            arg.notuninit = true;
        else if (childElement.tagName() == "strz")
            arg.strz = true;
        else if (childElement.tagName() == "formatstr")
            arg.formatstr = true;
        else if (childElement.tagName() == "valid")
            arg.valid = childElement.text();
        else if (childElement.tagName() == "minsize") {
            arg.minsize.type = childElement.attribute("type");
            arg.minsize.arg  = childElement.attribute("arg");
            arg.minsize.arg2 = childElement.attribute("arg2");
        }
    }
    return arg;
}

static LibraryDialog::Function loadFunction(const QDomElement &functionElement)
{
    LibraryDialog::Function function;
    function.name = functionElement.attribute("name");
    for (QDomElement childElement = functionElement.firstChildElement(); !childElement.isNull(); childElement = childElement.nextSiblingElement()) {
        const QString tagName = childElement.tagName();
        if (childElement.tagName() == "noreturn")
            function.noreturn = (childElement.text() == "true");
        else if (childElement.tagName() == "pure")
            function.gccPure = true;
        else if (childElement.tagName() == "const")
            function.gccConst = true;
        else if (childElement.tagName() == "leak-ignore")
            function.leakignore = true;
        else if (childElement.tagName() == "use-retval")
            function.useretval = true;
        else if (childElement.tagName() == "formatstr") {
            function.formatstr.scan   = childElement.attribute("scan");
            function.formatstr.secure = childElement.attribute("secure");
        } else if (childElement.tagName() == "arg") {
            const LibraryDialog::Function::Arg fa = loadFunctionArg(childElement);
            function.args.append(fa);
        } else {
            int x = 123;
            x++;

        }
    }
    return function;
}

bool LibraryDialog::loadFile(QFile &file)
{
    QDomDocument doc;
    if (!doc.setContent(&file))
        return false;

    QDomElement rootElement = doc.firstChildElement("def");
    for (QDomElement functionElement = rootElement.firstChildElement("function"); !functionElement.isNull(); functionElement = functionElement.nextSiblingElement("function")) {
        functions.append(loadFunction(functionElement));
    }

    return true;
}

void LibraryDialog::openCfg()
{
    const QSettings settings;
    const QString datadir = settings.value("DATADIR",QString()).toString();

    QString selectedFilter;
    const QString filter(tr("Library files (*.cfg)"));
    const QString selectedFile = QFileDialog::getOpenFileName(this,
                                 tr("Open library file"),
                                 datadir,
                                 filter,
                                 &selectedFilter);

    if (!selectedFile.isEmpty()) {
        QFile file(selectedFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            loadFile(file);

            mFileName = selectedFile;

            ui->buttonSave->setEnabled(false);
            ui->functions->clear();
            foreach (const struct Function &function, functions)
                ui->functions->addItem(function.name);
        }
    }
}

static QDomElement FunctionElement(QDomDocument &doc, const LibraryDialog::Function &function)
{
    QDomElement functionElement = doc.createElement("function");
    functionElement.setAttribute("name", function.name);
    if (!function.noreturn) {
        QDomElement e = doc.createElement("noreturn");
        e.appendChild(doc.createTextNode("false"));
        functionElement.appendChild(e);
    }
    if (function.useretval)
        functionElement.appendChild(doc.createElement("useretval"));
    if (function.leakignore)
        functionElement.appendChild(doc.createElement("leak-ignore"));

    // Argument info..
    foreach (const LibraryDialog::Function::Arg &arg, function.args) {
        QDomElement argElement = doc.createElement("arg");
        functionElement.appendChild(argElement);
        if (arg.nr == LibraryDialog::Function::Arg::ANY)
            argElement.setAttribute("nr", "any");
        else
            argElement.setAttribute("nr", arg.nr);
        if (arg.notbool)
            argElement.appendChild(doc.createElement("not-bool"));
        if (arg.notnull)
            argElement.appendChild(doc.createElement("not-null"));
        if (arg.notuninit)
            argElement.appendChild(doc.createElement("not-uninit"));
        if (arg.strz)
            argElement.appendChild(doc.createElement("strz"));
        if (arg.formatstr)
            argElement.appendChild(doc.createElement("formatstr"));

        if (!arg.valid.isEmpty()) {
            QDomElement e = doc.createElement("valid");
            e.appendChild(doc.createTextNode(arg.valid));
            argElement.appendChild(e);
        }

        if (!arg.minsize.type.isEmpty()) {
            QDomElement e = doc.createElement("minsize");
            e.setAttribute("type", arg.minsize.type);
            e.setAttribute("arg", arg.minsize.arg);
            if (!arg.minsize.arg2.isEmpty())
                e.setAttribute("arg2", arg.minsize.arg2);
            argElement.appendChild(e);
        }
    }

    return functionElement;
}

void LibraryDialog::saveCfg()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("def");
    doc.appendChild(root);
    root.setAttribute("format","2");

    foreach (const Function &function, functions) {
        root.appendChild(FunctionElement(doc, function));
    }

    QFile file(mFileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts( &file );
        ts << doc.toString();
        ui->buttonSave->setEnabled(false);
    }
}

void LibraryDialog::selectFunction(int row)
{
    const Function &function = functions[row];
    ui->functionreturn->setChecked(!function.noreturn);
    ui->useretval->setChecked(function.useretval);
    ui->leakignore->setChecked(function.leakignore);
    ui->arguments->clear();
    foreach (const Function::Arg &arg, function.args) {
        QString s("arg");
        if (arg.nr != Function::Arg::ANY)
            s += QString::number(arg.nr);
        if (arg.formatstr)
            s += " formatstr";
        else if (arg.strz)
            s += " strz";
        else if (!arg.valid.isNull())
            s += " " + arg.valid;
        ui->arguments->addItem(s);
    }
}

void LibraryDialog::changeFunction()
{
    foreach (const QListWidgetItem *item, ui->functions->selectedItems())
    {
        Function &function = functions[ui->functions->row(item)];
        function.noreturn   = !ui->functionreturn->isChecked();
        function.useretval  = ui->useretval->isChecked();
        function.leakignore = ui->leakignore->isChecked();
    }
    ui->buttonSave->setEnabled(true);
}


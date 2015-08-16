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

const unsigned int LibraryDialog::Function::Arg::ANY = ~0U;

// TODO: get/compare functions from header

LibraryDialog::LibraryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibraryDialog)
{
    ui->setupUi(this);
}

LibraryDialog::~LibraryDialog()
{
    delete ui;
}

void LibraryDialog::updateui()
{
    ui->functions->clear();
    for (const struct Function &function : functions) {
        ui->functions->addItem(function.name);
    }
}


QStringList getFunctions(QFile &file)
{
    QStringList ret;

    QDomDocument doc;
    if (!doc.setContent(&file))
        return ret;


    return ret;
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
            updateui();
        }
    }

}

void LibraryDialog::selectFunction(int row)
{
    const Function &function = functions[row];
    ui->functionreturn->setChecked(!function.noreturn);
    ui->useretval->setChecked(function.useretval);
    ui->leakignore->setChecked(function.leakignore);
    ui->arguments->clear();
    for (const Function::Arg &arg : function.args) {
        QString s("arg");
        if (arg.nr != Function::Arg::ANY)
            s += QString::number(arg.nr);
        if (arg.formatstr)
            s += " formatstr";
        else if (!arg.strz)
            s += " strz";
        else if (!arg.valid.isNull())
            s += " " + arg.valid;
        ui->arguments->addItem(s);
    }
}

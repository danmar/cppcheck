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
#include "libraryaddfunctiondialog.h"

#include <QFile>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>
#include <QInputDialog>

// TODO: get/compare functions from header

LibraryDialog::LibraryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibraryDialog),
    ignoreChanges(false)
{
    ui->setupUi(this);
    ui->buttonSave->setEnabled(false);
}

LibraryDialog::~LibraryDialog()
{
    delete ui;
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
        mFileName.clear();
        QFile file(selectedFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ignoreChanges = true;
            data.open(file);
            mFileName = selectedFile;
            ui->buttonSave->setEnabled(false);
            ui->functions->clear();
            foreach(const struct LibraryData::Function &function, data.functions) {
                ui->functions->addItem(function.name);
            }
            ignoreChanges = false;
        }
    }
}

void LibraryDialog::saveCfg()
{
    if (mFileName.isNull())
        return;
    QFile file(mFileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&file);
        ts << data.toString();
        ui->buttonSave->setEnabled(false);
    }
}

void LibraryDialog::addFunction()
{
    LibraryAddFunctionDialog *d = new LibraryAddFunctionDialog;
    if (d->exec() != QDialog::Accepted) {
        delete d;
        return;
    }

    LibraryData::Function f;
    f.name = d->functionName();
    int args = d->numberOfArguments();

    for (int i = 1; i <= args; i++) {
        LibraryData::Function::Arg arg;
        arg.nr = i;
        f.args.append(arg);
    }
    data.functions.append(f);
    ui->functions->addItem(f.name);
}

void LibraryDialog::selectFunction(int row)
{
    if (row == -1) {
        ui->functionreturn->setChecked(false);
        ui->useretval->setChecked(false);
        ui->leakignore->setChecked(false);
        ui->arguments->clear();
        return;
    }

    ignoreChanges = true;
    const LibraryData::Function &function = data.functions[row];
    ui->functionreturn->setChecked(!function.noreturn);
    ui->useretval->setChecked(function.useretval);
    ui->leakignore->setChecked(function.leakignore);
    ui->arguments->clear();
    foreach(const LibraryData::Function::Arg &arg, function.args) {
        QString s("arg");
        if (arg.nr != LibraryData::Function::Arg::ANY)
            s += QString::number(arg.nr);
        ui->arguments->addItem(s);

        QListWidgetItem *item = new QListWidgetItem(tr("Not bool"), ui->arguments);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(arg.notbool ? Qt::Checked : Qt::Unchecked);
        ui->arguments->addItem(item);

        item = new QListWidgetItem(tr("Not null"), ui->arguments);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(arg.notnull ? Qt::Checked : Qt::Unchecked);
        ui->arguments->addItem(item);

        item = new QListWidgetItem(tr("Not uninit"), ui->arguments);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(arg.notuninit ? Qt::Checked : Qt::Unchecked);
        ui->arguments->addItem(item);

        item = new QListWidgetItem(tr("Format string"), ui->arguments);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(arg.formatstr ? Qt::Checked : Qt::Unchecked);
        ui->arguments->addItem(item);

        item = new QListWidgetItem(tr("Zero-terminated string"), ui->arguments);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(arg.strz ? Qt::Checked : Qt::Unchecked);
        ui->arguments->addItem(item);

        ui->arguments->addItem("valid: " + ((!arg.valid.isNull()) ? arg.valid : "*"));
    }
    ignoreChanges = false;
}

void LibraryDialog::changeFunction()
{
    if (ignoreChanges)
        return;
    foreach(const QListWidgetItem *item, ui->functions->selectedItems()) {
        LibraryData::Function &function = data.functions[ui->functions->row(item)];
        function.noreturn   = !ui->functionreturn->isChecked();
        function.useretval  = ui->useretval->isChecked();
        function.leakignore = ui->leakignore->isChecked();
    }
    ui->buttonSave->setEnabled(true);
}


void LibraryDialog::argumentChanged(QListWidgetItem *changedItem)
{
    if (ignoreChanges)
        return;
    unsigned argnr = 0;
    for (int row = 0; row < ui->arguments->count(); row++) {
        const QListWidgetItem *argItem = ui->arguments->item(row);
        if (argItem == changedItem)
            break;
        if (argItem->text() == "arg")
            argnr = LibraryData::Function::Arg::ANY;
        else if (argItem->text().startsWith("arg"))
            argnr = argItem->text().mid(3).toInt();
    }

    foreach(const QListWidgetItem *functionItem, ui->functions->selectedItems()) {
        LibraryData::Function &function = data.functions[ui->functions->row(functionItem)];

        for (LibraryData::Function::Arg &arg : function.args) {
            if (arg.nr == argnr) {
                // TODO: Don't use a stringbased lookup
                if (changedItem->text() == "Not bool")
                    arg.notbool = (changedItem->checkState() != Qt::Unchecked);
                else if (changedItem->text() == "Not null")
                    arg.notnull = (changedItem->checkState() != Qt::Unchecked);
                else if (changedItem->text() == "Not uninit")
                    arg.notuninit = (changedItem->checkState() != Qt::Unchecked);
                else if (changedItem->text() == "Format string")
                    arg.formatstr = (changedItem->checkState() != Qt::Unchecked);
                else if (changedItem->text() == "Zero-terminated string")
                    arg.strz = (changedItem->checkState() != Qt::Unchecked);
                break;
            }
        }
    }

    ui->buttonSave->setEnabled(true);
}



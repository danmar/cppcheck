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
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>

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
            data.open(file);
            mFileName = selectedFile;
            ui->buttonSave->setEnabled(false);
            ui->functions->clear();
            foreach(const struct LibraryData::Function &function, data.functions)
            ui->functions->addItem(function.name);
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

void LibraryDialog::selectFunction(int row)
{
    const LibraryData::Function &function = data.functions[row];
    ui->functionreturn->setChecked(!function.noreturn);
    ui->useretval->setChecked(function.useretval);
    ui->leakignore->setChecked(function.leakignore);
    ui->arguments->clear();
    foreach(const LibraryData::Function::Arg &arg, function.args) {
        QString s("arg");
        if (arg.nr != LibraryData::Function::Arg::ANY)
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
    foreach(const QListWidgetItem *item, ui->functions->selectedItems()) {
        LibraryData::Function &function = data.functions[ui->functions->row(item)];
        function.noreturn   = !ui->functionreturn->isChecked();
        function.useretval  = ui->useretval->isChecked();
        function.leakignore = ui->leakignore->isChecked();
    }
    ui->buttonSave->setEnabled(true);
}


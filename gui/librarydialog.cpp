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

#include "librarydialog.h"
#include "ui_librarydialog.h"
#include "libraryaddfunctiondialog.h"
#include "libraryeditargdialog.h"
#include "path.h"

#include <QFile>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>
#include <QInputDialog>
#include <QMessageBox>

// TODO: get/compare functions from header

class FunctionListItem : public QListWidgetItem {
public:
    FunctionListItem(QListWidget *view,
                     CppcheckLibraryData::Function *function,
                     bool selected)
        : QListWidgetItem(view), function(function) {
        setText(function->name);
        setFlags(flags() | Qt::ItemIsEditable);
        setSelected(selected);
    }
    CppcheckLibraryData::Function *function;
};

LibraryDialog::LibraryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibraryDialog),
    ignoreChanges(false)
{
    ui->setupUi(this);
    ui->buttonSave->setEnabled(false);
    ui->buttonSaveAs->setEnabled(false);
    ui->sortFunctions->setEnabled(false);
    ui->filter->setEnabled(false);
    ui->addFunction->setEnabled(false);

    //As no function selected, this disables function editing widgets
    selectFunction();
}

LibraryDialog::~LibraryDialog()
{
    delete ui;
}

CppcheckLibraryData::Function *LibraryDialog::currentFunction()
{
    QList<QListWidgetItem *> selitems = ui->functions->selectedItems();
    if (selitems.count() != 1)
        return nullptr;
    return dynamic_cast<FunctionListItem *>(selitems.first())->function;
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
            ignoreChanges = true;
            data.open(file);
            mFileName = selectedFile;
            ui->buttonSave->setEnabled(false);
            ui->buttonSaveAs->setEnabled(true);
            ui->filter->clear();
            ui->functions->clear();
            for (struct CppcheckLibraryData::Function &function : data.functions) {
                ui->functions->addItem(new FunctionListItem(ui->functions,
                                       &function,
                                       false));
            }
            ui->sortFunctions->setEnabled(!data.functions.empty());
            ui->filter->setEnabled(!data.functions.empty());
            ui->addFunction->setEnabled(true);
            ignoreChanges = false;
        } else {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Cppcheck"),
                            tr("Can not open file %1.").arg(selectedFile),
                            QMessageBox::Ok,
                            this);
            msg.exec();
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
        ts << data.toString() << '\n';
        ui->buttonSave->setEnabled(false);
    } else {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Can not save file %1.").arg(mFileName),
                        QMessageBox::Ok,
                        this);
        msg.exec();
    }
}

void LibraryDialog::saveCfgAs()
{
    const QString filter(tr("Library files (*.cfg)"));
    const QString path = Path::getPathFromFilename(mFileName.toStdString()).c_str();
    QString selectedFile = QFileDialog::getSaveFileName(this,
                           tr("Save the library as"),
                           path,
                           filter);
    if (selectedFile.isEmpty())
        return;

    if (!selectedFile.endsWith(".cfg", Qt::CaseInsensitive))
        selectedFile += ".cfg";

    mFileName = selectedFile;
    saveCfg();
}

void LibraryDialog::addFunction()
{
    LibraryAddFunctionDialog *d = new LibraryAddFunctionDialog;
    if (d->exec() == QDialog::Accepted && !d->functionName().isEmpty()) {

        CppcheckLibraryData::Function f;
        f.name = d->functionName();
        int args = d->numberOfArguments();

        for (int i = 1; i <= args; i++) {
            CppcheckLibraryData::Function::Arg arg;
            arg.nr = i;
            f.args.append(arg);
        }
        data.functions.append(f);
        ui->functions->addItem(new FunctionListItem(ui->functions, &data.functions.back(), false));
        ui->buttonSave->setEnabled(true);
        ui->sortFunctions->setEnabled(!data.functions.empty());
        ui->filter->setEnabled(!data.functions.empty());
    }
    delete d;
}

void LibraryDialog::editFunctionName(QListWidgetItem* item)
{
    if (ignoreChanges)
        return;
    QString functionName = item->text();
    CppcheckLibraryData::Function * const function = dynamic_cast<FunctionListItem*>(item)->function;
    if (functionName != function->name) {
        if (QRegExp(NAMES).exactMatch(functionName)) {
            function->name = functionName;
            ui->buttonSave->setEnabled(true);
        } else {
            ignoreChanges = true;
            item->setText(function->name);
            ignoreChanges = false;
        }
    }
}

void LibraryDialog::selectFunction()
{
    const CppcheckLibraryData::Function * const function = currentFunction();

    if (function == nullptr) {
        ui->comments->clear();
        ui->comments->setEnabled(false);

        ui->noreturn->setCurrentIndex(0);
        ui->noreturn->setEnabled(false);

        ui->useretval->setChecked(false);
        ui->useretval->setEnabled(false);

        ui->leakignore->setChecked(false);
        ui->leakignore->setEnabled(false);

        ui->arguments->clear();
        ui->arguments->setEnabled(false);

        ui->editArgButton->setEnabled(false);
        return;
    }

    ignoreChanges = true;
    ui->comments->setPlainText(function->comments);
    ui->comments->setEnabled(true);

    ui->noreturn->setCurrentIndex(function->noreturn);
    ui->noreturn->setEnabled(true);

    ui->useretval->setChecked(function->useretval);
    ui->useretval->setEnabled(true);

    ui->leakignore->setChecked(function->leakignore);
    ui->leakignore->setEnabled(true);

    updateArguments(*function);
    ui->arguments->setEnabled(true);

    ui->editArgButton->setEnabled(true);
    ignoreChanges = false;
}

void LibraryDialog::sortFunctions(bool sort)
{
    if (sort) {
        ui->functions->sortItems();
    } else {
        ignoreChanges = true;
        CppcheckLibraryData::Function *selfunction = currentFunction();
        ui->functions->clear();
        for (struct CppcheckLibraryData::Function &function : data.functions) {
            ui->functions->addItem(new FunctionListItem(ui->functions,
                                   &function,
                                   selfunction == &function));
        }
        if (!ui->filter->text().isEmpty())
            filterFunctions(ui->filter->text());
        ignoreChanges = false;
    }
}

void LibraryDialog::filterFunctions(QString filter)
{
    QList<QListWidgetItem *> allItems = ui->functions->findItems(QString(), Qt::MatchContains);

    if (filter.isEmpty()) {
        foreach (QListWidgetItem *item, allItems) {
            item->setHidden(false);
        }
    } else {
        foreach (QListWidgetItem *item, allItems) {
            item->setHidden(!item->text().startsWith(filter));
        }
    }
}

void LibraryDialog::changeFunction()
{
    if (ignoreChanges)
        return;

    CppcheckLibraryData::Function *function = currentFunction();
    if (!function)
        return;

    function->comments   = ui->comments->toPlainText();
    function->noreturn   = (CppcheckLibraryData::Function::TrueFalseUnknown)ui->noreturn->currentIndex();
    function->useretval  = ui->useretval->isChecked();
    function->leakignore = ui->leakignore->isChecked();

    ui->buttonSave->setEnabled(true);
}

void LibraryDialog::editArg()
{
    CppcheckLibraryData::Function *function = currentFunction();
    if (!function)
        return;

    if (ui->arguments->selectedItems().count() != 1)
        return;
    CppcheckLibraryData::Function::Arg &arg = function->args[ui->arguments->row(ui->arguments->selectedItems().first())];

    LibraryEditArgDialog *d = new LibraryEditArgDialog(0, arg);
    if (d->exec() == QDialog::Accepted) {
        unsigned number = arg.nr;
        arg = d->getArg();
        arg.nr = number;
        ui->arguments->selectedItems().first()->setText(getArgText(arg));
    }

    delete d;
    ui->buttonSave->setEnabled(true);
}

QString LibraryDialog::getArgText(const CppcheckLibraryData::Function::Arg &arg)
{
    QString s("arg");
    if (arg.nr != CppcheckLibraryData::Function::Arg::ANY)
        s += QString::number(arg.nr);

    s += "\n    not bool: " + QString(arg.notbool ? "true" : "false");
    s += "\n    not null: " + QString(arg.notnull ? "true" : "false");
    s += "\n    not uninit: " + QString(arg.notuninit ? "true" : "false");
    s += "\n    format string: " + QString(arg.formatstr ? "true" : "false");
    s += "\n    strz: " + QString(arg.strz ? "true" : "false");
    s += "\n    valid: " + QString(arg.valid.isEmpty() ? "any" : arg.valid);
    foreach (const CppcheckLibraryData::Function::Arg::MinSize &minsize, arg.minsizes) {
        s += "\n    minsize: " + minsize.type + " " + minsize.arg + " " + minsize.arg2;
    }
    return s;
}

void LibraryDialog::updateArguments(const CppcheckLibraryData::Function &function)
{
    ui->arguments->clear();
    foreach (const CppcheckLibraryData::Function::Arg &arg, function.args) {
        ui->arguments->addItem(getArgText(arg));
    }
}

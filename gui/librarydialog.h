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

#ifndef LIBRARYDIALOG_H
#define LIBRARYDIALOG_H

#include <QDialog>
#include <QFile>

#include "cppchecklibrarydata.h"

class QListWidgetItem;

namespace Ui {
    class LibraryDialog;
}

class LibraryDialog : public QDialog {
    Q_OBJECT

public:
    explicit LibraryDialog(QWidget *parent = 0);
    ~LibraryDialog();

private slots:
    void openCfg();
    void saveCfg();
    void saveCfgAs();
    void addFunction();
    void changeFunction();
    void editArg();
    void editFunctionName(QListWidgetItem*);
    void filterFunctions(QString);
    void selectFunction();
    void sortFunctions(bool);

private:
    Ui::LibraryDialog *ui;
    CppcheckLibraryData data;
    QString mFileName;
    bool ignoreChanges;

    static QString getArgText(const CppcheckLibraryData::Function::Arg &arg);
    CppcheckLibraryData::Function *currentFunction();
    void updateArguments(const CppcheckLibraryData::Function &function);
};

#endif // LIBRARYDIALOG_H

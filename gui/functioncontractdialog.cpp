/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "functioncontractdialog.h"

#include "ui_functioncontractdialog.h"

static QString formatFunctionName(QString f)
{
    if (f.endsWith("()"))
        return f;
    f.replace("(", "(\n    ");
    f.replace(",", ",\n    ");
    return f;
}

FunctionContractDialog::FunctionContractDialog(QWidget *parent, const QString &name, const QString &expects) :
    QDialog(parent),
    mUi(new Ui::FunctionContractDialog)
{
    mUi->setupUi(this);
    mUi->functionName->setText(formatFunctionName(name));
    mUi->expects->setPlainText(expects);
}

FunctionContractDialog::~FunctionContractDialog()
{
    delete mUi;
}

QString FunctionContractDialog::getExpects() const
{
    return mUi->expects->toPlainText();
}


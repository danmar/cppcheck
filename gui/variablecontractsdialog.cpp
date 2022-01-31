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

#include "variablecontractsdialog.h"
#include "ui_variablecontractsdialog.h"

#include <QRegExpValidator>

VariableContractsDialog::VariableContractsDialog(QWidget *parent, QString var) :
    QDialog(parent),
    mUI(new Ui::VariableContractsDialog)
{
    mUI->setupUi(this);

    mVarName = var.indexOf(" ") < 0 ? var : var.mid(0, var.indexOf(" "));

    this->setWindowTitle(mVarName);

    auto getMinMax = [](const QString& var, const QString& minmax) {
        if (var.indexOf(" " + minmax + ":") < 0)
            return QString();
        int pos1 = var.indexOf(" " + minmax + ":") + 2 + minmax.length();
        int pos2 = var.indexOf(" ", pos1);
        if (pos2 < 0)
            return var.mid(pos1);
        return var.mid(pos1, pos2-pos1);
    };

    mUI->mMinValue->setText(getMinMax(var, "min"));
    mUI->mMaxValue->setText(getMinMax(var, "max"));

    mUI->mMinValue->setValidator(new QRegExpValidator(QRegExp("-?[0-9]*")));
    mUI->mMaxValue->setValidator(new QRegExpValidator(QRegExp("-?[0-9]*")));
}

VariableContractsDialog::~VariableContractsDialog()
{
    delete mUI;
}

QString VariableContractsDialog::getVarname() const
{
    return mVarName;
}
QString VariableContractsDialog::getMin() const
{
    return mUI->mMinValue->text();
}
QString VariableContractsDialog::getMax() const
{
    return mUI->mMaxValue->text();
}

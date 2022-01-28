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

#ifndef VARIABLECONSTRAINTSDIALOG_H
#define VARIABLECONSTRAINTSDIALOG_H

#include <QDialog>

namespace Ui {
    class VariableContractsDialog;
}

class VariableContractsDialog : public QDialog {
    Q_OBJECT

public:
    explicit VariableContractsDialog(QWidget *parent, QString var);
    ~VariableContractsDialog();

    QString getVarname() const;
    QString getMin() const;
    QString getMax() const;

private:
    Ui::VariableContractsDialog *mUI;
    QString mVarName;
};

#endif // VARIABLECONSTRAINTSDIALOG_H

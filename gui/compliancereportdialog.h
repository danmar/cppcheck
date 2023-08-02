/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#ifndef COMPLIANCEREPORTDIALOG_H
#define COMPLIANCEREPORTDIALOG_H

#include <QDialog>
#include <QObject>
#include <QString>

namespace Ui {
    class ComplianceReportDialog;
}

class ProjectFile;
class QAbstractButton;

class ComplianceReportDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ComplianceReportDialog(ProjectFile* projectFile, QString resultsFile);
    ~ComplianceReportDialog() final;

private slots:
    void buttonClicked(QAbstractButton* button);

private:
    void save();

    Ui::ComplianceReportDialog *mUI;
    ProjectFile* mProjectFile;
    QString mResultsFile;
};

#endif // COMPLIANCEREPORTDIALOG_H

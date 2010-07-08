/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

#ifndef PROJECTFILE_DIALOG_H
#define PROJECTFILE_DIALOG_H

#include <QDialog>
#include <QString>

#include "ui_projectfile.h"

class ProjectFile;

/// @addtogroup GUI
/// @{

class ProjectFileDialog : public QDialog
{
    Q_OBJECT
public:
    ProjectFileDialog(const QString &path, QWidget *parent = 0);

protected slots:
    void DialogAccepted();

protected:
    void ReadProjectFile();
    void UpdateProjectFileData();

private:
    Ui::ProjectFile mUI;
    QString mFileName;
    ProjectFile *mPFile;
    bool mDataSaved;
};

/// @}
#endif // PROJECTFILE_DIALOG_H

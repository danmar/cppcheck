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
#include <QStringList>

#include "ui_projectfile.h"

class ProjectFile;

/// @addtogroup GUI
/// @{


/**
* @brief A dialog for editing project file data.
*/
class ProjectFileDialog : public QDialog
{
    Q_OBJECT
public:
    ProjectFileDialog(const QString &path, QWidget *parent = 0);

    /**
    * @brief Return project root path from the dialog control.
    * @return Project root path.
    */
    QString GetRootPath() const;

    /**
    * @brief Return include paths from the dialog control.
    * @return List of include paths.
    */
    QStringList GetIncludePaths() const;

    /**
    * @brief Return define names from the dialog control.
    * @return List of define names.
    */
    QStringList GetDefines() const;

    /**
    * @brief Return check paths from the dialog control.
    * @return List of check paths.
    */
    QStringList GetPaths() const;

    /**
    * @brief Set project root path to dialog control.
    * @param root Project root path to set to dialog control.
    */
    void SetRootPath(const QString &root);

    /**
    * @brief Set include paths to dialog control.
    * @param includes List of include paths to set to dialog control.
    */
    void SetIncludepaths(const QStringList &includes);

    /**
    * @brief Set define names to dialog control.
    * @param defines List of define names to set to dialog control.
    */
    void SetDefines(const QStringList &defines);

    /**
    * @brief Set check paths to dialog control.
    * @param paths List of path names to set to dialog control.
    */
    void SetPaths(const QStringList &paths);

private:
    Ui::ProjectFile mUI;
};

/// @}
#endif // PROJECTFILE_DIALOG_H

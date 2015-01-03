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

#ifndef PROJECTFILE_DIALOG_H
#define PROJECTFILE_DIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QCheckBox>

#include "ui_projectfile.h"

class QWidget;

/// @addtogroup GUI
/// @{


/**
* @brief A dialog for editing project file data.
*/
class ProjectFileDialog : public QDialog {
    Q_OBJECT
public:
    ProjectFileDialog(const QString &path, QWidget *parent = 0);
    virtual ~ProjectFileDialog();

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
    * @brief Return excluded paths from the dialog control.
    * @return List of excluded paths.
    */
    QStringList GetExcludedPaths() const;

    /**
    * @brief Return selected libraries from the dialog control.
    * @return List of libraries.
    */
    QStringList GetLibraries() const;

    /**
    * @brief Return suppressions from the dialog control.
    * @return List of suppressions.
    */
    QStringList GetSuppressions() const;

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

    /**
    * @brief Set excluded paths to dialog control.
    * @param paths List of path names to set to dialog control.
    */
    void SetExcludedPaths(const QStringList &paths);

    /**
    * @brief Set libraries to dialog control.
    * @param libraries List of libraries to set to dialog control.
    */
    void SetLibraries(const QStringList &libraries);

    /**
    * @brief Set suppressions to dialog control.
    * @param suppressions List of suppressions to set to dialog control.
    */
    void SetSuppressions(const QStringList &suppressions);

protected slots:
    /**
    * @brief Browse for include directory.
    * Allow user to add new include directory to the list.
    */
    void AddIncludeDir();

    /**
    * @brief Add new path to check.
    */
    void AddPath();

    /**
    * @brief Remove include directory from the list.
    */
    void RemoveIncludeDir();

    /**
    * @brief Edit include directory in the list.
    */
    void EditIncludeDir();

    /**
    * @brief Edit path in the list.
    */
    void EditPath();

    /**
    * @brief Remove path from the list.
    */
    void RemovePath();

    /**
    * @brief Add new path to exclude.
    */
    void AddExcludePath();

    /**
    * @brief Edit excluded path in the list.
    */
    void EditExcludePath();

    /**
    * @brief Remove excluded path from the list.
    */
    void RemoveExcludePath();

    /**
      * @brief Move include path up in the list.
      */
    void MoveIncludePathUp();

    /**
      * @brief Move include path down in the list.
      */
    void MoveIncludePathDown();

    /**
    * @brief Add suppression to the list
    */
    void AddSuppression();

    /**
    * @brief Remove selected suppression from the list
    */
    void RemoveSuppression();

protected:

    /**
     * @brief Save dialog settings.
     */
    void LoadSettings();

    /**
     * @brief Load dialog settings.
     */
    void SaveSettings() const;

    /**
    * @brief Add new indlude directory.
    * @param dir Directory to add.
    */
    void AddIncludeDir(const QString &dir);

    /**
    * @brief Add new path to check.
    * @param path Path to add.
    */
    void AddPath(const QString &path);

    /**
    * @brief Add new path to ignore list.
    * @param path Path to add.
    */
    void AddExcludePath(const QString &path);

private:
    Ui::ProjectFile mUI;

    /**
     * @brief Projectfile path.
     */
    QString mFilePath;

    /** @brief Library checkboxes */
    QList<QCheckBox*> mLibraryCheckboxes;
};

/// @}
#endif // PROJECTFILE_DIALOG_H

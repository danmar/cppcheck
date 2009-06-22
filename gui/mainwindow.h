/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QAction>
#include <QFileDialog>

#include "resultsview.h"
#include "settingsdialog.h"

class ThreadHandler;

/**
* @brief Main window for cppcheck-gui
*
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    virtual ~MainWindow();

public slots:

    /**
    * @brief Slot for check files menu item
    *
    */
    void CheckFiles();

    /**
    * @brief Slot to recheck files
    *
    */
    void ReCheck();

    /**
    * @brief Slot to clear all search results
    *
    */
    void ClearResults();

    /**
    * @brief Show errors with type "all"
    * @param checked Should errors be shown (truw) or hidden (false)
    */
    void ShowAll(bool checked);

    /**
    * @brief Show errors with type "security"
    * @param checked Should errors be shown (truw) or hidden (false)
    */
    void ShowSecurity(bool checked);

    /**
    * @brief Show errors with type "style"
    * @param checked Should errors be shown (truw) or hidden (false)
    */
    void ShowStyle(bool checked);

    /**
    * @brief Show errors with type "unused"
    * @param checked Should errors be shown (truw) or hidden (false)
    */
    void ShowUnused(bool checked);

    /**
    * @brief Show errors with type "error"
    * @param checked Should errors be shown (truw) or hidden (false)
    */
    void ShowErrors(bool checked);

    /**
    * @brief Slot to check all "Show errors" menu items
    */
    void CheckAll();

    /**
    * @brief Slot to uncheck all "Show errors" menu items
    */
    void UncheckAll();

    /**
    * @brief Slot for check directory menu item
    *
    */
    void CheckDirectory();

    /**
    * @brief Slot to open program's settings dialog
    *
    */
    void ProgramSettings();

    /**
    * @brief Slot to open program's about dialog
    *
    */
    void About();

    /**
    * @brief Slot to to show license text
    *
    */
    void ShowLicense();

    /**
    * @brief Slot to to show authors list
    *
    */
    void ShowAuthors();

    /**
    * @brief Slot to stop processing files
    *
    */
    void Save();

protected slots:

    /**
    * @brief Slot for checkthread's done signal
    *
    */
    void CheckDone();

    /**
    * @brief Slot for enabling save and clear button
    *
    */
    void ResultsAdded();
protected:

    /**
    * @brief Create main window menus.
    */
    void CreateMenus();

    /**
    * @brief Create main window toolbar.
    */
    void CreateToolbar();

    /**
    * @brief Event coming when application is about to close.
    */
    virtual void closeEvent(QCloseEvent *event);

    /**
    * @brief Helper function to toggle all show error menu items
    * @param checked Should all errors be shown (true) or hidden (false)
    */
    void ToggleAllChecked(bool checked);

    /**
    * @brief Helper function to enable/disable all check,recheck buttons
    *
    */
    void EnableCheckButtons(bool enable);

    /**
    * @brief Select files/or directory to check.
    * Helper function to open a dialog to ask user to select files or
    * directory to check. Use native dialogs instead of QT:s own dialogs.
    *
    * @param mode Dialog open mode (files or directories)
    */
    void DoCheckFiles(QFileDialog::FileMode mode);

    /**
    * @brief Get all files recursively from given path
    *
    * @param path Path to get files from
    * @return List of file paths
    */
    QStringList GetFilesRecursively(const QString &path);

    /**
    * @brief Get our default cppcheck settings and read project file.
    *
    * @return Default cppcheck settings
    */
    Settings GetCppcheckSettings();

    /**
    * @brief Removes all unaccepted (by cppcheck core) files from the list
    *
    * @param list List to remove unaccepted files from
    * @return List of files that are all accepted by cppcheck core
    */
    QStringList RemoveUnacceptedFiles(const QStringList &list);

    /**
    * @brief Load program settings
    *
    */
    void LoadSettings();

    /**
    * @brief Save program settings
    *
    */
    void SaveSettings();

    /**
    * @brief Program settings
    *
    */
    QSettings mSettings;

    /**
    * @brief Menu action to exit program
    *
    */
    QAction mActionExit;

    /**
    * @brief Menu action to check files
    *
    */
    QAction mActionCheckFiles;

    /**
    * @brief Menu action to clear results
    *
    */
    QAction mActionClearResults;

    /**
    * @brief Menu action to re check
    *
    */
    QAction mActionReCheck;

    /**
    * @brief Menu action to check a directory
    *
    */
    QAction mActionCheckDirectory;

    /**
    * @brief Menu action to open settings dialog
    *
    */
    QAction mActionSettings;

    /**
    * @brief Action to show errors with type "all"
    *
    */
    QAction mActionShowAll;

    /**
    * @brief Action to show errors with type "security"
    *
    */
    QAction mActionShowSecurity;

    /**
    * @brief Action to show errors with type "style"
    *
    */
    QAction mActionShowStyle;

    /**
    * @brief Action to show errors with type "unused"
    *
    */
    QAction mActionShowUnused;

    /**
    * @brief Action to show errors with type "error"
    *
    */
    QAction mActionShowErrors;

    /**
    * @brief Action to check all "show error" menu items
    *
    */
    QAction mActionShowCheckAll;

    /**
    * @brief Action to uncheck all "show error" menu items
    *
    */
    QAction mActionShowUncheckAll;

    /**
    * @brief Action to collapse all items in the result tree.
    *
    */
    QAction mActionShowCollapseAll;

    /**
    * @brief Action to expand all items in the result tree.
    *
    */
    QAction mActionShowExpandAll;

    /**
    * @brief Action to show about dialog
    *
    */
    QAction mActionAbout;

    /**
    * @brief Action to show license text
    *
    */
    QAction mActionShowLicense;

    /**
    * @brief Action to show authors list
    *
    */
    QAction mActionShowAuthors;

    /**
    * @brief Action stop checking files
    *
    */
    QAction mActionStop;

    /**
    * @brief Action save found errors to a file
    *
    */
    QAction mActionSave;

    /**
    * @brief Results for checking
    *
    */
    ResultsView mResults;

    /**
    * @brief Thread to check files
    *
    */
    ThreadHandler *mThread;

    /**
    * @brief List of user defined applications to open errors with
    *
    */
    ApplicationList mApplications;

private:

    /**
    * @brief Current checked directory.
    */
    QString mCurrentDirectory;
};

#endif // MAINWINDOW_H

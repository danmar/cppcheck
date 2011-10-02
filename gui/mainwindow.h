/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QSignalMapper>
#include <QActionGroup>
#include <QTimer>
#include <QToolBar>
#include <QStringList>

#include "resultsview.h"
#include "settingsdialog.h"
#include "translationhandler.h"
#include "settings.h"
#include "platforms.h"
#include "ui_main.h"

class ThreadHandler;
class LogView;
class Project;
class ErrorItem;
class StatsDialog;
class QAction;

/// @addtogroup GUI
/// @{

/**
* @brief Main window for cppcheck-gui
*
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:

    /**
    * @brief Maximum number of MRU project items in File-menu.
    */
    enum { MaxRecentProjects = 5 };

    MainWindow();
    virtual ~MainWindow();

    /**
      * List of checked platforms.
      */
    Platforms mPlatforms;

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
    * @brief Slot to open XML report file
    *
    */
    void OpenXML();

    /**
    * @brief Show errors with type "style"
    * @param checked Should errors be shown (true) or hidden (false)
    */
    void ShowStyle(bool checked);

    /**
    * @brief Show errors with type "error"
    * @param checked Should errors be shown (true) or hidden (false)
    */
    void ShowErrors(bool checked);

    /**
    * @brief Show errors with type "warning"
    * @param checked Should errors be shown (true) or hidden (false)
    */
    void ShowWarnings(bool checked);

    /**
    * @brief Show errors with type "portability"
    * @param checked Should errors be shown (true) or hidden (false)
    */
    void ShowPortability(bool checked);

    /**
    * @brief Show errors with type "performance"
    * @param checked Should errors be shown (true) or hidden (false)
    */
    void ShowPerformance(bool checked);

    /**
    * @brief Show errors with type "information"
    * @param checked Should errors be shown (true) or hidden (false)
    */
    void ShowInformation(bool checked);

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

    /**
      * @brief Check the project.
      * @param project Pointer to the project to check.
      */
    void CheckProject(Project *project);

    /**
    * @brief Slot to create new project file..
    *
    */
    void NewProjectFile();

    /**
    * @brief Slot to open project file and start checking contained paths.
    *
    */
    void OpenProjectFile();

    /**
    * @brief Slot to close open project file.
    *
    */
    void CloseProjectFile();

    /**
    * @brief Slot to edit project file.
    *
    */
    void EditProjectFile();

    /**
    * @brief Slot for showing the log view.
    *
    */
    void ShowLogView();

    /**
    * @brief Slot for showing the scan and project statistics.
    *
    */
    void ShowStatistics();

protected slots:

    /**
    * @brief Slot for checkthread's done signal
    *
    */
    void CheckDone();

    /**
    * @brief Lock down UI while checking
    *
    */
    void CheckLockDownUI();

    /**
    * @brief Slot for enabling save and clear button
    *
    */
    void ResultsAdded();

    /**
    * @brief Slot for showing/hiding standard toolbar
    */
    void ToggleMainToolBar();

    /**
    * @brief Slot for showing/hiding Categories toolbar
    */
    void ToggleViewToolBar();

    /**
    * @brief Slot for showing/hiding Filter toolbar
    */
    void ToggleFilterToolBar();

    /**
    * @brief Slot for updating View-menu before it is shown.
    */
    void AboutToShowViewMenu();

    /**
    * @brief Slot when stop checking button is pressed
    *
    */
    void StopChecking();

    /**
    * @brief Open help file contents
    *
    */
    void OpenHelpContents();

    /**
    * @brief Add new line to log.
    *
    */
    void Log(const QString &logline);

    /**
    * @brief Handle new debug error.
    *
    */
    void DebugError(const ErrorItem &item);

    /**
    * @brief Filters the results in the result list.
    */
    void FilterResults();

    /**
    * @brief Opens recently opened project file.
    */
    void OpenRecentProject();

    /**
    * @brief Selects the platform as checked platform.
    */
    void SelectPlatform();

protected:

    /**
    * @brief Set current language
    * @param code Language code of the language to set (e.g. "en").
    */
    void SetLanguage(const QString &code);

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
    * @return QStringList of files or directories that were selected to check
    */
    QStringList SelectFilesToCheck(QFileDialog::FileMode mode);

    /**
    * @brief Check all files specified in parameter files
    *
    * @param files List of files and/or directories to check
    */
    void DoCheckFiles(const QStringList &files);

    /**
    * @brief Get our default cppcheck settings and read project file.
    *
    * @return Default cppcheck settings
    */
    Settings GetCppcheckSettings();

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
    * @brief Format main window title.
    * @param text Text added to end of the title.
    */
    void FormatAndSetTitle(const QString &text = QString());

    /**
    * @brief Show help contents
    */
    void OpenOnlineHelp();

    /**
    * @brief Enable or disable project file actions.
    * Project editing and closing actions should be only enabled when project is
    * open and we are not checking files.
    * @param enable If true then actions are enabled.
    */
    void EnableProjectActions(bool enable);

    /**
    * @brief Enable or disable project file actions.
    * Project opening and creating actions should be disabled when checking.
    * @param enable If true then actions are enabled.
    */
    void EnableProjectOpenActions(bool enable);

    /**
    * @brief Add include directories.
    * @param includeDirs List of include directories to add.
    * @param result Settings class where include directories are added.
    */
    void AddIncludeDirs(const QStringList &includeDirs, Settings &result);

    /**
    * @brief Handle command line parameters given to GUI.
    * @param params List of string given to command line.
    */
    void HandleCLIParams(const QStringList &params);

    /**
    * @brief Load project file to the GUI.
    * @param filePath Filename (inc. path) of project file to load.
    */
    void LoadProjectFile(const QString &filePath);

    /**
    * @brief Update project MRU items in File-menu.
    */
    void UpdateMRUMenuItems();

    /**
    * @brief Add project file (path) to the MRU list.
    * @param project Full path to the project file to add.
    */
    void AddProjectMRU(const QString &project);

    /**
    * @brief Remove project file (path) from the MRU list.
    * @param project Full path of the project file to remove.
    */
    void RemoveProjectMRU(const QString &project);

    /**
    * @brief Program settings
    *
    */
    QSettings *mSettings;

    /**
    * @brief Thread to check files
    *
    */
    ThreadHandler *mThread;

    /**
    * @brief List of user defined applications to open errors with
    *
    */
    ApplicationList *mApplications;

    /**
    * @brief Class to handle translation changes
    *
    */
    TranslationHandler *mTranslation;

    /**
    * @brief Class holding all UI components
    *
    */
    Ui::MainWindow mUI;

    /**
    * @brief Current checked directory.
    */
    QString mCurrentDirectory;

    /**
    * @brief Log view.
    */
    LogView *mLogView;

    /**
    * @brief Project (file).
    */
    Project *mProject;

    /**
    * @brief Filter field in the Filter toolbar.
    */
    QLineEdit* mLineEditFilter;

    /**
    * @brief Timer to delay filtering while typing.
    */
    QTimer* mFilterTimer;

private:

    /**
    * @brief Are we exiting the cppcheck?
    * If this is true then the cppcheck is waiting for check threads to exit
    * so that the application can be closed.
    */
    bool mExiting;

    /**
     * @brief Project MRU menu actions.
     * List of MRU menu actions. Needs also to store the separator.
     */
    QAction *mRecentProjectActs[MaxRecentProjects + 1];
};
/// @}
#endif // MAINWINDOW_H

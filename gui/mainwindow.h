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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QSignalMapper>
#include <QActionGroup>
#include <QToolBar>

#include "resultsview.h"
#include "settingsdialog.h"
#include "translationhandler.h"
#include "settings.h"
#include "ui_main.h"

class ThreadHandler;
class LogView;
class HelpWindow;
class Project;
class ErrorItem;

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

    /**
    * @brief Slot for changing the program's language
    *
    */
    void MapLanguage(QAction *);

    /**
    * @brief Slot for showing/hiding standard toolbar
    */
    void ToggleMainToolBar();

    /**
    * @brief Slot for showing/hiding Categories toolbar
    */
    void ToggleViewToolBar();

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

protected:

    /**
    * @brief Create menu items to change language
    *
    */
    void CreateLanguageMenuItems();

    /**
    * @brief Set current language
    * @param index Index of the language to set
    */
    void SetLanguage(const int index);

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
    * @brief Check if we have a project for the checked directory.
    * This method checks if there is open project for the directory. If no open
    * project then we check if there is project file in the directory and load
    * it.
    * @return true if we have project, false if no project.
    */
    bool GetCheckProject();

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
    void OpenHtmlHelpContents();

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
    * @brief Group holding all supported languages
    *
    */
    QActionGroup *mLanguages;

    /**
    * @brief Current checked directory.
    */
    QString mCurrentDirectory;

    /**
    * @brief Log view.
    */
    LogView *mLogView;

    /**
     * @brief Help window..
     */
    HelpWindow *mHelpWindow;

    /**
    * @brief Project (file).
    */
    Project *mProject;

private:

    /**
    * @brief Are we exiting the cppcheck?
    * If this is true then the cppcheck is waiting for check threads to exit
    * so that the application can be closed.
    */
    bool mExiting;

};
/// @}
#endif // MAINWINDOW_H

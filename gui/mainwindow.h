/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
#include "resultsview.h"
#include <QSettings>
#include <QAction>
#include <QFileDialog>


#include "threadhandler.h"
#include "settingsdialog.h"

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
    void ReCheck();
    void ClearResults();

    void ShowAll(bool checked);
    void ShowSecurity(bool checked);
    void ShowStyle(bool checked);
    void ShowUnused(bool checked);
    void ShowErrors(bool checked);
    void CheckAll();
    void UncheckAll();

    /**
    * @brief Slot for check directory menu item
    *
    */
    void CheckDirectory();

    void ProgramSettings();

protected slots:

    /**
    * @brief Slot for checkthread's done signal
    *
    */
    void CheckDone();
protected:
    void ToggleAllChecked(bool checked);
    void EnableCheckButtons(bool enable);
    void DoCheckFiles(QFileDialog::FileMode mode);
    QStringList GetFilesRecursively(const QString &path);
    Settings GetCppCheckSettings();
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

    QAction mActionShowAll;
    QAction mActionShowSecurity;
    QAction mActionShowStyle;
    QAction mActionShowUnused;
    QAction mActionShowErrors;
    QAction mActionShowCheckAll;
    QAction mActionShowUncheckAll;



    /**
    * @brief Results for checking
    *
    */
    ResultsView mResults;

    /**
    * @brief Thread to check files
    *
    */
    ThreadHandler mThread;

    ApplicationList mApplications;

private:
};

#endif // MAINWINDOW_H

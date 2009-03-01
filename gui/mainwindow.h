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
#include "../src/settings.h"

#include "checkdialog.h"
#include "threadhandler.h"

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
    * @brief Slot for check menu item
    *
    */
    void Check();

protected slots:

    /**
    * @brief Slot for checkthread's done signal
    *
    */
    void CheckDone();
protected:

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
    QAction mExit;

    /**
    * @brief Menu action to open checkdialog
    *
    */
    QAction mCheck;

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

private:
};

#endif // MAINWINDOW_H

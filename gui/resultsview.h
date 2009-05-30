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


#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H


#include <QWidget>
#include <QProgressBar>
#include "../src/errorlogger.h"
#include "resultstree.h"
#include "common.h"


/**
* @brief Widget to show cppcheck progressbar and result
*
*/
class ResultsView : public QWidget
{
    Q_OBJECT
public:

    ResultsView(QSettings &settings, ApplicationList &list);
    virtual ~ResultsView();

    /**
    * @brief Function to show/hide certain type of errors
    * Refreshes the tree.
    *
    * @param type Type of error to show/hide
    * @param Should specified errors be shown (true) or hidden (false)
    */
    void ShowResults(ShowTypes type, bool show);

    /**
    * @brief Clear results
    *
    */
    void Clear();
public slots:

    /**
    * @brief Slot for updating the checking progress
    *
    * @param value Current progress value
    * @param max Maximum progress value
    */
    void Progress(int value, int max);

    /**
    * @brief Slot for new error to be displayed
    *
    * @param file filename
    * @param severity error severity
    * @param message error message
    * @param files list of files affected by the error
    * @param lines list of file line numers affected by the error
    */
    void Error(const QString &file,
               const QString &severity,
               const QString &message,
               const QStringList &files,
               const QVariantList &lines);
protected:
    /**
    * @brief Tree to show cppcheck's results
    *
    */
    ResultsTree *mTree;

    /**
    * @brief Progressbar to show cppcheck's progress
    *
    */
    QProgressBar *mProgress;

private:
};

#endif // RESULTSVIEW_H

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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H


#include <QWidget>
#include <QProgressBar>
#include "../lib/errorlogger.h"
#include "resultstree.h"
#include "common.h"
#include "report.h"
#include "ui_resultsview.h"

/// @addtogroup GUI
/// @{

/**
* @brief Widget to show cppcheck progressbar and result
*
*/
class ResultsView : public QWidget
{
    Q_OBJECT
public:

    ResultsView(QWidget * parent = 0);
    void Initialize(QSettings *settings, ApplicationList *list);
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

    /**
    * @brief Save results to a file
    *
    * @param filename Filename to save results to
    * @param type Type of the report.
    */
    void Save(const QString &filename, Report::Type type);

    /**
    * @brief Update tree settings
    *
    * @param showFullPath Show full path of files in the tree
    * @param saveFullPath Save full path of files in reports
    * @param saveAllErrors Save all visible errors
    */
    void UpdateSettings(bool showFullPath,
                        bool saveFullPath,
                        bool saveAllErrors,
                        bool showNoErrorsMessage);

    /**
    * @brief Set the directory we are checking
    *
    * This is used to split error file path to relative if necessary
    * @param dir Directory we are checking
    */
    void SetCheckDirectory(const QString &dir);

    /**
    * @brief Inform the view that checking has started
    *
    * At the moment this only displays the progressbar
    */
    void CheckingStarted();

    /**
    * @brief Do we have visible results to show?
    *
    * @return true if there is at least one warning/error to show.
    */
    bool HasVisibleResults() const;

    /**
    * @brief Do we have results from check?
    *
    * @return true if there is at least one warning/error, hidden or visible.
    */
    bool HasResults() const;

    /**
    * @brief Save View's settings
    *
    */
    void SaveSettings();

    /**
    * @brief Translate this view
    *
    */
    void Translate();

    void DisableProgressbar();
signals:

    /**
    * @brief Signal to be emitted when we have results
    *
    */
    void GotResults();


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
               const QVariantList &lines,
               const QString &error);

    /**
    * @brief Collapse all results in the result list.
    */
    void CollapseAllResults();

    /**
    * @brief Expand all results in the result list.
    */
    void ExpandAllResults();

protected:
    /**
    * @brief Have any errors been found
    */
    bool mErrorsFound;

    /**
    * @brief Should we show a "No errors found dialog" everytime no errors were found?
    */
    bool mShowNoErrorsMessage;

    Ui::ResultsView mUI;

private:
};
/// @}
#endif // RESULTSVIEW_H

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "report.h"
#include "showtypes.h"
#include "ui_resultsview.h"

class ErrorItem;
class ApplicationList;
class QModelIndex;
class QPrinter;
class QSettings;
class CheckStatistics;

/// @addtogroup GUI
/// @{

/**
* @brief Widget to show cppcheck progressbar and result
*
*/
class ResultsView : public QWidget {
    Q_OBJECT
public:

    explicit ResultsView(QWidget * parent = 0);
    void Initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler);
    virtual ~ResultsView();

    /**
    * @brief Function to show/hide certain type of errors
    * Refreshes the tree.
    *
    * @param type Type of error to show/hide
    * @param show Should specified errors be shown (true) or hidden (false)
    */
    void ShowResults(ShowTypes::ShowType type, bool show);

    /**
     * @brief Clear results and statistics and reset progressinfo.
     * @param results Remove all the results from view?
     */
    void Clear(bool results);

    /**
     * @brief Remove a file from the results.
     */
    void Clear(const QString &filename);

    /**
     * @brief Remove a recheck file from the results.
     */
    void ClearRecheckFile(const QString &filename);

    /**
    * @brief Save results to a file
    *
    * @param filename Filename to save results to
    * @param type Type of the report.
    */
    void Save(const QString &filename, Report::Type type) const;

    /**
    * @brief Update tree settings
    *
    * @param showFullPath Show full path of files in the tree
    * @param saveFullPath Save full path of files in reports
    * @param saveAllErrors Save all visible errors
    * @param showNoErrorsMessage Show "no errors"?
    * @param showErrorId Show error id?
    * @param showInconclusive Show inconclusive?
    */
    void UpdateSettings(bool showFullPath,
                        bool saveFullPath,
                        bool saveAllErrors,
                        bool showNoErrorsMessage,
                        bool showErrorId,
                        bool showInconclusive);

    /**
    * @brief Set the directory we are checking
    *
    * This is used to split error file path to relative if necessary
    * @param dir Directory we are checking
    */
    void SetCheckDirectory(const QString &dir);

    /**
    * @brief Get the directory we are checking
    *
    * @return Directory containing source files
    */

    QString GetCheckDirectory(void);

    /**
    * @brief Inform the view that checking has started
    *
    * @param count Count of files to be checked.
    */
    void CheckingStarted(int count);

    /**
    * @brief Inform the view that checking finished.
    *
    */
    void CheckingFinished();

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
    * @param settings program settings.
    */
    void SaveSettings(QSettings *settings);

    /**
    * @brief Translate this view
    *
    */
    void Translate();

    void DisableProgressbar();

    /**
    * @brief Read errors from report XML file.
    * @param filename Report file to read.
    *
    */
    void ReadErrorsXml(const QString &filename);

    /**
     * @brief Return checking statistics.
     * @return Pointer to checking statistics.
     */
    CheckStatistics *GetStatistics() const {
        return mStatistics;
    }

    /**
     * @brief Return Showtypes.
     * @return Pointer to Showtypes.
     */
    ShowTypes * GetShowTypes() const {
        return &mUI.mTree->mShowSeverities;
    }

signals:

    /**
    * @brief Signal to be emitted when we have results
    *
    */
    void GotResults();

    /**
    * @brief Signal that results have been hidden or shown
    *
    * @param hidden true if there are some hidden results, or false if there are not
    */
    void ResultsHidden(bool hidden);

    /**
    * @brief Signal to perform recheck of selected files
    *
    * @param selectedFilesList list of selected files
    */
    void CheckSelected(QStringList selectedFilesList);

public slots:

    /**
    * @brief Slot for updating the checking progress
    *
    * @param value Current progress value
    * @param description Description to accompany the progress
    */
    void Progress(int value, const QString& description);

    /**
    * @brief Slot for new error to be displayed
    *
    * @param item Error data
    */
    void Error(const ErrorItem &item);

    /**
    * @brief Collapse all results in the result list.
    */
    void CollapseAllResults();

    /**
    * @brief Expand all results in the result list.
    */
    void ExpandAllResults();

    /**
    * @brief Filters the results in the result list.
    */
    void FilterResults(const QString& filter);

    /**
    * @brief Show hidden results in the result list.
    */
    void ShowHiddenResults();

    /**
    * @brief Update detailed message when selected item is changed.
    *
    * @param index Position of new selected item.
    */
    void UpdateDetails(const QModelIndex &index);

    /**
    * @brief Slot opening a print dialog to print the current report
    */
    void Print();

    /**
    * @brief Slot printing the current report to the printer.
    * @param printer The printer used for printing the report.
    */
    void Print(QPrinter* printer);

    /**
    * @brief Slot opening a print preview dialog
    */
    void PrintPreview();

protected:
    /**
    * @brief Should we show a "No errors found dialog" every time no errors were found?
    */
    bool mShowNoErrorsMessage;

    Ui::ResultsView mUI;

    CheckStatistics *mStatistics;

private:
};
/// @}
#endif // RESULTSVIEW_H

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


#ifndef RESULTSTREE_H
#define RESULTSTREE_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSettings>
#include <QContextMenuEvent>
#include "errorlogger.h" // Severity
#include "showtypes.h"

class ApplicationList;
class Report;
class ErrorItem;
class ErrorLine;
class QModelIndex;
class QWidget;
class QItemSelectionModel;

/// @addtogroup GUI
/// @{


/**
* @brief Cppcheck's results are shown in this tree
*
*/
class ResultsTree : public QTreeView {
    Q_OBJECT
public:
    explicit ResultsTree(QWidget * parent = 0);
    virtual ~ResultsTree();
    void Initialize(QSettings *settings, ApplicationList *list);

    /**
    * @brief Add a new item to the tree
    *
    * @param item Error item data
    */
    bool AddErrorItem(const ErrorItem &item);

    /**
    * @brief Clear all errors from the tree
    *
    */
    void Clear();

    /**
     * @brief Clear errors for a specific file from the tree
     */
    void Clear(const QString &filename);

    /**
    * @brief Function to show/hide certain type of errors
    * Refreshes the tree.
    *
    * @param type Type of error to show/hide
    * @param show Should specified errors be shown (true) or hidden (false)
    */
    void ShowResults(ShowTypes::ShowType type, bool show);

    /**
    * @brief Function to filter the displayed list of errors.
    * Refreshes the tree.
    *
    * @param filter String that must be found in the summary, description, file or id
    */
    void FilterResults(const QString& filter);

    /**
    * @brief Function to show results that were previous hidden with HideResult()
    */
    void ShowHiddenResults();

    /**
    * @brief Save results to a text stream
    *
    */
    void SaveResults(Report *report) const;

    /**
    * @brief Update tree settings
    *
    * @param showFullPath Show full path of files in the tree
    * @param saveFullPath Save full path of files in reports
    * @param saveAllErrors Save all visible errors
    * @param showErrorId Show error id
    */
    void UpdateSettings(bool showFullPath, bool saveFullPath, bool saveAllErrors, bool showErrorId);

    /**
    * @brief Set the directory we are checking
    *
    * This is used to split error file path to relative if necessary
    * @param dir Directory we are checking
    */
    void SetCheckDirectory(const QString &dir);

    /**
    * @brief Check if there are any visible results in view.
    * @return true if there is at least one visible warning/error.
    */
    bool HasVisibleResults() const;

    /**
    * @brief Do we have results from check?
    * @return true if there is at least one warning/error, hidden or visible.
    */
    bool HasResults() const;

    /**
    * @brief Save all settings
    * Column widths
    */
    void SaveSettings() const;

    /**
    * @brief Change all visible texts language
    *
    */
    void Translate();

    /**
    * @brief Show optional column "Id"
    */
    void ShowIdColumn(bool show);

    /**
    * @brief Returns true if column "Id" is shown
    */
    bool ShowIdColumn() const {
        return mShowErrorId;
    }

    /**
     * @brief GUI severities.
     */
    ShowTypes mShowSeverities;


signals:
    /**
    * @brief Signal that results have been hidden or shown
    *
    * @param hidden true if there are some hidden results, or false if there are not
    */
    void ResultsHidden(bool hidden);

    /**
    * @brief Signal for selection change in result tree.
    *
    * @param current Model index to specify new selected item.
    */
    void SelectionChanged(const QModelIndex &current);

protected slots:
    /**
    * @brief Slot to quickstart an error with default application
    *
    * @param index Model index to specify which error item to open
    */
    void QuickStartApplication(const QModelIndex &index);

    /**
    * @brief Slot for context menu item to open an error with specified application
    *
    * @param application Index of the application to open the error
    */
    void Context(int application);

    /**
    * @brief Slot for context menu item to copy filename to clipboard
    *
    */
    void CopyFilename();

    /**
    * @brief Slot for context menu item to copy full path to clipboard
    *
    */
    void CopyFullPath();

    /**
    * @brief Slot for context menu item to the current error message to clipboard
    *
    */
    void CopyMessage();

    /**
    * @brief Slot for context menu item to the current error message Id to clipboard
    *
    */
    void CopyMessageId();

    /**
    * @brief Slot for context menu item to hide the current error message
    *
    */
    void HideResult();

    /**
    * @brief Slot for context menu item to hide all messages with the current message Id
    *
    */
    void HideAllIdResult();

    /**
    * @brief Slot for selection change in the results tree.
    *
    * @param current Model index to specify new selected item.
    * @param previous Model index to specify previous selected item.
    */
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);

protected:

    /**
    * @brief Hides/shows full file path on all error file items according to mShowFullPath
    *
    */
    void RefreshFilePaths();

    /**
    * @brief Hides/shows full file path on all error file items according to mShowFullPath
    * @param item Parent item whose childrens paths to change
    */
    void RefreshFilePaths(QStandardItem *item);


    /**
    * @brief Removes checking directory from given path if mShowFullPath is false
    *
    * @param path Path to remove checking directory
    * @param saving are we saving? Check mSaveFullPath instead
    * @return Path that has checking directory removed
    */
    QString StripPath(const QString &path, bool saving) const;


    /**
    * @brief Save all errors under specified item
    * @param report Report that errors are saved to
    * @param item Item whose errors to save
    */
    void SaveErrors(Report *report, QStandardItem *item) const;

    /**
    * @brief Convert a severity string to a icon filename
    *
    * @param severity Severity
    */
    QString SeverityToIcon(Severity::SeverityType severity) const;

    /**
    * @brief Helper function to open an error within target with application*
    *
    * @param target Error tree item to open
    * @param application Index of the application to open with. Giving -1
    *  (default value) will open the default application.
    */
    void StartApplication(QStandardItem *target, int application = -1);

    /**
    * @brief Helper function to copy filename/full path to the clipboard
    *
    * @param target Error tree item to open
    * @param fullPath Are we copying full path or only filename?
    */
    void CopyPath(QStandardItem *target, bool fullPath);

    /**
    * @brief Context menu event (user right clicked on the tree)
    *
    * @param e Event
    */
    void contextMenuEvent(QContextMenuEvent * e);

    /**
    * @brief Add a new error item beneath a file or a backtrace item beneath an error
    *
    * @param parent Parent for the item. Either a file item or an error item
    * @param item Error line data
    * @param hide Should this be hidden (true) or shown (false)
    * @param icon Should a default backtrace item icon be added
    * @return newly created QStandardItem *
    */
    QStandardItem *AddBacktraceFiles(QStandardItem *parent,
                                     const ErrorLine &item,
                                     const bool hide,
                                     const QString &icon);


    /**
    * @brief Refresh tree by checking which of the items should be shown
    * and which should be hidden
    *
    */
    void RefreshTree();

    /**
    * @brief Convert Severity to translated string for GUI.
    * @param severity Severity to convert
    * @return Severity as translated string
    */
    static QString SeverityToTranslatedString(Severity::SeverityType severity);

    /**
    * @brief Load all settings
    * Column widths
    */
    void LoadSettings();

    /**
    * @brief Ask directory where file is located.
    * @param file File name.
    * @return Directory user chose.
    */
    QString AskFileDir(const QString &file);

    /**
    * @brief Create new normal item.
    *
    * Normal item has left alignment and text set also as tooltip.
    * @param name name for the item
    * @return new QStandardItem
    */
    static QStandardItem *CreateNormalItem(const QString &name);

    /**
    * @brief Create new line number item.
    *
    * Line number item has right align and text set as tooltip.
    * @param linenumber name for the item
    * @return new QStandardItem
    */
    static QStandardItem *CreateLineNumberItem(const QString &linenumber);

    /**
    * @brief Finds a file item
    *
    * @param name name of the file item to find
    * @return pointer to file item or null if none found
    */
    QStandardItem *FindFileItem(const QString &name) const;


    /**
    * @brief Ensures there's a item in the model for the specified file
    *
    * @param fullpath Full path to the file item.
    * @param file0 Source file
    * @param hide is the error (we want this file item for) hidden?
    * @return QStandardItem to be used as a parent for all errors for specified file
    */
    QStandardItem *EnsureFileItem(const QString &fullpath, const QString &file0, bool hide);

    /**
    * @brief Show a file item
    *
    * @param name Filename of the fileitem
    */
    void ShowFileItem(const QString &name);

    /**
    * @brief Item model for tree
    *
    */
    QStandardItemModel mModel;

    /**
    * @brief Program settings
    *
    */
    QSettings *mSettings;

    /**
    * @brief A string used to filter the results for display.
    *
    */
    QString mFilter;

    /**
    * @brief List of applications to open errors with
    *
    */
    ApplicationList *mApplications;

    /**
    * @brief Right clicked item (used by context menu slots)
    *
    */
    QStandardItem *mContextItem;

    /**
    * @brief Should full path of files be shown (true) or relative (false)
    *
    */
    bool mShowFullPath;

    /**
    * @brief Should full path of files be saved
    *
    */
    bool mSaveFullPath;

    /**
    * @brief Save all errors (true) or only visible (false)
    *
    */
    bool mSaveAllErrors;

    /**
    * @brief true if optional column "Id" is shown
    *
    */
    bool mShowErrorId;

    /**
    * @brief Path we are currently checking
    *
    */
    QString mCheckPath;

    /**
    * @brief Are there any visible errors
    *
    */
    bool mVisibleErrors;

private:
    QItemSelectionModel *mSelectionModel;
};
/// @}
#endif // RESULTSTREE_H

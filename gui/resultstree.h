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


#ifndef RESULTSTREE_H
#define RESULTSTREE_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSettings>
#include <QContextMenuEvent>
#include "common.h"
#include "applicationlist.h"


/**
* @brief Cppcheck's results are shown in this tree
*
*/
class ResultsTree : public QTreeView
{
    Q_OBJECT
public:
    ResultsTree(QSettings &settings, ApplicationList &list);
    virtual ~ResultsTree();

    /**
    * @brief Add a new item to the tree
    *
    * @param file filename
    * @param severity error severity
    * @param message error message
    * @param files list of files affected by the error
    * @param lines list of file line numers affected by the error
    */
    void AddErrorItem(const QString &file,
                      const QString &severity,
                      const QString &message,
                      const QStringList &files,
                      const QVariantList &lines);

    /**
    * @brief Clear all errors from the tree
    *
    */
    void Clear();

    /**
    * @brief Function to show/hide certain type of errors
    * Refreshes the tree.
    *
    * @param type Type of error to show/hide
    * @param Should specified errors be shown (true) or hidden (false)
    */
    void ShowResults(ShowTypes type, bool show);
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
protected:
    /**
    * @brief Convert a severity string to a icon filename
    *
    * @param severity Severity string
    * @param Icon filename
    */
    QString SeverityToIcon(const QString &severity);

    /**
    * @brief Helper function to open an error within target with application
    *
    *
    * @param target Error tree item to open
    * @param application Index of the application to open with
    */
    void StartApplication(QStandardItem *target, int application);

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
    * @param file Filename of the error
    * @param line Line numer
    * @param severity Error severity
    * @param message Error message
    * @param hide Should this be hidden (true) or shown (false)
    * @param addicon Should a default backtrace item icon be added
    * @return newly created QStandardItem *
    */
    QStandardItem *AddBacktraceFiles(QStandardItem *parent,
                                     const QString &file,
                                     const int line,
                                     const QString &severity,
                                     const QString &message,
                                     const bool hide,
                                     const QString &icon);


    /**
    * @brief Refresh tree by checking which of the items should be shown
    * and which should be hidden
    *
    */
    void RefreshTree();

    /**
    * @brief Convert QVariant (that contains an int) to Showtypes value
    *
    * @param data QVariant (that contains an int) to be converted
    * @return data converted to ShowTypes
    */
    ShowTypes VariantToShowType(const QVariant &data);

    /**
    * @brief Convert severity string to ShowTypes value
    * @param severity Error severity string
    * @return Severity converted to ShowTypes value
    */
    ShowTypes SeverityToShowType(const QString &severity);

    /**
    * @brief Load all settings
    * Colum widths
    */
    void LoadSettings();

    /**
    * @brief Save all settings
    * Colum widths
    */
    void SaveSettings();

    /**
    * @brief Create a new QStandardItem
    *
    * @param name name for the item
    * @return new QStandardItem
    */
    QStandardItem *CreateItem(const QString &name);

    /**
    * @brief Finds a file item
    *
    * @param name name of the file item to find
    * @return pointer to file item or null if none found
    */
    QStandardItem *FindFileItem(const QString &name);


    /**
    * @brief Ensures there's a item in the model for the specified file
    *
    * @param name Filename
    * @return QStandardItem to be used as a parent for all errors for specified file
    */
    QStandardItem *EnsureFileItem(const QString &name);

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
    QSettings &mSettings;

    /**
    * @brief List of bools to determine which of ShowTypes to display on the tree
    * (true) and which of them should be hidden (false)
    *
    */
    bool mShowTypes[SHOW_NONE];

    /**
    * @brief List of applications to open errors with
    *
    */
    ApplicationList &mApplications;

    /**
    * @brief Right clicked item (used by context menu slots)
    *
    */
    QStandardItem *mContextItem;
private:
};

#endif // RESULTSTREE_H

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
#include "common.h"

/**
* @brief Cppcheck's results are shown in this tree
*
*/
class ResultsTree : public QTreeView
{
public:
    ResultsTree(QSettings &settings);
    virtual ~ResultsTree();

    /**
    * @brief Add a new item to the tree
    *
    * @param file filename
    * @param severity error severity
    * @param error error message
    */
    void AddErrorItem(const QString &file,
                      const QString &severity,
                      const QString &message,
                      const QStringList &files,
                      const QList<int> &lines);

    /**
    * @brief Clear all errors from the tree
    *
    */
    void Clear();

    void ShowResults(ShowTypes type, bool show);
protected:
    void AddItem(int index);
    void RefreshTree();
    ShowTypes SeverityToShowType(const QString &severity);
    QString ShowTypeToString(ShowTypes type);

    typedef struct
    {
        QString file;
        ShowTypes type;
        QString message;
        QStringList files;
        QList<int> lines;
    }ErrorItem;

    QList<ErrorItem> mItems;
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
    * @brief Item model for tree
    *
    */
    QStandardItemModel mModel;

    /**
    * @brief Program settings
    *
    */
    QSettings &mSettings;

    bool mShowTypes[SHOW_NONE];
private:
};

#endif // RESULTSTREE_H

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


#include "resultstree.h"
#include <QDebug>

ResultsTree::ResultsTree(QSettings &settings) :
        mSettings(settings)
{
    setModel(&mModel);
    QStringList labels;
    labels << tr("severity") << tr("Line") << tr("Message");
    mModel.setHorizontalHeaderLabels(labels);

    LoadSettings();
}

ResultsTree::~ResultsTree()
{
    SaveSettings();
}



QStandardItem *ResultsTree::CreateItem(const QString &name)
{
    QStandardItem *item = new QStandardItem(name);
    item->setEditable(false);
    return item;
}


void ResultsTree::AddErrorItem(const QString &file,
                               const QString &severity,
                               const QString &message,
                               const QStringList &files,
                               const QList<int> &lines)
{
    Q_UNUSED(file);

    if (files.isEmpty())
        return;

    QString realfile = files[0];

    if (realfile.isEmpty())
        realfile = "Undefined file";

    ErrorItem item;
    item.file = realfile;
    item.type = SeverityToShowType(severity);
    item.message = message;
    item.files = files;
    item.lines = lines;
    mItems << item;

    if (mShowTypes[item.type])
    {
        AddItem(mItems.size() - 1);
    }
}

ShowTypes ResultsTree::SeverityToShowType(const QString & severity)
{
    if (severity == "all")
        return SHOW_ALL;
    if (severity == "error")
        return SHOW_ERRORS;
    if (severity == "style")
        return SHOW_STYLE;
    if (severity == "security")
        return SHOW_SECURITY;

    return SHOW_NONE;
}

QStandardItem *ResultsTree::FindFileItem(const QString &name)
{
    QList<QStandardItem *> list = mModel.findItems(name);
    if (list.size() > 0)
        return list[0];
    return 0;
}

void ResultsTree::Clear()
{
    mModel.removeRows(0, mModel.rowCount());
    mItems.clear();
}

void ResultsTree::LoadSettings()
{
    for (int i = 0;i < mModel.columnCount();i++)
    {
        //mFileTree.columnWidth(i);
        QString temp = QString(tr("Result column %1 width")).arg(i);
        setColumnWidth(i, mSettings.value(temp, 800 / mModel.columnCount()).toInt());
    }
}

void ResultsTree::SaveSettings()
{
    for (int i = 0;i < mModel.columnCount();i++)
    {
        QString temp = QString(tr("Result column %1 width")).arg(i);
        mSettings.setValue(temp, columnWidth(i));
    }
}

void ResultsTree::ShowResults(ShowTypes type, bool show)
{
    if (type != SHOW_NONE)
    {
        if (mShowTypes[type] != show)
        {
            mShowTypes[type] = show;
            RefreshTree();
        }
    }
}


void ResultsTree::RefreshTree()
{
    mModel.removeRows(0, mModel.rowCount());
    for (int i = 0;i < mItems.size();i++)
    {
        if (mShowTypes[mItems[i].type])
        {
            AddItem(i);
        }
    }
}

QString ResultsTree::ShowTypeToString(ShowTypes type)
{
    switch (type)
    {
    case SHOW_ALL:
        return "all";
    case SHOW_ERRORS:
        return "error";
    case SHOW_STYLE:
        return "style";
    case SHOW_SECURITY:
        return "security";
    case SHOW_UNUSED:
        return "unused";
    case SHOW_NONE:
        return "none";
    }

    return "";
}


void ResultsTree::AddItem(int index)
{
    if (index >= 0 && index < mItems.size())
    {
        QStandardItem *fileitem = FindFileItem(mItems[index].file);
        if (!fileitem)
        {
            //qDebug()<<"No previous error for file"<<realfile;
            fileitem = CreateItem(mItems[index].file);
            mModel.appendRow(fileitem);
        }

        QList<QStandardItem*> list;
        list << CreateItem(ShowTypeToString(mItems[index].type));
        list << CreateItem(QString("%1").arg(mItems[index].lines[0]));
        list << CreateItem(mItems[index].message);
        fileitem->appendRow(list);
    }
}


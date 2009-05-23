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
    labels <<tr("File")<< tr("Severity") << tr("Line") << tr("Message");
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
                               const QVariantList &lines)
{
    Q_UNUSED(file);


    if (files.isEmpty())
    {
        return;
    }

    QString realfile = files[0];

    if (realfile.isEmpty())
    {
        realfile = "Undefined file";
    }


    //Create the base item for the error and ensure it has a proper
    //file item as a parent
    QStandardItem *item = AddBacktraceFiles(EnsureFileItem(realfile),
                                            realfile,
                                            lines[0].toInt(),
                                            severity,
                                            message);


    //Add user data to that item
    QMap<QString, QVariant> data;
    data["severity"]  = SeverityToShowType(severity);
    data["message"]  = message;
    data["files"]  = files;
    data["lines"]  = lines;
    item->setData(QVariant(data));

    //Add backtrace files as children
    for (int i=1;i<files.size() && i < lines.size();i++)
    {
        AddBacktraceFiles(item,files[i],lines[i].toInt(),severity,message);
    }

    //TODO just hide/show current error and it's file
    //since this does a lot of unnecessary work
    RefreshTree();
}

QStandardItem *ResultsTree::AddBacktraceFiles(QStandardItem *parent,
        const QString &file,
        const int line,
        const QString &severity,
        const QString &message)

{

    QList<QStandardItem*> list;
    list << CreateItem(file);
    list << CreateItem(severity);
    list << CreateItem(QString("%1").arg(line));
    list << CreateItem(message);



    if (parent)
    {
        parent->appendRow(list);

    }
    else
    {
        mModel.appendRow(list);
    }

    //TODO Does this leak memory? Should items from list be deleted?

    return list[0];
}

ShowTypes ResultsTree::VariantToShowType(const QVariant &data)
{
    int value = data.toInt();
    if (value < SHOW_ALL && value > SHOW_ERRORS)
    {
        return SHOW_NONE;
    }
    return (ShowTypes)value;
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
    {
        return list[0];
    }
    return 0;
}

void ResultsTree::Clear()
{
    mModel.removeRows(0, mModel.rowCount());
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
    if (type != SHOW_NONE && mShowTypes[type] != show)
    {
        mShowTypes[type] = show;
        RefreshTree();

    }
}


void ResultsTree::RefreshTree()
{
    //Get the amount of files in the tree
    int filecount = mModel.rowCount();

    for (int i=0;i<filecount;i++)
    {
        //Get file i
        QStandardItem *file = mModel.item(i,0);
        if (!file)
        {
            continue;
        }

        //Get the amount of errors this file contains
        int errorcount = file->rowCount();

        //By default it shouldn't be visible
        bool show = false;

        for (int j=0;j<errorcount;j++)
        {
            //Get the error itself
            QStandardItem *child = file->child(j,0);
            if (!child)
            {
                continue;
            }

            //Get error's user data
            QVariant userdata = child->data();
            //Convert it to QVariantMap
            QVariantMap data = userdata.toMap();

            //Check if this error should be hidden
            bool hide = !mShowTypes[VariantToShowType(data["severity"])];

            //Hide/show accordingly
            setRowHidden(j,file->index(),hide);

            //If it was shown then the file itself has to be shown aswell
            if (!hide)
            {
                show = true;
            }
        }

        //Show the file if any of it's errors are visible
        setRowHidden(i,QModelIndex(),!show);
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

QStandardItem *ResultsTree::EnsureFileItem(const QString &name)
{
    QStandardItem *item = FindFileItem(name);

    if (item)
    {
        return item;
    }

    item = CreateItem(name);

    mModel.appendRow(item);

    return item;
}

void ResultsTree::ShowFileItem(const QString &name)
{
    QStandardItem *item = FindFileItem(name);
    if (item)
    {
        setRowHidden(0,mModel.indexFromItem(item),false);
    }
}

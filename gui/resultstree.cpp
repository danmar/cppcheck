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

    QStandardItem *fileitem = FindFileItem(realfile);

    if (!fileitem)
    {
        //qDebug()<<"No previous error for file"<<realfile;
        fileitem = CreateItem(realfile);
        mModel.appendRow(fileitem);
    }

    //qDebug() << "Adding error for file" << realfile << ". Message is" << message;

    QList<QStandardItem*> list;
    list << CreateItem(severity);
    list << CreateItem(QString("%1").arg(lines[0]));
    list << CreateItem(message);
    fileitem->appendRow(list);

    //qDebug()<<"\n";
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

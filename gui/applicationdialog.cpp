/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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

#include <QString>
#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include "applicationdialog.h"


ApplicationDialog::ApplicationDialog(const QString &name,
                                     const QString &path,
                                     const QString &title,
                                     QWidget *parent) :
    QDialog(parent)
{
    mUI.setupUi(this);

    connect(mUI.mButtonBrowse, SIGNAL(clicked()), this, SLOT(Browse()));
    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mUI.mButtons, SIGNAL(rejected()), this, SLOT(reject()));
    mUI.mPath->setText(path);
    mUI.mName->setText(name);
    setWindowTitle(title);
}


ApplicationDialog::~ApplicationDialog()
{
    //dtor
}

void ApplicationDialog::Browse()
{
    QString filter;
#ifdef Q_WS_WIN
    // In Windows (almost) all executables have .exe extension
    // so it does not make sense to show everything.
    filter += tr("Executable files (*.exe);;All files(*.*)");
#endif // Q_WS_WIN
    QString selectedFile = QFileDialog::getOpenFileName(this,
                           tr("Select viewer application"),
                           QString(),
                           filter);

    if (!selectedFile.isEmpty())
    {
        QString path(QDir::toNativeSeparators(selectedFile));

        // In Windows we must surround paths including spaces with quotation marks.
#ifdef Q_WS_WIN
        if (path.indexOf(" ") > -1)
        {
            path.insert(0, "\"");
            path.append("\"");
        }
#endif // Q_WS_WIN

        mUI.mPath->setText(path);
    }
}

QString ApplicationDialog::GetName()
{
    return mUI.mName->text();
}


QString ApplicationDialog::GetPath()
{
    return mUI.mPath->text();
}

void ApplicationDialog::Ok()
{
    if (mUI.mName->text().isEmpty() || mUI.mPath->text().isEmpty())
    {
        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        tr("You must specify a name and a path for the application!"),
                        QMessageBox::Ok,
                        this);

        msg.exec();

    }
    else
    {
        // Convert possible native (Windows) path to internal presentation format
        mUI.mPath->setText(QDir::fromNativeSeparators(mUI.mPath->text()));
        accept();
    }
}


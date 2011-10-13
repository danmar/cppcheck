/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include "application.h"


ApplicationDialog::ApplicationDialog(const QString &title,
                                     const Application &app,
                                     QWidget *parent) :
    QDialog(parent)
{
    mUI.setupUi(this);

    connect(mUI.mButtonBrowse, SIGNAL(clicked()), this, SLOT(Browse()));
    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mUI.mButtons, SIGNAL(rejected()), this, SLOT(reject()));
    mUI.mPath->setText(app.getPath());
    mUI.mName->setText(app.getName());
    mUI.mParameters->setText(app.getParameters());
    setWindowTitle(title);
    adjustSize();
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

    if (!selectedFile.isEmpty()) {
        QString path(QDir::toNativeSeparators(selectedFile));
        mUI.mPath->setText(path);
    }
}

Application ApplicationDialog::GetApplication() const
{
    Application app;
    app.setName(mUI.mName->text());
    app.setPath(mUI.mPath->text());
    app.setParameters(mUI.mParameters->text());
    return app;
}

void ApplicationDialog::Ok()
{
    if (mUI.mName->text().isEmpty() || mUI.mPath->text().isEmpty() ||
        mUI.mParameters->text().isEmpty()) {
        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        tr("You must specify a name, a path and parameters for the application!"),
                        QMessageBox::Ok,
                        this);

        msg.exec();

    } else {
        // Convert possible native (Windows) path to internal presentation format
        mUI.mPath->setText(QDir::fromNativeSeparators(mUI.mPath->text()));
        accept();
    }
}


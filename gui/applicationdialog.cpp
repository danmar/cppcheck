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

#include <QString>
#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include "applicationdialog.h"
#include "application.h"
#include "common.h"


ApplicationDialog::ApplicationDialog(const QString &title,
                                     Application &app,
                                     QWidget *parent) :
    QDialog(parent),
    mApplication(app)
{
    mUI.setupUi(this);

    connect(mUI.mButtonBrowse, SIGNAL(clicked()), this, SLOT(Browse()));
    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(Ok()));
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
#ifdef Q_OS_WIN
    // In Windows (almost) all executables have .exe extension
    // so it does not make sense to show everything.
    filter += tr("Executable files (*.exe);;All files(*.*)");
#endif // Q_OS_WIN
    QString selectedFile = QFileDialog::getOpenFileName(this,
                           tr("Select viewer application"),
                           GetPath(SETTINGS_LAST_APP_PATH),
                           filter);

    if (!selectedFile.isEmpty()) {
        SetPath(SETTINGS_LAST_APP_PATH, selectedFile);
        QString path(QDir::toNativeSeparators(selectedFile));
        mUI.mPath->setText(path);
    }
}

void ApplicationDialog::Ok()
{
    if (mUI.mName->text().isEmpty() || mUI.mPath->text().isEmpty()) {
        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        tr("You must specify a name, a path and optionally parameters for the application!"),
                        QMessageBox::Ok,
                        this);

        msg.exec();

        reject();
    } else {
        // Convert possible native (Windows) path to internal presentation format
        mApplication.setName(mUI.mName->text());
        mApplication.setPath(QDir::fromNativeSeparators(mUI.mPath->text()));
        mApplication.setParameters(mUI.mParameters->text());

        accept();
    }
}

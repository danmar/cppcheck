/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

#include "applicationdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>


ApplicationDialog::ApplicationDialog(const QString &name,
                                     const QString &path,
                                     const QString &title)
{
    QVBoxLayout *layout = new QVBoxLayout();
    mName = new QLineEdit(name);
    mPath = new QLineEdit(path);

    QString guide = tr("Here you can add applications that can open error files.\n" \
                       "Specify a name for the application and the application to execute.\n\n" \
                       "The following texts are replaced with appriproate values when application is executed:\n" \
                       "(file) - Filename containing the error\n" \
                       "(line) - Line number containing the error\n" \
                       "(message) - Error message\n" \
                       "(severity) - Error severity\n" \
                       "\n" \
                       "Example opening a file with Kate and make Kate scroll to the correct line:\n" \
                       "kate -l(line) (file)");

    layout->addWidget(new QLabel(guide));

    layout->addWidget(new QLabel(tr("Application's name")));
    layout->addWidget(mName);
    layout->addWidget(new QLabel(tr("Application to execute")));
    layout->addWidget(mPath);
    QPushButton *browse = new QPushButton(tr("Browse"));
    connect(browse, SIGNAL(clicked()), this, SLOT(Browse()));
    layout->addWidget(browse);

    QPushButton *cancel = new QPushButton(tr("Cancel"));
    QPushButton *ok = new QPushButton(tr("Ok"));

    //Add a layout for ok/cancel buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);
    layout->addLayout(buttonLayout);

    //Connect OK buttons
    connect(ok, SIGNAL(clicked()),
            this, SLOT(Ok()));
    connect(cancel, SIGNAL(clicked()),
            this, SLOT(reject()));
    setLayout(layout);
    setWindowTitle(title);
}


ApplicationDialog::~ApplicationDialog()
{
    //dtor
}


void ApplicationDialog::Browse()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec())
    {
        QStringList list = dialog.selectedFiles();
        if (list.size() > 0)
        {
            mPath->setText(list[0]);
        }
    }
}

QString ApplicationDialog::GetName()
{
    return mName->text();
}


QString ApplicationDialog::GetPath()
{
    return mPath->text();
}

void ApplicationDialog::Ok()
{
    if (mName->text().isEmpty() || mPath->text().isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("You must specify a name and a path for the application!");
        msgBox.exec();
    }
    else
    {
        // Convert possible native (Windows) path to internal presentation format
        mPath->setText(QDir::fromNativeSeparators(mPath->text()));
        accept();
    }
}


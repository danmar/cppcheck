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


#include "mainwindow.h"
#include <QDebug>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow() :
        mSettings("CppCheck", "CppCheck-GUI"),
        mExit("E&xit", this),
        mCheck("&Check", this),
        mResults(mSettings)
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(&mCheck);
    menu->addSeparator();
    menu->addAction(&mExit);

    setCentralWidget(&mResults);


    connect(&mExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(&mCheck, SIGNAL(triggered()), this, SLOT(Check()));
    connect(&mThread, SIGNAL(Done()), this, SLOT(CheckDone()));
    connect(&mThread, SIGNAL(CurrentFile(const QString &)),
            &mResults, SLOT(CurrentFile(const QString &)));

    connect(&mThread, SIGNAL(Progress(int, int)),
            &mResults, SLOT(Progress(int, int)));
    connect(&mThread, SIGNAL(Error(const QString &, const QString &, const QString &)),
            &mResults, SLOT(Error(const QString &, const QString &, const QString &)));
    LoadSettings();
}

MainWindow::~MainWindow()
{
    //dtor
}

void MainWindow::LoadSettings()
{
    if (mSettings.value("Window maximized", false).toBool())
    {
        showMaximized();
    }
    else
    {
        resize(mSettings.value("Window width", 800).toInt(), mSettings.value("Window height", 600).toInt());
    }
}

void MainWindow::SaveSettings()
{
    mSettings.setValue("Window width", size().width());
    mSettings.setValue("Window height", size().height());
    mSettings.setValue("Window maximized", isMaximized());
}


void MainWindow::Check()
{
    CheckDialog dialog(mSettings);
    if (dialog.exec() == QDialog::Accepted)
    {
        mResults.Clear();
        mThread.ClearFiles();

        QString str;
        qDebug("Selected files:");
        foreach(str, dialog.GetSelectedFiles())
        {
            qDebug() << str;
            mThread.AddFile(str);
        }

        mSettings.setValue("Check path", dialog.GetDefaultPath());
        dialog.SaveCheckboxValues();

        mThread.SetSettings(dialog.GetSettings());
        mCheck.setDisabled(true);

        mThread.start();
    }
}

void MainWindow::CheckDone()
{
    mCheck.setDisabled(false);
}

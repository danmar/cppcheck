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
#include <QDirIterator>
#include <QMenuBar>
#include "../src/filelister.h"


MainWindow::MainWindow() :
        mSettings(tr("CppCheck"), tr("CppCheck-GUI")),
        mActionExit(tr("E&xit"), this),
        mActionCheckFiles(tr("&Check files(s)"), this),
        mActionCheckDirectory(tr("&Check directory"), this),
        mActionSettings(tr("&Settings"), this),
        mResults(mSettings)
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(&mActionCheckFiles);
    menu->addAction(&mActionCheckDirectory);
    menu->addSeparator();
    menu->addAction(&mActionExit);

    QMenu *menuprogram = menuBar()->addMenu(tr("&Program"));
    menuprogram->addAction(&mActionSettings);

    setCentralWidget(&mResults);


    connect(&mActionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(&mActionCheckFiles, SIGNAL(triggered()), this, SLOT(CheckFiles()));
    connect(&mActionCheckDirectory, SIGNAL(triggered()), this, SLOT(CheckDirectory()));
    connect(&mActionSettings, SIGNAL(triggered()), this, SLOT(ProgramSettings()));
    connect(&mThread, SIGNAL(Done()), this, SLOT(CheckDone()));
    LoadSettings();
    mThread.Initialize(&mResults);
}

MainWindow::~MainWindow()
{
    SaveSettings();
}

void MainWindow::LoadSettings()
{
    if (mSettings.value(tr("Window maximized"), false).toBool())
    {
        showMaximized();
    }
    else
    {
        resize(mSettings.value(tr("Window width"), 800).toInt(),
               mSettings.value(tr("Window height"), 600).toInt());
    }
}

void MainWindow::SaveSettings()
{
    mSettings.setValue(tr("Window width"), size().width());
    mSettings.setValue(tr("Window height"), size().height());
    mSettings.setValue(tr("Window maximized"), isMaximized());
}


void MainWindow::DoCheckFiles(QFileDialog::FileMode mode)
{
    QFileDialog dialog(this);
    dialog.setDirectory(QDir(mSettings.value(tr("Check path"),"").toString()));
    dialog.setFileMode(mode);

    if (dialog.exec())
    {
        QStringList selected = dialog.selectedFiles();
        QStringList fileNames;
        QString selection;

        foreach(selection,selected)
        {
            fileNames << RemoveUnacceptedFiles(GetFilesRecursively(selection));
        }

        mResults.Clear();
        mThread.ClearFiles();
        mThread.SetFiles(RemoveUnacceptedFiles(fileNames));
        mSettings.setValue(tr("Check path"), dialog.directory().absolutePath());
        mActionCheckFiles.setDisabled(true);
        mThread.Check(GetCppCheckSettings());
    }
}

void MainWindow::CheckFiles()
{
    DoCheckFiles(QFileDialog::ExistingFiles);
}

void MainWindow::CheckDirectory()
{
    DoCheckFiles(QFileDialog::DirectoryOnly);
}

Settings MainWindow::GetCppCheckSettings()
{
    Settings result;
    result._debug = false;
    result._showAll = true;
    result._checkCodingStyle = true;
    result._errorsOnly = false;
    result._verbose = true;
    result._force = true;
    result._xml = false;
    result._unusedFunctions = true;
    result._security = true;
    result._jobs = mSettings.value(tr("Check threads"), 1).toInt();
    return result;
}


QStringList MainWindow::RemoveDuplicates(const QStringList &list)
{
    QHash<QString, int> hash;
    QString str;
    foreach(str, list)
    {
        hash[str] = 0;
    }

    return QStringList(hash.uniqueKeys());
}

QStringList MainWindow::GetFilesRecursively(const QString &path)
{
    QFileInfo info(path);
    QStringList list;

    if (info.isDir())
    {
        QDirIterator it(path, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            list << it.next();
        }
    }
    else
    {
        list << path;
    }

    return list;
}

QStringList MainWindow::RemoveUnacceptedFiles(const QStringList &list)
{
    QStringList result;
    QString str;
    foreach(str, list)
    {
        if (FileLister::AcceptFile(str.toStdString()))
        {
            result << str;
        }
    }

    return result;
}


void MainWindow::CheckDone()
{
    mActionCheckFiles.setDisabled(false);
}

void MainWindow::ProgramSettings()
{
    SettingsDialog dialog(mSettings);
    if (dialog.exec() == QDialog::Accepted)
    {
        dialog.SaveCheckboxValues();
    }
}


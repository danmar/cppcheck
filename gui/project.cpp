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

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include "project.h"
#include "projectfile.h"
#include "projectfiledialog.h"

Project::Project(QWidget *parent) :
    QObject(parent),
    mPFile(NULL),
    mParentWidget(parent)
{
}

Project::Project(const QString &filename, QWidget *parent) :
    QObject(parent),
    mFilename(filename),
    mPFile(NULL),
    mParentWidget(parent)
{
}

Project::~Project()
{
    delete mPFile;
}

void Project::SetFilename(const QString &filename)
{
    mFilename = filename;
}

bool Project::Open()
{
    mPFile = new ProjectFile(mFilename, this);
    if (QFile::exists(mFilename))
    {
        if (!mPFile->Read())
        {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Cppcheck"),
                            tr("Could not read the project file."),
                            QMessageBox::Ok,
                            mParentWidget);
            msg.exec();
            mFilename = QString();
            mPFile->SetFilename(mFilename);
            return false;
        }
        return true;
    }
    return false;
}

void Project::Edit()
{
    ProjectFileDialog dlg(mFilename, mParentWidget);

    QStringList includes = mPFile->GetIncludeDirs();
    dlg.SetIncludepaths(includes);
    QStringList defines = mPFile->GetDefines();
    dlg.SetDefines(defines);
    QStringList paths = mPFile->GetCheckPaths();
    dlg.SetPaths(paths);
    int rv = dlg.exec();
    if (rv == QDialog::Accepted)
    {
        QStringList includes = dlg.GetIncludePaths();
        mPFile->SetIncludes(includes);
        QStringList defines = dlg.GetDefines();
        mPFile->SetDefines(defines);
        QStringList paths = dlg.GetPaths();
        mPFile->SetCheckPaths(paths);
        bool writeSuccess = mPFile->Write();
        if (!writeSuccess)
        {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Cppcheck"),
                            tr("Could not write the project file."),
                            QMessageBox::Ok,
                            mParentWidget);
            msg.exec();
        }
    }
}

void Project::Create()
{
    mPFile = new ProjectFile(mFilename, this);
}

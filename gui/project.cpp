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

QString Project::Filename() const
{
    return mFilename;
}

void Project::SetFilename(const QString &filename)
{
    mFilename = filename;
}

bool Project::IsOpen() const
{
    return mPFile != NULL;
}

bool Project::Open()
{
    mPFile = new ProjectFile(mFilename, this);
    if (!QFile::exists(mFilename))
        return false;

    if (!mPFile->Read()) {
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

bool Project::Edit()
{
    ProjectFileDialog dlg(mFilename, mParentWidget);
    dlg.SetRootPath(mPFile->GetRootPath());
    dlg.SetBuildDir(mPFile->GetBuildDir());
    dlg.SetIncludepaths(mPFile->GetIncludeDirs());
    dlg.SetDefines(mPFile->GetDefines());
    dlg.SetPaths(mPFile->GetCheckPaths());
    dlg.SetImportProject(mPFile->GetImportProject());
    dlg.SetExcludedPaths(mPFile->GetExcludedPaths());
    dlg.SetLibraries(mPFile->GetLibraries());
    dlg.SetSuppressions(mPFile->GetSuppressions());

    if (dlg.exec() != QDialog::Accepted)
        return false;

    mPFile->SetRootPath(dlg.GetRootPath());
    mPFile->SetBuildDir(dlg.GetBuildDir());
    mPFile->SetImportProject(dlg.GetImportProject());
    mPFile->SetIncludes(dlg.GetIncludePaths());
    mPFile->SetDefines(dlg.GetDefines());
    mPFile->SetCheckPaths(dlg.GetPaths());
    mPFile->SetExcludedPaths(dlg.GetExcludedPaths());
    mPFile->SetLibraries(dlg.GetLibraries());
    mPFile->SetSuppressions(dlg.GetSuppressions());

    if (!mPFile->Write()) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Could not write the project file."),
                        QMessageBox::Ok,
                        mParentWidget);
        msg.exec();
        return false;
    }

    return true;
}

void Project::Create()
{
    mPFile = new ProjectFile(mFilename, this);
}

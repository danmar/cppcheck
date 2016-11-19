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
    mProjectFile(NULL),
    mParentWidget(parent)
{
}

Project::Project(const QString &filename, QWidget *parent) :
    QObject(parent),
    mFilename(filename),
    mProjectFile(NULL),
    mParentWidget(parent)
{
}

Project::~Project()
{
    delete mProjectFile;
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
    return mProjectFile != NULL;
}

bool Project::Open()
{
    mProjectFile = new ProjectFile(mFilename, this);
    if (!QFile::exists(mFilename))
        return false;

    if (!mProjectFile->Read()) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Could not read the project file."),
                        QMessageBox::Ok,
                        mParentWidget);
        msg.exec();
        mFilename = QString();
        mProjectFile->SetFilename(mFilename);
        return false;
    }

    return true;
}

bool Project::Edit()
{
    ProjectFileDialog dlg(mFilename, mParentWidget);
    dlg.SetRootPath(mProjectFile->GetRootPath());
    dlg.SetBuildDir(mProjectFile->GetBuildDir());
    dlg.SetIncludepaths(mProjectFile->GetIncludeDirs());
    dlg.SetDefines(mProjectFile->GetDefines());
    dlg.SetPaths(mProjectFile->GetCheckPaths());
    dlg.SetImportProject(mProjectFile->GetImportProject());
    dlg.SetExcludedPaths(mProjectFile->GetExcludedPaths());
    dlg.SetLibraries(mProjectFile->GetLibraries());
    dlg.SetSuppressions(mProjectFile->GetSuppressions());

    if (dlg.exec() != QDialog::Accepted)
        return false;

    mProjectFile->SetRootPath(dlg.GetRootPath());
    mProjectFile->SetBuildDir(dlg.GetBuildDir());
    mProjectFile->SetImportProject(dlg.GetImportProject());
    mProjectFile->SetIncludes(dlg.GetIncludePaths());
    mProjectFile->SetDefines(dlg.GetDefines());
    mProjectFile->SetCheckPaths(dlg.GetPaths());
    mProjectFile->SetExcludedPaths(dlg.GetExcludedPaths());
    mProjectFile->SetLibraries(dlg.GetLibraries());
    mProjectFile->SetSuppressions(dlg.GetSuppressions());

    if (!mProjectFile->Write()) {
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
    mProjectFile = new ProjectFile(mFilename, this);
}

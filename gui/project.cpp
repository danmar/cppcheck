/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
    if (QFile::exists(mFilename)) {
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
    return false;
}

bool Project::Edit()
{
    ProjectFileDialog dlg(mFilename, mParentWidget);
    QString root = mPFile->GetRootPath();
    dlg.SetRootPath(root);
    QStringList includes = mPFile->GetIncludeDirs();
    dlg.SetIncludepaths(includes);
    QStringList defines = mPFile->GetDefines();
    dlg.SetDefines(defines);
    QStringList paths = mPFile->GetCheckPaths();
    dlg.SetPaths(paths);
    QStringList ignorepaths = mPFile->GetExcludedPaths();
    dlg.SetExcludedPaths(ignorepaths);
    QStringList libraries = mPFile->GetLibraries();
    dlg.SetLibraries(libraries);
    QStringList suppressions = mPFile->GetSuppressions();
    dlg.SetSuppressions(suppressions);

    int rv = dlg.exec();
    if (rv == QDialog::Accepted) {
        QString root = dlg.GetRootPath();
        mPFile->SetRootPath(root);
        QStringList includes = dlg.GetIncludePaths();
        mPFile->SetIncludes(includes);
        QStringList defines = dlg.GetDefines();
        mPFile->SetDefines(defines);
        QStringList paths = dlg.GetPaths();
        mPFile->SetCheckPaths(paths);
        QStringList excludedpaths = dlg.GetExcludedPaths();
        mPFile->SetExcludedPaths(excludedpaths);
        QStringList libraries = dlg.GetLibraries();
        mPFile->SetLibraries(libraries);
        QStringList suppressions = dlg.GetSuppressions();
        mPFile->SetSuppressions(suppressions);

        bool writeSuccess = mPFile->Write();
        if (!writeSuccess) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Cppcheck"),
                            tr("Could not write the project file."),
                            QMessageBox::Ok,
                            mParentWidget);
            msg.exec();
        }
        return writeSuccess;
    }
    return false;
}

void Project::Create()
{
    mPFile = new ProjectFile(mFilename, this);
}

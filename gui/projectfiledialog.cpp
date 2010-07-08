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

#include <QStringList>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "projectfiledialog.h"
#include "projectfile.h"

ProjectFileDialog::ProjectFileDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
    , mFileName(path)
    , mDataSaved(false)
{
    mUI.setupUi(this);

    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(DialogAccepted()));

    mPFile = new ProjectFile(path, this);
    if (QFile::exists(path))
    {
        ReadProjectFile();
    }
}

void ProjectFileDialog::ReadProjectFile()
{
    if (!mPFile->Read())
    {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Could not read the project file."),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        mFileName = QString();
        mPFile->SetFilename(mFileName);
    }

    QStringList includes = mPFile->GetIncludeDirs();
    QString includestr;
    QString dir;
    foreach(dir, includes)
    {
        includestr += dir;
        includestr += ";";
    }
    mUI.mEditIncludePaths->setText(includestr);

    QStringList defines = mPFile->GetDefines();
    QString definestr;
    QString define;
    foreach(define, defines)
    {
        definestr += define;
        definestr += ";";
    }
    mUI.mEditDefines->setText(definestr);
}

void ProjectFileDialog::DialogAccepted()
{
    if (mDataSaved)
        return;

    UpdateProjectFileData();
    bool writesuccess = false;
    if (mFileName.isEmpty())
    {
        const QString filter = tr("Project files (*.cppcheck);;All files(*.*)");
        QString filepath = QFileDialog::getSaveFileName(this,
                           tr("Save Project File"),
                           QString(),
                           filter);

        if (!filepath.isEmpty())
        {
            writesuccess = mPFile->Write(filepath);
        }
    }
    else
    {
        writesuccess = mPFile->Write();
    }

    if (!writesuccess)
    {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("Could not write the project file."),
                        QMessageBox::Ok,
                        this);
        msg.exec();
    }
    mDataSaved = true;
}

void ProjectFileDialog::UpdateProjectFileData()
{
    QString include = mUI.mEditIncludePaths->text();
    QStringList includes;
    if (!include.isEmpty())
    {
        include = include.trimmed();
        if (include.indexOf(';') != -1)
            includes = include.split(";");
        else
            includes.append(include);
    }
    mPFile->SetIncludes(includes);

    QString define = mUI.mEditDefines->text();
    QStringList defines;
    if (!define.isEmpty())
    {
        define = define.trimmed();
        if (define.indexOf(';') != -1)
            defines = define.split(";");
        else
            defines.append(define);
    }
    mPFile->SetDefines(defines);
}

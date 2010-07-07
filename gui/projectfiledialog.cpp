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
#include "projectfiledialog.h"
#include "projectfile.h"

ProjectFileDialog::ProjectFileDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
    , mFileName(path)
{
    mUI.setupUi(this);

    mPFile = new ProjectFile(path, this);
    mPFile->Read();

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

    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
}

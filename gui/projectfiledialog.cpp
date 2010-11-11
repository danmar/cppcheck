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

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QFileDialog>
#include <QLineEdit>
#include "projectfiledialog.h"

ProjectFileDialog::ProjectFileDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
{
    mUI.setupUi(this);

    QFileInfo inf(path);
    QString filename = inf.fileName();
    QString title = tr("Project file: %1").arg(filename);
    setWindowTitle(title);

    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mUI.mBtnBrowseIncludes, SIGNAL(clicked()), this, SLOT(BrowseIncludes()));
    connect(mUI.mBtnBrowsePaths, SIGNAL(clicked()), this, SLOT(BrowsePaths()));
}

QString ProjectFileDialog::GetRootPath() const
{
    QString root = mUI.mEditProjectRoot->text();
    root = root.trimmed();
    return root;
}

QStringList ProjectFileDialog::GetIncludePaths() const
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
    return includes;
}

QStringList ProjectFileDialog::GetDefines() const
{
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
    return defines;
}

QStringList ProjectFileDialog::GetPaths() const
{
    QString path = mUI.mEditPaths->text();
    QStringList paths;
    if (!path.isEmpty())
    {
        path = path.trimmed();
        if (path.indexOf(';') != -1)
            paths = path.split(";");
        else
            paths.append(path);
    }
    return paths;
}

void ProjectFileDialog::SetRootPath(const QString &root)
{
    mUI.mEditProjectRoot->setText(root);
}

void ProjectFileDialog::SetIncludepaths(const QStringList &includes)
{
    QString includestr;
    QString dir;
    foreach(dir, includes)
    {
        includestr += dir;
        includestr += ";";
    }
    // Remove ; from the end of the string
    if (includestr.endsWith(';'))
        includestr = includestr.left(includestr.length() - 1);
    mUI.mEditIncludePaths->setText(includestr);
}

void ProjectFileDialog::SetDefines(const QStringList &defines)
{
    QString definestr;
    QString define;
    foreach(define, defines)
    {
        definestr += define;
        definestr += ";";
    }
    // Remove ; from the end of the string
    if (definestr.endsWith(';'))
        definestr = definestr.left(definestr.length() - 1);
    mUI.mEditDefines->setText(definestr);
}

void ProjectFileDialog::SetPaths(const QStringList &paths)
{
    QString pathstr;
    QString path;
    foreach(path, paths)
    {
        pathstr += path;
        pathstr += ";";
    }
    // Remove ; from the end of the string
    if (pathstr.endsWith(';'))
        pathstr = pathstr.left(pathstr.length() - 1);
    mUI.mEditPaths->setText(pathstr);
}

void ProjectFileDialog::BrowseIncludes()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select include directory"),
                          QString());

    if (!selectedDir.isEmpty())
    {
        AppendDirname(mUI.mEditIncludePaths, selectedDir);
    }
}

void ProjectFileDialog::BrowsePaths()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select directory to check"),
                          QString());

    if (!selectedDir.isEmpty())
    {
        AppendDirname(mUI.mEditPaths, selectedDir);
    }
}

void ProjectFileDialog::AppendDirname(QLineEdit *edit, const QString &dir)
{
    QString wholeText = edit->text();
    wholeText += ";";
    wholeText += dir;
    edit->setText(wholeText);
}

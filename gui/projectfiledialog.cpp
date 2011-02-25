/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
#include <QDir>
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
    connect(mUI.mBtnAddInclude, SIGNAL(clicked()), this, SLOT(AddIncludeDir()));
    connect(mUI.mBtnBrowsePaths, SIGNAL(clicked()), this, SLOT(BrowsePaths()));
    connect(mUI.mBtnEditInclude, SIGNAL(clicked()), this, SLOT(EditIncludeDir()));
    connect(mUI.mBtnRemoveInclude, SIGNAL(clicked()), this, SLOT(RemoveIncludeDir()));
}

void ProjectFileDialog::AddIncludeDir(const QString &dir)
{
    if (dir.isNull() || dir.isEmpty())
        return;

    QListWidgetItem *item = new QListWidgetItem(dir);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListIncludeDirs->addItem(item);
}

QString ProjectFileDialog::GetRootPath() const
{
    QString root = mUI.mEditProjectRoot->text();
    root = root.trimmed();
    return root;
}

QStringList ProjectFileDialog::GetIncludePaths() const
{
    const int count = mUI.mListIncludeDirs->count();
    QStringList includePaths;
    for (int i = 0; i < count; i++)
    {
        QListWidgetItem *item = mUI.mListIncludeDirs->item(i);
        includePaths << item->text();
    }
    return includePaths;
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
        path = QDir::fromNativeSeparators(path);
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
    QString dir;
    foreach(dir, includes)
    {
        AddIncludeDir(dir);
    }
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
    pathstr = QDir::toNativeSeparators(pathstr);
    mUI.mEditPaths->setText(pathstr);
}

void ProjectFileDialog::AddIncludeDir()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select include directory"),
                          QString());

    if (!selectedDir.isEmpty())
    {
        AddIncludeDir(selectedDir);
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
    if (!wholeText.endsWith(QDir::separator()))
        wholeText += QDir::separator();
    edit->setText(wholeText);
}

void ProjectFileDialog::RemoveIncludeDir()
{
    const int row = mUI.mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI.mListIncludeDirs->takeItem(row);
    delete item;
}

void ProjectFileDialog::EditIncludeDir()
{
    QListWidgetItem *item = mUI.mListIncludeDirs->currentItem();
    mUI.mListIncludeDirs->editItem(item);
}

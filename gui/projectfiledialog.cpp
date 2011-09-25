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
#include <QSettings>
#include "common.h"
#include "projectfiledialog.h"

ProjectFileDialog::ProjectFileDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
    , mFilePath(path)
{
    mUI.setupUi(this);

    QFileInfo inf(path);
    QString filename = inf.fileName();
    QString title = tr("Project file: %1").arg(filename);
    setWindowTitle(title);
    LoadSettings();

    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mUI.mBtnAddInclude, SIGNAL(clicked()), this, SLOT(AddIncludeDir()));
    connect(mUI.mBtnAddPath, SIGNAL(clicked()), this, SLOT(AddPath()));
    connect(mUI.mBtnEditInclude, SIGNAL(clicked()), this, SLOT(EditIncludeDir()));
    connect(mUI.mBtnRemoveInclude, SIGNAL(clicked()), this, SLOT(RemoveIncludeDir()));
    connect(mUI.mBtnEditPath, SIGNAL(clicked()), this, SLOT(EditPath()));
    connect(mUI.mBtnRemovePath, SIGNAL(clicked()), this, SLOT(RemovePath()));
    connect(mUI.mBtnAddIgnorePath, SIGNAL(clicked()), this, SLOT(AddExcludePath()));
    connect(mUI.mBtnEditIgnorePath, SIGNAL(clicked()), this, SLOT(EditExcludePath()));
    connect(mUI.mBtnRemoveIgnorePath, SIGNAL(clicked()), this, SLOT(RemoveExcludePath()));
    connect(mUI.mBtnIncludeUp, SIGNAL(clicked()), this, SLOT(MoveIncludePathUp()));
    connect(mUI.mBtnIncludeDown, SIGNAL(clicked()), this, SLOT(MoveIncludePathDown()));
}

ProjectFileDialog::~ProjectFileDialog()
{
    SaveSettings();
}

void ProjectFileDialog::LoadSettings()
{
    QSettings settings;
    resize(settings.value(SETTINGS_PROJECT_DIALOG_WIDTH, 470).toInt(),
           settings.value(SETTINGS_PROJECT_DIALOG_HEIGHT, 330).toInt());
}

void ProjectFileDialog::SaveSettings()
{
    QSettings settings;
    settings.setValue(SETTINGS_PROJECT_DIALOG_WIDTH, size().width());
    settings.setValue(SETTINGS_PROJECT_DIALOG_HEIGHT, size().height());
}

void ProjectFileDialog::AddIncludeDir(const QString &dir)
{
    if (dir.isNull() || dir.isEmpty())
        return;

    const QString newdir = QDir::toNativeSeparators(dir);
    QListWidgetItem *item = new QListWidgetItem(newdir);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListIncludeDirs->addItem(item);
}

void ProjectFileDialog::AddPath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    QListWidgetItem *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListPaths->addItem(item);
}

void ProjectFileDialog::AddExcludePath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    QListWidgetItem *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListExcludedPaths->addItem(item);
}

QString ProjectFileDialog::GetRootPath() const
{
    QString root = mUI.mEditProjectRoot->text();
    root = root.trimmed();
    root = QDir::fromNativeSeparators(root);
    return root;
}

QStringList ProjectFileDialog::GetIncludePaths() const
{
    const int count = mUI.mListIncludeDirs->count();
    QStringList includePaths;
    for (int i = 0; i < count; i++)
    {
        QListWidgetItem *item = mUI.mListIncludeDirs->item(i);
        includePaths << QDir::fromNativeSeparators(item->text());
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
    const int count = mUI.mListPaths->count();
    QStringList paths;
    for (int i = 0; i < count; i++)
    {
        QListWidgetItem *item = mUI.mListPaths->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

QStringList ProjectFileDialog::GetExcludedPaths() const
{
    const int count = mUI.mListExcludedPaths->count();
    QStringList paths;
    for (int i = 0; i < count; i++)
    {
        QListWidgetItem *item = mUI.mListExcludedPaths->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

void ProjectFileDialog::SetRootPath(const QString &root)
{
    QString newroot = QDir::toNativeSeparators(root);
    mUI.mEditProjectRoot->setText(newroot);
}

void ProjectFileDialog::SetIncludepaths(const QStringList &includes)
{
    foreach(QString dir, includes)
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
    foreach(QString path, paths)
    {
        AddPath(path);
    }
}

void ProjectFileDialog::SetExcludedPaths(const QStringList &paths)
{
    foreach(QString path, paths)
    {
        AddExcludePath(path);
    }
}

void ProjectFileDialog::AddIncludeDir()
{
    QFileInfo inf(mFilePath);
    const QString rootpath = inf.absolutePath();
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select include directory"),
                          rootpath);

    if (!selectedDir.isEmpty())
    {
        AddIncludeDir(selectedDir);
    }
}

void ProjectFileDialog::AddPath()
{
    QFileInfo inf(mFilePath);
    const QString rootpath = inf.absolutePath();
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select a directory to check"),
                          rootpath);

    if (!selectedDir.isEmpty())
    {
        AddPath(selectedDir);
    }
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

void ProjectFileDialog::EditPath()
{
    QListWidgetItem *item = mUI.mListPaths->currentItem();
    mUI.mListPaths->editItem(item);
}

void ProjectFileDialog::RemovePath()
{
    const int row = mUI.mListPaths->currentRow();
    QListWidgetItem *item = mUI.mListPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::AddExcludePath()
{
    QFileInfo inf(mFilePath);
    const QString rootpath = inf.absolutePath();

    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select directory to ignore"),
                          rootpath);

    if (!selectedDir.isEmpty())
    {
        if (!selectedDir.endsWith('/'))
            selectedDir += '/';
        AddExcludePath(selectedDir);
    }
}

void ProjectFileDialog::EditExcludePath()
{
    QListWidgetItem *item = mUI.mListExcludedPaths->currentItem();
    mUI.mListExcludedPaths->editItem(item);
}

void ProjectFileDialog::RemoveExcludePath()
{
    const int row = mUI.mListExcludedPaths->currentRow();
    QListWidgetItem *item = mUI.mListExcludedPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::MoveIncludePathUp()
{
    int row = mUI.mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI.mListIncludeDirs->takeItem(row);
    row = row > 0 ? row - 1 : 0;
    mUI.mListIncludeDirs->insertItem(row, item);
    mUI.mListIncludeDirs->setCurrentItem(item);
}

void ProjectFileDialog::MoveIncludePathDown()
{
    int row = mUI.mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI.mListIncludeDirs->takeItem(row);
    const int count = mUI.mListIncludeDirs->count();
    row = row < count ? row + 1 : count;
    mUI.mListIncludeDirs->insertItem(row, item);
    mUI.mListIncludeDirs->setCurrentItem(item);
}

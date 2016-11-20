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

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>
#include <QSettings>
#include "common.h"
#include "projectfiledialog.h"
#include "projectfile.h"
#include "library.h"
#include "cppcheck.h"
#include "errorlogger.h"

ProjectFileDialog::ProjectFileDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
    , mFilePath(path)
{
    mUI.setupUi(this);

    const QFileInfo inf(path);
    QString filename = inf.fileName();
    QString title = tr("Project file: %1").arg(filename);
    setWindowTitle(title);
    LoadSettings();

    // Checkboxes for the libraries..
    const QString applicationFilePath = QCoreApplication::applicationFilePath();
    const QString appPath = QFileInfo(applicationFilePath).canonicalPath();
    QSettings settings;
    const QString datadir = settings.value("DATADIR",QString()).toString();
    QStringList searchPaths;
    searchPaths << appPath << appPath + "/cfg" << inf.canonicalPath();
    if (!datadir.isEmpty())
        searchPaths << datadir << datadir + "/cfg";
    QStringList libs;
    foreach (const QString sp, searchPaths) {
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.cfg"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        foreach (QFileInfo item, dir.entryInfoList()) {
            QString library = item.fileName();
            {
                Library lib;
                const QString fullfilename = sp + "/" + library;
                const Library::Error err = lib.load(nullptr, fullfilename.toLatin1());
                if (err.errorcode != Library::OK)
                    continue;
            }
            library.chop(4);
            if (library.compare("std", Qt::CaseInsensitive) == 0)
                continue;
            if (libs.indexOf(library) == -1)
                libs << library;
        }
    }
    qSort(libs);
    foreach (const QString library, libs) {
        QCheckBox *checkbox = new QCheckBox(this);
        checkbox->setText(library);
        mUI.mLayoutLibraries->addWidget(checkbox);
        mLibraryCheckboxes << checkbox;
    }

    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mUI.mBtnBrowseBuildDir, SIGNAL(clicked()), this, SLOT(BrowseBuildDir()));
    connect(mUI.mBtnBrowseCompileDatabase, SIGNAL(clicked()), this, SLOT(BrowseCompileDatabase()));
    connect(mUI.mBtnBrowseVisualStudio, SIGNAL(clicked()), this, SLOT(BrowseVisualStudio()));
    connect(mUI.mBtnAddCheckPath, SIGNAL(clicked()), this, SLOT(AddCheckPath()));
    connect(mUI.mBtnEditCheckPath, SIGNAL(clicked()), this, SLOT(EditCheckPath()));
    connect(mUI.mBtnRemoveCheckPath, SIGNAL(clicked()), this, SLOT(RemoveCheckPath()));
    connect(mUI.mBtnAddInclude, SIGNAL(clicked()), this, SLOT(AddIncludeDir()));
    connect(mUI.mBtnEditInclude, SIGNAL(clicked()), this, SLOT(EditIncludeDir()));
    connect(mUI.mBtnRemoveInclude, SIGNAL(clicked()), this, SLOT(RemoveIncludeDir()));
    connect(mUI.mBtnAddIgnorePath, SIGNAL(clicked()), this, SLOT(AddExcludePath()));
    connect(mUI.mBtnEditIgnorePath, SIGNAL(clicked()), this, SLOT(EditExcludePath()));
    connect(mUI.mBtnRemoveIgnorePath, SIGNAL(clicked()), this, SLOT(RemoveExcludePath()));
    connect(mUI.mBtnIncludeUp, SIGNAL(clicked()), this, SLOT(MoveIncludePathUp()));
    connect(mUI.mBtnIncludeDown, SIGNAL(clicked()), this, SLOT(MoveIncludePathDown()));
    connect(mUI.mBtnAddSuppression, SIGNAL(clicked()), this, SLOT(AddSuppression()));
    connect(mUI.mBtnRemoveSuppression, SIGNAL(clicked()), this, SLOT(RemoveSuppression()));
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

void ProjectFileDialog::SaveSettings() const
{
    QSettings settings;
    settings.setValue(SETTINGS_PROJECT_DIALOG_WIDTH, size().width());
    settings.setValue(SETTINGS_PROJECT_DIALOG_HEIGHT, size().height());
}

void ProjectFileDialog::LoadFromProjectFile(const ProjectFile *projectFile)
{
    SetRootPath(projectFile->GetRootPath());
    SetBuildDir(projectFile->GetBuildDir());
    SetIncludepaths(projectFile->GetIncludeDirs());
    SetDefines(projectFile->GetDefines());
    SetCheckPaths(projectFile->GetCheckPaths());
    SetImportProject(projectFile->GetImportProject());
    SetExcludedPaths(projectFile->GetExcludedPaths());
    SetLibraries(projectFile->GetLibraries());
    SetSuppressions(projectFile->GetSuppressions());
}

void ProjectFileDialog::SaveToProjectFile(ProjectFile *projectFile) const
{
    projectFile->SetRootPath(GetRootPath());
    projectFile->SetBuildDir(GetBuildDir());
    projectFile->SetImportProject(GetImportProject());
    projectFile->SetIncludes(GetIncludePaths());
    projectFile->SetDefines(GetDefines());
    projectFile->SetCheckPaths(GetCheckPaths());
    projectFile->SetExcludedPaths(GetExcludedPaths());
    projectFile->SetLibraries(GetLibraries());
    projectFile->SetSuppressions(GetSuppressions());
}

QString ProjectFileDialog::getExistingDirectory(const QString &caption, bool trailingSlash)
{
    const QFileInfo inf(mFilePath);
    const QString rootpath = inf.absolutePath();
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          caption,
                          rootpath);

    if (selectedDir.isEmpty())
        return QString();

    // Check if the path is relative to project file's path and if so
    // make it a relative path instead of absolute path.
    const QDir dir(rootpath);
    const QString relpath(dir.relativeFilePath(selectedDir));
    if (!relpath.startsWith("."))
        selectedDir = relpath;

    // Trailing slash..
    if (trailingSlash && !selectedDir.endsWith('/'))
        selectedDir += '/';

    return selectedDir;
}

void ProjectFileDialog::BrowseBuildDir()
{
    const QString dir(getExistingDirectory(tr("Select Cppcheck build dir"), false));
    if (!dir.isEmpty())
        mUI.mEditBuildDir->setText(dir);
}

void ProjectFileDialog::BrowseCompileDatabase()
{
    const QFileInfo inf(mFilePath);
    const QDir &dir = inf.absoluteDir();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Compile Database"),
                       dir.canonicalPath(),
                       tr("Compile database (compile_database.json)"));
    mUI.mEditCompileDatabase->setText(dir.relativeFilePath(fileName));
}

void ProjectFileDialog::BrowseVisualStudio()
{
    const QFileInfo inf(mFilePath);
    const QDir &dir = inf.absoluteDir();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Visual Studio"),
                       dir.canonicalPath(),
                       tr("Visual Studio Solution/Project (*.sln *.vcxproj)"));
    mUI.mEditVisualStudio->setText(dir.relativeFilePath(fileName));
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

void ProjectFileDialog::AddCheckPath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    QListWidgetItem *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListCheckPaths->addItem(item);
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

QString ProjectFileDialog::GetBuildDir() const
{
    return mUI.mEditBuildDir->text();
}

QString ProjectFileDialog::GetImportProject() const
{
    return mUI.mEditCompileDatabase->text() + mUI.mEditVisualStudio->text();
}

QStringList ProjectFileDialog::GetIncludePaths() const
{
    const int count = mUI.mListIncludeDirs->count();
    QStringList includePaths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListIncludeDirs->item(i);
        includePaths << QDir::fromNativeSeparators(item->text());
    }
    return includePaths;
}

QStringList ProjectFileDialog::GetDefines() const
{
    QString define = mUI.mEditDefines->text();
    QStringList defines;
    if (!define.isEmpty()) {
        define = define.trimmed();
        if (define.indexOf(';') != -1)
            defines = define.split(";");
        else
            defines.append(define);
    }
    return defines;
}

QStringList ProjectFileDialog::GetCheckPaths() const
{
    const int count = mUI.mListCheckPaths->count();
    QStringList paths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListCheckPaths->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

QStringList ProjectFileDialog::GetExcludedPaths() const
{
    const int count = mUI.mListExcludedPaths->count();
    QStringList paths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListExcludedPaths->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

QStringList ProjectFileDialog::GetLibraries() const
{
    QStringList libraries;
    foreach (const QCheckBox *checkbox, mLibraryCheckboxes) {
        if (checkbox->isChecked())
            libraries << checkbox->text();
    }
    return libraries;
}

QStringList ProjectFileDialog::GetSuppressions() const
{
    QStringList suppressions;
    const int count = mUI.mListSuppressions->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListSuppressions->item(i);
        suppressions << item->text();
    }
    return suppressions;
}

void ProjectFileDialog::SetRootPath(const QString &root)
{
    mUI.mEditProjectRoot->setText(QDir::toNativeSeparators(root));
}

void ProjectFileDialog::SetBuildDir(const QString &buildDir)
{
    mUI.mEditBuildDir->setText(buildDir);
}

void ProjectFileDialog::SetImportProject(const QString &importProject)
{
    mUI.mEditCompileDatabase->setText("");
    mUI.mEditVisualStudio->setText("");
    if (importProject.endsWith("compile_database.json", Qt::CaseInsensitive))
        mUI.mEditCompileDatabase->setText(importProject);
    else if (importProject.endsWith(".sln",Qt::CaseInsensitive) || importProject.endsWith(".vcxproj",Qt::CaseInsensitive))
        mUI.mEditVisualStudio->setText(importProject);
}

void ProjectFileDialog::SetIncludepaths(const QStringList &includes)
{
    foreach (QString dir, includes) {
        AddIncludeDir(dir);
    }
}

void ProjectFileDialog::SetDefines(const QStringList &defines)
{
    QString definestr;
    QString define;
    foreach (define, defines) {
        definestr += define;
        definestr += ";";
    }
    // Remove ; from the end of the string
    if (definestr.endsWith(';'))
        definestr = definestr.left(definestr.length() - 1);
    mUI.mEditDefines->setText(definestr);
}

void ProjectFileDialog::SetCheckPaths(const QStringList &paths)
{
    foreach (QString path, paths) {
        AddCheckPath(path);
    }
}

void ProjectFileDialog::SetExcludedPaths(const QStringList &paths)
{
    foreach (QString path, paths) {
        AddExcludePath(path);
    }
}

void ProjectFileDialog::SetLibraries(const QStringList &libraries)
{
    for (int i = 0; i < mLibraryCheckboxes.size(); i++) {
        QCheckBox *checkbox = mLibraryCheckboxes[i];
        checkbox->setChecked(libraries.contains(checkbox->text()));
    }
}

void ProjectFileDialog::SetSuppressions(const QStringList &suppressions)
{
    mUI.mListSuppressions->clear();
    mUI.mListSuppressions->addItems(suppressions);
    mUI.mListSuppressions->sortItems();
}

void ProjectFileDialog::AddCheckPath()
{
    QString dir = getExistingDirectory(tr("Select a directory to check"), false);
    if (!dir.isEmpty())
        AddCheckPath(dir);
}

void ProjectFileDialog::EditCheckPath()
{
    QListWidgetItem *item = mUI.mListCheckPaths->currentItem();
    mUI.mListCheckPaths->editItem(item);
}

void ProjectFileDialog::RemoveCheckPath()
{
    const int row = mUI.mListCheckPaths->currentRow();
    QListWidgetItem *item = mUI.mListCheckPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::AddIncludeDir()
{
    const QString dir = getExistingDirectory(tr("Select include directory"), true);
    if (!dir.isEmpty())
        AddIncludeDir(dir);
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

void ProjectFileDialog::AddExcludePath()
{
    QString dir = getExistingDirectory(tr("Select directory to ignore"), true);
    if (!dir.isEmpty())
        AddExcludePath(dir);
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

void ProjectFileDialog::AddSuppression()
{
    class QErrorLogger : public ErrorLogger {
    public:
        virtual void reportOut(const std::string &/*outmsg*/) {}
        virtual void reportErr(const ErrorLogger::ErrorMessage &msg) {
            errorIds << QString::fromStdString(msg._id);
        }
        QStringList errorIds;
    };

    QErrorLogger errorLogger;
    CppCheck cppcheck(errorLogger,false);
    cppcheck.getErrorMessages();
    errorLogger.errorIds.sort();

    bool ok;
    QString item = QInputDialog::getItem(this, tr("Add Suppression"),
                                         tr("Select error id suppress:"), errorLogger.errorIds, 0, false, &ok);
    if (ok && !item.isEmpty()) {
        mUI.mListSuppressions->addItem(item);
        mUI.mListSuppressions->sortItems();
    }
}

void ProjectFileDialog::RemoveSuppression()
{
    const int row = mUI.mListSuppressions->currentRow();
    QListWidgetItem *item = mUI.mListSuppressions->takeItem(row);
    delete item;
}

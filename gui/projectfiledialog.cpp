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
#include <QProcess>
#include "common.h"
#include "projectfiledialog.h"
#include "checkthread.h"
#include "projectfile.h"
#include "library.h"
#include "cppcheck.h"
#include "errorlogger.h"

ProjectFileDialog::ProjectFileDialog(ProjectFile *projectFile, QWidget *parent)
    : QDialog(parent)
    , mProjectFile(projectFile)
{
    mUI.setupUi(this);

    mUI.mToolClangAnalyzer->hide();

    const QFileInfo inf(projectFile->getFilename());
    QString filename = inf.fileName();
    QString title = tr("Project file: %1").arg(filename);
    setWindowTitle(title);
    loadSettings();

    // Checkboxes for the libraries..
    const QString applicationFilePath = QCoreApplication::applicationFilePath();
    const QString appPath = QFileInfo(applicationFilePath).canonicalPath();
    QSettings settings;
#ifdef CFGDIR
    const QString cfgdir = CFGDIR;
#endif
    const QString datadir = settings.value("DATADIR",QString()).toString();
    QStringList searchPaths;
    searchPaths << appPath << appPath + "/cfg" << inf.canonicalPath();
#ifdef CFGDIR
    if (!cfgdir.isEmpty())
        searchPaths << cfgdir << cfgdir + "/cfg";
#endif
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

    mUI.mEditTags->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9 ;]*"),this));

    connect(mUI.mButtons, &QDialogButtonBox::accepted, this, &ProjectFileDialog::ok);
    connect(mUI.mBtnBrowseBuildDir, &QPushButton::clicked, this, &ProjectFileDialog::browseBuildDir);
    connect(mUI.mBtnClearImportProject, &QPushButton::clicked, this, &ProjectFileDialog::clearImportProject);
    connect(mUI.mBtnBrowseImportProject, &QPushButton::clicked, this, &ProjectFileDialog::browseImportProject);
    connect(mUI.mBtnAddCheckPath, SIGNAL(clicked()), this, SLOT(addCheckPath()));
    connect(mUI.mBtnEditCheckPath, &QPushButton::clicked, this, &ProjectFileDialog::editCheckPath);
    connect(mUI.mBtnRemoveCheckPath, &QPushButton::clicked, this, &ProjectFileDialog::removeCheckPath);
    connect(mUI.mBtnAddInclude, SIGNAL(clicked()), this, SLOT(addIncludeDir()));
    connect(mUI.mBtnEditInclude, &QPushButton::clicked, this, &ProjectFileDialog::editIncludeDir);
    connect(mUI.mBtnRemoveInclude, &QPushButton::clicked, this, &ProjectFileDialog::removeIncludeDir);
    connect(mUI.mBtnAddIgnorePath, SIGNAL(clicked()), this, SLOT(addExcludePath()));
    connect(mUI.mBtnEditIgnorePath, &QPushButton::clicked, this, &ProjectFileDialog::editExcludePath);
    connect(mUI.mBtnRemoveIgnorePath, &QPushButton::clicked, this, &ProjectFileDialog::removeExcludePath);
    connect(mUI.mBtnIncludeUp, &QPushButton::clicked, this, &ProjectFileDialog::moveIncludePathUp);
    connect(mUI.mBtnIncludeDown, &QPushButton::clicked, this, &ProjectFileDialog::moveIncludePathDown);
    connect(mUI.mBtnAddSuppression, &QPushButton::clicked, this, &ProjectFileDialog::addSuppression);
    connect(mUI.mBtnRemoveSuppression, &QPushButton::clicked, this, &ProjectFileDialog::removeSuppression);

    loadFromProjectFile(projectFile);
}

ProjectFileDialog::~ProjectFileDialog()
{
    saveSettings();
}

void ProjectFileDialog::loadSettings()
{
    QSettings settings;
    resize(settings.value(SETTINGS_PROJECT_DIALOG_WIDTH, 470).toInt(),
           settings.value(SETTINGS_PROJECT_DIALOG_HEIGHT, 330).toInt());
}

void ProjectFileDialog::saveSettings() const
{
    QSettings settings;
    settings.setValue(SETTINGS_PROJECT_DIALOG_WIDTH, size().width());
    settings.setValue(SETTINGS_PROJECT_DIALOG_HEIGHT, size().height());
}

static void updateAddonCheckBox(QCheckBox *cb, const ProjectFile *projectFile, const QString &dataDir, const QString &addon)
{
    cb->setChecked(projectFile->getAddons().contains(addon));
    if (CheckThread::getAddonFilePath(dataDir, addon + ".py").isEmpty()) {
        cb->setEnabled(false);
        cb->setText(cb->text() + QObject::tr(" (Not found)"));
    }
}

void ProjectFileDialog::loadFromProjectFile(const ProjectFile *projectFile)
{
    setRootPath(projectFile->getRootPath());
    setBuildDir(projectFile->getBuildDir());
    setIncludepaths(projectFile->getIncludeDirs());
    setDefines(projectFile->getDefines());
    setCheckPaths(projectFile->getCheckPaths());
    setImportProject(projectFile->getImportProject());
    mUI.mChkAllVsConfigs->setChecked(projectFile->getAnalyzeAllVsConfigs());
    setExcludedPaths(projectFile->getExcludedPaths());
    setLibraries(projectFile->getLibraries());
    setSuppressions(projectFile->getSuppressions());

    QSettings settings;
    const QString dataDir = settings.value("DATADIR", QString()).toString();
    updateAddonCheckBox(mUI.mAddonThreadSafety, projectFile, dataDir, "threadsafety");
    updateAddonCheckBox(mUI.mAddonY2038, projectFile, dataDir, "y2038");
    updateAddonCheckBox(mUI.mAddonCert, projectFile, dataDir, "cert");

    mUI.mAddonY2038->setChecked(projectFile->getAddons().contains("y2038"));
    mUI.mAddonCert->setChecked(projectFile->getAddons().contains("cert"));
    mUI.mToolClangAnalyzer->setChecked(projectFile->getClangAnalyzer());
    mUI.mToolClangTidy->setChecked(projectFile->getClangTidy());
    if (CheckThread::clangTidyCmd().isEmpty()) {
        mUI.mToolClangTidy->setText(tr("Clang-tidy (not found)"));
        mUI.mToolClangTidy->setEnabled(false);
    }
    QString tags;
    foreach (const QString tag, projectFile->getTags()) {
        if (tags.isEmpty())
            tags = tag;
        else
            tags += ';' + tag;
    }
    mUI.mEditTags->setText(tags);
    updatePathsAndDefines();
}

void ProjectFileDialog::saveToProjectFile(ProjectFile *projectFile) const
{
    projectFile->setRootPath(getRootPath());
    projectFile->setBuildDir(getBuildDir());
    projectFile->setImportProject(getImportProject());
    projectFile->setAnalyzeAllVsConfigs(mUI.mChkAllVsConfigs->isChecked());
    projectFile->setIncludes(getIncludePaths());
    projectFile->setDefines(getDefines());
    projectFile->setCheckPaths(getCheckPaths());
    projectFile->setExcludedPaths(getExcludedPaths());
    projectFile->setLibraries(getLibraries());
    projectFile->setSuppressions(getSuppressions());
    QStringList list;
    if (mUI.mAddonThreadSafety->isChecked())
        list << "threadsafety";
    if (mUI.mAddonY2038->isChecked())
        list << "y2038";
    if (mUI.mAddonCert->isChecked())
        list << "cert";
    projectFile->setAddons(list);
    projectFile->setClangAnalyzer(mUI.mToolClangAnalyzer->isChecked());
    projectFile->setClangTidy(mUI.mToolClangTidy->isChecked());
    QStringList tags(mUI.mEditTags->text().split(";"));
    tags.removeAll(QString());
    projectFile->setTags(tags);
}

void ProjectFileDialog::ok()
{
    saveToProjectFile(mProjectFile);
    mProjectFile->write();
    accept();
}

QString ProjectFileDialog::getExistingDirectory(const QString &caption, bool trailingSlash)
{
    const QFileInfo inf(mProjectFile->getFilename());
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

void ProjectFileDialog::browseBuildDir()
{
    const QString dir(getExistingDirectory(tr("Select Cppcheck build dir"), false));
    if (!dir.isEmpty())
        mUI.mEditBuildDir->setText(dir);
}

void ProjectFileDialog::updatePathsAndDefines()
{
    const QString &fileName = mUI.mEditImportProject->text();
    bool importProject = !fileName.isEmpty();
    mUI.mBtnClearImportProject->setEnabled(importProject);
    mUI.mListCheckPaths->setEnabled(!importProject);
    mUI.mListIncludeDirs->setEnabled(!importProject);
    mUI.mBtnAddCheckPath->setEnabled(!importProject);
    mUI.mBtnEditCheckPath->setEnabled(!importProject);
    mUI.mBtnRemoveCheckPath->setEnabled(!importProject);
    mUI.mEditDefines->setEnabled(!importProject);
    mUI.mBtnAddInclude->setEnabled(!importProject);
    mUI.mBtnEditInclude->setEnabled(!importProject);
    mUI.mBtnRemoveInclude->setEnabled(!importProject);
    mUI.mBtnIncludeUp->setEnabled(!importProject);
    mUI.mBtnIncludeDown->setEnabled(!importProject);
    mUI.mChkAllVsConfigs->setEnabled(fileName.endsWith(".sln") || fileName.endsWith(".vcxproj"));
}

void ProjectFileDialog::clearImportProject()
{
    mUI.mEditImportProject->clear();
    updatePathsAndDefines();
}

void ProjectFileDialog::browseImportProject()
{
    const QFileInfo inf(mProjectFile->getFilename());
    const QDir &dir = inf.absoluteDir();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Project"),
                       dir.canonicalPath(),
                       tr("Visual Studio (*.sln *.vcxproj);;Compile database (compile_commands.json)"));
    if (!fileName.isEmpty()) {
        mUI.mEditImportProject->setText(dir.relativeFilePath(fileName));
        updatePathsAndDefines();
    }
}

QString ProjectFileDialog::getImportProject() const
{
    return mUI.mEditImportProject->text();
}

void ProjectFileDialog::addIncludeDir(const QString &dir)
{
    if (dir.isNull() || dir.isEmpty())
        return;

    const QString newdir = QDir::toNativeSeparators(dir);
    QListWidgetItem *item = new QListWidgetItem(newdir);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListIncludeDirs->addItem(item);
}

void ProjectFileDialog::addCheckPath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    QListWidgetItem *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListCheckPaths->addItem(item);
}

void ProjectFileDialog::addExcludePath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    QListWidgetItem *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListExcludedPaths->addItem(item);
}

QString ProjectFileDialog::getRootPath() const
{
    QString root = mUI.mEditProjectRoot->text();
    root = root.trimmed();
    root = QDir::fromNativeSeparators(root);
    return root;
}

QString ProjectFileDialog::getBuildDir() const
{
    return mUI.mEditBuildDir->text();
}


QStringList ProjectFileDialog::getIncludePaths() const
{
    const int count = mUI.mListIncludeDirs->count();
    QStringList includePaths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListIncludeDirs->item(i);
        includePaths << QDir::fromNativeSeparators(item->text());
    }
    return includePaths;
}

QStringList ProjectFileDialog::getDefines() const
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

QStringList ProjectFileDialog::getCheckPaths() const
{
    const int count = mUI.mListCheckPaths->count();
    QStringList paths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListCheckPaths->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

QStringList ProjectFileDialog::getExcludedPaths() const
{
    const int count = mUI.mListExcludedPaths->count();
    QStringList paths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListExcludedPaths->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

QStringList ProjectFileDialog::getLibraries() const
{
    QStringList libraries;
    foreach (const QCheckBox *checkbox, mLibraryCheckboxes) {
        if (checkbox->isChecked())
            libraries << checkbox->text();
    }
    return libraries;
}

QStringList ProjectFileDialog::getSuppressions() const
{
    QStringList suppressions;
    const int count = mUI.mListSuppressions->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListSuppressions->item(i);
        suppressions << item->text();
    }
    return suppressions;
}

void ProjectFileDialog::setRootPath(const QString &root)
{
    mUI.mEditProjectRoot->setText(QDir::toNativeSeparators(root));
}

void ProjectFileDialog::setBuildDir(const QString &buildDir)
{
    mUI.mEditBuildDir->setText(buildDir);
}

void ProjectFileDialog::setImportProject(const QString &importProject)
{
    mUI.mEditImportProject->setText(importProject);
}

void ProjectFileDialog::setIncludepaths(const QStringList &includes)
{
    foreach (QString dir, includes) {
        addIncludeDir(dir);
    }
}

void ProjectFileDialog::setDefines(const QStringList &defines)
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

void ProjectFileDialog::setCheckPaths(const QStringList &paths)
{
    foreach (QString path, paths) {
        addCheckPath(path);
    }
}

void ProjectFileDialog::setExcludedPaths(const QStringList &paths)
{
    foreach (QString path, paths) {
        addExcludePath(path);
    }
}

void ProjectFileDialog::setLibraries(const QStringList &libraries)
{
    for (int i = 0; i < mLibraryCheckboxes.size(); i++) {
        QCheckBox *checkbox = mLibraryCheckboxes[i];
        checkbox->setChecked(libraries.contains(checkbox->text()));
    }
}

void ProjectFileDialog::setSuppressions(const QStringList &suppressions)
{
    mUI.mListSuppressions->clear();
    mUI.mListSuppressions->addItems(suppressions);
    mUI.mListSuppressions->sortItems();
}

void ProjectFileDialog::addCheckPath()
{
    QString dir = getExistingDirectory(tr("Select a directory to check"), false);
    if (!dir.isEmpty())
        addCheckPath(dir);
}

void ProjectFileDialog::editCheckPath()
{
    QListWidgetItem *item = mUI.mListCheckPaths->currentItem();
    mUI.mListCheckPaths->editItem(item);
}

void ProjectFileDialog::removeCheckPath()
{
    const int row = mUI.mListCheckPaths->currentRow();
    QListWidgetItem *item = mUI.mListCheckPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::addIncludeDir()
{
    const QString dir = getExistingDirectory(tr("Select include directory"), true);
    if (!dir.isEmpty())
        addIncludeDir(dir);
}

void ProjectFileDialog::removeIncludeDir()
{
    const int row = mUI.mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI.mListIncludeDirs->takeItem(row);
    delete item;
}

void ProjectFileDialog::editIncludeDir()
{
    QListWidgetItem *item = mUI.mListIncludeDirs->currentItem();
    mUI.mListIncludeDirs->editItem(item);
}

void ProjectFileDialog::addExcludePath()
{
    QString dir = getExistingDirectory(tr("Select directory to ignore"), true);
    if (!dir.isEmpty())
        addExcludePath(dir);
}

void ProjectFileDialog::editExcludePath()
{
    QListWidgetItem *item = mUI.mListExcludedPaths->currentItem();
    mUI.mListExcludedPaths->editItem(item);
}

void ProjectFileDialog::removeExcludePath()
{
    const int row = mUI.mListExcludedPaths->currentRow();
    QListWidgetItem *item = mUI.mListExcludedPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::moveIncludePathUp()
{
    int row = mUI.mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI.mListIncludeDirs->takeItem(row);
    row = row > 0 ? row - 1 : 0;
    mUI.mListIncludeDirs->insertItem(row, item);
    mUI.mListIncludeDirs->setCurrentItem(item);
}

void ProjectFileDialog::moveIncludePathDown()
{
    int row = mUI.mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI.mListIncludeDirs->takeItem(row);
    const int count = mUI.mListIncludeDirs->count();
    row = row < count ? row + 1 : count;
    mUI.mListIncludeDirs->insertItem(row, item);
    mUI.mListIncludeDirs->setCurrentItem(item);
}

void ProjectFileDialog::addSuppression()
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

void ProjectFileDialog::removeSuppression()
{
    const int row = mUI.mListSuppressions->currentRow();
    QListWidgetItem *item = mUI.mListSuppressions->takeItem(row);
    delete item;
}

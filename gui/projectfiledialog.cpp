/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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
#include "newsuppressiondialog.h"
#include "projectfiledialog.h"
#include "checkthread.h"
#include "projectfile.h"
#include "library.h"
#include "platforms.h"

/** Return paths from QListWidget */
static QStringList getPaths(const QListWidget *list)
{
    const int count = list->count();
    QStringList paths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = list->item(i);
        paths << QDir::fromNativeSeparators(item->text());
    }
    return paths;
}

/** Platforms shown in the platform combobox */
static const cppcheck::Platform::PlatformType builtinPlatforms[] = {
    cppcheck::Platform::Native,
    cppcheck::Platform::Win32A,
    cppcheck::Platform::Win32W,
    cppcheck::Platform::Win64,
    cppcheck::Platform::Unix32,
    cppcheck::Platform::Unix64
};

static const int numberOfBuiltinPlatforms = sizeof(builtinPlatforms) / sizeof(builtinPlatforms[0]);

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

    // Platforms..
    Platforms p;
    for (int i = 0; i < numberOfBuiltinPlatforms; i++)
        mUI.mComboBoxPlatform->addItem(p.get(builtinPlatforms[i]).mTitle);
    QStringList platformFiles;
    foreach (QString sp, searchPaths) {
        if (sp.endsWith("/cfg"))
            sp = sp.mid(0,sp.length()-3) + "platforms";
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.xml"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        foreach (QFileInfo item, dir.entryInfoList()) {
            const QString platformFile = item.fileName();

            cppcheck::Platform p;
            if (!p.loadPlatformFile(applicationFilePath.toStdString().c_str(), platformFile.toStdString()))
                continue;

            if (platformFiles.indexOf(platformFile) == -1)
                platformFiles << platformFile;
        }
    }
    qSort(platformFiles);
    mUI.mComboBoxPlatform->addItems(platformFiles);

    mUI.mEditTags->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9 ;]*"),this));

    const QRegExp undefRegExp("\\s*([a-zA-Z_][a-zA-Z0-9_]*[; ]*)*");
    mUI.mEditUndefines->setValidator(new QRegExpValidator(undefRegExp, this));

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
    connect(mUI.mListSuppressions, &QListWidget::doubleClicked, this, &ProjectFileDialog::editSuppression);
    connect(mUI.mBtnBrowseMisraFile, &QPushButton::clicked, this, &ProjectFileDialog::browseMisraFile);

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
    if (projectFile)
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
    setUndefines(projectFile->getUndefines());
    setCheckPaths(projectFile->getCheckPaths());
    setImportProject(projectFile->getImportProject());
    mUI.mChkAllVsConfigs->setChecked(projectFile->getAnalyzeAllVsConfigs());
    setExcludedPaths(projectFile->getExcludedPaths());
    setLibraries(projectFile->getLibraries());
    const QString platform = projectFile->getPlatform();
    if (platform.endsWith(".xml")) {
        int i;
        for (i = numberOfBuiltinPlatforms; i < mUI.mComboBoxPlatform->count(); ++i) {
            if (mUI.mComboBoxPlatform->itemText(i) == platform)
                break;
        }
        if (i < mUI.mComboBoxPlatform->count())
            mUI.mComboBoxPlatform->setCurrentIndex(i);
        else {
            mUI.mComboBoxPlatform->addItem(platform);
            mUI.mComboBoxPlatform->setCurrentIndex(i);
        }
    } else {
        int i;
        for (i = 0; i < numberOfBuiltinPlatforms; ++i) {
            const cppcheck::Platform::PlatformType p = builtinPlatforms[i];
            if (platform == cppcheck::Platform::platformString(p))
                break;
        }
        if (i < numberOfBuiltinPlatforms)
            mUI.mComboBoxPlatform->setCurrentIndex(i);
        else
            mUI.mComboBoxPlatform->setCurrentIndex(-1);
    }

    mUI.mComboBoxPlatform->setCurrentText(projectFile->getPlatform());
    setSuppressions(projectFile->getSuppressions());

    QSettings settings;
    const QString dataDir = settings.value("DATADIR", QString()).toString();
    updateAddonCheckBox(mUI.mAddonThreadSafety, projectFile, dataDir, "threadsafety");
    updateAddonCheckBox(mUI.mAddonY2038, projectFile, dataDir, "y2038");
    updateAddonCheckBox(mUI.mAddonCert, projectFile, dataDir, "cert");
    updateAddonCheckBox(mUI.mAddonMisra, projectFile, dataDir, "misra");

    const QString &misraFile = settings.value(SETTINGS_MISRA_FILE, QString()).toString();
    mUI.mEditMisraFile->setText(misraFile);
    if (!mUI.mAddonMisra->isEnabled()) {
        mUI.mEditMisraFile->setEnabled(false);
        mUI.mBtnBrowseMisraFile->setEnabled(false);
    } else if (misraFile.isEmpty()) {
        mUI.mAddonMisra->setEnabled(false);
        mUI.mAddonMisra->setText(mUI.mAddonMisra->text() + ' ' + tr("(no rule texts file)"));
    }

    mUI.mToolClangAnalyzer->setChecked(projectFile->getClangAnalyzer());
    mUI.mToolClangTidy->setChecked(projectFile->getClangTidy());
    if (CheckThread::clangTidyCmd().isEmpty()) {
        mUI.mToolClangTidy->setText(tr("Clang-tidy (not found)"));
        mUI.mToolClangTidy->setEnabled(false);
    }
    mUI.mEditTags->setText(projectFile->getTags().join(';'));
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
    projectFile->setUndefines(getUndefines());
    projectFile->setCheckPaths(getCheckPaths());
    projectFile->setExcludedPaths(getExcludedPaths());
    projectFile->setLibraries(getLibraries());
    if (mUI.mComboBoxPlatform->currentText().endsWith(".xml"))
        projectFile->setPlatform(mUI.mComboBoxPlatform->currentText());
    else {
        int i = mUI.mComboBoxPlatform->currentIndex();
        if (i < numberOfBuiltinPlatforms)
            projectFile->setPlatform(cppcheck::Platform::platformString(builtinPlatforms[i]));
        else
            projectFile->setPlatform(QString());
    }
    projectFile->setSuppressions(getSuppressions());
    QStringList list;
    if (mUI.mAddonThreadSafety->isChecked())
        list << "threadsafety";
    if (mUI.mAddonY2038->isChecked())
        list << "y2038";
    if (mUI.mAddonCert->isChecked())
        list << "cert";
    if (mUI.mAddonMisra->isChecked())
        list << "misra";
    projectFile->setAddons(list);
    projectFile->setClangAnalyzer(mUI.mToolClangAnalyzer->isChecked());
    projectFile->setClangTidy(mUI.mToolClangTidy->isChecked());
    projectFile->setTags(mUI.mEditTags->text().split(";", QString::SkipEmptyParts));
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
    mUI.mEditUndefines->setEnabled(!importProject);
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
    QMap<QString,QString> filters;
    filters[tr("Visual Studio")] = "*.sln *.vcxproj";
    filters[tr("Compile database")] = "compile_commands.json";
    filters[tr("Borland C++ Builder 6")] = "*.bpr";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Project"),
                       dir.canonicalPath(),
                       toFilterString(filters));
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
    return getPaths(mUI.mListIncludeDirs);
}

QStringList ProjectFileDialog::getDefines() const
{
    return mUI.mEditDefines->text().trimmed().split(QRegExp("\\s*;\\s*"), QString::SkipEmptyParts);
}

QStringList ProjectFileDialog::getUndefines() const
{
    const QString undefine = mUI.mEditUndefines->text().trimmed();
    QStringList undefines = undefine.split(QRegExp("\\s*;\\s*"), QString::SkipEmptyParts);
    undefines.removeDuplicates();
    return undefines;
}

QStringList ProjectFileDialog::getCheckPaths() const
{
    return getPaths(mUI.mListCheckPaths);
}

QStringList ProjectFileDialog::getExcludedPaths() const
{
    return getPaths(mUI.mListExcludedPaths);
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
    mUI.mEditDefines->setText(defines.join(";"));
}

void ProjectFileDialog::setUndefines(const QStringList &undefines)
{
    mUI.mEditUndefines->setText(undefines.join(";"));
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

void ProjectFileDialog::setSuppressions(const QList<Suppressions::Suppression> &suppressions)
{
    mSuppressions = suppressions;

    QStringList s;
    foreach (const Suppressions::Suppression &suppression, mSuppressions) {
        s << QString::fromStdString(suppression.getText());
    }

    mUI.mListSuppressions->clear();
    mUI.mListSuppressions->addItems(s);
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
    NewSuppressionDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        setSuppressions(mSuppressions << dlg.getSuppression());
    }
}

void ProjectFileDialog::removeSuppression()
{
    const int row = mUI.mListSuppressions->currentRow();
    QListWidgetItem *item = mUI.mListSuppressions->takeItem(row);
    if (!item)
        return;

    int suppressionIndex = getSuppressionIndex(item->text());
    if (suppressionIndex >= 0)
        mSuppressions.removeAt(suppressionIndex);
    delete item;
}

void ProjectFileDialog::editSuppression(const QModelIndex &)
{
    const int row = mUI.mListSuppressions->currentRow();
    QListWidgetItem *item = mUI.mListSuppressions->item(row);
    int suppressionIndex = getSuppressionIndex(item->text());
    if (suppressionIndex >= 0) { // TODO what if suppression is not found?
        NewSuppressionDialog dlg;
        dlg.setSuppression(mSuppressions[suppressionIndex]);
        if (dlg.exec() == QDialog::Accepted) {
            mSuppressions[suppressionIndex] = dlg.getSuppression();
            setSuppressions(mSuppressions);
        }
    }
}

int ProjectFileDialog::getSuppressionIndex(const QString &shortText) const
{
    const std::string s = shortText.toStdString();
    for (int i = 0; i < mSuppressions.size(); ++i) {
        if (mSuppressions[i].getText() == s)
            return i;
    }
    return -1;
}

void ProjectFileDialog::browseMisraFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Select MISRA rule texts file"), QDir::homePath(), tr("Misra rule texts file (%1)").arg("*.txt"));
    if (!fileName.isEmpty()) {
        QSettings settings;
        mUI.mEditMisraFile->setText(fileName);
        settings.setValue(SETTINGS_MISRA_FILE, fileName);

        mUI.mAddonMisra->setText("MISRA C 2012");
        mUI.mAddonMisra->setEnabled(true);
        updateAddonCheckBox(mUI.mAddonMisra, nullptr, settings.value("DATADIR", QString()).toString(), "misra");
    }
}

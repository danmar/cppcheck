/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "projectfiledialog.h"

#include "checkthread.h"
#include "common.h"
#include "importproject.h"
#include "library.h"
#include "newsuppressiondialog.h"
#include "platform.h"
#include "platforms.h"
#include "projectfile.h"
#include "settings.h"

#include "ui_projectfile.h"

#include <list>
#include <string>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSettings>

class QModelIndex;

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

QStringList ProjectFileDialog::getProjectConfigs(const QString &fileName)
{
    if (!fileName.endsWith(".sln") && !fileName.endsWith(".vcxproj"))
        return QStringList();
    QStringList ret;
    ImportProject importer;
    Settings projSettings;
    importer.import(fileName.toStdString(), &projSettings);
    for (const std::string &cfg : importer.getVSConfigs())
        ret << QString::fromStdString(cfg);
    return ret;
}

ProjectFileDialog::ProjectFileDialog(ProjectFile *projectFile, QWidget *parent)
    : QDialog(parent)
    , mUI(new Ui::ProjectFile)
    , mProjectFile(projectFile)
{
    mUI->setupUi(this);

    mUI->mToolClangAnalyzer->hide();

    const QFileInfo inf(projectFile->getFilename());
    QString filename = inf.fileName();
    QString title = tr("Project file: %1").arg(filename);
    setWindowTitle(title);
    loadSettings();

    // Checkboxes for the libraries..
    const QString applicationFilePath = QCoreApplication::applicationFilePath();
    const QString appPath = QFileInfo(applicationFilePath).canonicalPath();
    const QString datadir = getDataDir();
    QStringList searchPaths;
    searchPaths << appPath << appPath + "/cfg" << inf.canonicalPath();
#ifdef FILESDIR
    if (FILESDIR[0])
        searchPaths << FILESDIR << FILESDIR "/cfg";
#endif
    if (!datadir.isEmpty())
        searchPaths << datadir << datadir + "/cfg";
    QStringList libs;
    // Search the std.cfg first since other libraries could depend on it
    QString stdLibraryFilename;
    for (const QString &sp : searchPaths) {
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.cfg"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo& item : dir.entryInfoList()) {
            QString library = item.fileName();
            if (library.compare("std.cfg", Qt::CaseInsensitive) != 0)
                continue;
            Library lib;
            const QString fullfilename = sp + "/" + library;
            const Library::Error err = lib.load(nullptr, fullfilename.toLatin1());
            if (err.errorcode != Library::ErrorCode::OK)
                continue;
            // Working std.cfg found
            stdLibraryFilename = fullfilename;
            break;
        }
        if (!stdLibraryFilename.isEmpty())
            break;
    }
    // Search other libraries
    for (const QString &sp : searchPaths) {
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.cfg"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo& item : dir.entryInfoList()) {
            QString library = item.fileName();
            {
                Library lib;
                const QString fullfilename = sp + "/" + library;
                Library::Error err = lib.load(nullptr, fullfilename.toLatin1());
                if (err.errorcode != Library::ErrorCode::OK) {
                    // Some libraries depend on std.cfg so load it first and test again
                    lib.load(nullptr, stdLibraryFilename.toLatin1());
                    err = lib.load(nullptr, fullfilename.toLatin1());
                }
                if (err.errorcode != Library::ErrorCode::OK)
                    continue;
            }
            library.chop(4);
            if (library.compare("std", Qt::CaseInsensitive) == 0)
                continue;
            if (libs.indexOf(library) == -1)
                libs << library;
        }
    }
    libs.sort();
    mUI->mLibraries->clear();
    for (const QString &lib : libs) {
        QListWidgetItem* item = new QListWidgetItem(lib, mUI->mLibraries);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Unchecked); // AND initialize check state
    }

    // Platforms..
    Platforms platforms;
    for (cppcheck::Platform::PlatformType builtinPlatform : builtinPlatforms)
        mUI->mComboBoxPlatform->addItem(platforms.get(builtinPlatform).mTitle);
    QStringList platformFiles;
    for (QString sp : searchPaths) {
        if (sp.endsWith("/cfg"))
            sp = sp.mid(0,sp.length()-3) + "platforms";
        QDir dir(sp);
        dir.setSorting(QDir::Name);
        dir.setNameFilters(QStringList("*.xml"));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo& item : dir.entryInfoList()) {
            const QString platformFile = item.fileName();

            cppcheck::Platform plat2;
            if (!plat2.loadPlatformFile(applicationFilePath.toStdString().c_str(), platformFile.toStdString()))
                continue;

            if (platformFiles.indexOf(platformFile) == -1)
                platformFiles << platformFile;
        }
    }
    platformFiles.sort();
    mUI->mComboBoxPlatform->addItems(platformFiles);

    mUI->mEditTags->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9 ;]*"),this));

    const QRegularExpression undefRegExp("\\s*([a-zA-Z_][a-zA-Z0-9_]*[; ]*)*");
    mUI->mEditUndefines->setValidator(new QRegularExpressionValidator(undefRegExp, this));

    connect(mUI->mButtons, &QDialogButtonBox::accepted, this, &ProjectFileDialog::ok);
    connect(mUI->mBtnBrowseBuildDir, &QPushButton::clicked, this, &ProjectFileDialog::browseBuildDir);
    connect(mUI->mBtnClearImportProject, &QPushButton::clicked, this, &ProjectFileDialog::clearImportProject);
    connect(mUI->mBtnBrowseImportProject, &QPushButton::clicked, this, &ProjectFileDialog::browseImportProject);
    connect(mUI->mBtnAddCheckPath, SIGNAL(clicked()), this, SLOT(addCheckPath()));
    connect(mUI->mBtnEditCheckPath, &QPushButton::clicked, this, &ProjectFileDialog::editCheckPath);
    connect(mUI->mBtnRemoveCheckPath, &QPushButton::clicked, this, &ProjectFileDialog::removeCheckPath);
    connect(mUI->mBtnAddInclude, SIGNAL(clicked()), this, SLOT(addIncludeDir()));
    connect(mUI->mBtnEditInclude, &QPushButton::clicked, this, &ProjectFileDialog::editIncludeDir);
    connect(mUI->mBtnRemoveInclude, &QPushButton::clicked, this, &ProjectFileDialog::removeIncludeDir);
    connect(mUI->mBtnAddIgnorePath, SIGNAL(clicked()), this, SLOT(addExcludePath()));
    connect(mUI->mBtnAddIgnoreFile, SIGNAL(clicked()), this, SLOT(addExcludeFile()));
    connect(mUI->mBtnEditIgnorePath, &QPushButton::clicked, this, &ProjectFileDialog::editExcludePath);
    connect(mUI->mBtnRemoveIgnorePath, &QPushButton::clicked, this, &ProjectFileDialog::removeExcludePath);
    connect(mUI->mBtnIncludeUp, &QPushButton::clicked, this, &ProjectFileDialog::moveIncludePathUp);
    connect(mUI->mBtnIncludeDown, &QPushButton::clicked, this, &ProjectFileDialog::moveIncludePathDown);
    connect(mUI->mBtnAddSuppression, &QPushButton::clicked, this, &ProjectFileDialog::addSuppression);
    connect(mUI->mBtnRemoveSuppression, &QPushButton::clicked, this, &ProjectFileDialog::removeSuppression);
    connect(mUI->mListSuppressions, &QListWidget::doubleClicked, this, &ProjectFileDialog::editSuppression);
    connect(mUI->mBtnBrowseMisraFile, &QPushButton::clicked, this, &ProjectFileDialog::browseMisraFile);
    connect(mUI->mChkAllVsConfigs, &QCheckBox::clicked, this, &ProjectFileDialog::checkAllVSConfigs);
    loadFromProjectFile(projectFile);
}

ProjectFileDialog::~ProjectFileDialog()
{
    saveSettings();
    delete mUI;
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
    if (ProjectFile::getAddonFilePath(dataDir, addon).isEmpty()) {
        cb->setEnabled(false);
        cb->setText(cb->text() + QObject::tr(" (Not found)"));
    }
}

void ProjectFileDialog::checkAllVSConfigs()
{
    if (mUI->mChkAllVsConfigs->isChecked()) {
        for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
            QListWidgetItem *item = mUI->mListVsConfigs->item(row);
            item->setCheckState(Qt::Checked);
        }
    }
    mUI->mListVsConfigs->setEnabled(!mUI->mChkAllVsConfigs->isChecked());
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
    mUI->mChkAllVsConfigs->setChecked(projectFile->getAnalyzeAllVsConfigs());
    setProjectConfigurations(getProjectConfigs(mUI->mEditImportProject->text()));
    for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
        QListWidgetItem *item = mUI->mListVsConfigs->item(row);
        if (projectFile->getAnalyzeAllVsConfigs() || projectFile->getVsConfigurations().contains(item->text()))
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
    }
    mUI->mCheckHeaders->setChecked(projectFile->getCheckHeaders());
    mUI->mCheckUnusedTemplates->setChecked(projectFile->getCheckUnusedTemplates());
    mUI->mMaxCtuDepth->setValue(projectFile->getMaxCtuDepth());
    mUI->mMaxTemplateRecursion->setValue(projectFile->getMaxTemplateRecursion());
    if (projectFile->clangParser)
        mUI->mBtnClangParser->setChecked(true);
    else
        mUI->mBtnCppcheckParser->setChecked(true);
    mUI->mBtnSafeClasses->setChecked(projectFile->safeChecks.classes);
    setExcludedPaths(projectFile->getExcludedPaths());
    setLibraries(projectFile->getLibraries());
    const QString platform = projectFile->getPlatform();
    if (platform.endsWith(".xml")) {
        int i;
        for (i = numberOfBuiltinPlatforms; i < mUI->mComboBoxPlatform->count(); ++i) {
            if (mUI->mComboBoxPlatform->itemText(i) == platform)
                break;
        }
        if (i < mUI->mComboBoxPlatform->count())
            mUI->mComboBoxPlatform->setCurrentIndex(i);
        else {
            mUI->mComboBoxPlatform->addItem(platform);
            mUI->mComboBoxPlatform->setCurrentIndex(i);
        }
    } else {
        int i;
        for (i = 0; i < numberOfBuiltinPlatforms; ++i) {
            const cppcheck::Platform::PlatformType p = builtinPlatforms[i];
            if (platform == cppcheck::Platform::platformString(p))
                break;
        }
        if (i < numberOfBuiltinPlatforms)
            mUI->mComboBoxPlatform->setCurrentIndex(i);
        else
            mUI->mComboBoxPlatform->setCurrentIndex(-1);
    }

    mUI->mComboBoxPlatform->setCurrentText(projectFile->getPlatform());
    setSuppressions(projectFile->getSuppressions());

    // Human knowledge..
    /*
       mUI->mListUnknownFunctionReturn->clear();
       mUI->mListUnknownFunctionReturn->addItem("rand()");
       for (int row = 0; row < mUI->mListUnknownFunctionReturn->count(); ++row) {
        QListWidgetItem *item = mUI->mListUnknownFunctionReturn->item(row);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        const bool unknownValues = projectFile->getCheckUnknownFunctionReturn().contains(item->text());
        item->setCheckState(unknownValues ? Qt::Checked : Qt::Unchecked); // AND initialize check state
       }
       mUI->mCheckSafeClasses->setChecked(projectFile->getSafeChecks().classes);
       mUI->mCheckSafeExternalFunctions->setChecked(projectFile->getSafeChecks().externalFunctions);
       mUI->mCheckSafeInternalFunctions->setChecked(projectFile->getSafeChecks().internalFunctions);
       mUI->mCheckSafeExternalVariables->setChecked(projectFile->getSafeChecks().externalVariables);
     */

    // Addons..
    QSettings settings;
    const QString dataDir = getDataDir();
    updateAddonCheckBox(mUI->mAddonThreadSafety, projectFile, dataDir, "threadsafety");
    updateAddonCheckBox(mUI->mAddonY2038, projectFile, dataDir, "y2038");
    updateAddonCheckBox(mUI->mAddonMisra, projectFile, dataDir, "misra");

    const QString &misraFile = settings.value(SETTINGS_MISRA_FILE, QString()).toString();
    mUI->mEditMisraFile->setText(misraFile);
    if (!mUI->mAddonMisra->isEnabled()) {
        mUI->mEditMisraFile->setEnabled(false);
        mUI->mBtnBrowseMisraFile->setEnabled(false);
    }

    mUI->mToolClangAnalyzer->setChecked(projectFile->getClangAnalyzer());
    mUI->mToolClangTidy->setChecked(projectFile->getClangTidy());
    if (CheckThread::clangTidyCmd().isEmpty()) {
        mUI->mToolClangTidy->setText(tr("Clang-tidy (not found)"));
        mUI->mToolClangTidy->setEnabled(false);
    }
    mUI->mEditTags->setText(projectFile->getTags().join(';'));
    updatePathsAndDefines();
}

void ProjectFileDialog::saveToProjectFile(ProjectFile *projectFile) const
{
    projectFile->setRootPath(getRootPath());
    projectFile->setBuildDir(getBuildDir());
    projectFile->setImportProject(getImportProject());
    projectFile->setAnalyzeAllVsConfigs(mUI->mChkAllVsConfigs->isChecked());
    projectFile->setVSConfigurations(getProjectConfigurations());
    projectFile->setCheckHeaders(mUI->mCheckHeaders->isChecked());
    projectFile->setCheckUnusedTemplates(mUI->mCheckUnusedTemplates->isChecked());
    projectFile->setMaxCtuDepth(mUI->mMaxCtuDepth->value());
    projectFile->setMaxTemplateRecursion(mUI->mMaxTemplateRecursion->value());
    projectFile->setIncludes(getIncludePaths());
    projectFile->setDefines(getDefines());
    projectFile->setUndefines(getUndefines());
    projectFile->setCheckPaths(getCheckPaths());
    projectFile->setExcludedPaths(getExcludedPaths());
    projectFile->setLibraries(getLibraries());
    projectFile->clangParser = mUI->mBtnClangParser->isChecked();
    projectFile->safeChecks.classes = mUI->mBtnSafeClasses->isChecked();
    if (mUI->mComboBoxPlatform->currentText().endsWith(".xml"))
        projectFile->setPlatform(mUI->mComboBoxPlatform->currentText());
    else {
        int i = mUI->mComboBoxPlatform->currentIndex();
        if (i < numberOfBuiltinPlatforms)
            projectFile->setPlatform(cppcheck::Platform::platformString(builtinPlatforms[i]));
        else
            projectFile->setPlatform(QString());
    }
    projectFile->setSuppressions(getSuppressions());
    // Human knowledge
    /*
       QStringList unknownReturnValues;
       for (int row = 0; row < mUI->mListUnknownFunctionReturn->count(); ++row) {
        QListWidgetItem *item = mUI->mListUnknownFunctionReturn->item(row);
        if (item->checkState() == Qt::Checked)
            unknownReturnValues << item->text();
       }
       projectFile->setCheckUnknownFunctionReturn(unknownReturnValues);
       ProjectFile::SafeChecks safeChecks;
       safeChecks.classes = mUI->mCheckSafeClasses->isChecked();
       safeChecks.externalFunctions = mUI->mCheckSafeExternalFunctions->isChecked();
       safeChecks.internalFunctions = mUI->mCheckSafeInternalFunctions->isChecked();
       safeChecks.externalVariables = mUI->mCheckSafeExternalVariables->isChecked();
       projectFile->setSafeChecks(safeChecks);
     */
    // Addons
    QStringList list;
    if (mUI->mAddonThreadSafety->isChecked())
        list << "threadsafety";
    if (mUI->mAddonY2038->isChecked())
        list << "y2038";
    if (mUI->mAddonMisra->isChecked())
        list << "misra";
    projectFile->setAddons(list);
    projectFile->setClangAnalyzer(mUI->mToolClangAnalyzer->isChecked());
    projectFile->setClangTidy(mUI->mToolClangTidy->isChecked());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    projectFile->setTags(mUI->mEditTags->text().split(";", Qt::SkipEmptyParts));
#else
    projectFile->setTags(mUI->mEditTags->text().split(";", QString::SkipEmptyParts));
#endif
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
    if (!relpath.startsWith("../.."))
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
        mUI->mEditBuildDir->setText(dir);
}

void ProjectFileDialog::updatePathsAndDefines()
{
    const QString &fileName = mUI->mEditImportProject->text();
    bool importProject = !fileName.isEmpty();
    bool hasConfigs = fileName.endsWith(".sln") || fileName.endsWith(".vcxproj");
    mUI->mBtnClearImportProject->setEnabled(importProject);
    mUI->mListCheckPaths->setEnabled(!importProject);
    mUI->mListIncludeDirs->setEnabled(!importProject);
    mUI->mBtnAddCheckPath->setEnabled(!importProject);
    mUI->mBtnEditCheckPath->setEnabled(!importProject);
    mUI->mBtnRemoveCheckPath->setEnabled(!importProject);
    mUI->mEditDefines->setEnabled(!importProject);
    mUI->mEditUndefines->setEnabled(!importProject);
    mUI->mBtnAddInclude->setEnabled(!importProject);
    mUI->mBtnEditInclude->setEnabled(!importProject);
    mUI->mBtnRemoveInclude->setEnabled(!importProject);
    mUI->mBtnIncludeUp->setEnabled(!importProject);
    mUI->mBtnIncludeDown->setEnabled(!importProject);
    mUI->mChkAllVsConfigs->setEnabled(hasConfigs);
    mUI->mListVsConfigs->setEnabled(hasConfigs && !mUI->mChkAllVsConfigs->isChecked());
    if (!hasConfigs)
        mUI->mListVsConfigs->clear();
}

void ProjectFileDialog::clearImportProject()
{
    mUI->mEditImportProject->clear();
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
        mUI->mEditImportProject->setText(dir.relativeFilePath(fileName));
        updatePathsAndDefines();
        setProjectConfigurations(getProjectConfigs(fileName));
        for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
            QListWidgetItem *item = mUI->mListVsConfigs->item(row);
            item->setCheckState(Qt::Checked);
        }
    }
}

QStringList ProjectFileDialog::getProjectConfigurations() const
{
    QStringList configs;
    for (int row = 0; row < mUI->mListVsConfigs->count(); ++row) {
        QListWidgetItem *item = mUI->mListVsConfigs->item(row);
        if (item->checkState() == Qt::Checked)
            configs << item->text();
    }
    return configs;
}

void ProjectFileDialog::setProjectConfigurations(const QStringList &configs)
{
    mUI->mListVsConfigs->clear();
    mUI->mListVsConfigs->setEnabled(!configs.isEmpty() && !mUI->mChkAllVsConfigs->isChecked());
    for (const QString &cfg : configs) {
        QListWidgetItem* item = new QListWidgetItem(cfg, mUI->mListVsConfigs);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Unchecked);
    }
}

QString ProjectFileDialog::getImportProject() const
{
    return mUI->mEditImportProject->text();
}

void ProjectFileDialog::addIncludeDir(const QString &dir)
{
    if (dir.isNull() || dir.isEmpty())
        return;

    const QString newdir = QDir::toNativeSeparators(dir);
    QListWidgetItem *item = new QListWidgetItem(newdir);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI->mListIncludeDirs->addItem(item);
}

void ProjectFileDialog::addCheckPath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    QListWidgetItem *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI->mListCheckPaths->addItem(item);
}

void ProjectFileDialog::addExcludePath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    const QString newpath = QDir::toNativeSeparators(path);
    QListWidgetItem *item = new QListWidgetItem(newpath);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI->mListExcludedPaths->addItem(item);
}

QString ProjectFileDialog::getRootPath() const
{
    QString root = mUI->mEditProjectRoot->text();
    root = root.trimmed();
    root = QDir::fromNativeSeparators(root);
    return root;
}

QString ProjectFileDialog::getBuildDir() const
{
    return mUI->mEditBuildDir->text();
}

QStringList ProjectFileDialog::getIncludePaths() const
{
    return getPaths(mUI->mListIncludeDirs);
}

QStringList ProjectFileDialog::getDefines() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    return mUI->mEditDefines->text().trimmed().split(QRegularExpression("\\s*;\\s*"), Qt::SkipEmptyParts);
#else
    return mUI->mEditDefines->text().trimmed().split(QRegularExpression("\\s*;\\s*"), QString::SkipEmptyParts);
#endif
}

QStringList ProjectFileDialog::getUndefines() const
{
    const QString undefine = mUI->mEditUndefines->text().trimmed();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList undefines = undefine.split(QRegularExpression("\\s*;\\s*"), Qt::SkipEmptyParts);
#else
    QStringList undefines = undefine.split(QRegularExpression("\\s*;\\s*"), QString::SkipEmptyParts);
#endif
    undefines.removeDuplicates();
    return undefines;
}

QStringList ProjectFileDialog::getCheckPaths() const
{
    return getPaths(mUI->mListCheckPaths);
}

QStringList ProjectFileDialog::getExcludedPaths() const
{
    return getPaths(mUI->mListExcludedPaths);
}

QStringList ProjectFileDialog::getLibraries() const
{
    QStringList libraries;
    for (int row = 0; row < mUI->mLibraries->count(); ++row) {
        QListWidgetItem *item = mUI->mLibraries->item(row);
        if (item->checkState() == Qt::Checked)
            libraries << item->text();
    }
    return libraries;
}

void ProjectFileDialog::setRootPath(const QString &root)
{
    mUI->mEditProjectRoot->setText(QDir::toNativeSeparators(root));
}

void ProjectFileDialog::setBuildDir(const QString &buildDir)
{
    mUI->mEditBuildDir->setText(buildDir);
}

void ProjectFileDialog::setImportProject(const QString &importProject)
{
    mUI->mEditImportProject->setText(importProject);
}

void ProjectFileDialog::setIncludepaths(const QStringList &includes)
{
    for (const QString& dir : includes) {
        addIncludeDir(dir);
    }
}

void ProjectFileDialog::setDefines(const QStringList &defines)
{
    mUI->mEditDefines->setText(defines.join(";"));
}

void ProjectFileDialog::setUndefines(const QStringList &undefines)
{
    mUI->mEditUndefines->setText(undefines.join(";"));
}

void ProjectFileDialog::setCheckPaths(const QStringList &paths)
{
    for (const QString& path : paths) {
        addCheckPath(path);
    }
}

void ProjectFileDialog::setExcludedPaths(const QStringList &paths)
{
    for (const QString& path : paths) {
        addExcludePath(path);
    }
}

void ProjectFileDialog::setLibraries(const QStringList &libraries)
{
    for (int row = 0; row < mUI->mLibraries->count(); ++row) {
        QListWidgetItem *item = mUI->mLibraries->item(row);
        item->setCheckState(libraries.contains(item->text()) ? Qt::Checked : Qt::Unchecked);
    }
}

void ProjectFileDialog::addSingleSuppression(const Suppressions::Suppression &suppression)
{
    QString suppression_name;
    static char sep = QDir::separator().toLatin1();
    bool found_relative = false;

    // Replace relative file path in the suppression with the absolute one
    if ((suppression.fileName.find("*") == std::string::npos) &&
        (suppression.fileName.find(sep) == std::string::npos)) {
        QFileInfo inf(mProjectFile->getFilename());
        QString rootpath = inf.absolutePath();
        if (QFile::exists(QString{"%1%2%3"}.arg(rootpath,
                                                QDir::separator(),
                                                QString::fromStdString(suppression.fileName)))) {
            Suppressions::Suppression sup = suppression;
            sup.fileName = rootpath.toLatin1().constData();
            sup.fileName += sep;
            sup.fileName += suppression.fileName;
            mSuppressions += sup;
            suppression_name = QString::fromStdString(sup.getText());
            found_relative = true;
        }
    }

    if (!found_relative) {
        mSuppressions += suppression;
        suppression_name = QString::fromStdString(suppression.getText());
    }

    mUI->mListSuppressions->addItem(suppression_name);
}

void ProjectFileDialog::setSuppressions(const QList<Suppressions::Suppression> &suppressions)
{
    mUI->mListSuppressions->clear();
    QList<Suppressions::Suppression> new_suppressions = suppressions;
    mSuppressions.clear();
    for (const Suppressions::Suppression &suppression : new_suppressions) {
        addSingleSuppression(suppression);
    }
    mUI->mListSuppressions->sortItems();
}

void ProjectFileDialog::addCheckPath()
{
    QString dir = getExistingDirectory(tr("Select a directory to check"), false);
    if (!dir.isEmpty())
        addCheckPath(dir);
}

void ProjectFileDialog::editCheckPath()
{
    QListWidgetItem *item = mUI->mListCheckPaths->currentItem();
    mUI->mListCheckPaths->editItem(item);
}

void ProjectFileDialog::removeCheckPath()
{
    const int row = mUI->mListCheckPaths->currentRow();
    QListWidgetItem *item = mUI->mListCheckPaths->takeItem(row);
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
    const int row = mUI->mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI->mListIncludeDirs->takeItem(row);
    delete item;
}

void ProjectFileDialog::editIncludeDir()
{
    QListWidgetItem *item = mUI->mListIncludeDirs->currentItem();
    mUI->mListIncludeDirs->editItem(item);
}

void ProjectFileDialog::addExcludePath()
{
    addExcludePath(getExistingDirectory(tr("Select directory to ignore"), true));
}

void ProjectFileDialog::addExcludeFile()
{
    const QFileInfo inf(mProjectFile->getFilename());
    const QDir &dir = inf.absoluteDir();
    QMap<QString,QString> filters;
    filters[tr("Source files")] = "*.c *.cpp";
    filters[tr("All files")] = "*.*";
    addExcludePath(QFileDialog::getOpenFileName(this, tr("Exclude file"), dir.canonicalPath(), toFilterString(filters)));
}

void ProjectFileDialog::editExcludePath()
{
    QListWidgetItem *item = mUI->mListExcludedPaths->currentItem();
    mUI->mListExcludedPaths->editItem(item);
}

void ProjectFileDialog::removeExcludePath()
{
    const int row = mUI->mListExcludedPaths->currentRow();
    QListWidgetItem *item = mUI->mListExcludedPaths->takeItem(row);
    delete item;
}

void ProjectFileDialog::moveIncludePathUp()
{
    int row = mUI->mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI->mListIncludeDirs->takeItem(row);
    row = row > 0 ? row - 1 : 0;
    mUI->mListIncludeDirs->insertItem(row, item);
    mUI->mListIncludeDirs->setCurrentItem(item);
}

void ProjectFileDialog::moveIncludePathDown()
{
    int row = mUI->mListIncludeDirs->currentRow();
    QListWidgetItem *item = mUI->mListIncludeDirs->takeItem(row);
    const int count = mUI->mListIncludeDirs->count();
    row = row < count ? row + 1 : count;
    mUI->mListIncludeDirs->insertItem(row, item);
    mUI->mListIncludeDirs->setCurrentItem(item);
}

void ProjectFileDialog::addSuppression()
{
    NewSuppressionDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        addSingleSuppression(dlg.getSuppression());
    }
}

void ProjectFileDialog::removeSuppression()
{
    const int row = mUI->mListSuppressions->currentRow();
    QListWidgetItem *item = mUI->mListSuppressions->takeItem(row);
    if (!item)
        return;

    int suppressionIndex = getSuppressionIndex(item->text());
    if (suppressionIndex >= 0)
        mSuppressions.removeAt(suppressionIndex);
    delete item;
}

void ProjectFileDialog::editSuppression(const QModelIndex &)
{
    const int row = mUI->mListSuppressions->currentRow();
    QListWidgetItem *item = mUI->mListSuppressions->item(row);
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
    const QString fileName = QFileDialog::getOpenFileName(this,
                                                          tr("Select MISRA rule texts file"),
                                                          QDir::homePath(),
                                                          tr("MISRA rule texts file (%1)").arg("*.txt"));
    if (!fileName.isEmpty()) {
        QSettings settings;
        mUI->mEditMisraFile->setText(fileName);
        settings.setValue(SETTINGS_MISRA_FILE, fileName);

        mUI->mAddonMisra->setText("MISRA C 2012");
        mUI->mAddonMisra->setEnabled(true);
        updateAddonCheckBox(mUI->mAddonMisra, nullptr, getDataDir(), "misra");
    }
}

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

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QAction>
#include <QActionGroup>
#include <QFile>
#include "mainwindow.h"
#include "cppcheck.h"
#include "applicationlist.h"
#include "aboutdialog.h"
#include "common.h"
#include "threadhandler.h"
#include "fileviewdialog.h"
#include "projectfile.h"
#include "project.h"
#include "report.h"
#include "scratchpad.h"
#include "statsdialog.h"
#include "settingsdialog.h"
#include "threadresult.h"
#include "translationhandler.h"
#include "logview.h"
#include "filelist.h"
#include "showtypes.h"

static const QString OnlineHelpURL("http://cppcheck.sourceforge.net/manual.html");

MainWindow::MainWindow(TranslationHandler* th, QSettings* settings) :
    mSettings(settings),
    mApplications(new ApplicationList(this)),
    mTranslation(th),
    mLogView(NULL),
    mScratchPad(NULL),
    mProject(NULL),
    mPlatformActions(new QActionGroup(this)),
    mCStandardActions(new QActionGroup(this)),
    mCppStandardActions(new QActionGroup(this)),
    mExiting(false)
{
    mUI.setupUi(this);
    mUI.mResults->Initialize(mSettings, mApplications);

    mThread = new ThreadHandler(this);

    // Filter timer to delay filtering results slightly while typing
    mFilterTimer = new QTimer(this);
    mFilterTimer->setInterval(500);
    mFilterTimer->setSingleShot(true);
    connect(mFilterTimer, SIGNAL(timeout()), this, SLOT(FilterResults()));

    // "Filter" toolbar
    mLineEditFilter = new QLineEdit(mUI.mToolBarFilter);
    mLineEditFilter->setPlaceholderText(tr("Quick Filter:"));
    mUI.mToolBarFilter->addWidget(mLineEditFilter);
    connect(mLineEditFilter, SIGNAL(textChanged(const QString&)), mFilterTimer, SLOT(start()));
    connect(mLineEditFilter, SIGNAL(returnPressed()), this, SLOT(FilterResults()));

    connect(mUI.mActionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(mUI.mActionCheckFiles, SIGNAL(triggered()), this, SLOT(CheckFiles()));
    connect(mUI.mActionCheckDirectory, SIGNAL(triggered()), this, SLOT(CheckDirectory()));
    connect(mUI.mActionSettings, SIGNAL(triggered()), this, SLOT(ProgramSettings()));
    connect(mUI.mActionClearResults, SIGNAL(triggered()), this, SLOT(ClearResults()));
    connect(mUI.mActionOpenXML, SIGNAL(triggered()), this, SLOT(OpenResults()));

    connect(mUI.mActionShowStyle, SIGNAL(toggled(bool)), this, SLOT(ShowStyle(bool)));
    connect(mUI.mActionShowErrors, SIGNAL(toggled(bool)), this, SLOT(ShowErrors(bool)));
    connect(mUI.mActionShowWarnings, SIGNAL(toggled(bool)), this, SLOT(ShowWarnings(bool)));
    connect(mUI.mActionShowPortability, SIGNAL(toggled(bool)), this, SLOT(ShowPortability(bool)));
    connect(mUI.mActionShowPerformance, SIGNAL(toggled(bool)), this, SLOT(ShowPerformance(bool)));
    connect(mUI.mActionShowInformation, SIGNAL(toggled(bool)), this, SLOT(ShowInformation(bool)));
    connect(mUI.mActionCheckAll, SIGNAL(triggered()), this, SLOT(CheckAll()));
    connect(mUI.mActionUncheckAll, SIGNAL(triggered()), this, SLOT(UncheckAll()));
    connect(mUI.mActionCollapseAll, SIGNAL(triggered()), mUI.mResults, SLOT(CollapseAllResults()));
    connect(mUI.mActionExpandAll, SIGNAL(triggered()), mUI.mResults, SLOT(ExpandAllResults()));
    connect(mUI.mActionShowHidden, SIGNAL(triggered()), mUI.mResults, SLOT(ShowHiddenResults()));
    connect(mUI.mActionViewLog, SIGNAL(triggered()), this, SLOT(ShowLogView()));
    connect(mUI.mActionViewStats, SIGNAL(triggered()), this, SLOT(ShowStatistics()));

    connect(mUI.mActionRecheck, SIGNAL(triggered()), this, SLOT(ReCheck()));

    connect(mUI.mActionStop, SIGNAL(triggered()), this, SLOT(StopChecking()));
    connect(mUI.mActionSave, SIGNAL(triggered()), this, SLOT(Save()));

    // About menu
    connect(mUI.mActionAbout, SIGNAL(triggered()), this, SLOT(About()));
    connect(mUI.mActionLicense, SIGNAL(triggered()), this, SLOT(ShowLicense()));

    // View > Toolbar menu
    connect(mUI.mActionToolBarMain, SIGNAL(toggled(bool)), this, SLOT(ToggleMainToolBar()));
    connect(mUI.mActionToolBarView, SIGNAL(toggled(bool)), this, SLOT(ToggleViewToolBar()));
    connect(mUI.mActionToolBarFilter, SIGNAL(toggled(bool)), this, SLOT(ToggleFilterToolBar()));

    connect(mUI.mActionAuthors, SIGNAL(triggered()), this, SLOT(ShowAuthors()));
    connect(mThread, SIGNAL(Done()), this, SLOT(CheckDone()));
    connect(mUI.mResults, SIGNAL(GotResults()), this, SLOT(ResultsAdded()));
    connect(mUI.mResults, SIGNAL(ResultsHidden(bool)), mUI.mActionShowHidden, SLOT(setEnabled(bool)));
    connect(mUI.mMenuView, SIGNAL(aboutToShow()), this, SLOT(AboutToShowViewMenu()));

    // File menu
    connect(mUI.mActionNewProjectFile, SIGNAL(triggered()), this, SLOT(NewProjectFile()));
    connect(mUI.mActionOpenProjectFile, SIGNAL(triggered()), this, SLOT(OpenProjectFile()));
    connect(mUI.mActionShowScratchpad, SIGNAL(triggered()), this, SLOT(ShowScratchpad()));
    connect(mUI.mActionCloseProjectFile, SIGNAL(triggered()), this, SLOT(CloseProjectFile()));
    connect(mUI.mActionEditProjectFile, SIGNAL(triggered()), this, SLOT(EditProjectFile()));

    connect(mUI.mActionHelpContents, SIGNAL(triggered()), this, SLOT(OpenHelpContents()));

    LoadSettings();

    mThread->Initialize(mUI.mResults);
    FormatAndSetTitle();

    EnableCheckButtons(true);

    mUI.mActionClearResults->setEnabled(false);
    mUI.mActionSave->setEnabled(false);
    mUI.mActionRecheck->setEnabled(false);
    EnableProjectOpenActions(true);
    EnableProjectActions(false);

    // Must setup MRU menu before CLI param handling as it can load a
    // project file and update MRU menu.
    for (int i = 0; i < MaxRecentProjects; ++i) {
        mRecentProjectActs[i] = new QAction(this);
        mRecentProjectActs[i]->setVisible(false);
        connect(mRecentProjectActs[i], SIGNAL(triggered()),
                this, SLOT(OpenRecentProject()));
    }
    mRecentProjectActs[MaxRecentProjects] = NULL; // The separator
    mUI.mActionProjectMRU->setVisible(false);
    UpdateMRUMenuItems();

    QStringList args = QCoreApplication::arguments();
    //Remove the application itself
    args.removeFirst();
    if (!args.isEmpty()) {
        HandleCLIParams(args);
    }

    for (int i = 0; i < mPlatforms.getCount(); i++) {
        Platform plat = mPlatforms.mPlatforms[i];
        QAction *act = new QAction(this);
        plat.mActMainWindow = act;
        mPlatforms.mPlatforms[i] = plat;
        act->setText(plat.mTitle);
        act->setData(plat.mType);
        act->setCheckable(true);
        act->setActionGroup(mPlatformActions);
        mUI.mMenuCheck->insertAction(mUI.mActionPlatforms, act);
        connect(act, SIGNAL(triggered()), this, SLOT(SelectPlatform()));
    }

    mUI.mActionC89->setActionGroup(mCStandardActions);
    mUI.mActionC99->setActionGroup(mCStandardActions);
    mUI.mActionC11->setActionGroup(mCStandardActions);

    mUI.mActionCpp03->setActionGroup(mCppStandardActions);
    mUI.mActionCpp11->setActionGroup(mCppStandardActions);

    // For Windows platforms default to Win32 checked platform.
    // For other platforms default to unspecified/default which means the
    // platform Cppcheck GUI was compiled on.
#if defined(_WIN32)
    const Settings::PlatformType defaultPlat = Settings::Win32W;
#else
    const Settings::PlatformType defaultPlat = Settings::Unspecified;
#endif
    Platform &plat = mPlatforms.get((Settings::PlatformType)mSettings->value(SETTINGS_CHECKED_PLATFORM, defaultPlat).toInt());
    plat.mActMainWindow->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete mLogView;
    delete mProject;
    delete mScratchPad;
}

void MainWindow::HandleCLIParams(const QStringList &params)
{
    if (params.contains("-p")) {
        const int ind = params.indexOf("-p");
        if ((ind + 1) < params.length())
            LoadProjectFile(params[ind + 1]);
    } else if (params.contains("-l")) {
        QString logFile;
        const int ind = params.indexOf("-l");
        if ((ind + 1) < params.length())
            logFile = params[ind + 1];

        if (params.contains("-d")) {
            QString checkedDir;
            const int ind = params.indexOf("-d");
            if ((ind + 1) < params.length())
                checkedDir = params[ind + 1];

            LoadResults(logFile, checkedDir);
        } else {
            LoadResults(logFile);
        }
    } else
        DoCheckFiles(params);
}

void MainWindow::LoadSettings()
{
    // Window/dialog sizes
    if (mSettings->value(SETTINGS_WINDOW_MAXIMIZED, false).toBool()) {
        showMaximized();
    } else {
        resize(mSettings->value(SETTINGS_WINDOW_WIDTH, 800).toInt(),
               mSettings->value(SETTINGS_WINDOW_HEIGHT, 600).toInt());
    }

    ShowTypes *types = mUI.mResults->GetShowTypes();
    mUI.mActionShowStyle->setChecked(types->isShown(ShowTypes::ShowStyle));
    mUI.mActionShowErrors->setChecked(types->isShown(ShowTypes::ShowErrors));
    mUI.mActionShowWarnings->setChecked(types->isShown(ShowTypes::ShowWarnings));
    mUI.mActionShowPortability->setChecked(types->isShown(ShowTypes::ShowPortability));
    mUI.mActionShowPerformance->setChecked(types->isShown(ShowTypes::ShowPerformance));
    mUI.mActionShowInformation->setChecked(types->isShown(ShowTypes::ShowInformation));

    const bool stdCpp03 = mSettings->value(SETTINGS_STD_CPP03, false).toBool();
    mUI.mActionCpp03->setChecked(stdCpp03);
    const bool stdCpp11 = mSettings->value(SETTINGS_STD_CPP11, true).toBool();
    mUI.mActionCpp11->setChecked(stdCpp11 || !stdCpp03);
    const bool stdC89 = mSettings->value(SETTINGS_STD_C89, false).toBool();
    mUI.mActionC89->setChecked(stdC89);
    const bool stdC11 = mSettings->value(SETTINGS_STD_C11, false).toBool();
    mUI.mActionC11->setChecked(stdC11);
    const bool stdC99 = mSettings->value(SETTINGS_STD_C99, true).toBool();
    mUI.mActionC99->setChecked(stdC99 || (!stdC89 && !stdC11));
    const bool stdPosix = mSettings->value(SETTINGS_STD_POSIX, false).toBool();
    mUI.mActionPosix->setChecked(stdPosix);

    // Main window settings
    const bool showMainToolbar = mSettings->value(SETTINGS_TOOLBARS_MAIN_SHOW, true).toBool();
    mUI.mActionToolBarMain->setChecked(showMainToolbar);
    mUI.mToolBarMain->setVisible(showMainToolbar);

    const bool showViewToolbar = mSettings->value(SETTINGS_TOOLBARS_VIEW_SHOW, true).toBool();
    mUI.mActionToolBarView->setChecked(showViewToolbar);
    mUI.mToolBarView->setVisible(showViewToolbar);

    const bool showFilterToolbar = mSettings->value(SETTINGS_TOOLBARS_FILTER_SHOW, true).toBool();
    mUI.mActionToolBarFilter->setChecked(showFilterToolbar);
    mUI.mToolBarFilter->setVisible(showFilterToolbar);

    bool succeeded = mApplications->LoadSettings();
    if (!succeeded) {
        const QString msg = tr("There was a problem with loading the editor application settings.\n\n"
                               "This is probably because the settings were changed between the Cppcheck versions. "
                               "Please check (and fix) the editor application settings, otherwise the editor "
                               "program might not start correctly.");
        QMessageBox msgBox(QMessageBox::Warning,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgBox.exec();

    }

}

void MainWindow::SaveSettings() const
{
    // Window/dialog sizes
    mSettings->setValue(SETTINGS_WINDOW_WIDTH, size().width());
    mSettings->setValue(SETTINGS_WINDOW_HEIGHT, size().height());
    mSettings->setValue(SETTINGS_WINDOW_MAXIMIZED, isMaximized());

    // Show * states
    mSettings->setValue(SETTINGS_SHOW_STYLE, mUI.mActionShowStyle->isChecked());
    mSettings->setValue(SETTINGS_SHOW_ERRORS, mUI.mActionShowErrors->isChecked());
    mSettings->setValue(SETTINGS_SHOW_WARNINGS, mUI.mActionShowWarnings->isChecked());
    mSettings->setValue(SETTINGS_SHOW_PORTABILITY, mUI.mActionShowPortability->isChecked());
    mSettings->setValue(SETTINGS_SHOW_PERFORMANCE, mUI.mActionShowPerformance->isChecked());
    mSettings->setValue(SETTINGS_SHOW_INFORMATION, mUI.mActionShowInformation->isChecked());

    mSettings->setValue(SETTINGS_STD_CPP03, mUI.mActionCpp03->isChecked());
    mSettings->setValue(SETTINGS_STD_CPP11, mUI.mActionCpp11->isChecked());
    mSettings->setValue(SETTINGS_STD_C89, mUI.mActionC89->isChecked());
    mSettings->setValue(SETTINGS_STD_C99, mUI.mActionC99->isChecked());
    mSettings->setValue(SETTINGS_STD_C11, mUI.mActionC11->isChecked());
    mSettings->setValue(SETTINGS_STD_POSIX, mUI.mActionPosix->isChecked());

    // Main window settings
    mSettings->setValue(SETTINGS_TOOLBARS_MAIN_SHOW, mUI.mToolBarMain->isVisible());
    mSettings->setValue(SETTINGS_TOOLBARS_VIEW_SHOW, mUI.mToolBarView->isVisible());
    mSettings->setValue(SETTINGS_TOOLBARS_FILTER_SHOW, mUI.mToolBarFilter->isVisible());

    mApplications->SaveSettings();

    mSettings->setValue(SETTINGS_LANGUAGE, mTranslation->GetCurrentLanguage());
    mUI.mResults->SaveSettings(mSettings);
}

void MainWindow::DoCheckFiles(const QStringList &files)
{
    if (files.isEmpty()) {
        return;
    }
    ClearResults();

    FileList pathList;
    pathList.AddPathList(files);
    if (mProject)
        pathList.AddExcludeList(mProject->GetProjectFile()->GetExcludedPaths());
    QStringList fileNames = pathList.GetFileList();

    mUI.mResults->Clear(true);
    mThread->ClearFiles();

    if (fileNames.isEmpty()) {
        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        tr("No suitable files found to check!"),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    mUI.mResults->CheckingStarted(fileNames.count());

    mThread->SetFiles(fileNames);
    QDir inf(mCurrentDirectory);
    const QString checkPath = inf.canonicalPath();
    SetPath(SETTINGS_LAST_CHECK_PATH, checkPath);

    CheckLockDownUI(); // lock UI while checking

    mUI.mResults->SetCheckDirectory(checkPath);
    Settings checkSettings = GetCppcheckSettings();

    if (mProject)
        qDebug() << "Checking project file" << mProject->GetProjectFile()->GetFilename();

    mThread->Check(checkSettings, false);
}

void MainWindow::CheckCode(const QString& code, const QString& filename)
{
    // Initialize dummy ThreadResult as ErrorLogger
    ThreadResult result;
    result.SetFiles(QStringList(filename));
    connect(&result, SIGNAL(Progress(int, const QString&)),
            mUI.mResults, SLOT(Progress(int, const QString&)));
    connect(&result, SIGNAL(Error(const ErrorItem &)),
            mUI.mResults, SLOT(Error(const ErrorItem &)));
    connect(&result, SIGNAL(Log(const QString &)),
            this, SLOT(Log(const QString &)));
    connect(&result, SIGNAL(DebugError(const ErrorItem &)),
            this, SLOT(DebugError(const ErrorItem &)));

    // Create CppCheck instance
    CppCheck cppcheck(result, true);
    cppcheck.settings() = GetCppcheckSettings();

    // Check
    CheckLockDownUI();
    ClearResults();
    mUI.mResults->CheckingStarted(1);
    cppcheck.check(filename.toStdString(), code.toStdString());
    CheckDone();
}

QStringList MainWindow::SelectFilesToCheck(QFileDialog::FileMode mode)
{
    if (mProject) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Cppcheck"));
        const QString msg(tr("You must close the project file before selecting new files or directories!"));
        msgBox.setText(msg);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return QStringList();
    }

    QStringList selected;

    // NOTE: we use QFileDialog::getOpenFileNames() and
    // QFileDialog::getExistingDirectory() because they show native Windows
    // selection dialog which is a lot more usable than Qt:s own dialog.
    if (mode == QFileDialog::ExistingFiles) {
        selected = QFileDialog::getOpenFileNames(this,
                   tr("Select files to check"),
                   GetPath(SETTINGS_LAST_CHECK_PATH));
        if (selected.isEmpty())
            mCurrentDirectory.clear();
        else {
            QFileInfo inf(selected[0]);
            mCurrentDirectory = inf.absolutePath();
        }
        FormatAndSetTitle();
    } else if (mode == QFileDialog::DirectoryOnly) {
        QString dir = QFileDialog::getExistingDirectory(this,
                      tr("Select directory to check"),
                      GetPath(SETTINGS_LAST_CHECK_PATH));
        if (!dir.isEmpty()) {
            qDebug() << "Setting current directory to: " << dir;
            mCurrentDirectory = dir;
            selected.append(dir);
            dir = QDir::toNativeSeparators(dir);
            FormatAndSetTitle(dir);
        }
    }

    SetPath(SETTINGS_LAST_CHECK_PATH, mCurrentDirectory);

    return selected;
}

void MainWindow::CheckFiles()
{
    DoCheckFiles(SelectFilesToCheck(QFileDialog::ExistingFiles));
}

void MainWindow::CheckDirectory()
{
    QStringList dir = SelectFilesToCheck(QFileDialog::DirectoryOnly);
    if (dir.isEmpty())
        return;

    QDir checkDir(dir[0]);
    QStringList filters;
    filters << "*.cppcheck";
    checkDir.setFilter(QDir::Files | QDir::Readable);
    checkDir.setNameFilters(filters);
    QStringList projFiles = checkDir.entryList();
    if (!projFiles.empty()) {
        if (projFiles.size() == 1) {
            // If one project file found, suggest loading it
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(tr("Cppcheck"));
            const QString msg(tr("Found project file: %1\n\nDo you want to "
                                 "load this project file instead?").arg(projFiles[0]));
            msgBox.setText(msg);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.addButton(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int dlgResult = msgBox.exec();
            if (dlgResult == QMessageBox::Yes) {
                QString path = checkDir.canonicalPath();
                if (!path.endsWith("/"))
                    path += "/";
                path += projFiles[0];
                LoadProjectFile(path);
            } else {
                DoCheckFiles(dir);
            }
        } else {
            // If multiple project files found inform that there are project
            // files also available.
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(tr("Cppcheck"));
            const QString msg(tr("Found project files from the directory.\n\n"
                                 "Do you want to proceed checking without "
                                 "using any of these project files?"));
            msgBox.setText(msg);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.addButton(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int dlgResult = msgBox.exec();
            if (dlgResult == QMessageBox::Yes) {
                DoCheckFiles(dir);
            }
        }
    } else {
        DoCheckFiles(dir);
    }
}

void MainWindow::AddIncludeDirs(const QStringList &includeDirs, Settings &result)
{
    QString dir;
    foreach(dir, includeDirs) {
        QString incdir;
        if (!QDir::isAbsolutePath(dir))
            incdir = mCurrentDirectory + "/";
        incdir += dir;
        incdir = QDir::cleanPath(incdir);

        // include paths must end with '/'
        if (!incdir.endsWith("/"))
            incdir += "/";
        result._includePaths.push_back(incdir.toStdString());
    }
}

Library::Error MainWindow::LoadLibrary(Library *library, QString filename)
{
    Library::Error ret;

    // Try to load the library from the project folder..
    if (mProject) {
        QString path = QFileInfo(mProject->GetProjectFile()->GetFilename()).canonicalPath();
        ret = library->load(NULL, (path+"/"+filename).toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
    }

    // Try to load the library from the application folder..
    const QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath();
    ret = library->load(NULL, (appPath+"/"+filename).toLatin1());
    if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
        return ret;
    ret = library->load(NULL, (appPath+"/cfg/"+filename).toLatin1());
    if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
        return ret;

    // Try to load the library from the cfg subfolder..
    const QString datadir = mSettings->value("DATADIR", QString()).toString();
    if (!datadir.isEmpty()) {
        ret = library->load(NULL, (datadir+"/"+filename).toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
        ret = library->load(NULL, (datadir+"/cfg/"+filename).toLatin1());
        if (ret.errorcode != Library::ErrorCode::FILE_NOT_FOUND)
            return ret;
    }

    return ret;
}

bool MainWindow::TryLoadLibrary(Library *library, QString filename)
{
    const Library::Error error = LoadLibrary(library, filename);
    if (error.errorcode != Library::ErrorCode::OK) {
        if (error.errorcode == Library::UNKNOWN_ELEMENT) {
            QMessageBox::information(this, tr("Information"), tr("The library '%1' contains unknown elements:\n%2").arg(filename).arg(error.reason.c_str()));
            return true;
        }

        QString errmsg;
        switch (error.errorcode) {
        case Library::ErrorCode::OK:
            break;
        case Library::ErrorCode::FILE_NOT_FOUND:
            errmsg = tr("File not found");
            break;
        case Library::ErrorCode::BAD_XML:
            errmsg = tr("Bad XML");
            break;
        case Library::ErrorCode::MISSING_ATTRIBUTE:
            errmsg = tr("Missing attribute");
            break;
        case Library::ErrorCode::BAD_ATTRIBUTE_VALUE:
            errmsg = tr("Bad attribute value");
            break;
        case Library::ErrorCode::UNSUPPORTED_FORMAT:
            errmsg = tr("Unsupported format");
            break;
        case Library::ErrorCode::DUPLICATE_PLATFORM_TYPE:
            errmsg = tr("Duplicate platform type");
            break;
        case Library::ErrorCode::PLATFORM_TYPE_REDEFINED:
            errmsg = tr("Platform type redefined");
            break;
        }
        if (!error.reason.empty())
            errmsg += " '" + QString::fromStdString(error.reason) + "'";
        QMessageBox::information(this, tr("Information"), tr("Failed to load the selected library '%1'.\n%2").arg(filename).arg(errmsg));
        return false;
    }
    return true;
}

Settings MainWindow::GetCppcheckSettings()
{
    Settings result;

    // If project file loaded, read settings from it
    if (mProject) {
        ProjectFile *pfile = mProject->GetProjectFile();
        QStringList dirs = pfile->GetIncludeDirs();
        AddIncludeDirs(dirs, result);

        const QStringList defines = pfile->GetDefines();
        QString define;
        foreach(define, defines) {
            if (!result.userDefines.empty())
                result.userDefines += ";";
            result.userDefines += define.toStdString();
        }

        const QStringList libraries = pfile->GetLibraries();
        foreach(QString library, libraries) {
            const QString filename = library + ".cfg";
            TryLoadLibrary(&result.library, filename);
        }

        const QStringList suppressions = pfile->GetSuppressions();
        foreach(QString suppression, suppressions) {
            result.nomsg.addSuppressionLine(suppression.toStdString());
        }

        // Only check the given -D configuration
        if (!defines.isEmpty())
            result._maxConfigs = 1;
    }

    // Include directories (and files) are searched in listed order.
    // Global include directories must be added AFTER the per project include
    // directories so per project include directories can override global ones.
    const QString globalIncludes = mSettings->value(SETTINGS_GLOBAL_INCLUDE_PATHS).toString();
    if (!globalIncludes.isEmpty()) {
        QStringList includes = globalIncludes.split(";");
        AddIncludeDirs(includes, result);
    }

    result.addEnabled("warning");
    result.addEnabled("style");
    result.addEnabled("performance");
    result.addEnabled("portability");
    result.addEnabled("information");
    result.addEnabled("missingInclude");
    result.debug = false;
    result.debugwarnings = mSettings->value(SETTINGS_SHOW_DEBUG_WARNINGS, false).toBool();
    result._errorsOnly = false;
    result._verbose = true;
    result._force = mSettings->value(SETTINGS_CHECK_FORCE, 1).toBool();
    result._xml = false;
    result._jobs = mSettings->value(SETTINGS_CHECK_THREADS, 1).toInt();
    result._inlineSuppressions = mSettings->value(SETTINGS_INLINE_SUPPRESSIONS, false).toBool();
    result.inconclusive = mSettings->value(SETTINGS_INCONCLUSIVE_ERRORS, false).toBool();
    result.platformType = (Settings::PlatformType) mSettings->value(SETTINGS_CHECKED_PLATFORM, 0).toInt();
    result.standards.cpp = mSettings->value(SETTINGS_STD_CPP11, true).toBool() ? Standards::CPP11 : Standards::CPP03;
    result.standards.c = mSettings->value(SETTINGS_STD_C99, true).toBool() ? Standards::C99 : (mSettings->value(SETTINGS_STD_C11, false).toBool() ? Standards::C11 : Standards::C89);
    result.standards.posix = mSettings->value(SETTINGS_STD_POSIX, false).toBool();

    const bool std = TryLoadLibrary(&result.library, "std.cfg");
    bool posix = true;
    if (result.standards.posix)
        posix = TryLoadLibrary(&result.library, "posix.cfg");
    bool windows = true;
    if (result.platformType == Settings::Win32A || result.platformType == Settings::Win32W || result.platformType == Settings::Win64)
        windows = TryLoadLibrary(&result.library, "windows.cfg");

    if (!std || !posix || !windows)
        QMessageBox::critical(this, tr("Error"), tr("Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=<directory> at the command line to specify where this file is located.").arg(!std ? "std.cfg" : !posix ? "posix.cfg" : "windows.cfg"));

    if (result._jobs <= 1) {
        result._jobs = 1;
    }

    return result;
}

void MainWindow::CheckDone()
{
    if (mExiting) {
        close();
        return;
    }

    mUI.mResults->CheckingFinished();
    EnableCheckButtons(true);
    mUI.mActionSettings->setEnabled(true);
    mUI.mActionOpenXML->setEnabled(true);
    EnableProjectActions(true);
    EnableProjectOpenActions(true);
    mPlatformActions->setEnabled(true);
    mCStandardActions->setEnabled(true);
    mCppStandardActions->setEnabled(true);
    mUI.mActionPosix->setEnabled(true);
    if (mScratchPad)
        mScratchPad->setEnabled(true);

    if (mUI.mResults->HasResults()) {
        mUI.mActionClearResults->setEnabled(true);
        mUI.mActionSave->setEnabled(true);
    }

    for (int i = 0; i < MaxRecentProjects + 1; i++) {
        if (mRecentProjectActs[i] != NULL)
            mRecentProjectActs[i]->setEnabled(true);
    }

    // Notify user - if the window is not active - that check is ready
    QApplication::alert(this, 3000);
}

void MainWindow::CheckLockDownUI()
{
    EnableCheckButtons(false);
    mUI.mActionSettings->setEnabled(false);
    mUI.mActionOpenXML->setEnabled(false);
    EnableProjectActions(false);
    EnableProjectOpenActions(false);
    mPlatformActions->setEnabled(false);
    mCStandardActions->setEnabled(false);
    mCppStandardActions->setEnabled(false);
    mUI.mActionPosix->setEnabled(false);
    if (mScratchPad)
        mScratchPad->setEnabled(false);

    for (int i = 0; i < MaxRecentProjects + 1; i++) {
        if (mRecentProjectActs[i] != NULL)
            mRecentProjectActs[i]->setEnabled(false);
    }
}

void MainWindow::ProgramSettings()
{
    SettingsDialog dialog(mApplications, mTranslation, this);
    if (dialog.exec() == QDialog::Accepted) {
        dialog.SaveSettingValues();
        mUI.mResults->UpdateSettings(dialog.ShowFullPath(),
                                     dialog.SaveFullPath(),
                                     dialog.SaveAllErrors(),
                                     dialog.ShowNoErrorsMessage(),
                                     dialog.ShowErrorId());
        const QString newLang = mSettings->value(SETTINGS_LANGUAGE, "en").toString();
        SetLanguage(newLang);
    }
}

void MainWindow::ReCheck()
{
    const QStringList files = mThread->GetReCheckFiles();
    if (files.empty())
        return;

    // Clear details, statistics and progress
    mUI.mResults->Clear(false);

    // Clear results for changed files
    for (int i = 0; i < files.size(); ++i)
        mUI.mResults->Clear(files[i]);

    CheckLockDownUI(); // lock UI while checking
    mUI.mResults->CheckingStarted(files.size());

    if (mProject)
        qDebug() << "Rechecking project file" << mProject->GetProjectFile()->GetFilename();

    mThread->Check(GetCppcheckSettings(), true);
}

void MainWindow::ClearResults()
{
    mUI.mResults->Clear(true);
    mUI.mActionClearResults->setEnabled(false);
    mUI.mActionSave->setEnabled(false);
}

void MainWindow::OpenResults()
{
    if (mUI.mResults->HasResults()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Cppcheck"));
        const QString msg(tr("Current results will be cleared.\n\n"
                             "Opening a new XML file will clear current results."
                             "Do you want to proceed?"));
        msgBox.setText(msg);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int dlgResult = msgBox.exec();
        if (dlgResult == QMessageBox::No) {
            return;
        }
    }

    QString selectedFilter;
    const QString filter(tr("XML files (*.xml)"));
    QString selectedFile = QFileDialog::getOpenFileName(this,
                           tr("Open the report file"),
                           GetPath(SETTINGS_LAST_RESULT_PATH),
                           filter,
                           &selectedFilter);

    if (!selectedFile.isEmpty()) {
        LoadResults(selectedFile);
    }
}

void MainWindow::LoadResults(const QString selectedFile)
{
    if (!selectedFile.isEmpty()) {
        mUI.mResults->Clear(true);
        mUI.mResults->ReadErrorsXml(selectedFile);
        SetPath(SETTINGS_LAST_RESULT_PATH, selectedFile);
    }
}

void MainWindow::LoadResults(const QString selectedFile, const QString sourceDirectory)
{
    LoadResults(selectedFile);
    mUI.mResults->SetCheckDirectory(sourceDirectory);
}

void MainWindow::EnableCheckButtons(bool enable)
{
    mUI.mActionStop->setEnabled(!enable);
    mUI.mActionCheckFiles->setEnabled(enable);

    if (!enable || mThread->HasPreviousFiles())
        mUI.mActionRecheck->setEnabled(enable);

    mUI.mActionCheckDirectory->setEnabled(enable);
}

void MainWindow::ShowStyle(bool checked)
{
    mUI.mResults->ShowResults(ShowTypes::ShowStyle, checked);
}

void MainWindow::ShowErrors(bool checked)
{
    mUI.mResults->ShowResults(ShowTypes::ShowErrors, checked);
}

void MainWindow::ShowWarnings(bool checked)
{
    mUI.mResults->ShowResults(ShowTypes::ShowWarnings, checked);
}

void MainWindow::ShowPortability(bool checked)
{
    mUI.mResults->ShowResults(ShowTypes::ShowPortability, checked);
}

void MainWindow::ShowPerformance(bool checked)
{
    mUI.mResults->ShowResults(ShowTypes::ShowPerformance, checked);
}

void MainWindow::ShowInformation(bool checked)
{
    mUI.mResults->ShowResults(ShowTypes::ShowInformation, checked);
}

void MainWindow::CheckAll()
{
    ToggleAllChecked(true);
}

void MainWindow::UncheckAll()
{
    ToggleAllChecked(false);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check that we aren't checking files
    if (!mThread->IsChecking()) {
        SaveSettings();
        event->accept();
    } else {
        const QString text(tr("Checking is running.\n\n" \
                              "Do you want to stop the checking and exit Cppcheck?"));

        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        text,
                        QMessageBox::Yes | QMessageBox::No,
                        this);

        msg.setDefaultButton(QMessageBox::No);
        int rv = msg.exec();
        if (rv == QMessageBox::Yes) {
            // This isn't really very clean way to close threads but since the app is
            // exiting it doesn't matter.
            mThread->Stop();
            SaveSettings();
            mExiting = true;
        }
        event->ignore();
    }
}

void MainWindow::ToggleAllChecked(bool checked)
{
    mUI.mActionShowStyle->setChecked(checked);
    ShowStyle(checked);
    mUI.mActionShowErrors->setChecked(checked);
    ShowErrors(checked);
    mUI.mActionShowWarnings->setChecked(checked);
    ShowWarnings(checked);
    mUI.mActionShowPortability->setChecked(checked);
    ShowPortability(checked);
    mUI.mActionShowPerformance->setChecked(checked);
    ShowPerformance(checked);
    mUI.mActionShowInformation->setChecked(checked);
    ShowInformation(checked);
}

void MainWindow::About()
{
    AboutDialog *dlg = new AboutDialog(CppCheck::version(), CppCheck::extraVersion(), this);
    dlg->exec();
}

void MainWindow::ShowLicense()
{
    FileViewDialog *dlg = new FileViewDialog(":COPYING", tr("License"), this);
    dlg->resize(570, 400);
    dlg->exec();
}

void MainWindow::ShowAuthors()
{
    FileViewDialog *dlg = new FileViewDialog(":AUTHORS", tr("Authors"), this);
    dlg->resize(350, 400);
    dlg->exec();
}

void MainWindow::Save()
{
    QString selectedFilter;
    const QString filter(tr("XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)"));
    QString selectedFile = QFileDialog::getSaveFileName(this,
                           tr("Save the report file"),
                           GetPath(SETTINGS_LAST_RESULT_PATH),
                           filter,
                           &selectedFilter);

    if (!selectedFile.isEmpty()) {
        Report::Type type = Report::TXT;
        if (selectedFilter == tr("XML files version 1 (*.xml)")) {
            type = Report::XML;
            if (!selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                selectedFile += ".xml";
        } else if (selectedFilter == tr("XML files version 2 (*.xml)")) {
            type = Report::XMLV2;
            if (!selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                selectedFile += ".xml";
        } else if (selectedFilter == tr("Text files (*.txt)")) {
            type = Report::TXT;
            if (!selectedFile.endsWith(".txt", Qt::CaseInsensitive))
                selectedFile += ".txt";
        } else if (selectedFilter == tr("CSV files (*.csv)")) {
            type = Report::CSV;
            if (!selectedFile.endsWith(".csv", Qt::CaseInsensitive))
                selectedFile += ".csv";
        } else {
            if (selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                type = Report::XML;
            else if (selectedFile.endsWith(".txt", Qt::CaseInsensitive))
                type = Report::TXT;
            else if (selectedFile.endsWith(".csv", Qt::CaseInsensitive))
                type = Report::CSV;
        }

        mUI.mResults->Save(selectedFile, type);
        SetPath(SETTINGS_LAST_RESULT_PATH, selectedFile);
    }
}

void MainWindow::ResultsAdded()
{
}

void MainWindow::ToggleMainToolBar()
{
    mUI.mToolBarMain->setVisible(mUI.mActionToolBarMain->isChecked());
}

void MainWindow::ToggleViewToolBar()
{
    mUI.mToolBarView->setVisible(mUI.mActionToolBarView->isChecked());
}

void MainWindow::ToggleFilterToolBar()
{
    mUI.mToolBarFilter->setVisible(mUI.mActionToolBarFilter->isChecked());
    mLineEditFilter->clear(); // Clearing the filter also disables filtering
}

void MainWindow::FormatAndSetTitle(const QString &text)
{
    QString title;
    if (text.isEmpty())
        title = tr("Cppcheck");
    else
        title = QString(tr("Cppcheck - %1")).arg(text);
    setWindowTitle(title);
}

void MainWindow::SetLanguage(const QString &code)
{
    const QString currentLang = mTranslation->GetCurrentLanguage();
    if (currentLang == code)
        return;

    if (mTranslation->SetLanguage(code)) {
        //Translate everything that is visible here
        mUI.retranslateUi(this);
        mUI.mResults->Translate();
        delete mLogView;
        mLogView = 0;
    }
}

void MainWindow::AboutToShowViewMenu()
{
    mUI.mActionToolBarMain->setChecked(mUI.mToolBarMain->isVisible());
    mUI.mActionToolBarView->setChecked(mUI.mToolBarView->isVisible());
    mUI.mActionToolBarFilter->setChecked(mUI.mToolBarFilter->isVisible());
}

void MainWindow::StopChecking()
{
    mThread->Stop();
    mUI.mResults->DisableProgressbar();
}

void MainWindow::OpenHelpContents()
{
    OpenOnlineHelp();
}

void MainWindow::OpenOnlineHelp()
{
    QDesktopServices::openUrl(QUrl(OnlineHelpURL));
}

void MainWindow::OpenProjectFile()
{
    const QString lastPath = mSettings->value(SETTINGS_LAST_PROJECT_PATH, QString()).toString();
    const QString filter = tr("Project files (*.cppcheck);;All files(*.*)");
    const QString filepath = QFileDialog::getOpenFileName(this,
                             tr("Select Project File"),
                             GetPath(SETTINGS_LAST_PROJECT_PATH),
                             filter);

    if (!filepath.isEmpty()) {
        const QFileInfo fi(filepath);
        if (fi.exists() && fi.isFile() && fi.isReadable()) {
            SetPath(SETTINGS_LAST_PROJECT_PATH, filepath);
            LoadProjectFile(filepath);
        }
    }
}

void MainWindow::ShowScratchpad()
{
    if (!mScratchPad)
        mScratchPad = new ScratchPad(*this);

    mScratchPad->show();

    if (!mScratchPad->isActiveWindow())
        mScratchPad->activateWindow();
}

void MainWindow::LoadProjectFile(const QString &filePath)
{
    QFileInfo inf(filePath);
    const QString filename = inf.fileName();
    FormatAndSetTitle(tr("Project:") + QString(" ") + filename);
    AddProjectMRU(filePath);

    mUI.mActionCloseProjectFile->setEnabled(true);
    mUI.mActionEditProjectFile->setEnabled(true);
    delete mProject;
    mProject = new Project(filePath, this);
    CheckProject(mProject);
}

void MainWindow::CheckProject(Project *project)
{
    if (!project->IsOpen()) {
        if (!project->Open()) {
            delete mProject;
            mProject = 0;
            return;
        }
    }

    QFileInfo inf(project->Filename());
    const QString rootpath = project->GetProjectFile()->GetRootPath();

    // If the root path is not given or is not "current dir", use project
    // file's location directory as root path
    if (rootpath.isEmpty() || rootpath == ".")
        mCurrentDirectory = inf.canonicalPath();
    else
        mCurrentDirectory = rootpath;

    QStringList paths = project->GetProjectFile()->GetCheckPaths();

    // If paths not given then check the root path (which may be the project
    // file's location, see above). This is to keep the compatibility with
    // old "silent" project file loading when we checked the director where the
    // project file was located.
    if (paths.isEmpty()) {
        paths << mCurrentDirectory;
    }

    // Convert relative paths to absolute paths
    for (int i = 0; i < paths.size(); i++) {
        if (!QDir::isAbsolutePath(paths[i])) {
            QString path = mCurrentDirectory + "/";
            path += paths[i];
            paths[i] = QDir::cleanPath(path);
        }
    }
    DoCheckFiles(paths);
}

void MainWindow::NewProjectFile()
{
    const QString filter = tr("Project files (*.cppcheck);;All files(*.*)");
    QString filepath = QFileDialog::getSaveFileName(this,
                       tr("Select Project Filename"),
                       GetPath(SETTINGS_LAST_PROJECT_PATH),
                       filter);

    if (filepath.isEmpty())
        return;

    SetPath(SETTINGS_LAST_PROJECT_PATH, filepath);

    EnableProjectActions(true);
    QFileInfo inf(filepath);
    const QString filename = inf.fileName();
    FormatAndSetTitle(tr("Project:") + QString(" ") + filename);

    delete mProject;
    mProject = new Project(filepath, this);
    mProject->Create();
    if (mProject->Edit()) {
        AddProjectMRU(filepath);
        CheckProject(mProject);
    }
}

void MainWindow::CloseProjectFile()
{
    delete mProject;
    mProject = NULL;
    EnableProjectActions(false);
    EnableProjectOpenActions(true);
    FormatAndSetTitle();
}

void MainWindow::EditProjectFile()
{
    if (!mProject) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        QString(tr("No project file loaded")),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }
    mProject->Edit();
}

void MainWindow::ShowLogView()
{
    if (mLogView == NULL)
        mLogView = new LogView;

    mLogView->show();
    if (!mLogView->isActiveWindow())
        mLogView->activateWindow();
}

void MainWindow::ShowStatistics()
{
    StatsDialog statsDialog(this);

    // Show a dialog with the previous scan statistics and project information
    if (mProject) {
        statsDialog.setProject(*mProject);
    }
    statsDialog.setPathSelected(mCurrentDirectory);
    statsDialog.setNumberOfFilesScanned(mThread->GetPreviousFilesCount());
    statsDialog.setScanDuration(mThread->GetPreviousScanDuration() / 1000.0);
    statsDialog.setStatistics(mUI.mResults->GetStatistics());

    statsDialog.exec();
}

void MainWindow::Log(const QString &logline)
{
    if (mLogView) {
        mLogView->AppendLine(logline);
    }
}

void MainWindow::DebugError(const ErrorItem &item)
{
    if (mLogView) {
        mLogView->AppendLine(item.ToString());
    }
}

void MainWindow::FilterResults()
{
    mUI.mResults->FilterResults(mLineEditFilter->text());
}

void MainWindow::EnableProjectActions(bool enable)
{
    mUI.mActionCloseProjectFile->setEnabled(enable);
    mUI.mActionEditProjectFile->setEnabled(enable);
}

void MainWindow::EnableProjectOpenActions(bool enable)
{
    mUI.mActionNewProjectFile->setEnabled(enable);
    mUI.mActionOpenProjectFile->setEnabled(enable);
}

void MainWindow::OpenRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        const QString project = action->data().toString();
        QFileInfo inf(project);
        if (inf.exists()) {
            LoadProjectFile(project);
        } else {
            const QString text(tr("The project file\n\n%1\n\n could not be found!\n\n"
                                  "Do you want to remove the file from the recently "
                                  "used projects -list?").arg(project));

            QMessageBox msg(QMessageBox::Warning,
                            tr("Cppcheck"),
                            text,
                            QMessageBox::Yes | QMessageBox::No,
                            this);

            msg.setDefaultButton(QMessageBox::No);
            int rv = msg.exec();
            if (rv == QMessageBox::Yes) {
                RemoveProjectMRU(project);
            }

        }
    }
}

void MainWindow::UpdateMRUMenuItems()
{
    for (int i = 0; i < MaxRecentProjects + 1; i++) {
        if (mRecentProjectActs[i] != NULL)
            mUI.mMenuFile->removeAction(mRecentProjectActs[i]);
    }

    QStringList projects = mSettings->value(SETTINGS_MRU_PROJECTS).toStringList();

    // Do a sanity check - remove duplicates and empty or space only items
    int removed = projects.removeDuplicates();
    for (int i = projects.size() - 1; i >= 0; i--) {
        QString text = projects[i].trimmed();
        if (text.isEmpty()) {
            projects.removeAt(i);
            removed++;
        }
    }

    if (removed)
        mSettings->setValue(SETTINGS_MRU_PROJECTS, projects);

    const int numRecentProjects = qMin(projects.size(), (int)MaxRecentProjects);
    for (int i = 0; i < numRecentProjects; i++) {
        const QString filename = QFileInfo(projects[i]).fileName();
        const QString text = QString("&%1 %2").arg(i + 1).arg(filename);
        mRecentProjectActs[i]->setText(text);
        mRecentProjectActs[i]->setData(projects[i]);
        mRecentProjectActs[i]->setVisible(true);
        mUI.mMenuFile->insertAction(mUI.mActionProjectMRU, mRecentProjectActs[i]);
    }

    if (numRecentProjects > 1)
        mRecentProjectActs[numRecentProjects] = mUI.mMenuFile->insertSeparator(mUI.mActionProjectMRU);
}

void MainWindow::AddProjectMRU(const QString &project)
{
    QStringList files = mSettings->value(SETTINGS_MRU_PROJECTS).toStringList();
    files.removeAll(project);
    files.prepend(project);
    while (files.size() > MaxRecentProjects)
        files.removeLast();

    mSettings->setValue(SETTINGS_MRU_PROJECTS, files);
    UpdateMRUMenuItems();
}

void MainWindow::RemoveProjectMRU(const QString &project)
{
    QStringList files = mSettings->value(SETTINGS_MRU_PROJECTS).toStringList();
    files.removeAll(project);

    mSettings->setValue(SETTINGS_MRU_PROJECTS, files);
    UpdateMRUMenuItems();
}

void MainWindow::SelectPlatform()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        const Settings::PlatformType platform = (Settings::PlatformType) action->data().toInt();
        mSettings->setValue(SETTINGS_CHECKED_PLATFORM, platform);
    }
}

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

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QDirIterator>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QKeySequence>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include "mainwindow.h"
#include "aboutdialog.h"
#include "threadhandler.h"
#include "fileviewdialog.h"
#include "projectfile.h"
#include "project.h"
#include "report.h"
#include "statsdialog.h"
#include "logview.h"
#include "filelist.h"

static const QString OnlineHelpURL("http://cppcheck.sourceforge.net/manual.html");

MainWindow::MainWindow() :
    mSettings(new QSettings("Cppcheck", "Cppcheck-GUI", this)),
    mApplications(new ApplicationList(this)),
    mTranslation(new TranslationHandler(this)),
    mLogView(NULL),
    mProject(NULL),
    mExiting(false)
{
    mUI.setupUi(this);
    mUI.mResults->Initialize(mSettings, mApplications);

    mThread = new ThreadHandler(this);
    mLogView = new LogView;

    // Filter timer to delay filtering results slightly while typing
    mFilterTimer = new QTimer(this);
    mFilterTimer->setInterval(500);
    mFilterTimer->setSingleShot(true);
    connect(mFilterTimer, SIGNAL(timeout()), this, SLOT(FilterResults()));

    // "Filter" toolbar
    mLineEditFilter = new QLineEdit(mUI.mToolBarFilter);
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    mLineEditFilter->setPlaceholderText(tr("Quick Filter:"));
#endif
    mUI.mToolBarFilter->addWidget(mLineEditFilter);
    connect(mLineEditFilter, SIGNAL(textChanged(const QString&)), mFilterTimer, SLOT(start()));
    connect(mLineEditFilter, SIGNAL(returnPressed()), this, SLOT(FilterResults()));

    connect(mUI.mActionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(mUI.mActionCheckFiles, SIGNAL(triggered()), this, SLOT(CheckFiles()));
    connect(mUI.mActionCheckDirectory, SIGNAL(triggered()), this, SLOT(CheckDirectory()));
    connect(mUI.mActionSettings, SIGNAL(triggered()), this, SLOT(ProgramSettings()));
    connect(mUI.mActionClearResults, SIGNAL(triggered()), this, SLOT(ClearResults()));
    connect(mUI.mActionOpenXML, SIGNAL(triggered()), this, SLOT(OpenXML()));

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

    QStringList args = QCoreApplication::arguments();
    //Remove the application itself
    args.removeFirst();
    if (!args.isEmpty())
    {
        HandleCLIParams(args);
    }

    for (int i = 0; i < MaxRecentProjects; ++i)
    {
        mRecentProjectActs[i] = new QAction(this);
        mRecentProjectActs[i]->setVisible(false);
        connect(mRecentProjectActs[i], SIGNAL(triggered()),
                this, SLOT(OpenRecentProject()));
    }
    mUI.mActionProjectMRU->setVisible(false);
    UpdateMRUMenuItems();
}

MainWindow::~MainWindow()
{
    delete mLogView;
    delete mProject;
}

void MainWindow::HandleCLIParams(const QStringList &params)
{
    if (params.contains("-p"))
    {
        QString projFile;
        const int ind = params.indexOf("-p");
        if ((ind + 1) < params.length())
            projFile = params[ind + 1];

        LoadProjectFile(projFile);
    }
    else
        DoCheckFiles(params);
}

void MainWindow::LoadSettings()
{
    // Window/dialog sizes
    if (mSettings->value(SETTINGS_WINDOW_MAXIMIZED, false).toBool())
    {
        showMaximized();
    }
    else
    {
        resize(mSettings->value(SETTINGS_WINDOW_WIDTH, 800).toInt(),
               mSettings->value(SETTINGS_WINDOW_HEIGHT, 600).toInt());
    }

    // Show * states
    mUI.mActionShowStyle->setChecked(mSettings->value(SETTINGS_SHOW_STYLE, true).toBool());
    mUI.mActionShowErrors->setChecked(mSettings->value(SETTINGS_SHOW_ERRORS, true).toBool());
    mUI.mActionShowWarnings->setChecked(mSettings->value(SETTINGS_SHOW_WARNINGS, true).toBool());
    mUI.mActionShowPortability->setChecked(mSettings->value(SETTINGS_SHOW_PORTABILITY, true).toBool());
    mUI.mActionShowPerformance->setChecked(mSettings->value(SETTINGS_SHOW_PERFORMANCE, true).toBool());
    mUI.mActionShowInformation->setChecked(mSettings->value(SETTINGS_SHOW_INFORMATION, true).toBool());

    mUI.mResults->ShowResults(SHOW_ERRORS, mUI.mActionShowErrors->isChecked());
    mUI.mResults->ShowResults(SHOW_STYLE, mUI.mActionShowStyle->isChecked());

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

    SetLanguage(mSettings->value(SETTINGS_LANGUAGE, mTranslation->SuggestLanguage()).toString());

    bool succeeded = mApplications->LoadSettings();
    if (!succeeded)
    {
        QString msg = tr("There was a problem with loading the editor application settings.\n\n"
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

void MainWindow::SaveSettings()
{
    //Force toolbar checkbox value to be updated
    AboutToShowViewMenu();

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
    if (files.isEmpty())
    {
        return;
    }
    ClearResults();

    FileList pathList;
    pathList.AddPathList(files);
    if (mProject)
        pathList.AddIngoreList(mProject->GetProjectFile()->GetIgnoredPaths());
    QStringList fileNames = pathList.GetFileList();

    mUI.mResults->Clear();
    mThread->ClearFiles();

    if (fileNames.isEmpty())
    {
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
    mSettings->setValue(SETTINGS_CHECK_PATH, checkPath);

    CheckLockDownUI(); // lock UI while checking

    mUI.mResults->SetCheckDirectory(checkPath);
    Settings checkSettings = GetCppcheckSettings();

    if (mProject)
        qDebug() << "Checking project file" << mProject->GetProjectFile()->GetFilename();

    mThread->Check(checkSettings, false);
}

QStringList MainWindow::SelectFilesToCheck(QFileDialog::FileMode mode)
{
    if (mProject)
    {
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
    // selection dialog which is a lot more usable than QT:s own dialog.
    if (mode == QFileDialog::ExistingFiles)
    {
        selected = QFileDialog::getOpenFileNames(this,
                   tr("Select files to check"),
                   mSettings->value(SETTINGS_CHECK_PATH, "").toString());
        if (selected.isEmpty())
            mCurrentDirectory.clear();
        else
        {
            QFileInfo inf(selected[0]);
            mCurrentDirectory = inf.absolutePath();
        }
        FormatAndSetTitle();
    }
    else if (mode == QFileDialog::DirectoryOnly)
    {
        QString dir = QFileDialog::getExistingDirectory(this,
                      tr("Select directory to check"),
                      mSettings->value(SETTINGS_CHECK_PATH, "").toString());
        if (!dir.isEmpty())
        {
            qDebug() << "Setting current directory to: " << dir;
            mCurrentDirectory = dir;
            selected.append(dir);
            dir = QDir::toNativeSeparators(dir);
            FormatAndSetTitle(dir);
        }
    }

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
    if (!projFiles.empty())
    {
        if (projFiles.size() == 1)
        {
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
            if (dlgResult == QMessageBox::Yes)
            {
                QString path = checkDir.canonicalPath();
                if (!path.endsWith("/"))
                    path += "/";
                path += projFiles[0];
                LoadProjectFile(path);
            }
            else
            {
                DoCheckFiles(dir);
            }
        }
        else
        {
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
            if (dlgResult == QMessageBox::Yes)
            {
                DoCheckFiles(dir);
            }
        }
    }
    else
    {
        DoCheckFiles(dir);
    }
}

void MainWindow::AddIncludeDirs(const QStringList &includeDirs, Settings &result)
{
    QString dir;
    foreach(dir, includeDirs)
    {
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

Settings MainWindow::GetCppcheckSettings()
{
    Settings result;

    QString globalIncludes = mSettings->value(SETTINGS_GLOBAL_INCLUDE_PATHS).toString();
    if (!globalIncludes.isEmpty())
    {
        QStringList includes = globalIncludes.split(";");
        AddIncludeDirs(includes, result);
    }

    // If project file loaded, read settings from it
    if (mProject)
    {
        ProjectFile *pfile = mProject->GetProjectFile();
        QStringList dirs = pfile->GetIncludeDirs();
        AddIncludeDirs(dirs, result);

        QStringList defines = pfile->GetDefines();
        QString define;
        foreach(define, defines)
        {
            if (!result.userDefines.empty())
                result.userDefines += ";";
            result.userDefines += define.toStdString();
        }
    }

    result.addEnabled("style");
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

    if (result._jobs <= 0)
    {
        result._jobs = 1;
    }

    return result;
}

void MainWindow::CheckDone()
{
    if (mExiting)
    {
        close();
        return;
    }

    mUI.mResults->CheckingFinished();
    EnableCheckButtons(true);
    mUI.mActionSettings->setEnabled(true);
    mUI.mActionOpenXML->setEnabled(true);
    EnableProjectActions(true);
    EnableProjectOpenActions(true);

    if (mUI.mResults->HasResults())
    {
        mUI.mActionClearResults->setEnabled(true);
        mUI.mActionSave->setEnabled(true);
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
}

void MainWindow::ProgramSettings()
{
    SettingsDialog dialog(mApplications, mTranslation, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        dialog.SaveSettingValues();
        mUI.mResults->UpdateSettings(dialog.ShowFullPath(),
                                     dialog.SaveFullPath(),
                                     dialog.SaveAllErrors(),
                                     dialog.ShowNoErrorsMessage());
        const QString newLang = mSettings->value(SETTINGS_LANGUAGE, "en").toString();
        SetLanguage(newLang);
    }
}

void MainWindow::ReCheck()
{
    ClearResults();
    CheckLockDownUI(); // lock UI while checking

    const int filesCount = mThread->GetPreviousFilesCount();
    Q_ASSERT(filesCount > 0); // If no files should not be able to recheck
    mUI.mResults->CheckingStarted(filesCount);

    if (mProject)
        qDebug() << "Rechecking project file" << mProject->GetProjectFile()->GetFilename();

    mThread->Check(GetCppcheckSettings(), true);
}

void MainWindow::ClearResults()
{
    mUI.mResults->Clear();
    mUI.mActionClearResults->setEnabled(false);
    mUI.mActionSave->setEnabled(false);
}

void MainWindow::OpenXML()
{
    QString selectedFilter;
    QString filter(tr("XML files (*.xml)"));
    QString selectedFile = QFileDialog::getOpenFileName(this,
                           tr("Open the report file"),
                           QString(),
                           filter,
                           &selectedFilter);

    if (!selectedFile.isEmpty())
    {
        mUI.mResults->ReadErrorsXml(selectedFile);
    }
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
    mUI.mResults->ShowResults(SHOW_STYLE, checked);
}

void MainWindow::ShowErrors(bool checked)
{
    mUI.mResults->ShowResults(SHOW_ERRORS, checked);
}

void MainWindow::ShowWarnings(bool checked)
{
    mUI.mResults->ShowResults(SHOW_WARNINGS, checked);
}

void MainWindow::ShowPortability(bool checked)
{
    mUI.mResults->ShowResults(SHOW_PORTABILITY, checked);
}

void MainWindow::ShowPerformance(bool checked)
{
    mUI.mResults->ShowResults(SHOW_PERFORMANCE, checked);
}

void MainWindow::ShowInformation(bool checked)
{
    mUI.mResults->ShowResults(SHOW_INFORMATION, checked);
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
    if (!mThread->IsChecking())
    {
        SaveSettings();
        event->accept();
    }
    else
    {
        QString text(tr("Checking is running.\n\n" \
                        "Do you want to stop the checking and exit Cppcheck?."));

        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        text,
                        QMessageBox::Yes | QMessageBox::No,
                        this);

        msg.setDefaultButton(QMessageBox::No);
        int rv = msg.exec();
        if (rv == QMessageBox::Yes)
        {
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
    QString filter(tr("XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)"));
    QString selectedFile = QFileDialog::getSaveFileName(this,
                           tr("Save the report file"),
                           QString(),
                           filter,
                           &selectedFilter);

    if (!selectedFile.isEmpty())
    {
        Report::Type type = Report::TXT;
        if (selectedFilter == tr("XML files version 1 (*.xml)"))
        {
            type = Report::XML;
            if (!selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                selectedFile += ".xml";
        }
        else if (selectedFilter == tr("XML files version 2 (*.xml)"))
        {
            type = Report::XMLV2;
            if (!selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                selectedFile += ".xml";
        }
        else if (selectedFilter == tr("Text files (*.txt)"))
        {
            type = Report::TXT;
            if (!selectedFile.endsWith(".txt", Qt::CaseInsensitive))
                selectedFile += ".txt";
        }
        else if (selectedFilter == tr("CSV files (*.csv)"))
        {
            type = Report::CSV;
            if (!selectedFile.endsWith(".csv", Qt::CaseInsensitive))
                selectedFile += ".csv";
        }
        else
        {
            if (selectedFile.endsWith(".xml", Qt::CaseInsensitive))
                type = Report::XML;
            else if (selectedFile.endsWith(".txt", Qt::CaseInsensitive))
                type = Report::TXT;
            else if (selectedFile.endsWith(".csv", Qt::CaseInsensitive))
                type = Report::CSV;
        }

        mUI.mResults->Save(selectedFile, type);
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

    QString error;
    if (!mTranslation->SetLanguage(code, error))
    {
        const QString msg(tr("Failed to change the user interface language:"
                             "\n\n%1\n\n"
                             "The user interface language has been reset to English. Open "
                             "the Preferences-dialog to select any of the available "
                             "languages.").arg(error));
        QMessageBox msgBox(QMessageBox::Warning,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgBox.exec();
    }
    else
    {
        //Translate everything that is visible here
        mUI.retranslateUi(this);
        mUI.mResults->Translate();
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
    const QString filter = tr("Project files (*.cppcheck);;All files(*.*)");
    QString filepath = QFileDialog::getOpenFileName(this,
                       tr("Select Project File"),
                       QString(),
                       filter);

    if (!filepath.isEmpty())
    {
        LoadProjectFile(filepath);
    }
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
    mProject->Open();
    QString rootpath = mProject->GetProjectFile()->GetRootPath();

    // If the root path is not given or is not "current dir", use project
    // file's location directory as root path
    if (rootpath.isEmpty() || rootpath == ".")
        mCurrentDirectory = inf.canonicalPath();
    else
        mCurrentDirectory = rootpath;

    QStringList paths = mProject->GetProjectFile()->GetCheckPaths();

    // If paths not given then check the root path (which may be the project
    // file's location, see above). This is to keep the compatibility with
    // old "silent" project file loading when we checked the director where the
    // project file was located.
    if (paths.isEmpty())
    {
        paths << mCurrentDirectory;
    }

    // Convert relative paths to absolute paths
    for (int i = 0; i < paths.size(); i++)
    {
        if (!QDir::isAbsolutePath(paths[i]))
        {
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
                       QString(),
                       filter);

    if (!filepath.isEmpty())
    {
        EnableProjectActions(true);
        QFileInfo inf(filepath);
        const QString filename = inf.fileName();
        FormatAndSetTitle(tr("Project:") + QString(" ") + filename);

        delete mProject;
        mProject = new Project(filepath, this);
        mProject->Create();
        mProject->Edit();
    }
    AddProjectMRU(filepath);
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
    if (!mProject)
    {
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
    if (mProject)
    {
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
    if (mLogView)
    {
        mLogView->AppendLine(logline);
    }
}

void MainWindow::DebugError(const ErrorItem &item)
{
    if (mLogView)
    {
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
    if (action)
    {
        const QString project = action->data().toString();
        QFileInfo inf(project);
        if (inf.exists())
        {
            LoadProjectFile(project);
        }
        else
        {
            QString text(tr("The project file\n\n%1\n\n could not be found!\n\n"
                            "Do you want to remove the file from the recently "
                            "used projects -list?").arg(project));

            QMessageBox msg(QMessageBox::Warning,
                            tr("Cppcheck"),
                            text,
                            QMessageBox::Yes | QMessageBox::No,
                            this);

            msg.setDefaultButton(QMessageBox::No);
            int rv = msg.exec();
            if (rv == QMessageBox::Yes)
            {
                RemoveProjectMRU(project);
            }

        }
    }
}

void MainWindow::UpdateMRUMenuItems()
{
    for (int i = 0; i < MaxRecentProjects; i++)
    {
        if (mRecentProjectActs[i] != NULL)
            mUI.mMenuFile->removeAction(mRecentProjectActs[i]);
    }

    QStringList projects = mSettings->value(SETTINGS_MRU_PROJECTS).toStringList();
    const int numRecentProjects = qMin(projects.size(), (int)MaxRecentProjects);
    for (int i = 0; i < numRecentProjects; i++)
    {
        const QString filename = QFileInfo(projects[i]).fileName();
        const QString text = QString("&%1 %2").arg(i + 1).arg(filename);
        mRecentProjectActs[i]->setText(text);
        mRecentProjectActs[i]->setData(projects[i]);
        mRecentProjectActs[i]->setVisible(true);
        mUI.mMenuFile->insertAction(mUI.mActionProjectMRU, mRecentProjectActs[i]);
    }

    if (numRecentProjects > 1)
        mUI.mMenuFile->insertSeparator(mUI.mActionProjectMRU);
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

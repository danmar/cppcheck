/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

#include "mainwindow.h"
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
#include "aboutdialog.h"
#include "threadhandler.h"
#include "fileviewdialog.h"
#include "projectfile.h"
#include "report.h"
#include "../lib/filelister.h"

// HTMLHelp is only available in Windows
#ifdef WIN32
#include <windows.h>
#include <htmlhelp.h>
#endif

MainWindow::MainWindow() :
        mSettings(new QSettings("Cppcheck", "Cppcheck-GUI", this)),
        mApplications(new ApplicationList(this)),
        mTranslation(new TranslationHandler(this)),
        mLanguages(new QActionGroup(this))
{
    mUI.setupUi(this);
    mUI.mResults->Initialize(mSettings, mApplications);

    mThread = new ThreadHandler(this);

    connect(mUI.mActionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(mUI.mActionCheckFiles, SIGNAL(triggered()), this, SLOT(CheckFiles()));
    connect(mUI.mActionCheckDirectory, SIGNAL(triggered()), this, SLOT(CheckDirectory()));
    connect(mUI.mActionSettings, SIGNAL(triggered()), this, SLOT(ProgramSettings()));
    connect(mUI.mActionClearResults, SIGNAL(triggered()), this, SLOT(ClearResults()));

    connect(mUI.mActionShowAll, SIGNAL(toggled(bool)), this, SLOT(ShowAll(bool)));
    connect(mUI.mActionShowStyle, SIGNAL(toggled(bool)), this, SLOT(ShowStyle(bool)));
    connect(mUI.mActionShowAllStyle, SIGNAL(toggled(bool)), this, SLOT(ShowAllStyle(bool)));
    connect(mUI.mActionShowErrors, SIGNAL(toggled(bool)), this, SLOT(ShowErrors(bool)));
    connect(mUI.mActionCheckAll, SIGNAL(triggered()), this, SLOT(CheckAll()));
    connect(mUI.mActionUncheckAll, SIGNAL(triggered()), this, SLOT(UncheckAll()));
    connect(mUI.mActionCollapseAll, SIGNAL(triggered()), mUI.mResults, SLOT(CollapseAllResults()));
    connect(mUI.mActionExpandAll, SIGNAL(triggered()), mUI.mResults, SLOT(ExpandAllResults()));

    connect(mUI.mActionRecheck, SIGNAL(triggered()), this, SLOT(ReCheck()));

    connect(mUI.mActionStop, SIGNAL(triggered()), this, SLOT(StopChecking()));
    connect(mUI.mActionSave, SIGNAL(triggered()), this, SLOT(Save()));

    connect(mUI.mActionAbout, SIGNAL(triggered()), this, SLOT(About()));
    connect(mUI.mActionLicense, SIGNAL(triggered()), this, SLOT(ShowLicense()));
    connect(mUI.mActionToolbar, SIGNAL(toggled(bool)), this, SLOT(ToggleToolbar()));

    connect(mUI.mActionAuthors, SIGNAL(triggered()), this, SLOT(ShowAuthors()));
    connect(mThread, SIGNAL(Done()), this, SLOT(CheckDone()));
    connect(mUI.mResults, SIGNAL(GotResults()), this, SLOT(ResultsAdded()));
    connect(mUI.mMenuView, SIGNAL(aboutToShow()), this, SLOT(AboutToShowViewMenu()));

#ifdef WIN32
    connect(mUI.mActionHelpContents, SIGNAL(triggered()), this, SLOT(OpenHelpContents()));
#else
    // Hide if not Windows
    mUI.mActionHelpContents->setVisible(false);
#endif

    CreateLanguageMenuItems();
    LoadSettings();

    mThread->Initialize(mUI.mResults);
    FormatAndSetTitle();

    EnableCheckButtons(true);

    mUI.mActionClearResults->setEnabled(false);
    mUI.mActionSave->setEnabled(false);
    mUI.mActionRecheck->setEnabled(false);

    QStringList args = QCoreApplication::arguments();
    //Remove the application itself
    args.removeFirst();
    if (!args.isEmpty())
    {
        DoCheckFiles(args);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::CreateLanguageMenuItems()
{
    QStringList languages = mTranslation->GetNames();

    for (int i = 0; i < languages.size(); i++)
    {
        //Create an action for each language
        //Language name is pre translated
        QAction *temp = new QAction(languages[i], this);

        temp->setCheckable(true);

        //Add the action to menu
        mUI.mMenuLanguage->addAction(temp);

        //Add action to the group
        mLanguages->addAction(temp);

        //Check it if it's the value stored to settings
        if (i == mSettings->value(SETTINGS_LANGUAGE, 0).toInt())
        {
            temp->setChecked(true);
        }
        else
        {
            temp->setChecked(false);
        }
    }

    connect(mLanguages, SIGNAL(triggered(QAction *)),
            this, SLOT(MapLanguage(QAction *)));
}

void MainWindow::LoadSettings()
{
    if (mSettings->value(SETTINGS_WINDOW_MAXIMIZED, false).toBool())
    {
        showMaximized();
    }
    else
    {
        resize(mSettings->value(SETTINGS_WINDOW_WIDTH, 800).toInt(),
               mSettings->value(SETTINGS_WINDOW_HEIGHT, 600).toInt());
    }

    mUI.mActionShowAll->setChecked(mSettings->value(SETTINGS_SHOW_ALL, true).toBool());
    mUI.mActionShowStyle->setChecked(mSettings->value(SETTINGS_SHOW_STYLE, true).toBool());
    mUI.mActionShowAllStyle->setChecked(mSettings->value(SETTINGS_SHOW_ALL_STYLE, true).toBool());
    mUI.mActionShowErrors->setChecked(mSettings->value(SETTINGS_SHOW_ERRORS, true).toBool());

    mUI.mResults->ShowResults(SHOW_ALL, mUI.mActionShowAll->isChecked());
    mUI.mResults->ShowResults(SHOW_ERRORS, mUI.mActionShowErrors->isChecked());
    mUI.mResults->ShowResults(SHOW_STYLE, mUI.mActionShowStyle->isChecked());
    mUI.mResults->ShowResults(SHOW_ALL_STYLE, mUI.mActionShowAllStyle->isChecked());

    mUI.mActionToolbar->setChecked(mSettings->value(SETTINGS_TOOLBARS_SHOW, true).toBool());
    mUI.mToolBar->setVisible(mSettings->value(SETTINGS_TOOLBARS_SHOW, true).toBool());

    mApplications->LoadSettings(mSettings);

    SetLanguage(mSettings->value(SETTINGS_LANGUAGE, mTranslation->SuggestLanguage()).toInt());
}

void MainWindow::SaveSettings()
{
    //Force toolbar checkbox value to be updated
    AboutToShowViewMenu();

    mSettings->setValue(SETTINGS_WINDOW_WIDTH, size().width());
    mSettings->setValue(SETTINGS_WINDOW_HEIGHT, size().height());
    mSettings->setValue(SETTINGS_WINDOW_MAXIMIZED, isMaximized());

    mSettings->setValue(SETTINGS_SHOW_ALL, mUI.mActionShowAll->isChecked());
    mSettings->setValue(SETTINGS_SHOW_STYLE, mUI.mActionShowStyle->isChecked());
    mSettings->setValue(SETTINGS_SHOW_ALL_STYLE, mUI.mActionShowAllStyle->isChecked());
    mSettings->setValue(SETTINGS_SHOW_ERRORS, mUI.mActionShowErrors->isChecked());
    mSettings->setValue(SETTINGS_TOOLBARS_SHOW, mUI.mToolBar->isVisible());

    mApplications->SaveSettings(mSettings);

    mSettings->setValue(SETTINGS_LANGUAGE, mTranslation->GetCurrentLanguage());
    mUI.mResults->SaveSettings();
}

void MainWindow::DoCheckFiles(const QStringList &files)
{
    if (files.isEmpty())
    {
        return;
    }
    ClearResults();

    QStringList fileNames;
    QString selection;

    foreach(selection, files)
    {
        fileNames << RemoveUnacceptedFiles(GetFilesRecursively(selection));
    }

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

    mUI.mResults->CheckingStarted();

    mThread->SetFiles(RemoveUnacceptedFiles(fileNames));
    QFileInfo inf(fileNames[0]);
    QString absDirectory = inf.absoluteDir().path();
    mSettings->setValue(SETTINGS_CHECK_PATH, absDirectory);
    EnableCheckButtons(false);
    mUI.mActionSettings->setEnabled(false);

    mUI.mResults->SetCheckDirectory(absDirectory);

    Settings checkSettings = GetCppcheckSettings();
    mThread->Check(checkSettings, false);
}

QStringList MainWindow::SelectFilesToCheck(QFileDialog::FileMode mode)
{
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
    DoCheckFiles(SelectFilesToCheck(QFileDialog::DirectoryOnly));
}

Settings MainWindow::GetCppcheckSettings()
{
    ProjectFile pfile;
    Settings result;

    if (!mCurrentDirectory.isEmpty())
    {
        // Format project filename (directory name + .cppcheck) and load
        // the project file if it is found.
        QStringList parts = mCurrentDirectory.split("/");
        QString projfile = mCurrentDirectory + "/" + parts[parts.count() - 1] + ".cppcheck";
        bool projectRead = false;
        if (QFile::exists(projfile))
        {
            qDebug() << "Reading project file " << projfile;
            projectRead = pfile.Read(projfile);
        }

        if (projectRead)
        {
            QStringList classes = pfile.GetDeAllocatedClasses();
            QString classname;
            foreach(classname, classes)
            {
                result.addAutoAllocClass(classname.toStdString());
            }

            QStringList dirs = pfile.GetIncludeDirs();
            QString dir;
            foreach(dir, dirs)
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
    }

    result._debug = false;
    result._checkCodingStyle = true;
    result._errorsOnly = false;
    result._verbose = true;
    result._force = mSettings->value(SETTINGS_CHECK_FORCE, 1).toBool();
    result._xml = false;
    result._jobs = mSettings->value(SETTINGS_CHECK_THREADS, 1).toInt();

    if (result._jobs <= 0)
    {
        result._jobs = 1;
    }

    return result;
}

QStringList MainWindow::GetFilesRecursively(const QString &path)
{
    QFileInfo info(path);
    QStringList list;

    if (info.isDir())
    {
        QDirIterator it(path, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            list << it.next();
        }
    }
    else
    {
        list << path;
    }

    return list;
}

QStringList MainWindow::RemoveUnacceptedFiles(const QStringList &list)
{
    QStringList result;
    QString str;
    foreach(str, list)
    {
        if (getFileLister()->acceptFile(str.toStdString()))
        {
            result << str;
        }
    }

    return result;
}

void MainWindow::CheckDone()
{
    EnableCheckButtons(true);
    mUI.mActionSettings->setEnabled(true);
    if (mUI.mResults->HasResults())
    {
        mUI.mActionClearResults->setEnabled(true);
        mUI.mActionSave->setEnabled(true);
    }

    // Notify user - if the window is not active - that check is ready
    QApplication::alert(this, 3000);
}

void MainWindow::ProgramSettings()
{
    SettingsDialog dialog(mSettings, mApplications, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        dialog.SaveCheckboxValues();
        mUI.mResults->UpdateSettings(dialog.ShowFullPath(),
                                     dialog.SaveFullPath(),
                                     dialog.SaveAllErrors(),
                                     dialog.ShowNoErrorsMessage());
    }
}

void MainWindow::ReCheck()
{
    ClearResults();
    EnableCheckButtons(false);
    mThread->Check(GetCppcheckSettings(), true);
}

void MainWindow::ClearResults()
{
    mUI.mResults->Clear();
    mUI.mActionClearResults->setEnabled(false);
    mUI.mActionSave->setEnabled(false);
}

void MainWindow::EnableCheckButtons(bool enable)
{
    mUI.mActionStop->setEnabled(!enable);
    mUI.mActionCheckFiles->setEnabled(enable);

    if (!enable || mThread->HasPreviousFiles())
        mUI.mActionRecheck->setEnabled(enable);

    mUI.mActionCheckDirectory->setEnabled(enable);
}

void MainWindow::ShowAll(bool checked)
{
    mUI.mResults->ShowResults(SHOW_ALL, checked);
}

void MainWindow::ShowStyle(bool checked)
{
    mUI.mResults->ShowResults(SHOW_STYLE, checked);
}

void MainWindow::ShowAllStyle(bool checked)
{
    mUI.mResults->ShowResults(SHOW_ALL_STYLE, checked);
}

void MainWindow::ShowErrors(bool checked)
{
    mUI.mResults->ShowResults(SHOW_ERRORS, checked);
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
        QString text(tr("Cannot exit while checking.\n\n" \
                        "Stop the checking before exiting."));

        QMessageBox msg(QMessageBox::Warning,
                        tr("Cppcheck"),
                        text,
                        QMessageBox::Ok,
                        this);

        msg.exec();
        event->ignore();
    }
}

void MainWindow::ToggleAllChecked(bool checked)
{
    mUI.mActionShowAll->setChecked(checked);
    ShowAll(checked);

    mUI.mActionShowStyle->setChecked(checked);
    ShowStyle(checked);

    mUI.mActionShowAllStyle->setChecked(checked);
    ShowStyle(checked);

    mUI.mActionShowErrors->setChecked(checked);
    ShowErrors(checked);
}

void MainWindow::About()
{
    AboutDialog *dlg = new AboutDialog(CppCheck::version(), this);
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
    QString filter(tr("XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)"));
    QString selectedFile = QFileDialog::getSaveFileName(this,
                           tr("Save the report file"),
                           QString(),
                           filter,
                           &selectedFilter);

    if (!selectedFile.isEmpty())
    {
        Report::Type type = Report::TXT;
        if (selectedFilter == tr("XML files (*.xml)"))
        {
            type = Report::XML;
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

void MainWindow::ToggleToolbar()
{
    mUI.mToolBar->setVisible(mUI.mActionToolbar->isChecked());
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


void MainWindow::SetLanguage(int index)
{
    if (mTranslation->GetCurrentLanguage() == index)
    {
        return;
    }

    QString error;
    if (!mTranslation->SetLanguage(index, error))
    {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        QString(tr("Failed to change the language:\n\n%1\n\n")).arg(error),
                        QMessageBox::Ok,
                        this);

        msg.exec();
    }
    else
    {
        //Translate everything that is visible here
        mUI.retranslateUi(this);
        mUI.mResults->Translate();
        QStringList languages = mTranslation->GetNames();
        QList<QAction *> actions = mLanguages->actions();

        if (languages.size() <= actions.size())
        {
            for (int i = 0; i < languages.size(); i++)
            {
                actions[i]->setText(tr(languages[i].toLatin1()));
            }
        }
    }
}

void MainWindow::MapLanguage(QAction *action)
{
    //Find the action that has the language that user clicked
    QList<QAction *> actions = mLanguages->actions();
    for (int i = 0; i < actions.size(); i++)
    {
        if (actions[i] == action)
        {
            SetLanguage(i);
        }
    }
}

void MainWindow::AboutToShowViewMenu()
{
    mUI.mActionToolbar->setChecked(mUI.mToolBar->isVisible());
}

void MainWindow::StopChecking()
{
    mThread->Stop();
    mUI.mResults->DisableProgressbar();
}

void MainWindow::OpenHelpContents()
{
    OpenHtmlHelpContents();
}

void MainWindow::OpenHtmlHelpContents()
{
#ifdef WIN32
    QString file("/cppcheck.chm");
    QString exeFolder = QDir::currentPath();
    exeFolder += file;
    exeFolder = QDir::toNativeSeparators(exeFolder);
    HtmlHelp(NULL, exeFolder.utf16(), HH_DISPLAY_TOPIC, NULL);
#endif // WIN32
}

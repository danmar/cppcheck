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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
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
#include "aboutdialog.h"
#include "threadhandler.h"
#include "fileviewdialog.h"
#include "projectfile.h"
#include "../src/filelister.h"
#include "../src/cppcheckexecutor.h"

MainWindow::MainWindow() :
        mSettings(tr("Cppcheck"), tr("Cppcheck-GUI")),
        mActionExit(tr("E&xit"), this),
        mActionCheckFiles(tr("&Check files(s)"), this),
        mActionClearResults(tr("Clear &results"), this),
        mActionReCheck(tr("Recheck files"), this),
        mActionCheckDirectory(tr("Check &directory"), this),
        mActionSettings(tr("&Settings"), this),
        mActionViewStandardToolbar(tr("Toolbar"), this),
        mActionShowAll(tr("Show possible false positives"), this),
        mActionShowSecurity(tr("Show &security errors"), this),
        mActionShowStyle(tr("Show s&tyle errors"), this),
        mActionShowErrors(tr("Show &common errors"), this),
        mActionShowCheckAll(tr("Show all"), this),
        mActionShowUncheckAll(tr("Hide all"), this),
        mActionShowCollapseAll(tr("Collapse all"), this),
        mActionShowExpandAll(tr("Expand all"), this),
        mActionAbout(tr("About"), this),
        mActionShowLicense(tr("License..."), this),
        mActionShowAuthors(tr("Authors..."), this),
        mActionStop(tr("Stop checking"), this),
        mActionSave(tr("Save results to a file"), this),
        mResults(mSettings, mApplications)
{
    CreateMenus();
    CreateToolbar();
    mThread = new ThreadHandler(this);

    setCentralWidget(&mResults);

    connect(&mActionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(&mActionCheckFiles, SIGNAL(triggered()), this, SLOT(CheckFiles()));
    connect(&mActionCheckDirectory, SIGNAL(triggered()), this, SLOT(CheckDirectory()));
    connect(&mActionSettings, SIGNAL(triggered()), this, SLOT(ProgramSettings()));
    connect(&mActionClearResults, SIGNAL(triggered()), this, SLOT(ClearResults()));

    connect(&mActionViewStandardToolbar, SIGNAL(toggled(bool)), this, SLOT(ViewStandardToolbar(bool)));
    connect(&mActionShowAll, SIGNAL(toggled(bool)), this, SLOT(ShowAll(bool)));
    connect(&mActionShowSecurity, SIGNAL(toggled(bool)), this, SLOT(ShowSecurity(bool)));
    connect(&mActionShowStyle, SIGNAL(toggled(bool)), this, SLOT(ShowStyle(bool)));
    connect(&mActionShowErrors, SIGNAL(toggled(bool)), this, SLOT(ShowErrors(bool)));
    connect(&mActionShowCheckAll, SIGNAL(triggered()), this, SLOT(CheckAll()));
    connect(&mActionShowUncheckAll, SIGNAL(triggered()), this, SLOT(UncheckAll()));
    connect(&mActionShowCollapseAll, SIGNAL(triggered()), &mResults, SLOT(CollapseAllResults()));
    connect(&mActionShowExpandAll, SIGNAL(triggered()), &mResults, SLOT(ExpandAllResults()));

    connect(&mActionReCheck, SIGNAL(triggered()), this, SLOT(ReCheck()));

    connect(&mActionStop, SIGNAL(triggered()), mThread, SLOT(Stop()));
    connect(&mActionSave, SIGNAL(triggered()), this, SLOT(Save()));

    connect(&mActionAbout, SIGNAL(triggered()), this, SLOT(About()));
    connect(&mActionShowLicense, SIGNAL(triggered()), this, SLOT(ShowLicense()));
    connect(&mActionShowAuthors, SIGNAL(triggered()), this, SLOT(ShowAuthors()));

    connect(mThread, SIGNAL(Done()), this, SLOT(CheckDone()));
    connect(&mResults, SIGNAL(GotResults()), this, SLOT(ResultsAdded()));

    mActionCheckDirectory.setIcon(QIcon(":icon.png"));
    mActionReCheck.setIcon(QIcon(":images/view-refresh.png"));
    mActionSettings.setIcon(QIcon(":images/preferences-system.png"));
    mActionAbout.setIcon(QIcon(":images/help-browser.png"));
    mActionStop.setIcon(QIcon(":images/process-stop.png"));
    mActionSave.setIcon(QIcon(":images/media-floppy.png"));
    mActionClearResults.setIcon(QIcon(":images/edit-clear.png"));

    mActionReCheck.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    mActionCheckDirectory.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    mActionSave.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    mActionAbout.setShortcut(QKeySequence(Qt::Key_F1));

    LoadSettings();
    mThread->Initialize(&mResults);
    FormatAndSetTitle();

    EnableCheckButtons(true);

    mActionClearResults.setEnabled(false);
    mActionSave.setEnabled(false);
}

MainWindow::~MainWindow()
{
    SaveSettings();
}

void MainWindow::CreateMenus()
{
    // File-menu
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(&mActionCheckFiles);
    menu->addAction(&mActionCheckDirectory);
    menu->addAction(&mActionReCheck);
    menu->addAction(&mActionStop);
    menu->addAction(&mActionClearResults);
    menu->addAction(&mActionSave);
    menu->addSeparator();
    menu->addAction(&mActionExit);

    // View-menu
    QMenu *menuview = menuBar()->addMenu(tr("&View"));
    menuview->addAction(&mActionViewStandardToolbar);
    menuview->addSeparator();
    menuview->addAction(&mActionShowAll);
    menuview->addAction(&mActionShowSecurity);
    menuview->addAction(&mActionShowStyle);
    menuview->addAction(&mActionShowErrors);
    menuview->addSeparator();
    menuview->addAction(&mActionShowCheckAll);
    menuview->addAction(&mActionShowUncheckAll);
    menuview->addSeparator();
    menuview->addAction(&mActionShowCollapseAll);
    menuview->addAction(&mActionShowExpandAll);

    mActionViewStandardToolbar.setCheckable(true);
    mActionShowAll.setCheckable(true);
    mActionShowSecurity.setCheckable(true);
    mActionShowStyle.setCheckable(true);
    mActionShowErrors.setCheckable(true);

    connect(menuview, SIGNAL(aboutToShow()), this, SLOT(AboutToShowViewMenu()));

    // Program-menu
    QMenu *menuprogram = menuBar()->addMenu(tr("&Program"));
    menuprogram->addAction(&mActionSettings);

    // Help-menu
    QMenu *menuHelp = menuBar()->addMenu(tr("&Help"));
    menuHelp->addAction(&mActionShowLicense);
    menuHelp->addAction(&mActionShowAuthors);
    menuHelp->addSeparator();
    menuHelp->addAction(&mActionAbout);
}

void MainWindow::CreateToolbar()
{
    mStandardToolbar = addToolBar("Standard");
    mStandardToolbar->setIconSize(QSize(22, 22));

    mStandardToolbar->addAction(&mActionCheckDirectory);
    mStandardToolbar->addAction(&mActionSave);
    mStandardToolbar->addAction(&mActionReCheck);
    mStandardToolbar->addAction(&mActionStop);
    mStandardToolbar->addAction(&mActionClearResults);
    mStandardToolbar->addAction(&mActionSettings);
    mStandardToolbar->addAction(&mActionAbout);
}

void MainWindow::LoadSettings()
{
    if (mSettings.value(tr("Window maximized"), false).toBool())
    {
        showMaximized();
    }
    else
    {
        resize(mSettings.value(tr("Window width"), 800).toInt(),
               mSettings.value(tr("Window height"), 600).toInt());
    }

    mActionShowAll.setChecked(mSettings.value(tr("Show all"), true).toBool());
    mActionShowSecurity.setChecked(mSettings.value(tr("Show security"), true).toBool());
    mActionShowStyle.setChecked(mSettings.value(tr("Show style"), true).toBool());
    mActionShowErrors.setChecked(mSettings.value(tr("Show errors"), true).toBool());

    mResults.ShowResults(SHOW_ALL, mActionShowAll.isChecked());
    mResults.ShowResults(SHOW_ERRORS, mActionShowErrors.isChecked());
    mResults.ShowResults(SHOW_SECURITY, mActionShowSecurity.isChecked());
    mResults.ShowResults(SHOW_STYLE, mActionShowStyle.isChecked());
    mApplications.LoadSettings(mSettings);
}

void MainWindow::SaveSettings()
{
    mSettings.setValue(tr("Window width"), size().width());
    mSettings.setValue(tr("Window height"), size().height());
    mSettings.setValue(tr("Window maximized"), isMaximized());

    mSettings.setValue(tr("Show all"), mActionShowAll.isChecked());
    mSettings.setValue(tr("Show security"), mActionShowSecurity.isChecked());
    mSettings.setValue(tr("Show style"), mActionShowStyle.isChecked());
    mSettings.setValue(tr("Show errors"), mActionShowErrors.isChecked());
    mApplications.SaveSettings(mSettings);
}

void MainWindow::DoCheckFiles(QFileDialog::FileMode mode)
{
    QStringList selected;

    // NOTE: we use QFileDialog::getOpenFileNames() and
    // QFileDialog::getExistingDirectory() because they show native Windows
    // selection dialog which is a lot more usable than QT:s own dialog.
    if (mode == QFileDialog::ExistingFiles)
    {
        selected = QFileDialog::getOpenFileNames(this,
                   tr("Select files to check"),
                   mSettings.value(tr("Check path"), "").toString());
        if (selected.isEmpty())
            mCurrentDirectory.clear();
        FormatAndSetTitle();
    }
    else if (mode == QFileDialog::DirectoryOnly)
    {
        QString dir = QFileDialog::getExistingDirectory(this,
                      tr("Select directory to check"),
                      mSettings.value(tr("Check path"), "").toString());
        if (!dir.isEmpty())
        {
            mCurrentDirectory = dir;
            selected.append(dir);
            FormatAndSetTitle(dir);
        }
    }

    if (selected.count() > 0)
    {
        ClearResults();

        QStringList fileNames;
        QString selection;

        foreach(selection, selected)
        {
            fileNames << RemoveUnacceptedFiles(GetFilesRecursively(selection));
        }

        mResults.Clear();
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

        mResults.CheckingStarted();

        mThread->SetFiles(RemoveUnacceptedFiles(fileNames));
        QFileInfo inf(fileNames[0]);
        QString absDirectory = inf.absoluteDir().path();
        mSettings.setValue(tr("Check path"), absDirectory);
        EnableCheckButtons(false);
        mActionSettings.setEnabled(false);
        mResults.SetCheckDirectory(absDirectory);

        Settings checkSettings = GetCppcheckSettings();
        mThread->Check(checkSettings, false);
    }
}

void MainWindow::CheckFiles()
{
    DoCheckFiles(QFileDialog::ExistingFiles);
}

void MainWindow::CheckDirectory()
{
    DoCheckFiles(QFileDialog::DirectoryOnly);
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
        QString projfile = parts[parts.count() - 1] + ".cppcheck";
        bool projectRead = false;
        if (QFile::exists(projfile))
            projectRead = pfile.Read(mCurrentDirectory + "/" + projfile);

        if (projectRead)
        {
            QStringList classes = pfile.GetDeAllocatedClasses();
            QString classname;
            foreach(classname, classes)
            {
                result.addAutoAllocClass(classname.toStdString());
            }
        }
    }

    result._debug = false;
    result._showAll = true;
    result._checkCodingStyle = true;
    result._errorsOnly = false;
    result._verbose = true;
    result._force = mSettings.value(tr("Check force"), 1).toBool();
    result._xml = false;
    result._unusedFunctions = false;
    result._security = true;
    result._jobs = mSettings.value(tr("Check threads"), 1).toInt();

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
        if (FileLister::AcceptFile(str.toStdString()))
        {
            result << str;
        }
    }

    return result;
}

void MainWindow::CheckDone()
{
    EnableCheckButtons(true);
    mActionSettings.setEnabled(true);
    if (mResults.HasResults())
    {
        mActionClearResults.setEnabled(true);
        mActionSave.setEnabled(true);
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
        mResults.UpdateSettings(dialog.ShowFullPath(),
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
    mResults.Clear();
    mActionClearResults.setEnabled(false);
    mActionSave.setEnabled(false);
}

void MainWindow::EnableCheckButtons(bool enable)
{
    mActionStop.setEnabled(!enable);
    mActionCheckFiles.setEnabled(enable);
    mActionReCheck.setEnabled(enable);
    mActionCheckDirectory.setEnabled(enable);
}


void MainWindow::ShowAll(bool checked)
{
    mResults.ShowResults(SHOW_ALL, checked);
}

void MainWindow::ShowSecurity(bool checked)
{
    mResults.ShowResults(SHOW_SECURITY, checked);
}

void MainWindow::ShowStyle(bool checked)
{
    mResults.ShowResults(SHOW_STYLE, checked);
}

void MainWindow::ShowErrors(bool checked)
{
    mResults.ShowResults(SHOW_ERRORS, checked);
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
        event->accept();
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
    mActionShowAll.setChecked(checked);
    ShowAll(checked);

    mActionShowSecurity.setChecked(checked);
    ShowSecurity(checked);

    mActionShowStyle.setChecked(checked);
    ShowStyle(checked);

    mActionShowErrors.setChecked(checked);
    ShowErrors(checked);
}

void MainWindow::About()
{
    //TODO make a "GetVersionNumber" function to core cppcheck
    CppCheckExecutor exec;
    CppCheck check(exec);
    const char *argv[] = {"", "--version"};
    QString version = check.parseFromArgs(2, argv).c_str();
    version.replace("Cppcheck ", "");

    AboutDialog *dlg = new AboutDialog(version, this);
    dlg->exec();
}

void MainWindow::ShowLicense()
{
    FileViewDialog *dlg = new FileViewDialog(":COPYING", this);
    dlg->resize(550, 400);
    dlg->exec();
}

void MainWindow::ShowAuthors()
{
    FileViewDialog *dlg = new FileViewDialog(":AUTHORS", this);
    dlg->resize(350, 400);
    dlg->exec();
}

void MainWindow::Save()
{
    QString selectedFilter;
    QString filter(tr("XML files (*.xml);;Text files (*.txt)"));
    QString selectedFile = QFileDialog::getSaveFileName(this,
                           tr("Save the report file"),
                           QString(),
                           filter,
                           &selectedFilter);

    if (!selectedFile.isEmpty())
    {
        //Check if xml file type was selected
        bool xml = selectedFilter == tr("XML files (*.xml)");

        //Force xml extension to the file
        if (xml && !selectedFile.endsWith(".xml", Qt::CaseInsensitive))
        {
            selectedFile += ".xml";
        }

        //Force .txt extension
        if (!xml && !selectedFile.endsWith(".txt", Qt::CaseInsensitive))
        {
            selectedFile += ".txt";
        }

        mResults.Save(selectedFile, xml);
    }
}

void MainWindow::ResultsAdded()
{
}

void MainWindow::ViewStandardToolbar(bool view)
{
    if (view)
        mStandardToolbar->show();
    else
        mStandardToolbar->hide();
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

void MainWindow::AboutToShowViewMenu()
{
    if (mStandardToolbar->isVisible())
        mActionViewStandardToolbar.setChecked(true);
    else
        mActionViewStandardToolbar.setChecked(false);
}

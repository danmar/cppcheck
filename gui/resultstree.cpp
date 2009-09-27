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

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QSignalMapper>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QFileInfo>
#include <QClipboard>
#include "resultstree.h"
#include "xmlreport.h"

ResultsTree::ResultsTree(QWidget * parent) :
        QTreeView(parent),
        mContextItem(0),
        mCheckPath(""),
        mVisibleErrors(false)
{
    setModel(&mModel);
    QStringList labels;
    labels << tr("File") << tr("Severity") << tr("Line") << tr("Message");
    mModel.setHorizontalHeaderLabels(labels);
    setExpandsOnDoubleClick(false);
    setSortingEnabled(true);

    connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(QuickStartApplication(const QModelIndex &)));
}

ResultsTree::~ResultsTree()
{
}

void ResultsTree::Initialize(QSettings *settings, ApplicationList *list)
{
    mSettings = settings;
    mApplications = list;
    LoadSettings();
}


QStandardItem *ResultsTree::CreateItem(const QString &name)
{
    QStandardItem *item = new QStandardItem(name);
    item->setEditable(false);
    return item;
}

void ResultsTree::AddErrorItem(const QString &file,
                               const QString &severity,
                               const QString &message,
                               const QStringList &files,
                               const QVariantList &lines,
                               const QString &id)
{
    Q_UNUSED(file);

    if (files.isEmpty())
    {
        return;
    }

    QString realfile = StripPath(files[0], false);

    if (realfile.isEmpty())
    {
        realfile = tr("Undefined file");
    }

    bool hide = !mShowTypes[SeverityToShowType(severity)];

    //if there is at least on error that is not hidden, we have a visible error
    if (!hide)
    {
        mVisibleErrors = true;
    }

    //Create the base item for the error and ensure it has a proper
    //file item as a parent
    QStandardItem *item = AddBacktraceFiles(EnsureFileItem(files[0], hide),
                                            realfile,
                                            lines[0].toInt(),
                                            severity,
                                            message,
                                            hide,
                                            SeverityToIcon(severity));


    //Add user data to that item
    QMap<QString, QVariant> data;
    data["severity"]  = SeverityToShowType(severity);
    data["message"]  = message;
    data["files"]  = files;
    data["lines"]  = lines;
    data["id"]  = id;
    item->setData(QVariant(data));

    //Add backtrace files as children
    for (int i = 1; i < files.size() && i < lines.size(); i++)
    {
        AddBacktraceFiles(item,
                          StripPath(files[i], false),
                          lines[i].toInt(),
                          severity,
                          message,
                          hide,
                          ":images/go-down.png");
    }

    //TODO just hide/show current error and it's file
    //since this does a lot of unnecessary work
    if (!hide)
    {
        ShowFileItem(realfile);
    }
}

QStandardItem *ResultsTree::AddBacktraceFiles(QStandardItem *parent,
        const QString &file,
        const int line,
        const QString &severity,
        const QString &message,
        const bool hide,
        const QString &icon)

{
    if (!parent)
    {
        return 0;
    }

    QList<QStandardItem*> list;
    list << CreateItem(file);
    list << CreateItem(tr(severity.toLatin1()));
    list << CreateItem(QString("%1").arg(line));
    //TODO message has parameter names so we'll need changes to the core
    //cppcheck so we can get proper translations
    list << CreateItem(tr(message.toLatin1()));

    QModelIndex index = QModelIndex();

    parent->appendRow(list);

    setRowHidden(parent->rowCount() - 1, parent->index(), hide);

    if (!icon.isEmpty())
    {
        list[0]->setIcon(QIcon(icon));
    }

    //TODO Does this leak memory? Should items from list be deleted?

    return list[0];
}

ShowTypes ResultsTree::VariantToShowType(const QVariant &data)
{
    int value = data.toInt();
    if (value < SHOW_ALL && value > SHOW_ERRORS)
    {
        return SHOW_NONE;
    }
    return (ShowTypes)value;
}

ShowTypes ResultsTree::SeverityToShowType(const QString & severity)
{
    if (severity == "possible error")
        return SHOW_ALL;
    if (severity == "error")
        return SHOW_ERRORS;
    if (severity == "style")
        return SHOW_STYLE;
    if (severity == "possible style")
        return SHOW_ALL_STYLE;

    return SHOW_NONE;
}

QStandardItem *ResultsTree::FindFileItem(const QString &name)
{
    QList<QStandardItem *> list = mModel.findItems(name);
    if (list.size() > 0)
    {
        return list[0];
    }
    return 0;
}

void ResultsTree::Clear()
{
    mModel.removeRows(0, mModel.rowCount());
}

void ResultsTree::LoadSettings()
{
    for (int i = 0; i < mModel.columnCount(); i++)
    {
        //mFileTree.columnWidth(i);
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        setColumnWidth(i, mSettings->value(temp, 800 / mModel.columnCount()).toInt());
    }

    mSaveFullPath = mSettings->value(SETTINGS_SAVE_FULL_PATH, false).toBool();
    mSaveAllErrors = mSettings->value(SETTINGS_SAVE_ALL_ERRORS, false).toBool();
    mShowFullPath = mSettings->value(SETTINGS_SHOW_FULL_PATH, false).toBool();
}

void ResultsTree::SaveSettings()
{
    for (int i = 0; i < mModel.columnCount(); i++)
    {
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        mSettings->setValue(temp, columnWidth(i));
    }
}

void ResultsTree::ShowResults(ShowTypes type, bool show)
{
    if (type != SHOW_NONE && mShowTypes[type] != show)
    {
        mShowTypes[type] = show;
        RefreshTree();
    }
}


void ResultsTree::RefreshTree()
{
    mVisibleErrors = false;
    //Get the amount of files in the tree
    int filecount = mModel.rowCount();

    for (int i = 0; i < filecount; i++)
    {
        //Get file i
        QStandardItem *file = mModel.item(i, 0);
        if (!file)
        {
            continue;
        }

        //Get the amount of errors this file contains
        int errorcount = file->rowCount();

        //By default it shouldn't be visible
        bool show = false;

        for (int j = 0; j < errorcount; j++)
        {
            //Get the error itself
            QStandardItem *child = file->child(j, 0);
            if (!child)
            {
                continue;
            }

            //Get error's user data
            QVariant userdata = child->data();
            //Convert it to QVariantMap
            QVariantMap data = userdata.toMap();

            //Check if this error should be hidden
            bool hide = !mShowTypes[VariantToShowType(data["severity"])];

            if (!hide)
            {
                mVisibleErrors = true;
            }

            //Hide/show accordingly
            setRowHidden(j, file->index(), hide);

            //If it was shown then the file itself has to be shown aswell
            if (!hide)
            {
                show = true;
            }
        }

        //Show the file if any of it's errors are visible
        setRowHidden(i, QModelIndex(), !show);
    }
}

QStandardItem *ResultsTree::EnsureFileItem(const QString &fullpath, bool hide)
{
    QString name = StripPath(fullpath, false);
    QStandardItem *item = FindFileItem(name);

    if (item)
    {
        return item;
    }

    item = CreateItem(name);
    item->setIcon(QIcon(":images/text-x-generic.png"));

    //Add user data to that item
    QMap<QString, QVariant> data;
    data["files"] = fullpath;
    item->setData(QVariant(data));
    mModel.appendRow(item);

    setRowHidden(mModel.rowCount() - 1, QModelIndex(), hide);

    return item;
}

void ResultsTree::ShowFileItem(const QString &name)
{
    QStandardItem *item = FindFileItem(name);
    if (item)
    {
        setRowHidden(0, mModel.indexFromItem(item), false);
    }
}

void ResultsTree::contextMenuEvent(QContextMenuEvent * e)
{
    QModelIndex index = indexAt(e->pos());
    if (index.isValid())
    {
        mContextItem = mModel.itemFromIndex(index);

        //Create a new context menu
        QMenu menu(this);

        //Store all applications in a list
        QList<QAction*> actions;

        //Create a signal mapper so we don't have to store data to class
        //member variables
        QSignalMapper *signalMapper = new QSignalMapper(this);

        if (mContextItem && mApplications->GetApplicationCount() > 0 && mContextItem->parent())
        {
            //Go through all applications and add them to the context menu
            for (int i = 0; i < mApplications->GetApplicationCount(); i++)
            {
                //Create an action for the application
                QAction *start = new QAction(mApplications->GetApplicationName(i), &menu);

                //Add it to our list so we can disconnect later on
                actions << start;

                //Add it to context menu
                menu.addAction(start);

                //Connect the signal to signal mapper
                connect(start, SIGNAL(triggered()), signalMapper, SLOT(map()));

                //Add a new mapping
                signalMapper->setMapping(start, i);
            }

            connect(signalMapper, SIGNAL(mapped(int)),
                    this, SLOT(Context(int)));
        }

        // Add menuitems to copy full path/filename to clipboard
        if (mContextItem)
        {
            if (mApplications->GetApplicationCount() > 0)
            {
                menu.addSeparator();
            }

            //Create an action for the application
            QAction *copyfilename = new QAction(tr("Copy filename"), &menu);
            QAction *copypath = new QAction(tr("Copy full path"), &menu);

            menu.addAction(copyfilename);
            menu.addAction(copypath);

            connect(copyfilename, SIGNAL(triggered()), this, SLOT(CopyFilename()));
            connect(copypath, SIGNAL(triggered()), this, SLOT(CopyFullPath()));
        }

        //Start the menu
        menu.exec(e->globalPos());

        if (mContextItem && mApplications->GetApplicationCount() > 0 && mContextItem->parent())
        {
            //Disconnect all signals
            for (int i = 0; i < actions.size(); i++)
            {

                disconnect(actions[i], SIGNAL(triggered()), signalMapper, SLOT(map()));
            }

            disconnect(signalMapper, SIGNAL(mapped(int)),
                       this, SLOT(Context(int)));
            //And remove the signal mapper
            delete signalMapper;
        }

    }
}

void ResultsTree::StartApplication(QStandardItem *target, int application)
{
    //If there are now application's specified, tell the user about it
    if (mApplications->GetApplicationCount() == 0)
    {
        QMessageBox msg(QMessageBox::Information,
                        tr("Cppcheck"),
                        tr("Configure the text file viewer program in Cppcheck preferences/Applications."),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    if (target && application >= 0 && application < mApplications->GetApplicationCount() && target->parent())
    {
        QVariantMap data = target->data().toMap();

        QString program = mApplications->GetApplicationPath(application);

        //TODO Check which line was actually right clicked, now defaults to 0
        unsigned int index = 0;

        //Replace (file) with filename
        QStringList files = data["files"].toStringList();
        if (files.size() > 0)
        {
            program.replace("(file)", files[index], Qt::CaseInsensitive);
        }
        else
        {
            qDebug("Failed to get filename!");
        }


        QVariantList lines = data["lines"].toList();
        if (lines.size() > 0)
        {
            program.replace("(line)", QString("%1").arg(lines[index].toInt()), Qt::CaseInsensitive);
        }
        else
        {
            qDebug("Failed to get filenumber!");
        }

        program.replace("(message)", data["message"].toString(), Qt::CaseInsensitive);
        program.replace("(severity)", data["severity"].toString(), Qt::CaseInsensitive);

        bool success = QProcess::startDetached(program);
        if (!success)
        {
            QString app = mApplications->GetApplicationName(application);
            QString text = tr("Could not start %1\n\nPlease check the application path and parameters are correct.").arg(app);

            QMessageBox msgbox(this);
            msgbox.setWindowTitle("Cppcheck");
            msgbox.setText(text);
            msgbox.setIcon(QMessageBox::Critical);

            msgbox.exec();
        }
    }
}

void ResultsTree::CopyFilename()
{
    CopyPath(mContextItem, false);
}

void ResultsTree::CopyFullPath()
{
    CopyPath(mContextItem, true);
}

void ResultsTree::Context(int application)
{
    StartApplication(mContextItem, application);
}

void ResultsTree::QuickStartApplication(const QModelIndex &index)
{
    StartApplication(mModel.itemFromIndex(index), 0);
}

void ResultsTree::CopyPath(QStandardItem *target, bool fullPath)
{
    if (target)
    {
        QVariantMap data = target->data().toMap();
        QString pathStr;

        //Replace (file) with filename
        QStringList files = data["files"].toStringList();
        if (files.size() > 0)
        {
            pathStr = files[0];
            if (!fullPath)
            {
                QFileInfo fi(pathStr);
                pathStr = fi.fileName();
            }
        }
        else
        {
            qDebug("Failed to get filename!");
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(pathStr);
    }
}

QString ResultsTree::SeverityToIcon(const QString &severity)
{
    if (severity == "possible error")
        return ":images/dialog-warning.png";
    if (severity == "error")
        return ":images/dialog-error.png";
    if (severity == "style" || severity == "possible style")
        return ":images/dialog-information.png";

    return "";
}

void ResultsTree::SaveResults(Report *report)
{
    report->WriteHeader();

    for (int i = 0; i < mModel.rowCount(); i++)
    {
        QStandardItem *item = mModel.item(i, 0);
        if (!isRowHidden(i, item->index()))
            SaveErrors(report, item);
    }

    report->WriteFooter();
}

void ResultsTree::SaveErrors(Report *report, QStandardItem *item)
{
    if (!item)
    {
        return;
    }

    //qDebug() << item->text() << "has" << item->rowCount() << "errors";

    for (int i = 0; i < item->rowCount(); i++)
    {
        QStandardItem *error = item->child(i, 0);

        if (!error)
        {
            continue;
        }

        if (isRowHidden(i, item->index()) && !mSaveAllErrors)
        {
            continue;
        }

        //Get error's user data
        QVariant userdata = error->data();
        //Convert it to QVariantMap
        QVariantMap data = userdata.toMap();

        QString line;
        QString severity = ShowTypeToString(VariantToShowType(data["severity"]));
        QString message = data["message"].toString();
        QString id = data["id"].toString();
        QStringList files = data["files"].toStringList();
        QVariantList lines = data["lines"].toList();

        if (files.size() <= 0 || lines.size() <= 0 || lines.size() != files.size())
        {
            continue;
        }

        for (int i = 0; i < files.count(); i++)
            files[i] = StripPath(files[i], true);

        QStringList linesStr;
        for (int i = 0; i < lines.count(); i++)
            linesStr << lines[i].toString();

        report->WriteError(files, linesStr, id, severity, message);
    }
}

QString ResultsTree::ShowTypeToString(ShowTypes type)
{
    switch (type)
    {
    case SHOW_ALL:
        return tr("possible error");
        break;

    case SHOW_STYLE:
        return tr("style");
        break;

    case SHOW_ALL_STYLE:
        return tr("possible style");
        break;

    case SHOW_ERRORS:
        return tr("error");
        break;

    case SHOW_NONE:
        return "";
        break;
    }

    return "";
}

void ResultsTree::UpdateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors)
{
    if (mShowFullPath != showFullPath)
    {
        mShowFullPath = showFullPath;
        RefreshFilePaths();
    }

    mSaveFullPath = saveFullPath;
    mSaveAllErrors = saveAllErrors;
}

void ResultsTree::SetCheckDirectory(const QString &dir)
{
    mCheckPath = dir;
}

QString ResultsTree::StripPath(const QString &path, bool saving)
{
    if ((!saving && mShowFullPath) || (saving && mSaveFullPath))
    {
        return QString(path);
    }

    QDir dir(mCheckPath);
    return dir.relativeFilePath(path);
}

void ResultsTree::RefreshFilePaths(QStandardItem *item)
{
    if (!item)
    {
        return;
    }

    //Mark that this file's path hasn't been updated yet
    bool updated = false;

    //Loop through all errors within this file
    for (int i = 0; i < item->rowCount(); i++)
    {
        //Get error i
        QStandardItem *error = item->child(i, 0);

        if (!error)
        {
            continue;
        }

        //Get error's user data
        QVariant userdata = error->data();
        //Convert it to QVariantMap
        QVariantMap data = userdata.toMap();

        //Get list of files
        QStringList files = data["files"].toStringList();

        //We should always have at least 1 file per error
        if (files.size() == 0)
        {
            continue;
        }

        //Update this error's text
        error->setText(StripPath(files[0], false));

        //If this error has backtraces make sure the files list has enough filenames
        if (error->rowCount() <= files.size() - 1)
        {
            //Loop through all files within the error
            for (int j = 0; j < error->rowCount(); j++)
            {
                //Get file
                QStandardItem *file = error->child(j, 0);
                if (!file)
                {
                    continue;
                }
                //Update file's path
                file->setText(StripPath(files[j+1], false));
            }
        }

        //if the main file hasn't been updated yet, update it now
        if (!updated)
        {
            updated = true;
            item->setText(error->text());
        }

    }
}

void ResultsTree::RefreshFilePaths()
{
    qDebug("Refreshing file paths");

    //Go through all file items (these are parent items that contain the errors)
    for (int i = 0; i < mModel.rowCount(); i++)
    {
        RefreshFilePaths(mModel.item(i, 0));
    }
}

bool ResultsTree::HasVisibleResults() const
{
    return mVisibleErrors;
}

bool ResultsTree::HasResults() const
{
    return mModel.rowCount() > 0;
}

void ResultsTree::Translate()
{
    QStringList labels;
    labels << tr("File") << tr("Severity") << tr("Line") << tr("Message");
    mModel.setHorizontalHeaderLabels(labels);
    //TODO go through all the errors in the tree and translate severity and message
}


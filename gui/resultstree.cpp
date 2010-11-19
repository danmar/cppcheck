/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#include <QWidget>
#include <QStandardItem>
#include <QModelIndex>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QMenu>
#include <QSignalMapper>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QAction>
#include <QFileInfo>
#include <QFileDialog>
#include <QClipboard>
#include <QContextMenuEvent>
#include "erroritem.h"
#include "settings.h"
#include "applicationlist.h"
#include "resultstree.h"
#include "report.h"
#include "xmlreport.h"

ResultsTree::ResultsTree(QWidget * parent) :
    QTreeView(parent),
    mContextItem(0),
    mVisibleErrors(false)
{
    for (int i = 0; i < SHOW_NONE; i++)
        mShowTypes[i] = false;

    setModel(&mModel);
    QStringList labels;
    labels << tr("File") << tr("Severity") << tr("Line") << tr("Summary") << tr("Message");
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


QStandardItem *ResultsTree::CreateNormalItem(const QString &name)
{
    QStandardItem *item = new QStandardItem(name);
    item->setData(name, Qt::ToolTipRole);
    item->setEditable(false);
    return item;
}

QStandardItem *ResultsTree::CreateLineNumberItem(const QString &linenumber)
{
    QStandardItem *item = new QStandardItem(linenumber);
    item->setData(linenumber, Qt::ToolTipRole);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    item->setEditable(false);
    return item;
}

void ResultsTree::AddErrorItem(const ErrorItem &item)
{
    if (item.files.isEmpty())
    {
        return;
    }

    QString realfile = StripPath(item.files[0], false);

    if (realfile.isEmpty())
    {
        realfile = tr("Undefined file");
    }

    bool hide = !mShowTypes[SeverityToShowType(item.severity)];

    //if there is at least one error that is not hidden, we have a visible error
    if (!hide)
    {
        mVisibleErrors = true;
    }

    ErrorLine line;
    line.file = realfile;
    line.id = item.id;
    line.line = item.lines[0];
    line.summary = item.summary;
    line.message = item.message;
    line.severity = item.severity;
    //Create the base item for the error and ensure it has a proper
    //file item as a parent
    QStandardItem *stditem = AddBacktraceFiles(EnsureFileItem(line.file, hide),
                             line,
                             hide,
                             SeverityToIcon(line.severity));

    if (!stditem)
        return;

    //Add user data to that item
    QMap<QString, QVariant> data;
    data["severity"]  = SeverityToShowType(item.severity);
    data["summary"] = item.summary;
    data["message"]  = item.message;
    data["file"]  = item.files[0];
    data["line"]  = item.lines[0];
    data["id"]  = item.id;
    stditem->setData(QVariant(data));

    //Add backtrace files as children
    for (int i = 1; i < item.files.size() && i < item.lines.size(); i++)
    {
        line.file = StripPath(item.files[i], false);
        line.line = item.lines[i];
        QStandardItem *child_item;
        child_item = AddBacktraceFiles(stditem,
                                       line,
                                       hide,
                                       ":images/go-down.png");

        //Add user data to that item
        QMap<QString, QVariant> child_data;
        child_data["severity"]  = SeverityToShowType(line.severity);
        child_data["summary"] = line.summary;
        child_data["message"]  = line.message;
        child_data["file"]  = item.files[i];
        child_data["line"]  = line.line;
        child_data["id"]  = line.id;
        child_item->setData(QVariant(child_data));
    }

    //TODO just hide/show current error and it's file
    //since this does a lot of unnecessary work
    if (!hide)
    {
        ShowFileItem(realfile);
    }
}

QStandardItem *ResultsTree::AddBacktraceFiles(QStandardItem *parent,
        const ErrorLine &item,
        const bool hide,
        const QString &icon)

{
    if (!parent)
    {
        return 0;
    }

    QList<QStandardItem*> list;
    // Ensure shown path is with native separators
    const QString file = QDir::toNativeSeparators(item.file);
    list << CreateNormalItem(file);
    list << CreateNormalItem(tr(item.severity.toLatin1()));
    list << CreateLineNumberItem(QString("%1").arg(item.line));
    //TODO message has parameter names so we'll need changes to the core
    //cppcheck so we can get proper translations
    list << CreateNormalItem(tr(item.summary.toLatin1()));
    list << CreateNormalItem(tr(item.message.toLatin1()));

    // Check for duplicate rows and don't add them if found
    for (int i = 0; i < parent->rowCount(); i++)
    {
        // The first column is the file name and is always the same so
        // we skip it in other platforms than Windows, where filename case
        // is ignored. So in Windows we can get filenames "header.h" and
        // "Header.h" and must compare them as identical filenames.

        // the third column is the line number so check it first
        if (parent->child(i, 2)->text() == list[2]->text())
        {
            // the second column is the severity so check it next
            if (parent->child(i, 1)->text() == list[1]->text())
            {
                // the fourth column is the summary so check it last
                if (parent->child(i, 3)->text() == list[3]->text())
                {

                    // the fifth column is the message so check it last
                    if (parent->child(i, 4)->text() == list[4]->text())
                    {
#if defined(_WIN32)
                        const QString first = parent->child(i, 0)->text().toLower();
                        const QString second = list[0]->text().toLower();
                        if (first == second)
                            return 0;
#else
                        // this row matches so don't add it
                        return 0;
#endif // _WIN32
                    }
                }
            }
        }
    }

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
    if (value < SHOW_STYLE && value > SHOW_ERRORS)
    {
        return SHOW_NONE;
    }
    return (ShowTypes)value;
}

ShowTypes ResultsTree::SeverityToShowType(const QString & severity) const
{
    if (severity == "error")
        return SHOW_ERRORS;
    if (severity == "style")
        return SHOW_STYLE;
    if (severity == "warning")
        return SHOW_WARNINGS;
    if (severity == "performance")
        return SHOW_PERFORMANCE;

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

            //If it was shown then the file itself has to be shown as well
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
    // Since item has path with native separators we must use path with
    // native separators to find it.
    QStandardItem *item = FindFileItem(QDir::toNativeSeparators(name));

    if (item)
    {
        return item;
    }

    // Ensure shown path is with native separators
    name = QDir::toNativeSeparators(name);
    item = CreateNormalItem(name);
    item->setIcon(QIcon(":images/text-x-generic.png"));

    //Add user data to that item
    QMap<QString, QVariant> data;
    data["file"] = fullpath;
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
            QAction *copyfilename 	= new QAction(tr("Copy filename"), &menu);
            QAction *copypath 		= new QAction(tr("Copy full path"), &menu);
            QAction *copymessage 	= new QAction(tr("Copy message"), &menu);

            menu.addAction(copyfilename);
            menu.addAction(copypath);
            menu.addAction(copymessage);

            connect(copyfilename, SIGNAL(triggered()), this, SLOT(CopyFilename()));
            connect(copypath, SIGNAL(triggered()), this, SLOT(CopyFullPath()));
            connect(copymessage, SIGNAL(triggered()), this, SLOT(CopyMessage()));
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
    //If there are no applications specified, tell the user about it
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
        // Make sure we are working with the first column
        if (target->column() != 0)
            target = target->parent()->child(target->row(), 0);

        QVariantMap data = target->data().toMap();

        QString program = mApplications->GetApplicationPath(application);

        //Replace (file) with filename
        QString file = data["file"].toString();

        QFileInfo info(file);
        if (!info.exists())
        {
            if (info.isAbsolute())
            {
                QMessageBox msgbox(this);
                msgbox.setWindowTitle("Cppcheck");
                msgbox.setText(tr("Could not find the file!"));
                msgbox.setIcon(QMessageBox::Critical);
                msgbox.exec();
            }
            else
            {
                QDir checkdir(mCheckPath);
                if (checkdir.isAbsolute() && checkdir.exists())
                {
                    file = mCheckPath + "/" + file;
                }
                else
                {
                    QString dir = AskFileDir(file);
                    dir += '/';
                    file = dir + file;
                }
            }
        }

        if (file.indexOf(" ") > -1)
        {
            file.insert(0, "\"");
            file.append("\"");
        }

        program.replace("(file)", file, Qt::CaseInsensitive);

        QVariant line = data["line"];
        program.replace("(line)", QString("%1").arg(line.toInt()), Qt::CaseInsensitive);

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

QString ResultsTree::AskFileDir(const QString &file)
{
    QString text = tr("Could not find file:\n%1\nPlease select the directory where file is located.").arg(file);
    QMessageBox msgbox(this);
    msgbox.setWindowTitle("Cppcheck");
    msgbox.setText(text);
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.exec();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                  "",
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    mCheckPath = dir;
    return dir;
}

void ResultsTree::CopyFilename()
{
    CopyPath(mContextItem, false);
}

void ResultsTree::CopyFullPath()
{
    CopyPath(mContextItem, true);
}

void ResultsTree::CopyMessage()
{
    if (mContextItem)
    {
        // Make sure we are working with the first column
        if (mContextItem->column() != 0)
            mContextItem = mContextItem->parent()->child(mContextItem->row(), 0);

        QVariantMap data = mContextItem->data().toMap();

        QString message = data["message"].toString();

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(message);
    }
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
        // Make sure we are working with the first column
        if (target->column() != 0)
            target = target->parent()->child(target->row(), 0);

        QVariantMap data = target->data().toMap();
        QString pathStr;

        //Replace (file) with filename
        QString file = data["file"].toString();
        pathStr = file;
        if (!fullPath)
        {
            QFileInfo fi(pathStr);
            pathStr = fi.fileName();
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(pathStr);
    }
}

QString ResultsTree::SeverityToIcon(const QString &severity) const
{
    if (severity == "error")
        return ":images/dialog-error.png";
    if (severity == "style")
        return ":images/dialog-information.png";
    if (severity == "warning")
        return ":images/dialog-warning.png";
    if (severity == "performance")
        return ":images/utilities-system-monitor.png";

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

        ErrorItem item;
        item.severity = ShowTypeToString(VariantToShowType(data["severity"]));
        item.summary = data["summary"].toString();
        item.message = data["message"].toString();
        item.id = data["id"].toString();
        QString file = StripPath(data["file"].toString(), true);
        unsigned int line = data["line"].toUInt();

        item.files << file;
        item.lines << line;

        for (int j = 0; j < error->rowCount(); j++)
        {
            QStandardItem *child_error = error->child(j, 0);
            //Get error's user data
            QVariant child_userdata = child_error->data();
            //Convert it to QVariantMap
            QVariantMap child_data = child_userdata.toMap();

            file = StripPath(child_data["file"].toString(), true);
            line = child_data["line"].toUInt();

            item.files << file;
            item.lines << line;
        }

        report->WriteError(item);
    }
}

QString ResultsTree::ShowTypeToString(ShowTypes type)
{
    switch (type)
    {
    case SHOW_STYLE:
        return tr("style");
        break;

    case SHOW_ERRORS:
        return tr("error");
        break;

    case SHOW_WARNINGS:
        return tr("warning");
        break;

    case SHOW_PERFORMANCE:
        return tr("performance");
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
        QString file = data["file"].toString();

        //Update this error's text
        error->setText(StripPath(file, false));

        //If this error has backtraces make sure the files list has enough filenames
        if (error->hasChildren())
        {
            //Loop through all files within the error
            for (int j = 0; j < error->rowCount(); j++)
            {
                //Get file
                QStandardItem *child = error->child(j, 0);
                if (!child)
                {
                    continue;
                }
                //Get childs's user data
                QVariant child_userdata = child->data();
                //Convert it to QVariantMap
                QVariantMap child_data = child_userdata.toMap();

                //Get list of files
                QString child_files = child_data["file"].toString();
                //Update file's path
                child->setText(StripPath(child_files, false));
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
    labels << tr("File") << tr("Severity") << tr("Line") << tr("Summary") << tr("Message");
    mModel.setHorizontalHeaderLabels(labels);
    //TODO go through all the errors in the tree and translate severity and message
}


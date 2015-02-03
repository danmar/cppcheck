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
#include <QWidget>
#include <QStandardItem>
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
#include <QModelIndex>
#include "common.h"
#include "erroritem.h"
#include "applicationlist.h"
#include "resultstree.h"
#include "report.h"
#include "application.h"
#include "showtypes.h"

ResultsTree::ResultsTree(QWidget * parent) :
    QTreeView(parent),
    mContextItem(0),
    mShowErrorId(false),
    mVisibleErrors(false),
    mSelectionModel(0)
{
    setModel(&mModel);
    Translate(); // Adds columns to grid
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
    QStandardItem *item = new QStandardItem();
    item->setData(QVariant(linenumber.toULongLong()), Qt::DisplayRole);
    item->setToolTip(linenumber);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    item->setEditable(false);
    return item;
}

bool ResultsTree::AddErrorItem(const ErrorItem &item)
{
    if (item.files.isEmpty()) {
        return false;
    }

    QString realfile = StripPath(item.files[0], false);

    if (realfile.isEmpty()) {
        realfile = tr("Undefined file");
    }

    bool hide = !mShowSeverities.isShown(item.severity);
    //bool hide = !mShowTypes[SeverityToShowType(item.severity)];

    //If specified, filter on summary, message, filename, and id
    if (!hide && !mFilter.isEmpty()) {
        if (!item.summary.contains(mFilter, Qt::CaseInsensitive) &&
            !item.message.contains(mFilter, Qt::CaseInsensitive) &&
            !item.file.contains(mFilter, Qt::CaseInsensitive) &&
            !item.errorId.contains(mFilter, Qt::CaseInsensitive)) {
            hide = true;
        }
    }

    //if there is at least one error that is not hidden, we have a visible error
    if (!hide) {
        mVisibleErrors = true;
    }

    ErrorLine line;
    line.file = realfile;
    line.errorId = item.errorId;
    line.inconclusive = item.inconclusive;
    line.line = item.lines[0];
    line.summary = item.summary;
    line.message = item.message;
    line.severity = item.severity;
    //Create the base item for the error and ensure it has a proper
    //file item as a parent
    QStandardItem *stditem = AddBacktraceFiles(EnsureFileItem(line.file, item.file0, hide),
                             line,
                             hide,
                             SeverityToIcon(line.severity));

    if (!stditem)
        return false;

    //Add user data to that item
    QMap<QString, QVariant> data;
    data["hide"] = false;
    data["severity"]  = ShowTypes::SeverityToShowType(item.severity);
    data["summary"] = item.summary;
    data["message"]  = item.message;
    data["file"]  = item.files[0];
    data["line"]  = item.lines[0];
    data["id"]  = item.errorId;
    data["inconclusive"] = item.inconclusive;
    data["file0"] = item.file0;
    stditem->setData(QVariant(data));

    //Add backtrace files as children
    for (int i = 1; i < item.files.size(); i++) {
        line.file = StripPath(item.files[i], false);
        line.line = item.lines[i];
        QStandardItem *child_item;
        child_item = AddBacktraceFiles(stditem,
                                       line,
                                       hide,
                                       ":images/go-down.png");

        //Add user data to that item
        QMap<QString, QVariant> child_data;
        child_data["severity"]  = ShowTypes::SeverityToShowType(line.severity);
        child_data["summary"] = line.summary;
        child_data["message"]  = line.message;
        child_data["file"]  = item.files[i];
        child_data["line"]  = line.line;
        child_data["id"]  = line.errorId;
        child_data["inconclusive"] = line.inconclusive;
        child_item->setData(QVariant(child_data));
    }

    //TODO just hide/show current error and it's file
    //since this does a lot of unnecessary work
    if (!hide) {
        ShowFileItem(realfile);
    }
    return true;
}

QStandardItem *ResultsTree::AddBacktraceFiles(QStandardItem *parent,
        const ErrorLine &item,
        const bool hide,
        const QString &icon)

{
    if (!parent) {
        return 0;
    }

    QList<QStandardItem*> list;
    // Ensure shown path is with native separators
    const QString file = QDir::toNativeSeparators(item.file);
    list << CreateNormalItem(file);
    const QString severity = SeverityToTranslatedString(item.severity);
    list << CreateNormalItem(severity);
    list << CreateLineNumberItem(QString("%1").arg(item.line));
    list << CreateNormalItem(item.errorId);
    //TODO message has parameter names so we'll need changes to the core
    //cppcheck so we can get proper translations
    QString summary;
    if (item.inconclusive) {
        summary = tr("[Inconclusive]");
        summary += " ";
    }
    summary += item.summary.toLatin1();
    list << CreateNormalItem(summary);

    // Check for duplicate rows and don't add them if found
    for (int i = 0; i < parent->rowCount(); i++) {
        // The first column is the file name and is always the same

        // the third column is the line number so check it first
        if (parent->child(i, 2)->text() == list[2]->text()) {
            // the second column is the severity so check it next
            if (parent->child(i, 1)->text() == list[1]->text()) {
                // the fourth column is the summary so check it last
                if (parent->child(i, 4)->text() == list[4]->text()) {
                    // this row matches so don't add it
                    return 0;
                }
            }
        }
    }

    parent->appendRow(list);

    setRowHidden(parent->rowCount() - 1, parent->index(), hide);

    if (!icon.isEmpty()) {
        list[0]->setIcon(QIcon(icon));
    }

    //TODO Does this leak memory? Should items from list be deleted?

    return list[0];
}

QString ResultsTree::SeverityToTranslatedString(Severity::SeverityType severity)
{
    switch (severity) {
    case Severity::style:
        return tr("style");

    case Severity::error:
        return tr("error");

    case Severity::warning:
        return tr("warning");

    case Severity::performance:
        return tr("performance");

    case Severity::portability:
        return tr("portability");

    case Severity::information:
        return tr("information");

    case Severity::debug:
        return tr("debug");

    case Severity::none:
    default:
        return "";
    }
}

QStandardItem *ResultsTree::FindFileItem(const QString &name) const
{
    // The first column contains the file name. In Windows we can get filenames
    // "header.h" and "Header.h" and must compare them as identical.

    for (int i = 0; i < mModel.rowCount(); i++) {
#ifdef _WIN32
        if (QString::compare(mModel.item(i, 0)->text(), name, Qt::CaseInsensitive) == 0)
#else
        if (mModel.item(i, 0)->text() == name)
#endif
            return mModel.item(i, 0);
    }
    return 0;
}

void ResultsTree::Clear()
{
    mModel.removeRows(0, mModel.rowCount());
}

void ResultsTree::Clear(const QString &filename)
{
    const QString stripped = StripPath(filename, false);

    for (int i = 0; i < mModel.rowCount(); ++i) {
        const QStandardItem *item = mModel.item(i, 0);
        if (!item)
            continue;

        QVariantMap data = item->data().toMap();
        if (stripped == data["file"].toString() ||
            filename == data["file0"].toString()) {
            mModel.removeRow(i);
            break;
        }
    }
}


void ResultsTree::LoadSettings()
{
    for (int i = 0; i < mModel.columnCount(); i++) {
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        setColumnWidth(i, qMax(20, mSettings->value(temp, 800 / mModel.columnCount()).toInt()));
    }

    mSaveFullPath = mSettings->value(SETTINGS_SAVE_FULL_PATH, false).toBool();
    mSaveAllErrors = mSettings->value(SETTINGS_SAVE_ALL_ERRORS, false).toBool();
    mShowFullPath = mSettings->value(SETTINGS_SHOW_FULL_PATH, false).toBool();

    ShowIdColumn(mSettings->value(SETTINGS_SHOW_ERROR_ID, false).toBool());
}

void ResultsTree::SaveSettings() const
{
    for (int i = 0; i < mModel.columnCount(); i++) {
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        mSettings->setValue(temp, columnWidth(i));
    }
}

void ResultsTree::ShowResults(ShowTypes::ShowType type, bool show)
{
    if (type != ShowTypes::ShowNone && mShowSeverities.isShown(type) != show) {
        mShowSeverities.show(type, show);
        RefreshTree();
    }
}

void ResultsTree::FilterResults(const QString& filter)
{
    mFilter = filter;
    RefreshTree();
}

void ResultsTree::ShowHiddenResults()
{
    //Clear the "hide" flag for each item
    int filecount = mModel.rowCount();
    for (int i = 0; i < filecount; i++) {
        QStandardItem *file = mModel.item(i, 0);
        if (!file)
            continue;

        QVariantMap data = file->data().toMap();
        data["hide"] = false;
        file->setData(QVariant(data));

        int errorcount = file->rowCount();
        for (int j = 0; j < errorcount; j++) {
            QStandardItem *child = file->child(j, 0);
            if (child) {
                data = child->data().toMap();
                data["hide"] = false;
                child->setData(QVariant(data));
            }
        }
    }
    RefreshTree();
    emit ResultsHidden(false);
}


void ResultsTree::RefreshTree()
{
    mVisibleErrors = false;
    //Get the amount of files in the tree
    int filecount = mModel.rowCount();

    for (int i = 0; i < filecount; i++) {
        //Get file i
        QStandardItem *file = mModel.item(i, 0);
        if (!file) {
            continue;
        }

        //Get the amount of errors this file contains
        int errorcount = file->rowCount();

        //By default it shouldn't be visible
        bool show = false;

        for (int j = 0; j < errorcount; j++) {
            //Get the error itself
            QStandardItem *child = file->child(j, 0);
            if (!child) {
                continue;
            }

            //Get error's user data
            QVariant userdata = child->data();
            //Convert it to QVariantMap
            QVariantMap data = userdata.toMap();

            //Check if this error should be hidden
            bool hide = (data["hide"].toBool() || !mShowSeverities.isShown(ShowTypes::VariantToShowType(data["severity"])));

            //If specified, filter on summary, message, filename, and id
            if (!hide && !mFilter.isEmpty()) {
                if (!data["summary"].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !data["message"].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !data["file"].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !data["id"].toString().contains(mFilter, Qt::CaseInsensitive)) {
                    hide = true;
                }
            }

            if (!hide) {
                mVisibleErrors = true;
            }

            //Hide/show accordingly
            setRowHidden(j, file->index(), hide);

            //If it was shown then the file itself has to be shown as well
            if (!hide) {
                show = true;
            }
        }

        //Hide the file if its "hide" attribute is set
        if (file->data().toMap()["hide"].toBool()) {
            show = false;
        }

        //Show the file if any of it's errors are visible
        setRowHidden(i, QModelIndex(), !show);
    }
}

QStandardItem *ResultsTree::EnsureFileItem(const QString &fullpath, const QString &file0, bool hide)
{
    QString name = StripPath(fullpath, false);
    // Since item has path with native separators we must use path with
    // native separators to find it.
    QStandardItem *item = FindFileItem(QDir::toNativeSeparators(name));

    if (item) {
        return item;
    }

    // Ensure shown path is with native separators
    name = QDir::toNativeSeparators(name);
    item = CreateNormalItem(name);
    item->setIcon(QIcon(":images/text-x-generic.png"));

    //Add user data to that item
    QMap<QString, QVariant> data;
    data["file"] = fullpath;
    data["file0"] = file0;
    item->setData(QVariant(data));
    mModel.appendRow(item);

    setRowHidden(mModel.rowCount() - 1, QModelIndex(), hide);

    return item;
}

void ResultsTree::ShowFileItem(const QString &name)
{
    QStandardItem *item = FindFileItem(name);
    if (item) {
        setRowHidden(0, mModel.indexFromItem(item), false);
    }
}

void ResultsTree::contextMenuEvent(QContextMenuEvent * e)
{
    QModelIndex index = indexAt(e->pos());
    if (index.isValid()) {
        bool multipleSelection = false;
        mSelectionModel = selectionModel();
        if (mSelectionModel->selectedRows().count() > 1)
            multipleSelection = true;

        mContextItem = mModel.itemFromIndex(index);

        //Create a new context menu
        QMenu menu(this);

        //Store all applications in a list
        QList<QAction*> actions;

        //Create a signal mapper so we don't have to store data to class
        //member variables
        QSignalMapper *signalMapper = new QSignalMapper(this);

        if (mContextItem && mApplications->GetApplicationCount() > 0 && mContextItem->parent()) {
            //Go through all applications and add them to the context menu
            for (int i = 0; i < mApplications->GetApplicationCount(); i++) {
                //Create an action for the application
                const Application& app = mApplications->GetApplication(i);
                QAction *start = new QAction(app.getName(), &menu);
                if (multipleSelection)
                    start->setDisabled(true);

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
        if (mContextItem) {
            if (mApplications->GetApplicationCount() > 0) {
                menu.addSeparator();
            }

            //Create an action for the application
            QAction *copyfilename   = new QAction(tr("Copy filename"), &menu);
            QAction *copypath       = new QAction(tr("Copy full path"), &menu);
            QAction *copymessage    = new QAction(tr("Copy message"), &menu);
            QAction *copymessageid  = new QAction(tr("Copy message id"), &menu);
            QAction *hide           = new QAction(tr("Hide"), &menu);
            QAction *hideallid      = new QAction(tr("Hide all with id"), &menu);

            if (multipleSelection) {
                copyfilename->setDisabled(true);
                copypath->setDisabled(true);
                copymessage->setDisabled(true);
                copymessageid->setDisabled(true);
                hideallid->setDisabled(true);
            }

            menu.addAction(copyfilename);
            menu.addAction(copypath);
            menu.addAction(copymessage);
            menu.addAction(copymessageid);
            menu.addAction(hide);
            menu.addAction(hideallid);

            connect(copyfilename, SIGNAL(triggered()), this, SLOT(CopyFilename()));
            connect(copypath, SIGNAL(triggered()), this, SLOT(CopyFullPath()));
            connect(copymessage, SIGNAL(triggered()), this, SLOT(CopyMessage()));
            connect(copymessageid, SIGNAL(triggered()), this, SLOT(CopyMessageId()));
            connect(hide, SIGNAL(triggered()), this, SLOT(HideResult()));
            connect(hideallid, SIGNAL(triggered()), this, SLOT(HideAllIdResult()));
        }

        //Start the menu
        menu.exec(e->globalPos());

        if (mContextItem && mApplications->GetApplicationCount() > 0 && mContextItem->parent()) {
            //Disconnect all signals
            for (int i = 0; i < actions.size(); i++) {

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
    if (mApplications->GetApplicationCount() == 0) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("No editor application configured.\n\n"
                           "Configure the editor application for Cppcheck in preferences/Applications."),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;
    }

    if (application == -1)
        application = mApplications->GetDefaultApplication();

    if (application == -1) {
        QMessageBox msg(QMessageBox::Critical,
                        tr("Cppcheck"),
                        tr("No default editor application selected.\n\n"
                           "Please select the default editor application in preferences/Applications."),
                        QMessageBox::Ok,
                        this);
        msg.exec();
        return;

    }

    if (target && application >= 0 && application < mApplications->GetApplicationCount() && target->parent()) {
        // Make sure we are working with the first column
        if (target->column() != 0)
            target = target->parent()->child(target->row(), 0);

        QVariantMap data = target->data().toMap();

        //Replace (file) with filename
        QString file = data["file"].toString();
        file = QDir::toNativeSeparators(file);
#ifdef Q_WS_WIN
        file.replace(QString("\\"), QString("\\\\"));
#endif
        qDebug() << "Opening file: " << file;

        QFileInfo info(file);
        if (!info.exists()) {
            if (info.isAbsolute()) {
                QMessageBox msgbox(this);
                msgbox.setWindowTitle("Cppcheck");
                msgbox.setText(tr("Could not find the file!"));
                msgbox.setIcon(QMessageBox::Critical);
                msgbox.exec();
            } else {
                QDir checkdir(mCheckPath);
                if (checkdir.isAbsolute() && checkdir.exists()) {
                    file = mCheckPath + "/" + file;
                } else {
                    QString dir = AskFileDir(file);
                    dir += '/';
                    file = dir + file;
                }
            }
        }

        if (file.indexOf(" ") > -1) {
            file.insert(0, "\"");
            file.append("\"");
        }

        const Application& app = mApplications->GetApplication(application);
        QString params = app.getParameters();
        params.replace("(file)", file, Qt::CaseInsensitive);

        QVariant line = data["line"];
        params.replace("(line)", QString("%1").arg(line.toInt()), Qt::CaseInsensitive);

        params.replace("(message)", data["message"].toString(), Qt::CaseInsensitive);
        params.replace("(severity)", data["severity"].toString(), Qt::CaseInsensitive);

        QString program = app.getPath();

        // In Windows we must surround paths including spaces with quotation marks.
#ifdef Q_WS_WIN
        if (program.indexOf(" ") > -1) {
            if (!program.startsWith('"') && !program.endsWith('"')) {
                program.insert(0, "\"");
                program.append("\"");
            }
        }
#endif // Q_WS_WIN

        const QString cmdLine = QString("%1 %2").arg(program).arg(params);

        bool success = QProcess::startDetached(cmdLine);
        if (!success) {
            QString text = tr("Could not start %1\n\nPlease check the application path and parameters are correct.").arg(program);

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
                  GetPath(SETTINGS_LAST_SOURCE_PATH),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    mCheckPath = dir;
    SetPath(SETTINGS_LAST_SOURCE_PATH, dir);
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
    if (mContextItem) {
        // Make sure we are working with the first column
        if (mContextItem->column() != 0)
            mContextItem = mContextItem->parent()->child(mContextItem->row(), 0);

        QVariantMap data = mContextItem->data().toMap();

        QString message;
        if (data["inconclusive"].toBool()) {
            message = tr("[Inconclusive]");
            message += " ";
        }
        message += data["message"].toString();

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(message);
    }
}

void ResultsTree::CopyMessageId()
{
    if (mContextItem) {
        // Make sure we are working with the first column
        if (mContextItem->column() != 0)
            mContextItem = mContextItem->parent()->child(mContextItem->row(), 0);
        QVariantMap data = mContextItem->data().toMap();

        QString messageId = data["id"].toString();

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(messageId);
    }
}

void ResultsTree::HideResult()
{
    if (!mSelectionModel)
        return;

    QModelIndexList selectedRows = mSelectionModel->selectedRows();
    QModelIndex index;
    foreach(index, selectedRows) {
        QStandardItem *item = mModel.itemFromIndex(index);
        //Set the "hide" flag for this item
        QVariantMap data = item->data().toMap();
        data["hide"] = true;
        item->setData(QVariant(data));

        RefreshTree();
        emit ResultsHidden(true);
    }
}

void ResultsTree::HideAllIdResult()
{
    if (mContextItem) {
        // Make sure we are working with the first column
        if (mContextItem->column() != 0)
            mContextItem = mContextItem->parent()->child(mContextItem->row(), 0);
        QVariantMap data = mContextItem->data().toMap();

        QString messageId = data["id"].toString();

        // hide all errors with that message Id
        int filecount = mModel.rowCount();
        for (int i = 0; i < filecount; i++) {
            //Get file i
            QStandardItem *file = mModel.item(i, 0);
            if (!file) {
                continue;
            }

            //Get the amount of errors this file contains
            int errorcount = file->rowCount();

            for (int j = 0; j < errorcount; j++) {
                //Get the error itself
                QStandardItem *child = file->child(j, 0);
                if (!child) {
                    continue;
                }

                QVariantMap userdata = child->data().toMap();
                if (userdata["id"].toString() == messageId) {
                    userdata["hide"] = true;
                    child->setData(QVariant(userdata));
                }
            }
        }

        RefreshTree();
        emit ResultsHidden(true);
    }
}

void ResultsTree::Context(int application)
{
    StartApplication(mContextItem, application);
}

void ResultsTree::QuickStartApplication(const QModelIndex &index)
{
    StartApplication(mModel.itemFromIndex(index));
}

void ResultsTree::CopyPath(QStandardItem *target, bool fullPath)
{
    if (target) {
        // Make sure we are working with the first column
        if (target->column() != 0)
            target = target->parent()->child(target->row(), 0);

        QVariantMap data = target->data().toMap();
        QString pathStr;

        //Replace (file) with filename
        QString file = data["file"].toString();
        pathStr = QDir::toNativeSeparators(file);
        if (!fullPath) {
            QFileInfo fi(pathStr);
            pathStr = fi.fileName();
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(pathStr);
    }
}

QString ResultsTree::SeverityToIcon(Severity::SeverityType severity) const
{
    switch (severity) {
    case Severity::error:
        return ":images/dialog-error.png";
    case Severity::style:
        return ":images/applications-development.png";
    case Severity::warning:
        return ":images/dialog-warning.png";
    case Severity::portability:
        return ":images/applications-system.png";
    case Severity::performance:
        return ":images/utilities-system-monitor.png";
    case Severity::information:
        return ":images/dialog-information.png";
    default:
        return "";
    }
}

void ResultsTree::SaveResults(Report *report) const
{
    report->WriteHeader();

    for (int i = 0; i < mModel.rowCount(); i++) {
        QStandardItem *item = mModel.item(i, 0);
        if (!isRowHidden(i, item->index()))
            SaveErrors(report, item);
    }

    report->WriteFooter();
}

void ResultsTree::SaveErrors(Report *report, QStandardItem *item) const
{
    if (!item) {
        return;
    }

    for (int i = 0; i < item->rowCount(); i++) {
        QStandardItem *error = item->child(i, 0);

        if (!error) {
            continue;
        }

        if (isRowHidden(i, item->index()) && !mSaveAllErrors) {
            continue;
        }

        //Get error's user data
        QVariant userdata = error->data();
        //Convert it to QVariantMap
        QVariantMap data = userdata.toMap();

        ErrorItem item;
        item.severity = ShowTypes::ShowTypeToSeverity(ShowTypes::VariantToShowType(data["severity"]));
        item.summary = data["summary"].toString();
        item.message = data["message"].toString();
        item.errorId = data["id"].toString();
        item.inconclusive = data["inconclusive"].toBool();
        item.file0 = data["file0"].toString();
        QString file = StripPath(data["file"].toString(), true);
        unsigned int line = data["line"].toUInt();

        item.files << file;
        item.lines << line;

        for (int j = 0; j < error->rowCount(); j++) {
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

void ResultsTree::UpdateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors,
                                 bool showErrorId)
{
    if (mShowFullPath != showFullPath) {
        mShowFullPath = showFullPath;
        RefreshFilePaths();
    }

    mSaveFullPath = saveFullPath;
    mSaveAllErrors = saveAllErrors;

    ShowIdColumn(showErrorId);
}

void ResultsTree::SetCheckDirectory(const QString &dir)
{
    mCheckPath = dir;
}

QString ResultsTree::StripPath(const QString &path, bool saving) const
{
    if ((!saving && mShowFullPath) || (saving && mSaveFullPath)) {
        return QString(path);
    }

    QDir dir(mCheckPath);
    return dir.relativeFilePath(path);
}

void ResultsTree::RefreshFilePaths(QStandardItem *item)
{
    if (!item) {
        return;
    }

    //Mark that this file's path hasn't been updated yet
    bool updated = false;

    //Loop through all errors within this file
    for (int i = 0; i < item->rowCount(); i++) {
        //Get error i
        QStandardItem *error = item->child(i, 0);

        if (!error) {
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
        if (error->hasChildren()) {
            //Loop through all files within the error
            for (int j = 0; j < error->rowCount(); j++) {
                //Get file
                QStandardItem *child = error->child(j, 0);
                if (!child) {
                    continue;
                }
                //Get child's user data
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
        if (!updated) {
            updated = true;
            item->setText(error->text());
        }

    }
}

void ResultsTree::RefreshFilePaths()
{
    qDebug("Refreshing file paths");

    //Go through all file items (these are parent items that contain the errors)
    for (int i = 0; i < mModel.rowCount(); i++) {
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
    labels << tr("File") << tr("Severity") << tr("Line") << tr("Id") << tr("Summary");
    mModel.setHorizontalHeaderLabels(labels);
    //TODO go through all the errors in the tree and translate severity and message
}

void ResultsTree::ShowIdColumn(bool show)
{
    mShowErrorId = show;
    if (show)
        showColumn(3);
    else
        hideColumn(3);
}

void ResultsTree::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    emit SelectionChanged(current);
}

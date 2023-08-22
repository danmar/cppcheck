/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "resultstree.h"

#include "application.h"
#include "applicationlist.h"
#include "common.h"
#include "config.h"
#include "erroritem.h"
#include "path.h"
#include "projectfile.h"
#include "report.h"
#include "showtypes.h"
#include "suppressions.h"
#include "threadhandler.h"
#include "xmlreportv2.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QModelIndexList>
#include <QProcess>
#include <QSet>
#include <QSettings>
#include <QSignalMapper>
#include <QStandardItem>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <Qt>

static const char COLUMN[] = "column";
static const char CWE[] = "cwe";
static const char ERRORID[] = "id";
static const char FILENAME[] = "file";
static const char FILE0[] = "file0";
static const char HASH[] = "hash";
static const char HIDE[] = "hide";
static const char INCONCLUSIVE[] = "inconclusive";
static const char LINE[] = "line";
static const char MESSAGE[] = "message";
static const char SEVERITY[] = "severity";
static const char SINCEDATE[] = "sinceDate";
static const char SYMBOLNAMES[] = "symbolNames";
static const char SUMMARY[] = "summary";
static const char TAGS[] = "tags";

// These must match column headers given in ResultsTree::translate()
static const int COLUMN_SINCE_DATE = 6;
static const int COLUMN_TAGS       = 7;

ResultsTree::ResultsTree(QWidget * parent) :
    QTreeView(parent)
{
    setModel(&mModel);
    translate(); // Adds columns to grid
    setExpandsOnDoubleClick(false);
    setSortingEnabled(true);

    connect(this, &ResultsTree::doubleClicked, this, &ResultsTree::quickStartApplication);
}

void ResultsTree::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        quickStartApplication(this->currentIndex());
    }
    QTreeView::keyPressEvent(event);
}

void ResultsTree::initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler)
{
    mSettings = settings;
    mApplications = list;
    mThread = checkThreadHandler;
    loadSettings();
}


QStandardItem *ResultsTree::createNormalItem(const QString &name)
{
    QStandardItem *item = new QStandardItem(name);
    item->setData(name, Qt::ToolTipRole);
    item->setEditable(false);
    return item;
}

QStandardItem *ResultsTree::createCheckboxItem(bool checked)
{
    QStandardItem *item = new QStandardItem;
    item->setCheckable(true);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    item->setEnabled(false);
    return item;
}

QStandardItem *ResultsTree::createLineNumberItem(const QString &linenumber)
{
    QStandardItem *item = new QStandardItem();
    item->setData(QVariant(linenumber.toInt()), Qt::DisplayRole);
    item->setToolTip(linenumber);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    item->setEditable(false);
    return item;
}

bool ResultsTree::addErrorItem(const ErrorItem &item)
{
    if (item.errorPath.isEmpty()) {
        return false;
    }

    const QErrorPathItem &loc = item.errorId.startsWith("clang") ? item.errorPath.front() : item.errorPath.back();
    QString realfile = stripPath(loc.file, false);

    if (realfile.isEmpty()) {
        realfile = tr("Undefined file");
    }

    bool hide = false;

    // Ids that are temporarily hidden..
    if (mHiddenMessageId.contains(item.errorId))
        hide = true;

    //If specified, filter on summary, message, filename, and id
    if (!hide && !mFilter.isEmpty()) {
        if (!item.summary.contains(mFilter, Qt::CaseInsensitive) &&
            !item.message.contains(mFilter, Qt::CaseInsensitive) &&
            !item.errorPath.back().file.contains(mFilter, Qt::CaseInsensitive) &&
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
    line.line = loc.line;
    line.errorId = item.errorId;
    line.cwe = item.cwe;
    line.hash = item.hash;
    line.inconclusive = item.inconclusive;
    line.summary = item.summary;
    line.message = item.message;
    line.severity = item.severity;
    line.sinceDate = item.sinceDate;
    if (const ProjectFile *activeProject = ProjectFile::getActiveProject()) {
        line.tags = activeProject->getWarningTags(item.hash);
    }
    //Create the base item for the error and ensure it has a proper
    //file item as a parent
    QStandardItem* fileItem = ensureFileItem(loc.file, item.file0, hide);
    QStandardItem* stditem = addBacktraceFiles(fileItem,
                                               line,
                                               hide,
                                               severityToIcon(line.severity),
                                               false);

    if (!stditem)
        return false;

    //Add user data to that item
    QMap<QString, QVariant> data;
    data[SEVERITY]  = ShowTypes::SeverityToShowType(item.severity);
    data[SUMMARY] = item.summary;
    data[MESSAGE]  = item.message;
    data[FILENAME]  = loc.file;
    data[LINE]  = loc.line;
    data[COLUMN] = loc.column;
    data[ERRORID]  = item.errorId;
    data[CWE] = item.cwe;
    data[HASH] = item.hash;
    data[INCONCLUSIVE] = item.inconclusive;
    data[FILE0] = stripPath(item.file0, true);
    data[SINCEDATE] = item.sinceDate;
    data[SYMBOLNAMES] = item.symbolNames;
    data[TAGS] = line.tags;
    data[HIDE] = hide;
    stditem->setData(QVariant(data));

    //Add backtrace files as children
    if (item.errorPath.size() > 1) {
        for (int i = 0; i < item.errorPath.size(); i++) {
            const QErrorPathItem &e = item.errorPath[i];
            line.file = e.file;
            line.line = e.line;
            line.message = line.summary = e.info;
            QStandardItem *child_item;
            child_item = addBacktraceFiles(stditem,
                                           line,
                                           hide,
                                           ":images/go-down.png",
                                           true);
            if (!child_item)
                continue;

            // Add user data to that item
            QMap<QString, QVariant> child_data;
            child_data[SEVERITY]  = ShowTypes::SeverityToShowType(line.severity);
            child_data[SUMMARY] = line.summary;
            child_data[MESSAGE]  = line.message;
            child_data[FILENAME]  = e.file;
            child_data[LINE]  = e.line;
            child_data[COLUMN] = e.column;
            child_data[ERRORID]  = line.errorId;
            child_data[CWE] = line.cwe;
            child_data[HASH] = line.hash;
            child_data[INCONCLUSIVE] = line.inconclusive;
            child_data[SYMBOLNAMES] = item.symbolNames;
            child_item->setData(QVariant(child_data));
        }
    }

    // Partially refresh the tree: Unhide file item if necessary
    if (!hide) {
        setRowHidden(fileItem->row(), QModelIndex(), !mShowSeverities.isShown(item.severity));
    }
    return true;
}

QStandardItem *ResultsTree::addBacktraceFiles(QStandardItem *parent,
                                              const ErrorLine &item,
                                              const bool hide,
                                              const QString &icon,
                                              bool childOfMessage)
{
    if (!parent) {
        return nullptr;
    }

    QList<QStandardItem*> list;
    // Ensure shown path is with native separators
    list << createNormalItem(QDir::toNativeSeparators(item.file))
         << createNormalItem(childOfMessage ? tr("note") : severityToTranslatedString(item.severity))
         << createLineNumberItem(QString::number(item.line))
         << createNormalItem(childOfMessage ? QString() : item.errorId)
         << (childOfMessage ? createNormalItem(QString()) : createCheckboxItem(item.inconclusive))
         << createNormalItem(item.summary)
         << createNormalItem(item.sinceDate)
         << createNormalItem(item.tags);

    //TODO message has parameter names so we'll need changes to the core
    //cppcheck so we can get proper translations

    // Check for duplicate rows and don't add them if found
    for (int i = 0; i < parent->rowCount(); i++) {
        // The first column is the file name and is always the same

        // the third column is the line number so check it first
        if (parent->child(i, 2)->text() == list[2]->text()) {
            // the second column is the severity so check it next
            if (parent->child(i, 1)->text() == list[1]->text()) {
                // the sixth column is the summary so check it last
                if (parent->child(i, 5)->text() == list[5]->text()) {
                    // this row matches so don't add it
                    return nullptr;
                }
            }
        }
    }

    parent->appendRow(list);

    setRowHidden(parent->rowCount() - 1, parent->index(), hide);

    if (!icon.isEmpty()) {
        list[0]->setIcon(QIcon(icon));
    }

    /* TODO: the list items leak memory
        Indirect leak of 80624 byte(s) in 5039 object(s) allocated from:
     #0 0xa15a2d in operator new(unsigned long) (/mnt/s/GitHub/cppcheck-fw/cmake-build-debug-wsl-kali-clang-asan-ubsan/bin/cppcheck-gui+0xa15a2d)
     #1 0xdda276 in ResultsTree::createNormalItem(QString const&) /mnt/s/GitHub/cppcheck-fw/gui/resultstree.cpp:122:27
     #2 0xde4290 in ResultsTree::addBacktraceFiles(QStandardItem*, ErrorLine const&, bool, QString const&, bool) /mnt/s/GitHub/cppcheck-fw/gui/resultstree.cpp:289:13
     #3 0xddd754 in ResultsTree::addErrorItem(ErrorItem const&) /mnt/s/GitHub/cppcheck-fw/gui/resultstree.cpp:199:30
     #4 0xe37046 in ResultsView::error(ErrorItem const&) /mnt/s/GitHub/cppcheck-fw/gui/resultsview.cpp:129:21
     #5 0xd2448d in QtPrivate::FunctorCall<QtPrivate::IndexesList<0>, QtPrivate::List<ErrorItem const&>, void, void (ResultsView::*)(ErrorItem const&)>::call(void (ResultsView::*)(ErrorItem const&), ResultsView*, void**) /usr/include/x86_64-linux-gnu/qt5/QtCore/qobjectdefs_impl.h:152:13
     #6 0xd2402c in void QtPrivate::FunctionPointer<void (ResultsView::*)(ErrorItem const&)>::call<QtPrivate::List<ErrorItem const&>, void>(void (ResultsView::*)(ErrorItem const&), ResultsView*, void**) /usr/include/x86_64-linux-gnu/qt5/QtCore/qobjectdefs_impl.h:185:13
     #7 0xd23b45 in QtPrivate::QSlotObject<void (ResultsView::*)(ErrorItem const&), QtPrivate::List<ErrorItem const&>, void>::impl(int, QtPrivate::QSlotObjectBase*, QObject*, void**, bool*) /usr/include/x86_64-linux-gnu/qt5/QtCore/qobjectdefs_impl.h:418:17
     #8 0x7fd2536cc0dd in QObject::event(QEvent*) (/usr/lib/x86_64-linux-gnu/libQt5Core.so.5+0x2dc0dd)
     #9 0x7fd2541836be in QApplicationPrivate::notify_helper(QObject*, QEvent*) (/usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5+0x1636be)
     */

    return list[0];
}

QString ResultsTree::severityToTranslatedString(Severity::SeverityType severity)
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
        return QString();
    }
}

QStandardItem *ResultsTree::findFileItem(const QString &name) const
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
    return nullptr;
}

void ResultsTree::clear()
{
    mModel.removeRows(0, mModel.rowCount());
}

void ResultsTree::clear(const QString &filename)
{
    const QString stripped = stripPath(filename, false);

    for (int i = 0; i < mModel.rowCount(); ++i) {
        const QStandardItem *fileItem = mModel.item(i, 0);
        if (!fileItem)
            continue;

        QVariantMap data = fileItem->data().toMap();
        if (stripped == data[FILENAME].toString() ||
            filename == data[FILE0].toString()) {
            mModel.removeRow(i);
            break;
        }
    }
}

void ResultsTree::clearRecheckFile(const QString &filename)
{
    for (int i = 0; i < mModel.rowCount(); ++i) {
        const QStandardItem *fileItem = mModel.item(i, 0);
        if (!fileItem)
            continue;

        QString actualfile((!mCheckPath.isEmpty() && filename.startsWith(mCheckPath)) ? filename.mid(mCheckPath.length() + 1) : filename);
        QVariantMap data = fileItem->data().toMap();
        QString storedfile = data[FILENAME].toString();
        storedfile = ((!mCheckPath.isEmpty() && storedfile.startsWith(mCheckPath)) ? storedfile.mid(mCheckPath.length() + 1) : storedfile);
        if (actualfile == storedfile) {
            mModel.removeRow(i);
            break;
        }
    }
}


void ResultsTree::loadSettings()
{
    for (int i = 0; i < mModel.columnCount(); i++) {
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        setColumnWidth(i, qMax(20, mSettings->value(temp, 800 / mModel.columnCount()).toInt()));
    }

    mSaveFullPath = mSettings->value(SETTINGS_SAVE_FULL_PATH, false).toBool();
    mSaveAllErrors = mSettings->value(SETTINGS_SAVE_ALL_ERRORS, false).toBool();
    mShowFullPath = mSettings->value(SETTINGS_SHOW_FULL_PATH, false).toBool();

    showIdColumn(mSettings->value(SETTINGS_SHOW_ERROR_ID, false).toBool());
    showInconclusiveColumn(mSettings->value(SETTINGS_INCONCLUSIVE_ERRORS, false).toBool());
}

void ResultsTree::saveSettings() const
{
    for (int i = 0; i < mModel.columnCount(); i++) {
        QString temp = QString(SETTINGS_RESULT_COLUMN_WIDTH).arg(i);
        mSettings->setValue(temp, columnWidth(i));
    }
}

void ResultsTree::showResults(ShowTypes::ShowType type, bool show)
{
    if (type != ShowTypes::ShowNone && mShowSeverities.isShown(type) != show) {
        mShowSeverities.show(type, show);
        refreshTree();
    }
}

void ResultsTree::showCppcheckResults(bool show)
{
    mShowCppcheck = show;
    refreshTree();
}

void ResultsTree::showClangResults(bool show)
{
    mShowClang = show;
    refreshTree();
}

void ResultsTree::filterResults(const QString& filter)
{
    mFilter = filter;
    refreshTree();
}

void ResultsTree::showHiddenResults()
{
    //Clear the "hide" flag for each item
    mHiddenMessageId.clear();
    const int filecount = mModel.rowCount();
    for (int i = 0; i < filecount; i++) {
        QStandardItem *fileItem = mModel.item(i, 0);
        if (!fileItem)
            continue;

        QVariantMap data = fileItem->data().toMap();
        data[HIDE] = false;
        fileItem->setData(QVariant(data));

        const int errorcount = fileItem->rowCount();
        for (int j = 0; j < errorcount; j++) {
            QStandardItem *child = fileItem->child(j, 0);
            if (child) {
                data = child->data().toMap();
                data[HIDE] = false;
                child->setData(QVariant(data));
            }
        }
    }
    refreshTree();
    emit resultsHidden(false);
}


void ResultsTree::refreshTree()
{
    mVisibleErrors = false;
    //Get the amount of files in the tree
    const int filecount = mModel.rowCount();

    for (int i = 0; i < filecount; i++) {
        //Get file i
        QStandardItem *fileItem = mModel.item(i, 0);
        if (!fileItem) {
            continue;
        }

        //Get the amount of errors this file contains
        const int errorcount = fileItem->rowCount();

        //By default it shouldn't be visible
        bool show = false;

        for (int j = 0; j < errorcount; j++) {
            //Get the error itself
            QStandardItem *child = fileItem->child(j, 0);
            if (!child) {
                continue;
            }

            //Get error's user data
            QVariant userdata = child->data();
            //Convert it to QVariantMap
            QVariantMap data = userdata.toMap();

            //Check if this error should be hidden
            bool hide = (data[HIDE].toBool() || !mShowSeverities.isShown(ShowTypes::VariantToShowType(data[SEVERITY])));

            //If specified, filter on summary, message, filename, and id
            if (!hide && !mFilter.isEmpty()) {
                if (!data[SUMMARY].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !data[MESSAGE].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !data[FILENAME].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !data[ERRORID].toString().contains(mFilter, Qt::CaseInsensitive)) {
                    hide = true;
                }
            }

            // Tool filter
            if (!hide) {
                if (data[ERRORID].toString().startsWith("clang"))
                    hide = !mShowClang;
                else
                    hide = !mShowCppcheck;
            }

            if (!hide) {
                mVisibleErrors = true;
            }

            //Hide/show accordingly
            setRowHidden(j, fileItem->index(), hide);

            //If it was shown then the file itself has to be shown as well
            if (!hide) {
                show = true;
            }
        }

        //Hide the file if its "hide" attribute is set
        if (fileItem->data().toMap()["hide"].toBool()) {
            show = false;
        }

        //Show the file if any of it's errors are visible
        setRowHidden(i, QModelIndex(), !show);
    }
}

QStandardItem *ResultsTree::ensureFileItem(const QString &fullpath, const QString &file0, bool hide)
{
    QString name = stripPath(fullpath, false);
    // Since item has path with native separators we must use path with
    // native separators to find it.
    QStandardItem *item = findFileItem(QDir::toNativeSeparators(name));

    if (item) {
        return item;
    }

    // Ensure shown path is with native separators
    name = QDir::toNativeSeparators(name);
    item = createNormalItem(name);
    item->setIcon(QIcon(":images/text-x-generic.png"));

    //Add user data to that item
    QMap<QString, QVariant> data;
    data[FILENAME] = fullpath;
    data[FILE0] = file0;
    item->setData(QVariant(data));
    mModel.appendRow(item);

    setRowHidden(mModel.rowCount() - 1, QModelIndex(), hide);

    return item;
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

        if (mContextItem && mApplications->getApplicationCount() > 0 && mContextItem->parent()) {
            //Create an action for the application
            int defaultApplicationIndex = mApplications->getDefaultApplication();
            if (defaultApplicationIndex < 0)
                defaultApplicationIndex = 0;
            const Application& app = mApplications->getApplication(defaultApplicationIndex);
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
            signalMapper->setMapping(start, defaultApplicationIndex);

            connect(signalMapper, SIGNAL(mapped(int)),
                    this, SLOT(context(int)));
        }

        // Add popup menuitems
        if (mContextItem) {
            if (mApplications->getApplicationCount() > 0) {
                menu.addSeparator();
            }

            //Create an action for the application
            QAction *recheckSelectedFiles   = new QAction(tr("Recheck"), &menu);
            QAction *copy                   = new QAction(tr("Copy"), &menu);
            QAction *hide                   = new QAction(tr("Hide"), &menu);
            QAction *hideallid              = new QAction(tr("Hide all with id"), &menu);
            QAction *opencontainingfolder   = new QAction(tr("Open containing folder"), &menu);

            if (multipleSelection) {
                hideallid->setDisabled(true);
                opencontainingfolder->setDisabled(true);
            }
            if (mThread->isChecking())
                recheckSelectedFiles->setDisabled(true);
            else
                recheckSelectedFiles->setDisabled(false);

            menu.addAction(recheckSelectedFiles);
            menu.addSeparator();
            menu.addAction(copy);
            menu.addSeparator();
            menu.addAction(hide);
            menu.addAction(hideallid);

            QAction *suppress = new QAction(tr("Suppress selected id(s)"), &menu);
            menu.addAction(suppress);
            connect(suppress, &QAction::triggered, this, &ResultsTree::suppressSelectedIds);

            menu.addSeparator();
            menu.addAction(opencontainingfolder);

            connect(recheckSelectedFiles, SIGNAL(triggered()), this, SLOT(recheckSelectedFiles()));
            connect(copy, SIGNAL(triggered()), this, SLOT(copy()));
            connect(hide, SIGNAL(triggered()), this, SLOT(hideResult()));
            connect(hideallid, SIGNAL(triggered()), this, SLOT(hideAllIdResult()));
            connect(opencontainingfolder, SIGNAL(triggered()), this, SLOT(openContainingFolder()));

            const ProjectFile *currentProject = ProjectFile::getActiveProject();
            if (currentProject && !currentProject->getTags().isEmpty()) {
                menu.addSeparator();
                QMenu *tagMenu = menu.addMenu(tr("Tag"));
                {
                    QAction *action = new QAction(tr("No tag"), tagMenu);
                    tagMenu->addAction(action);
                    connect(action, &QAction::triggered, [=]() {
                        tagSelectedItems(QString());
                    });
                }

                for (const QString& tagstr : currentProject->getTags()) {
                    QAction *action = new QAction(tagstr, tagMenu);
                    tagMenu->addAction(action);
                    connect(action, &QAction::triggered, [=]() {
                        tagSelectedItems(tagstr);
                    });
                }
            }
        }

        //Start the menu
        menu.exec(e->globalPos());
        index = indexAt(e->pos());
        if (index.isValid()) {
            mContextItem = mModel.itemFromIndex(index);
            if (mContextItem && mApplications->getApplicationCount() > 0 && mContextItem->parent()) {
                //Disconnect all signals
                for (const QAction* action : actions) {
                    disconnect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
                }

                disconnect(signalMapper, SIGNAL(mapped(int)),
                           this, SLOT(context(int)));
                //And remove the signal mapper
                delete signalMapper;
            }
        }
    }
}

void ResultsTree::startApplication(QStandardItem *target, int application)
{
    //If there are no applications specified, tell the user about it
    if (mApplications->getApplicationCount() == 0) {
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
        application = mApplications->getDefaultApplication();

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

    if (target && application >= 0 && application < mApplications->getApplicationCount() && target->parent()) {
        // Make sure we are working with the first column
        if (target->column() != 0)
            target = target->parent()->child(target->row(), 0);

        QVariantMap data = target->data().toMap();

        //Replace (file) with filename
        QString file = data[FILENAME].toString();
        file = QDir::toNativeSeparators(file);
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
                    QString dir = askFileDir(file);
                    dir += '/';
                    file = dir + file;
                }
            }
        }

        if (file.indexOf(" ") > -1) {
            file.insert(0, "\"");
            file.append("\"");
        }

        const Application& app = mApplications->getApplication(application);
        QString params = app.getParameters();
        params.replace("(file)", file, Qt::CaseInsensitive);

        QVariant line = data[LINE];
        params.replace("(line)", QString("%1").arg(line.toInt()), Qt::CaseInsensitive);

        params.replace("(message)", data[MESSAGE].toString(), Qt::CaseInsensitive);
        params.replace("(severity)", data[SEVERITY].toString(), Qt::CaseInsensitive);

        QString program = app.getPath();

        // In Windows we must surround paths including spaces with quotation marks.
#ifdef Q_OS_WIN
        if (program.indexOf(" ") > -1) {
            if (!program.startsWith('"') && !program.endsWith('"')) {
                program.insert(0, "\"");
                program.append("\"");
            }
        }
#endif // Q_OS_WIN

        const QString cmdLine = QString("%1 %2").arg(program).arg(params);

        // this is reported as deprecated in Qt 5.15.2 but no longer in Qt 6
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        SUPPRESS_WARNING_CLANG_PUSH("-Wdeprecated")
        SUPPRESS_WARNING_GCC_PUSH("-Wdeprecated-declarations")
#endif
        const bool success = QProcess::startDetached(cmdLine);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        SUPPRESS_WARNING_GCC_POP
            SUPPRESS_WARNING_CLANG_POP
#endif
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

QString ResultsTree::askFileDir(const QString &file)
{
    QString text = tr("Could not find file:") + '\n' + file + '\n';
    QString title;
    if (file.indexOf('/')) {
        QString folderName = file.mid(0, file.indexOf('/'));
        text += tr("Please select the folder '%1'").arg(folderName);
        title = tr("Select Directory '%1'").arg(folderName);
    } else {
        text += tr("Please select the directory where file is located.");
        title = tr("Select Directory");
    }

    QMessageBox msgbox(this);
    msgbox.setWindowTitle("Cppcheck");
    msgbox.setText(text);
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.exec();

    QString dir = QFileDialog::getExistingDirectory(this, title,
                                                    getPath(SETTINGS_LAST_SOURCE_PATH),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return QString();

    // User selected root path
    if (QFileInfo::exists(dir + '/' + file))
        mCheckPath = dir;

    // user selected checked folder
    else if (file.indexOf('/') > 0) {
        dir += '/';
        QString folderName = file.mid(0, file.indexOf('/'));
        if (dir.indexOf('/' + folderName + '/'))
            dir = dir.mid(0, dir.lastIndexOf('/' + folderName + '/'));
        if (QFileInfo::exists(dir + '/' + file))
            mCheckPath = dir;
    }

    // Otherwise; return
    else
        return QString();

    setPath(SETTINGS_LAST_SOURCE_PATH, mCheckPath);
    return mCheckPath;
}

void ResultsTree::copy()
{
    if (!mSelectionModel)
        return;

    QString text;
    for (QModelIndex index : mSelectionModel->selectedRows()) {
        QStandardItem *item = mModel.itemFromIndex(index);
        if (!item->parent()) {
            text += item->text() + '\n';
            continue;
        }
        if (item->parent()->parent())
            item = item->parent();
        QVariantMap data = item->data().toMap();
        if (!data.contains("id"))
            continue;
        QString inconclusive = data[INCONCLUSIVE].toBool() ? ",inconclusive" : "";
        text += '[' + data[FILENAME].toString() + ':' + QString::number(data[LINE].toInt())
                + "] ("
                + QString::fromStdString(Severity::toString(ShowTypes::ShowTypeToSeverity((ShowTypes::ShowType)data[SEVERITY].toInt()))) + inconclusive
                + ") "
                + data[MESSAGE].toString()
                + " ["
                + data[ERRORID].toString()
                + "]\n";
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void ResultsTree::hideResult()
{
    if (!mSelectionModel)
        return;

    for (QModelIndex index : mSelectionModel->selectedRows()) {
        QStandardItem *item = mModel.itemFromIndex(index);
        //Set the "hide" flag for this item
        QVariantMap data = item->data().toMap();
        data[HIDE] = true;
        item->setData(QVariant(data));

        refreshTree();
        emit resultsHidden(true);
    }
}

void ResultsTree::recheckSelectedFiles()
{
    if (!mSelectionModel)
        return;

    QStringList selectedItems;
    for (QModelIndex index : mSelectionModel->selectedRows()) {
        QStandardItem *item = mModel.itemFromIndex(index);
        while (item->parent())
            item = item->parent();
        QVariantMap data = item->data().toMap();
        QString currentFile = data[FILENAME].toString();
        if (!currentFile.isEmpty()) {
            QString fileNameWithCheckPath;
            QFileInfo curfileInfo(currentFile);
            if (!curfileInfo.exists() && !mCheckPath.isEmpty() && currentFile.indexOf(mCheckPath) != 0)
                fileNameWithCheckPath = mCheckPath + "/" + currentFile;
            else
                fileNameWithCheckPath = currentFile;
            const QFileInfo fileInfo(fileNameWithCheckPath);
            if (!fileInfo.exists()) {
                askFileDir(currentFile);
                return;
            }
            if (Path::isHeader(currentFile.toStdString())) {
                if (!data[FILE0].toString().isEmpty() && !selectedItems.contains(data[FILE0].toString())) {
                    selectedItems<<((!mCheckPath.isEmpty() && (data[FILE0].toString().indexOf(mCheckPath) != 0)) ? (mCheckPath + "/" + data[FILE0].toString()) : data[FILE0].toString());
                    if (!selectedItems.contains(fileNameWithCheckPath))
                        selectedItems<<fileNameWithCheckPath;
                }
            } else if (!selectedItems.contains(fileNameWithCheckPath))
                selectedItems<<fileNameWithCheckPath;
        }
    }
    emit checkSelected(selectedItems);
}

void ResultsTree::hideAllIdResult()
{
    if (!mContextItem || !mContextItem->parent())
        return;

    // Make sure we are working with the first column
    if (mContextItem->column() != 0)
        mContextItem = mContextItem->parent()->child(mContextItem->row(), 0);
    QVariantMap data = mContextItem->data().toMap();

    QString messageId = data[ERRORID].toString();

    mHiddenMessageId.append(messageId);

    // hide all errors with that message Id
    const int filecount = mModel.rowCount();
    for (int i = 0; i < filecount; i++) {
        //Get file i
        QStandardItem *file = mModel.item(i, 0);
        if (!file) {
            continue;
        }

        //Get the amount of errors this file contains
        const int errorcount = file->rowCount();

        for (int j = 0; j < errorcount; j++) {
            //Get the error itself
            QStandardItem *child = file->child(j, 0);
            if (!child) {
                continue;
            }

            QVariantMap userdata = child->data().toMap();
            if (userdata[ERRORID].toString() == messageId) {
                userdata[HIDE] = true;
                child->setData(QVariant(userdata));
            }
        }
    }

    refreshTree();
    emit resultsHidden(true);
}

void ResultsTree::suppressSelectedIds()
{
    if (!mSelectionModel)
        return;

    QSet<QString> selectedIds;
    for (QModelIndex index : mSelectionModel->selectedRows()) {
        QStandardItem *item = mModel.itemFromIndex(index);
        if (!item->parent())
            continue;
        if (item->parent()->parent())
            item = item->parent();
        QVariantMap data = item->data().toMap();
        if (!data.contains("id"))
            continue;
        selectedIds << data[ERRORID].toString();
    }

    // delete all errors with selected message Ids
    for (int i = 0; i < mModel.rowCount(); i++) {
        QStandardItem * const file = mModel.item(i, 0);
        for (int j = 0; j < file->rowCount();) {
            QStandardItem *errorItem = file->child(j, 0);
            QVariantMap userdata = errorItem->data().toMap();
            if (selectedIds.contains(userdata[ERRORID].toString())) {
                file->removeRow(j);
            } else {
                j++;
            }
        }
        if (file->rowCount() == 0)
            mModel.removeRow(file->row());
    }


    emit suppressIds(selectedIds.values());
}

void ResultsTree::suppressHash()
{
    if (!mSelectionModel)
        return;

    // Extract selected warnings
    QSet<QStandardItem *> selectedWarnings;
    for (QModelIndex index : mSelectionModel->selectedRows()) {
        QStandardItem *item = mModel.itemFromIndex(index);
        if (!item->parent())
            continue;
        while (item->parent()->parent())
            item = item->parent();
        selectedWarnings.insert(item);
    }

    bool changed = false;
    ProjectFile *projectFile = ProjectFile::getActiveProject();
    for (QStandardItem *item: selectedWarnings) {
        QStandardItem *fileItem = item->parent();
        const QVariantMap data = item->data().toMap();
        if (projectFile && data.contains(HASH)) {
            Suppressions::Suppression suppression;
            suppression.hash = data[HASH].toULongLong();
            suppression.errorId = data[ERRORID].toString().toStdString();
            suppression.fileName = data[FILENAME].toString().toStdString();
            suppression.lineNumber = data[LINE].toInt();
            projectFile->addSuppression(suppression);
            changed = true;
        }
        fileItem->removeRow(item->row());
        if (fileItem->rowCount() == 0)
            mModel.removeRow(fileItem->row());
    }

    if (changed)
        projectFile->write();
}

void ResultsTree::openContainingFolder()
{
    QString filePath = getFilePath(mContextItem, true);
    if (!filePath.isEmpty()) {
        filePath = QFileInfo(filePath).absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}

void ResultsTree::tagSelectedItems(const QString &tag)
{
    if (!mSelectionModel)
        return;
    bool isTagged = false;
    ProjectFile *currentProject = ProjectFile::getActiveProject();
    for (QModelIndex index : mSelectionModel->selectedRows()) {
        QStandardItem *item = mModel.itemFromIndex(index);
        QVariantMap data = item->data().toMap();
        if (data.contains("tags")) {
            data[TAGS] = tag;
            item->setData(QVariant(data));
            item->parent()->child(index.row(), COLUMN_TAGS)->setText(tag);
            if (currentProject && data.contains(HASH)) {
                isTagged = true;
                currentProject->setWarningTags(data[HASH].toULongLong(), tag);
            }
        }
    }
    if (isTagged)
        currentProject->write();
}

void ResultsTree::context(int application)
{
    startApplication(mContextItem, application);
}

void ResultsTree::quickStartApplication(const QModelIndex &index)
{
    startApplication(mModel.itemFromIndex(index));
}

QString ResultsTree::getFilePath(QStandardItem *target, bool fullPath)
{
    if (target) {
        // Make sure we are working with the first column
        if (target->column() != 0)
            target = target->parent()->child(target->row(), 0);

        QVariantMap data = target->data().toMap();
        QString pathStr;

        //Replace (file) with filename
        QString file = data[FILENAME].toString();
        pathStr = QDir::toNativeSeparators(file);
        if (!fullPath) {
            QFileInfo fi(pathStr);
            pathStr = fi.fileName();
        }

        return pathStr;
    }

    return QString();
}

QString ResultsTree::severityToIcon(Severity::SeverityType severity)
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
        return QString();
    }
}

void ResultsTree::saveResults(Report *report) const
{
    report->writeHeader();

    for (int i = 0; i < mModel.rowCount(); i++) {
        if (mSaveAllErrors || !isRowHidden(i, QModelIndex()))
            saveErrors(report, mModel.item(i, 0));
    }

    report->writeFooter();
}

void ResultsTree::saveErrors(Report *report, const QStandardItem *fileItem) const
{
    if (!fileItem) {
        return;
    }

    for (int i = 0; i < fileItem->rowCount(); i++) {
        const QStandardItem *error = fileItem->child(i, 0);

        if (!error) {
            continue;
        }

        if (isRowHidden(i, fileItem->index()) && !mSaveAllErrors) {
            continue;
        }

        ErrorItem item;
        readErrorItem(error, &item);

        report->writeError(item);
    }
}

static int indexOf(const QList<ErrorItem> &list, const ErrorItem &item)
{
    for (int i = 0; i < list.size(); i++) {
        if (ErrorItem::sameCID(item, list[i])) {
            return i;
        }
    }
    return -1;
}

void ResultsTree::updateFromOldReport(const QString &filename)
{
    QList<ErrorItem> oldErrors;
    XmlReportV2 oldReport(filename);
    if (oldReport.open()) {
        oldErrors = oldReport.read();
        oldReport.close();
    }

    // Read current results..
    for (int i = 0; i < mModel.rowCount(); i++) {
        QStandardItem *fileItem = mModel.item(i,0);
        for (int j = 0; j < fileItem->rowCount(); j++) {
            QStandardItem *error = fileItem->child(j,0);
            ErrorItem errorItem;
            readErrorItem(error, &errorItem);
            const int oldErrorIndex = indexOf(oldErrors, errorItem);
            QVariantMap data = error->data().toMap();

            // New error .. set the "sinceDate" property
            if (oldErrorIndex >= 0 && !oldErrors[oldErrorIndex].sinceDate.isEmpty()) {
                data[SINCEDATE] = oldErrors[oldErrorIndex].sinceDate;
                error->setData(data);
                fileItem->child(j, COLUMN_SINCE_DATE)->setText(oldErrors[oldErrorIndex].sinceDate);
            } else if (oldErrorIndex < 0 || data[SINCEDATE].toString().isEmpty()) {
                const QString sinceDate = QLocale::system().dateFormat(QLocale::ShortFormat);
                data[SINCEDATE] = sinceDate;
                error->setData(data);
                fileItem->child(j, COLUMN_SINCE_DATE)->setText(sinceDate);
                if (oldErrorIndex < 0)
                    continue;
            }

            if (!errorItem.tags.isEmpty())
                continue;

            const ErrorItem &oldErrorItem = oldErrors[oldErrorIndex];
            data[TAGS] = oldErrorItem.tags;
            error->setData(data);
        }
    }
}

void ResultsTree::readErrorItem(const QStandardItem *error, ErrorItem *item) const
{
    // Get error's user data
    QVariantMap data = error->data().toMap();

    item->severity = ShowTypes::ShowTypeToSeverity(ShowTypes::VariantToShowType(data[SEVERITY]));
    item->summary = data[SUMMARY].toString();
    item->message = data[MESSAGE].toString();
    item->errorId = data[ERRORID].toString();
    item->cwe = data[CWE].toInt();
    item->hash = data[HASH].toULongLong();
    item->inconclusive = data[INCONCLUSIVE].toBool();
    item->file0 = data[FILE0].toString();
    item->sinceDate = data[SINCEDATE].toString();
    item->tags = data[TAGS].toString();

    if (error->rowCount() == 0) {
        QErrorPathItem e;
        e.file = stripPath(data[FILENAME].toString(), true);
        e.line = data[LINE].toInt();
        e.info = data[MESSAGE].toString();
        item->errorPath << e;
    }

    for (int j = 0; j < error->rowCount(); j++) {
        const QStandardItem *child_error = error->child(j, 0);
        //Get error's user data
        QVariant child_userdata = child_error->data();
        //Convert it to QVariantMap
        QVariantMap child_data = child_userdata.toMap();

        QErrorPathItem e;
        e.file = stripPath(child_data[FILENAME].toString(), true);
        e.line = child_data[LINE].toInt();
        e.info = child_data[MESSAGE].toString();
        item->errorPath << e;
    }
}

void ResultsTree::updateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors,
                                 bool showErrorId,
                                 bool showInconclusive)
{
    if (mShowFullPath != showFullPath) {
        mShowFullPath = showFullPath;
        refreshFilePaths();
    }

    mSaveFullPath = saveFullPath;
    mSaveAllErrors = saveAllErrors;

    showIdColumn(showErrorId);
    showInconclusiveColumn(showInconclusive);
}

void ResultsTree::setCheckDirectory(const QString &dir)
{
    mCheckPath = dir;
}


QString ResultsTree::getCheckDirectory()
{
    return mCheckPath;
}

QString ResultsTree::stripPath(const QString &path, bool saving) const
{
    if ((!saving && mShowFullPath) || (saving && mSaveFullPath)) {
        return QString(path);
    }

    QDir dir(mCheckPath);
    return dir.relativeFilePath(path);
}

void ResultsTree::refreshFilePaths(QStandardItem *item)
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
        QString file = data[FILENAME].toString();

        //Update this error's text
        error->setText(stripPath(file, false));

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
                QString child_files = child_data[FILENAME].toString();
                //Update file's path
                child->setText(stripPath(child_files, false));
            }
        }

        //if the main file hasn't been updated yet, update it now
        if (!updated) {
            updated = true;
            item->setText(error->text());
        }

    }
}

void ResultsTree::refreshFilePaths()
{
    qDebug("Refreshing file paths");

    //Go through all file items (these are parent items that contain the errors)
    for (int i = 0; i < mModel.rowCount(); i++) {
        refreshFilePaths(mModel.item(i, 0));
    }
}

bool ResultsTree::hasVisibleResults() const
{
    return mVisibleErrors;
}

bool ResultsTree::hasResults() const
{
    return mModel.rowCount() > 0;
}

void ResultsTree::translate()
{
    QStringList labels;
    labels << tr("File") << tr("Severity") << tr("Line") << tr("Id") << tr("Inconclusive") << tr("Summary") << tr("Since date") << tr("Tag");
    mModel.setHorizontalHeaderLabels(labels);
    //TODO go through all the errors in the tree and translate severity and message
}

void ResultsTree::showIdColumn(bool show)
{
    mShowErrorId = show;
    if (show)
        showColumn(3);
    else
        hideColumn(3);
}

void ResultsTree::showInconclusiveColumn(bool show)
{
    if (show)
        showColumn(4);
    else
        hideColumn(4);
}

void ResultsTree::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    emit treeSelectionChanged(current);
}

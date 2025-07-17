/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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
#include "checkers.h"
#include "common.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "path.h"
#include "projectfile.h"
#include "report.h"
#include "showtypes.h"
#include "suppressions.h"
#include "threadhandler.h"
#include "xmlreportv2.h"

#include <algorithm>
#include <utility>

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
#include <QObject>
#include <QProcess>
#include <QSet>
#include <QSettings>
#include <QSignalMapper>
#include <QStandardItem>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <Qt>
#include <QHeaderView>

static constexpr char COLUMN[] = "column";
static constexpr char CWE[] = "cwe";
static constexpr char ERRORID[] = "id";
static constexpr char FILENAME[] = "file";
static constexpr char FILE0[] = "file0";
static constexpr char HASH[] = "hash";
static constexpr char HIDE[] = "hide";
static constexpr char INCONCLUSIVE[] = "inconclusive";
static constexpr char LINE[] = "line";
static constexpr char MESSAGE[] = "message";
static constexpr char REMARK[] = "remark";
static constexpr char SEVERITY[] = "severity";
static constexpr char SINCEDATE[] = "sinceDate";
static constexpr char SYMBOLNAMES[] = "symbolNames";
static constexpr char SUMMARY[] = "summary";
static constexpr char TAGS[] = "tags";

// These must match column headers given in ResultsTree::translate()
static constexpr int COLUMN_FILE                  = 0;
static constexpr int COLUMN_LINE                  = 1;
static constexpr int COLUMN_SEVERITY              = 2;
static constexpr int COLUMN_MISRA_CLASSIFICATION  = 3;
static constexpr int COLUMN_CERT_LEVEL            = 4;
static constexpr int COLUMN_INCONCLUSIVE          = 5;
static constexpr int COLUMN_SUMMARY               = 6;
static constexpr int COLUMN_ID                    = 7;
static constexpr int COLUMN_MISRA_GUIDELINE       = 8;
static constexpr int COLUMN_CERT_RULE             = 9;
static constexpr int COLUMN_SINCE_DATE            = 10;
static constexpr int COLUMN_TAGS                  = 11;
static constexpr int COLUMN_CWE                   = 12;

static QString getGuideline(ReportType reportType, const std::map<std::string, std::string> &guidelineMapping,
                            const QString& errorId, Severity severity) {
    return QString::fromStdString(getGuideline(errorId.toStdString(),
                                               reportType, guidelineMapping,
                                               severity));
}

static QString getClassification(ReportType reportType, const QString& guideline) {
    return QString::fromStdString(getClassification(guideline.toStdString(), reportType));
}

static Severity getSeverityFromClassification(const QString &c) {
    if (c == checkers::Man)
        return Severity::error;
    if (c == checkers::Req)
        return Severity::warning;
    if (c == checkers::Adv)
        return Severity::style;
    if (c == checkers::Doc)
        return Severity::information;
    if (c == "L1")
        return Severity::error;
    if (c == "L2")
        return Severity::warning;
    if (c == "L3")
        return Severity::style;
    return Severity::none;
}

static QStringList getLabels() {
    return QStringList{
        QObject::tr("File"),
        QObject::tr("Line"),
        QObject::tr("Severity"),
        QObject::tr("Classification"),
        QObject::tr("Level"),
        QObject::tr("Inconclusive"),
        QObject::tr("Summary"),
        QObject::tr("Id"),
        QObject::tr("Guideline"),
        QObject::tr("Rule"),
        QObject::tr("Since date"),
        QObject::tr("Tags"),
        QObject::tr("CWE")};
}

ResultsTree::ResultsTree(QWidget * parent) :
    QTreeView(parent)
{
    setModel(&mModel);
    translate(); // Adds columns to grid
    clear();
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

void ResultsTree::setReportType(ReportType reportType) {
    mReportType = reportType;

    mGuideline = createGuidelineMapping(reportType);

    for (int i = 0; i < mModel.rowCount(); ++i) {
        const QStandardItem *fileItem = mModel.item(i, COLUMN_FILE);
        if (!fileItem)
            continue;
        for (int j = 0; j < fileItem->rowCount(); ++j) {
            const auto& childdata = fileItem->child(j,0)->data().toMap();
            const QString& errorId = childdata[ERRORID].toString();
            Severity severity = ShowTypes::ShowTypeToSeverity(ShowTypes::VariantToShowType(childdata[SEVERITY]));
            const QString& guideline = getGuideline(mReportType, mGuideline, errorId, severity);
            const QString& classification = getClassification(mReportType, guideline);
            fileItem->child(j, COLUMN_CERT_LEVEL)->setText(classification);
            fileItem->child(j, COLUMN_CERT_RULE)->setText(guideline);
            fileItem->child(j, COLUMN_MISRA_CLASSIFICATION)->setText(classification);
            fileItem->child(j, COLUMN_MISRA_GUIDELINE)->setText(guideline);
        }
    }

    if (isAutosarMisraReport()) {
        showColumn(COLUMN_MISRA_CLASSIFICATION);
        showColumn(COLUMN_MISRA_GUIDELINE);
    } else {
        hideColumn(COLUMN_MISRA_CLASSIFICATION);
        hideColumn(COLUMN_MISRA_GUIDELINE);
    }

    if (isCertReport()) {
        showColumn(COLUMN_CERT_LEVEL);
        showColumn(COLUMN_CERT_RULE);
    } else {
        hideColumn(COLUMN_CERT_LEVEL);
        hideColumn(COLUMN_CERT_RULE);
    }

    if (mReportType == ReportType::normal) {
        showColumn(COLUMN_SEVERITY);
    } else {
        hideColumn(COLUMN_SEVERITY);
    }

    refreshTree();
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
    auto *item = new QStandardItem(name);
    item->setData(name, Qt::ToolTipRole);
    item->setEditable(false);
    return item;
}

QStandardItem *ResultsTree::createCheckboxItem(bool checked)
{
    auto *item = new QStandardItem;
    item->setCheckable(true);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    item->setEnabled(false);
    return item;
}

QStandardItem *ResultsTree::createLineNumberItem(const QString &linenumber)
{
    auto *item = new QStandardItem();
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

    bool showItem = true;

    // Ids that are temporarily hidden..
    if (mHiddenMessageId.contains(item.errorId))
        showItem = false;

    //If specified, filter on summary, message, filename, and id
    if (showItem && !mFilter.isEmpty()) {
        if (!item.summary.contains(mFilter, Qt::CaseInsensitive) &&
            !item.message.contains(mFilter, Qt::CaseInsensitive) &&
            !item.errorPath.back().file.contains(mFilter, Qt::CaseInsensitive) &&
            !item.errorId.contains(mFilter, Qt::CaseInsensitive)) {
            showItem = false;
        }
    }

    if (showItem) {
        if (mReportType == ReportType::normal)
            showItem = mShowSeverities.isShown(item.severity);
        else {
            const QString& guideline = getGuideline(mReportType, mGuideline, item.errorId, item.severity);
            const QString& classification = getClassification(mReportType, guideline);
            showItem = !classification.isEmpty() && mShowSeverities.isShown(getSeverityFromClassification(classification));
        }
    }

    // if there is at least one error that is not hidden, we have a visible error
    mVisibleErrors |= showItem;

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
    line.remark = item.remark;
    //Create the base item for the error and ensure it has a proper
    //file item as a parent
    QStandardItem* fileItem = ensureFileItem(loc.file, item.file0, !showItem);
    QStandardItem* stditem = addBacktraceFiles(fileItem,
                                               line,
                                               !showItem,
                                               severityToIcon(line.severity),
                                               false);

    if (!stditem)
        return false;

    //Add user data to that item
    QMap<QString, QVariant> itemdata;
    itemdata[SEVERITY]  = ShowTypes::SeverityToShowType(item.severity);
    itemdata[SUMMARY] = item.summary;
    itemdata[MESSAGE]  = item.message;
    itemdata[FILENAME]  = loc.file;
    itemdata[LINE]  = loc.line;
    itemdata[COLUMN] = loc.column;
    itemdata[ERRORID]  = item.errorId;
    itemdata[CWE] = item.cwe;
    itemdata[HASH] = item.hash;
    itemdata[INCONCLUSIVE] = item.inconclusive;
    itemdata[FILE0] = stripPath(item.file0, true);
    itemdata[SINCEDATE] = item.sinceDate;
    itemdata[SYMBOLNAMES] = item.symbolNames;
    itemdata[TAGS] = line.tags;
    itemdata[REMARK] = line.remark;
    itemdata[HIDE] = false;
    stditem->setData(QVariant(itemdata));

    //Add backtrace files as children
    if (item.errorPath.size() > 1) {
        for (int i = 0; i < item.errorPath.size(); i++) {
            const QErrorPathItem &e = item.errorPath[i];
            line.file = e.file;
            line.line = e.line;
            line.message = line.summary = e.info;
            QStandardItem *child_item = addBacktraceFiles(stditem,
                                                          line,
                                                          false,
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

    return true;
}

QStandardItem *ResultsTree::addBacktraceFiles(QStandardItem *parent,
                                              const ErrorLine &item,
                                              const bool hide,
                                              const QString &icon,
                                              bool childOfMessage)
{
    if (!parent)
        return nullptr;

    //TODO message has parameter names so we'll need changes to the core
    //cppcheck so we can get proper translations

    const QString itemSeverity = childOfMessage ? tr("note") : severityToTranslatedString(item.severity);

    // Check for duplicate rows and don't add them if found
    for (int i = 0; i < parent->rowCount(); i++) {
        // The first column is the file name and is always the same

        // the third column is the line number so check it first
        if (parent->child(i, COLUMN_LINE)->text() == QString::number(item.line)) {
            // the second column is the severity so check it next
            if (parent->child(i, COLUMN_SEVERITY)->text() == itemSeverity) {
                // the sixth column is the summary so check it last
                if (parent->child(i, COLUMN_SUMMARY)->text() == item.summary) {
                    // this row matches so don't add it
                    return nullptr;
                }
            }
        }
    }

    QMap<int, QStandardItem*> columns;
    const QString guideline = getGuideline(mReportType, mGuideline, item.errorId, item.severity);
    const QString classification = getClassification(mReportType, guideline);
    columns[COLUMN_CERT_LEVEL] = createNormalItem(classification);
    columns[COLUMN_CERT_RULE] = createNormalItem(guideline);
    columns[COLUMN_CWE] = createNormalItem(QString::number(item.cwe));
    columns[COLUMN_FILE] = createNormalItem(QDir::toNativeSeparators(item.file));
    columns[COLUMN_ID] = createNormalItem(childOfMessage ? QString() : item.errorId);
    columns[COLUMN_INCONCLUSIVE] = childOfMessage ? createNormalItem(QString()) : createCheckboxItem(item.inconclusive);
    columns[COLUMN_LINE] = createLineNumberItem(QString::number(item.line));
    columns[COLUMN_MISRA_CLASSIFICATION] = createNormalItem(classification);
    columns[COLUMN_MISRA_GUIDELINE] = createNormalItem(guideline);
    columns[COLUMN_SEVERITY] = createNormalItem(itemSeverity);
    columns[COLUMN_SINCE_DATE] = createNormalItem(item.sinceDate);
    columns[COLUMN_SUMMARY] = createNormalItem(item.summary);
    columns[COLUMN_TAGS] = createNormalItem(item.tags);

    const int numberOfColumns = getLabels().size();
    QList<QStandardItem*> list;
    for (int i = 0; i < numberOfColumns; ++i)
        list << columns[i];

    parent->appendRow(list);

    setRowHidden(parent->rowCount() - 1, parent->index(), hide);

    if (!icon.isEmpty()) {
        list[0]->setIcon(QIcon(icon));
    }

    return list[0];
}

QString ResultsTree::severityToTranslatedString(Severity severity)
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

    case Severity::internal:
        return tr("internal");

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
        if (QString::compare(mModel.item(i, COLUMN_FILE)->text(), name, Qt::CaseInsensitive) == 0)
#else
        if (mModel.item(i, COLUMN_FILE)->text() == name)
#endif
            return mModel.item(i, COLUMN_FILE);
    }
    return nullptr;
}

void ResultsTree::clear()
{
    mModel.removeRows(0, mModel.rowCount());

    if (const ProjectFile *activeProject = ProjectFile::getActiveProject()) {
        hideColumn(COLUMN_SINCE_DATE);
        if (activeProject->getTags().isEmpty())
            hideColumn(COLUMN_TAGS);
        else
            showColumn(COLUMN_TAGS);
    } else {
        hideColumn(COLUMN_SINCE_DATE);
        hideColumn(COLUMN_TAGS);
    }
}

void ResultsTree::clear(const QString &filename)
{
    const QString stripped = stripPath(filename, false);

    for (int i = 0; i < mModel.rowCount(); ++i) {
        const QStandardItem *fileItem = mModel.item(i, COLUMN_FILE);
        if (!fileItem)
            continue;

        QVariantMap fitemdata = fileItem->data().toMap();
        if (stripped == fitemdata[FILENAME].toString() ||
            filename == fitemdata[FILE0].toString()) {
            mModel.removeRow(i);
            break;
        }
    }
}

void ResultsTree::clearRecheckFile(const QString &filename)
{
    for (int i = 0; i < mModel.rowCount(); ++i) {
        const QStandardItem *fileItem = mModel.item(i, COLUMN_FILE);
        if (!fileItem)
            continue;

        QString actualfile((!mCheckPath.isEmpty() && filename.startsWith(mCheckPath)) ? filename.mid(mCheckPath.length() + 1) : filename);
        QVariantMap fitemdata = fileItem->data().toMap();
        QString storedfile = fitemdata[FILENAME].toString();
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

    showIdColumn(mSettings->value(SETTINGS_SHOW_ERROR_ID, true).toBool());
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
        bool showFile = false;

        for (int j = 0; j < errorcount; j++) {
            //Get the error itself
            QStandardItem *child = fileItem->child(j, 0);
            if (!child) {
                continue;
            }

            //Get error's user data and convert it to QVariantMap
            QVariantMap userdata = child->data().toMap();

            //Check if this error should be hidden
            bool hide = userdata[HIDE].toBool() || mHiddenMessageId.contains(userdata[ERRORID].toString());

            if (!hide) {
                if (mReportType == ReportType::normal)
                    hide = !mShowSeverities.isShown(ShowTypes::VariantToShowType(userdata[SEVERITY]));
                else {
                    const QString& classification = fileItem->child(j, COLUMN_MISRA_CLASSIFICATION)->text();
                    hide = classification.isEmpty() || !mShowSeverities.isShown(getSeverityFromClassification(classification));
                }
            }

            // If specified, filter on summary, message, filename, and id
            if (!hide && !mFilter.isEmpty()) {
                if (!userdata[SUMMARY].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !userdata[MESSAGE].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !userdata[FILENAME].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !userdata[ERRORID].toString().contains(mFilter, Qt::CaseInsensitive) &&
                    !fileItem->child(j, COLUMN_MISRA_CLASSIFICATION)->text().contains(mFilter, Qt::CaseInsensitive)) {
                    hide = true;
                }
            }

            // Tool filter
            if (!hide) {
                if (userdata[ERRORID].toString().startsWith("clang"))
                    hide = !mShowClang;
                else
                    hide = !mShowCppcheck;
            }

            if (!hide) {
                showFile = true;
                mVisibleErrors = true;
            }

            //Hide/show accordingly
            setRowHidden(j, fileItem->index(), hide);
        }

        // Show the file if any of it's errors are visible
        setRowHidden(i, QModelIndex(), !showFile);
    }
    sortByColumn(header()->sortIndicatorSection(), header()->sortIndicatorOrder());
}

QStandardItem *ResultsTree::ensureFileItem(const QString &fullpath, const QString &file0, bool hide)
{
    QString name = stripPath(fullpath, false);
    // Since item has path with native separators we must use path with
    // native separators to find it.
    QStandardItem *item = findFileItem(QDir::toNativeSeparators(name));

    if (item) {
        if (!hide)
            setRowHidden(item->row(), QModelIndex(), hide);
        return item;
    }

    // Ensure shown path is with native separators
    name = QDir::toNativeSeparators(name);
    item = createNormalItem(name);
    item->setIcon(QIcon(":images/text-x-generic.png"));

    //Add user data to that item
    QMap<QString, QVariant> itemdata;
    itemdata[FILENAME] = fullpath;
    itemdata[FILE0] = file0;
    item->setData(QVariant(itemdata));
    mModel.appendRow(item);

    setRowHidden(item->row(), QModelIndex(), hide);

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

        //Create a signal mapper so we don't have to store data to class
        //member variables
        QSignalMapper signalMapper;

        if (mContextItem && mApplications->getApplicationCount() > 0 && mContextItem->parent()) {
            //Create an action for the application
            int defaultApplicationIndex = mApplications->getDefaultApplication();
            defaultApplicationIndex = std::max(defaultApplicationIndex, 0);
            const Application& app = mApplications->getApplication(defaultApplicationIndex);
            auto *start = new QAction(app.getName(), &menu);
            if (multipleSelection)
                start->setDisabled(true);

            //Add it to context menu
            menu.addAction(start);

            //Connect the signal to signal mapper
            connect(start, &QAction::triggered, &signalMapper, QOverload<>::of(&QSignalMapper::map));

            //Add a new mapping
            signalMapper.setMapping(start, defaultApplicationIndex);

            connect(&signalMapper, SIGNAL(mapped(int)),
                    this, SLOT(context(int)));
        }

        // Add popup menuitems
        if (mContextItem) {
            if (mApplications->getApplicationCount() > 0) {
                menu.addSeparator();
            }

            int selectedFiles = 0;
            int selectedResults = 0;

            for (auto row : mSelectionModel->selectedRows()) {
                auto *item = mModel.itemFromIndex(row);
                if (!item->parent())
                    selectedFiles++;
                else if (!item->parent()->parent())
                    selectedResults++;
            }

            //Create an action for the application
            auto *recheckAction          = new QAction(tr("Recheck %1 file(s)").arg(selectedFiles), &menu);
            auto *copyAction             = new QAction(tr("Copy"), &menu);
            auto *hide                   = new QAction(tr("Hide %1 result(s)").arg(selectedResults), &menu);
            auto *hideallid              = new QAction(tr("Hide all with id"), &menu);
            auto *opencontainingfolder   = new QAction(tr("Open containing folder"), &menu);

            if (selectedFiles == 0 || mThread->isChecking())
                recheckAction->setDisabled(true);

            if (selectedResults == 0)
                hide->setDisabled(true);

            if (selectedResults == 0 || multipleSelection)
                hideallid->setDisabled(true);

            if (multipleSelection)
                opencontainingfolder->setDisabled(true);

            menu.addAction(recheckAction);
            menu.addSeparator();
            menu.addAction(copyAction);
            menu.addSeparator();
            menu.addAction(hide);
            menu.addAction(hideallid);

            auto *suppress = new QAction(tr("Suppress selected id(s)"), &menu);
            {
                QVariantMap itemdata = mContextItem->data().toMap();
                const QString messageId = itemdata[ERRORID].toString();

                if (selectedResults == 0 || ErrorLogger::isCriticalErrorId(messageId.toStdString()))
                    suppress->setDisabled(true);
            }
            menu.addAction(suppress);
            connect(suppress, &QAction::triggered, this, &ResultsTree::suppressSelectedIds);

            menu.addSeparator();
            menu.addAction(opencontainingfolder);

            connect(recheckAction, &QAction::triggered, this, &ResultsTree::recheckSelectedFiles);
            connect(copyAction, &QAction::triggered, this, &ResultsTree::copy);
            connect(hide, &QAction::triggered, this, &ResultsTree::hideResult);
            connect(hideallid, &QAction::triggered, this, &ResultsTree::hideAllIdResult);
            connect(opencontainingfolder, &QAction::triggered, this, &ResultsTree::openContainingFolder);

            const ProjectFile *currentProject = ProjectFile::getActiveProject();
            if (currentProject && !currentProject->getTags().isEmpty()) {
                menu.addSeparator();
                QMenu *tagMenu = menu.addMenu(tr("Tag"));
                {
                    auto *action = new QAction(tr("No tag"), tagMenu);
                    tagMenu->addAction(action);
                    connect(action, &QAction::triggered, [=]() {
                        tagSelectedItems(QString());
                    });
                }

                for (const QString& tagstr : currentProject->getTags()) {
                    auto *action = new QAction(tagstr, tagMenu);
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
        }
    }
}

void ResultsTree::startApplication(const QStandardItem *target, int application)
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

        QVariantMap targetdata = target->data().toMap();

        //Replace (file) with filename
        QString file = targetdata[FILENAME].toString();
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

        QVariant line = targetdata[LINE];
        params.replace("(line)", QString("%1").arg(line.toInt()), Qt::CaseInsensitive);

        params.replace("(message)", targetdata[MESSAGE].toString(), Qt::CaseInsensitive);
        params.replace("(severity)", targetdata[SEVERITY].toString(), Qt::CaseInsensitive);

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

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        const QString cmdLine = QString("%1 %2").arg(program).arg(params);
#endif

        // this is reported as deprecated in Qt 5.15.2 but no longer in Qt 6
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        SUPPRESS_WARNING_CLANG_PUSH("-Wdeprecated")
        SUPPRESS_WARNING_GCC_PUSH("-Wdeprecated-declarations")
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        const bool success = QProcess::startDetached(cmdLine);
#else
        const bool success = QProcess::startDetached(program, QProcess::splitCommand(params));
#endif

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
    for (const QModelIndex& index : mSelectionModel->selectedRows()) {
        const QStandardItem *item = mModel.itemFromIndex(index);
        if (!item->parent()) {
            text += item->text() + '\n';
            continue;
        }
        if (item->parent()->parent())
            item = item->parent();
        QVariantMap itemdata = item->data().toMap();
        if (!itemdata.contains("id"))
            continue;
        QString inconclusive = itemdata[INCONCLUSIVE].toBool() ? ",inconclusive" : "";
        text += itemdata[FILENAME].toString() + ':' + QString::number(itemdata[LINE].toInt()) + ':' + QString::number(itemdata[COLUMN].toInt())
                + ": "
                + QString::fromStdString(severityToString(ShowTypes::ShowTypeToSeverity(static_cast<ShowTypes::ShowType>(itemdata[SEVERITY].toInt())))) + inconclusive
                + ": "
                + itemdata[MESSAGE].toString()
                + " ["
                + itemdata[ERRORID].toString()
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
        QVariantMap itemdata = item->data().toMap();
        itemdata[HIDE] = true;
        item->setData(QVariant(itemdata));

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
        QVariantMap itemdata = item->data().toMap();
        QString currentFile = itemdata[FILENAME].toString();
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
                if (!itemdata[FILE0].toString().isEmpty() && !selectedItems.contains(itemdata[FILE0].toString())) {
                    selectedItems<<((!mCheckPath.isEmpty() && (itemdata[FILE0].toString().indexOf(mCheckPath) != 0)) ? (mCheckPath + "/" + itemdata[FILE0].toString()) : itemdata[FILE0].toString());
                    if (!selectedItems.contains(fileNameWithCheckPath))
                        selectedItems<<fileNameWithCheckPath;
                }
            } else if (!selectedItems.contains(fileNameWithCheckPath))
                selectedItems<<fileNameWithCheckPath;
        }
    }
    emit checkSelected(std::move(selectedItems));
}

void ResultsTree::hideAllIdResult()
{
    if (!mContextItem || !mContextItem->parent())
        return;

    // Make sure we are working with the first column
    if (mContextItem->column() != 0)
        mContextItem = mContextItem->parent()->child(mContextItem->row(), 0);
    QVariantMap itemdata = mContextItem->data().toMap();

    QString messageId = itemdata[ERRORID].toString();

    mHiddenMessageId.append(messageId);

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
        QVariantMap itemdata = item->data().toMap();
        if (!itemdata.contains("id"))
            continue;
        selectedIds << itemdata[ERRORID].toString();
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
        const QVariantMap itemdata = item->data().toMap();
        if (projectFile && itemdata.contains(HASH)) {
            SuppressionList::Suppression suppression;
            suppression.hash = itemdata[HASH].toULongLong();
            suppression.errorId = itemdata[ERRORID].toString().toStdString();
            suppression.fileName = itemdata[FILENAME].toString().toStdString();
            suppression.lineNumber = itemdata[LINE].toInt();
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
        QVariantMap itemdata = item->data().toMap();
        if (itemdata.contains("tags")) {
            itemdata[TAGS] = tag;
            item->setData(QVariant(itemdata));
            item->parent()->child(index.row(), COLUMN_TAGS)->setText(tag);
            if (currentProject && itemdata.contains(HASH)) {
                isTagged = true;
                currentProject->setWarningTags(itemdata[HASH].toULongLong(), tag);
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

QString ResultsTree::getFilePath(const QStandardItem *target, bool fullPath)
{
    if (target) {
        // Make sure we are working with the first column
        if (target->column() != 0)
            target = target->parent()->child(target->row(), 0);

        QVariantMap targetdata = target->data().toMap();

        //Replace (file) with filename
        QString file = targetdata[FILENAME].toString();
        QString pathStr = QDir::toNativeSeparators(file);
        if (!fullPath) {
            QFileInfo fi(pathStr);
            pathStr = fi.fileName();
        }

        return pathStr;
    }

    return QString();
}

QString ResultsTree::severityToIcon(Severity severity)
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
    auto it = std::find_if(list.cbegin(), list.cend(), [&](const ErrorItem& e) {
        return ErrorItem::sameCID(item, e);
    });
    return it == list.cend() ? -1 : static_cast<int>(std::distance(list.cbegin(), it));
}

void ResultsTree::updateFromOldReport(const QString &filename)
{
    showColumn(COLUMN_SINCE_DATE);

    QList<ErrorItem> oldErrors;
    XmlReportV2 oldReport(filename, QString());
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
            QVariantMap errordata = error->data().toMap();

            // New error .. set the "sinceDate" property
            if (oldErrorIndex >= 0 && !oldErrors[oldErrorIndex].sinceDate.isEmpty()) {
                errordata[SINCEDATE] = oldErrors[oldErrorIndex].sinceDate;
                error->setData(errordata);
                fileItem->child(j, COLUMN_SINCE_DATE)->setText(oldErrors[oldErrorIndex].sinceDate);
            } else if (oldErrorIndex < 0 || errordata[SINCEDATE].toString().isEmpty()) {
                const QString sinceDate = QLocale::system().toString(QDate::currentDate(), QLocale::ShortFormat);
                errordata[SINCEDATE] = sinceDate;
                error->setData(errordata);
                fileItem->child(j, COLUMN_SINCE_DATE)->setText(sinceDate);
                if (oldErrorIndex < 0)
                    continue;
            }

            if (!errorItem.tags.isEmpty())
                continue;

            const ErrorItem &oldErrorItem = oldErrors[oldErrorIndex];
            errordata[TAGS] = oldErrorItem.tags;
            error->setData(errordata);
        }
    }
}

void ResultsTree::readErrorItem(const QStandardItem *error, ErrorItem *item) const
{
    // Get error's user data
    QVariantMap errordata = error->data().toMap();

    item->severity = ShowTypes::ShowTypeToSeverity(ShowTypes::VariantToShowType(errordata[SEVERITY]));
    item->summary = errordata[SUMMARY].toString();
    item->message = errordata[MESSAGE].toString();
    item->errorId = errordata[ERRORID].toString();
    item->cwe = errordata[CWE].toInt();
    item->hash = errordata[HASH].toULongLong();
    item->inconclusive = errordata[INCONCLUSIVE].toBool();
    item->file0 = errordata[FILE0].toString();
    item->sinceDate = errordata[SINCEDATE].toString();
    item->tags = errordata[TAGS].toString();
    item->remark = errordata[REMARK].toString();
    item->classification = error->parent()->child(error->row(), COLUMN_MISRA_CLASSIFICATION)->text();
    item->guideline = error->parent()->child(error->row(), COLUMN_MISRA_GUIDELINE)->text();

    if (error->rowCount() == 0) {
        QErrorPathItem e;
        e.file = stripPath(errordata[FILENAME].toString(), true);
        e.line = errordata[LINE].toInt();
        e.info = errordata[MESSAGE].toString();
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


const QString& ResultsTree::getCheckDirectory() const
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

        //Get error's user data and convert it to QVariantMap
        QVariantMap userdata = error->data().toMap();

        //Get list of files
        QString file = userdata[FILENAME].toString();

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
    mModel.setHorizontalHeaderLabels(getLabels());
    //TODO go through all the errors in the tree and translate severity and message
}

void ResultsTree::showIdColumn(bool show)
{
    mShowErrorId = show;
    if (show)
        showColumn(COLUMN_ID);
    else
        hideColumn(COLUMN_ID);
}

void ResultsTree::showInconclusiveColumn(bool show)
{
    if (show)
        showColumn(COLUMN_INCONCLUSIVE);
    else
        hideColumn(COLUMN_INCONCLUSIVE);
}

void ResultsTree::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    emit treeSelectionChanged(current);
}

bool ResultsTree::isCertReport() const {
    return mReportType == ReportType::certC || mReportType == ReportType::certCpp;
}

bool ResultsTree::isAutosarMisraReport() const {
    return mReportType == ReportType::autosar ||
           mReportType == ReportType::misraC2012 ||
           mReportType == ReportType::misraC2023 ||
           mReportType == ReportType::misraC2025 ||
           mReportType == ReportType::misraCpp2008 ||
           mReportType == ReportType::misraCpp2023;
}

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2017 Cppcheck team.
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

#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVariant>
#include <QString>
#include <QModelIndex>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QSettings>
#include <QDir>
#include <QDate>
#include <QMenu>
#include <QClipboard>
#include "common.h"
#include "erroritem.h"
#include "resultsview.h"
#include "report.h"
#include "txtreport.h"
#include "xmlreport.h"
#include "xmlreportv2.h"
#include "csvreport.h"
#include "printablereport.h"
#include "applicationlist.h"
#include "checkstatistics.h"
#include "path.h"

ResultsView::ResultsView(QWidget * parent) :
    QWidget(parent),
    mShowNoErrorsMessage(true),
    mStatistics(new CheckStatistics(this))
{
    mUI.setupUi(this);

    connect(mUI.mTree, &ResultsTree::resultsHidden, this, &ResultsView::resultsHidden);
    connect(mUI.mTree, &ResultsTree::checkSelected, this, &ResultsView::checkSelected);
    connect(mUI.mTree, &ResultsTree::selectionChanged, this, &ResultsView::updateDetails);
    connect(mUI.mTree, &ResultsTree::tagged, this, &ResultsView::tagged);
    connect(mUI.mTree, &ResultsTree::suppressIds, this, &ResultsView::suppressIds);
    connect(this, &ResultsView::showResults, mUI.mTree, &ResultsTree::showResults);
    connect(this, &ResultsView::showCppcheckResults, mUI.mTree, &ResultsTree::showCppcheckResults);
    connect(this, &ResultsView::showClangResults, mUI.mTree, &ResultsTree::showClangResults);
    connect(this, &ResultsView::collapseAllResults, mUI.mTree, &ResultsTree::collapseAll);
    connect(this, &ResultsView::expandAllResults, mUI.mTree, &ResultsTree::expandAll);
    connect(this, &ResultsView::showHiddenResults, mUI.mTree, &ResultsTree::showHiddenResults);

    mUI.mListLog->setContextMenuPolicy(Qt::CustomContextMenu);
}

void ResultsView::initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler)
{
    mUI.mProgress->setMinimum(0);
    mUI.mProgress->setVisible(false);

    QByteArray state = settings->value(SETTINGS_MAINWND_SPLITTER_STATE).toByteArray();
    mUI.mVerticalSplitter->restoreState(state);
    mShowNoErrorsMessage = settings->value(SETTINGS_SHOW_NO_ERRORS, true).toBool();

    mUI.mTree->initialize(settings, list, checkThreadHandler);
}

ResultsView::~ResultsView()
{
    //dtor
}

void ResultsView::clear(bool results)
{
    if (results) {
        mUI.mTree->clear();
    }

    mUI.mDetails->setText(QString());

    mStatistics->clear();

    //Clear the progressbar
    mUI.mProgress->setMaximum(PROGRESS_MAX);
    mUI.mProgress->setValue(0);
    mUI.mProgress->setFormat("%p%");
}

void ResultsView::clear(const QString &filename)
{
    mUI.mTree->clear(filename);
}

void ResultsView::clearRecheckFile(const QString &filename)
{
    mUI.mTree->clearRecheckFile(filename);
}

void ResultsView::progress(int value, const QString& description)
{
    mUI.mProgress->setValue(value);
    mUI.mProgress->setFormat(QString("%p% (%1)").arg(description));
}

void ResultsView::error(const ErrorItem &item)
{
    if (mUI.mTree->addErrorItem(item)) {
        emit gotResults();
        mStatistics->addItem(item.tool(), ShowTypes::SeverityToShowType(item.severity));
    }
}

void ResultsView::filterResults(const QString& filter)
{
    mUI.mTree->filterResults(filter);
}

void ResultsView::saveStatistics(const QString &filename) const
{
    QFile f(filename);
    if (!f.open(QIODevice::Text | QIODevice::Append))
        return;
    QTextStream ts(&f);
    ts <<  '[' << QDate::currentDate().toString("dd.MM.yyyy") << "]\n";
    ts << QDateTime::currentMSecsSinceEpoch() << '\n';
    foreach (QString tool, mStatistics->getTools()) {
        ts << tool << "-error:" << mStatistics->getCount(tool, ShowTypes::ShowErrors) << '\n';
        ts << tool << "-warning:" << mStatistics->getCount(tool, ShowTypes::ShowWarnings) << '\n';
        ts << tool << "-style:" << mStatistics->getCount(tool, ShowTypes::ShowStyle) << '\n';
        ts << tool << "-performance:" << mStatistics->getCount(tool, ShowTypes::ShowPerformance) << '\n';
        ts << tool << "-portability:" << mStatistics->getCount(tool, ShowTypes::ShowPortability) << '\n';
    }
}

void ResultsView::updateFromOldReport(const QString &filename) const
{
    mUI.mTree->updateFromOldReport(filename);
}

void ResultsView::save(const QString &filename, Report::Type type) const
{
    if (!hasResults()) {
        QMessageBox msgBox;
        msgBox.setText(tr("No errors found, nothing to save."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }

    Report *report = NULL;

    switch (type) {
    case Report::CSV:
        report = new CsvReport(filename);
        break;
    case Report::TXT:
        report = new TxtReport(filename);
        break;
    case Report::XMLV2:
        report = new XmlReportV2(filename);
        break;
    }

    if (report) {
        if (report->create())
            mUI.mTree->saveResults(report);
        else {
            QMessageBox msgBox;
            msgBox.setText(tr("Failed to save the report."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
        delete report;
        report = NULL;
    } else {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to save the report."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void ResultsView::print()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Report"));
    if (dialog.exec() != QDialog::Accepted)
        return;

    print(&printer);
}

void ResultsView::printPreview()
{
    QPrinter printer;
    QPrintPreviewDialog dialog(&printer, this);
    connect(&dialog, SIGNAL(paintRequested(QPrinter*)), SLOT(print(QPrinter*)));
    dialog.exec();
}

void ResultsView::print(QPrinter* printer)
{
    if (!hasResults()) {
        QMessageBox msgBox;
        msgBox.setText(tr("No errors found, nothing to print."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    PrintableReport report;
    mUI.mTree->saveResults(&report);
    QTextDocument doc(report.getFormattedReportText());
    doc.print(printer);
}

void ResultsView::updateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors,
                                 bool showNoErrorsMessage,
                                 bool showErrorId,
                                 bool showInconclusive)
{
    mUI.mTree->updateSettings(showFullPath, saveFullPath, saveAllErrors, showErrorId, showInconclusive);
    mShowNoErrorsMessage = showNoErrorsMessage;
}

void ResultsView::setCheckDirectory(const QString &dir)
{
    mUI.mTree->setCheckDirectory(dir);
}

QString ResultsView::getCheckDirectory(void)
{
    return mUI.mTree->getCheckDirectory();
}

void ResultsView::checkingStarted(int count)
{
    mUI.mProgress->setVisible(true);
    mUI.mProgress->setMaximum(PROGRESS_MAX);
    mUI.mProgress->setValue(0);
    mUI.mProgress->setFormat(tr("%p% (%1 of %2 files checked)").arg(0).arg(count));
}

void ResultsView::checkingFinished()
{
    mUI.mProgress->setVisible(false);
    mUI.mProgress->setFormat("%p%");

    //Should we inform user of non visible/not found errors?
    if (mShowNoErrorsMessage) {
        //Tell user that we found no errors
        if (!hasResults()) {
            QMessageBox msg(QMessageBox::Information,
                            tr("Cppcheck"),
                            tr("No errors found."),
                            QMessageBox::Ok,
                            this);

            msg.exec();
        } //If we have errors but they aren't visible, tell user about it
        else if (!mUI.mTree->hasVisibleResults()) {
            QString text = tr("Errors were found, but they are configured to be hidden.\n"\
                              "To toggle what kind of errors are shown, open view menu.");
            QMessageBox msg(QMessageBox::Information,
                            tr("Cppcheck"),
                            text,
                            QMessageBox::Ok,
                            this);

            msg.exec();
        }
    }
}

bool ResultsView::hasVisibleResults() const
{
    return mUI.mTree->hasVisibleResults();
}

bool ResultsView::hasResults() const
{
    return mUI.mTree->hasResults();
}

void ResultsView::saveSettings(QSettings *settings)
{
    mUI.mTree->saveSettings();
    QByteArray state = mUI.mVerticalSplitter->saveState();
    settings->setValue(SETTINGS_MAINWND_SPLITTER_STATE, state);
    mUI.mVerticalSplitter->restoreState(state);
}

void ResultsView::translate()
{
    mUI.mTree->translate();
}

void ResultsView::disableProgressbar()
{
    mUI.mProgress->setEnabled(false);
}

void ResultsView::readErrorsXml(const QString &filename)
{
    const int version = XmlReport::determineVersion(filename);
    if (version == 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to read the report."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    if (version == 1) {
        QMessageBox msgBox;
        msgBox.setText(tr("XML format version 1 is no longer supported."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    XmlReport *report = new XmlReportV2(filename);

    QList<ErrorItem> errors;
    if (report) {
        if (report->open())
            errors = report->read();
        else {
            QMessageBox msgBox;
            msgBox.setText(tr("Failed to read the report."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
        delete report;
        report = NULL;
    } else {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to read the report."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }

    ErrorItem item;
    foreach (item, errors) {
        mUI.mTree->addErrorItem(item);
    }
    mUI.mTree->setCheckDirectory(QString());
}

void ResultsView::updateDetails(const QModelIndex &index)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(mUI.mTree->model());
    QStandardItem *item = model->itemFromIndex(index);

    if (!item) {
        mUI.mDetails->setText(QString());
        return;
    }

    // Make sure we are working with the first column
    if (item->parent() && item->column() != 0)
        item = item->parent()->child(item->row(), 0);

    QVariantMap data = item->data().toMap();

    // If there is no severity data then it is a parent item without summary and message
    if (!data.contains("severity")) {
        mUI.mDetails->setText(QString());
        return;
    }

    const QString summary = data["summary"].toString();
    const QString message = data["message"].toString();
    QString formattedMsg = QString("%1: %2\n%3: %4")
                           .arg(tr("Summary")).arg(summary)
                           .arg(tr("Message")).arg(message);

    const QString file0 = data["file0"].toString();
    if (!file0.isEmpty() && Path::isHeader(data["file"].toString().toStdString()))
        formattedMsg += QString("\n\n%1: %2").arg(tr("First included by")).arg(QDir::toNativeSeparators(file0));

    if (mUI.mTree->showIdColumn())
        formattedMsg.prepend(tr("Id") + ": " + data["id"].toString() + "\n");
    mUI.mDetails->setText(formattedMsg);
}

void ResultsView::log(const QString &str)
{
    mUI.mListLog->addItem(str);
}

void ResultsView::debugError(const ErrorItem &item)
{
    mUI.mListLog->addItem(item.ToString());
}

void ResultsView::logClear()
{
    mUI.mListLog->clear();
}

void ResultsView::logCopyEntry()
{
    const QListWidgetItem * item = mUI.mListLog->currentItem();
    if (nullptr != item) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(item->text());
    }
}

void ResultsView::logCopyComplete()
{
    QString logText;
    for (int i=0; i < mUI.mListLog->count(); ++i) {
        const QListWidgetItem * item = mUI.mListLog->item(i);
        if (nullptr != item) {
            logText += item->text();
        }
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(logText);
}

void ResultsView::on_mListLog_customContextMenuRequested(const QPoint &pos)
{
    if (mUI.mListLog->count() > 0) {
        QPoint globalPos = mUI.mListLog->mapToGlobal(pos);

        QMenu contextMenu;
        contextMenu.addAction(tr("Clear Log"), this, SLOT(logClear()));
        contextMenu.addAction(tr("Copy this Log entry"), this, SLOT(logCopyEntry()));
        contextMenu.addAction(tr("Copy complete Log"), this, SLOT(logCopyComplete()));

        contextMenu.exec(globalPos);
    }
}

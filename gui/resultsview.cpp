/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "common.h"
#include "erroritem.h"
#include "resultsview.h"
#include "report.h"
#include "txtreport.h"
#include "xmlreport.h"
#include "xmlreportv1.h"
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

    connect(mUI.mTree, SIGNAL(ResultsHidden(bool)), this, SIGNAL(ResultsHidden(bool)));
    connect(mUI.mTree, SIGNAL(CheckSelected(QStringList)), this, SIGNAL(CheckSelected(QStringList)));
    connect(mUI.mTree, SIGNAL(SelectionChanged(const QModelIndex &)), this, SLOT(UpdateDetails(const QModelIndex &)));
}

void ResultsView::Initialize(QSettings *settings, ApplicationList *list, ThreadHandler *checkThreadHandler)
{
    mUI.mProgress->setMinimum(0);
    mUI.mProgress->setVisible(false);

    QByteArray state = settings->value(SETTINGS_MAINWND_SPLITTER_STATE).toByteArray();
    mUI.mVerticalSplitter->restoreState(state);
    mShowNoErrorsMessage = settings->value(SETTINGS_SHOW_NO_ERRORS, true).toBool();

    mUI.mTree->Initialize(settings, list, checkThreadHandler);
}

ResultsView::~ResultsView()
{
    //dtor
}

void ResultsView::Clear(bool results)
{
    if (results) {
        mUI.mTree->Clear();
    }

    mUI.mDetails->setText("");

    mStatistics->Clear();

    //Clear the progressbar
    mUI.mProgress->setMaximum(PROGRESS_MAX);
    mUI.mProgress->setValue(0);
    mUI.mProgress->setFormat("%p%");
}

void ResultsView::Clear(const QString &filename)
{
    mUI.mTree->Clear(filename);
}

void ResultsView::ClearRecheckFile(const QString &filename)
{
    mUI.mTree->ClearRecheckFile(filename);
}

void ResultsView::Progress(int value, const QString& description)
{
    mUI.mProgress->setValue(value);
    mUI.mProgress->setFormat(QString("%p% (%1)").arg(description));
}

void ResultsView::Error(const ErrorItem &item)
{
    if (mUI.mTree->AddErrorItem(item)) {
        emit GotResults();
        mStatistics->AddItem(ShowTypes::SeverityToShowType(item.severity));
    }
}

void ResultsView::ShowResults(ShowTypes::ShowType type, bool show)
{
    mUI.mTree->ShowResults(type, show);
}

void ResultsView::CollapseAllResults()
{
    mUI.mTree->collapseAll();
}

void ResultsView::ExpandAllResults()
{
    mUI.mTree->expandAll();
}

void ResultsView::ShowHiddenResults()
{
    mUI.mTree->ShowHiddenResults();
}

void ResultsView::FilterResults(const QString& filter)
{
    mUI.mTree->FilterResults(filter);
}

void ResultsView::Save(const QString &filename, Report::Type type) const
{
    if (!HasResults()) {
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
    case Report::XML:
        report = new XmlReportV1(filename);
        break;
    case Report::XMLV2:
        report = new XmlReportV2(filename);
        break;
    }

    if (report) {
        if (report->Create())
            mUI.mTree->SaveResults(report);
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

void ResultsView::Print()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Report"));
    if (dialog.exec() != QDialog::Accepted)
        return;

    Print(&printer);
}

void ResultsView::PrintPreview()
{
    QPrinter printer;
    QPrintPreviewDialog dialog(&printer, this);
    connect(&dialog, SIGNAL(paintRequested(QPrinter*)), SLOT(Print(QPrinter*)));
    dialog.exec();
}

void ResultsView::Print(QPrinter* printer)
{
    if (!HasResults()) {
        QMessageBox msgBox;
        msgBox.setText(tr("No errors found, nothing to print."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    PrintableReport report;
    mUI.mTree->SaveResults(&report);
    QTextDocument doc(report.GetFormattedReportText());
    doc.print(printer);
}

void ResultsView::UpdateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors,
                                 bool showNoErrorsMessage,
                                 bool showErrorId,
                                 bool showInconclusive)
{
    mUI.mTree->UpdateSettings(showFullPath, saveFullPath, saveAllErrors, showErrorId, showInconclusive);
    mShowNoErrorsMessage = showNoErrorsMessage;
}

void ResultsView::SetCheckDirectory(const QString &dir)
{
    mUI.mTree->SetCheckDirectory(dir);
}

QString ResultsView::GetCheckDirectory(void)
{
    return mUI.mTree->GetCheckDirectory();
}

void ResultsView::CheckingStarted(int count)
{
    mUI.mProgress->setVisible(true);
    mUI.mProgress->setMaximum(PROGRESS_MAX);
    mUI.mProgress->setValue(0);
    mUI.mProgress->setFormat(tr("%p% (%1 of %2 files checked)").arg(0).arg(count));
}

void ResultsView::CheckingFinished()
{
    mUI.mProgress->setVisible(false);
    mUI.mProgress->setFormat("%p%");

    //Should we inform user of non visible/not found errors?
    if (mShowNoErrorsMessage) {
        //Tell user that we found no errors
        if (!HasResults()) {
            QMessageBox msg(QMessageBox::Information,
                            tr("Cppcheck"),
                            tr("No errors found."),
                            QMessageBox::Ok,
                            this);

            msg.exec();
        } //If we have errors but they aren't visible, tell user about it
        else if (!mUI.mTree->HasVisibleResults()) {
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

bool ResultsView::HasVisibleResults() const
{
    return mUI.mTree->HasVisibleResults();
}

bool ResultsView::HasResults() const
{
    return mUI.mTree->HasResults();
}

void ResultsView::SaveSettings(QSettings *settings)
{
    mUI.mTree->SaveSettings();
    QByteArray state = mUI.mVerticalSplitter->saveState();
    settings->setValue(SETTINGS_MAINWND_SPLITTER_STATE, state);
    mUI.mVerticalSplitter->restoreState(state);
}

void ResultsView::Translate()
{
    mUI.mTree->Translate();
}

void ResultsView::DisableProgressbar()
{
    mUI.mProgress->setEnabled(false);
}

void ResultsView::ReadErrorsXml(const QString &filename)
{
    const int version = XmlReport::determineVersion(filename);
    if (version == 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to read the report."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    XmlReport *report = NULL;
    if (version == 1)
        report = new XmlReportV1(filename);
    else if (version == 2)
        report = new XmlReportV2(filename);

    QList<ErrorItem> errors;
    if (report) {
        if (report->Open())
            errors = report->Read();
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
        mUI.mTree->AddErrorItem(item);
    }
    mUI.mTree->SetCheckDirectory("");
}

void ResultsView::UpdateDetails(const QModelIndex &index)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(mUI.mTree->model());
    QStandardItem *item = model->itemFromIndex(index);

    if (!item) {
        mUI.mDetails->setText("");
        return;
    }

    // Make sure we are working with the first column
    if (item->parent() && item->column() != 0)
        item = item->parent()->child(item->row(), 0);

    QVariantMap data = item->data().toMap();

    // If there is no severity data then it is a parent item without summary and message
    if (!data.contains("severity")) {
        mUI.mDetails->setText("");
        return;
    }

    const QString summary = data["summary"].toString();
    const QString message = data["message"].toString();
    QString formattedMsg = QString("%1: %2\n%3: %4")
                           .arg(tr("Summary")).arg(summary)
                           .arg(tr("Message")).arg(message);

    const QString file0 = data["file0"].toString();
    if (file0 != "" && Path::isHeader(data["file"].toString().toStdString()))
        formattedMsg += QString("\n\n%1: %2").arg(tr("First included by")).arg(QDir::toNativeSeparators(file0));

    if (mUI.mTree->ShowIdColumn())
        formattedMsg.prepend(tr("Id") + ": " + data["id"].toString() + "\n");
    mUI.mDetails->setText(formattedMsg);
}

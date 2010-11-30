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

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QClipboard>
#include <QMimeData>
#include "common.h"
#include "project.h"
#include "projectfile.h"
#include "statsdialog.h"
#include "checkstatistics.h"

StatsDialog::StatsDialog(QWidget *parent)
    : QDialog(parent)
{
    mUI.setupUi(this);

    connect(mUI.mCopyToClipboard, SIGNAL(pressed()), this, SLOT(copyToClipboard()));
}

void StatsDialog::setProject(const Project& project)
{
    ProjectFile *projectFile = project.GetProjectFile();
    if (projectFile)
    {
        mUI.mProject->setText(projectFile->GetRootPath());
        mUI.mPaths->setText(projectFile->GetCheckPaths().join(";"));
        mUI.mIncludePaths->setText(projectFile->GetIncludeDirs().join(";"));
        mUI.mDefines->setText(projectFile->GetDefines().join(";"));
    }
    else
    {
        mUI.mProject->setText("");
        mUI.mPaths->setText("");
        mUI.mIncludePaths->setText("");
        mUI.mDefines->setText("");
    }
}

void StatsDialog::setPathSelected(const QString& path)
{
    mUI.mPath->setText(path);
}

void StatsDialog::setNumberOfFilesScanned(int num)
{
    mUI.mNumberOfFilesScanned->setText(QString::number(num));
}

void StatsDialog::setScanDuration(double seconds)
{
    mUI.mScanDuration->setText(tr("%1 secs").arg(seconds));
}

void StatsDialog::copyToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard)
    {
        // Plain text summary
        QString textSummary = tr(
                                  "Project Settings\n"
                                  "\tProject:\t%1\n"
                                  "\tPaths:\t%2\n"
                                  "\tInclude paths:\t%3\n"
                                  "\tDefines:\t%4\n"
                                  "Previous Scan\n"
                                  "\tPath selected:\t%5\n"
                                  "\tNumber of files scanned:\t%6\n"
                                  "\tScan duration:\t%7\n"
                                  "Statistics\n"
                                  "\tErrors:\t%8\n"
                                  "\tWarnings:\t%9\n"
                                  "\tStyle warnings:\t%10\n"
                                  "\tPerformance warnings:\t%11\n"
                              )
                              .arg(mUI.mProject->text())
                              .arg(mUI.mPaths->text())
                              .arg(mUI.mIncludePaths->text())
                              .arg(mUI.mDefines->text())
                              .arg(mUI.mPath->text())
                              .arg(mUI.mNumberOfFilesScanned->text())
                              .arg(mUI.mScanDuration->text())
                              .arg(mStatistics->GetCount(SHOW_ERRORS))
                              .arg(mStatistics->GetCount(SHOW_WARNINGS))
                              .arg(mStatistics->GetCount(SHOW_STYLE))
                              .arg(mStatistics->GetCount(SHOW_PERFORMANCE));

        // HTML summary
        QString htmlSummary = tr(
                                  "<h3>Project Settings<h3>\n"
                                  "<table>\n"
                                  " <tr><th>Project:</th><td>%1</td></tr>\n"
                                  " <tr><th>Paths:</th><td>%2</td></tr>\n"
                                  " <tr><th>Include paths:</th><td>%3</td></tr>\n"
                                  " <tr><th>Defines:</th><td>%4</td></tr>\n"
                                  "</table>\n"
                                  "<h3>Previous Scan</h3>\n"
                                  "<table>\n"
                                  " <tr><th>Path selected:</th><td>%5</td></tr>\n"
                                  " <tr><th>Number of files scanned:</th><td>%6</td></tr>\n"
                                  " <tr><th>Scan duration:</th><td>%7</td></tr>\n"
                                  "</table>\n"
                                  "<h3>Statistics</h3>\n"
                                  " <tr><th>Errors:</th><td>%8</td></tr>\n"
                                  " <tr><th>Warnings:</th><td>%9</td></tr>\n"
                                  " <tr><th>Style warnings:</th><td>%10</td></tr>\n"
                                  " <tr><th>Performance warnings:</th><td>%11</td></tr>\n"
                                  "</table>\n"
                              )
                              .arg(mUI.mProject->text())
                              .arg(mUI.mPaths->text())
                              .arg(mUI.mIncludePaths->text())
                              .arg(mUI.mDefines->text())
                              .arg(mUI.mPath->text())
                              .arg(mUI.mNumberOfFilesScanned->text())
                              .arg(mUI.mScanDuration->text())
                              .arg(mStatistics->GetCount(SHOW_ERRORS))
                              .arg(mStatistics->GetCount(SHOW_WARNINGS))
                              .arg(mStatistics->GetCount(SHOW_STYLE))
                              .arg(mStatistics->GetCount(SHOW_PERFORMANCE));

        QMimeData *mimeData = new QMimeData();
        mimeData->setText(textSummary);
        mimeData->setHtml(htmlSummary);
        clipboard->setMimeData(mimeData);

    }
}

void StatsDialog::setStatistics(const CheckStatistics *stats)
{
    mStatistics = const_cast<CheckStatistics*>(stats);
    mUI.mLblErrors->setText(QString("%1").arg(stats->GetCount(SHOW_ERRORS)));
    mUI.mLblWarnings->setText(QString("%1").arg(stats->GetCount(SHOW_WARNINGS)));
    mUI.mLblStyle->setText(QString("%1").arg(stats->GetCount(SHOW_STYLE)));
    mUI.mLblPerformance->setText(QString("%1").arg(stats->GetCount(SHOW_PERFORMANCE)));
}
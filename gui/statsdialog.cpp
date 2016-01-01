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

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QClipboard>
#include <QMimeData>
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
    if (projectFile) {
        mUI.mProject->setText(projectFile->GetRootPath());
        mUI.mPaths->setText(projectFile->GetCheckPaths().join(";"));
        mUI.mIncludePaths->setText(projectFile->GetIncludeDirs().join(";"));
        mUI.mDefines->setText(projectFile->GetDefines().join(";"));
    } else {
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
    // Factor the duration into units (days/hours/minutes/seconds)
    int secs = seconds;
    int days = secs / (24 * 60 * 60);
    secs -= days * (24 * 60 * 60);
    int hours = secs / (60 * 60);
    secs -= hours * (60 * 60);
    int mins = secs / 60;
    secs -= mins * 60;

    // Concatenate the two most significant units (e.g. "1 day and 3 hours")
    QStringList parts;
    if (days)
        parts << ((days == 1) ? tr("1 day") : tr("%1 days").arg(days));
    if (hours)
        parts << ((hours == 1) ? tr("1 hour") : tr("%1 hours").arg(hours));
    if (mins && parts.size() < 2)
        parts << ((mins == 1) ? tr("1 minute") : tr("%1 minutes").arg(mins));
    if (secs && parts.size() < 2)
        parts << ((secs == 1) ? tr("1 second") : tr("%1 seconds").arg(secs));

    // For durations < 1s, show the fraction of a second (e.g. "0.7 seconds")
    if (parts.isEmpty())
        parts << tr("0.%1 seconds").arg(int(10.0 *(seconds - secs)));

    mUI.mScanDuration->setText(parts.join(tr(" and ")));
}

void StatsDialog::copyToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard) {
        const QString projSettings(tr("Project Settings"));
        const QString project(tr("Project"));
        const QString paths(tr("Paths"));
        const QString incPaths(tr("Include paths"));
        const QString defines(tr("Defines"));
        const QString prevScan(tr("Previous Scan"));
        const QString selPath(tr("Path selected"));
        const QString numFiles(tr("Number of files scanned"));
        const QString duration(tr("Scan duration"));
        const QString stats(tr("Statistics"));
        const QString errors(tr("Errors"));
        const QString warnings(tr("Warnings"));
        const QString style(tr("Style warnings"));
        const QString portability(tr("Portability warnings"));
        const QString performance(tr("Performance warnings"));
        const QString information(tr("Information messages"));

        // Plain text summary
        const QString settings = QString(
                                     "%1\n"
                                     "\t%2:\t%3\n"
                                     "\t%4:\t%5\n"
                                     "\t%6:\t%7\n"
                                     "\t%8:\t%9\n"
                                 )
                                 .arg(projSettings)
                                 .arg(project)
                                 .arg(mUI.mProject->text())
                                 .arg(paths)
                                 .arg(mUI.mPaths->text())
                                 .arg(incPaths)
                                 .arg(mUI.mIncludePaths->text())
                                 .arg(defines)
                                 .arg(mUI.mDefines->text());

        const QString previous = QString(
                                     "%1\n"
                                     "\t%2:\t%3\n"
                                     "\t%4:\t%5\n"
                                     "\t%6:\t%7\n"
                                 )
                                 .arg(prevScan)
                                 .arg(selPath)
                                 .arg(mUI.mPath->text())
                                 .arg(numFiles)
                                 .arg(mUI.mNumberOfFilesScanned->text())
                                 .arg(duration)
                                 .arg(mUI.mScanDuration->text());

        const QString statistics = QString(
                                       "%1\n"
                                       "\t%2:\t%3\n"
                                       "\t%4:\t%5\n"
                                       "\t%6:\t%7\n"
                                       "\t%8:\t%9\n"
                                       "\t%10:\t%11\n"
                                       "\t%12:\t%13\n"
                                   )
                                   .arg(stats)
                                   .arg(errors)
                                   .arg(mStatistics->GetCount(ShowTypes::ShowErrors))
                                   .arg(warnings)
                                   .arg(mStatistics->GetCount(ShowTypes::ShowWarnings))
                                   .arg(style)
                                   .arg(mStatistics->GetCount(ShowTypes::ShowStyle))
                                   .arg(portability)
                                   .arg(mStatistics->GetCount(ShowTypes::ShowPortability))
                                   .arg(performance)
                                   .arg(mStatistics->GetCount(ShowTypes::ShowPerformance))
                                   .arg(information)
                                   .arg(mStatistics->GetCount(ShowTypes::ShowInformation));

        const QString textSummary = settings + previous + statistics;

        // HTML summary
        const QString htmlSettings = QString(
                                         "<h3>%1<h3>\n"
                                         "<table>\n"
                                         " <tr><th>%2:</th><td>%3</td></tr>\n"
                                         " <tr><th>%4:</th><td>%5</td></tr>\n"
                                         " <tr><th>%6:</th><td>%7</td></tr>\n"
                                         " <tr><th>%8:</th><td>%9</td></tr>\n"
                                         "</table>\n"
                                     )
                                     .arg(projSettings)
                                     .arg(project)
                                     .arg(mUI.mProject->text())
                                     .arg(paths)
                                     .arg(mUI.mPaths->text())
                                     .arg(incPaths)
                                     .arg(mUI.mIncludePaths->text())
                                     .arg(defines)
                                     .arg(mUI.mDefines->text());

        const QString htmlPrevious = QString(
                                         "<h3>%1</h3>\n"
                                         "<table>\n"
                                         " <tr><th>%2:</th><td>%3</td></tr>\n"
                                         " <tr><th>%4:</th><td>%5</td></tr>\n"
                                         " <tr><th>%6:</th><td>%7</td></tr>\n"
                                         "</table>\n"
                                     )
                                     .arg(prevScan)
                                     .arg(selPath)
                                     .arg(mUI.mPath->text())
                                     .arg(numFiles)
                                     .arg(mUI.mNumberOfFilesScanned->text())
                                     .arg(duration)
                                     .arg(mUI.mScanDuration->text());

        const QString htmlStatistics = QString(
                                           "<h3>%1</h3>\n"
                                           " <tr><th>%2:</th><td>%3</td></tr>\n"
                                           " <tr><th>%4:</th><td>%5</td></tr>\n"
                                           " <tr><th>%6:</th><td>%7</td></tr>\n"
                                           " <tr><th>%8:</th><td>%9</td></tr>\n"
                                           " <tr><th>%10:</th><td>%11</td></tr>\n"
                                           " <tr><th>%12:</th><td>%13</td></tr>\n"
                                           "</table>\n"
                                       )
                                       .arg(stats)
                                       .arg(errors)
                                       .arg(mStatistics->GetCount(ShowTypes::ShowErrors))
                                       .arg(warnings)
                                       .arg(mStatistics->GetCount(ShowTypes::ShowWarnings))
                                       .arg(style)
                                       .arg(mStatistics->GetCount(ShowTypes::ShowStyle))
                                       .arg(portability)
                                       .arg(mStatistics->GetCount(ShowTypes::ShowPortability))
                                       .arg(performance)
                                       .arg(mStatistics->GetCount(ShowTypes::ShowPerformance))
                                       .arg(information)
                                       .arg(mStatistics->GetCount(ShowTypes::ShowInformation));

        const QString htmlSummary = htmlSettings + htmlPrevious + htmlStatistics;

        QMimeData *mimeData = new QMimeData();
        mimeData->setText(textSummary);
        mimeData->setHtml(htmlSummary);
        clipboard->setMimeData(mimeData);
    }
}

void StatsDialog::setStatistics(const CheckStatistics *stats)
{
    mStatistics = const_cast<CheckStatistics*>(stats);
    mUI.mLblErrors->setText(QString("%1").arg(stats->GetCount(ShowTypes::ShowErrors)));
    mUI.mLblWarnings->setText(QString("%1").arg(stats->GetCount(ShowTypes::ShowWarnings)));
    mUI.mLblStyle->setText(QString("%1").arg(stats->GetCount(ShowTypes::ShowStyle)));
    mUI.mLblPortability->setText(QString("%1").arg(stats->GetCount(ShowTypes::ShowPortability)));
    mUI.mLblPerformance->setText(QString("%1").arg(stats->GetCount(ShowTypes::ShowPerformance)));
    mUI.mLblInformation->setText(QString("%1").arg(stats->GetCount(ShowTypes::ShowInformation)));
}

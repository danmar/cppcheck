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
#include <QPrinter>
#include <QDate>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextDocument>
#include <QWidget>
#include <QString>
#include <QClipboard>
#include <QMimeData>
#include "projectfile.h"
#include "statsdialog.h"
#include "checkstatistics.h"
#include "common.h"

static const QString CPPCHECK("cppcheck");

StatsDialog::StatsDialog(QWidget *parent)
    : QDialog(parent),
      mStatistics(nullptr)
{
    mUI.setupUi(this);

    setWindowFlags(Qt::Window);

    connect(mUI.mCopyToClipboard, &QPushButton::pressed, this, &StatsDialog::copyToClipboard);
    connect(mUI.mPDFexport, &QPushButton::pressed, this, &StatsDialog::pdfExport);
}

void StatsDialog::setProject(const ProjectFile* projectFile)
{
    if (projectFile) {
        mUI.mProject->setText(projectFile->getRootPath());
        mUI.mPaths->setText(projectFile->getCheckPaths().join(";"));
        mUI.mIncludePaths->setText(projectFile->getIncludeDirs().join(";"));
        mUI.mDefines->setText(projectFile->getDefines().join(";"));
#ifndef HAVE_QCHART
        mUI.mTabHistory->setVisible(false);
#else
        QString statsFile;
        if (!projectFile->getBuildDir().isEmpty()) {
            const QString prjpath = QFileInfo(projectFile->getFilename()).absolutePath();
            const QString buildDir = prjpath + '/' + projectFile->getBuildDir();
            if (QDir(buildDir).exists()) {
                statsFile = buildDir + "/statistics.txt";
            }
        }
        mUI.mLblHistoryFile->setText(tr("File: ") + (statsFile.isEmpty() ? tr("No cppcheck build dir") : statsFile));
        if (!statsFile.isEmpty()) {
            QChartView *chartView;
            chartView = createChart(statsFile, "cppcheck");
            mUI.mTabHistory->layout()->addWidget(chartView);
            if (projectFile->getClangAnalyzer()) {
                chartView = createChart(statsFile, CLANG_ANALYZER);
                mUI.mTabHistory->layout()->addWidget(chartView);
            }
            if (projectFile->getClangTidy()) {
                chartView = createChart(statsFile, CLANG_TIDY);
                mUI.mTabHistory->layout()->addWidget(chartView);
            }
        }
#endif
    } else {
        mUI.mProject->setText(QString());
        mUI.mPaths->setText(QString());
        mUI.mIncludePaths->setText(QString());
        mUI.mDefines->setText(QString());
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
void StatsDialog::pdfExport()
{
    const QString Stat = QString(
                             "<center><h1>%1   %2</h1></center>\n"
                             "<font color=\"red\"><h3>%3   :   %4</h3></font>\n"
                             "<font color=\"green\"><h3>%5   :   %6</h3></font>\n"
                             "<font color=\"orange\"><h3>%7   :   %8</h3></font>\n"
                             "<font color=\"blue\"><h3>%9   :   %10</h3></font>\n"
                             "<font color=\"blue\"><h3>%11  :   %12</h3></font>\n"
                             "<font color=\"purple\"><h3>%13  :   %14</h3></font>\n")
                         .arg(tr("Statistics"))
                         .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                         .arg(tr("Errors"))
                         .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowErrors))
                         .arg(tr("Warnings"))
                         .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowWarnings))
                         .arg(tr("Style warnings"))
                         .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowStyle))
                         .arg(tr("Portability warnings"))
                         .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowPortability))
                         .arg(tr("Performance warnings"))
                         .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowPerformance))
                         .arg(tr("Information messages"))
                         .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowInformation));

    QString fileName = QFileDialog::getSaveFileName((QWidget*)0, tr("Export PDF"), QString(), "*.pdf");
    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName.append(".pdf");
    }
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    doc.setHtml(Stat);
    // doc.setPageSize(printer.pageRect().size());
    doc.print(&printer);

}

void StatsDialog::copyToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (!clipboard)
        return;

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
                               .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowErrors))
                               .arg(warnings)
                               .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowWarnings))
                               .arg(style)
                               .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowStyle))
                               .arg(portability)
                               .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowPortability))
                               .arg(performance)
                               .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowPerformance))
                               .arg(information)
                               .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowInformation));

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
                                   .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowErrors))
                                   .arg(warnings)
                                   .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowWarnings))
                                   .arg(style)
                                   .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowStyle))
                                   .arg(portability)
                                   .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowPortability))
                                   .arg(performance)
                                   .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowPerformance))
                                   .arg(information)
                                   .arg(mStatistics->getCount(CPPCHECK,ShowTypes::ShowInformation));

    const QString htmlSummary = htmlSettings + htmlPrevious + htmlStatistics;

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(textSummary);
    mimeData->setHtml(htmlSummary);
    clipboard->setMimeData(mimeData);
}

void StatsDialog::setStatistics(const CheckStatistics *stats)
{
    mStatistics = stats;
    mUI.mLblErrors->setText(QString("%1").arg(stats->getCount(CPPCHECK,ShowTypes::ShowErrors)));
    mUI.mLblWarnings->setText(QString("%1").arg(stats->getCount(CPPCHECK,ShowTypes::ShowWarnings)));
    mUI.mLblStyle->setText(QString("%1").arg(stats->getCount(CPPCHECK,ShowTypes::ShowStyle)));
    mUI.mLblPortability->setText(QString("%1").arg(stats->getCount(CPPCHECK,ShowTypes::ShowPortability)));
    mUI.mLblPerformance->setText(QString("%1").arg(stats->getCount(CPPCHECK,ShowTypes::ShowPerformance)));
    mUI.mLblInformation->setText(QString("%1").arg(stats->getCount(CPPCHECK,ShowTypes::ShowInformation)));
}

#ifdef HAVE_QCHART
QChartView *StatsDialog::createChart(const QString &statsFile, const QString &tool)
{
    QChart *chart = new QChart;
    chart->addSeries(numberOfReports(statsFile, tool + "-error"));
    chart->addSeries(numberOfReports(statsFile, tool + "-warning"));
    chart->addSeries(numberOfReports(statsFile, tool + "-style"));
    chart->addSeries(numberOfReports(statsFile, tool + "-performance"));
    chart->addSeries(numberOfReports(statsFile, tool + "-portability"));

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);

    foreach (QAbstractSeries *s, chart->series()) {
        s->attachAxis(axisX);
    }

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Count");
    chart->addAxis(axisY, Qt::AlignLeft);

    qreal maxY = 0;
    foreach (QAbstractSeries *s, chart->series()) {
        s->attachAxis(axisY);
        if (QLineSeries *ls = dynamic_cast<QLineSeries*>(s)) {
            foreach (QPointF p, ls->points()) {
                if (p.y() > maxY)
                    maxY = p.y();
            }
        }
    }
    axisY->setMax(maxY);

    //chart->createDefaultAxes();
    chart->setTitle(tool);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    return chartView;
}

QLineSeries *StatsDialog::numberOfReports(const QString &fileName, const QString &severity) const
{
    QLineSeries *series = new QLineSeries();
    series->setName(severity);
    QFile f(fileName);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        quint64 t = 0;
        QTextStream in(&f);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QRegExp rxdate("\\[(\\d\\d)\\.(\\d\\d)\\.(\\d\\d\\d\\d)\\]");
            if (rxdate.exactMatch(line)) {
                int y = rxdate.cap(3).toInt();
                int m = rxdate.cap(2).toInt();
                int d = rxdate.cap(1).toInt();
                QDateTime dt;
                dt.setDate(QDate(y,m,d));
                if (t == dt.toMSecsSinceEpoch())
                    t += 1000;
                else
                    t = dt.toMSecsSinceEpoch();
            }
            if (line.startsWith(severity + ':')) {
                int y = line.mid(1+severity.length()).toInt();
                series->append(t, y);
            }
        }
    }
    return series;
}
#endif

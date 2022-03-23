/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include <QDialog>

class ProjectFile;
class CheckStatistics;
class QObject;
class QWidget;
namespace Ui {
    class StatsDialog;
}

#ifdef HAVE_QCHART
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
namespace QtCharts {
#endif
class QChartView;
class QLineSeries;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
}
#endif
#endif

/// @addtogroup GUI
/// @{

/**
 * @brief A dialog that shows project and scan statistics.
 *
 */
class StatsDialog : public QDialog {
    Q_OBJECT
public:
    explicit StatsDialog(QWidget *parent = nullptr);
    ~StatsDialog() override;

    /**
     * @brief Sets the project to extract statistics from
     */
    void setProject(const ProjectFile *projectFile);

    /**
     * @brief Sets the string to display beside "Path Selected:"
     */
    void setPathSelected(const QString& path);

    /**
     * @brief Sets the number to display beside "Number of Files Scanned:"
     */
    void setNumberOfFilesScanned(int num);

    /**
     * @brief Sets the number of seconds to display beside "Scan Duration:"
     */
    void setScanDuration(double seconds);

    /**
     * @brief Sets the numbers of different error/warnings found."
     */
    void setStatistics(const CheckStatistics *stats);

private slots:
    void copyToClipboard();
    void pdfExport();
#ifdef HAVE_QCHART
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QChartView *createChart(const QString &statsFile, const QString &tool);
    QLineSeries *numberOfReports(const QString &fileName, const QString &severity) const;
#else
    QtCharts::QChartView *createChart(const QString &statsFile, const QString &tool);
    QtCharts::QLineSeries *numberOfReports(const QString &fileName, const QString &severity) const;
#endif
#endif
private:
    Ui::StatsDialog *mUI;
    const CheckStatistics *mStatistics;
};

/// @}

#endif // STATSDIALOG_H

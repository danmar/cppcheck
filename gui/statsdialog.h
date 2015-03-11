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

#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include <QDialog>
#include "ui_stats.h"

class Project;
class CheckStatistics;

/// @addtogroup GUI
/// @{

/**
* @brief A dialog that shows project and scan statistics.
*
*/
class StatsDialog : public QDialog {
    Q_OBJECT
public:
    explicit StatsDialog(QWidget *parent = 0);

    /**
    * @brief Sets the project to extract statistics from
    */
    void setProject(const Project& project);

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

private:
    Ui::StatsDialog mUI;
    CheckStatistics *mStatistics;
};

/// @}

#endif // STATSDIALOG_H

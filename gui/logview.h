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

#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include "ui_logview.h"

/// @addtogroup GUI
/// @{

/**
* @brief A tool window that shows checking log.
*
*/
class LogView : public QWidget
{
    Q_OBJECT
public:
    LogView(QWidget *parent = 0);

    /**
    * @brief Append new log file to view.
    * @param line String to add.
    *
    */
    void AppendLine(const QString &line);

private:
    Ui::LogView mUI;
};

/// @}

#endif // LOGVIEW_H

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

#ifndef ERRORITEM_H
#define ERRORITEM_H

#include <QString>
#include <QStringList>

/// @addtogroup GUI
/// @{

/**
* @brief A class containing error data for one error.
*/
class ErrorItem
{
public:
    QString file;
    QStringList files;
    QList<unsigned int> lines;
    QString id;
    QString severity;
    QString msg;
};

/**
* @brief A class containing error data for one shown error line.
*/
class ErrorLine
{
public:
    QString file;
    QString line;
    QString id;
    QString severity;
    QString msg;
};

/// @}
#endif // ERRORITEM_H

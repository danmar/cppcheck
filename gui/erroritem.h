/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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
#include <QMetaType>
#include "errorlogger.h"

class ErrorLine;

/// @addtogroup GUI
/// @{


/**
 * @brief GUI versions of severity conversions.
 * GUI needs wrappers for conversion functions since GUI uses Qt's QString
 * instead of the std::string used by lib/cli.
 */
class GuiSeverity {
public:
    static QString toString(Severity::SeverityType severity) {
        return QString::fromStdString(Severity::toString(severity));
    }

    static Severity::SeverityType fromString(const QString &severity) {
        return Severity::fromString(severity.toStdString());
    }
};

/**
* @brief A class containing data for one error path item
*/
class QErrorPathItem {
public:
    QErrorPathItem() : line(0), col(-1) {}
    explicit QErrorPathItem(const ErrorLogger::ErrorMessage::FileLocation &loc);
    QString file;
    unsigned int line;
    int col;
    QString info;
};

bool operator==(const QErrorPathItem &i1, const QErrorPathItem &i2);

/**
* @brief A class containing error data for one error.
*
* The paths are stored with internal ("/") separators. Only when we show the
* path or copy if for user (to clipboard) we convert to native separators.
* Full path is stored instead of relative path for flexibility. It is easy
* to get the relative path from full path when needed.
*/
class ErrorItem {
public:
    ErrorItem();
    explicit ErrorItem(const ErrorLogger::ErrorMessage &errmsg);

    /**
    * @brief Convert error item to string.
    * @return Error item as string.
    */
    QString ToString() const;
    QString tool() const;

    QString file0;
    QString errorId;
    Severity::SeverityType severity;
    bool inconclusive;
    QString summary;
    QString message;
    int cwe;
    QList<QErrorPathItem> errorPath;
    QString symbolNames;

    // Special GUI properties
    QString sinceDate;
    QString tags;

    /**
     * Compare "CID"
     */
    static bool sameCID(const ErrorItem &errorItem1, const ErrorItem &errorItem2);
};

Q_DECLARE_METATYPE(ErrorItem);

/**
* @brief A class containing error data for one shown error line.
*/
class ErrorLine {
public:
    QString file;
    unsigned int line;
    QString file0;
    QString errorId;
    bool inconclusive;
    Severity::SeverityType severity;
    QString summary;
    QString message;
    QString sinceDate;
    QString tags;
};

/// @}
#endif // ERRORITEM_H

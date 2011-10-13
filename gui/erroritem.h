/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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
 * GUI needs its own versions of conversions since GUI uses Qt's QString
 * instead of the std::string used by lib/cli.
 */
class GuiSeverity : Severity {
public:
    static QString toString(SeverityType severity) {
        switch (severity) {
        case none:
            return "";
        case error:
            return "error";
        case warning:
            return "warning";
        case style:
            return "style";
        case performance:
            return "performance";
        case portability:
            return "portability";
        case information:
            return "information";
        case debug:
            return "debug";
        };
        return "???";
    }

    static SeverityType fromString(const QString &severity) {
        if (severity.isEmpty())
            return none;
        if (severity == "none")
            return none;
        if (severity == "error")
            return error;
        if (severity == "warning")
            return warning;
        if (severity == "style")
            return style;
        if (severity == "performance")
            return performance;
        if (severity == "portability")
            return portability;
        if (severity == "information")
            return information;
        if (severity == "debug")
            return debug;
        return none;
    }
};

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
    ErrorItem(const ErrorItem &item);
    ErrorItem(const ErrorLine &line);
    ~ErrorItem() { }

    /**
    * @brief Convert error item to string.
    * @return Error item as string.
    */
    QString ToString() const;

    QString file;
    QStringList files;
    QList<unsigned int> lines;
    QString errorId;
    Severity::SeverityType severity;
    bool inconclusive;
    QString summary;
    QString message;
};

Q_DECLARE_METATYPE(ErrorItem);

/**
* @brief A class containing error data for one shown error line.
*/
class ErrorLine {
public:
    QString file;
    unsigned int line;
    QString errorId;
    bool inconclusive;
    Severity::SeverityType severity;
    QString summary;
    QString message;
};

/// @}
#endif // ERRORITEM_H

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

#ifndef TXT_REPORT_H
#define TXT_REPORT_H

#include <QString>
#include <QTextStream>
#include "report.h"

/// @addtogroup GUI
/// @{


/**
* @brief Text file report.
* This report mimics the output of the command line cppcheck.
*/
class TxtReport : public Report {
    Q_OBJECT

public:
    explicit TxtReport(const QString &filename);
    virtual ~TxtReport();

    /**
    * @brief Create the report (file).
    * @return true if succeeded, false if file could not be created.
    */
    virtual bool Create();

    /**
    * @brief Write report header.
    */
    virtual void WriteHeader();

    /**
    * @brief Write report footer.
    */
    virtual void WriteFooter();

    /**
    * @brief Write error to report.
    * @param error Error data.
    */
    virtual void WriteError(const ErrorItem &error);

private:

    /**
    * @brief Text stream writer for writing the report in text format.
    */
    QTextStream mTxtWriter;
};
/// @}
#endif // TXT_REPORT_H

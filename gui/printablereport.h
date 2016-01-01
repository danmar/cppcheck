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

#ifndef PRINTABLE_REPORT_H
#define PRINTABLE_REPORT_H

#include "report.h"

/// @addtogroup GUI
/// @{


/**
* @brief Printable (in-memory) report.
* This report formats results and exposes them for printing.
*/
class PrintableReport : public Report {
public:
    PrintableReport();
    virtual ~PrintableReport();

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

    /**
    * @brief Returns the formatted report.
    */
    QString GetFormattedReportText() const;

private:

    /**
    * @brief Stores the formatted report contents.
    */
    QString mFormattedReport;
};
/// @}
#endif // PRINTABLE_REPORT_H

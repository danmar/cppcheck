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

#ifndef REPORT_H
#define REPORT_H

#include <QObject>
#include <QString>
#include <QFile>
#include "erroritem.h"

/// @addtogroup GUI
/// @{

/**
* @brief A base class for reports.
*/
class Report : public QObject {
public:

    enum Type {
        TXT,
        XML,
        XMLV2,
        CSV,
    };

    explicit Report(const QString &filename);
    virtual ~Report();

    /**
    * @brief Create the report (file).
    * @return true if succeeded, false if file could not be created.
    */
    virtual bool Create();

    /**
    * @brief Open the existing report (file).
    * @return true if succeeded, false if file could not be created.
    */
    virtual bool Open();

    /**
    * @brief Close the report (file).
    */
    virtual void Close();

    /**
    * @brief Write report header.
    */
    virtual void WriteHeader() = 0;

    /**
    * @brief Write report footer.
    */
    virtual void WriteFooter() = 0;

    /**
    * @brief Write error to report.
    * @param error Error data.
    */
    virtual void WriteError(const ErrorItem &error) = 0;

protected:

    /**
    * @brief Get the file object where the report is written to.
    */
    QFile* GetFile();

private:

    /**
    * @brief Filename of the report.
    */
    QString mFilename;

    /**
    * @brief Fileobject for the report file.
    */
    QFile mFile;
};
/// @}
#endif // REPORT_H

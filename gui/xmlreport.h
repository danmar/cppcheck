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

#ifndef XML_REPORT_H
#define XML_REPORT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "report.h"

/// @addtogroup GUI
/// @{


/**
* @brief XML file report.
* This report outputs XML-formatted report. The XML format must match command
* line version's XML output.
*/
class XmlReport : public Report
{
public:
    XmlReport(const QString &filename, QObject * parent = 0);
    virtual ~XmlReport();

    /**
    * @brief Create the report (file).
    * @return true if succeeded, false if file could not be created.
    */
    virtual bool Create();

    /**
    * @brief Open existing report file.
    */
    bool Open();

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
    * @brief Read contents of the report file.
    */
    QList<ErrorLine> Read();

protected:
    /**
    * @brief Read and parse error item from XML stream.
    * @param reader XML stream reader to use.
    */
    ErrorLine ReadError(QXmlStreamReader *reader);

private:
    /**
    * @brief XML stream reader for reading the report in XML format.
    */
    QXmlStreamReader *mXmlReader;

    /**
    * @brief XML stream writer for writing the report in XML format.
    */
    QXmlStreamWriter *mXmlWriter;
};
/// @}
#endif // XML_REPORT_H

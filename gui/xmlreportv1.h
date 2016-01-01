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

#ifndef XML_REPORTV1_H
#define XML_REPORTV1_H

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "xmlreport.h"

/// @addtogroup GUI
/// @{


/**
* @brief XML file report version 1.
* This report outputs XML-formatted report, version 1. The XML format must match command
* line version's XML output.
*/
class XmlReportV1 : public XmlReport {
public:
    explicit XmlReportV1(const QString &filename);
    virtual ~XmlReportV1();

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
    virtual QList<ErrorItem> Read();

protected:
    /**
    * @brief Read and parse error item from XML stream.
    * @param reader XML stream reader to use.
    */
    ErrorItem ReadError(QXmlStreamReader *reader);

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
#endif // XML_REPORTV1_H

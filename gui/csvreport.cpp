/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2017 Cppcheck team.
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

#include <QObject>
#include <QString>
#include <QDir>
#include <QTextStream>
#include "report.h"
#include "csvreport.h"

CsvReport::CsvReport(const QString &filename) :
    Report(filename)
{
}

CsvReport::~CsvReport()
{
}

bool CsvReport::create()
{
    if (Report::create()) {
        mTxtWriter.setDevice(Report::getFile());
        return true;
    }
    return false;
}

void CsvReport::writeHeader()
{
    // No header for CSV report
}

void CsvReport::writeFooter()
{
    // No footer for CSV report
}

void CsvReport::writeError(const ErrorItem &error)
{
    /*
    Error as CSV line
    gui/test.cpp,23,error,Mismatching allocation and deallocation: k
    */

    const QString file = QDir::toNativeSeparators(error.errorPath.back().file);
    QString line = QString("%1,%2,").arg(file).arg(error.errorPath.back().line);
    line += QString("%1,%2").arg(GuiSeverity::toString(error.severity)).arg(error.summary);

    mTxtWriter << line << endl;
}

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

bool CsvReport::Create()
{
    if (Report::Create()) {
        mTxtWriter.setDevice(Report::GetFile());
        return true;
    }
    return false;
}

void CsvReport::WriteHeader()
{
    // No header for CSV report
}

void CsvReport::WriteFooter()
{
    // No footer for CSV report
}

void CsvReport::WriteError(const ErrorItem &error)
{
    /*
    Error as CSV line
    gui/test.cpp,23,error,Mismatching allocation and deallocation: k
    */

    const QString file = QDir::toNativeSeparators(error.files[error.files.size() - 1]);
    QString line = QString("%1,%2,").arg(file).arg(error.lines[error.lines.size() - 1]);
    line += QString("%1,%2").arg(GuiSeverity::toString(error.severity)).arg(error.summary);

    mTxtWriter << line << endl;
}

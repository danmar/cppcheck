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

#include "printablereport.h"
#include <QDir>

PrintableReport::PrintableReport() :
    Report(QString())
{
}

PrintableReport::~PrintableReport()
{
}

bool PrintableReport::Create()
{
    return true;
}

void PrintableReport::WriteHeader()
{
    // No header for printable report
}

void PrintableReport::WriteFooter()
{
    // No footer for printable report
}

void PrintableReport::WriteError(const ErrorItem &error)
{
    const QString file = QDir::toNativeSeparators(error.files[error.files.size() - 1]);
    QString line = QString("%1,%2,").arg(file).arg(error.lines[error.lines.size() - 1]);
    line += QString("%1,%2").arg(GuiSeverity::toString(error.severity)).arg(error.summary);

    mFormattedReport += line;
    mFormattedReport += "\n";
}

QString PrintableReport::GetFormattedReportText() const
{
    return mFormattedReport;
}


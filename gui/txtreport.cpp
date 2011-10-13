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

#include <QDir>
#include <QFile>
#include <QTextStream>
#include "txtreport.h"

TxtReport::TxtReport(const QString &filename, QObject * parent) :
    Report(filename, parent)
{
}

TxtReport::~TxtReport()
{
    Close();
}

bool TxtReport::Create()
{
    bool success = false;
    if (Report::Create()) {
        mTxtWriter.setDevice(Report::GetFile());
        success = true;
    }
    return success;
}

void TxtReport::WriteHeader()
{
    // No header for txt report
}

void TxtReport::WriteFooter()
{
    // No footer for txt report
}

void TxtReport::WriteError(const ErrorItem &error)
{
    /*
    Error example from the core program in text
    [gui/test.cpp:23] -> [gui/test.cpp:14]: (error) Mismatching allocation and deallocation: k
    */

    QString line;

    for (int i = 0; i < error.lines.size(); i++) {
        const QString file = QDir::toNativeSeparators(error.files[i]);
        line += QString("[%1:%2]").arg(file).arg(error.lines[i]);
        if (i < error.lines.size() - 1 && error.lines.size() > 0) {
            line += " -> ";
        }

        if (i == error.lines.size() - 1) {
            line += ": ";
        }
    }
    line += QString("(%1) ").arg(GuiSeverity::toString(error.severity));
    if (error.inconclusive) {
        line += tr("inconclusive");
        line += " ";
    }
    line += error.summary;

    mTxtWriter << line << endl;
}

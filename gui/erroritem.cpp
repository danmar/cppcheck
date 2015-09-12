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

#include "erroritem.h"

ErrorItem::ErrorItem()
    : severity(Severity::none)
    , inconclusive(false)
{
}

ErrorItem::ErrorItem(const ErrorLine &line)
    : file(line.file)
    , files(line.file)
    , errorId(line.errorId)
    , severity(line.severity)
    , inconclusive(line.inconclusive)
    , summary(line.summary)
    , message(line.message)
{
    lines.append(line.line);
}

QString ErrorItem::ToString() const
{
    QString str = file + " - " + errorId + " - ";
    if (inconclusive)
        str += "inconclusive ";
    str += GuiSeverity::toString(severity) +"\n";
    str += summary + "\n";
    str += message + "\n";
    for (int i = 0; i < files.size(); i++) {
        str += "  " + files[i] + ": " + QString::number(lines[i]) + "\n";
    }
    return str;
}

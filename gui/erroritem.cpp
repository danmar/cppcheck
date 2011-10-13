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

#include "erroritem.h"

ErrorItem::ErrorItem()
    : severity(Severity::none)
    , inconclusive(false)
{
}

ErrorItem::ErrorItem(const ErrorItem &item)
{
    file = item.file;
    files = item.files;
    lines = item.lines;
    errorId = item.errorId;
    inconclusive = item.inconclusive;
    severity = item.severity;
    summary = item.summary;
    message = item.message;
}

ErrorItem::ErrorItem(const ErrorLine &line)
{
    file = line.file;
    files.append(line.file);
    lines.append(line.line);
    errorId = line.errorId;
    inconclusive = line.inconclusive;
    severity = line.severity;
    summary = line.summary;
    message = line.message;
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

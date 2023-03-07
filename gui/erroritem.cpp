/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "common.h"

#include <list>

QErrorPathItem::QErrorPathItem(const ErrorMessage::FileLocation &loc)
    : file(QString::fromStdString(loc.getfile(false)))
    , line(loc.line)
    , column(loc.column)
    , info(QString::fromStdString(loc.getinfo()))
{}

bool operator==(const QErrorPathItem &i1, const QErrorPathItem &i2)
{
    return i1.file == i2.file && i1.column == i2.column && i1.line == i2.line && i1.info == i2.info;
}

ErrorItem::ErrorItem()
    : severity(Severity::none)
    , inconclusive(false)
    , cwe(-1)
    , hash(0)
{}

ErrorItem::ErrorItem(const ErrorMessage &errmsg)
    : file0(QString::fromStdString(errmsg.file0))
    , errorId(QString::fromStdString(errmsg.id))
    , severity(errmsg.severity)
    , inconclusive(errmsg.certainty == Certainty::inconclusive)
    , summary(QString::fromStdString(errmsg.shortMessage()))
    , message(QString::fromStdString(errmsg.verboseMessage()))
    , cwe(errmsg.cwe.id)
    , hash(errmsg.hash)
    , symbolNames(QString::fromStdString(errmsg.symbolNames()))
{
    for (std::list<ErrorMessage::FileLocation>::const_iterator loc = errmsg.callStack.cbegin();
         loc != errmsg.callStack.cend();
         ++loc) {
        errorPath << QErrorPathItem(*loc);
    }
}

QString ErrorItem::tool() const
{
    if (errorId == CLANG_ANALYZER)
        return CLANG_ANALYZER;
    if (errorId.startsWith(CLANG_TIDY))
        return CLANG_TIDY;
    if (errorId.startsWith("clang-"))
        return "clang";
    return "cppcheck";
}

QString ErrorItem::toString() const
{
    QString str = errorPath.back().file + " - " + errorId + " - ";
    if (inconclusive)
        str += "inconclusive ";
    str += GuiSeverity::toString(severity) +"\n";
    str += summary + "\n";
    str += message + "\n";
    for (const QErrorPathItem& i : errorPath) {
        str += "  " + i.file + ": " + QString::number(i.line) + "\n";
    }
    return str;
}

bool ErrorItem::sameCID(const ErrorItem &errorItem1, const ErrorItem &errorItem2)
{
    if (errorItem1.hash || errorItem2.hash)
        return errorItem1.hash == errorItem2.hash;

    // fallback
    return errorItem1.errorId == errorItem2.errorId &&
           errorItem1.errorPath == errorItem2.errorPath &&
           errorItem1.file0 == errorItem2.file0 &&
           errorItem1.message == errorItem2.message &&
           errorItem1.inconclusive == errorItem2.inconclusive &&
           errorItem1.severity == errorItem2.severity;
}

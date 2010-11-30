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

#include <QDebug>
#include "common.h"
#include "checkstatistics.h"

CheckStatistics::CheckStatistics(QObject *parent)
    : QObject(parent)
{
    Clear();
}

void CheckStatistics::AddItem(ShowTypes type)
{
    switch (type)
    {
    case SHOW_STYLE:
        mStyle++;
        break;
    case SHOW_WARNINGS:
        mWarning++;
        break;
    case SHOW_PERFORMANCE:
        mPerformance++;
        break;
    case SHOW_ERRORS:
        mError++;
        break;
    case SHOW_NONE:
    default:
        qDebug() << "Unknown error type - not added to statistics.";
        break;
    }
}

void CheckStatistics::Clear()
{
    mStyle = 0;
    mWarning = 0;
    mPerformance = 0;
    mError = 0;
}

unsigned CheckStatistics::GetCount(ShowTypes type) const
{
    unsigned count = 0;
    switch (type)
    {
    case SHOW_STYLE:
        count = mStyle;
        break;
    case SHOW_WARNINGS:
        count = mWarning;
        break;
    case SHOW_PERFORMANCE:
        count = mPerformance;
        break;
    case SHOW_ERRORS:
        count = mError;
        break;
    case SHOW_NONE:
    default:
        qDebug() << "Unknown error type - returning zero statistices.";
        break;
    }
    return count;
}

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

#include <QDebug>
#include "checkstatistics.h"

CheckStatistics::CheckStatistics(QObject *parent)
    : QObject(parent)
{
    Clear();
}

void CheckStatistics::AddItem(ShowTypes::ShowType type)
{
    switch (type) {
    case ShowTypes::ShowStyle:
        mStyle++;
        break;
    case ShowTypes::ShowWarnings:
        mWarning++;
        break;
    case ShowTypes::ShowPerformance:
        mPerformance++;
        break;
    case ShowTypes::ShowPortability:
        mPortability++;
        break;
    case ShowTypes::ShowErrors:
        mError++;
        break;
    case ShowTypes::ShowInformation:
        mInformation++;
        break;
    case ShowTypes::ShowNone:
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
    mPortability = 0;
    mInformation = 0;
    mError = 0;
}

unsigned CheckStatistics::GetCount(ShowTypes::ShowType type) const
{
    switch (type) {
    case ShowTypes::ShowStyle:
        return mStyle;
    case ShowTypes::ShowWarnings:
        return mWarning;
    case ShowTypes::ShowPerformance:
        return mPerformance;
    case ShowTypes::ShowPortability:
        return mPortability;
    case ShowTypes::ShowErrors:
        return mError;
    case ShowTypes::ShowInformation:
        return mInformation;
    case ShowTypes::ShowNone:
    default:
        qDebug() << "Unknown error type - returning zero statistics.";
        return 0;
    }
}

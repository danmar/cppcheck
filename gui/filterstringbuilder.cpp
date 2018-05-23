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


#include "filterstringbuilder.h"
#include <QStringList>


FilterStringBuilder& FilterStringBuilder::add(const QString& desc, const QString& patterns)
{
    mFilters.insert(desc, patterns);
    return *this;
}

FilterStringBuilder& FilterStringBuilder::addAll()
{
    mDisplayAll = true;
    return *this;
}

FilterStringBuilder& FilterStringBuilder::addAllSupported()
{
    mDisplayAllSupported = true;
    return *this;
}

QString FilterStringBuilder::toFilterString() const
{
    QStringList entries;

    if (mDisplayAllSupported) {
        entries << tr("All supported files (%1)").arg(QStringList(mFilters.values()).join(" "));
    }

    if (mDisplayAll) {
        entries << tr("All files (%1)").arg("*.*");
    }

    // We're using the description of the filters as the map keys, the file
    // name patterns are our values. The generated filter string list will
    // thus be sorted alphabetically over the descriptions.
    for (auto k: mFilters.keys()) {
        entries << QString("%1 (%2)").arg(k).arg(mFilters.value(k));
    }

    return entries.join(";;");
}

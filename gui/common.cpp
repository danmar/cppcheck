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


#include "common.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>


QString GetPath(const QString &type)
{
    QSettings settings;
    QString path = settings.value(type, "").toString();
    if (path.isEmpty()) {
        // if not set, fallback to last check path hoping that it will be close enough
        path = settings.value(SETTINGS_LAST_CHECK_PATH, "").toString();
        if (path.isEmpty())
            // if not set, return user's home directory as the best we can do for now
            return QDir::homePath();
    }
    return path;
}

void SetPath(const QString &type, const QString &value)
{
    QSettings settings;
    settings.setValue(type, value);
}

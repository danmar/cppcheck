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

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include "filelist.h"

QStringList FileList::GetDefaultFilters()
{
    QStringList extensions;
    extensions << "*.cpp" << "*.cxx" << "*.cc" << "*.c" << "*.c++" << "*.txx" << "*.tpp";
    return extensions;
}

bool FileList::FilterMatches(const QFileInfo &inf)
{
    if (inf.isFile()) {
        const QStringList filters = FileList::GetDefaultFilters();
        QString ext("*.");
        ext += inf.suffix();
        if (filters.contains(ext))
            return true;
    }
    return false;
}

void FileList::AddFile(const QString &filepath)
{
    QFileInfo inf(filepath);
    if (FilterMatches(inf))
        mFileList << inf;
}

void FileList::AddDirectory(const QString &directory, bool recursive)
{
    QDir dir(directory);
    dir.setSorting(QDir::Name);
    const QStringList filters = FileList::GetDefaultFilters();
    const QStringList origNameFilters = dir.nameFilters();
    dir.setNameFilters(filters);
    if (!recursive) {
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        QFileInfoList items = dir.entryInfoList();
        mFileList += items;
    } else {
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        QFileInfoList items = dir.entryInfoList();
        mFileList += items;

        dir.setNameFilters(origNameFilters);
        dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList list = dir.entryInfoList();
        QFileInfo item;
        foreach(item, list) {
            const QString path = item.canonicalFilePath();
            AddDirectory(path, recursive);
        }
    }
}

void FileList::AddPathList(const QStringList &paths)
{
    QString path;
    foreach(path, paths) {
        QFileInfo inf(path);
        if (inf.isFile())
            AddFile(path);
        else
            AddDirectory(path, true);
    }
}

QStringList FileList::GetFileList() const
{
    if (mExcludedPaths.empty()) {
        QStringList names;
        foreach(QFileInfo item, mFileList) {
            QString name = QDir::fromNativeSeparators(item.canonicalFilePath());
            names << name;
        }
        return names;
    } else {
        return ApplyExcludeList();
    }
}

void FileList::AddExcludeList(const QStringList &paths)
{
    mExcludedPaths = paths;
}

QStringList FileList::ApplyExcludeList() const
{
    QStringList paths;
    foreach(QFileInfo item, mFileList) {
        QString name = QDir::fromNativeSeparators(item.canonicalFilePath());
        if (!Match(name))
            paths << name;
    }
    return paths;
}

bool FileList::Match(const QString &path) const
{
    for (int i = 0; i < mExcludedPaths.size(); i++) {
        if (mExcludedPaths[i].endsWith('/')) {
            const QString pathexclude("/" + mExcludedPaths[i]);
            if (path.indexOf(pathexclude) != -1)
                return true;
        } else {
            if (path.endsWith(mExcludedPaths[i]))
                return true;
        }
    }
    return false;
}

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

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include "filelist.h"
#include "path.h"
#include "pathmatch.h"

QStringList FileList::getDefaultFilters()
{
    QStringList extensions;
    extensions << "*.cpp" << "*.cxx" << "*.cc" << "*.c" << "*.c++" << "*.txx" << "*.tpp";
    return extensions;
}

bool FileList::filterMatches(const QFileInfo &inf)
{
    if (inf.isFile()) {
        const QStringList filters = FileList::getDefaultFilters();
        QString ext("*.");
        ext += inf.suffix();
        if (filters.contains(ext, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

void FileList::addFile(const QString &filepath)
{
    QFileInfo inf(filepath);
    if (filterMatches(inf))
        mFileList << inf;
}

void FileList::addDirectory(const QString &directory, bool recursive)
{
    QDir dir(directory);
    dir.setSorting(QDir::Name);
    const QStringList filters = FileList::getDefaultFilters();
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
        foreach (item, list) {
            const QString path = item.canonicalFilePath();
            addDirectory(path, recursive);
        }
    }
}

void FileList::addPathList(const QStringList &paths)
{
    QString path;
    foreach (path, paths) {
        QFileInfo inf(path);
        if (inf.isFile())
            addFile(path);
        else
            addDirectory(path, true);
    }
}

QStringList FileList::getFileList() const
{
    if (mExcludedPaths.empty()) {
        QStringList names;
        foreach (QFileInfo item, mFileList) {
            QString name = QDir::fromNativeSeparators(item.canonicalFilePath());
            names << name;
        }
        return names;
    } else {
        return applyExcludeList();
    }
}

void FileList::addExcludeList(const QStringList &paths)
{
    mExcludedPaths = paths;
}

static std::vector<std::string> toStdStringList(const QStringList &stringList)
{
    std::vector<std::string> ret;
    foreach (const QString &s, stringList) {
        ret.push_back(s.toStdString());
    }
    return ret;
}

QStringList FileList::applyExcludeList() const
{
#ifdef _WIN32
    const PathMatch pathMatch(toStdStringList(mExcludedPaths), true);
#else
    const PathMatch pathMatch(toStdStringList(mExcludedPaths), false);
#endif

    QStringList paths;
    foreach (QFileInfo item, mFileList) {
        QString name = QDir::fromNativeSeparators(item.canonicalFilePath());
        if (!pathMatch.match(name.toStdString()))
            paths << name;
    }
    return paths;
}

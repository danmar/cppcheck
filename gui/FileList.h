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

#ifndef FILELIST_H
#define FILELIST_H

#include <QList>
#include <QFileInfoList>
#include <QStringList>

/**
 * @brief A class for listing files and directories to check.
 * This class creates a list of files to check. If directory name is given then
 * all files in the directory matching the filter will be added. The directory
 * can be also added recursively when all files in subdirectories are added too.
 * The filenames are matched against the filter and only those files whose
 * filename extension is included in the filter list are added.
 */
class FileList
{
public:

    /**
    * @brief Add filename to the list.
    * @param filepath Full path to the file.
    */
    void AddFile(const QString &filepath);

    /**
    * @brief Add files in the directory to the list.
    * @param directory Full pathname to directory to add.
    * @param recursive If true also files in subdirectories are added.
    */
    void AddDirectory(const QString &directory, bool recursive = false);

    /**
    * @brief Add list of filenames and directories to the list.
    * @param paths List of paths to add.
    */
    void AddPathList(const QStringList &paths);

    /**
    * @brief Return list of filenames (to check).
    * @return list of filenames to check.
    */
    QStringList GetFileList() const;

protected:

    /**
    * @brief Return list of default filename extensions included.
    * @return list of default filename extensions included.
    */
    static QStringList GetDefaultFilters();

    /**
    * @brief Test if filename matches the filename extensions filtering.
    * @param true if filename matches filterin.
    */
    bool FilterMatches(const QFileInfo &inf);

private:
    QFileInfoList mFileList;
};

#endif // FILELIST_H

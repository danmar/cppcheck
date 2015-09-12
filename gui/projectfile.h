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

#ifndef PROJECT_FILE_H
#define PROJECT_FILE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>

/// @addtogroup GUI
/// @{


/**
* @brief A class that reads and writes project files.
* The project files contain project-specific settings for checking. For
* example a list of include paths.
*/
class ProjectFile : public QObject {
    Q_OBJECT

public:
    explicit ProjectFile(QObject *parent = 0);
    ProjectFile(const QString &filename, QObject *parent = 0);

    /**
    * @brief Read the project file.
    * @param filename Filename (can be also given to constructor).
    */
    bool Read(const QString &filename = QString());

    /**
    * @brief Get project root path.
    * @return project root path.
    */
    QString GetRootPath() const {
        return mRootPath;
    }

    /**
    * @brief Get list of include directories.
    * @return list of directories.
    */
    QStringList GetIncludeDirs() const;

    /**
    * @brief Get list of defines.
    * @return list of defines.
    */
    QStringList GetDefines() const;

    /**
    * @brief Get list of paths to check.
    * @return list of paths.
    */
    QStringList GetCheckPaths() const;

    /**
    * @brief Get list of paths to exclude from the check.
    * @return list of paths.
    */
    QStringList GetExcludedPaths() const;

    /**
    * @brief Get list libraries.
    * @return list of libraries.
    */
    QStringList GetLibraries() const;

    /**
    * @brief Get list suppressions.
    * @return list of suppressions.
    */
    QStringList GetSuppressions() const;

    /**
    * @brief Get filename for the project file.
    * @return file name.
    */
    QString GetFilename() const {
        return mFilename;
    }

    /**
    * @brief Set project root path.
    * @param rootpath new project root path.
    */
    void SetRootPath(const QString &rootpath) {
        mRootPath = rootpath;
    }

    /**
    * @brief Set list of includes.
    * @param includes List of defines.
    */
    void SetIncludes(const QStringList &includes);

    /**
    * @brief Set list of defines.
    * @param defines List of defines.
    */
    void SetDefines(const QStringList &defines);

    /**
    * @brief Set list of paths to check.
    * @param paths List of paths.
    */
    void SetCheckPaths(const QStringList &paths);

    /**
    * @brief Set list of paths to exclude from the check.
    * @param paths List of paths.
    */
    void SetExcludedPaths(const QStringList &paths);

    /**
    * @brief Set list of libraries.
    * @param libraries List of libraries.
    */
    void SetLibraries(const QStringList &libraries);

    /**
    * @brief Set list of suppressions.
    * @param suppressions List of suppressions.
    */
    void SetSuppressions(const QStringList &suppressions);

    /**
    * @brief Write project file (to disk).
    * @param filename Filename to use.
    */
    bool Write(const QString &filename = QString());

    /**
     * @brief Write string list
     * @param xmlWriter xml writer
     * @param stringlist string list to write
     * @param startelementname name of start element
     * @param stringelementname name of each string element
     */
    static void WriteStringList(QXmlStreamWriter &xmlWriter, const QStringList &stringlist, const char startelementname[], const char stringelementname[]);

    /**
    * @brief Set filename for the project file.
    * @param filename Filename to use.
    */
    void SetFilename(const QString &filename) {
        mFilename = filename;
    }

protected:

    /**
    * @brief Read optional root path from XML.
    * @param reader XML stream reader.
    */
    void ReadRootPath(QXmlStreamReader &reader);

    /**
    * @brief Read list of include directories from XML.
    * @param reader XML stream reader.
    */
    void ReadIncludeDirs(QXmlStreamReader &reader);

    /**
    * @brief Read list of defines from XML.
    * @param reader XML stream reader.
    */
    void ReadDefines(QXmlStreamReader &reader);

    /**
    * @brief Read list paths to check.
    * @param reader XML stream reader.
    */
    void ReadCheckPaths(QXmlStreamReader &reader);

    /**
    * @brief Read lists of excluded paths.
    * @param reader XML stream reader.
    */
    void ReadExcludes(QXmlStreamReader &reader);

    /**
     * @brief Read string list
     * @param stringlist   destination string list
     * @param reader       XML stream reader
     * @param elementname  elementname for each string
     */
    void ReadStringList(QStringList &stringlist, QXmlStreamReader &reader, const char elementname[]);

private:

    /**
    * @brief Filename (+path) of the project file.
    */
    QString mFilename;

    /**
    * @brief Root path (optional) for the project.
    * This is the project root path. If it is present then all relative paths in
    * the project file are relative to this path. Otherwise paths are relative
    * to project file's path.
    */
    QString mRootPath;

    /**
    * @brief List of include directories used to search include files.
    */
    QStringList mIncludeDirs;

    /**
    * @brief List of defines.
    */
    QStringList mDefines;

    /**
    * @brief List of paths to check.
    */
    QStringList mPaths;

    /**
    * @brief Paths excluded from the check.
    */
    QStringList mExcludedPaths;

    /**
    * @brief List of libraries.
    */
    QStringList mLibraries;

    /**
     * @brief List of suppressions.
     */
    QStringList mSuppressions;
};
/// @}
#endif  // PROJECT_FILE_H

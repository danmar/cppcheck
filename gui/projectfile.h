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

#ifndef PROJECT_FILE_H
#define PROJECT_FILE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>

/// @addtogroup GUI
/// @{


/**
* @brief A class that reads and writes (TODO) project files.
* The project files contain project-specific settings for checking. For
* example a list of include paths.
*/
class ProjectFile : public QObject
{
    Q_OBJECT

public:
    ProjectFile(QObject *parent = 0);
    ProjectFile(const QString &filename, QObject *parent = 0);

    /**
    * @brief Read the project file.
    * @param filename Filename (can be also given to constructor).
    */
    bool Read(const QString &filename = QString());

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
    * @brief Set list of includes.
    * @param includes List of defines.
    */
    void SetIncludes(QStringList includes);

    /**
    * @brief Set list of defines.
    * @param defines List of defines.
    */
    void SetDefines(QStringList defines);

    /**
    * @brief Set list of paths to check.
    * @param defines List of paths.
    */
    void SetCheckPaths(QStringList paths);

    /**
    * @brief Write project file (to disk).
    * @param filename Filename to use.
    */
    bool Write(const QString &filename = QString());

    /**
    * @brief Set filename for the project file.
    * @param filename Filename to use.
    */
    void SetFilename(const QString &filename)
    {
        mFilename = filename;
    }

protected:
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

private:

    /**
    * @brief Filename (+path) of the project file.
    */
    QString mFilename;

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
};
/// @}
#endif  // PROJECT_FILE_H

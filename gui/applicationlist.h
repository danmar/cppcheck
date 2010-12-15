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

#ifndef APPLICATIONLIST_H
#define APPLICATIONLIST_H

#include <QObject>
#include <QSettings>

/// @addtogroup GUI
/// @{


/**
* @brief List of applications user has specified to open errors with
* Each application has a name and a path. Name is displayed to the user
* and has no other meaning. It isn't used to start the application.
* Path contains the path to the application as well as the executable itself and
* any possible argument user might want to specify.
*
* User can also specify certain predefined strings to path. These strings
* will be replaced with appropriate values concerning the error. Strings are:
* (file) - Filename containing the error
* (line) - Line number containing the error
* (message) - Error message
* (severity) - Error severity
*
* Example opening a file with Kate and make Kate scroll to the correct line:
* kate -l(line) (file)
*
*/
class ApplicationList : public QObject
{
    Q_OBJECT
public:

    /**
    * @brief Struct containing information of the application
    *
    */
    typedef struct
    {
        /**
        * @brief Applicaton's name
        *
        */
        QString Name;

        /**
        * @brief Application's path and commandline arguments
        *
        */
        QString Path;
    } ApplicationType;

    ApplicationList(QObject *parent = 0);
    virtual ~ApplicationList();

    /**
    * @brief Load all applications
    *
    * @param programSettings QSettings to load application list from
    */
    void LoadSettings(QSettings *programSettings);

    /**
    * @brief Save all applications
    * @param programSettings QSettings to save applications to
    */
    void SaveSettings(QSettings *programSettings);

    /**
    * @brief Get the amount of applications in the list
    * @return The count of applications
    */
    int GetApplicationCount() const;

    /**
    * @brief Get specific application's name
    *
    * @param index Index of the application whose name to get
    * @return Name of the application
    */
    QString GetApplicationName(const int index) const;

    /**
    * @brief Get Application's path
    *
    * @param index of the application whose path to get
    * @return Application's path
    */
    QString GetApplicationPath(const int index) const;

    /**
    * @brief Modify an application
    *
    * @param index Index of the application to modify
    * @param name New name for the application
    * @param path New path for the application
    */
    void SetApplicationType(const int index,
                            const QString &name,
                            const QString &path);

    /**
    * @brief Add a new application
    *
    * @param name Name of the application
    * @param path Path to the application
    */
    void AddApplicationType(const QString &name, const QString &path);

    /**
    * @brief Remove an application from the list
    *
    * @param index Index of the application to remove.
    */
    void RemoveApplication(const int index);

    /**
    * @brief Move certain application as first.
    * Position of the application is used by the application to determine
    * which of the applications is the default application. First application
    * (index 0) is the default application.
    *
    * @param index Index of the application to make the default one
    */
    void MoveFirst(const int index);

    /**
    * @brief Remove all applications from this list and copy all applications from
    * list given as a parameter.
    * @param list Copying source
    */
    void Copy(ApplicationList *list);
protected:

    /**
    * @brief Clear the list
    *
    */
    void Clear();

    /**
    * @brief Find editor used by default in Windows.
    * Check if Notepad++ is installed and use it. If not, use Notepad.
    */
    bool FindDefaultWindowsEditor();

    /**
    * @brief List of applications
    *
    */
    QList<ApplicationType> mApplications;
private:
};
/// @}
#endif // APPLICATIONLIST_H

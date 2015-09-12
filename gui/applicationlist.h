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

#ifndef APPLICATIONLIST_H
#define APPLICATIONLIST_H

#include <QObject>
#include "application.h"

/// @addtogroup GUI
/// @{


/**
* @brief List of applications user has specified to open errors with.
*/
class ApplicationList : public QObject {
    Q_OBJECT
public:

    explicit ApplicationList(QObject *parent = 0);
    virtual ~ApplicationList();

    /**
    * @brief Load all applications
    *
    * @return true if loading succeeded, false if there is problem with
    *  application list. Most probably because of older version settings need
    *  to be upgraded.
    */
    bool LoadSettings();

    /**
    * @brief Save all applications
    */
    void SaveSettings() const;

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
    const Application& GetApplication(const int index) const;
    Application& GetApplication(const int index);

    /**
    * @brief Return the default application.
    * @return Index of the default application.
    */
    int GetDefaultApplication() const {
        return mDefaultApplicationIndex;
    }

    /**
    * @brief Add a new application
    *
    * @param app Application to add.
    */
    void AddApplication(const Application &app);

    /**
    * @brief Remove an application from the list
    *
    * @param index Index of the application to remove.
    */
    void RemoveApplication(const int index);

    /**
    * @brief Set application as default application.
    * @param index Index of the application to make the default one
    */
    void SetDefault(const int index);

    /**
    * @brief Remove all applications from this list and copy all applications from
    * list given as a parameter.
    * @param list Copying source
    */
    void Copy(const ApplicationList *list);

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

private:

    /**
    * @brief List of applications
    *
    */
    QList<Application> mApplications;

    /**
    * @brief Index of the default application.
    *
    */
    int mDefaultApplicationIndex;
};
/// @}
#endif // APPLICATIONLIST_H

/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QString>

/**
* @brief A class containing information of the application to execute.
*
*/
class Application
{
public:
    Application(const QString &name, const QString &path, const QString &params);

    /**
    * @brief Get application name.
    * @return Application name.
    */
    QString getName() const
    {
        return mName;
    }

    /**
    * @brief Get application path.
    * @return Application path.
    */
    QString getPath() const
    {
        return mPath;
    }

    /**
    * @brief Get application command line parameters.
    * @return Application command line parameters.
    */
    QString getParameters() const
    {
        return mParameters;
    }

    /**
    * @brief Set application name.
    * @param name Application name.
    */
    void setName(const QString &name)
    {
        mName = name;
    }

    /**
    * @brief Set application path.
    * @param path Application path.
    */
    void setPath(const QString &path)
    {
        mPath = path;
    }

    /**
    * @brief Set application command line parameters.
    * @param parameters Application command line parameters.
    */
    void setParameters(const QString &parameters)
    {
        mParameters = parameters;
    }

private:

    /**
    * @brief Application's name
    */
    QString mName;

    /**
    * @brief Application's path
    */
    QString mPath;

    /**
    * @brief Application's parameters
    */
    QString mParameters;
};

#endif // APPLICATION_H

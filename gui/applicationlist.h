/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef APPLICATIONLIST_H
#define APPLICATIONLIST_H

#include <QObject>
#include <QSettings>


class ApplicationList : public QObject
{
public:
    typedef struct
    {
        QString Name;
        QString Path;
    }ApplicationType;

    ApplicationList();
    virtual ~ApplicationList();

    void LoadSettings(QSettings &programSettings);

    void SaveSettings(QSettings &programSettings);

    int GetApplicationCount();

    QString GetApplicationName(const  int index);

    QString GetApplicationPath(const  int index);

    void SetApplicationType(const  int index,
                            const QString &name,
                            const QString &path);

    void AddApplicationType(const QString &name, const QString &path);

    void RemoveApplication(const int index);
protected:


    QList<ApplicationType> mApplications;
private:
};

#endif // APPLICATIONLIST_H

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

#include <QStringList>
#include <QFileInfo>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <stdlib.h>
#include "common.h"
#include "applicationlist.h"


ApplicationList::ApplicationList(QObject *parent) :
    QObject(parent),
    mDefaultApplicationIndex(-1)
{
    //ctor
}

ApplicationList::~ApplicationList()
{
    Clear();
}

bool ApplicationList::LoadSettings(QSettings *programSettings)
{

    QStringList names = programSettings->value(SETTINGS_APPLICATION_NAMES, QStringList()).toStringList();
    QStringList paths = programSettings->value(SETTINGS_APPLICATION_PATHS, QStringList()).toStringList();
    QStringList params = programSettings->value(SETTINGS_APPLICATION_PARAMS, QStringList()).toStringList();
    int defapp = programSettings->value(SETTINGS_APPLICATION_DEFAULT, -1).toInt();

    // Params will be empty first time starting with the new setting.
    // Return false and inform user about problem with application settings.
    bool succeeded = true;
    if (params.empty())
    {
        for (int i = 0; i < paths.length(); i++)
            params << "";
        succeeded = false;
    }

    if (names.empty() && paths.empty() && params.empty())
    {
        do
        {
            // use as default for gnome environments
            if (QFileInfo("/usr/bin/gedit").isExecutable())
            {
                AddApplication("gedit", "/usr/bin/gedit", "+(line) (file)");
                defapp = 0;
                break;
            }
            // use as default for kde environments
            if (QFileInfo("/usr/bin/kate").isExecutable())
            {
                AddApplication("kate", "/usr/bin/kate", "-l(line) (file)");
                defapp = 0;
                break;
            }
            if (FindDefaultWindowsEditor())
            {
                defapp = 0;
                break;
            }
        }
        while (0);
    }

    if (names.size() > 0 && (names.size() == paths.size()))
    {
        for (int i = 0; i < names.size(); i++)
            AddApplication(names[i], paths[i], params[i]);

        if (defapp == -1)
            mDefaultApplicationIndex = 0;
        else if (defapp < names.size())
            mDefaultApplicationIndex = defapp;
        else
            mDefaultApplicationIndex = 0;
    }
    return succeeded;
}

void ApplicationList::SaveSettings(QSettings *programSettings)
{
    QStringList names;
    QStringList paths;
    QStringList params;

    for (int i = 0; i < GetApplicationCount(); i++)
    {
        names << GetApplicationName(i);
        paths << GetApplicationPath(i);
        params << GetApplicationParameters(i);
    }

    programSettings->setValue(SETTINGS_APPLICATION_NAMES, names);
    programSettings->setValue(SETTINGS_APPLICATION_PATHS, paths);
    programSettings->setValue(SETTINGS_APPLICATION_PARAMS, params);
    programSettings->setValue(SETTINGS_APPLICATION_DEFAULT, mDefaultApplicationIndex);

}

int ApplicationList::GetApplicationCount() const
{
    return mApplications.size();
}

QString ApplicationList::GetApplicationName(const int index) const
{
    if (index >= 0 && index < mApplications.size())
    {
        return mApplications[index].getName();
    }

    return QString();
}

QString ApplicationList::GetApplicationPath(const int index) const
{
    if (index >= 0 && index < mApplications.size())
    {
        return mApplications[index].getPath();
    }

    return QString();
}

QString ApplicationList::GetApplicationParameters(const int index) const
{
    if (index >= 0 && index < mApplications.size())
    {
        return mApplications[index].getParameters();
    }

    return QString();
}

void ApplicationList::SetApplication(const int index,
                                     const QString &name,
                                     const QString &path,
                                     const QString &parameters)
{
    if (index >= 0 && index < mApplications.size())
    {
        Application app(name, path, parameters);
        mApplications.replace(index, app);
    }
}

void ApplicationList::AddApplication(const QString &name,
                                     const QString &path,
                                     const QString &parameters)
{
    if (name.isEmpty() || path.isEmpty())
    {
        return;
    }

    Application app(name, path, parameters);
    mApplications << app;
}

void ApplicationList::RemoveApplication(const int index)
{
    mApplications.removeAt(index);
}

void ApplicationList::SetDefault(const int index)
{
    if (index < mApplications.size() && index >= 0)
    {
        mDefaultApplicationIndex = index;
    }
}

void ApplicationList::Copy(const ApplicationList *list)
{
    if (!list)
    {
        return;
    }

    Clear();
    for (int i = 0; i < list->GetApplicationCount(); i++)
    {
        AddApplication(list->GetApplicationName(i), list->GetApplicationPath(i),
                       list->GetApplicationParameters(i));
    }
    mDefaultApplicationIndex = list->GetDefaultApplication();
}

void ApplicationList::Clear()
{
    mApplications.clear();
    mDefaultApplicationIndex = -1;
}

bool ApplicationList::FindDefaultWindowsEditor()
{
    const QString appPath(getenv("ProgramFiles"));
    const QString notepadppPath = appPath + "\\Notepad++\\notepad++.exe";
    if (QFileInfo(notepadppPath).isExecutable())
    {
        AddApplication("Notepad++", "\"" + notepadppPath + "\"", "-n(line) (file)");
        return true;
    }

    const QString windowsPath(getenv("windir"));
    const QString notepadPath = windowsPath + "\\system32\\notepad.exe";
    if (QFileInfo(notepadPath).isExecutable())
    {
        AddApplication("Notepad", notepadPath, "(file)");
        return true;
    }
    return false;
}

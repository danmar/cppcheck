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

#include "applicationlist.h"
#include <QStringList>
#include <QFileInfo>
#include <stdlib.h>
#include "common.h"

ApplicationList::ApplicationList(QObject *parent) :
    QObject(parent)
{
    //ctor
}

ApplicationList::~ApplicationList()
{
    Clear();
}

void ApplicationList::LoadSettings(QSettings *programSettings)
{

    QStringList names = programSettings->value(SETTINGS_APPLICATION_NAMES, QStringList()).toStringList();
    QStringList paths = programSettings->value(SETTINGS_APPLICATION_PATHS, QStringList()).toStringList();

    if (names.empty() && paths.empty())
    {
        do
        {
            // use as default for gnome environments
            if (QFileInfo("/usr/bin/gedit").isExecutable())
            {
                AddApplicationType("gedit", "/usr/bin/gedit +(line) (file)");
                break;
            }
            // use as default for kde environments
            if (QFileInfo("/usr/bin/kate").isExecutable())
            {
                AddApplicationType("kate", "/usr/bin/kate -l(line) (file)");
                break;
            }
            // use as default for windows environments
            if (FindDefaultWindowsEditor())
                break;
        }
        while (0);
    }

    if (names.size() == paths.size())
    {
        for (int i = 0; i < names.size(); i++)
        {
            AddApplicationType(names[i], paths[i]);
        }
    }
}

void ApplicationList::SaveSettings(QSettings *programSettings)
{
    QStringList names;
    QStringList paths;

    for (int i = 0; i < GetApplicationCount(); i++)
    {
        names << GetApplicationName(i);
        paths << GetApplicationPath(i);
    }

    programSettings->setValue(SETTINGS_APPLICATION_NAMES, names);
    programSettings->setValue(SETTINGS_APPLICATION_PATHS, paths);

}

int ApplicationList::GetApplicationCount() const
{
    return mApplications.size();
}

QString ApplicationList::GetApplicationName(const int index) const
{
    if (index >= 0 && index < mApplications.size())
    {
        return mApplications[index].Name;
    }

    return QString();
}

QString ApplicationList::GetApplicationPath(const int index) const
{
    if (index >= 0 && index < mApplications.size())
    {
        return mApplications[index].Path;
    }

    return QString();

}


void ApplicationList::SetApplicationType(const int index,
        const QString &name,
        const QString &path)
{
    if (index >= 0 && index < mApplications.size())
    {
        mApplications[index].Name = name;
        mApplications[index].Path = path;
    }
}

void ApplicationList::AddApplicationType(const QString &name, const QString &path)
{
    if (name.isEmpty() || path.isEmpty())
    {
        return;
    }

    ApplicationType type;
    type.Name = name;
    type.Path = path;
    mApplications << type;
}

void ApplicationList::RemoveApplication(const int index)
{
    mApplications.removeAt(index);
}


void ApplicationList::MoveFirst(const int index)
{
    if (index < mApplications.size() && index > 0)
    {
        mApplications.move(index, 0);
    }
}


void ApplicationList::Copy(ApplicationList *list)
{
    if (!list)
    {
        return;
    }

    Clear();
    for (int i = 0; i < list->GetApplicationCount(); i++)
    {
        AddApplicationType(list->GetApplicationName(i), list->GetApplicationPath(i));
    }
}

void ApplicationList::Clear()
{
    mApplications.clear();
}

bool ApplicationList::FindDefaultWindowsEditor()
{
    const QString appPath(getenv("ProgramFiles"));
    const QString notepadppPath = appPath + "\\Notepad++\\notepad++.exe";
    if (QFileInfo(notepadppPath).isExecutable())
    {
        AddApplicationType("Notepad++", "\"" + notepadppPath + "\" -n(line) (file)");
        return true;
    }

    const QString windowsPath(getenv("windir"));
    const QString notepadPath = windowsPath + "\\system32\\notepad.exe";
    if (QFileInfo(notepadPath).isExecutable())
    {
        AddApplicationType("Notepad", notepadPath + " (file)");
        return true;
    }
    return false;
}

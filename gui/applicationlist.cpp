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

#include <QFileInfo>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <stdlib.h>
#include "common.h"
#include "applicationlist.h"
#include "application.h"


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

bool ApplicationList::LoadSettings()
{
    QSettings settings;
    QStringList names = settings.value(SETTINGS_APPLICATION_NAMES, QStringList()).toStringList();
    QStringList paths = settings.value(SETTINGS_APPLICATION_PATHS, QStringList()).toStringList();
    QStringList params = settings.value(SETTINGS_APPLICATION_PARAMS, QStringList()).toStringList();
    int defapp = settings.value(SETTINGS_APPLICATION_DEFAULT, -1).toInt();

    // Params will be empty first time starting with the new setting.
    // Return false and inform user about problem with application settings.
    bool succeeded = true;
    if (!names.empty() && !paths.empty() && params.empty()) {
        for (int i = 0; i < paths.length(); i++)
            params << "";
        succeeded = false;
    }

    if (names.empty() && paths.empty() && params.empty()) {
#ifndef _WIN32
        // use as default for gnome environments
        if (QFileInfo("/usr/bin/gedit").isExecutable()) {
            Application app;
            app.setName("gedit");
            app.setPath("/usr/bin/gedit");
            app.setParameters("+(line) (file)");
            AddApplication(app);
            defapp = 0;
        }
        // use as default for kde environments
        if (QFileInfo("/usr/bin/kate").isExecutable()) {
            Application app;
            app.setName("kate");
            app.setPath("/usr/bin/kate");
            app.setParameters("-l(line) (file)");
            AddApplication(app);
            defapp = 0;
        }
#else
        if (FindDefaultWindowsEditor()) {
            defapp = 0;
        }
#endif
    } else if (names.size() == paths.size()) {
        for (int i = 0; i < names.size(); i++) {
            const Application app(names[i], paths[i], params[i]);
            AddApplication(app);
        }
    }

    if (defapp == -1)
        mDefaultApplicationIndex = 0;
    else if (defapp < names.size())
        mDefaultApplicationIndex = defapp;
    else
        mDefaultApplicationIndex = 0;

    return succeeded;
}

void ApplicationList::SaveSettings() const
{
    QSettings settings;
    QStringList names;
    QStringList paths;
    QStringList params;

    for (int i = 0; i < GetApplicationCount(); i++) {
        const Application& app = GetApplication(i);
        names << app.getName();
        paths << app.getPath();
        params << app.getParameters();
    }

    settings.setValue(SETTINGS_APPLICATION_NAMES, names);
    settings.setValue(SETTINGS_APPLICATION_PATHS, paths);
    settings.setValue(SETTINGS_APPLICATION_PARAMS, params);
    settings.setValue(SETTINGS_APPLICATION_DEFAULT, mDefaultApplicationIndex);
}

int ApplicationList::GetApplicationCount() const
{
    return mApplications.size();
}

Application& ApplicationList::GetApplication(const int index)
{
    if (index >= 0 && index < mApplications.size()) {
        return mApplications[index];
    }

    static Application dummy; // TODO: Throw exception instead?
    return dummy;
}

const Application& ApplicationList::GetApplication(const int index) const
{
    if (index >= 0 && index < mApplications.size()) {
        return mApplications[index];
    }

    static const Application dummy; // TODO: Throw exception instead?
    return dummy;
}

void ApplicationList::AddApplication(const Application &app)
{
    if (app.getName().isEmpty() || app.getPath().isEmpty()) {
        return;
    }
    mApplications << app;
}

void ApplicationList::RemoveApplication(const int index)
{
    mApplications.removeAt(index);
}

void ApplicationList::SetDefault(const int index)
{
    if (index < mApplications.size() && index >= 0) {
        mDefaultApplicationIndex = index;
    }
}

void ApplicationList::Copy(const ApplicationList *list)
{
    if (!list) {
        return;
    }

    Clear();
    for (int i = 0; i < list->GetApplicationCount(); i++) {
        const Application& app = list->GetApplication(i);
        AddApplication(app);
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
    bool foundOne = false;
    const QString appPath(getenv("ProgramFiles"));
    const QString notepadppPath = appPath + "\\Notepad++\\notepad++.exe";
    if (QFileInfo(notepadppPath).exists() && QFileInfo(notepadppPath).isExecutable()) {
        Application app;
        app.setName("Notepad++");
        app.setPath("\"" + notepadppPath + "\"");
        app.setParameters("-n(line) (file)");
        AddApplication(app);
        foundOne = true;
    }

    const QString notepadTwoPath = appPath + "\\Notepad2\\Notepad2.exe";
    if (QFileInfo(notepadTwoPath).exists() && QFileInfo(notepadTwoPath).isExecutable()) {
        Application app;
        app.setName("Notepad2");
        app.setPath("\"" + notepadTwoPath + "\"");
        app.setParameters("/g (line) (file)");
        AddApplication(app);
        foundOne = true;
    }

    const QString windowsPath(getenv("windir"));
    const QString notepadPath = windowsPath + "\\system32\\notepad.exe";
    if (QFileInfo(notepadPath).exists() && QFileInfo(notepadPath).isExecutable()) {
        Application app;
        app.setName("Notepad");
        app.setPath(notepadPath);
        app.setParameters("(file)");
        AddApplication(app);
        foundOne = true;
    }
    return foundOne;
}

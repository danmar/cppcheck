#include "applicationsettings.h"
#include <QSettings>

static const char RESULTSFOLDER[] = "resultsFolder";
static const char CMD_CLANG[]     = "clang";
static const char CMD_CPPCHECK[]  = "cppcheck";
static const char CMD_GCC[]       = "gcc";
static const char CURRENTPROJECT[] = "currentProject";

ApplicationSettings::ApplicationSettings()
{
    QSettings settings;
    resultsFolder  = settings.value(RESULTSFOLDER).toString();
    clang          = settings.value(CMD_CLANG).toString();
    cppcheck       = settings.value(CMD_CPPCHECK).toString();
    gcc            = settings.value(CMD_GCC).toString();
    currentProject = settings.value(CURRENTPROJECT).toString();
}

void ApplicationSettings::save()
{
    QSettings settings;
    settings.setValue(RESULTSFOLDER,  resultsFolder);
    settings.setValue(CMD_CLANG,      clang);
    settings.setValue(CMD_CPPCHECK,   cppcheck);
    settings.setValue(CMD_GCC,        gcc);
    settings.setValue(CURRENTPROJECT, currentProject);
}

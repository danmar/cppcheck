#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QString>

class ApplicationSettings {
public:
    ApplicationSettings();

    void save();

    QString resultsFolder;
    QString clang;
    QString cppcheck;
    QString gcc;
    QString currentProject;
};

#endif // APPLICATIONSETTINGS_H

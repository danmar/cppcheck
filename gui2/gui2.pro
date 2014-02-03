#-------------------------------------------------
#
# Project created by QtCreator 2014-01-31T06:45:11
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cppcheck-gui-2
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp \
    applicationsettings.cpp \
    resultsmodel.cpp \
    solution.cpp \
    configureprojects.cpp \
    projectwidget.cpp \
    resultsform.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    applicationsettings.h \
    resultsmodel.h \
    solution.h \
    configureprojects.h \
    projectwidget.h \
    resultsform.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    configureprojects.ui \
    resultsform.ui

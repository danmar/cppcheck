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
    configureprojects.cpp \
    projectwidget.cpp \
    resultsform.cpp \
    projectlist.cpp \
    codebrowser.cpp \
    graph.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    applicationsettings.h \
    resultsmodel.h \
    configureprojects.h \
    projectwidget.h \
    resultsform.h \
    projectlist.h \
    codebrowser.h \
    graph.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    configureprojects.ui \
    resultsform.ui

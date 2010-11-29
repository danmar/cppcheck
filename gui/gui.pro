TEMPLATE = app
TARGET = cppcheck-gui
QT += xml
CONFIG += warn_on help
DEPENDPATH += . \
    ../lib
INCLUDEPATH += . \
    ../lib

DESTDIR = .
RCC_DIR = temp
MOC_DIR = temp
OBJECTS_DIR = temp
UI_DIR = temp

win32 {
   DESTDIR = ../Build/gui
   RCC_DIR = ../BuildTmp/gui
   MOC_DIR = ../BuildTmp/gui
   OBJECTS_DIR = ../BuildTmp/gui
   UI_DIR = ../BuildTmp/gui
}

RESOURCES = gui.qrc
FORMS = main.ui \
    resultsview.ui \
    application.ui \
    settings.ui \
    file.ui \
    projectfile.ui \
    about.ui \
    logview.ui \
    helpwindow.ui \
    stats.ui

TRANSLATIONS =  cppcheck_fi.ts \
                cppcheck_nl.ts \
                cppcheck_en.ts \
                cppcheck_se.ts \
                cppcheck_de.ts \
                cppcheck_pl.ts \
                cppcheck_ru.ts 

# Windows-specific options
CONFIG += embed_manifest_exe

include($$PWD/../lib/lib.pri)
HEADERS += mainwindow.h \
           checkthread.h \
           resultsview.h \
           resultstree.h \
           settingsdialog.h \
           threadresult.h \
           threadhandler.h \
           applicationlist.h \
           applicationdialog.h \
           aboutdialog.h \
           common.h \
           erroritem.h \
           fileviewdialog.h \
           project.h \
           projectfile.h \
           projectfiledialog.h \
           report.h \
           txtreport.h \
           xmlreport.h \
           translationhandler.h \
           csvreport.h \
           logview.h \
           filelist.h \
           helpwindow.h \
           statsdialog.h \
           checkstatistics.h 

SOURCES += main.cpp \
           mainwindow.cpp\
           checkthread.cpp \
           resultsview.cpp \
           resultstree.cpp \
           threadresult.cpp \
           threadhandler.cpp \
           settingsdialog.cpp \
           applicationlist.cpp \
           applicationdialog.cpp \
           aboutdialog.cpp \
           fileviewdialog.cpp \
           project.cpp \
           projectfile.cpp \
           projectfiledialog.cpp \
           erroritem.cpp \
           report.cpp \
           txtreport.cpp \
           xmlreport.cpp \
           translationhandler.cpp \
           csvreport.cpp \
           logview.cpp \
           filelist.cpp \
           helpwindow.cpp \
           statsdialog.cpp \
           checkstatistics.cpp

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = cppcheck-gui.rc
    HEADERS += ../cli/resource.h
    LIBS += -lshlwapi
}


TEMPLATE = app
TARGET = cppcheck-gui
CONFIG += warn_on help
DEPENDPATH += . \
    ../lib
INCLUDEPATH += . \
    ../lib
LIBS += -L../externals

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
FORMS = about.ui \
        application.ui \
        file.ui \
        logview.ui \
        main.ui \
        projectfile.ui \
        resultsview.ui \
        settings.ui \
        stats.ui

TRANSLATIONS =  cppcheck_de.ts \
                cppcheck_en.ts \
                cppcheck_es.ts \
                cppcheck_fi.ts \
                cppcheck_fr.ts \
                cppcheck_ja.ts \
                cppcheck_nl.ts \
                cppcheck_pl.ts \
                cppcheck_ru.ts \
                cppcheck_sr.ts \
                cppcheck_sv.ts

# Windows-specific options
CONFIG += embed_manifest_exe

BASEPATH = ../lib/
include($$PWD/../lib/lib.pri)

HEADERS += aboutdialog.h \
           application.h \
           applicationdialog.h \
           applicationlist.h \
           checkstatistics.h \
           checkthread.h \
           common.h \
           csvreport.h \
           erroritem.h \
           filelist.h \
           fileviewdialog.h \
           logview.h \
           mainwindow.h \
           platforms.h \
           project.h \
           projectfile.h \
           projectfiledialog.h \
           report.h \
           resultstree.h \
           resultsview.h \
           settingsdialog.h \
           statsdialog.h \
           threadhandler.h \
           threadresult.h \
           translationhandler.h \
           txtreport.h \
           xmlreport.h \
           xmlreportv1.h \
           xmlreportv2.h

SOURCES += aboutdialog.cpp \
           application.cpp \
           applicationdialog.cpp \
           applicationlist.cpp \
           checkstatistics.cpp \
           checkthread.cpp \
           csvreport.cpp \
           erroritem.cpp \
           filelist.cpp \
           fileviewdialog.cpp \
           logview.cpp \
           main.cpp \
           mainwindow.cpp\
           platforms.cpp \
           project.cpp \
           projectfile.cpp \
           projectfiledialog.cpp \
           report.cpp \
           resultstree.cpp \
           resultsview.cpp \
           settingsdialog.cpp \
           statsdialog.cpp \
           threadhandler.cpp \
           threadresult.cpp \
           translationhandler.cpp \
           txtreport.cpp \
           xmlreport.cpp \
           xmlreportv1.cpp \
           xmlreportv2.cpp

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = cppcheck-gui.rc
    HEADERS += ../cli/resource.h
    LIBS += -lshlwapi
}

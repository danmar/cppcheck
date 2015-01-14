TEMPLATE = app
TARGET = cppcheck-gui
CONFIG += warn_on debug
DEPENDPATH += . \
    ../lib
INCLUDEPATH += . \
    ../lib

# In Qt 5 widgets are in separate module
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

contains(LINKCORE, [yY][eE][sS]) {
    LIBS += -l../bin/cppcheck-core
    DEFINES += CPPCHECKLIB_IMPORT
}
LIBS += -L$$PWD/../externals

DESTDIR = .
RCC_DIR = temp
MOC_DIR = temp
OBJECTS_DIR = temp
UI_DIR = temp

win32 {
   CONFIG += windows
   contains(LINKCORE, [yY][eE][sS]) {
      DESTDIR = ../bin
      RCC_DIR = temp/generated
      MOC_DIR = temp/generated
      OBJECTS_DIR = temp/generated
      UI_DIR = temp/generated
   } else {
      DESTDIR = ../Build/gui
      RCC_DIR = ../BuildTmp/gui
      MOC_DIR = ../BuildTmp/gui
      OBJECTS_DIR = ../BuildTmp/gui
      UI_DIR = ../BuildTmp/gui
   }
}

RESOURCES = gui.qrc
FORMS = about.ui \
        application.ui \
        file.ui \
        logview.ui \
        main.ui \
        projectfile.ui \
        resultsview.ui \
        scratchpad.ui \
        settings.ui \
        stats.ui

TRANSLATIONS =  cppcheck_de.ts \
                cppcheck_es.ts \
                cppcheck_fi.ts \
                cppcheck_fr.ts \
                cppcheck_it.ts \
                cppcheck_ja.ts \
                cppcheck_ko.ts \
                cppcheck_nl.ts \
                cppcheck_ru.ts \
                cppcheck_sr.ts \
                cppcheck_sv.ts \
                cppcheck_zh_CN.ts

# Windows-specific options
CONFIG += embed_manifest_exe

contains(LINKCORE, [yY][eE][sS]) {
} else {
    BASEPATH = ../lib/
    include($$PWD/../lib/lib.pri)
}

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
           scratchpad.h \
           settingsdialog.h \
           showtypes.h \
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
           common.cpp \
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
           scratchpad.cpp \
           settingsdialog.cpp \
           showtypes.cpp \
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
    HEADERS += ../lib/version.h
    LIBS += -lshlwapi
}

contains(QMAKE_CC, gcc) {
    QMAKE_CXXFLAGS += -std=c++0x
}

macx {
    contains(QMAKE_CXX, clang++) {
        QMAKE_CXXFLAGS += -std=c++11
    }
}

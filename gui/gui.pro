lessThan(QT_MAJOR_VERSION, 5): error(requires >= Qt 5 (You used: $$QT_VERSION))

TEMPLATE = app
TARGET = cppcheck-gui
CONFIG += warn_on debug
DEPENDPATH += . \
    ../lib
INCLUDEPATH += . \
    ../lib \
    ../externals/z3/include
QT += widgets
QT += printsupport
QT += help

# Build online help
onlinehelp.target = online-help.qhc
equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 12) {
    # qcollectiongenerator is used in case of QT version < 5.12
    onlinehelp.commands = qcollectiongenerator $$PWD/help/online-help.qhcp -o $$PWD/help/online-help.qhc
} else {
    onlinehelp.commands = qhelpgenerator $$PWD/help/online-help.qhcp -o $$PWD/help/online-help.qhc
}
QMAKE_EXTRA_TARGETS += onlinehelp
PRE_TARGETDEPS += online-help.qhc

contains(LINKCORE, [yY][eE][sS]) {
    LIBS += -l../bin/cppcheck-core
    DEFINES += CPPCHECKLIB_IMPORT
}
LIBS += -L$$PWD/../externals -L$$PWD/../externals/z3/bin

# z3
win32 {
    LIBS += -llibz3
} else {
    LIBS += -lz3
}
QMAKE_CXXFLAGS += -DUSE_Z3

DESTDIR = .
RCC_DIR = temp
MOC_DIR = temp
OBJECTS_DIR = temp
UI_DIR = temp

isEmpty(QMAKE_CXX) {
    isEmpty(CXX)) {
        QMAKE_CXX = gcc
    } else {
        QMAKE_CXX = $$(CXX)
    }
}

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
        functioncontractdialog.ui \
        helpdialog.ui \
        mainwindow.ui \
        projectfiledialog.ui \
        resultsview.ui \
        scratchpad.ui \
        settings.ui \
        stats.ui \
        librarydialog.ui \
        libraryaddfunctiondialog.ui \
        libraryeditargdialog.ui \
        newsuppressiondialog.ui \
        variablecontractsdialog.ui

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

win32-msvc* {
    MSVC_VER = $$(VisualStudioVersion)
    message($$MSVC_VER)
    MSVC_VER_SPLIT = $$split(MSVC_VER, .)
    MSVC_VER_MAJOR = $$first(MSVC_VER_SPLIT)
    # doesn't compile with older VS versions - assume VS2019 (16.x) is the first working for now
    !lessThan(MSVC_VER_MAJOR, 16) {
        message("using precompiled header")
        CONFIG += precompile_header
        PRECOMPILED_HEADER = precompiled_qmake.h
    }
}

HEADERS += aboutdialog.h \
           application.h \
           applicationdialog.h \
           applicationlist.h \
           checkstatistics.h \
           checkthread.h \
           codeeditstylecontrols.h \
           codeeditorstyle.h \
           codeeditstyledialog.h \
           codeeditor.h \
           common.h \
           csvreport.h \
           erroritem.h \
           filelist.h \
           fileviewdialog.h \
           functioncontractdialog.h \
           helpdialog.h \
           mainwindow.h \
           platforms.h \
           printablereport.h \
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
           variablecontractsdialog.h \
           xmlreport.h \
           xmlreportv2.h \
           librarydialog.h \
           cppchecklibrarydata.h \
           libraryaddfunctiondialog.h \
           libraryeditargdialog.h \
           newsuppressiondialog.h

SOURCES += aboutdialog.cpp \
           application.cpp \
           applicationdialog.cpp \
           applicationlist.cpp \
           checkstatistics.cpp \
           checkthread.cpp \
           codeeditorstyle.cpp \
           codeeditstylecontrols.cpp \
           codeeditstyledialog.cpp \
           codeeditor.cpp \
           common.cpp \
           csvreport.cpp \
           erroritem.cpp \
           filelist.cpp \
           fileviewdialog.cpp \
           functioncontractdialog.cpp \
           helpdialog.cpp \
           main.cpp \
           mainwindow.cpp\
           platforms.cpp \
           printablereport.cpp \
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
           variablecontractsdialog.cpp \
           xmlreport.cpp \
           xmlreportv2.cpp \
           librarydialog.cpp \
           cppchecklibrarydata.cpp \
           libraryaddfunctiondialog.cpp \
           libraryeditargdialog.cpp \
           newsuppressiondialog.cpp

win32 {
    RC_FILE = cppcheck-gui.rc
    HEADERS += ../lib/version.h
    contains(LINKCORE, [yY][eE][sS]) {
    } else {
        LIBS += -lshlwapi
    }
}

contains(QMAKE_CC, gcc) {
    QMAKE_CXXFLAGS += -std=c++0x -pedantic -Wall -Wextra -Wcast-qual -Wno-deprecated-declarations -Wfloat-equal -Wmissing-declarations -Wmissing-format-attribute -Wno-long-long -Wpacked -Wredundant-decls -Wundef -Wno-shadow -Wno-missing-field-initializers -Wno-missing-braces -Wno-sign-compare -Wno-multichar
}

contains(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -std=c++0x -pedantic -Wall -Wextra -Wcast-qual -Wno-deprecated-declarations -Wfloat-equal -Wmissing-declarations -Wmissing-format-attribute -Wno-long-long -Wpacked -Wredundant-decls -Wundef -Wno-shadow -Wno-missing-field-initializers -Wno-missing-braces -Wno-sign-compare -Wno-multichar
}

contains(HAVE_QCHART, [yY][eE][sS]) {
    QT += charts
    DEFINES += HAVE_QCHART
} else {
    message("Charts disabled - to enable it pass HAVE_QCHART=yes to qmake.")
}


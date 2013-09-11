TEMPLATE = app
TARGET = cppcheck
DEPENDPATH += .
INCLUDEPATH += . ../lib
OBJECTS_DIR = temp
CONFIG += warn_on
CONFIG -= qt app_bundle

include(../console_common.pri)

BASEPATH = ../lib/
include($$PWD/../lib/lib.pri)

SOURCES += main.cpp \
           cppcheckexecutor.cpp \
           cmdlineparser.cpp \
           filelister.cpp \
           pathmatch.cpp \
           threadexecutor.cpp

HEADERS += cppcheckexecutor.h \
           cmdlineparser.h \
           filelister.h \
           pathmatch.h \
           threadexecutor.h

win32 {
    RC_FILE = version.rc
    HEADERS += ../lib/version.h
}

# Enable STL checking in GCC debug builds
contains(QMAKE_CXX, g++) {
    CONFIG(debug, debug|release) {
        # checked STL
        DEFINES += _GLIBCXX_DEBUG
    }
}

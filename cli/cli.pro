TEMPLATE = app
TARGET = cppcheck
DEPENDPATH += .
TINYXML_DIR = ../externals/tinyxml/
INCLUDEPATH += . ../lib $${TINYXML_DIR}
OBJECTS_DIR = temp
CONFIG += warn_on
CONFIG -= qt app_bundle

BASEPATH = $${TINYXML_DIR}
include($$PWD/$${TINYXML_DIR}tinyxml.pri)
BASEPATH = ../lib/
include($$PWD/../lib/lib.pri)

SOURCES += main.cpp \
           cppcheckexecutor.cpp \
           cmdlineparser.cpp \
           filelister.cpp \
           filelister_unix.cpp \
           filelister_win32.cpp \
           threadexecutor.cpp
HEADERS += cppcheckexecutor.h \
           cmdlineparser.h \
           filelister.h \
           filelister_unix.h \
           filelister_win32.h \
           threadexecutor.h

CONFIG(release, debug|release) {
	DEFINES += NDEBUG
}

win32 {
    CONFIG += embed_manifest_exe console
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = cppcheck.rc
    HEADERS += resource.h
    LIBS += -lshlwapi
}

# Add more strict compiling flags for GCC
contains(QMAKE_CXX, g++) {
    QMAKE_CXXFLAGS_WARN_ON += -Wextra -pedantic -Wfloat-equal -Wcast-qual -Wlogical-op -Wno-long-long

    CONFIG(debug, debug|release) {
        # checked STL
        DEFINES += _GLIBCXX_DEBUG
    }
}

# Change Visual Studio compiler (CL) warning level to W4
contains(QMAKE_CXX, cl) {
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
}

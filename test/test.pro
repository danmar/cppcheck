
TEMPLATE = app
TARGET = testrunner
DEPENDPATH += .
INCLUDEPATH += . ../cli ../lib
OBJECTS_DIR = temp
CONFIG += warn_on console
CONFIG -= qt app_bundle
win32 {
    LIBS += -lshlwapi
}

BASEPATH = ../externals/tinyxml/
include(../externals/tinyxml/tinyxml.pri)
BASEPATH = ../lib/
include(../lib/lib.pri)
BASEPATH = .
include($$PWD/testfiles.pri)

# cli/*
SOURCES += ../cli/cmdlineparser.cpp \
           ../cli/cppcheckexecutor.cpp \
           ../cli/filelister.cpp \
           ../cli/pathmatch.cpp \
           ../cli/threadexecutor.cpp

HEADERS += ../cli/cmdlineparser.h \
           ../cli/cppcheckexecutor.h \
           ../cli/filelister.h \
           ../cli/pathmatch.h \
           ../cli/threadexecutor.h

# test/*

HEADERS += options.h redirect.h testsuite.h
SOURCES += options.cpp

# Change Visual Studio compiler (CL) warning level to W4
contains(QMAKE_CXX, cl) {
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

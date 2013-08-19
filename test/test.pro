TEMPLATE = app
TARGET = testrunner
DEPENDPATH += .
INCLUDEPATH += . ../cli ../lib
OBJECTS_DIR = temp
CONFIG += warn_on console
CONFIG -= qt app_bundle

include(../console_common.pri)

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

HEADERS += options.h redirect.h testsuite.h
SOURCES += options.cpp

TEMPLATE = app
TARGET = test-projectfile
DEPENDPATH += .
INCLUDEPATH += . ../../../externals/simplecpp
OBJECTS_DIR = ../build
MOC_DIR = ../build

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testprojectfile.cpp \
    ../../projectfile.cpp \
    ../../../lib/path.cpp \
    ../../../externals/simplecpp/simplecpp.cpp

HEADERS += testprojectfile.h \
    ../../projectfile.h \
    ../../../lib/path.h \
    ../../../externals/simplecpp/simplecpp.h

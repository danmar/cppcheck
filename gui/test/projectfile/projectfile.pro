TEMPLATE = app
TARGET = test-projectfile
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testprojectfile.cpp

HEADERS += testprojectfile.h

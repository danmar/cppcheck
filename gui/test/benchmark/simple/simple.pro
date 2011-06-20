TEMPLATE = app
TARGET = benchmark-simple
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../../build
MOC_DIR = ../../build

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += benchmarksimple.cpp

HEADERS += benchmarksimple.h

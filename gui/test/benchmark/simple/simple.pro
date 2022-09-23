TEMPLATE = app
TARGET = benchmark-simple
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../../../temp
MOC_DIR = ../../temp

include(../../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += benchmarksimple.cpp

HEADERS += benchmarksimple.h

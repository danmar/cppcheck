TEMPLATE = app
TARGET = test-filelist
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build
QT += testlib

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testfilelist.cpp

HEADERS += testfilelist.h

TEMPLATE = app
TARGET = test-xmlreportv2
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testxmlreportv2.cpp

HEADERS += testxmlreportv2.h

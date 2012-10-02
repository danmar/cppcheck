TEMPLATE = app
TARGET = test-xmlreportv1
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testxmlreportv1.cpp

HEADERS += testxmlreportv1.h

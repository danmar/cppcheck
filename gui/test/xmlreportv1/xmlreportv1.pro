TEMPLATE = app
TARGET = test-xmlreportv1
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build

DEFINES += SRCDIR=\\\"$$PWD\\\"

include(../common.pri)

# tests
SOURCES += testxmlreportv1.cpp

HEADERS += testxmlreportv1.h

TEMPLATE = app
TARGET = test-xmlreport
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testxmlreport.cpp

HEADERS += testxmlreport.h

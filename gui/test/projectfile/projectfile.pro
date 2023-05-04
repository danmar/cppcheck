TEMPLATE = app
TARGET = test-projectfile
DEPENDPATH += .
INCLUDEPATH += . ../../../externals/simplecpp ../../../externals/tinyxml2 ../../../externals/picojson
OBJECTS_DIR = ../../temp
MOC_DIR = ../../temp

QT -= gui
QT += core
QT += testlib

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testprojectfile.cpp \
    ../../projectfile.cpp

HEADERS += testprojectfile.h \
    ../../projectfile.h \
    ../../../externals/picojson/picojson.h

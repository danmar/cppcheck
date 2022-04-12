TEMPLATE = app
TARGET = test-projectfile
DEPENDPATH += .
INCLUDEPATH += . ../../../externals/simplecpp ../../../externals/tinyxml2 ../../../externals/picojson
OBJECTS_DIR = ../build
MOC_DIR = ../build
QT -= gui
QT += core
CONFIG += console

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testprojectfile.cpp \
    ../../projectfile.cpp

HEADERS += testprojectfile.h \
    ../../projectfile.h \
    ../../../externals/picojson/picojson.h

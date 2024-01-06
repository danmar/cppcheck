TEMPLATE = app
TARGET = test-projectfile
DEPENDPATH += .
INCLUDEPATH += . ../../../externals/simplecpp ../../../externals/tinyxml2 ../../../externals/picojson ../../../externals/valijson
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
    ../../../externals/picojson/picojson.h \
    ../../../externals/valijson/valijson_picojson_bundled.hpp

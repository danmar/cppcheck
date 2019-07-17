TEMPLATE = app
TARGET = test-filelist
DEPENDPATH += .
INCLUDEPATH += . ../../../externals/simplecpp
OBJECTS_DIR = ../build
MOC_DIR = ../build
QT += testlib

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testfilelist.cpp \
    ../../filelist.cpp \
    ../../../lib/pathmatch.cpp \
    ../../../lib/path.cpp \
    ../../../externals/simplecpp/simplecpp.cpp

HEADERS += testfilelist.h \
    ../../filelist.h \
    ../../../lib/pathmatch.h \
    ../../../lib/path.h \
    ../../../externals/simplecpp/simplecpp.h

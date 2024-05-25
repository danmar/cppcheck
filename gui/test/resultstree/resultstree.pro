TEMPLATE = app
TARGET = test-resultstree
DEPENDPATH += .
INCLUDEPATH += . ../../../lib
OBJECTS_DIR = ../../temp
MOC_DIR = ../../temp

QT += widgets core
QT += testlib

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testresultstree.cpp \
    ../../resultstree.cpp \
    ../../erroritem.cpp \
    ../../showtypes.cpp \
    ../../report.cpp \
    ../../xmlreportv2.cpp

HEADERS += testresultstree.h \
    ../../resultstree.h \
    ../../erroritem.h \
    ../../showtypes.h \
    ../../report.h \
    ../../xmlreportv2.h

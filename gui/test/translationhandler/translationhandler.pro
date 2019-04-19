TEMPLATE = app
TARGET = test-translationhandler
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build
QT += widgets testlib

include(../common.pri)

# tests
SOURCES += testtranslationhandler.cpp

HEADERS += testtranslationhandler.h

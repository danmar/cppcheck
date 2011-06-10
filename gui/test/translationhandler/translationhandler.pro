TEMPLATE = app
TARGET = test-translationhandler
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build

include(../common.pri)

# tests
SOURCES += testtranslationhandler.cpp

HEADERS += testtranslationhandler.h

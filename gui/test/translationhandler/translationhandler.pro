TEMPLATE = app
TARGET = test-translationhandler
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../../temp
MOC_DIR = ../../temp

QT -= gui
QT += core
QT += widgets # TODO: get rid of this - causes X server dependency
QT += testlib

include(../common.pri)

# tests
SOURCES += testtranslationhandler.cpp \
    ../../translationhandler.cpp \
    ../../common.cpp

HEADERS += testtranslationhandler.h \
    ../../translationhandler.h \
    ../../common.h

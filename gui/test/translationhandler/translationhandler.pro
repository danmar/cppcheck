TEMPLATE = app
TARGET = test-translationhandler
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../../temp
MOC_DIR = ../../temp
QT += widgets

include(../common.pri)

# tests
SOURCES += testtranslationhandler.cpp \
    ../../translationhandler.cpp \
    ../../common.cpp

HEADERS += testtranslationhandler.h \
    ../../translationhandler.h \
    ../../common.h

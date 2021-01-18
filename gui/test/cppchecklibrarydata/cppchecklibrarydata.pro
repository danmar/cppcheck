TEMPLATE = app
TARGET = test-cppchecklibrarydata
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../build
MOC_DIR = ../build

QT -= gui
QT += core
CONFIG += console

include(../common.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

SOURCES += testcppchecklibrarydata.cpp \
    ../../cppchecklibrarydata.cpp

HEADERS += testcppchecklibrarydata.h \
    ../../cppchecklibrarydata.h \

RESOURCES += \
    resources.qrc

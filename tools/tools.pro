TEMPLATE = app
TARGET = dmake
DEPENDPATH += .
INCLUDEPATH += . ../lib
OBJECTS_DIR = temp
CONFIG += warn_on
CONFIG -= qt app_bundle

include(../console_common.pri)

SOURCES += dmake.cpp \
           ../cli/filelister.cpp \
           ../lib/path.cpp

HEADERS += ../cli/filelister.h \
           ../lib/path.h

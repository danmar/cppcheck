TEMPLATE = app
TARGET = test-xmlreportv2
DEPENDPATH += .
INCLUDEPATH += . ../../../externals/simplecpp
OBJECTS_DIR = ../../temp
MOC_DIR = ../../temp

include(../common.pri)
include(../../../lib/lib.pri)

DEFINES += SRCDIR=\\\"$$PWD\\\"

# tests
SOURCES += testxmlreportv2.cpp \
    ../../erroritem.cpp \
    ../../report.cpp \
    ../../xmlreport.cpp \
    ../../xmlreportv2.cpp

HEADERS += testxmlreportv2.h \
    ../../erroritem.h \
    ../../report.h \
    ../../xmlreport.cpp \
    ../../xmlreportv2.h

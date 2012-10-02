CONFIG += qtestlib
#DEPENDPATH += . ..
INCLUDEPATH += . ../.. ../../../../lib

LIBS += -L../../../../externals -lpcre
INCLUDEPATH += ../../../externals

BASEPATH = ../../../../lib/
include($$PWD/../../../lib/lib.pri)

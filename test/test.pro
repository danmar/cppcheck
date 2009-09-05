TEMPLATE = app
TARGET = test
DEPENDPATH += .
INCLUDEPATH += ../src
OBJECTS_DIR = temp
CONFIG += warn_on debug
CONFIG -= qt app_bundle
DEFINES += UNIT_TESTING

include($$PWD/../src/src.pri)
HEADERS += testsuite.h
SOURCES += testautovariables.cpp \
           testbufferoverrun.cpp \
           testcharvar.cpp \
           testclass.cpp \
           testconstructors.cpp \
           testcppcheck.cpp \
           testdangerousfunctions.cpp \
           testdivision.cpp \
           testfilelister.cpp \
           testincompletestatement.cpp \
           testmathlib.cpp \
           testmemleak.cpp \
           testother.cpp \
           testpreprocessor.cpp \
           testredundantif.cpp \
           testrunner.cpp \
           testsimplifytokens.cpp \
           teststl.cpp \
           testsuite.cpp \
           testtoken.cpp \
           testtokenize.cpp \
           testunusedfunctions.cpp \
           testunusedprivfunc.cpp \
           testunusedvar.cpp

win32 {
    CONFIG += console
    LIBS += -lshlwapi
}

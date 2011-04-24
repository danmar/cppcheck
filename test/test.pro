
TEMPLATE = app
TARGET = test
DEPENDPATH += .
INCLUDEPATH += . ../cli ../lib
OBJECTS_DIR = temp
CONFIG += warn_on console
CONFIG -= qt app_bundle
win32 {
    LIBS += -lshlwapi
}

BASEPATH = ../externals/tinyxml/
include(../externals/tinyxml/tinyxml.pri)
BASEPATH = ../lib/
include(../lib/lib.pri)

# cli/*
SOURCES += ../cli/cmdlineparser.cpp \
           ../cli/cppcheckexecutor.cpp \
           ../cli/filelister.cpp \
           ../cli/pathmatch.cpp \
           ../cli/threadexecutor.cpp \
    testpathmatch.cpp
HEADERS += ../cli/cmdlineparser.h \
           ../cli/cppcheckexecutor.h \
           ../cli/filelister.h \
           ../cli/pathmatch.h \
           ../cli/threadexecutor.h

# test/*

# Note:
# testfilelister_unix.cpp omitted since there is test fail when run in QtCreator
# Test assumes the test (executable) is built from the test directory (or
# directory containing source files). But QtCreator builds to separate build
# directory. Hence the test does not find the source files.

HEADERS += options.h redirect.h testsuite.h
SOURCES += options.cpp \
           testautovariables.cpp \
           testbufferoverrun.cpp \
           testcharvar.cpp \
           testclass.cpp \
           testcmdlineparser.cpp \
           testconstructors.cpp \
           testcppcheck.cpp \
           testdivision.cpp \
           testerrorlogger.cpp \
           testexceptionsafety.cpp \
           testfilelister.cpp \
           testincompletestatement.cpp \
           testmathlib.cpp \
           testmemleak.cpp \
           testnullpointer.cpp \
           testobsoletefunctions.cpp \
           testoptions.cpp \
           testother.cpp \
           testpath.cpp \
           testpathmatch.cpp \
           testpostfixoperator.cpp \
           testpreprocessor.cpp \
           testrunner.cpp \
           testsettings.cpp \
           testsimplifytokens.cpp \
           teststl.cpp \
           testsuite.cpp \
           testsuppressions.cpp \
           testsymboldatabase.cpp \
           testthreadexecutor.cpp \
           testtoken.cpp \
           testtokenize.cpp \
           testuninitvar.cpp \
           testunusedfunctions.cpp \
           testunusedprivfunc.cpp \
           testunusedvar.cpp

# Change Visual Studio compiler (CL) warning level to W4
contains(QMAKE_CXX, cl) {
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

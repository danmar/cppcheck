
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
           ../cli/filelister_unix.cpp \
           ../cli/filelister_win32.cpp \
           ../cli/pathmatch.cpp \
           ../cli/threadexecutor.cpp \
    testpathmatch.cpp
HEADERS += ../cli/cmdlineparser.h \
           ../cli/cppcheckexecutor.h \
           ../cli/filelister.h \
           ../cli/filelister_unix.h \
           ../cli/filelister_win32.h \
           ../cli/pathmatch.h \
           ../cli/threadexecutor.h

# test/*
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

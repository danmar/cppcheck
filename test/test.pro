TEMPLATE = app
TARGET = test
DEPENDPATH += .
INCLUDEPATH += ../lib
OBJECTS_DIR = temp
CONFIG += warn_on debug
CONFIG -= qt app_bundle
DEFINES += UNIT_TESTING

include($$PWD/../lib/lib.pri)
HEADERS += testsuite.h
SOURCES += testautovariables.cpp \
           testbufferoverrun.cpp \
           testcharvar.cpp \
           testclass.cpp \
           testcmdlineparser.cpp \
           testconstructors.cpp \
           testcppcheck.cpp \
           testdivision.cpp \
           testexceptionsafety.cpp \
           testfilelister.cpp \
           testincompletestatement.cpp \
           testmathlib.cpp \
           testmemleak.cpp \
           testobsoletefunctions.cpp \
           testother.cpp \
           testpreprocessor.cpp \
           testrunner.cpp \
           testsettings.cpp \
           testsimplifytokens.cpp \
           teststl.cpp \
           testsuite.cpp \
           testthreadexecutor.cpp \
           testtoken.cpp \
           testtokenize.cpp \
           testunusedfunctions.cpp \
           testunusedprivfunc.cpp \
           testunusedvar.cpp

win32 {
    CONFIG += console
    LIBS += -lshlwapi
}

contains(QMAKE_CXX, g++) {
    QMAKE_CXXFLAGS_WARN_ON += -Wextra -pedantic

    CONFIG(debug, debug|release) {
        # checked STL
        DEFINES += _GLIBCXX_DEBUG
    }
}



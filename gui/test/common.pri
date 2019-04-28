QT += testlib

INCLUDEPATH += $${PWD}/.. \
    $${PWD}/../../lib

contains(QMAKE_CC, gcc) {
    QMAKE_CXXFLAGS += -std=c++11
}

contains(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -std=c++11
}

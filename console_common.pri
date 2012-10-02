# console_common.pri
# These are common definitions for console builds.

win32 {
    CONFIG += embed_manifest_exe console
    DEFINES += _CRT_SECURE_NO_WARNINGS
    LIBS += -lshlwapi
}

# Add more strict compiling flags for GCC
contains(QMAKE_CXX, g++) {
    QMAKE_CXXFLAGS_WARN_ON += -Wextra -pedantic -Wfloat-equal -Wcast-qual -Wlogical-op -Wno-long-long
}

# Change Visual Studio compiler (CL) warning level to W4
contains(QMAKE_CXX, cl) {
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
}

CONFIG(release, debug|release) {
    DEFINES += NDEBUG
}

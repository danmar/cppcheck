#!/bin/bash
set -e # abort on error
set -x # be verbose

if [[ $(pwd) == */test/cfg ]] ; then # we are in test/cfg
	CPPCHECK="../../cppcheck"
	DIR=""
else # assume we are in repo root
	CPPCHECK="./cppcheck"
	DIR=test/cfg/
fi

# Cppcheck options
CPPCHECK_OPT='--check-library --enable=information --enable=style --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --template="{file}:{line}:{severity}:{id}:{message}"'

# Compiler settings
CXX=g++
CXX_OPT='-fsyntax-only -std=c++0x -Wno-format -Wno-format-security'
CC=gcc
CC_OPT='-Wno-format -Wno-nonnull -Wno-implicit-function-declaration -Wno-deprecated-declarations -Wno-format-security -Wno-nonnull -fsyntax-only'

# posix.c
${CC} ${CC_OPT} ${DIR}posix.c
${CPPCHECK} ${CPPCHECK_OPT} --library=posix  ${DIR}posix.c

# gnu.c
${CC} ${CC_OPT} -D_GNU_SOURCE ${DIR}gnu.c
${CPPCHECK} ${CPPCHECK_OPT} --library=gnu ${DIR}gnu.c

# qt.cpp
${CXX} ${CXX_OPT} ${DIR}qt.cpp
${CPPCHECK} --enable=style --enable=information --inconclusive --inline-suppr --error-exitcode=1 --library=qt ${DIR}qt.cpp

# std.c
${CC} ${CC_OPT} ${DIR}std.c
${CPPCHECK} ${CPPCHECK_OPT} ${DIR}std.c
${CXX} ${CXX_OPT} ${DIR}std.cpp
${CPPCHECK} ${CPPCHECK_OPT} ${DIR}std.cpp

# windows.cpp
# Syntax check via g++ does not work because it can not find a valid windows.h
#${CXX} ${CXX_OPT} ${DIR}windows.cpp
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --platform=win32A  ${DIR}windows.cpp
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --platform=win32W  ${DIR}windows.cpp
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --platform=win64  ${DIR}windows.cpp

# wxwidgets.cpp
set +e
WXCONFIG=`wx-config --cxxflags`
WXCONFIG_RETURNCODE=$?
set -e
if [ $WXCONFIG_RETURNCODE -ne 0 ]; then
    echo "wx-config does not work, skipping syntax check for wxWidgets tests."
else
    set +e
    echo -e "#include <wx/filefn.h>\n#include <wx/app.h>\n#include <wx/artprov.h>\n#include <wx/version.h>\n#if wxVERSION_NUMBER<2950\n#error \"Old version\"\n#endif" | ${CXX} ${CXX_OPT} ${WXCONFIG} -x c++ -
    WXCHECK_RETURNCODE=$?
    set -e
    if [ $WXCHECK_RETURNCODE -ne 0 ]; then
        echo "wxWidgets not completely present (with GUI classes) or not working, skipping syntax check with ${CXX}."
    else
        echo "wxWidgets found, checking syntax with ${CXX} now."
        ${CXX} ${CXX_OPT} ${WXCONFIG} -Wno-deprecated-declarations ${DIR}wxwidgets.cpp
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=wxwidgets -f ${DIR}wxwidgets.cpp

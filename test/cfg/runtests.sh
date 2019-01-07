#!/bin/bash
set -e # abort on error
#set -x # be verbose

if [[ $(pwd) == */test/cfg ]] ; then # we are in test/cfg
	CPPCHECK="../../cppcheck"
	DIR=""
	CFG="../../cfg/"
else # assume we are in repo root
	CPPCHECK="./cppcheck"
	DIR=test/cfg/
	CFG="cfg/"
fi

# Cppcheck options
CPPCHECK_OPT='--check-library --enable=information --enable=style --error-exitcode=-1 --suppress=missingIncludeSystem --inline-suppr --template="{file}:{line}:{severity}:{id}:{message}"'

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
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve Qt configuration is not available, skipping syntax check."
else
    set +e
    QTCONFIG=$(pkg-config --cflags Qt5Core)
    QTCONFIG_RETURNCODE=$?
    set -e
    if [ $QTCONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <QString>" | ${CXX} ${CXX_OPT} ${QTCONFIG} -x c++ -
        QTCHECK_RETURNCODE=$?
        set -e
        if [ $QTCHECK_RETURNCODE -ne 0 ]; then
            echo "Qt not completely present or not working, skipping syntax check with ${CXX}."
        else
            echo "Qt found and working, checking syntax with ${CXX} now."
            ${CXX} ${CXX_OPT} ${QTCONFIG} ${DIR}qt.cpp
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=qt ${DIR}qt.cpp

# bsd.c
${CPPCHECK} ${CPPCHECK_OPT} --library=bsd ${DIR}bsd.c

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
WXCONFIG=$(wx-config --cxxflags)
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

# gtk.c
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve GTK+ configuration is not available, skipping syntax check."
else
    set +e
    GTKCONFIG=$(pkg-config --cflags gtk+-3.0)
    GTKCONFIG_RETURNCODE=$?
    set -e
    if [ $GTKCONFIG_RETURNCODE -ne 0 ]; then
        set +e
        GTKCONFIG=$(pkg-config --cflags gtk+-2.0)
        GTKCONFIG_RETURNCODE=$?
        set -e
    fi
    if [ $GTKCONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <gtk/gtk.h>" | ${CC} ${CC_OPT} ${GTKCONFIG} -x c -
        GTKCHECK_RETURNCODE=$?
        set -e
        if [ $GTKCHECK_RETURNCODE -ne 0 ]; then
            echo "GTK+ not completely present or not working, skipping syntax check with ${CXX}."
        else
            echo "GTK+ found and working, checking syntax with ${CXX} now."
            ${CC} ${CC_OPT} ${GTKCONFIG} ${DIR}gtk.c
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=gtk -f ${DIR}gtk.c

# Check the syntax of the defines in the configuration files
set +e
xmlstarlet --version
XMLSTARLET_RETURNCODE=$?
set -e
if [ $XMLSTARLET_RETURNCODE -ne 0 ]; then
    echo "xmlstarlet needed to extract defines, skipping defines check."
else
    for configfile in ${CFG}*.cfg; do
        echo "Checking defines in $configfile"
        # Disable debugging output temporarily since there could be many defines
        set +x
        # XMLStarlet returns 1 if no elements were found which is no problem here
        set +e
        EXTRACTED_DEFINES=$(xmlstarlet sel -t -m '//define' -c . -n <$configfile)
        set -e
        EXTRACTED_DEFINES=$(echo "$EXTRACTED_DEFINES" | sed 's/<define name="/#define /g' | sed 's/" value="/ /g' | sed 's/"\/>//g')
        echo "$EXTRACTED_DEFINES" | gcc -fsyntax-only -xc -Werror -
    done
fi

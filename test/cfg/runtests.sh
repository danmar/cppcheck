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
${CPPCHECK} ${CPPCHECK_OPT} --library=posix,gnu ${DIR}gnu.c

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
        QTBUILDCONFIG=$(pkg-config --variable=qt_config Qt5Core)
        [[ $QTBUILDCONFIG =~ (^|[[:space:]])reduce_relocations($|[[:space:]]) ]] && QTCONFIG="${QTCONFIG} -fPIC"
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
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive ${DIR}std.c
${CXX} ${CXX_OPT} ${DIR}std.cpp
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive ${DIR}std.cpp

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

# boost.cpp
set +e
echo -e "#include <boost/config.hpp>" | ${CXX} ${CXX_OPT} -x c++ -
BOOSTCHECK_RETURNCODE=$?
set -e
if [ ${BOOSTCHECK_RETURNCODE} -ne 0 ]; then
    echo "Boost not completely present or not working, skipping syntax check with ${CXX}."
else
    echo "Boost found and working, checking syntax with ${CXX} now."
    ${CXX} ${CXX_OPT} ${DIR}boost.cpp
fi
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=boost ${DIR}boost.cpp

# sqlite3.c
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve SQLite3 configuration is not available, skipping syntax check."
else
    set +e
    SQLITE3CONFIG=$(pkg-config --cflags sqlite3)
    SQLITE3CONFIG_RETURNCODE=$?
    set -e
    if [ $SQLITE3CONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <sqlite3.h>" | ${CC} ${CC_OPT} ${SQLITE3CONFIG} -x c -
        SQLITE3CHECK_RETURNCODE=$?
        set -e
        if [ $SQLITE3CHECK_RETURNCODE -ne 0 ]; then
            echo "SQLite3 not completely present or not working, skipping syntax check with ${CC}."
        else
            echo "SQLite3 found and working, checking syntax with ${CC} now."
            ${CC} ${CC_OPT} ${SQLITE3CONFIG} ${DIR}sqlite3.c
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=sqlite3 ${DIR}sqlite3.c

# openmp.c
${CC} ${CC_OPT} -fopenmp ${DIR}openmp.c
${CPPCHECK} ${CPPCHECK_OPT} --library=openmp ${DIR}openmp.c

# python.c
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve Python 3 configuration is not available, skipping syntax check."
else
    set +e
    PYTHON3CONFIG=$(pkg-config --cflags python3)
    PYTHON3CONFIG_RETURNCODE=$?
    set -e
    if [ $PYTHON3CONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <Python.h>" | ${CC} ${CC_OPT} ${PYTHON3CONFIG} -x c -
        PYTHON3CONFIG_RETURNCODE=$?
        set -e
        if [ $PYTHON3CONFIG_RETURNCODE -ne 0 ]; then
            echo "Python 3 not completely present or not working, skipping syntax check with ${CC}."
        else
            echo "Python 3 found and working, checking syntax with ${CC} now."
            ${CC} ${CC_OPT} ${PYTHON3CONFIG} ${DIR}python.c
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --library=python ${DIR}python.c

# lua.c
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve Lua configuration is not available, skipping syntax check."
else
    set +e
    LUACONFIG=$(pkg-config --cflags lua-5.3)
    LUACONFIG_RETURNCODE=$?
    set -e
    if [ $LUACONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <lua.h>" | ${CC} ${CC_OPT} ${LUACONFIG} -x c -
        LUACONFIG_RETURNCODE=$?
        set -e
        if [ $LUACONFIG_RETURNCODE -ne 0 ]; then
            echo "Lua not completely present or not working, skipping syntax check with ${CC}."
        else
            echo "Lua found and working, checking syntax with ${CC} now."
            ${CC} ${CC_OPT} ${LUACONFIG} ${DIR}lua.c
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --library=lua ${DIR}lua.c

# libcurl.c
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve libcurl configuration is not available, skipping syntax check."
else
    set +e
    LIBCURLCONFIG=$(pkg-config --cflags libcurl)
    LIBCURLCONFIG_RETURNCODE=$?
    set -e
    if [ $LIBCURLCONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <curl/curl.h>" | ${CC} ${CC_OPT} ${LIBCURLCONFIG} -x c -
        LIBCURLCONFIG_RETURNCODE=$?
        set -e
        if [ $LIBCURLCONFIG_RETURNCODE -ne 0 ]; then
            echo "libcurl not completely present or not working, skipping syntax check with ${CC}."
        else
            echo "libcurl found and working, checking syntax with ${CC} now."
            ${CC} ${CC_OPT} ${LIBCURLCONFIG} ${DIR}libcurl.c
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --library=libcurl ${DIR}libcurl.c

# cairo.c
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve cairo configuration is not available, skipping syntax check."
else
    set +e
    CAIROCONFIG=$(pkg-config --cflags cairo)
    CAIROCONFIG_RETURNCODE=$?
    set -e
    if [ $CAIROCONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <cairo.h>" | ${CC} ${CC_OPT} ${CAIROCONFIG} -x c -
        CAIROCONFIG_RETURNCODE=$?
        set -e
        if [ $CAIROCONFIG_RETURNCODE -ne 0 ]; then
            echo "cairo not completely present or not working, skipping syntax check with ${CC}."
        else
            echo "cairo found and working, checking syntax with ${CC} now."
            ${CC} ${CC_OPT} ${CAIROCONFIG} ${DIR}cairo.c
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --library=cairo ${DIR}cairo.c

${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=googletest ${DIR}googletest.cpp

# kde.cpp
set +e
KDECONFIG=$(kde4-config --path include)
KDECONFIG_RETURNCODE=$?
set -e
if [ $KDECONFIG_RETURNCODE -ne 0 ]; then
    echo "kde4-config does not work, skipping syntax check."
else
    set +e
    KDEQTCONFIG=$(pkg-config --cflags QtCore)
    KDEQTCONFIG_RETURNCODE=$?
    set -e
    if [ $KDEQTCONFIG_RETURNCODE -ne 0 ]; then
        echo "Suitable Qt not present, Qt is necessary for KDE. Skipping syntax check."
    else
        set +e
        echo -e "#include <KDE/KGlobal>\n" | ${CXX} ${CXX_OPT} -I${KDECONFIG} ${KDEQTCONFIG} -x c++ -
        KDECHECK_RETURNCODE=$?
        set -e
        if [ $KDECHECK_RETURNCODE -ne 0 ]; then
            echo "KDE headers not completely present or not working, skipping syntax check with ${CXX}."
        else
            echo "KDE found, checking syntax with ${CXX} now."
            ${CXX} ${CXX_OPT} -I${KDECONFIG} ${KDEQTCONFIG} ${DIR}kde.cpp
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=kde ${DIR}kde.cpp

# libsigc++.cpp
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve libsigc++ configuration is not available, skipping syntax check."
else
    set +e
    LIBSIGCPPCONFIG=$(pkg-config --cflags sigc++-2.0)
    LIBSIGCPPCONFIG_RETURNCODE=$?
    set -e
    if [ $LIBSIGCPPCONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <sigc++/sigc++.h>\n" | ${CXX} ${CXX_OPT} ${LIBSIGCPPCONFIG} -x c++ -
        LIBSIGCPPCONFIG_RETURNCODE=$?
        set -e
        if [ $LIBSIGCPPCONFIG_RETURNCODE -ne 0 ]; then
            echo "libsigc++ not completely present or not working, skipping syntax check with ${CXX}."
        else
            echo "libsigc++ found and working, checking syntax with ${CXX} now."
            ${CXX} ${CXX_OPT} ${LIBSIGCPPCONFIG} ${DIR}libsigc++.cpp
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --library=libsigc++ ${DIR}libsigc++.cpp

# openssl.c
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve OpenSSL configuration is not available, skipping syntax check."
else
    set +e
    OPENSSLCONFIG=$(pkg-config --cflags libssl)
    OPENSSLCONFIG_RETURNCODE=$?
    set -e
    if [ $OPENSSLCONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <openssl/ssl.h>" | ${CC} ${CC_OPT} ${OPENSSLCONFIG} -x c -
        OPENSSLCONFIG_RETURNCODE=$?
        set -e
        if [ $OPENSSLCONFIG_RETURNCODE -ne 0 ]; then
            echo "OpenSSL not completely present or not working, skipping syntax check with ${CC}."
        else
            echo "OpenSSL found and working, checking syntax with ${CC} now."
            ${CC} ${CC_OPT} ${OPENSSLCONFIG} ${DIR}openssl.c
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --library=openssl ${DIR}openssl.c

# opencv2.cpp
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e
if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve OpenCV configuration is not available, skipping syntax check."
else
    set +e
    OPENCVCONFIG=$(pkg-config --cflags opencv)
    OPENCVCONFIG_RETURNCODE=$?
    set -e
    if [ $OPENCVCONFIG_RETURNCODE -eq 0 ]; then
        set +e
        echo -e "#include <opencv2/opencv.hpp>\n" | ${CXX} ${CXX_OPT} ${OPENCVCONFIG} -x c++ -
        OPENCVCONFIG_RETURNCODE=$?
        set -e
        if [ $OPENCVCONFIG_RETURNCODE -ne 0 ]; then
            echo "OpenCV not completely present or not working, skipping syntax check with ${CXX}."
        else
            echo "OpenCV found and working, checking syntax with ${CXX} now."
            ${CXX} ${CXX_OPT} ${OPENCVCONFIG} ${DIR}opencv2.cpp
        fi
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --library=opencv2 ${DIR}opencv2.cpp

# cppunit.cpp
set +e
pkg-config --version
PKGCONFIG_RETURNCODE=$?
set -e

if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
    echo "pkg-config needed to retrieve cppunit configuration is not available, skipping syntax check."
else
    set +e
    CPPUNIT=$(pkg-config cppunit)
    CPPUNIT_RETURNCODE=$?
    set -e
    if [ $CPPUNIT_RETURNCODE -ne 0 ]; then
        echo "cppunit not found, skipping syntax check for cppunit"
    else
        echo "cppunit found, checking syntax with ${CXX} now."
        ${CXX} ${CXX_OPT} -Wno-deprecated-declarations ${DIR}cppunit.cpp
    fi
fi
${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=cppunit -f ${DIR}cppunit.cpp

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

echo SUCCESS

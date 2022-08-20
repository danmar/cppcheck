#!/bin/bash
set -e # abort on error
#set -x # be verbose

function exit_if_strict {
    if [ -n "${STRICT}" ] && [ "${STRICT}" -eq 1 ]; then
      exit 1
    fi
}

echo "Checking for pkg-config..."
if pkg-config --version; then
  HAS_PKG_CONFIG=1
  echo "pkg-config found."
else
  HAS_PKG_CONFIG=0
  echo "pkg-config is not available, skipping all syntax checks."
  exit_if_strict
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/
CPPCHECK="$DIR"../../cppcheck
CFG="$DIR"../../cfg/

# Cppcheck options
CPPCHECK_OPT='--check-library --platform=unix64 --enable=information --enable=style --error-exitcode=-1 --suppress=missingIncludeSystem --inline-suppr --template="{file}:{line}:{severity}:{id}:{message}"'

# Compiler settings
CXX=g++
CXX_OPT='-fsyntax-only -std=c++0x -Wno-format -Wno-format-security -Wno-deprecated-declarations'
CC=gcc
CC_OPT='-Wno-format -Wno-stringop-overread -Wno-nonnull -Wno-implicit-function-declaration -Wno-deprecated-declarations -Wno-format-security -Wno-nonnull -fsyntax-only'

function get_pkg_config_cflags {
    set +e
    PKGCONFIG=$(pkg-config --cflags $1)
    PKGCONFIG_RETURNCODE=$?
    set -e
    if [ $PKGCONFIG_RETURNCODE -ne 0 ]; then
        PKGCONFIG=
    else
        # make sure the config is not empty when no flags were found - happens with e.g. libssl and sqlite3
        if [ -z "$PKGCONFIG" ]; then
            PKGCONFIG=" "
        fi
    fi
    echo "$PKGCONFIG"
}

# posix.c
function posix_fn {
    ${CC} ${CC_OPT} ${DIR}posix.c
}

# gnu.c
function gnu_fn {
    ${CC} ${CC_OPT} -D_GNU_SOURCE ${DIR}gnu.c
}

# qt.cpp
function qt_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        QTCONFIG=$(get_pkg_config_cflags Qt5Core)
        if [ -n "$QTCONFIG" ]; then
            QTBUILDCONFIG=$(pkg-config --variable=qt_config Qt5Core)
            [[ $QTBUILDCONFIG =~ (^|[[:space:]])reduce_relocations($|[[:space:]]) ]] && QTCONFIG="${QTCONFIG} -fPIC"
            set +e
            echo -e "#include <QString>" | ${CXX} ${CXX_OPT} ${QTCONFIG} -x c++ -
            QTCHECK_RETURNCODE=$?
            set -e
            if [ $QTCHECK_RETURNCODE -ne 0 ]; then
                echo "Qt not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "Qt found and working, checking syntax with ${CXX} now."
                ${CXX} ${CXX_OPT} ${QTCONFIG} ${DIR}qt.cpp
            fi
        else
            echo "Qt not present, skipping syntax check with ${CXX}."
            exit_if_strict
        fi
    fi
}

# bsd.c
function bsd_fn {
  true
}

# std.c
function std_c_fn {
    ${CC} ${CC_OPT} ${DIR}std.c
}

# std.cpp
function std_cpp_fn {
    ${CXX} ${CXX_OPT} ${DIR}std.cpp
}

# windows.cpp
function windows_fn {
    # TODO: Syntax check via g++ does not work because it can not find a valid windows.h
    #${CXX} ${CXX_OPT} ${DIR}windows.cpp
    true
}

# wxwidgets.cpp
function wxwidgets_fn {
    set +e
    WXCONFIG=$(wx-config --cxxflags)
    WXCONFIG_RETURNCODE=$?
    set -e
    if [ $WXCONFIG_RETURNCODE -ne 0 ]; then
        echo "wx-config does not work, skipping syntax check for wxWidgets tests."
        exit_if_strict
    else
        set +e
        echo -e "#include <wx/filefn.h>\n#include <wx/app.h>\n#include <wx/artprov.h>\n#include <wx/version.h>\n#if wxVERSION_NUMBER<2950\n#error \"Old version\"\n#endif" | ${CXX} ${CXX_OPT} ${WXCONFIG} -x c++ -
        WXCHECK_RETURNCODE=$?
        set -e
        if [ $WXCHECK_RETURNCODE -ne 0 ]; then
            echo "wxWidgets not completely present (with GUI classes) or not working, skipping syntax check with ${CXX}."
            exit_if_strict
        else
            echo "wxWidgets found, checking syntax with ${CXX} now."
            ${CXX} ${CXX_OPT} ${WXCONFIG} -Wno-deprecated-declarations ${DIR}wxwidgets.cpp
        fi
    fi
}

# gtk.c
function gtk_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        GTKCONFIG=$(get_pkg_config_cflags gtk+-3.0)
        if [ -z "$GTKCONFIG" ]; then
            GTKCONFIG=$(get_pkg_config_cflags gtk+-2.0)
        fi
        if [ -n "$GTKCONFIG" ]; then
            set +e
            echo -e "#include <gtk/gtk.h>" | ${CC} ${CC_OPT} ${GTKCONFIG} -x c -
            GTKCHECK_RETURNCODE=$?
            set -e
            if [ $GTKCHECK_RETURNCODE -ne 0 ]; then
                echo "GTK+ not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "GTK+ found and working, checking syntax with ${CXX} now."
                ${CC} ${CC_OPT} ${GTKCONFIG} ${DIR}gtk.c
            fi
        else
            echo "GTK+ not present, skipping syntax check with ${CXX}."
            exit_if_strict
        fi
    fi
}

# boost.cpp
function boost_fn {
    set +e
    echo -e "#include <boost/config.hpp>" | ${CXX} ${CXX_OPT} -x c++ -
    BOOSTCHECK_RETURNCODE=$?
    set -e
    if [ ${BOOSTCHECK_RETURNCODE} -ne 0 ]; then
        echo "Boost not completely present or not working, skipping syntax check with ${CXX}."
        exit_if_strict
    else
        echo "Boost found and working, checking syntax with ${CXX} now."
        ${CXX} ${CXX_OPT} ${DIR}boost.cpp
    fi
}

# sqlite3.c
function sqlite3_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        SQLITE3CONFIG=$(get_pkg_config_cflags sqlite3)
        if [ -n "$SQLITE3CONFIG" ]; then
            set +e
            echo -e "#include <sqlite3.h>" | ${CC} ${CC_OPT} ${SQLITE3CONFIG} -x c -
            SQLITE3CHECK_RETURNCODE=$?
            set -e
            if [ $SQLITE3CHECK_RETURNCODE -ne 0 ]; then
                echo "SQLite3 not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "SQLite3 found and working, checking syntax with ${CC} now."
                ${CC} ${CC_OPT} ${SQLITE3CONFIG} ${DIR}sqlite3.c
            fi
        else
            echo "SQLite3 not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# openmp.c
function openmp_fn {
    ${CC} ${CC_OPT} -fopenmp ${DIR}openmp.c
}

# python.c
function python_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        PYTHON3CONFIG=$(get_pkg_config_cflags python3)
        if [ -n "$PYTHON3CONFIG" ]; then
            set +e
            echo -e "#include <Python.h>" | ${CC} ${CC_OPT} ${PYTHON3CONFIG} -x c -
            PYTHON3CONFIG_RETURNCODE=$?
            set -e
            if [ $PYTHON3CONFIG_RETURNCODE -ne 0 ]; then
                echo "Python 3 not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "Python 3 found and working, checking syntax with ${CC} now."
                ${CC} ${CC_OPT} ${PYTHON3CONFIG} ${DIR}python.c
            fi
        else
            echo "Python 3 not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# lua.c
function lua_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        LUACONFIG=$(get_pkg_config_cflags lua-5.3)
        if [ -n "$LUACONFIG" ]; then
            set +e
            echo -e "#include <lua.h>" | ${CC} ${CC_OPT} ${LUACONFIG} -x c -
            LUACONFIG_RETURNCODE=$?
            set -e
            if [ $LUACONFIG_RETURNCODE -ne 0 ]; then
                echo "Lua not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "Lua found and working, checking syntax with ${CC} now."
                ${CC} ${CC_OPT} ${LUACONFIG} ${DIR}lua.c
            fi
        else
            echo "Lua not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# libcurl.c
function libcurl_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        LIBCURLCONFIG=$(get_pkg_config_cflags libcurl)
        if [ -n "$LIBCURLCONFIG" ]; then
            set +e
            echo -e "#include <curl/curl.h>" | ${CC} ${CC_OPT} ${LIBCURLCONFIG} -x c -
            LIBCURLCONFIG_RETURNCODE=$?
            set -e
            if [ $LIBCURLCONFIG_RETURNCODE -ne 0 ]; then
                echo "libcurl not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "libcurl found and working, checking syntax with ${CC} now."
                ${CC} ${CC_OPT} ${LIBCURLCONFIG} ${DIR}libcurl.c
            fi
        else
            echo "libcurl not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# cairo.c
function cairo_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        CAIROCONFIG=$(get_pkg_config_cflags cairo)
        if [ -n "$CAIROCONFIG" ]; then
            set +e
            echo -e "#include <cairo.h>" | ${CC} ${CC_OPT} ${CAIROCONFIG} -x c -
            CAIROCONFIG_RETURNCODE=$?
            set -e
            if [ $CAIROCONFIG_RETURNCODE -ne 0 ]; then
                echo "cairo not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "cairo found and working, checking syntax with ${CC} now."
                ${CC} ${CC_OPT} ${CAIROCONFIG} ${DIR}cairo.c
            fi
        else
            echo "cairo not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# googletest.cpp
function googletest_fn {
    true
}

# kde.cpp
function kde_fn {
    set +e
    KDECONFIG=$(kde4-config --path include)
    KDECONFIG_RETURNCODE=$?
    set -e
    if [ $KDECONFIG_RETURNCODE -ne 0 ]; then
        echo "kde4-config does not work, skipping syntax check."
        exit_if_strict
    else
        KDEQTCONFIG=$(get_pkg_config_cflags QtCore)
        if [ -n "$KDEQTCONFIG" ]; then
            echo "Suitable Qt not present, Qt is necessary for KDE. Skipping syntax check."
            exit_if_strict
        else
            set +e
            echo -e "#include <KDE/KGlobal>\n" | ${CXX} ${CXX_OPT} -I${KDECONFIG} ${KDEQTCONFIG} -x c++ -
            KDECHECK_RETURNCODE=$?
            set -e
            if [ $KDECHECK_RETURNCODE -ne 0 ]; then
                echo "KDE headers not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "KDE found, checking syntax with ${CXX} now."
                ${CXX} ${CXX_OPT} -I${KDECONFIG} ${KDEQTCONFIG} ${DIR}kde.cpp
            fi
        fi
    fi
}

# libsigc++.cpp
function libsigcpp_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        LIBSIGCPPCONFIG=$(get_pkg_config_cflags sigc++-2.0)
        if [ -n "$LIBSIGCPPCONFIG" ]; then
            set +e
            echo -e "#include <sigc++/sigc++.h>\n" | ${CXX} ${CXX_OPT} ${LIBSIGCPPCONFIG} -x c++ -
            LIBSIGCPPCONFIG_RETURNCODE=$?
            set -e
            if [ $LIBSIGCPPCONFIG_RETURNCODE -ne 0 ]; then
                echo "libsigc++ not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "libsigc++ found and working, checking syntax with ${CXX} now."
                ${CXX} ${CXX_OPT} ${LIBSIGCPPCONFIG} ${DIR}libsigc++.cpp
            fi
        else
            echo "libsigc++ not present, skipping syntax check with ${CXX}."
            exit_if_strict
        fi
    fi
}

# openssl.c
function openssl_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        OPENSSLCONFIG=$(get_pkg_config_cflags libssl)
        if [ -n "$OPENSSLCONFIG" ]; then
            set +e
            echo -e "#include <openssl/ssl.h>" | ${CC} ${CC_OPT} ${OPENSSLCONFIG} -x c -
            OPENSSLCONFIG_RETURNCODE=$?
            set -e
            if [ $OPENSSLCONFIG_RETURNCODE -ne 0 ]; then
                echo "OpenSSL not completely present or not working, skipping syntax check with ${CC}."
                exit_if_strict
            else
                echo "OpenSSL found and working, checking syntax with ${CC} now."
                ${CC} ${CC_OPT} ${OPENSSLCONFIG} ${DIR}openssl.c
            fi
        else
            echo "OpenSSL not present, skipping syntax check with ${CC}."
            exit_if_strict
        fi
    fi
}

# opencv2.cpp
function opencv2_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        OPENCVCONFIG=$(get_pkg_config_cflags opencv)
        if [ -n "$OPENCVCONFIG" ]; then
            set +e
            echo -e "#include <opencv2/opencv.hpp>\n" | ${CXX} ${CXX_OPT} ${OPENCVCONFIG} -x c++ -
            OPENCVCONFIG_RETURNCODE=$?
            set -e
            if [ $OPENCVCONFIG_RETURNCODE -ne 0 ]; then
                echo "OpenCV not completely present or not working, skipping syntax check with ${CXX}."
                exit_if_strict
            else
                echo "OpenCV found and working, checking syntax with ${CXX} now."
                ${CXX} ${CXX_OPT} ${OPENCVCONFIG} ${DIR}opencv2.cpp
            fi
        else
            echo "OpenCV not present, skipping syntax check with ${CXX}."
            exit_if_strict
        fi
    fi
}

# cppunit.cpp
function cppunit_fn {
    if [ $HAS_PKG_CONFIG -eq 1 ]; then
        if ! pkg-config cppunit; then
            echo "cppunit not found, skipping syntax check for cppunit"
            exit_if_strict
        else
            echo "cppunit found, checking syntax with ${CXX} now."
            ${CXX} ${CXX_OPT} -Wno-deprecated-declarations ${DIR}cppunit.cpp
        fi
    fi
}

for f in "${DIR}"*.{c,cpp}
do
    f=$(basename $f)
    case $f in
        boost.cpp)
            boost_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=boost ${DIR}boost.cpp
            ;;
        bsd.c)
            bsd_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=bsd ${DIR}bsd.c
            ;;
        cairo.c)
            cairo_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=cairo ${DIR}cairo.c
            ;;
        cppunit.cpp)
            cppunit_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=cppunit -f ${DIR}cppunit.cpp
            ;;
        gnu.c)
            gnu_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=posix,gnu ${DIR}gnu.c
            ;;
        googletest.cpp)
            googletest_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=googletest ${DIR}googletest.cpp
            ;;
        gtk.c)
            gtk_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=gtk -f ${DIR}gtk.c
            ;;
        kde.cpp)
            # TODO: "kde-4config" is no longer commonly available in recent distros
            #kde_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=kde ${DIR}kde.cpp
            ;;
        libcurl.c)
            libcurl_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=libcurl ${DIR}libcurl.c
            ;;
        libsigc++.cpp)
            libsigcpp_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=libsigc++ ${DIR}libsigc++.cpp
            ;;
        lua.c)
            lua_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=lua ${DIR}lua.c
            ;;
        opencv2.cpp)
            # TODO: "opencv.pc" is not commonly available in distros
            #opencv2_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=opencv2 ${DIR}opencv2.cpp
            ;;
        openmp.c)
            openmp_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=openmp ${DIR}openmp.c
            ;;
        openssl.c)
            openssl_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=openssl ${DIR}openssl.c
            ;;
        posix.c)
            posix_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=posix ${DIR}posix.c
            ;;
        python.c)
            python_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --library=python ${DIR}python.c
            ;;
        qt.cpp)
            qt_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=qt ${DIR}qt.cpp
            ;;
        sqlite3.c)
            sqlite3_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=sqlite3 ${DIR}sqlite3.c
            ;;
        std.c)
            std_c_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive ${DIR}std.c
            ;;
        std.cpp)
            std_cpp_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive ${DIR}std.cpp
            ;;
        windows.cpp)
            windows_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --platform=win32A --library=windows ${DIR}windows.cpp
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --platform=win32W --library=windows ${DIR}windows.cpp
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --platform=win64  --library=windows ${DIR}windows.cpp
            ;;
        wxwidgets.cpp)
            wxwidgets_fn
            ${CPPCHECK} ${CPPCHECK_OPT} --inconclusive --library=wxwidgets,windows -f ${DIR}wxwidgets.cpp
            ;;
        *)
          echo "Unhandled file $f"
          exit_if_strict
    esac
done

# Check the syntax of the defines in the configuration files
if ! xmlstarlet --version; then
    echo "xmlstarlet needed to extract defines, skipping defines check."
    exit_if_strict
else
    for configfile in ${CFG}*.cfg; do
        echo "Checking defines in $configfile"
        # Disable debugging output temporarily since there could be many defines
        set +x
        # XMLStarlet returns 1 if no elements were found which is no problem here
        EXTRACTED_DEFINES=$(xmlstarlet sel -t -m '//define' -c . -n <$configfile || true)
        EXTRACTED_DEFINES=$(echo "$EXTRACTED_DEFINES" | sed 's/<define name="/#define /g' | sed 's/" value="/ /g' | sed 's/"\/>//g')
        echo "$EXTRACTED_DEFINES" | gcc -fsyntax-only -xc -Werror -
    done
fi

echo SUCCESS

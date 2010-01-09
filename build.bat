@echo off
REM A simple script to build different cppcheck targets from project root
REM folder. This script can be run from VS prompt or Qt prompt.
REM
REM Usage: build <target> [release|debug]
REM  where <target> is any of cppcheck/gui/tests/all
REM        release or debug is the configuration
REM  all-target builds both cppcheck and gui.
REM
REM TODO:
REM  - run tests too

if "%1" == "" goto help

REM QT prompt sets QMAKESPEC
if NOT "%QMAKESPEC%" == "" (
  set MAKE=mingw32-make
) else (
  set MAKE=nmake
)

if "%2" == "" set TARGET=release
if "%2" == "release" set TARGET=release
if "%2" == "debug" set TARGET=debug

if "%1" == "cppcheck" goto cppcheck
if "%1" == "gui" goto gui
if "%1" == "tests" goto tests
if "%1" == "all" goto cppcheck
goto help

:cppcheck
cd cli
qmake -config %TARGET%
%MAKE%
cd ..
if "%1" == "all" goto gui
goto end

:gui
cd gui
qmake -config %TARGET%
%MAKE%
lrelease gui.pro
cd ..
goto end

:tests
cd test
qmake -config %TARGET%
%MAKE%
cd ..
goto end

:help
echo "Syntax: build <target> [debug|release]"
echo "  where <target> is any of cppcheck/gui/tests/all"
echo "        debug or release define used configuration"
echo "  all- target builds both cppcheck and gui.

:end

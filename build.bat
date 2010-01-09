@echo off
REM A simple script to build different cppcheck targets from project root
REM folder.
REM
REM Usage: build <target> [release|debug]
REM  where <target> is any of cppcheck/gui/tests
REM        release or debug is the configuration
REM
REM TODO:
REM  - add "all" target
REM  - run tests too

if "%1" == "" goto help

REM QT prompt sets QMAKESPEC
if NOT "%QMAKESPEC%" == "" (
  set MAKE=mingw32-make
) else (
  set MAKE=nmake
)

if "%1" == "cppcheck" goto cppcheck
if "%1" == "gui" goto gui
if "%1" == "tests" goto tests
goto help

:cppcheck
cd cli
qmake -config %2
%MAKE%
cd ..
goto end

:gui
cd gui
qmake -config %2
%MAKE%
lrelease gui.pro
cd ..
goto end

:tests
cd test
qmake -config %2
%MAKE%
cd ..
goto end

:help
echo "Syntax: build <target> [debug|release]"
echo "  where <target> is any of cppcheck/gui/tests"
echo "        debug or release define used configuration"

:end

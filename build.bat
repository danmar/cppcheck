@echo off
REM A simple script to build different cppcheck targets from project root
REM folder.
REM
REM Usage: build <target>
REM  where <target> is any of cppcheck/gui/tests
REM
REM TODO:
REM  - build release targets (new switch?)
REM  - add "all" target
REM  - run tests too

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
qmake
%MAKE%
cd ..
goto end

:gui
cd gui
qmake
%MAKE%
lrelease gui.pro
cd ..
goto end

:tests
cd test
qmake
%MAKE%
cd ..
goto end

:help
echo Syntax: build <target>
echo  where <target> is any of cppcheck/gui/tests

:end

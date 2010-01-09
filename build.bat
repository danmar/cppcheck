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

if "%1" == "cppcheck" goto cppcheck
if "%1" == "gui" goto gui
if "%1" == "tests" goto tests
goto help

:cppcheck
cd cli
qmake
nmake
cd ..
goto end

:gui
cd gui
qmake
nmake
lrelease gui.pro
copy *.qm debug
cd ..
goto end

:tests
cd test
qmake
nmake
cd ..
goto end

:help
echo Syntax: build <target>
echo  where <target> is any of cppcheck/gui/tests

:end

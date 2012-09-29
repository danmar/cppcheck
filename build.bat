@echo off
REM A simple script to build different cppcheck targets from project root
REM folder. This script can be run from VS prompt or Qt prompt.
REM
REM Usage: build <target> [release|debug]
REM  where <target> is any of cppcheck/gui/tests/all
REM        release or debug is the configuration
REM  all-target builds both cppcheck and gui.
REM
REM Run the command before build.bat to enable rules using pcre:
REM   set HAVE_RULES=yes
REM
REM TODO:
REM  - run tests too

pushd %~dp0

if "%1" == "" goto help

REM Qt prompt sets QMAKESPEC
if "%QMAKESPEC%" == "" (
REM parse qmakespec to see if it's some msvc
  if "%QMAKESPEC:~6,4%" == "msvc" (
    set MAKE=nmake
  ) else (
    set MAKE=mingw32-make
  )
) else (
  set MAKE=nmake
)

if "%2" == ""        set TARGET=release
if "%2" == "debug"   set TARGET=debug
if "%2" == "release" set TARGET=release

if "%1" == "all"      goto cppcheck
if "%1" == "cppcheck" goto cppcheck
if "%1" == "gui"      goto gui
if "%1" == "tests"    goto tests
goto help

:cppcheck
pushd cli
qmake -config %TARGET% HAVE_RULES=%HAVE_RULES%
%MAKE%
popd
if "%1" == "all" goto gui
goto end

:gui
pushd gui
qmake -config %TARGET% HAVE_RULES=%HAVE_RULES%
%MAKE%
lupdate -no-obsolete gui.pro
lrelease gui.pro
popd
goto end

:tests
pushd test
qmake -config %TARGET% HAVE_RULES=%HAVE_RULES%
%MAKE%
popd
goto end

:help
echo Syntax: build ^<target^> [debug^|release]
echo   where ^<target^> is any of cppcheck/gui/tests/all
echo         debug or release define used configuration
echo   all- target builds both cppcheck and gui.

:end

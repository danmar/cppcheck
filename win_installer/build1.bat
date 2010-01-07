
set QTBASE=c:\Qt\2009.05
set QTDIR=%QTBASE%\qt
set PATH=%QTDIR%\bin
set PATH=%PATH%;%QTBASE%\bin;%QTBASE%\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

cd ..

del gui\Makefile
del /Q gui\temp\*
del gui\release\gui.exe
cd gui
qmake -config release
make


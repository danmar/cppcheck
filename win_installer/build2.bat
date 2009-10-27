
set QTBASE=c:\Qt\2009.01
set QTDIR=%QTBASE%\qt
set PATH=%QTDIR%\bin
set PATH=%PATH%;%QTBASE%\bin;%QTBASE%\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

cd ..

del cli\Makefile
del /Q cli\temp\*
del cli\release\gui.exe
cd cli
qmake -config release
make



set QTDIR=c:\Qt\2009.01\qt
set PATH=c:\Qt\2009.01\qt\bin
set PATH=%PATH%;c:\Qt\2009.01\bin;c:\Qt\2009.01\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

cd ..

del gui\Makefile
del /Q gui\temp\*
del gui\release\gui.exe
cd gui
qmake -config release
make


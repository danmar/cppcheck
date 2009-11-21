del *.wixobj
del *.wixpdb
del *.msi

del gui.exe
del cppcheck.exe
copy ..\gui\release\gui.exe .
copy ..\cli\release\cppcheck.exe .

candle cppcheck.wxs
light -ext WixUIExtension cppcheck.wixobj

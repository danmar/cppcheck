del *.wixobj
del *.wixpdb
del *.msi

del gui.exe
del cppcheck.exe
copy ..\gui\release\gui.exe .
copy ..\cli\release\cppcheck.exe .

candle gui.wxs
light -ext WixUIExtension gui.wixobj
move gui.msi cppcheck.msi

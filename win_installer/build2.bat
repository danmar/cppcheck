del *.wixobj
del *.wixpdb
del cppcheck.exe
del gui.exe

cd ..
del /Q release\*
del cppcheck.exe
del cppcheck.map
del cppcheck.tds

msbuild cppcheck.cbproj /target:clean
msbuild cppcheck.cbproj /target:build /property:"config=release"

copy cppcheck.exe win_installer
copy gui\release\gui.exe win_installer

cd win_installer
candle cppcheck.wxs
light -ext WixUIExtension cppcheck.wixobj
rename cppcheck.msi cppcheck-light.msi
candle gui.wxs
light -ext WixUIExtension gui.wixobj



cd..

msbuild cppcheck.cbproj /target:clean
msbuild cppcheck.cbproj /target:build /property:"config=release" > htdocs\bcb.txt

cppcheck --all --style --unused-functions src 2> htdocs\cppcheck-results.txt

REM A script to run Astyle for the sources

SET STYLE=--style=stroustrup --indent=spaces=4 --indent-namespaces --lineend=linux --min-conditional-indent=0
SET OPTIONS=--pad-header --pad-oper --unpad-paren --convert-tabs --suffix=none --formatted --preserve-date

astyle %STYLE% %OPTIONS% cli/*.cpp
astyle %STYLE% %OPTIONS% cli/*.h
astyle %STYLE% %OPTIONS% gui/*.cpp
astyle %STYLE% %OPTIONS% gui/*.h
astyle %STYLE% %OPTIONS% -r gui/test/*.cpp
astyle %STYLE% %OPTIONS% -r gui/test/*.h
astyle %STYLE% %OPTIONS% lib/*.cpp
astyle %STYLE% %OPTIONS% lib/*.h
astyle %STYLE% %OPTIONS% test/*.cpp
astyle %STYLE% %OPTIONS% test/*.h

astyle %STYLE% %OPTIONS% tools/*.cpp
astyle %STYLE% %OPTIONS% --recursive "samples/*.c"
astyle %STYLE% %OPTIONS% --recursive "samples/*.cpp"

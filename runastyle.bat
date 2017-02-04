@REM Script to run AStyle on the sources

@SET STYLE=--style=stroustrup --indent=spaces=4 --indent-namespaces --lineend=linux --min-conditional-indent=0
@SET OPTIONS=--pad-header --unpad-paren --suffix=none --convert-tabs --attach-inlines

astyle %STYLE% %OPTIONS% cli/*.cpp
astyle %STYLE% %OPTIONS% cli/*.h
astyle %STYLE% %OPTIONS% democlient/*.cpp
astyle %STYLE% %OPTIONS% gui/*.cpp
astyle %STYLE% %OPTIONS% gui/*.h
astyle %STYLE% %OPTIONS% -r gui/test/*.cpp
astyle %STYLE% %OPTIONS% -r gui/test/*.h
astyle %STYLE% %OPTIONS% lib/*.cpp
astyle %STYLE% %OPTIONS% lib/*.h
astyle %STYLE% %OPTIONS% test/*.cpp
astyle %STYLE% %OPTIONS% test/cfg/*.cpp
astyle %STYLE% %OPTIONS% test/*.h
astyle %STYLE% %OPTIONS% tools/*.cpp
astyle %STYLE% %OPTIONS% -r samples/*.c
astyle %STYLE% %OPTIONS% -r samples/*.cpp

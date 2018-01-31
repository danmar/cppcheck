@REM Script to run AStyle on the sources
@REM The version check in this script is used to avoid commit battles
@REM between different developers that use different astyle versions as
@REM different versions might have different output (this has happened in
@REM the past).

@REM If project management wishes to take a newer astyle version into use
@REM just change this string to match the start of astyle version string.
set ASTYLE_VERSION="Artistic Style Version 3.0.1"
set ASTYLE="astyle"

set DETECTED_VERSION=""
for /f "tokens=*" %%i in ('%ASTYLE% --version') do set DETECTED_VERSION=%%i
echo %DETECTED_VERSION% | findstr /b /c:%ASTYLE_VERSION% > nul && (
    echo "%DETECTED_VERSION%" matches %ASTYLE_VERSION%
) || (
    echo You should use: %ASTYLE_VERSION%
    echo Detected: "%DETECTED_VERSION%"
    goto EXIT_ERROR
)

@SET STYLE=--style=kr --indent=spaces=4 --indent-namespaces --lineend=linux --min-conditional-indent=0
@SET OPTIONS=--pad-header --unpad-paren --suffix=none --convert-tabs --attach-inlines --attach-classes --attach-namespaces

%ASTYLE% %STYLE% %OPTIONS% cli/*.cpp
%ASTYLE% %STYLE% %OPTIONS% cli/*.h
%ASTYLE% %STYLE% %OPTIONS% democlient/*.cpp
%ASTYLE% %STYLE% %OPTIONS% gui/*.cpp
%ASTYLE% %STYLE% %OPTIONS% gui/*.h
%ASTYLE% %STYLE% %OPTIONS% -r gui/test/*.cpp
%ASTYLE% %STYLE% %OPTIONS% -r gui/test/*.h
%ASTYLE% %STYLE% %OPTIONS% lib/*.cpp
%ASTYLE% %STYLE% %OPTIONS% lib/*.h
%ASTYLE% %STYLE% %OPTIONS% test/*.cpp
%ASTYLE% %STYLE% %OPTIONS% test/cfg/*.c*
%ASTYLE% %STYLE% %OPTIONS% test/*.h

%ASTYLE% %STYLE% %OPTIONS% -r tools/*.cpp
%ASTYLE% %STYLE% %OPTIONS% -r tools/*.h

%ASTYLE% %STYLE% %OPTIONS% -r samples/*.c
%ASTYLE% %STYLE% %OPTIONS% -r samples/*.cpp

goto :EOF

:EXIT_ERROR
exit /b 1

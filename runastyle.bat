@REM Script to run AStyle on the sources
@REM The version check in this script is used to avoid commit battles
@REM between different developers that use different astyle versions as
@REM different versions might have different output (this has happened in
@REM the past).

@REM If project management wishes to take a newer astyle version into use
@REM just change this string to match the start of astyle version string.
@SET ASTYLE_VERSION="Artistic Style Version 3.0.1"
@SET ASTYLE="astyle"

@SET DETECTED_VERSION=""
@FOR /F "tokens=*" %%i IN ('%ASTYLE% --version') DO SET DETECTED_VERSION=%%i
@ECHO %DETECTED_VERSION% | FINDSTR /B /C:%ASTYLE_VERSION% > nul && (
    ECHO "%DETECTED_VERSION%" matches %ASTYLE_VERSION%
) || (
    ECHO You should use: %ASTYLE_VERSION%
    ECHO Detected: "%DETECTED_VERSION%"
    GOTO EXIT_ERROR
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

@REM Format configuration files
@SET XMLLINT=xmllint
WHERE %XMLLINT%
@IF %ERRORLEVEL% NEQ 0 (
    ECHO WARNING: %XMLLINT% was not found. Skipping configuration file formatting!
) ELSE (
    PUSHD "cfg"
    FOR /F "tokens=* delims=" %%f IN ('DIR /B *.cfg') DO @CALL :runxmllint "%%f"
    POPD
)

@GOTO :EOF

:EXIT_ERROR
EXIT /B 1

@REM Function that formats one XML file
@REM Argument: %1: XML-File to format
:runxmllint
    xmllint --format -o "%~1_" "%~1"
    MOVE /Y "%~1_" "%~1"
@GOTO :EOF

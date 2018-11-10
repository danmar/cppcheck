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

@SET RCFILE=.astylerc

%ASTYLE% --options=%RCFILE% cli/*.cpp
%ASTYLE% --options=%RCFILE% cli/*.h
%ASTYLE% --options=%RCFILE% democlient/*.cpp
%ASTYLE% --options=%RCFILE% gui/*.cpp
%ASTYLE% --options=%RCFILE% gui/*.h
%ASTYLE% --options=%RCFILE% -r gui/test/*.cpp
%ASTYLE% --options=%RCFILE% -r gui/test/*.h
%ASTYLE% --options=%RCFILE% lib/*.cpp
%ASTYLE% --options=%RCFILE% lib/*.h
%ASTYLE% --options=%RCFILE% test/*.cpp
%ASTYLE% --options=%RCFILE% test/cfg/*.c
%ASTYLE% --options=%RCFILE% test/cfg/*.cpp
%ASTYLE% --options=%RCFILE% test/*.h

%ASTYLE% --options=%RCFILE% -r tools/*.cpp
%ASTYLE% --options=%RCFILE% -r tools/*.h

%ASTYLE% --options=%RCFILE% -r samples/*.c
%ASTYLE% --options=%RCFILE% -r samples/*.cpp

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

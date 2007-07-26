
REM Sample DOS shellscript
REM cppcheck all *.cpp files in the subdirectory 'proj'

@ECHO OFF

date /t > report.txt
time /t >> report.txt

FOR %%s IN (proj\*.cpp) DO (
    cppcheck proj\%%s
)


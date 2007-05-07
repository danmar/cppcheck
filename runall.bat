
@ECHO OFF

REM Test the 'checkcode' program

date /t > report.txt
time /t >> report.txt

FOR /D %%s IN (test*) DO (
    checkcode %%s\%%s.cpp 2> %%s\out.msg
    hydfc %%s\err.msg %%s\out.msg >> report.txt
)

type report.txt


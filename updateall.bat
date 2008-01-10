
@ECHO OFF

FOR /D %%s IN (test*) DO (
    cppcheck %%s\%%s.cpp 2> %%s\err.msg
)



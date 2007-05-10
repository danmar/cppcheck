
@ECHO OFF

date /t > scintilla.txt
time /t >> scintilla.txt

FOR %%s IN (npp41\scintilla\src\*.cxx) DO (
    ECHO %%s
    ECHO ---------------------------------- >> scintilla.txt
    ECHO %%s >> scintilla.txt
    cppcheck %%s 2>> scintilla.txt
)


date /t > npp41.txt
time /t >> npp41.txt

ECHO Notepad++ 4.1 >> npp41.txt
ECHO ====================================== >> npp41.txt

FOR %%s IN (npp41\PowerEditor\src\*.cpp) DO (
    ECHO %%s
    ECHO ---------------------------------- >> npp41.txt
    ECHO %%s >> npp41.txt
    cppcheck %%s 2>> npp41.txt
)



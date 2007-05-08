
@ECHO OFF

date /t > scintilla.txt
time /t >> scintilla.txt

FOR %%s IN (npp41\scintilla\src\*.cxx) DO (
    ECHO %%s
    ECHO ---------------------------------- >> scintilla.txt
    ECHO %%s >> scintilla.txt
    cppcheck %%s 2>> scintilla.txt
)


date /t > powereditor.txt
time /t >> powereditor.txt

FOR %%s IN (npp41\PowerEditor\src\*.cpp) DO (
    ECHO %%s
    ECHO ---------------------------------- >> powereditor.txt
    ECHO %%s >> powereditor.txt
    cppcheck %%s 2>> powereditor.txt
)



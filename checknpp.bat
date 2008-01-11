
@ECHO OFF

date /t > scintilla.txt
time /t >> scintilla.txt

FOR %%s IN (npp472\scintilla\src\*.cxx) DO (
    ECHO %%s
    ECHO ---------------------------------- >> scintilla.txt
    ECHO %%s >> scintilla.txt
    cppcheck %%s 2>> scintilla.txt
)


date /t > npp472.txt
time /t >> npp472.txt

ECHO Notepad++ 4.7.2 >> npp472.txt
ECHO ====================================== >> npp472.txt

FOR %%s IN (npp472\PowerEditor\src\*.cpp) DO (
    ECHO %%s
    ECHO ---------------------------------- >> npp472.txt
    ECHO %%s >> npp472.txt
    cppcheck %%s 2>> npp472.txt
)




=========
Cppcheck
=========


About

    The original name of this program is "C++check" but it was later changed to "cppcheck".

Manual

    A manual is available online:
    http://cppcheck.sf.net/manual.pdf

Compiling

    Any C++ compiler should work.

    To build the GUI, you need Qt.

    When building the command line tool, PCRE is normally used.
    PCRE is optional.

    There are multiple compilation choices:
      * qmake - cross platform build tool
      * Windows: Visual Studio
      * Windows: Qt Creator + mingw
      * gnu make
      * g++

    qmake
    =====
        You can use the gui/gui.pro file to build the GUI.
            cd gui
            qmake
            make

    Visual Studio
    =============
        Use the cppcheck.sln file. The rules are normally enabled.

        To compile with rules (pcre dependency):
            * the pcre dll is needed. it can be downloaded from:
                http://cppcheck.sf.net/pcre-8.10-vs.zip

        To compile without rules (no dependencies):
            * remove the preprocessor define HAVE_RULES from the project
            * remove the pcre.lib from the project

    Qt Creator + mingw
    ==================
        The PCRE dll is needed to build the CLI. It can be downloaded here:
            http://software-download.name/pcre-library-windows/

    gnu make
    ========
        To build Cppcheck with rules (pcre dependency):
            make HAVE_RULES=yes

        To build Cppcheck without rules (no dependencies):
            make

    g++ (for experts)
    =================
        If you just want to build Cppcheck without dependencies then you can use this command:
            g++ -o cppcheck -Ilib cli/*.cpp lib/*.cpp

        If you want to use --rule and --rule-file then dependencies are needed:
            g++ -o cppcheck -lpcre -DHAVE_RULES -Ilib -Iexternals cli/*.cpp lib/*.cpp externals/tinyxml/*.cpp
    mingw
    =====
        make LDFLAGS=-lshlwapi

Cross compiling Win32 (CLI) version of Cppcheck in Linux

    sudo apt-get install mingw32
    make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi"
    mv cppcheck cppcheck.exe

Webpage

    http://cppcheck.sourceforge.net/

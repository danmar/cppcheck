=========
Cppcheck
=========


About

  The original name of this program is "C++check".
  The name was changed to "cppcheck".


Compiling

  Any C++ compiler should work.

  The Makefile works under Linux.
  To make it work under Windows with DJGPP, change "g++" to "gxx".

  To build cppcheck with qmake, run this commands:
    qmake -config release – this will generate Makefile (use 'debug' instead of
        'release' if doing developer build)
    make – this will build command-line tool, GUI and autotests
    make sub-src – this will build command-line tool only
    make check – this one will build and run autotests


Usage

  Run the cppcheck program without parameters and a help text will be shown.


Recommendations

  When the "--all" flag is given you may get a lot of error messages.


Webpage

  http://www.sf.net/projects/cppcheck




=========
C++ check
=========


About

  The original name of this program is "C++check".
  The name was changed to "cppcheck" (a google search is more successful now).


Compiling

  Any C++ compiler should work.

  The Makefile works under Linux.
  To make it work under Windows with DJGPP, change "g++" to "gxx".


Usage

  The syntax is:
      cppcheck [--all] [--errorsonly] [--style] [--recursive] [filename1] [filename2]

  The error messages will be printed to stderr.

  Example (Check all files. Use all checks):
      cppcheck --style --all --recursive

  To output error messages to a file use this syntax (works both on Windows and Linux):
      cppcheck file.cpp 2> err.txt


Recommendations

  When the "--all" flag is given you may get a lot of error messages.


Webpage

  http://www.sf.net/projects/cppcheck



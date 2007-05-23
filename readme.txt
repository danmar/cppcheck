=========
C++ check
=========


Compiling

  Any C++ compiler should work. 
  There are no dependencies.

  Linux:
      g++ -o cppcheck main.cpp

  Windows:
      gxx -o cppcheck main.cpp



Usage

  The syntax is:
      cppcheck [-w] filename.cpp

  The error messages will be printed to stderr.

  If you specify '-w', additional warning 
  messages will be printed.



Recommendations

  Create a shell script that checks all files.



Author

  Daniel Marjamäki
  A 29 year old from sweden who works in
  Stockholm as a programmer (developing 
  a RAD tool for control systems that
  control hydraulics).

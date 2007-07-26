

=========
C++ check
=========



Compiling

  Any C++ compiler should work. 
  There are no dependencies.

  The Makefile works under Linux.
  To make it work under Windows, change "g++" to "gxx".

  I haven't been able to test it on other platforms.



Usage

  The syntax is:
      cppcheck [--all] [--style] filename.cpp

  The error messages will be printed to stderr.




Recommendations

  Create a shell script that checks all files.
  See "checkproj.bat" for an example of how it can be done under Windows.

  When the "--all" flag is given you may get a lot of error messages.

  To dump the messages to a textfile you can use a command like this:
      cppcheck --all filename.cpp 2> messages.txt

  If you want to filter the messages you could use:
    * grep to filter out specific types of messages
    * diff to compare old messages with new messages. There are even GUIs for
      this.



Suggestions

  I'd like to get suggestions about new checks.



Author

  Daniel Marjamäki   (danielm77@spray.se)


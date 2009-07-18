cppcheck GUI
============
This is a GUI for cppcheck. It allows selecting folder or set of files to check
with cppcheck and shows list of found errors.

Running
-------
You need QT4 libraries installed in your system. Packages/files to install
depends on your operating system:
- Windows download QT4 from http://www.qtsoftware.com
- Linux install QT4 using your package manager, look for packages having QT4
  in their name, e.g. for Ubuntu install libqt4-core, libqt4-gui and libqt4-xml

Compiling
---------
Windows:
- The easy way is to download Qt SDK from http://www.qtsoftware.com and use
  Qt Creator and/or command line tools to build the GUI.
- The harder way is to download QT sources and build QT with Visual Studio
  (Express Edition works). Compiling QT alone may take over 4 hours!

Linux:
- Install QT development packages (make sure qmake -tool gets installed!). The
  names depend on distribution, but e.g. for Ubuntu the needed packages are:
  * libqt4-core
  * libqt4-gui
  * libqt4-xml
  * libqt4-dev
  * qt4-dev-tools
  * qt4-qmake

After you have needed libraries and tools installed, open command
prompt/console, go to gui directory and run command:
- qmake (in Linux and in Windows if build with MinGW/gcc or nmake)
- qmake -tp vc (to generate Visual Studio project file)

These commands generate makefiles to actually build the software. After that
the actual building is done in IDE or command line as usual. Note that you
don't need to run qmake again unless you add/remove files from the project.

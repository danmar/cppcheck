Cppcheck for 64-bit Windows
===========================

This is quick start to get you started with compiling Cppcheck for 64-bit
Windows with free VS Express editions.

Software needed:
- Visual Studio 2010 (or later) Express edition
- Only for VS2010: Windows SDK 7.1


LIB, CLI and testsuite
----------------------

Visual Studio 2010 and later:
Just open cppcheck.sln, choose "x64" as platform and compile.


GUI
---

Software needed:
- Visual Studio 2010 Express edition
- Windows SDK 7.1 (containing x64 compiler)
- latest Qt SDK (4.7.0 or later, earlier versions don't support VS 2010)

Turns out you just need to use Windows SDK's Command prompt and 64-bit
environment to configure and build Qt. No extra steps needed. But of course you
should build 64-bit Qt to different folder than 32-bit Qt.

Compiling 64-bit GUI works fine from Windows SDK Console. With VS2010 Express IDE
everything works fine after adding new x64 platform for the project.

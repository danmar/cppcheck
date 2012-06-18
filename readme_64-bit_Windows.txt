Cppcheck for 64-bit Windows
===========================

This is quick start to get you started with compiling Cppcheck for 64-bit
Windows with free VS Express editions. This is work in progress so these
instructions will be updated as we progress with the work...

Software needed:
- Visual Studio 2008 or 2010 Express edition
- Windows SDK 7.0 (for VS2008) or Windows SDK 7.1 (for VS2010)


Cppcheck.exe
------------

Visual Studio 2010:
Just open cppcheck_vs2010.sln, choose "x64" as platform and compile.

Visual Studio 2008:
Make sure you have the Windows SDK installed! VS Express doesn't have 64-bit
tools, libraries or headers so you cannot compile 64-bit binaries without
Windows SDK.

To compile 64-bit binaries you need to start VS Express to 64-bit environment.
(by default VS Express starts to 32-bit environment). To do this, open Windows
SDK Command Prompt and switch to 64-bit environment with command:
> setenv /x64 /debug

Then start VS Express:
> "C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\VCExpress" /useenv

VS Express starts otherwise normally but now all environment variables point to
64-bit folders for libraries.

Now you can open the cppcheck.sln solution file and compile 64-bit targets.
With VS 2008 there are configurations Debug-x64 and Release-x64 for 64-bit targets.
This is because VS 2008 express does not allow adding new platform.

You can use e.g. Dependency Walker -program (http://www.dependencywalker.com/)
to check that build binaries are really 64-bit binaries.


GUI
---

Software needed:
- Visual Studio 2008 or 2010 Express edition
- Windows SDK 7.0 (for VS2008) or Windows SDK 7.1 (for VS2010)
- latest Qt SDK (4.7.0 or later, earlier versions don't support VS 2010)

Turns out you just need to use Windows SDK's Command prompt and 64-bit
environment to configure and build Qt. No extra steps needed. But of course you
should build 64-bit Qt to different folder than 32-bit Qt.

Compiling 64-bit GUI works fine from Windows SDK Console. But seems there is no
easy way to make it work with VS 2008 Express IDE. With VS2010 Express IDE
everything works fine after adding new x64 platform for the project.

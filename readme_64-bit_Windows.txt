Cppcheck for 64-bit Windows
===========================

This is quick start to get you started with compiling Cppcheck for 64-bit
Windows. This is work in progress so these instructions will be updated as we
progress with the work...

Software needed:
- Visual Studio 2010 Express (Free download from MS) or VS 2010 Pro
- latest Windows SDK (currently v 7.1) if compiling with VS Express


Cppcheck.exe
------------

With VS Express:
Make sure you have the Windows SDK installed! VS Express doesn't install 64-bit
tools, libraries or headers so you cannot compile 64-bit binaries without
Windows SDK.

To compile 64-bit binaries you need to start VS Express to 64-bit environment.
(by default VS Express starts to 32-bit enviroment). To do this, open Windows
SDK Command Prompt and switch to 64-bit environment with command:
> setenv /x64 /debug

Then start VS Express:
> "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\VCExpress" /useenv
VC Express starts otherwise normally but now all environment variables point to
64-bit folders for libraries.

Now you can open the cppcheck_vs2010.sln solution file and compile 64-bit
targets. Remember that you cannot compile 32-bit targets from this VS intance!

With VS Pro (and other commercial editions) you can just open the
cppcheck_vs2010.sln solution file and compile 64-bit targets.

You can use e.g. Dependency Walker -program (http://www.dependencywalker.com/)
to check that build binaries are really 64-bit binaries.


GUI
---

Software needed:
- Visual Studio 2010 Express (Free download from MS) or VS 2010 Pro
- latest Windows SDK (currently v 7.1) if compiling with VS Express
- latest Qt SDK (4.7.0 or later, earlier versions don't support VS 2010)

TODO.

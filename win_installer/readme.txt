The Wix Installer for Windows
=============================

New cppcheck Windows installer is created with WiX:
http://wix.sourceforge.net/

You'll need:
 - latest Wix (3.0 or later)
 - MSBuild (coming with Visual Studio, also with VS 2008 express)
 - VS 2008 CRT merge module
 
Configuring
-----------

Installer configuration is done in file config.wxi. Depending how you build
cppcheck you may need to alter the paths for binaries.

Product version and other info
------------------------------

Version number and product name are set in file productInfo.wxi.

Building installer
------------------

Before building the installer make sure all the components are build:
 - CLI executable (cppcheck.exe)
 - GUI executable (cppcheck-gui.exe)
 - GUI translations (*.qm)
 - Manual (onlinehelp.qhc)

And that runtime files are available:
  - Qt runtimes:
      QtCLucene4.dll, QtCore4.dll, QtGui4.dll, QtHelp4.dll, QtNetwork4.dll,
      QtSql4.dll and QtXml4.dll
  - MS CRT merge module (Microsoft_VC90_CRT_x86.msm)

Build installer by giving this command line in VS command prompt (or run
vcvars32.bat in DOS prompt first to setup environment):

> msbuild cppcheck.wixproj /p:Platform=x86,ProductVersion=X.YY

For example:
> msbuild cppcheck.wixproj /p:Platform=x86,ProductVersion=1.40

Installer is created to Build -folder.

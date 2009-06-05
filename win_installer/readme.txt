Windows installer for the cppcheck
----------------------------------

Windows installer for both command line cppcheck and for QT-based GUI. All
needed runtimes and libraries are installed.

Command line cppccheck shortcuts are created to start cmd.exe in installation
folder. So when the user selects start menu/desktop icon he gets command prompt
in cppcheck folder.

Get the InnoSetup from:
http://www.innosetup.com/
Be sure to download the 'QuickStart Pack' as it installs some nice tools
like ISTool and preprocessor support.

Files the installer needs:
/COPYING
/readme.txt
/AUTHORS
/gui/release/gui.exe
/Release/cppcheck.exe
/win_installer/icon.bmp
/win_installer/LargeLogo.bmp
/win_installer/

NOTE: Remember to convert COPYING and AUTHORS to Windows EOL format! Otherwise
Windows Notepad (default viewer) can't show then properly.

VS Runtime files:
Copy following files to same folder:
- Microsoft.VC90.CRT.manifest
- msvcp90.dll
- msvcr90.dll
and modify RuntimesFolder -macro in begin of cppcheck.iss to point to the
folder where files are. You can find runtime files from VS installation or from
net.

QT Libraries:
Visual Studio is used to build the GUI executable. And QT must be build with VS
also. When building QT make sure you build release targets!

Copy following files to same RuntimesFolder than VS runtime files:
- QtCore4.dll
- QtGui4.dll

Creating the installer executable:
#1 Open the ISTool and load cppcheck.iss
#2 Update the release version number:
   - look for line "#define AppVersion"
#3 Check all files are present:
   - from menu: Project / Verify files...
#4 Compile the installer
   - from menu: Project / Compile Setup

If compilation succeeds, the installer executable is created into /Build
-folder. The filename is cppcheck-[version]-setup.exe.

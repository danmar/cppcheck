InnoSetup Windows installer for the cppcheck
--------------------------------------------

NOTE: This installer is OLD and not maintained anymore. See readme.txt for
information about new WiX installer!

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
/src/Release/cppcheck.exe
/win_installer/icon.bmp
/win_installer/LargeLogo.bmp
/win_installer/
/gui/release/gui.exe
/gui/cppcheck_de.qm
/gui/cppcheck_en.qm
/gui/cppcheck_fi.qm
/gui/cppcheck_pl.qm
/gui/cppcheck_ru.qm
/gui/cppcheck_se.qm

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

NOTE: To make local installation of runtimes to work you must remove the
publicKeyToken="blahblah" -attribute from the manifest file.

QT Libraries:
Visual Studio is used to build the GUI executable. And QT must be build with VS
also. When building QT make sure you build release targets!

Copy following files to same RuntimesFolder than VS runtime files:
- QtCore4.dll
- QtGui4.dll
- QtXml4.dll

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

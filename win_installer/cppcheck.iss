; cppcheck InnoSetup installer script
; Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.

; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/

; To create an installer, get the latest InnoSetup installer creation
; program from: http://www.jrsoftware.org/


#define MyAppName "cppcheck"
#define AppVersion "1.37"
#define MyAppURL "http://cppcheck.wiki.sourceforge.net/"
#define MyAppExeName "cppcheck.exe"
#define QTGuiExe "gui.exe"
#define QTGuiName "cppcheck GUI"

; Set this macro to point to folder where VS and QT runtimes are
; Runtime files are not included in repository so you need to
; get them from elsewhere. VS runtimes come with VS installation. QT runtimes
; you must compile yourself.
#define RuntimesFolder "..\..\Runtimes"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{48254FD9-669B-4A53-A060-D56EB50DDF72}
AppName={#MyAppName}
AppVersion={#AppVersion}
AppVerName={#MyAppName} {#AppVersion}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; This is for the installer file
VersionInfoVersion={#AppVersion}
VersionInfoDescription=cppcheck installer

DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=true
LicenseFile=..\COPYING

OutputBaseFilename={#MyAppName}-{#AppVersion}-setup

ChangesEnvironment=true
OutputDir=..\Build

Compression=lzma/ultra
InternalCompressLevel=ultra
SolidCompression=true

; Installer graphics
WizardImageFile=.\LargeLogo.bmp
WizardSmallImageFile=.\icon.bmp
WizardImageStretch=false

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Types]
Name: full; Description: Full installation
Name: compact; Description: Compact installation
Name: custom; Description: Custom installation; Flags: iscustom

; We have two components:
; - Core contains all C-runtimes, command line executable and basic documents
; - QTGui contains QT libraries and QT-based GUI
[Components]
Name: Core; Description: Core files (command line executable); Types: full custom compact; Flags: fixed
Name: QTGui; Description: GUI [Beta]; Types: full

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: guidesktopicon; Description: Create a desktop icon for GUI; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: modifypath; Description: &Add {#MyAppName} folder to your system path; Flags: unchecked

[InstallDelete]
; Name was changed to COPYING.txt so remove the old file.
Type: files; Name: {app}\COPYING

[Files]
; Core / command line
Source: ..\src\Release\cppcheck.exe; DestDir: {app}; Flags: ignoreversion; Components: Core
Source: ..\COPYING; DestDir: {app}; DestName: COPYING.txt; Flags: ignoreversion; Components: Core
Source: ..\readme.txt; DestDir: {app}; Flags: ignoreversion; Components: Core
Source: ..\AUTHORS; DestDir: {app}; DestName: AUTHORS.txt; Flags: ignoreversion; Components: Core
; VS runtimes
Source: {#RuntimesFolder}\Microsoft.VC90.CRT.manifest; DestDir: {app}; Components: Core
Source: {#RuntimesFolder}\msvcp90.dll; DestDir: {app}; Components: Core
Source: {#RuntimesFolder}\msvcr90.dll; DestDir: {app}; Components: Core
; GUI executable
Source: ..\gui\Release\gui.exe; DestDir: {app}; Flags: ignoreversion; Components: QTGui
; GUI translations
Source: ..\gui\cppcheck_de.qm; DestDir: {app}; Components: QTGui
Source: ..\gui\cppcheck_en.qm; DestDir: {app}; Components: QTGui
Source: ..\gui\cppcheck_fi.qm; DestDir: {app}; Components: QTGui
Source: ..\gui\cppcheck_pl.qm; DestDir: {app}; Components: QTGui
Source: ..\gui\cppcheck_ru.qm; DestDir: {app}; Components: QTGui
Source: ..\gui\cppcheck_se.qm; DestDir: {app}; Components: QTGui
; QT runtimes
Source: {#RuntimesFolder}\QtCore4.dll; DestDir: {app}; Components: QTGui
Source: {#RuntimesFolder}\QtGui4.dll; DestDir: {app}; Components: QTGui
Source: {#RuntimesFolder}\QtXml4.dll; DestDir: {app}; Components: QTGui

[Icons]
; As cppcheck is a program run from command prompt, make icons to open
; command prompt in install folder
Name: {group}\{#MyAppName}; Filename: {sys}\cmd.exe; WorkingDir: {app}; Components: Core
Name: {group}\{#QTGuiName}; Filename: {app}\{#QTGuiExe}; WorkingDir: {app}; Components: QTGui
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {#MyAppURL}; Components: Core
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}; Components: Core
; Desktop icons
Name: {commondesktop}\{#MyAppName}; Filename: {sys}\cmd.exe; WorkingDir: {app}; Tasks: desktopicon; Components: Core
Name: {commondesktop}\{#QTGuiName}; Filename: {app}\{#QTGuiExe}; WorkingDir: {app}; Tasks: guidesktopicon; Components: QTGui
; Doc icons
Name: {group}\Authors; Filename: {app}\AUTHORS.txt; IconFileName: {win}\NOTEPAD.EXE; Components: Core
Name: {group}\Copying; Filename: {app}\COPYING.txt; IconFileName: {win}\NOTEPAD.EXE; Components: Core

[Code]
function ModPathDir(): TArrayOfString;
var
	Dir: TArrayOfString;
begin
	setArrayLength(Dir, 1)
	Dir[0] := ExpandConstant('{app}');
	Result := Dir;
end;
#include "modpath.iss"

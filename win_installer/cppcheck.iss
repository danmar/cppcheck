; cppcheck InnoSetup installer script
; Copyright (c) 2009 Kimmo Varis

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
#define AppVersion "1.32"
#define MyAppURL "http://cppcheck.wiki.sourceforge.net/"
#define MyAppExeName "cppcheck.exe"

; Set this macro to point to folder where VS runtimes are
; Runtime files are not included in repository so you need to
; get them from elsewhere (e.g. from VS installation).
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

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: modifypath; Description: &Add {#MyAppName} folder to your system path; Flags: unchecked

[InstallDelete]
; Name was changed to COPYING.txt so remove the old file.
Type: files; Name: {app}\COPYING

[Files]
Source: ..\Build\Release\cppcheck.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\COPYING; DestDir: {app}; DestName: COPYING.txt; Flags: ignoreversion
Source: ..\readme.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\AUTHORS; DestDir: {app}; DestName: AUTHORS.txt; Flags: ignoreversion
; VS runtimes
Source: {#RuntimesFolder}\Microsoft.VC90.CRT.manifest; DestDir: {app}
Source: {#RuntimesFolder}\msvcp90.dll; DestDir: {app}
Source: {#RuntimesFolder}\msvcr90.dll; DestDir: {app}

[Icons]
; As cppcheck is a program run from command prompt, make icons to open
; command prompt in install folder
Name: {group}\{#MyAppName}; Filename: {sys}\cmd.exe; WorkingDir: {app}
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {#MyAppURL}
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {commondesktop}\{#MyAppName}; Filename: {sys}\cmd.exe; WorkingDir: {app}; Tasks: desktopicon

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

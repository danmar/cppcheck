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
#define AppVersion "1.30"
#define MyAppURL "http://sourceforge.net/projects/cppcheck/"
#define MyAppExeName "cppcheck.exe"

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

DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=..\COPYING

OutputBaseFilename={#MyAppName}-{#AppVersion}-setup

ChangesEnvironment=yes

Compression=lzma/ultra
InternalCompressLevel=ultra
SolidCompression=true

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: modifypath; Description: &Add {#MyAppName} folder to your system path; Flags: unchecked

[Files]
Source: ..\Release\cppcheck.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion
Source: ..\readme.txt; DestDir: {app}; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

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

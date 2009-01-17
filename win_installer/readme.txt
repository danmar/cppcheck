Simple Windows installer installing executable, license file and readme.txt to program
files/cppcheck folder.

Shortcuts are created to start cmd.exe in installation folder. So when 
user selects start menu/desktop icon he gets command prompt in cppcheck 
folder.

I'm just attaching the script file. Copy it to 'win_installer' -folder 
in cppcheck folder. It expects to find ../Release/cppcheck.exe.

Get the InnoSetup from:
http://www.innosetup.com/
Be sure to download the 'QuickStart Pack' as it installs some nice tools
like preprocessor support.

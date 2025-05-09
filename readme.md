# **Cppcheck** 

|release-windows|OSS-Fuzz|Coverity Scan Build Status|include-what-you-use|License|
|:--:|:--:|:--:|:--:|:--:|
|[![release-windows](https://github.com/danmar/cppcheck/actions/workflows/release-windows.yml/badge.svg?branch=main)](https://github.com/danmar/cppcheck/actions/workflows/release-windows.yml)|[![OSS-Fuzz](https://oss-fuzz-build-logs.storage.googleapis.com/badges/cppcheck.svg)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=1&q=proj:cppcheck)|[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/512.svg)](https://scan.coverity.com/projects/512)|[![include-what-you-use](https://github.com/danmar/cppcheck/actions/workflows/iwyu.yml/badge.svg?branch=main)](https://github.com/danmar/cppcheck/actions/workflows/iwyu.yml)|[![License](https://img.shields.io/badge/license-GPL3.0-blue.svg)](https://opensource.org/licenses/GPL-3.0)


## About the name

The original name of this program was "C++check", but it was later changed to "Cppcheck".

Despite the name, Cppcheck is designed for both C and C++.

## Manual

A manual is available [online](https://cppcheck.sourceforge.io/manual.pdf).

## Donate CPU

Cppcheck is a hobby project with limited resources. You can help us by donating CPU (1 core or as many as you like). It is simple:

 1. Download (and extract) Cppcheck source code.
 2. Run:
    ```
    cd cppcheck/
    virtualenv .env
    .env/bin/pip install -r tools/donate-cpu-requirements.txt
    .env/bin/python tools/donate-cpu.py
    ```

The script will analyse debian source code and upload the results to a cppcheck server. We need these results both to improve Cppcheck and to detect regressions.

You can stop the script whenever you like with Ctrl C.

## Compiling

Cppcheck requires a C++ compiler with (partial) C++11 support. Minimum required versions are GCC 5.1 / Clang 3.5 / Visual Studio 2015.

To build the GUI application, you need to use the CMake build system.

When building the command line tool, [PCRE](http://www.pcre.org/) is optional. It is used if you build with rules.

There are multiple compilation choices:
* CMake - cross platform build tool
* Windows: Visual Studio
* Windows: Qt Creator + MinGW
* GNU make
* GCC (g++)
* Clang (clang++)

The minimum required Python version is 3.6.

### CMake

The minimum required version is CMake 3.13.

Example, compiling Cppcheck with cmake:

```shell
mkdir build
cd build
cmake ..
cmake --build .
```

If you want to compile the GUI you can use the flag.
-DBUILD_GUI=ON

For rules support (requires pcre) use the flag.
-DHAVE_RULES=ON

For release builds it is recommended that you use:
-DUSE_MATCHCOMPILER=ON

For building the tests use the flag.
-DBUILD_TESTS=ON

Using cmake you can generate project files for Visual Studio,XCode,etc.

#### Building a specific configuration

For single-configuration generators (like "Unix Makefiles") you can generate and build a specific configuration (e.g. "RelWithDebInfo") using:

```shell
mkdir build_RelWithDebInfo
cd build_RelWithDebInfo
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build . --config RelWithDebInfo
```

For multi-configuration generators (like "Visual Studio 17 2022") the same is achieved using:

```shell
mkdir build
cd build
cmake ..
cmake --build . --config RelWithDebInfo
```

### Visual Studio

Use the cppcheck.sln file. The file is configured for Visual Studio 2019, but the platform toolset can be changed easily to older or newer versions. The solution contains platform targets for both x86 and x64.

To compile with rules, select "Release-PCRE" or "Debug-PCRE" configuration. pcre.lib (pcre64.lib for x64 builds) and pcre.h are expected to be in /externals then. A current version of PCRE for Visual Studio can be obtained using [vcpkg](https://github.com/microsoft/vcpkg).

### Visual Studio (from command line)

If you do not wish to use the Visual Studio IDE, you can compile cppcheck from the command line the following command.

```shell
msbuild cppcheck.sln
```

### VS Code (on Windows)

Install MSYS2 to get GNU toolchain with g++ and gdb (https://www.msys2.org/).
Create a settings.json file in the .vscode folder with the following content (adjust path as necessary):

```
{
    "terminal.integrated.shell.windows": "C:\\msys64\\usr\\bin\\bash.exe",
    "terminal.integrated.shellArgs.windows": [
        "--login",
    ],
    "terminal.integrated.env.windows": {
        "CHERE_INVOKING": "1",
        "MSYSTEM": "MINGW64",
    }
}
```

Run "make" in the terminal to build cppcheck.

For debugging create a launch.json file in the .vscode folder with the following content, which covers configuration for debugging cppcheck and misra.py:

```
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "cppcheck",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/cppcheck.exe",
            "args": [
                "--dump",
                "${workspaceFolder}/addons/test/misra/misra-test.c"
            ],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": true,
            "MIMode": "gdb",
            "miDebuggerPath": "C:/msys64/mingw64/bin/gdb.exe",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        },
        {
            "name": "misra.py",
            "type": "python",
            "request": "launch",
            "program": "${workspaceFolder}/addons/misra.py",
            "console": "integratedTerminal",
            "args": [
                "${workspaceFolder}/addons/test/misra/misra-test.c.dump"
            ]
        }
    ]
}
```

### Qt Creator + MinGW

The PCRE dll is needed to build the CLI. It can be downloaded here:
http://software-download.name/pcre-library-windows/

### GNU make

Simple, unoptimized build (no dependencies):

```shell
make
```

The recommended release build is:

```shell
make MATCHCOMPILER=yes FILESDIR=/usr/share/cppcheck HAVE_RULES=yes CXXFLAGS="-O2 -DNDEBUG -Wall -Wno-sign-compare -Wno-unused-function"
```

Flags:

1.  `MATCHCOMPILER=yes`
    Python is used to optimise cppcheck. The Token::Match patterns are converted into C++ code at compile time.

2.  `FILESDIR=/usr/share/cppcheck`
    Specify folder where cppcheck files are installed (addons, cfg, platform)

3.  `HAVE_RULES=yes`
    Enable rules (PCRE is required if this is used)

4.  `CXXFLAGS="-O2 -DNDEBUG -Wall -Wno-sign-compare -Wno-unused-function"`
    Enables most compiler optimizations, disables cppcheck-internal debugging code and enables basic compiler warnings.

### g++ (for experts)

If you just want to build Cppcheck without dependencies then you can use this command:

```shell
g++ -o cppcheck -std=c++11 -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml2 -Iexternals/picojson -Ilib cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml2/*.cpp
```

If you want to use `--rule` and `--rule-file` then dependencies are needed:

```shell
g++ -o cppcheck -std=c++11 -lpcre -DHAVE_RULES -Ilib -Iexternals -Iexternals/simplecpp -Iexternals/tinyxml2 cli/*.cpp lib/*.cpp externals/simplecpp/simplecpp.cpp externals/tinyxml2/*.cpp
```

### MinGW

```shell
mingw32-make
```

If you encounter the following error with `MATCHCOMPILER=yes` you need to specify your Python interpreter via `PYTHON_INTERPRETER`.

```
process_begin: CreateProcess(NULL, which python3, ...) failed.
makefile:24: pipe: No error
process_begin: CreateProcess(NULL, which python, ...) failed.
makefile:27: pipe: No error
makefile:30: *** Did not find a Python interpreter.  Stop.
```

### Other Compiler/IDE

1. Create an empty project file / makefile.
2. Add all cpp files in the cppcheck cli and lib folders to the project file / makefile.
3. Add all cpp files in the externals folders to the project file / makefile.
4. Compile.

### Cross compiling Win32 (CLI) version of Cppcheck in Linux

```shell
sudo apt-get install mingw32
make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi" RDYNAMIC=""
mv cppcheck cppcheck.exe
```

## Packages

Besides building yourself on the platform of your choice there are also several ways to obtain pre-built packages.<br/>

Official packages maintained by the Cppcheck team:
- (Windows) An official Windows installer is available via the official Cppcheck SourceForge page: https://cppcheck.sourceforge.io.
- (Windows) Official builds of the current development versions are available via the [release-windows](https://github.com/danmar/cppcheck/actions/workflows/release-windows.yml) workflow. They are built nightly for the `main` branch and for each commit for release branches. As these are development versions please refrain from using these in production environments!
  - A portable package (i.e. does not require installation) is available as the `portable` artifact. This is still a work-in-progress - see https://trac.cppcheck.net/ticket/10771 for details.
  - An installer is available via the `installer` artifact.
- (Multi-Platform) A premium version with additional features provided by the original author of Cppcheck is available for purchase via https://www.cppcheck.com.

Unofficial packages *not* maintained by the Cppcheck team but their respective packagers:
- (Windows / Outdated) A portable package is available via https://portableapps.com/apps/development/cppcheck-portable.
- (Windows / Outdated) A package is available via https://community.chocolatey.org/packages/cppcheck.
- (Windows / Outdated) A package is available via https://winget.run/pkg/Cppcheck/Cppcheck.
- (Windows / Outdated) A package is available via https://osdn.net/projects/sfnet_cppcheck.
- (Windows) A package is available via https://scoop.sh/#/apps?q=cppcheck.
- (Linux/Unix) Many major distros offer Cppcheck packages via their integrated package managers (`yum`, `apt`, `pacman`, etc.). See https://pkgs.org/search/?q=cppcheck or https://repology.org/project/cppcheck for an overview.
- (Linux/Unix) Unless you are using a "rolling" distro, it is likely that they are not carrying the latest version. There are several external (mainly unsupported) repositories like AUR (ArchLinux), PPA (ubuntu), EPEL (CentOS/Fedora) etc. which might provide up-to-date packages. 
- (Linux/Unix / Outdated) The Canonical Snapcraft packages (https://snapcraft.io/cppcheck / https://snapcraft.io/cppcheckgui) are unmaintained and contain very old (development) versions. Please refrain from using them! See https://trac.cppcheck.net/ticket/11641 for more details.
- (MacOS) A package is available via Homebrew (`brew`). See https://formulae.brew.sh/formula/cppcheck.
- (MacOS) A package is available via https://ports.macports.org/port/cppcheck
- (Multi-Platform) A package is available via https://anaconda.org/conda-forge/cppcheck.
- Packages are also available from various download portals (mainly the Windows installer - sometimes re-packaged).

*Note:* This is list is purely informational and listed in no particular order.

*Note:* Please always try to obtain the package from the primary official source of your operating system/distro first and make sure you are getting the latest released version.

*Note:* Some issues might be related to additional patches carried by the builds in these packages or the packaging itself. In that case issues might need to be reported to the respective project.

## Webpage

https://cppcheck.sourceforge.io/

---
title: Cppcheck GUI manual
subtitle: Version 2.1.99
author: Cppcheck team
lang: en
documentclass: report
---


# Standalone analysis

It is possible to quickly analyze files. Open the `Analyze` menu and click on
either `Files...` or `Directory...`.

It is recommended that you create a project for analysis. A properly configured
project will give you better analysis.

# Project

## Creating project

Open the `File` menu and click on `New project...`.

## Project options

The `Project file` dialog contains 4 tabs:
 - Paths and defines; paths to check and basic preprocessor settings.
 - Types and Functions; configuration of platform and 3rd party libraries
 - Analysis; analysis options
 - Warning options; formatting warnings, suppressing warnings, etc
 - Addons; extra analysis with addons

### Paths and defines

It is recommended to import a project file.

#### Import project

Project to import. Cppcheck will get:
 * what files to check
 * preprocessor defines
 * preprocessor include paths
 * language standard if set

#### Paths (If you do not import project)

What paths to check.

#### Defines (If you do not import project)

Cppcheck automatically checks the code with different preprocessor configurations.

    #ifdef A
        code1
    #endif
    #ifdef B
        code2
    #endif

Cppcheck will automatically perform analysis both when A is defined and B is
defined. So any bugs in both code1 and code2 will be detected.

If you want to configure that A will always be defined in Cppcheck analysis you
can do that here.

Defines are separated by semicolon. So you can for instance write:

    A;B=3;C

#### Undefines (If you do not import project)


Cppcheck automatically checks the code with different preprocessor configurations.

    #ifdef A
        code1
    #endif
    #ifdef B
        code2
    #endif

Cppcheck will automatically perform analysis both when A is defined and B is
defined. So any bugs in both code1 and code2 will be detected.

If you want to configure that A is never defined in Cppcheck analysis you can
do that here.

Undefines are separated by semicolon. So you can for instance write:

    A;C

#### Include paths (If you do not import project)

Specify include paths.

### Types and Functions

Cppcheck uses `Platform` setting to determine size of short/int/long/pointer/etc.

Check the libraries that you use in the `Libraries` listbox.

### Analysis

#### Cppcheck build dir

This is a work-folder that Cppcheck uses. Each Cppcheck project should have a
separate build dir. It is used for:
 * whole program analysis
 * debug output
 * faster analysis (if a source file has changed check it, if source file is
   not changed then reuse old results)
 * statistics

#### Parser

It is in general recommended to use Cppcheck parser. However you can choose to
use Clang parser; Clang will be executed with a command line flag that tells it
to dump its AST and Cppcheck will read that AST and convert it into a
corresponding Cppcheck AST and use that.

#### Analysis

Configure what kind of analysis you want.

The `Normal analysis` is recommended for most use cases. Especially if you use
Cppcheck in CI.

The `Bug hunting` can be used if you really want to find a bug in your code
and can invest time looking at bad results and providing extra configuration.

#### Limit analysis

You can turn off checking of headers. That could be interesting if Cppcheck is
very slow. But normally, you should check the code in headers.

It is possible to check the code in unused templates. However the Cppcheck AST
will be incomplete/wrong. The recommendation is that you do not check unused
templates to avoid wrong warnings. The templates will be checked properly when
you do use them.

Max CTU depth: How deep should the whole program analysis be. The risk with a
"too high" value is that Cppcheck will be slow.

Max recursion in template instantiation: Max recursion when Cppcheck
instantiates templates. The risk with a "too high" value is that Cppcheck will
be slow and can require much memory.


### Warning options

#### Root path

The root path for warnings. Cppcheck will strip away this part of the path
from warnings. For instance if there is a warning in
`../myproject/foo/bar/file.cpp` and the root path is `../myproject/foo` then
the path for the warning will be `bar/file.cpp`.

#### Warning Tags

Tags allow you to manually categorize warnings.

#### Exclude source files

Excluded source files will not be analyzed by Cppcheck

#### Suppressions

List of suppressions. These warnings will not be shown.

### Addons

Y2038 - 32-bit timers that count number of seconds since 1970 will overflow in
year 2038. Check that the code does not use such timers.

Thread safety - Check that the code is thread safe

Cert - Ensure that the Cert coding standard is followed

Misra - Ensure that the Misra coding standard is followed. Please note you
need to have a textfile with the misra rule texts to get proper warning
messages. Cppcheck is not legally allowed to distribute the misra rule texts.

Clang-tidy - Run Clang-tidy


# Preferences

`Number of threads`: Number of threads to use in analysis. Each thread checks
its own source file.

`Force checking of all #ifdef configurations`: Cppcheck try to check all code
and will therefore guess different preprocessor configurations. The maximum
number of configurations that is checked is 14 by default.

`Show full path of files`: Show the full paths in the results.

`Show "No errors found" message when no errors found`: If you want to get a
message box about this.

`Display error id column "Id"`: Show error id in results

`Enable inline suppressions`: You can suppress warnings with comments. See the
Cppcheck manual (http://cppcheck.sf.net/manual.pdf) for more information about
those.

`Check for inconclusive errors also`: When full analysis of the code can not
determine if there should be a warning or not, it is inconclusive. Normally
Cppcheck does not warn then.

`Show statistics on check completion`: Show statistics in a window when
analysis finish.

`Show internal warnings in log`: Internal warnings (for debugging) is shown
in the `Analysis log`.

`Applications`: Configure external editor to open from context menu when you
right click on a warning.

`Save all errors when creating report`: If hidden warnings should be saved or
not.

`Save full path to files in report`: If you use `Root path` the warnings on the
screen will not have the full path.

`Language`: Configure language to use for GUI.

`Python binary`: To be able to execute addons, Cppcheck needs to know where
python is. Unless you configure something, Cppcheck will try to execute python
in your PATH.

`Misra rule texts`: Only needed if you want to use the Misra addon. Cppcheck is
not legally allowed to distribute the Misra rule texts and these must be
provided by users. The Misra rule texts are proprietary. An example rule text
file can be found here: https://github.com/danmar/cppcheck/blob/main/addons/test/misra/misra2012_rules_dummy_ascii.txt

`Clang path`: The path to `clang` binary. If no path is provided then system
PATH is used.

`Visual studio headers`: If you want to use the Visual Studio headers in the
analysis you can provide the path(s) here. Hint: Open a visual studio command
prompt and type `SET INCLUDE`. Then copy/paste the paths.

`Code editor style`: The visual theme to use for the code editor that is used
when you investigate results.



# Looking at results

When you have run the analysis it is time to look at the results.

If you click on a warning then the corresponding code will be shown in the
"Warning details" at the bottom.

You can right click warnings to get options. The difference of "hiding" a
warning and "suppressing" a warning is that the suppression is permanent and
hiding the warning is only temporary.


# Tagging warnings

You can manually categorize warnings.

You choose the names of the categories yourself in the project file dialog.

If tag names are configured then when you look at results you can right click
on a warning and tag it.




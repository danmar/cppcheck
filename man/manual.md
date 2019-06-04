---
title: Cppcheck manual
subtitle: Version 1.88
author: Cppcheck team
lang: en
toc: true
documentclass: report
---

# Introduction

Cppcheck is an analysis tool for C/C++ code. It provides unique code analysis to detect bugs and focuses on detecting undefined behaviour and dangerous coding constructs. The goal is to detect only real errors in the code (i.e. have very few false positives).

Supported code and platforms:

- You can check non-standard code that contains various compiler extensions, inline assembly code, etc.
- Cppcheck should be compilable by any C++ compiler that handles the latest C++ standard.
- Cppcheck should work on any platform that has sufficient CPU and memory.

Please understand that there are limits of Cppcheck. Cppcheck is rarely wrong about reported errors. But there are
many bugs that it doesn't detect.

You will find more bugs in your software by testing your software carefully, than by using Cppcheck. You will find
more bugs in your software by instrumenting your software, than by using Cppcheck. But Cppcheck can still detect some
of the bugs that you miss when testing and instrumenting your software.

# Getting started

## GUI

It is not required but creating a new project file is a good first step. There are a few options you can tweak to get
good results.

In the project settings dialog, the first option you see is "Import project". It is recommended that you use this
feature if you can. Cppcheck can import:

- Visual studio solution / project
- Compile database (can be generated from cmake/qbs/etc build files)
- Borland C++ Builder 6

When you have filled out the project settings and click on OK; the Cppcheck analysis will start.

## Command line

### First test

Here is a simple code

    int main()
    {
        char a[10];
        a[10] = 0;
        return 0;
    }

If you save that into file1.c and execute:

    cppcheck file1.c

The output from cppcheck will then be:

    Checking file1.c...
    [file1.c:4]: (error) Array 'a[10]' index 10 out of bounds

### Checking all files in a folder

Normally a program has many source files. And you want to check them all. Cppcheck can check all source files in a directory:

    cppcheck path

If "path" is a folder then cppcheck will recursively check all source files in this folder.

    Checking path/file1.cpp...
    1/2 files checked 50% done
    Checking path/file2.cpp...
    2/2 files checked 100% done

### Check files manually or use project file

With Cppcheck you can check files manually, by specifying files/paths to check and settings. Or you can use a project file (cmake/visual studio/etc).

We don't know which approach (project file or manual configuration) will give you the best results. It is recommended that you try both. It is possible that you will get different results so that to find most bugs you need to use both approaches.

Later chapters will describe this in more detail.

### Excluding a file or folder from checking

To exclude a file or folder, there are two options. The first option is to only provide the paths and files you want to check.

    cppcheck src/a src/b

All files under src/a and src/b are then checked.

The second option is to use -i, with it you specify files/paths to ignore. With this command no files in src/c are checked:

    cppcheck -isrc/c src

This option does not currently work with the `--project` option and is only valid when supplying an input directory. To ignore multiple directories supply the -i multiple times. The following command ignores both the src/b and src/c directories.

    cppcheck -isrc/b -isrc/c

## Severities

The possible severities for messages are:

**error**

used when bugs are found

**warning**

suggestions about defensive programming to prevent bugs

**style**

stylistic issues related to code cleanup (unused functions, redundant code, constness, and such)

**performance**

Suggestions for making the code faster. These suggestions are only based on common knowledge. It is not certain you'll get any measurable difference in speed by fixing these messages.

**portability**

portability warnings. 64-bit portability. code might work different on different compilers. etc.

**information**

Configuration problems. The recommendation is to only enable these during configuration.

# Importing project

You can import some project files and build configurations into Cppcheck.

## Cppcheck GUI project

You can import and use Cppcheck GUI project files in the command line tool:

    cppcheck --project=foobar.cppcheck

## CMake

Generate a compile database:

    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .

The file `compile_commands.json` is created in the current folder. Now run Cppcheck like this:

    cppcheck --project=compile_commands.json

## Visual Studio

You can run Cppcheck on individual project files (\*.vcxproj) or on a whole solution (\*.sln)

Running Cppcheck on an entire Visual Studio solution:

    cppcheck --project=foobar.sln

Running Cppcheck on a Visual Studio project:

    cppcheck --project=foobar.vcxproj

## C++ Builder 6

Running Cppcheck on a C++ Builder 6 project:

    cppcheck --project=foobar.bpr

## Other

If you can generate a compile database then it's possible to import that in Cppcheck.

In Linux you can use for instance the `bear` (build ear) utility to generate a compile database from arbitrary build tools:

    bear make

# Platform

You should use a platform configuration that match your target.

By default Cppcheck uses native platform configuration that works well if your code is compiled and executed locally.

Cppcheck has builtin configurations for Unix and Windows targets. You can easily use these with the --platform command line flag.

You can also create your own custom platform configuration in a XML file. Here is an example:

    <?xml version="1"?>
    <platform>
      <char_bit>8</char_bit>
      <default-sign>signed</default-sign>
      <sizeof>
        <short>2</short>
        <int>4</int>
        <long>4</long>
        <long-long>8</long-long>
        <float>4</float>
        <double>8</double>
        <long-double>12</long-double>
        <pointer>4</pointer>
        <size_t>4</size_t>
        <wchar_t>2</wchar_t>
      </sizeof>
    </platform>

# Preprocessor Settings

If you use `--project` then Cppcheck will use the preprocessor settings from the imported project. Otherwise you'll probably want to configure the include paths, defines, etc.

## Defines

Here is a file that has 2 preprocessor configurations (with A defined and without A defined):

    #ifdef A
        x = y;
    #else
        x = z;
    #endif

By default Cppcheck will check all preprocessor configurations (except those that have #error in them). So the above code will by default be analyzed both with `A` defined and without `A` defined.

You can use `-D` to change this. When you use `-D`, cppcheck will by default only check the given configuration and nothing else. This is how compilers work. But you can use `--force` or `--max-configs` to override the number of configurations.

Check all configurations:

    cppcheck file.c

Only check the configuration A:

    cppcheck -DA file.c

Check all configurations when macro A is defined

    cppcheck -DA --force file.c

Another useful flag might be `-U`. It tells Cppcheck that a macro is not defined. Example usage:

    cppcheck -UX file.c

That will mean that X is not defined. Cppcheck will not check what happens when X is defined.

## Include paths

To add an include path, use `-I`, followed by the path.

Cppcheck's preprocessor basically handles includes like any other preprocessor. However, while other preprocessors stop working when they encounter a missing header, cppcheck will just print an information message and continues parsing the code.

The purpose of this behaviour is that cppcheck is meant to work without necessarily seeing the entire code. Actually, it is recommended to not give all include paths. While it is useful for cppcheck to see the declaration of a class when checking the implementation of its members, passing standard library headers is highly discouraged because it will result in worse results and longer checking time. For such cases, .cfg files (see below) are the better way to provide information about the implementation of functions and types to cppcheck.

# Suppressions

If you want to filter out certain errors you can suppress these.

Please note that if you see a false positive then we (the Cppcheck team) want that you report it so we can fix it.

## Plain text suppressions

You can suppress certain types of errors. The format for such a suppression is one of:

    [error id]:[filename]:[line]
    [error id]:[filename2]
    [error id]

The `error id` is the id that you want to suppress. The easiest way to get it is to use the --template=gcc command line flag. The id is shown in brackets.

The filename may include the wildcard characters \* or ?, which match any sequence of characters or any single character respectively. It is recommended that you use "/" as path separator on all operating systems.

## Command line suppression

The `--suppress=` command line option is used to specify suppressions on the command line. Example:

    cppcheck --suppress=memleak:src/file1.cpp src/

## Suppressions in a file

You can create a suppressions file. Example:

    // suppress memleak and exceptNew errors in the file src/file1.cpp
    memleak:src/file1.cpp
    exceptNew:src/file1.cpp

    // suppress all uninitvar errors in all files
    uninitvar

Note that you may add empty lines and comments in the suppressions file.

You can use the suppressions file like this:

    cppcheck --suppressions-list=suppressions.txt src/

## XML suppressions

You can specify suppressions in a XML file. Example file:

    <?xml version="1.0"?>
    <suppressions>
      <suppress>
        <id>uninitvar</id>
        <fileName>src/file1.c</fileName>
        <lineNumber>10</lineNumber>
        <symbolName>var</symbolName>
      </suppress>
    </suppressions>

The XML format is extensible and may be extended with further attributes in the future.

You can use the suppressions file like this:

    cppcheck --suppress-xml=suppressions.xml src/

## Inline suppressions

Suppressions can also be added directly in the code by adding comments that contain special keywords. Before adding such comments, consider that the code readability is sacrificed a little.

This code will normally generate an error message:

    void f() {
        char arr[5];
        arr[10] = 0;
    }

The output is:

    cppcheck test.c
    [test.c:3]: (error) Array 'arr[5]' index 10 out of bounds

To suppress the error message, a comment can be added:

    void f() {
        char arr[5];

        // cppcheck-suppress arrayIndexOutOfBounds
        arr[10] = 0;
    }

Now the `--inline-suppr` flag can be used to suppress the warning. No error is reported when invoking cppcheck this way:

    cppcheck --inline-suppr test.c

You can specify that the inline suppression only applies to a specific symbol:

    // cppcheck-suppress arrayIndexOutOfBounds symbolName=arr

You can write comments for the suppress, however is recommended to use ; or // to specify where they start:

    // cppcheck-suppress arrayIndexOutOfBounds ; some comment
    // cppcheck-suppress arrayIndexOutOfBounds // some comment

# XML output

Cppcheck can generate output in XML format. Use `--xml` to enable this format.

A sample command to check a file and output errors in the XML format:

    cppcheck --xml file1.cpp

Here is a sample report:

    <?xml version="1.0" encoding="UTF-8"?>
    <results version="2">
      <cppcheck version="1.66">
      <errors>
        <error id="someError" severity="error" msg="short error text"
           verbose="long error text" inconclusive="true" cwe="312">
          <location file0="file.c" file="file.h" line="1"/>
       </error>
      </errors>
    </results>

## The `<error>` element

Each error is reported in a `<error>` element. Attributes:

**id**

id of error. These are always valid symbolnames.

**severity**

error/warning/style/performance/portability/information

**msg**

the error message in short format

**verbose**

the error message in long format

**inconclusive**

this attribute is only used when the error message is inconclusive

**cwe**

CWE ID for the problem. This attribute is only used when the CWE ID for the message is known.

## The `<location>` element

All locations related to an error are listed with `<location>` elements. The primary location is listed first.

Attributes:

**file**

filename. both relative and absolute paths are possible.

**file0**

name of the source file (optional)

**line**

line number

**info**

short information for each location (optional)

# Reformatting the text output

If you want to reformat the output so it looks different you can use templates.

## Predefined output formats

To get Visual Studio compatible output you can use --template=vs:

    cppcheck --template=vs samples/arrayIndexOutOfBounds/bad.c

This output will look like this:

    Checking samples/arrayIndexOutOfBounds/bad.c ...
    samples/arrayIndexOutOfBounds/bad.c(6): error: Array 'a[2]' accessed at index 2, which is out of bounds.

To get gcc compatible output you can use --template=gcc:

    cppcheck --template=gcc samples/arrayIndexOutOfBounds/bad.c

The output will look like this:

    Checking samples/arrayIndexOutOfBounds/bad.c ...
    samples/arrayIndexOutOfBounds/bad.c:6:6: warning: Array 'a[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]
    a[2] = 0;
      ^

## User defined output format (single line)

You can write your own pattern. For instance, to get warning messages that are formatted like old gcc such format can be used:

    cppcheck --template="{file}:{line}: {severity}: {message}" samples/arrayIndexOutOfBounds/bad.c

The output will look like this:

    Checking samples/arrayIndexOutOfBounds/bad.c ...
    samples/arrayIndexOutOfBounds/bad.c:6: error: Array 'a[2]' accessed at index 2, which is out of bounds.

A comma separated format:

    cppcheck --template="{file},{line},{severity},{id},{message}" samples/arrayIndexOutOfBounds/bad.c

The output will look like this:

    Checking samples/arrayIndexOutOfBounds/bad.c ...
    samples/arrayIndexOutOfBounds/bad.c,6,error,arrayIndexOutOfBounds,Array 'a[2]' accessed at index 2, which is out of bounds.

## User defined output format (multi line)

Many warnings have multiple locations. Example code:

    void f(int *p)
    {
        *p = 3;       // line 3
    }

    int main()
    {
        int *p = 0;   // line 8
        f(p);         // line 9
        return 0;
    }

There is a possible null pointer dereference at line 3. Cppcheck can show how it came to that conclusion by showing extra location information. You need to use both --template and --template-location at the command line.

Example command:

    cppcheck --template="{file}:{line}: {severity}: {message}\n{code}" --template-location="{file}:{line}: note: {info}\n{code}" multiline.c

The output from Cppcheck is:

    Checking multiline.c ...
    multiline.c:3: warning: Possible null pointer dereference: p
        *p = 3;
         ^
    multiline.c:8: note: Assignment 'p=0', assigned value is 0
        int *p = 0;
                 ^
    multiline.c:9: note: Calling function 'f', 1st argument 'p' value is 0
        f(p);
          ^
    multiline.c:3: note: Null pointer dereference
        *p = 3;
         ^

The first line in the warning is formatted by the --template format.

The other lines in the warning are formatted by the --template-location format.

### Format specifiers for --template

The available specifiers for --template are:

**{file}**

File name

**{line}**

Line number

**{column}**

Column number

**{callstack}**

Write all locations. Each location is written in [{file}:{line}] format and the locations are separated by ->. For instance it might look like: [multiline.c:8] -> [multiline.c:9] -> [multiline.c:3]

**{inconclusive:text}**

If warning is inconclusive then the given text is written. The given text can be any arbitrary text that does not contain }. Example: {inconclusive:inconclusive,}

**{severity}**

error/warning/style/performance/portability/information

**{message}**

The warning message

**{id}**

Warning id

**{code}**

The real code.

**\\t**

Tab

**\\n**

Newline

**\\r**

Carriage return

### Format specifiers for --template-location

The available specifiers for `--template-location` are:

**{file}**

File name

**{line}**

Line number

**{column}**

Column number

**{info}**

Information message about current location

**{code}**

The real code.

**\\t**

Tab

**\\n**

Newline

**\\r**

Carriage return

# Speeding up Cppcheck

It is possible to speed up Cppcheck analysis in a few different ways.

## Preprocessor configurations

Imagine this source code:

    void foo()
    {
        int x;
    #ifdef __GNUC__
        x = 0;
    #endif
    #ifdef _MSC_VER
        x = 1;
    #endif
        return x;
    }

By default Cppcheck will try to check all the configurations. There are 3 important configurations here:

- Neither `__GNUC__` nor `_MSC_VER` is defined
- `__GNUC__` is defined
- `_MSC_VER` is defined

When you run Cppcheck, the output will be something like:

    $ cppcheck test.c
    Checking test.c ...
    [test.c:10]: (error) Uninitialized variable: x
    Checking test.c: __GNUC__...
    Checking test.c: _MSC_VER...

Now if you want you can limit the analysis. You probably know what the target compiler is. If `-D` is supplied and you do not specify `--force` then Cppcheck will only check the configuration you give.

    $ cppcheck -D __GNUC__ test.c
    Checking test.c ...
    Checking test.c: __GNUC__=1...

## Unused templates

If you think Cppcheck is slow and you are using templates, then you should try how it works to remove unused templates.

Imagine this code:

    template <class T> struct Foo {
        T x = 100;
    };

    template <class T> struct Bar {
        T x = 200 / 0;
    };

    int main() {
        Foo<int> foo;
        return 0;
    }

Cppcheck says:

    $ cppcheck test.cpp
    Checking test.cpp ...
    [test.cpp:7]: (error) Division by zero.

It complains about division by zero in `Bar` even though `Bar` is not instantiated.

You can use the option `--remove-unused-templates` to remove unused templates from Cppcheck analysis.

Example:

    $ cppcheck --remove-unused-templates test.cpp
    Checking test.cpp ...

This lost message is in theory not critical, since `Bar` is not instantiated the division by zero should not occur in your real program.

The speedup you get can be remarkable.

## Check headers

TBD

# Addons

Addons are scripts with extra checks. Cppcheck is distributed with a few addons. You can easily write your own custom addon.

If an addon does not need any arguments, you can run it directly on the cppcheck command line. For instance you can run the addon "misc" like this:

    cppcheck --addon=misc somefile.c

If an addon need additional arguments, you can not execute it directly on the command line. Create a json file with the addon configuration:

    {
        "script": "misra",
        "args": [ "--rule-texts=misra.txt" ]
    }

And then such configuration can be executed on the cppcheck command line:

    cppcheck --addon=misra.json somefile.c

## CERT

Check CERT coding rules. No configuration is needed.

Example usage:

    cppcheck --addon=cert somefile.c

## Findcasts

Will just locate C-style casts in the code. No configuration is needed.

Example usage:

    cppcheck --addon=findcasts somefile.c

## Misc

Misc checks. No configuration is needed.

These are checks that we thought would be useful, however it could sometimes warn for coding style that is by intention. For instance it warns about missing comma
between string literals in array initializer.. that could be a mistake but maybe you use string concatenation by intention.

Example usage:

    cppcheck --addon=misc somefile.c

## Misra

Check that your code is Misra C 2012 compliant.

To run the Misra addon you need to write a configuration file, because the addon require parameters.

To run this addon you need to have a text file with the misra rule texts. You copy/paste these rule texts from the Misra C 2012 PDF, buy this PDF from <http://www.misra.org.uk> (costs 15-20 pounds)

This is an example misra configuration file:

    {
        "script": "misra",
        "args": [ "--rule-texts=misra.txt" ]
    }

The file misra.txt contains the text from "Appendix A Summary of guidelines" in the Misra C 2012 PDF.

    Appendix A Summary of guidelines
    Rule 1.1
    Rule text
    Rule 1.2
    Rule text
    ...

Usage:

    cppcheck --addon=my-misra-config.json somefile.c

## Naming

Check naming conventions. You specify your naming conventions for variables/functions/etc using regular expressions.

Example configuration (variable names must start with lower case, function names must start with upper case):

    {
        "script": "naming",
        "args": [
            "--var=[a-z].*",
            "--function=[A-Z].*"
        ]
    }

Usage:

    cppcheck --addon=my-naming.json somefile.c

## Namingng

Check naming conventions. You specify the naming conventions using regular expressions in a json file.

Example addon configuration:

    {
        "script": "namingng",
        args: [ "--configfile=ROS_naming.json" ]
    }

Usage:

    cppcheck --addon=namingng-ros.json somefile.c

## Threadsafety

This will warn if you have local static objects that are not threadsafe. No configuration is needed.

Example usage:

    cppcheck --addon=threadsafety somefile.c

## Y2038

Check for the Y2038 bug. No configuration is needed.

Example usage:

    cppcheck --addon=y2038 somefile.c

# Library configuration

When external libraries are used, such as WinAPI, POSIX, gtk, Qt, etc, Cppcheck doesn't know how the external functions behave. Cppcheck then fails to detect various problems such as leaks, buffer overflows, possible null pointer dereferences, etc. But this can be fixed with configuration files.

Cppcheck already contains configurations for several libraries. They can be loaded as described below. Note that the configuration for the standard libraries of C and C++, std.cfg, is always loaded by cppcheck. If you create or update a configuration file for a popular library, we would appreciate if you upload it to us.

## Using your own custom .cfg file

You can create and use your own .cfg files for your projects. Use `--check-library` and `--enable=information` to get hints about what you should configure.

It is recommended that you use the `Library Editor` in the `Cppcheck GUI` to edit configuration files. It is available in the `View` menu. Not all settings are documented in this manual yet.

If you have a question about the .cfg file format it is recommended that you ask in the forum (<http://sourceforge.net/p/cppcheck/discussion/>).

The command line cppcheck will try to load custom .cfg files from the working path - execute cppcheck from the path where the .cfg files are.

The cppcheck GUI will try to load custom .cfg files from the project file path. The custom .cfg files should be shown in the Edit Project File dialog that you open from the `File` menu.

## Memory and resource leaks

Cppcheck has configurable checking for leaks, e.g. you can specify which functions allocate and free memory or resources and which functions do not affect the allocation at all.

### `<alloc>` and `<dealloc>`

Here is an example program:

    void test()
    {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(255,0,0));
    }

The code example above has a resource leak - CreatePen() is a WinAPI function that creates a pen. However, Cppcheck doesn't assume that return values from functions must be freed. There is no error message:

    $ cppcheck pen1.c
    Checking pen1.c...

If you provide a configuration file then Cppcheck detects the bug:

    $ cppcheck --library=windows.cfg pen1.c
    Checking pen1.c...
    [pen1.c:3]: (error) Resource leak: pen

Here is a minimal windows.cfg file:

    <?xml version="1.0"?>
    <def>
      <resource>
        <alloc>CreatePen</alloc>
        <dealloc>DeleteObject</dealloc>
      </resource>
    </def>

The allocation and deallocation functions are organized in groups. Each group is defined in a `<resource>` or `<memory>` tag and is identified by its `<dealloc>` functions. This means, groups with overlapping `<dealloc>` tags are merged.

### `<leak-ignore>` and `<use>`

Often the allocated pointer is passed to functions. Example:

    void test()
    {
        char *p = malloc(100);
        dostuff(p);
    }

If Cppcheck doesn't know what `dostuff` does, without configuration it will assume that `dostuff` takes care of the memory so there is no memory leak.

To specify that `dostuff` doesn't take care of the memory in any way, use `<leak-ignore/>` in the `<function>` tag (see next section):

    <?xml version="1.0"?>
    <def>
     <function name="dostuff">
        <leak-ignore/>
        <arg nr="1"/>
      </function>
    </def>

If instead `dostuff` takes care of the memory then this can be configured with:

    <?xml version="1.0"?>
    <def>
      <memory>
        <dealloc>free</dealloc>
        <use>dostuff</use>
      </memory>
    </def>

The `<use>` configuration has no logical purpose. You will get the same warnings without it. Use it to silence --check-library information messages.

## Function behavior

To specify the behaviour of functions and how they should be used, `<function>` tags can be used. Functions are identified by their name, specified in the name attribute and their number of arguments. The name is a comma-separated list of function names. For functions in namespaces or classes, just provide their fully qualified name. For example: `<function name="memcpy,std::memcpy">`. If you have template functions then provide their instantiated names `<function name="dostuff<int>">`.

### Function arguments

The arguments a function takes can be specified by `<arg>` tags. Each of them takes the number of the argument (starting from 1) in the nr attribute, `nr="any"` for arbitrary arguments, or `nr="variadic"` for variadic arguments. Optional arguments can be specified by providing a default value: `default="value"`. The specifications for individual arguments override this setting.

#### Not bool

Here is an example program with misplaced comparison:

    void test()
    {
        if (MemCmp(buffer1, buffer2, 1024==0)) {}
    }

Cppcheck assumes that it is fine to pass boolean values to functions:

    $ cppcheck notbool.c
    Checking notbool.c...

If you provide a configuration file then Cppcheck detects the bug:

    $ cppcheck --library=notbool.cfg notbool.c
    Checking notbool.c...
    [notbool.c:5]: (error) Invalid MemCmp() argument nr 3. A non-boolean value is required.

Here is the minimal notbool.cfg

    <?xml version="1.0"?>
    <def>
      <function name="MemCmp">
        <arg nr="1"/>
        <arg nr="2"/>
        <arg nr="3">
          <not-bool/>
        </arg>
      </function>
    </def>

#### Uninitialized memory

Here is an example program:

    void test()
    {
        char buffer1[1024];
        char buffer2[1024];
        CopyMemory(buffer1, buffer2, 1024);
    }

The bug here is that buffer2 is uninitialized. The second argument for CopyMemory needs to be initialized. However, Cppcheck assumes that it is fine to pass uninitialized variables to functions:

    $ cppcheck uninit.c
    Checking uninit.c...

If you provide a configuration file then Cppcheck detects the bug:

    $ cppcheck --library=windows.cfg uninit.c
    Checking uninit.c...
    [uninit.c:5]: (error) Uninitialized variable: buffer2

Note that this implies for pointers that the memory they point at has to be initialized, too.

Here is the minimal windows.cfg:

    <?xml version="1.0"?>
    <def>
      <function name="CopyMemory">
        <arg nr="1"/>
        <arg nr="2">
          <not-uninit/>
        </arg>
        <arg nr="3"/>
      </function>
    </def>

#### Null pointers

Cppcheck assumes it's ok to pass NULL pointers to functions. Here is an example program:

    void test()
    {
        CopyMemory(NULL, NULL, 1024);
    }

The MSDN documentation is not clear if that is ok or not. But let's assume it's bad. Cppcheck assumes that it's ok to pass NULL to functions so no error is reported:

    $ cppcheck null.c
    Checking null.c...

If you provide a configuration file then Cppcheck detects the bug:

    $ cppcheck --library=windows.cfg null.c
    Checking null.c...
    [null.c:3]: (error) Null pointer dereference

Note that this implies `<not-uninit>` as far as values are concerned. Uninitialized memory might still be passed to the function.

Here is a minimal windows.cfg file:

    <?xml version="1.0"?>
    <def>
      <function name="CopyMemory">
        <arg nr="1">
          <not-null/>
        </arg>
        <arg nr="2"/>
        <arg nr="3"/>
      </function>
    </def>

#### Format string

You can define that a function takes a format string. Example:

    void test()
    {
        do_something("%i %i\n", 1024);
    }

No error is reported for that:

    $ cppcheck formatstring.c
    Checking formatstring.c...

A configuration file can be created that says that the string is a format string. For instance:

    <?xml version="1.0"?>
    <def>
      <function name="do_something">
        <formatstr type="printf"/>
        <arg nr="1">
          <formatstr/>
        </arg>
      </function>
    </def>

Now Cppcheck will report an error:

    $ cppcheck --library=test.cfg formatstring.c
    Checking formatstring.c...
    [formatstring.c:3]: (error) do_something format string requires 2 parameters but only 1 is given.

The type attribute can be either:

printf - format string follows the printf rules

scanf - format string follows the scanf rules

#### Value range

The valid values can be defined. Imagine:

    void test()
    {
        do_something(1024);
    }

No error is reported for that:

    $ cppcheck valuerange.c
    Checking valuerange.c...

A configuration file can be created that says that 1024 is out of bounds. For instance:

    <?xml version="1.0"?>
    <def>
      <function name="do_something">
        <arg nr="1">
          <valid>0:1023</valid>
        </arg>
      </function>
    </def>

Now Cppcheck will report an error:

    $ cppcheck --library=test.cfg range.c
    Checking range.c...
    [range.c:3]: (error) Invalid do_something() argument nr 1. The value is 1024 but the valid values are '0-1023'.

Some example expressions you can use in the valid element:

0,3,5  => only values 0, 3 and 5 are valid
-10:20  =>  all values between -10 and 20 are valid
:0  =>  all values that are less or equal to 0 are valid
0:  =>  all values that are greater or equal to 0 are valid
0,2:32  =>  the value 0 and all values between 2 and 32 are valid
-1.5:5.6  =>  all values between -1.5 and 5.6 are valid

#### `<minsize>`

Some function arguments take a buffer. With minsize you can configure the min size of the buffer (in bytes, not elements). Imagine:

    void test()
    {
        char str[5];
        do_something(str,"12345");
    }

No error is reported for that:

    $ cppcheck minsize.c
    Checking minsize.c...

A configuration file can for instance be created that says that the size of the buffer in argument 1 must be larger than the strlen of argument 2. For instance:

    <?xml version="1.0"?>
    <def>
      <function name="do_something">
        <arg nr="1">
          <minsize type="strlen" arg="2"/>
        </arg>
        <arg nr="2"/>
      </function>
    </def>

Now Cppcheck will report this error:

    $ cppcheck --library=1.cfg minsize.c
    Checking minsize.c...
    [minsize.c:4]: (error) Buffer is accessed out of bounds: str

There are different types of minsizes:

strlen
buffer size must be larger than other arguments string length. Example: see strcpy configuration in std.cfg

argvalue
buffer size must be larger than value in other argument. Example: see memset configuration in std.cfg

sizeof
buffer size must be larger than other argument buffer size. Example: see memcpy configuration in posix.cfg

mul
buffer size must be larger than multiplication result when multiplying values given in two other arguments. Typically one argument defines the element size and another element defines the number of elements. Example: see fread configuration in std.cfg

strz
With this you can say that an argument must be a zero-terminated string.

    <?xml version="1.0"?>
    <def>
      <function name="do_something">
        <arg nr="1">
         <strz/>
        </arg>
      </function>
    </def>

#### `<noreturn>`

Cppcheck doesn't assume that functions always return. Here is an example code:

    void test(int x)
    {
        int data, buffer[1024];
        if (x == 1)
            data = 123;
        else
            ZeroMemory(buffer, sizeof(buffer));
        buffer[0] = data;  // <- error: data is uninitialized if x is not 1
    }

In theory, if ZeroMemory terminates the program then there is no bug. Cppcheck therefore reports no error:

    $ cppcheck noreturn.c
    Checking noreturn.c...

However if you use `--check-library` and `--enable=information` you'll get this:

    $ cppcheck --check-library --enable=information noreturn.c
    Checking noreturn.c...
    [noreturn.c:7]: (information) --check-library: Function ZeroMemory() should have <noreturn> configuration

If a proper windows.cfg is provided, the bug is detected:

    $ cppcheck --library=windows.cfg noreturn.c
    Checking noreturn.c...
    [noreturn.c:8]: (error) Uninitialized variable: data

Here is a minimal windows.cfg file:

    <?xml version="1.0"?>
    <def>
      <function name="ZeroMemory">
        <noreturn>false</noreturn>
        <arg nr="1"/>
        <arg nr="2"/>
      </function>
    </def>

#### `<use-retval>`

As long as nothing else is specified, cppcheck assumes that ignoring the return value of a function is ok:

    bool test(const char* a, const char* b)
    {
        strcmp(a, b);  // <- bug: The call of strcmp does not have side-effects, but the return value is ignored.
        return true;
    }

In case strcmp has side effects, such as assigning the result to one of the parameters passed to it, nothing bad would happen:

    $ cppcheck useretval.c
    Checking useretval.c...

If a proper lib.cfg is provided, the bug is detected:

    $ cppcheck --library=lib.cfg --enable=warning useretval.c
    Checking useretval.c...
    [useretval.c:3]: (warning) Return value of function strcmp() is not used.

Here is a minimal lib.cfg file:

    <?xml version="1.0"?>
    <def>
      <function name="strcmp">
        <use-retval/>
        <arg nr="1"/>
        <arg nr="2"/>
      </function>
    </def>

#### `<pure>` and `<const>`

These correspond to the GCC function attributes `<pure>` and `<const>`.

A pure function has no effects except to return a value, and its return value depends only on the parameters and global variables.

A const function has no effects except to return a value, and its return value depends only on the parameters.

Here is an example code:

    void f(int x)
    {
        if (calculate(x) == 213) {
        } else if (calculate(x) == 213) {
             // unreachable code
        }
    }

If calculate() is a const function then the result of calculate(x) will be the same in both conditions, since the same parameter value is used.

Cppcheck normally assumes that the result might be different, and reports no warning for the code:

    $ cppcheck const.c
    Checking const.c...

If a proper const.cfg is provided, the unreachable code is detected:

    $ cppcheck --enable=style --library=const const.c
    Checking const.c...
    [const.c:7]: (style) Expression is always false because 'else if' condition matches previous condition at line 5.

Here is a minimal const.cfg file:

    <?xml version="1.0"?>
    <def>
      <function name="calculate">
        <const/>
        <arg nr="1"/>
      </function>
    </def>

#### Example configuration for strcpy()

The proper configuration for the standard strcpy() function would be:

    <function name="strcpy">
      <leak-ignore/>
      <noreturn>false</noreturn>
      <arg nr="1">
        <not-null/>
      </arg>
      <arg nr="2">
        <not-null/>
        <not-uninit/>
        <strz/>
      </arg>
    </function>

The `<leak-ignore/>` tells Cppcheck to ignore this function call in the leaks checking. Passing allocated memory to this function won't mean it will be deallocated.

The `<noreturn>` tells Cppcheck if this function returns or not.

The first argument that the function takes is a pointer. It must not be a null pointer, therefore `<not-null>` is used.

The second argument the function takes is a pointer. It must not be null. And it must point at initialized data. Using `<not-null>` and `<not-uninit>` is correct. Moreover it must point at a zero-terminated string so `<strz>` is also used.

## `<define>`

Libraries can be used to define preprocessor macros as well. For example:

    <?xml version="1.0"?>
    <def>
      <define name="NULL_VALUE" value="0"/>
    </def>

Each occurrence of "NULL_VALUE" in the code would then be replaced by "0" at preprocessor stage.

## `<podtype>`

Use this for integer/float/bool/pointer types. Not for structs/unions.

Lots of code relies on typedefs providing platform independent types. "podtype"-tags can be used to provide necessary information to cppcheck to support them. Without further information, cppcheck does not understand the type "uint16_t" in the following example:

    void test() {
        uint16_t a;
    }

No message about variable 'a' being unused is printed:

    $ cppcheck --enable=style unusedvar.cpp
    Checking unusedvar.cpp...

If uint16_t is defined in a library as follows, the result improves:

    <?xml version="1.0"?>
    <def>
      <podtype name="uint16_t" sign="u" size="2"/>
    </def>

The size of the type is specified in bytes. Possible values for the "sign" attribute are "s" (signed) and "u" (unsigned). Both attributes are optional. Using this library, cppcheck prints:

    $ cppcheck --library=lib.cfg --enable=style unusedvar.cpp
    Checking unusedvar.cpp...
    [unusedvar.cpp:2]: (style) Unused variable: a

## `<container>`

A lot of C++ libraries, among those the STL itself, provide containers with very similar functionality. Libraries can be used to tell cppcheck about their behaviour. Each container needs a unique ID. It can optionally have a startPattern, which must be a valid Token::Match pattern and an endPattern that is compared to the linked token of the first token with such a link. The optional attribute "inherits" takes an ID from a previously defined container.

Inside the `<container>` tag, functions can be defined inside of the tags `<size>`, `<access>` and `<other>` (on your choice). Each of them can specify an action like "resize" and/or the result it yields, for example "end-iterator".

The following example provides a definition for std::vector, based on the definition of "stdContainer" (not shown):

    <?xml version="1.0"?>
    <def>
      <container id="stdVector" startPattern="std :: vector &lt;" inherits="stdContainer">
        <size>
          <function name="push_back" action="push"/>
          <function name="pop_back" action="pop"/>
        </size>
        <access indexOperator="array-like">
          <function name="at" yields="at_index"/>
          <function name="front" yields="item"/>
          <function name="back" yields="item"/>
        </access>
      </container>
    </def>

The tag `<type>` can be added as well to provide more information about the type of container. Here is some of the attributes that can be set:

* `string='std-like'` can be set for containers that match `std::string` interfaces.
* `associative='std-like'` can be set for containers that match C++'s `AssociativeContainer` interfaces.

# HTML Report

You can convert the XML output from cppcheck into a HTML report. You'll need Python and the pygments module (<http://pygments.org/)> for this to work. In the Cppcheck source tree there is a folder htmlreport that contains a script that transforms a Cppcheck XML file into HTML output.

This command generates the help screen:

    htmlreport/cppcheck-htmlreport -h

The output screen says:

    Usage: cppcheck-htmlreport [options]

    Options:
       -h, --help      show this help message and exit
       --file=FILE     The cppcheck xml output file to read defects from.
                       Default is reading from stdin.
       --report-dir=REPORT_DIR
                       The directory where the html report content is written.
       --source-dir=SOURCE_DIR
                       Base directory where source code files can be found.

An example usage:

    ./cppcheck gui/test.cpp --xml 2> err.xml
    htmlreport/cppcheck-htmlreport --file=err.xml --report-dir=test1 --source-dir=.

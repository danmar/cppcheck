

# Cppcheck 1.88 dev

## Introduction

Cppcheck is an analysis tool for C/C++ code. Unlike C/C++ compilers and many other analysis tools, it doesn't detect
syntax errors. Instead, Cppcheck detects the types of bugs that the compilers normally fail to detect. The goal is no
false positives.

Supported code and platforms:

  - You can check non-standard code that contains various compiler extensions, inline assembly code, etc.
  - Cppcheck should be compilable by any C++ compiler that handles the latest C++ standard.
  - Cppcheck should work on any platform that has sufficient CPU and memory.

Please understand that there are limits of Cppcheck. Cppcheck is rarely wrong about reported errors. But there are
many bugs that it doesn't detect.

You will find more bugs in your software by testing your software carefully, than by using Cppcheck. You will find
more bugs in your software by instrumenting your software, than by using Cppcheck. But Cppcheck can still detect some
of the bugs that you miss when testing and instrumenting your software.

## Getting started

### GUI

It is not required but creating a new project file is a good first step. There are a few options you can tweak to get
good results.

In the project settings dialog, the first option you see is "Import project". It is recommended that you use this
feature if you can. Cppcheck can import:
 - Visual studio solution / project
 - Compile database (can be generated from cmake/qbs/etc build files)
 - Borland C++ Builder 6

When you have filled out the project settings and click on OK; the Cppcheck analysis will start.

### Command line

A good first command to try is either...

If you have a Visual studio solution / compile database (cmake/qbs/etc) / C++ Builder 6 project:

    cppcheck --enable=warning --project=<path of solution / project / compile database>

Or:

    cppcheck --enable=warning <folder where your source code is>

You can extend and adjust the analysis in many ways later.

## Importing project

You can import some project files and build configurations into Cppcheck.

### Cppcheck GUI project

You can import and use Cppcheck GUI project files in the command line tool:

    cppcheck --project=foobar.cppcheck

### CMake

Generate a compile database:

    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .

The file `compile_commands.json` is created in the current folder. Now run Cppcheck like this:

    cppcheck --project=compile_commands.json

### Visual Studio

You can run Cppcheck on individual project files (\*.vcxproj) or on a whole solution (\*.sln)

Running Cppcheck on an entire Visual Studio solution:

    cppcheck --project=foobar.sln

Running Cppcheck on a Visual Studio project:

    cppcheck --project=foobar.vcxproj

### C++ Builder 6

Running Cppcheck on a C++ Builder 6 project:

    cppcheck --project=foobar.bpr

### Other

If you can generate a compile database then it's possible to import that in Cppcheck.

In Linux you can use for instance the `bear` (build ear) utility to generate a compile database from arbitrary build tools:

    bear make


## Preprocessor Settings

If you use `--project` then Cppcheck will use the preprocessor settings from the imported project. Otherwise you'll probably want to configure the include paths, defines, etc.

### Defines

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


### Include paths

To add an include path, use `-I`, followed by the path.

Cppcheck's preprocessor basically handles includes like any other preprocessor. However, while other preprocessors stop working when they encounter a missing header, cppcheck will just print an information message and continues parsing the code.

The purpose of this behaviour is that cppcheck is meant to work without necessarily seeing the entire code. Actually, it is recommended to not give all include paths. While it is useful for cppcheck to see the declaration of a class when checking the implementation of its members, passing standard library headers is highly discouraged because it will result in worse results and longer checking time. For such cases, .cfg files (see below) are the better way to provide information about the implementation of functions and types to cppcheck.

## Suppressions

If you want to filter out certain errors you can suppress these.

Please note that if you see a false positive then we (the Cppcheck team) want that you report it so we can fix it. 

### Plain text suppressions

You can suppress certain types of errors. The format for such a suppression is one of:

    [error id]:[filename]:[line]
    [error id]:[filename2]
    [error id]

The `error id` is the id that you want to suppress. The easiest way to get it is to use the --template=gcc command line flag. The id is shown in brackets.

The filename may include the wildcard characters \* or ?, which match any sequence of characters or any single character respectively. It is recommended that you use "/" as path separator on all operating systems.

### Command line suppression

The `--suppress=` command line option is used to specify suppressions on the command line. Example:

    cppcheck --suppress=memleak:src/file1.cpp src/


### Suppressions in a file

You can create a suppressions file. Example:

    // suppress memleak and exceptNew errors in the file src/file1.cpp
    memleak:src/file1.cpp
    exceptNew:src/file1.cpp
    
    // suppress all uninitvar errors in all files
    uninitvar

Note that you may add empty lines and comments in the suppressions file.

You can use the suppressions file like this:

    cppcheck --suppressions-list=suppressions.txt src/

### XML suppressions

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

The xml format is extensible and may be extended with further attributes in the future.

You can use the suppressions file like this:

    cppcheck --suppress-xml=suppressions.xml src/

### Inline suppressions

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


## XML output

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

### The `<error>` element

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

### The `<location>` element

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

## Reformatting the text output

If you want to reformat the output so it looks different you can use templates.

### Predefined output formats

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

### User defined output format (single line)

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

### User defined output format (multi line)

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


#### Format specifiers for --template

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

**\t**

Tab

**\n**

Newline

**\r**

Carriage return


#### Format specifiers for --template-location

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

**\t**

Tab

**\t**

Newline

**\r**

Carriage return


## Misra

Cppcheck has an addon that checks for MISRA C 2012 compliance.

### Requirements

You need:

Python 2.X or 3.X

The MISRA C 2012 PDF. You can buy this from http://www.misra.org.uk (costs 15-20 pounds)

#### MISRA Text file

It is not allowed to publish the MISRA rule texts. Therefore the MISRA rule texts are not available directly in the addon. We can not publish the rule texts. Instead, you must provide the rule texts; the addon can read the rule texts from a text file. If you copy/paste all text in "Appendix A Summary of guidelines" from the MISRA pdf, then you have all the rule texts.

If you have installed xpdf, such text file can be generated on the command line (using pdftotext that is included in xpdf):

    pdftotext misra-c-2012.pdf output.txt

The output might not be 100% perfect so you might need to make minor tweaks manually.

Other pdf-to-text utilities might work also.

To create the text file manually, copy paste Appendix A "Summary of guidelines" from the MISRA PDF. Format:

    Appendix A Summary of guidelines
    Rule 1.1
    Rule text
    Rule 1.2
    Rule text
    ...

Rules that you want to disable does not need to have a rule text. Rules that don't have rule text will be suppressed by the addon.


## Library configuration

When external libraries are used, such as WinAPI, POSIX, gtk, Qt, etc, Cppcheck doesn't know how the external functions behave. Cppcheck then fails to detect various problems such as leaks, buffer overflows, possible null pointer dereferences, etc. But this can be fixed with configuration files.

Cppcheck already contains configurations for several libraries. They can be loaded as described below. Note that the configuration for the standard libraries of C and C++, std.cfg, is always loaded by cppcheck. If you create or update a configuration file for a popular library, we would appreciate if you upload it to us.



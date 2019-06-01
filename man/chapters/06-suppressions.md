
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


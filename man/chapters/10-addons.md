
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


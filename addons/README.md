# Cppcheck addons

Addons are scripts designed to analyze Cppcheck dump files, ensuring compatibility with secure coding standards and identifying a variety of issues.

## Supported addons

+ [misra.py](https://github.com/danmar/cppcheck/blob/main/addons/misra.py) 
  This addon is designed to check compliance with the MISRA C 2012 guidelines, a specialized set of rules developed for embedded systems aimed at preventing questionable code practices. As this standard is proprietary, Cppcheck will only indicate the number of each violated rule (e.g., [c2012-21.3]) without displaying the error text. To access detailed descriptions of the violated rules, users need to create a text file with the MISRA rules and use the `--rule-texts` option when executing the script. Examples of such rule text files can be found in the [tests directory](https://github.com/danmar/cppcheck/blob/main/addons/test/misra/) on the Cppcheck GitHub repository.
+ [y2038.py](https://github.com/danmar/cppcheck/blob/main/addons/y2038.py) 
  Checks Linux kernel source for [year 2038 problem](https://en.wikipedia.org/wiki/Year_2038_problem) safety. This requires [modified environment](https://github.com/3adev/y2038). See complete description [here](https://github.com/danmar/cppcheck/blob/main/addons/doc/y2038.txt).
+ [threadsafety.py](https://github.com/danmar/cppcheck/blob/main/addons/threadsafety.py) 
  This addon analyses Cppcheck dump files to identify thread safety issues, such as the use of static local objects by multiple threads.
+ [naming.py](https://github.com/danmar/cppcheck/blob/main/addons/naming.py)
  This addon enforces naming conventions across the code.
+ [namingng.py](https://github.com/danmar/cppcheck/blob/main/addons/namingng.py)
  This addon enforces naming conventions across the code. Enhanced version with support for type prefixes in variable and function names, fine grained control of allowed and forbidden patterns, file naming, include guard naming, presence and validity.
+ [findcasts.py](https://github.com/danmar/cppcheck/blob/main/addons/findcasts.py)
  Locates casts in the code.
+ [misc.py](https://github.com/danmar/cppcheck/blob/main/addons/misc.py)
  Performs miscellaneous checks.

### Other files

- doc
  Additional files for documentation generation.
- tests
  Contains various unit tests for the addons.
- cppcheck.py
  Internal helper used by Cppcheck binary to run the addons.
- cppcheckdata.doxyfile
  Configuration file for documentation generation.
- cppcheckdata.py
  Helper class for reading Cppcheck dump files within an addon.
- misra\_9.py
  Implementation of the MISRA 9.x rules used by `misra` addon. 
- namingng.config.json
  Example configuration for `namingng` addon. 
- namingng.json
  Example JSON file that can be used using --addon=namingng.json, referring to namingng.py and namingng.config.json
- ROS\_naming.json
  Example configuration for the `namingng` addon enforcing the [ROS naming convention for C++ ](http://wiki.ros.org/CppStyleGuide#Files).
- runaddon.py
  Internal helper used by Cppcheck binary to run the addons.

## Usage

### Command line interface

```bash
cppcheck --addon=misc src/test.c
```

It is also possible to call scripts as follows:
```bash
cppcheck --dump --quiet src/test.c
python misc.py src/test.c.dump
python misra.py --rule-texts=~/misra_rules.txt src/test.c.dump
```

This allows you to add additional parameters when calling the script (for example, `--rule-texts` for `misra.py`). The full list of available parameters can be found by calling any script with the `--help` flag.

### GUI

When using the graphical interface `cppcheck-gui`, the selection and configuration of addons is carried out on the tab `Addons and tools` in the project settings (`Edit Project File`):

![Screenshot](https://raw.githubusercontent.com/danmar/cppcheck/main/addons/doc/img/cppcheck-gui-addons.png)


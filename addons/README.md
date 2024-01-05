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

Addons can be used in three ways:
- Using the `--addon` argument to cppcheck
- Standalone, working on cppcheck dump files
- In the GUI `cppcheck-gui`

### Using the --addon argument to cppcheck

There are several ways to specify an addon on the commandline of cppcheck:
- `--addon=misc` to make cppcheck look for `misc.py` as in next bullet
- `--addon=misc.py` to make cppcheck look for `misc.py` in the current directory and the addon directory
- `--addon=rela/file.py` to load `file.py` from a relative path `./rela/`
- `--addon=/abs/file.py` to load `file.py` from an absolute path `/abs/`
- `--addon=misc.json` to load `misc.json` in the current directory, and [parse as JSON](#JSON-specification)
- `--addon=rela/misc.json` to load `misc.json` from a relative path `./rela/misc.json`, and [parse as JSON](#JSON-specification)
- `--addon=/abs/misc.json` to load `misc.json` from an absolute path `/abs/misc.json`, and [parse as JSON](#JSON-specification)
- `--addon={"script":"misc.py"}` to [parse the JSON](#JSON-specification) given as the argument

The addon directory is `addons/` below the directory where `cppcheck` resides, i.e.

```bash
ADDONS=$(dirname $(which cppcheck))/addons/
echo $ADDONS
```

#### JSON specification

In case JSON is used to specify the addon, the JSON should be structured as follows:

```json
{
    "script":"namingng.py",
    "args":[
        "--configfile=namingng.config.json"
    ]
}
```

### Standalone

To run an addon in standalone mode, first create a dump file, then give it as an argument to one or more addon scripts:

```bash
cppcheck --dump test.c
ADDONS=$(dirname $(which cppcheck))/addons/
$ADDONS/namingng.py test.c.dump
$ADDONS/misc.py test.c.dump
$ADDONS/misra.py --rule-texts=~/misra_rules.txt test.c.dump
```

This allows you to add additional parameters when calling the script (for example, `--rule-texts` for `misra.py`). Note that [using JSON](#JSON-specification) this is also possible with addons specified on the cppcheck commandline, by adding arguments to the addon to the `args` list. The full list of available parameters can be found by calling any script with the `--help` flag.

### GUI

When using the graphical interface `cppcheck-gui`, the selection and configuration of addons is carried out on the tab `Addons and tools` in the project settings (`Edit Project File`):

![Screenshot](https://raw.githubusercontent.com/danmar/cppcheck/main/addons/doc/img/cppcheck-gui-addons.png)


## Configuring addons

Addons can have configuration options, which are described below.

### Configuring namingng.py

The `namingng.py` addon can be invoked using the methods described above, e.g. utilizing the example `namingng.json` file, referencing the `namingng.config.json` config file:

```bash
ADDONS=$(dirname $(which cppcheck))/addons/
cp $ADDONS/namingng.json .              # copy JSON addon file
cp $ADDONS/namingng.config.json .       # copy JSON addon config file
cppcheck --addon=namingng.json --enable=all test.c
```

Or in standalone mode, using:

```bash
ADDONS=$(dirname $(which cppcheck))/addons/
cp $ADDONS/namingng.config.json .       # copy JSON addon config file
cppcheck --dump test.c
$ADDONS/namingng.py test.c.dump
```

Both commands utilize the example `namingng.config.json` config file. Customize the config file to meet your project's specific requirements.

The config file, structured as a JSON object, allows you to set rules for:
- files and directories (`RE_FILE`)
- include guards (including checking presence and validity) (`include_guard`)
- namespaces (`RE_NAMESPACE`)
- variables (global, local, private/public members) (`RE_GLOBAL_VARNAME`, `RE_VARNAME`, `RE_PRIVATE_MEMBER_VARIABLE`, `RE_PUBLIC_MEMBER_VARIABLE`)
- functions (`RE_FUNCTIONNAME`)
- classes (`RE_CLASS_NAME`)

Optional rules include function and local variable name prefixes (`function_prefixes` and `variable_prefixes`), and the ability to skip one-character variable names (`skip_one_char_variables`). Removing any option from the config file disables that specific check.

Options starting with `RE_` accept either a list or a dictionary:
- A list contains regular expressions that must all match.
- A dictionary contains key-value pairs, where the key is a regular expression, and the value is an array `[bool,string]`. The boolean indicates what match result is to be reported as an error, and string a message that is appended to the reported error.

Regular expressions follow Pythons `re` module syntax, which has the following specifics:
- No opening/closing delimiter such as `/`, so `/` need not be escaped.
- Implied `^` for start-of-string, but not `$` for end-of-string.
- `\Z` matches end of string (written as `\\Z` due to escaping)
- `$` matches end of string, or newline.

`RE_FILE` can only be a list. The file check fails for a file if no pattern matches either the basename or the full path. The full path is either an absolute or relative path, depending on the build setup. This can be used to specify different rules for different paths, e.g.

```
    "RE_FILE":[
        "/.*",                                  // matches any absolute path, typically includes internal to libraries
        "Drivers/.*",                           // matches anything under Drivers/, containing boilerplate code
        "Core/Src/[a-z][a-z0-9_]+.c\\Z",        // ensures all application C code is under Core/Src/ with lowercase, numbers and underscore, starting with [a-z]
        "Core/Inc/[a-z][a-z0-9_]+.h\\Z",        // ensures all application headers are under Core/Src/ with lowercase, numbers and underscore, starting with [a-z]
        "Modules/[a-z]+/",                      // ensures that any module is included with a simple lowercase string
        "[a-z0-9_]*.inc\\Z",                    // suffix .inc is always acceptable, anywhere in the project
    ],
```

These configurations not only enforce file naming conventions but also implement rules regarding the placement of various files in your project.

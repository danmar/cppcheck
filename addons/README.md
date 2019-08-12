# Cppcheck addons

Addons are scripts that analyses Cppcheck dump files to check compatibility with secure coding standards and to locate various issues.

## Supported addons

+ [cert.py](https://github.com/danmar/cppcheck/blob/master/addons/cert.py) 
  Checks for compliance with the safe programming standard [CERT](http://www.cert.org/secure-coding/).
+ [misra.py](https://github.com/danmar/cppcheck/blob/master/addons/misra.py) 
  Used to verify compliance with MISRA C 2012 - a proprietary set of guidelines to avoid such questionable code, developed for embedded systems. Since this standard is proprietary, cppcheck does not display error text by specifying only the number of violated rules (for example, [c2012-21.3]). If you want to display full texts for violated rules, you will need to create a text file containing MISRA rules, which you will have to pass when calling the script with `--rule-texts` key. Some examples of rule texts files available in [tests directory](https://github.com/danmar/cppcheck/blob/master/addons/test/misra/).
+ [y2038.py](https://github.com/danmar/cppcheck/blob/master/addons/y2038.py) 
  Checks Linux system for [year 2038 problem](https://en.wikipedia.org/wiki/Year_2038_problem) safety. This required [modified environment](https://github.com/3adev/y2038). See complete description [here](https://github.com/danmar/cppcheck/blob/master/addons/doc/y2038.txt).
+ [threadsafety.py](https://github.com/danmar/cppcheck/blob/master/addons/threadsafety.py) 
  Analyse Cppcheck dump files to locate threadsafety issues like static local objects used by multiple threads.

## Usage

### Command line interface

```bash
cppcheck --addon=cert --addon=y2038 src/test.c
```

It is also possible to call scripts as follows:
```bash
cppcheck --dump --quiet src/test.c
python cert.py src/test.c.dump
python misra.py --rules-texts=~/misra_rules.txt src/test.c.dump
```

This allows you to add additional parameters when calling the script (for example, `--rule-tests` for `misra.py`). The full list of available parameters can be found by calling any script with the `--help` flag.

### GUI

When using the graphical interface `cppcheck-gui`, the selection and configuration of addons is carried out on the tab `Addons and tools` in the project settings (`Edit Project File`):

[](https://raw.githubusercontent.com/danmar/cppcheck/master/addons/doc/img/cppcheck-gui-addons.png)

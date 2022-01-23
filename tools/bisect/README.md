# Bisecting

NOTE: THIS IS WORK IN PROGRESS

`bisect.sh` is a script to bisect issues.

## Command

```
./bisect.sh <hash-good> <hash-bad> "<cppcheck-options>"
```

`hash-good` - the last known good commit hash - in case of daca it is the last tagged minor release (not patch release - i.e. 2.x)
`hash-bad` - the known bad commit hash - in case of daca the one from the `head-info:` line
`cppcheck-options` - the options for the Cppcheck invokation - in case of daca the ones from the `cppcheck-options:` line and the path to the folder/file to scan

If possible use `main` as the function to test stuff with since it won't emit an `unusedFunction` warning.

## Bisecting scan time regressions

We use daca to track differences in scan time. An overview of regressions in scan time can be found at http://cppcheck1.osuosl.org:8000/time_gt.html.

You need to download the archive as specified by the second line in the output and extract it.

If the overall scan time regressed you need to specify the whole folder.

If a timeout (potential hang) was introduced you can simply specify the file from `error: Internal error: Child process crashed with signal 15 [cppcheckError]`.


## Bisecting result regressions

Results regressions are being bisected based on the `--error-exitcode=` result.
If nothing is found the result will be `0` and it is treated as a _good_ commit.
If a finding occurs the result will be `1` which is treated as a _bad_ commit.

### False positive

Provide a code sample which will trigger the false postive.

```cpp
// cppcheck-suppress unusedFunction
static void f()
{
    <code triggering FP>
}
```

### False negative

Provide a code sample which will trigger a `unmatchedSuppression`.

```cpp
// cppcheck-suppress unusedFunction
static void f()
{
    // cppcheck-suppress unreadVariable
    int i;
}
```

## Notes

### Compilation issues:

- 2.5 and before can only be built with GCC<=10 because of missing includes caused by cleanups within the standard headers. You need to specify `CXX=g++-10`.
- 1.88 and 1.89 cannot be compiled:
```
make: python: No such file or directory
```
- 1.39 to 1.49 (possibly more versions - 1.54 and up work) cannot be compiled:
```
lib/mathlib.cpp:70:42: error: invalid conversion from ‘char’ to ‘char**’ [-fpermissive]
   70 |         return std::strtoul(str.c_str(), '\0', 16);
      |                                          ^~~~
      |                                          |
      |                                          char
```
- some commits between 2.0 and 2.2 cannot be compiled:
```
cli/cppcheckexecutor.cpp:333:22: error: size of array ‘mytstack’ is not an integral constant-expression
  333 | static char mytstack[MYSTACKSIZE]= {0}; // alternative stack for signal handler
  |                      ^~~~~~~~~~~
```
RESOLVED: a hot-patch is applied before compilation.
- some commits between 1.54 and 1.55 cannot be compiled:
```
lib/preprocessor.cpp:2103:5: error: ‘errorLogger’ was not declared in this scope; did you mean ‘_errorLogger’?
2103 |     errorLogger->reportInfo(errmsg);
|     ^~~~~~~~~~~
|     _errorLogger
```
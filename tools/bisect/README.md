# Bisecting

`bisect.sh` is a script to bisect regressions in Cppcheck utilizing `git-bisect`.

To learn more about bisecting please refer to https://git-scm.com/docs/git-bisect. 

## Command

```
./bisect.sh <hash-good> <hash-bad> "<cppcheck-options>" "[expected]"
```

`hash-good` the latest known good commit hash or tag<br/>
`hash-bad` the earliest known bad commit hash or tag<br/>
`cppcheck-options` the options for the Cppcheck invokatio<br/>
`expected` (optional) a string that is expected in the output. Will be used instead of the exitcode

If possible use `main` as the function to test stuff with since it won't emit an `unusedFunction` warning.

## Bisecting result regressions

Results regressions are being bisected based on the `--error-exitcode=` result.

If nothing is found the result will be `0` and it is treated as a _good_ commit.<br/>
If a finding occurs the result will be `1` which is treated as a _bad_ commit.<br/>
If a crash occurs it is treated as a _bad_ commit.

You can also bisect based on expected output via the `expected` parameter.

If the given string is found in the output it is treated as a _good_ commit.<br/>
If the given string is _not_ found in the output it is treated as a _bad_ commit.<br/>
If a crash occurs it is treated as a _bad_ commit.

### False positive

Provide a code sample which will trigger a single(!) false postive only. Trying to bisect multiple issues at the same time will most likely result in an incorrect result (see below).

```cpp
// cppcheck-suppress unusedFunction
static void f()
{
    <code triggering FP>
}
```

```
./bisect.sh <hash-good> <hash-bad> "<cppcheck-options>"
```

After the bisecting check the output to make sure that only expected false positive and no additional finding was reported for the _bad_ commits. Any other finding will also cause the commit to be marked as _bad_ leading to an incorrect result.  

### False negative

#### Via suppression

Provide a code sample which will trigger a `unmatchedSuppression`.

```cpp
// cppcheck-suppress unusedFunction
static void f()
{
    // cppcheck-suppress unreadVariable
    int i;
}
```

```
./bisect.sh <hash-good> <hash-bad> "<cppcheck-options>"
```

#### Via output

```cpp
static void f()
{
    int i;
}
```

Provide the expected error ID (`unreadVariable`) as the `expected` parameter.

```
./bisect.sh <hash-good> <hash-bad> "<cppcheck-options>" "unreadVariable"
```

## Bisecting scan time regressions

We use daca@home to track differences in scan time. An overview of regressions in scan time can be found at http://cppcheck1.osuosl.org:8000/time_gt.html.

You need to download the archive as specified by the second line in the output and extract it.

If the overall scan time regressed you need to specify the whole folder.

If a timeout (potential hang) was introduced you can simply specify the file from `error: Internal error: Child process crashed with signal 15 [cppcheckError]`.

## Notes

### Bisecting daca@home issues

Use the following data as respective parameters:

`hash-good` the latest tagged release - the second value from the `cppcheck:` line<br/>
`hash-bad` the commit hash from the `head-info:` line<br/>
`cppcheck-options` the `cppcheck-options:` line and the path to the folder/file to scan<br/>

### Known compilation issues:

- 2.5 and before can only be built with GCC<=10 because of missing includes caused by cleanups within the standard headers. You need to specify `CXX=g++-10`.
- 1.88 and 1.89 cannot be compiled:
```
make: python: No such file or directory
```
RESOLVED: a hot-patch is applied before compilation.
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
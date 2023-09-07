# Compiler defines

NOTE: this is preliminary.

This folder contains scripts to extract compiler-specific defines and definitions for use with a Cppcheck scan.

Specifying these will get rid of `` warnings (which are reported when `--debug-warnings` is specified) which allows for increased coverage of the ValueFlow.

## Files

- `define.sh`
Invokes the compilation and execution of the various programs which output the compiler-specific defines.
- `float.c`
Outputs the defines provided by `float.h`/`cfloat`.
- `limits.c`
Outputs the defines provided by `limits.h`/`climits`.
- `stdint.c`
Outputs the defines provided by `stdint.h`/`cstdint`.
# Cppcheck Safety Notes

## Process Stability

By default the analysis is performed in the main process. So if you encounter an assertion or crash bug within Cppcheck the analysis will abort prematurely.

If you use `-j<n>` a new process (up to the amount of `n`) will be spawned for each file which is analyzed (currently only available on non-Windows platforms - see https://trac.cppcheck.net/ticket/12464). This will make sure the analysis will always finish and a result is being provided. Assertion and crash fixes will be provided as `cppcheckError` findings. If you do not want to spawn process you can use `--executor=thread` to use threads instead.

If you encountered any such bugs please report them at https://trac.cppcheck.net/.

## Code Execution

The analyzed code will only be parsed and/or compiled but no executable code is being generated. So there is no risk in analyzing code containing known exploits as it will never be executed.

## Process Elevation

No process elevation is being performed for any of the spawned processes. They currently use the same permissions as the main process.

See https://trac.cppcheck.net/ticket/14237 about a poetntial future improvement by reducing the permissions of spawned processes.

## Invoked executables

By default no additional external executables will be invoked by the analysis.

But there are some options which will utilize additional executables.

`python`/`python3` - Used for the execution of addons. Trigger by using the `--addon=<addon>` option and specifying addons in `cppcheck.cfg`.
The executable will be looked for in `PATH` by default and can be configured by TODO.

`clang-tidy` - Used for invoking an additional analysis via Clang-Tidy. Triggered by the `--clang-tidy` option.
The executable will be looked for in `PATH` by default. A versioned executable or absolute path can be specified by using `--clang-tidy=<exe>`.

`clang` - Used to generate a CLang AST instead of an internal one. Triggered by the `--clang` option.
The executable will be looked for in `PATH` by default. A versioned executable or absolute path can be specified by using `--clang=<exe>`.

## Output Files

By default no files will be written.

But there are some options which will cause files to be written.

`--cppcheck-build-dir=<dir>` - TODO
`--dump` - TODO
`--plist-output` - TODO
TODO: more options to add?

## Required Permissions

By default you only need the permissions to read the files specified for analysis and the specified include paths. No administrator permissions should be necessary to perform the analysis.
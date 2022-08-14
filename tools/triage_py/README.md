# triage.py

A script to run a code sample against a given set of Cppcheck versions.

## Usage

```
usage: triage.py [-h] [--compare] [--verbose] dir infile [repo]

positional arguments:
  dir         directory with versioned folders
  infile      the file to analyze
  repo        the git repository (for sorting commit hashes)

options:
  -h, --help  show this help message and exit
  --compare   compare output and only show when changed
  --verbose   verbose output for debugging
```

### Structure of `dir`

It expects the given `dir` to contain folders which are named after the containg version or commit hash. These folder must contain a `cppcheck` and the associated files for that version.

If the first folder is not a valid version the names are interpreted as commit hashes. These are internally sorted but that requires the `repo` parameter to be set.

It is not possible to mix versions and commit hashes.
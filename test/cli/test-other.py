
# python -m pytest test-other.py

import os
import pytest

from testutils import cppcheck


def __test_missing_include(tmpdir, use_j):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                #include "test.h"
                """)

    args = ['--enable=missingInclude', '--template={file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]', test_file]
    if use_j:
        args.insert(0, '-j2')

    _, _, stderr = cppcheck(args)
    assert stderr == '{}:2:0: information: Include file: "test.h" not found. [missingInclude]\n'.format(test_file)


def test_missing_include(tmpdir):
    __test_missing_include(tmpdir, False)


def test_missing_include_j(tmpdir): #11283
    __test_missing_include(tmpdir, True)


def __test_missing_include_check_config(tmpdir, use_j):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                #include "test.h"
                """)

    # TODO: -rp is not working requiring the full path in the assert
    args = '--check-config -rp={} {}'.format(tmpdir, test_file)
    if use_j:
        args = '-j2 ' + args

    _, _, stderr = cppcheck(args.split())
    assert stderr == '' # --check-config no longer reports the missing includes


def test_missing_include_check_config(tmpdir):
    __test_missing_include_check_config(tmpdir, False)


def test_missing_include_check_config_j(tmpdir):
    __test_missing_include_check_config(tmpdir, True)


def test_missing_include_inline_suppr(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                // cppcheck-suppress missingInclude
                #include "missing.h"
                // cppcheck-suppress missingIncludeSystem
                #include <missing2.h>
                """)

    args = ['--enable=missingInclude', '--inline-suppr', test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == ''


def test_invalid_library(tmpdir):
    args = ['--library=none', '--library=posix', '--library=none2', '--platform=native', 'file.c']

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 1
    assert (stdout == "cppcheck: Failed to load library configuration file 'none'. File not found\n"
                      "cppcheck: Failed to load library configuration file 'none2'. File not found\n")
    assert stderr == ""


def test_message_j(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("")

    args = ['-j2', '--platform=native', test_file]

    _, stdout, _ = cppcheck(args)
    assert stdout == "Checking {} ...\n".format(test_file) # we were adding stray \0 characters at the end

# TODO: test missing std.cfg


def test_progress(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                }
                """)

    args = ['--report-progress=0', '--enable=all', '--inconclusive', '--platform=native', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    pos = stdout.find('\n')
    assert(pos != -1)
    pos += 1
    assert stdout[:pos] == "Checking {} ...\n".format(test_file)
    assert (stdout[pos:] ==
            "progress: Tokenize (typedef) 0%\n"
            "progress: Tokenize (typedef) 12%\n"
            "progress: Tokenize (typedef) 25%\n"
            "progress: Tokenize (typedef) 37%\n"
            "progress: Tokenize (typedef) 50%\n"
            "progress: Tokenize (typedef) 62%\n"
            "progress: Tokenize (typedef) 75%\n"
            "progress: Tokenize (typedef) 87%\n"
            "progress: SymbolDatabase 0%\n"
            "progress: SymbolDatabase 12%\n"
            "progress: SymbolDatabase 87%\n"
            )
    assert stderr == ""


def test_progress_j(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt') as f:
        f.write("""
                int main(int argc)
                {
                }
                """)

    args = ['--report-progress=0', '--enable=all', '--inconclusive', '-j2', '--disable=unusedFunction', '--platform=native', test_file]

    exitcode, stdout, stderr = cppcheck(args)
    assert exitcode == 0
    assert stdout == "Checking {} ...\n".format(test_file)
    assert stderr == ""

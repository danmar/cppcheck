
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


@pytest.mark.timeout(10)
def test_slow_array_many_floats(tmpdir):
    # 11649
    # cppcheck valueflow takes a long time when an array has many floats
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("const float f[] = {\n")
        for i in range(20000):
            f.write('    13.6f,\n')
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(10)
def test_slow_array_many_strings(tmpdir):
    # 11901
    # cppcheck valueflow takes a long time when analyzing a file with many strings
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("const char *strings[] = {\n")
        for i in range(20000):
            f.write('    "abc",\n')
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


def test_execute_addon_failure(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f();
                """)

    args = ['--addon=naming', test_file]

    # provide empty PATH environment variable so python is not found and execution of addon fails
    env = {'PATH': ''}
    _, _, stderr = cppcheck(args, env)
    assert stderr == '{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to auto detect python [internalError]\n\n^\n'.format(test_file)


def test_execute_addon_failure_2(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
                void f();
                """)

    # specify non-existent python executbale so execution of addon fails
    args = ['--addon=naming', '--addon-python=python5.x', test_file]

    _, _, stderr = cppcheck(args)
    # /tmp/pytest-of-sshuser/pytest-215/test_execute_addon_failure_20/test.cpp:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon 'naming' (command: 'python5.x /mnt/s/GitHub/cppcheck-fw/addons/runaddon.py /mnt/s/GitHub/cppcheck-fw/addons/naming.py --cli /tmp/pytest-of-sshuser/pytest-215/test_execute_addon_failure_20/test.cpp.7306.dump'). Exitcode is nonzero. [internalError]\n\n^\n
    # "C:\\Users\\Quotenjugendlicher\\AppData\\Local\\Temp\\pytest-of-Quotenjugendlicher\\pytest-15\\test_execute_addon_failure_20\\test.cpp:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon (command: 'python5.x S:\\GitHub\\cppcheck-fw\\bin\\debug\\addons\\runaddon.py S:\\GitHub\\cppcheck-fw\\bin\\debug\\addons\\naming.py --cli C:\\Users\\Quotenjugendlicher\\AppData\\Local\\Temp\\pytest-of-Quotenjugendlicher\\pytest-15\\test_execute_addon_failure_20\\test.cpp.9892.dump'). Exitcode is nonzero. [internalError]\n\n^\n
    assert stderr.startswith('{}:0:0: error: Bailing out from analysis: Checking file failed: Failed to execute addon \'naming\' (command: \'python5.x '.format(test_file))
    assert stderr.endswith(' [internalError]\n\n^\n')


# TODO: find a test case which always fails
@pytest.mark.skip
def test_internal_error(tmpdir):
    test_file = os.path.join(tmpdir, 'test.cpp')
    with open(test_file, 'wt') as f:
        f.write("""
#include <cstdio>

void f() {
    double gc = 3333.3333;
    char stat[80];
    sprintf(stat,"'%2.1f'",gc);
}
                """)

    args = [test_file]

    _, _, stderr = cppcheck(args)
    assert stderr == '{}:0:0: error: Bailing from out analysis: Checking file failed: converting \'1f\' to integer failed - not an integer [internalError]\n\n^\n'.format(test_file)
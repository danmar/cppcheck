
# python3 -m pytest test-unused_function_test.py

import os
import json
import sys
import pytest
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

__project_dir = os.path.join(__script_dir, 'unusedFunction')
__project_dir_sep = __project_dir + os.path.sep


# TODO: make this a generic helper function
def __create_compdb(tmpdir, projpath):
    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    files = []
    for f in os.listdir(projpath):
        files.append(f)
    files.sort()
    j = []
    for f in files:
        j.append({
            'directory': projpath,
            'file': os.path.join(projpath, f),
            'command': 'gcc -c {}'.format(f)
        })
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j, indent=4))
    return compile_commands


def test_unused_functions():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j1', __project_dir])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions_j():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j2', __project_dir])
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check requires --cppcheck-build-dir to be active with -j."
    ]
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_unused_functions_project():
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(os.path.join(__project_dir, 'unusedFunction.cppcheck')),
                                    '-j1'])
    assert stdout.splitlines() == []
    assert [
        "{}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ] == stderr.splitlines()
    assert ret == 0, stdout


def test_unused_functions_project_j():
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(os.path.join(__project_dir, 'unusedFunction.cppcheck')),
                                    '-j2'])
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check requires --cppcheck-build-dir to be active with -j."
    ]
    assert [] == stderr.splitlines()
    assert ret == 0, stdout


def test_unused_functions_compdb(tmpdir):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(compdb_file),
                                    '-j1'
                                    ])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions_compdb_j(tmpdir):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(compdb_file),
                                    '-j2'
                                    ])
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check requires --cppcheck-build-dir to be active with -j."
    ]
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def __test_unused_functions_builddir(tmpdir, extra_args):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    args = ['-q',
            '--template=simple',
            '--enable=unusedFunction',
            '--inline-suppr',
            '--cppcheck-build-dir={}'.format(build_dir),
            __project_dir
            ]
    args += extra_args
    ret, stdout, stderr = cppcheck()
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions_builddir(tmpdir):
    __test_unused_functions_builddir(tmpdir, ['-j1'])


@pytest.mark.xfail(strict=True)
def test_unused_functions_builddir_j(tmpdir):
    __test_unused_functions_builddir(tmpdir, ['-j2'])

@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_unused_functions_builddir_j_process(tmpdir):
    __test_unused_functions_builddir(tmpdir, ['-j2', '--executor=process'])


def __test_unused_functions_builddir_project(tmpdir, extra_args):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    args = ['-q',
            '--template=simple',
            '--enable=unusedFunction',
            '--inline-suppr',
            '--project={}'.format(os.path.join(__project_dir, 'unusedFunction.cppcheck')),
            '--cppcheck-build-dir={}'.format(build_dir)]
    args += extra_args
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions_builddir_project(tmpdir):
    __test_unused_functions_builddir_project(tmpdir, ['-j1'])


@pytest.mark.xfail(strict=True)
def test_unused_functions_builddir_project_j(tmpdir):
    __test_unused_functions_builddir_project(tmpdir, ['-j2'])

@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_unused_functions_builddir_project_j_process(tmpdir):
    __test_unused_functions_builddir_project(tmpdir, ['-j2', '--executor=process'])

def __test_unused_functions_builddir_compdb(tmpdir, extra_args):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    args = ['-q',
            '--template=simple',
            '--enable=unusedFunction',
            '--inline-suppr',
            '--project={}'.format(compdb_file),
            '--cppcheck-build-dir={}'.format(build_dir)
            ]
    args += extra_args
    ret, stdout, stderr = cppcheck(args)
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(__project_dir_sep)
    ]
    assert ret == 0, stdout


def test_unused_functions_builddir_compdb(tmpdir):
    __test_unused_functions_builddir_compdb(tmpdir, ['-j1'])


@pytest.mark.xfail(strict=True)
def test_unused_functions_builddir_compdb_j(tmpdir):
    __test_unused_functions_builddir_compdb(tmpdir, ['-j2'])

@pytest.mark.skipif(sys.platform == 'win32', reason='ProcessExecutor not available on Windows')
def test_unused_functions_builddir_compdb_j_process(tmpdir):
    __test_unused_functions_builddir_compdb(tmpdir, ['-j2', '--executor=process'])

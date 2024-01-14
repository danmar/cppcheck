
# python3 -m pytest test-unused_function_test.py

import os
import json
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

PROJECT_DIR = os.path.join(__script_dir, 'unusedFunction')


# TODO: make this a generic helper function
def __create_compdb(tmpdir, projpath):
    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    j = [
        {
            'directory': projpath,
            'file': os.path.join(projpath, '1.c'),
            'command': 'gcc -c 1.c'
        },
        {
            'directory': projpath,
            'file': os.path.join(projpath, '2.c'),
            'command': 'gcc -c 2.c'
        },
        {
            'directory': projpath,
            'file': os.path.join(projpath, '3.c'),
            'command': 'gcc -c 3.c'
        }
        ]
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j, indent=4))
    return compile_commands


def test_unused_functions():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', PROJECT_DIR])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


def test_unused_functions_j():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j2', PROJECT_DIR])
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check can't be used with '-j' option. Disabling unusedFunction check."
    ]
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_unused_functions_project():
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(os.path.join(PROJECT_DIR, 'unusedFunction.cppcheck'))])
    assert stdout.splitlines() == []
    assert [
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ] == stderr.splitlines()
    assert ret == 0, stdout


def test_unused_functions_project_j():
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(os.path.join(PROJECT_DIR, 'unusedFunction.cppcheck')),
                                    '-j2'])
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check can't be used with '-j' option. Disabling unusedFunction check."
    ]
    assert [] == stderr.splitlines()
    assert ret == 0, stdout


def test_unused_functions_compdb(tmpdir):
    compdb_file = __create_compdb(tmpdir, PROJECT_DIR)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(compdb_file)
                                    ])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


def test_unused_functions_compdb_j(tmpdir):
    compdb_file = __create_compdb(tmpdir, PROJECT_DIR)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(compdb_file),
                                    '-j2'
                                    ])
    assert stdout.splitlines() == [
        "cppcheck: unusedFunction check can't be used with '-j' option. Disabling unusedFunction check."
    ]
    assert stderr.splitlines() == []
    assert ret == 0, stdout


def test_unused_functions_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '--cppcheck-build-dir={}'.format(build_dir), PROJECT_DIR])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


# TODO: only f3_3 is unused
def test_unused_functions_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j2', '--cppcheck-build-dir={}'.format(build_dir), PROJECT_DIR])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/1.c:4:0: style: The function 'f1' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/2.c:4:0: style: The function 'f2' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


def test_unused_functions_builddir_project(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(os.path.join(PROJECT_DIR, 'unusedFunction.cppcheck')),
                                    '--cppcheck-build-dir={}'.format(build_dir)])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


# TODO: only f3_3 is unused
def test_unused_functions_builddir_project_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(os.path.join(PROJECT_DIR, 'unusedFunction.cppcheck')),
                                    '--cppcheck-build-dir={}'.format(build_dir),
                                    '-j2'])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/1.c:4:0: style: The function 'f1' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/2.c:4:0: style: The function 'f2' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


def test_unused_functions_builddir_compdb(tmpdir):
    compdb_file = __create_compdb(tmpdir, PROJECT_DIR)
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(compdb_file),
                                    '--cppcheck-build-dir={}'.format(build_dir)
                                    ])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


# TODO: only f3_3 is unused
def test_unused_functions_builddir_compdb_j(tmpdir):
    compdb_file = __create_compdb(tmpdir, PROJECT_DIR)
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(compdb_file),
                                    '--cppcheck-build-dir={}'.format(build_dir),
                                    '-j2'
                                    ])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        "{}/1.c:4:0: style: The function 'f1' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/2.c:4:0: style: The function 'f2' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout

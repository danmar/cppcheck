# python3 -m pytest test-unused_function_test.py

import os
import json
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
            'command': f'gcc -c {f}'
        })
    with open(compile_commands, 'w') as f:
        f.write(json.dumps(j, indent=4))
    return compile_commands


def test_unused_functions():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j1', __project_dir])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


def test_unused_functions_j():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j2', __project_dir])
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
                                    '--project={}'.format(os.path.join(__project_dir, 'unusedFunction.cppcheck')),
                                    '-j1'])
    assert stdout.splitlines() == []
    assert [
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]"
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
        "cppcheck: unusedFunction check can't be used with '-j' option. Disabling unusedFunction check."
    ]
    assert [] == stderr.splitlines()
    assert ret == 0, stdout


def test_unused_functions_compdb(tmpdir):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    f'--project={compdb_file}',
                                    '-j1'
                                    ])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


def test_unused_functions_compdb_j(tmpdir):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    f'--project={compdb_file}',
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
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j1', f'--cppcheck-build-dir={build_dir}', __project_dir])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


# TODO: only f3_3 is unused
def test_unused_functions_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--enable=unusedFunction', '--inline-suppr', '-j2', f'--cppcheck-build-dir={build_dir}', __project_dir])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}1.c:4:0: style: The function 'f1' is never used. [unusedFunction]",
        f"{__project_dir_sep}2.c:4:0: style: The function 'f2' is never used. [unusedFunction]",
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]",
        f"{__project_dir_sep}4.c:4:0: style: The function 'f4_1' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


def test_unused_functions_builddir_project(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    '--project={}'.format(os.path.join(__project_dir, 'unusedFunction.cppcheck')),
                                    f'--cppcheck-build-dir={build_dir}',
                                    '-j1'])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]"
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
                                    '--project={}'.format(os.path.join(__project_dir, 'unusedFunction.cppcheck')),
                                    f'--cppcheck-build-dir={build_dir}',
                                    '-j2'])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}1.c:4:0: style: The function 'f1' is never used. [unusedFunction]",
        f"{__project_dir_sep}2.c:4:0: style: The function 'f2' is never used. [unusedFunction]",
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]",
        f"{__project_dir_sep}4.c:4:0: style: The function 'f4_1' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


def test_unused_functions_builddir_compdb(tmpdir):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    f'--project={compdb_file}',
                                    f'--cppcheck-build-dir={build_dir}',
                                    '-j1'
                                    ])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


# TODO: only f3_3 is unused
def test_unused_functions_builddir_compdb_j(tmpdir):
    compdb_file = __create_compdb(tmpdir, __project_dir)
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['-q',
                                    '--template=simple',
                                    '--enable=unusedFunction',
                                    '--inline-suppr',
                                    f'--project={compdb_file}',
                                    f'--cppcheck-build-dir={build_dir}',
                                    '-j2'
                                    ])
    assert stdout.splitlines() == []
    assert stderr.splitlines() == [
        f"{__project_dir_sep}1.c:4:0: style: The function 'f1' is never used. [unusedFunction]",
        f"{__project_dir_sep}2.c:4:0: style: The function 'f2' is never used. [unusedFunction]",
        f"{__project_dir_sep}3.c:3:0: style: The function 'f3_3' is never used. [unusedFunction]",
        f"{__project_dir_sep}4.c:4:0: style: The function 'f4_1' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout

import os
import pytest
import json
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

# TODO: use dedicated addon
# TODO: test CheckNullPointer
# TODO: test CheckUninitVar
# TODO: test CheckBufferOverrun


def __create_compile_commands(dir, entries):
    j = []
    for e in entries:
        f = os.path.basename(e)
        obj = {
            'directory': os.path.dirname(os.path.abspath(e)),
            'command': 'gcc -c {}'.format(f),
            'file': f
        }
        j.append(obj)
    compile_commands = os.path.join(dir, 'compile_commmands.json')
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j))
    return compile_commands


def __test_addon_suppress_inline(extra_args):
    args = [
        '-q',
        '--addon=misra',
        '--template=simple',
        '--enable=information,style',
        '--disable=missingInclude',  # TODO: remove
        '--inline-suppr',
        '--error-exitcode=1',
        'whole-program/whole1.c',
        'whole-program/whole2.c'
    ]
    args += extra_args
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_addon_suppress_inline():
    __test_addon_suppress_inline(['-j1'])


# TODO: inline suppressions currently do not work with whole program analysis and addons - see #12835
# whole program analysis requires a build dir with -j
@pytest.mark.xfail(strict=True)
def test_addon_suppress_inline_j():
    __test_addon_suppress_inline(['-j2'])


def test_addon_suppress_inline_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_addon_suppress_inline(['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


# TODO: inline suppressions currently do not work with whole program analysis and addons - see #12835
@pytest.mark.xfail(strict=True)
def test_addon_suppress_inline_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_addon_suppress_inline(['-j2', '--cppcheck-build-dir={}'.format(build_dir)])


def __test_addon_suppress_inline_project(tmpdir, extra_args):
    compile_db = __create_compile_commands(tmpdir, [
        os.path.join(__script_dir, 'whole-program', 'whole1.c'),
        os.path.join(__script_dir, 'whole-program', 'whole2.c')
    ])

    args = [
        '-q',
        '--addon=misra',
        '--template=simple',
        '--enable=information,style',
        '--disable=missingInclude',  # TODO: remove
        '--inline-suppr',
        '--error-exitcode=1',
        '--project={}'.format(compile_db)
    ]
    args += extra_args
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_addon_suppress_inline_project(tmpdir):
    __test_addon_suppress_inline_project(tmpdir, ['-j1'])


# TODO: inline suppressions currently do not work with whole program analysis and addons - see #12835
# whole program analysis requires a build dir with -j
@pytest.mark.xfail(strict=True)
def test_addon_suppress_inline_project_j(tmpdir):
    __test_addon_suppress_inline_project(tmpdir, ['-j2'])


def test_addon_suppress_inline_project_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_addon_suppress_inline_project(tmpdir, ['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


# TODO: inline suppressions currently do not work with whole program analysis and addons - see #12835
@pytest.mark.xfail(strict=True)
def test_addon_suppress_inline_project_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_addon_suppress_inline_project(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir)])


def __test_suppress_inline(extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--disable=missingInclude',  # TODO: remove
        '--inline-suppr',
        '--error-exitcode=1',
        'whole-program/odr1.cpp',
        'whole-program/odr2.cpp'
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_suppress_inline():
    __test_suppress_inline(['-j1'])


# TODO: inline suppressions do not work with whole program analysis and -j
# whole program analysis requires a build dir with -j
@pytest.mark.xfail(strict=True)
def test_suppress_inline_j():
    __test_suppress_inline(['-j2'])


def test_suppress_inline_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_suppress_inline(['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


# TODO: inline suppressions do not work with whole program analysis and -j
@pytest.mark.xfail(strict=True)
def test_suppress_inline_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_suppress_inline(['-j2', '--cppcheck-build-dir={}'.format(build_dir)])


def __test_suppress_inline_project(tmpdir, extra_args):
    compile_db = __create_compile_commands(tmpdir, [
        os.path.join(__script_dir, 'whole-program', 'odr1.cpp'),
        os.path.join(__script_dir, 'whole-program', 'odr2.cpp')
    ])

    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--disable=missingInclude',  # TODO: remove
        '--inline-suppr',
        '--error-exitcode=1',
        '--project={}'.format(compile_db)
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_suppress_inline_project(tmpdir):
    __test_suppress_inline_project(tmpdir, ['-j1'])


# whole program analysis requires a build dir with -j
@pytest.mark.xfail(strict=True)
def test_suppress_inline_project_j(tmpdir):
    __test_suppress_inline_project(tmpdir, ['-j2'])


def test_suppress_inline_project_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_suppress_inline_project(tmpdir, ['-j1', '--cppcheck-build-dir={}'.format(build_dir)])

# TODO: inline suppressions do not work with whole program analysis and -j
@pytest.mark.xfail(strict=True)
def test_suppress_inline_project_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_suppress_inline_project(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir)])


def __test_checkclass(extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--disable=missingInclude',  # TODO: remove
        '--error-exitcode=1',
        'whole-program/odr1.cpp',
        'whole-program/odr2.cpp'
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        "whole-program{}odr1.cpp:6:1: error: The one definition rule is violated, different classes/structs have the same name 'C' [ctuOneDefinitionRuleViolation]".format(os.path.sep)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_checkclass():
    __test_checkclass(['-j1'])


# whole program analysis requires a build dir with -j
@pytest.mark.xfail(strict=True)
def test_checkclass_j():
    __test_checkclass(['-j2'])


def test_checkclass_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass(['--cppcheck-build-dir={}'.format(build_dir)])


def __test_checkclass_project(tmpdir, extra_args):
    odr_file_1 = os.path.join(__script_dir, 'whole-program', 'odr1.cpp')

    compile_db = __create_compile_commands(tmpdir, [
        odr_file_1,
        os.path.join(__script_dir, 'whole-program', 'odr2.cpp')
    ])

    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
        '--disable=missingInclude',  # TODO: remove
        '--error-exitcode=1',
        '--project={}'.format(compile_db)
    ]

    args += extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        "{}:6:1: error: The one definition rule is violated, different classes/structs have the same name 'C' [ctuOneDefinitionRuleViolation]".format(odr_file_1)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_checkclass_project(tmpdir):
    __test_checkclass_project(tmpdir, ['-j1'])


# whole program analysis requires a build dir with -j
@pytest.mark.xfail(strict=True)
def test_checkclass_project_j(tmpdir):
    __test_checkclass_project(tmpdir, ['-j2'])


def test_checkclass_project_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass_project(tmpdir, ['-j1', '--cppcheck-build-dir={}'.format(build_dir)])

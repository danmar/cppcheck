import glob
import os
import pytest
import json
import shutil
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

@pytest.mark.parametrize("builddir", (False,True))
def test_addon_rerun(tmp_path, builddir):
    """Rerun analysis and ensure that misra CTU works; with and without build dir"""
    args = [
        '--addon=misra',
        '--enable=style',
        '--template={id}',
        'whole-program']
    # do not use the injection because the directory needs to survive runs
    if builddir:
        args.append('--cppcheck-build-dir=' + str(tmp_path))
    else:
        args.append('--no-cppcheck-build-dir')
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr

def test_addon_builddir_use_ctuinfo(tmp_path):
    """Test that ctu-info files are used when builddir is used"""
    args = [
        '--cppcheck-build-dir=' + str(tmp_path),
        '--addon=misra',
        '--enable=style',
        '--template={id}',
        'whole-program']
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr
    with open(tmp_path / 'whole1.a1.ctu-info', 'wt'):
        pass
    with open(tmp_path / 'whole2.a1.ctu-info', 'wt'):
        pass
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' not in stderr

@pytest.mark.parametrize("builddir", (False,True))
def test_addon_no_artifacts(tmp_path, builddir):
    """Test that there are no artifacts left after analysis"""
    shutil.copyfile(os.path.join(__script_dir, 'whole-program', 'whole1.c'), tmp_path / 'whole1.c')
    shutil.copyfile(os.path.join(__script_dir, 'whole-program', 'whole2.c'), tmp_path / 'whole2.c')
    build_dir = str(tmp_path / 'b1')
    os.mkdir(build_dir)
    args = [
        '--addon=misra',
        '--enable=style',
        '--template={id}',
        str(tmp_path)]
    if builddir:
        args.append('--cppcheck-build-dir=' + build_dir)
    _, _, stderr = cppcheck(args, cwd=__script_dir)
    assert 'misra-c2012-5.8' in stderr
    files = []
    for f in glob.glob(str(tmp_path / '*')):
        if os.path.isfile(f):
            files.append(os.path.basename(f))
    files.sort()
    assert ' '.join(files) == 'whole1.c whole2.c'


def __test_checkclass(extra_args):
    args = [
        '-q',
        '--template=simple',
        '--enable=information,style',
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


@pytest.mark.xfail(strict=True) # TODO: check error
def test_checkclass_j():
    __test_checkclass(['-j2', '--no-cppcheck-build-dir'])


def test_checkclass_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass(['--cppcheck-build-dir={}'.format(build_dir)])


def test_checkclass_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass(['-j2', '--cppcheck-build-dir={}'.format(build_dir)])

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


@pytest.mark.xfail(strict=True) # TODO: check error
def test_checkclass_project_j(tmpdir):
    __test_checkclass_project(tmpdir, ['-j2', '--no-cppcheck-build-dir'])


def test_checkclass_project_builddir(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass_project(tmpdir, ['-j1', '--cppcheck-build-dir={}'.format(build_dir)])


def test_checkclass_project_builddir_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    __test_checkclass_project(tmpdir, ['-j2', '--cppcheck-build-dir={}'.format(build_dir)])

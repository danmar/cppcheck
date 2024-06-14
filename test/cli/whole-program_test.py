import os
import pytest
import json
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

# TODO: use dedicated addon


def __create_compile_commands(dir, entries):
    j = []
    for e in entries:
        f = os.path.basename(e)
        obj = {
            'directory': os.path.dirname(os.path.abspath(e)),
            'command': 'gcc -c {}'.format(f),
            'file': f
        }
        print(obj)
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


@pytest.mark.xfail(strict=True)
def test_addon_suppress_inline_j():
    __test_addon_suppress_inline(['-j2'])


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


@pytest.mark.xfail(strict=True)
def test_addon_suppress_inline_project(tmpdir):
    __test_addon_suppress_inline_project(tmpdir, ['-j1'])


@pytest.mark.xfail(strict=True)
def test_addon_suppress_inline_project_j(tmpdir):
    __test_addon_suppress_inline_project(tmpdir, ['-j2'])

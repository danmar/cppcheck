
# python -m pytest test-inline-suppress.py

import json
import os
import pytest
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_inline_suppres_path = 'proj-inline-suppress' + os.path.sep


def __create_unused_function_compile_commands(tmpdir):
    prjpath = os.path.realpath(os.path.join(__script_dir, 'proj-inline-suppress-unusedFunction'))
    j = [{'directory': prjpath,
          'command': '/usr/bin/c++ -I"' + prjpath + '" -o "' + os.path.join(prjpath, 'B.cpp.o') + '" -c "' + os.path.join(prjpath, 'B.cpp') + '"',
          'file': os.path.join(prjpath, 'B.cpp')},
         {'directory': prjpath,
          'command': '/usr/bin/c++ -I"' + prjpath + '" -o "' + os.path.join(prjpath, 'A.cpp.o') + '" -c "' + os.path.join(prjpath, 'A.cpp') + '"',
          'file': os.path.join(prjpath, 'A.cpp')}]
    compdb_path = os.path.join(tmpdir, 'proj-inline-suppress-unusedFunction')
    os.makedirs(compdb_path)
    compile_commands = os.path.join(compdb_path, 'compile_commands.json')
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j, indent=4))
    return compile_commands


def test_1():
    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        'proj-inline-suppress'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr == ''
    assert stdout == ''
    assert ret == 0, stdout


def test_2():
    args = [
        '-q',
        '--template=simple',
        'proj-inline-suppress'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}3.cpp:4:19: error: Division by zero. [zerodiv]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


def test_unmatched_suppression():
    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '--enable=information',
        '--error-exitcode=1',
        '{}2.c'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}2.c:2:0: information: Unmatched suppression: some_warning_id [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_unmatched_suppression_path_with_extra_stuff():
    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '--enable=information',
        '--error-exitcode=1',
        '{}2.c'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}2.c:2:0: information: Unmatched suppression: some_warning_id [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_backwards_compatibility():
    args = [
        '-q',
        '--template=simple',
        '{}3.cpp'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == [
        '{}3.cpp:4:19: error: Division by zero. [zerodiv]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout

    args = [
        '-q',
        '--template=simple',
        '--inline-suppr',
        '{}3.cpp'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def __test_compile_commands_unused_function(tmpdir, use_j):
    compdb_file = __create_unused_function_compile_commands(tmpdir)
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--error-exitcode=1',
        '--project={}'.format(compdb_file)
    ]
    if use_j:
        args.append('-j2')
    else:
        args.append('-j1')
    ret, stdout, stderr = cppcheck(args)
    proj_path_sep = os.path.join(__script_dir, 'proj-inline-suppress-unusedFunction') + os.path.sep
    lines = stderr.splitlines()
    assert lines == [
        "{}B.cpp:6:0: style: The function 'unusedFunctionTest' is never used. [unusedFunction]".format(proj_path_sep)
    ]
    assert stdout == ''
    assert ret == 1, stdout


def test_compile_commands_unused_function(tmpdir):
    __test_compile_commands_unused_function(tmpdir, False)


@pytest.mark.skip  # unusedFunction does not work with -j
def test_compile_commands_unused_function_j(tmpdir):
    __test_compile_commands_unused_function(tmpdir, True)


def __test_compile_commands_unused_function_suppression(tmpdir, use_j):
    compdb_file = __create_unused_function_compile_commands(tmpdir)
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        '--error-exitcode=1',
        '--project={}'.format(compdb_file)
    ]
    if use_j:
        args.append('-j2')
    else:
        args.append('-j1')
    ret, stdout, stderr = cppcheck(args)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_compile_commands_unused_function_suppression(tmpdir):
    __test_compile_commands_unused_function_suppression(tmpdir, False)


@pytest.mark.skip  # unusedFunction does not work with -j
def test_compile_commands_unused_function_suppression_j(tmpdir):
    __test_compile_commands_unused_function_suppression(tmpdir, True)


def test_unmatched_suppression_ifdef():
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--inline-suppr',
        '-DNO_ZERO_DIV',
        'trac5704/trac5704a.c'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_unmatched_suppression_ifdef_0():
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--inline-suppr',
        'trac5704/trac5704b.c'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_build_dir(tmpdir):
    args = [
        '-q',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(tmpdir),
        '--enable=all',
        '--inline-suppr',
        '{}4.c'.format(__proj_inline_suppres_path)
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def __test_build_dir_unused_template(tmpdir, use_j):
    args = [
        '-q',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(tmpdir),
        '--enable=all',
        '--inline-suppr',
        '{}template.cpp'.format(__proj_inline_suppres_path)
    ]
    if use_j:
        args.append('-j2')
    else:
        args.append('-j1')

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


def test_build_dir_unused_template(tmpdir):
    __test_build_dir_unused_template(tmpdir, False)


@pytest.mark.xfail(strict=True)
def test_build_dir_unused_template_j(tmpdir):
    __test_build_dir_unused_template(tmpdir, True)


def test_suppress_unmatched_inline_suppression():  # 11172
    args = [
        '-q',
        '--template=simple',
        '--enable=information',
        '--suppress=unmatchedSuppression',
        '--inline-suppr',
        '{}2.c'.format(__proj_inline_suppres_path)
    ]
    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    assert lines == []
    assert stdout == ''
    assert ret == 0, stdout


# reporting of inline unusedFunction is deferred
def __test_unused_function_unmatched(tmpdir, use_j):
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        'proj-inline-suppress/unusedFunctionUnmatched.cpp'
    ]

    if use_j:
        args.append('-j2')
    else:
        args.append('-j1')

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    lines.sort()
    assert lines == [
        '{}unusedFunctionUnmatched.cpp:5:0: information: Unmatched suppression: uninitvar [unmatchedSuppression]'.format(__proj_inline_suppres_path),
        '{}unusedFunctionUnmatched.cpp:5:0: information: Unmatched suppression: unusedFunction [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


def test_unused_function_unmatched(tmpdir):
    __test_unused_function_unmatched(tmpdir, False)


@pytest.mark.skip  # unusedFunction does not work with -j
def test_unused_function_unmatched_j(tmpdir):
    __test_unused_function_unmatched(tmpdir, True)


# reporting of inline unusedFunction is deferred
def __test_unused_function_unmatched_build_dir(tmpdir, extra_args):
    args = [
        '-q',
        '--template=simple',
        '--cppcheck-build-dir={}'.format(tmpdir),
        '--enable=all',
        '--inline-suppr',
        'proj-inline-suppress/unusedFunctionUnmatched.cpp'
    ]

    args = args + extra_args

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    lines = stderr.splitlines()
    lines.sort()
    print(lines)
    assert lines == [
        '{}unusedFunctionUnmatched.cpp:5:0: information: Unmatched suppression: uninitvar [unmatchedSuppression]'.format(__proj_inline_suppres_path),
        '{}unusedFunctionUnmatched.cpp:5:0: information: Unmatched suppression: unusedFunction [unmatchedSuppression]'.format(__proj_inline_suppres_path)
    ]
    assert stdout == ''
    assert ret == 0, stdout


def test_unused_function_unmatched_build_dir(tmpdir):
    __test_unused_function_unmatched_build_dir(tmpdir, ['-j1'])


@pytest.mark.xfail(strict=True)
def test_unused_function_unmatched_build_dir_j(tmpdir):
    __test_unused_function_unmatched_build_dir(tmpdir, ['-j2'])


@pytest.mark.xfail(strict=True)  # no error as inline suppressions are currently not being propagated back
def test_duplicate():
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        'proj-inline-suppress/duplicate.cpp'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == []
    assert stdout.splitlines() == [
        "cppcheck: error: suppression 'unreadVariable' already exists"
    ]
    assert ret == 0, stdout


@pytest.mark.xfail(strict=True)  # no error as inline suppressions are currently not being propagated back
def test_duplicate_cmd():
    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        '--suppress=unreadVariable',
        'proj-inline-suppress/4.c'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == []
    assert stdout.splitlines() == [
        "cppcheck: error: suppression 'unreadVariable' already exists"
    ]
    assert ret == 0, stdout


@pytest.mark.xfail(strict=True)  # no error as inline suppressions are currently not being propagated back
def test_duplicate_file(tmp_path):
    suppr_file = tmp_path / 'suppressions'
    with open(suppr_file, 'wt') as f:
        f.write('unreadVariable')

    args = [
        '-q',
        '--template=simple',
        '--enable=all',
        '--inline-suppr',
        '--suppressions-list={}'.format(suppr_file),
        'proj-inline-suppress/4.c'
    ]

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert stderr.splitlines() == []
    assert stdout.splitlines() == [
        "cppcheck: error: suppression 'unreadVariable' already exists"
    ]
    assert ret == 0, stdout
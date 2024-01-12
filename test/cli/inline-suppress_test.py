
# python -m pytest test-inline-suppress.py

import json
import os
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))


def create_unused_function_compile_commands(tmpdir):
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


def test1():
    ret, stdout, stderr = cppcheck(['--inline-suppr', 'proj-inline-suppress'], cwd=__script_dir)
    assert stderr == ''
    assert ret == 0, stdout


def test2():
    ret, stdout, stderr = cppcheck(['proj-inline-suppress'], cwd=__script_dir)
    assert len(stderr) > 0, stderr
    assert ret == 0, stdout


def test_unmatched_suppression():
    ret, stdout, stderr = cppcheck(['--inline-suppr', '--enable=information', '--error-exitcode=1', 'proj-inline-suppress/2.c'], cwd=__script_dir)
    assert 'Unmatched suppression: some_warning_id' in stderr
    assert ret == 1, stdout


def test_unmatched_suppression_path_with_extra_stuff():
    ret, stdout, stderr = cppcheck(['--inline-suppr', '--enable=information', '--error-exitcode=1', './proj-inline-suppress/2.c'], cwd=__script_dir)
    assert 'Unmatched suppression: some_warning_id' in stderr
    assert ret == 1, stdout


def test_backwards_compatibility():
    ret, stdout, stderr = cppcheck(['proj-inline-suppress/3.cpp'], cwd=__script_dir)
    assert '[zerodiv]' in stderr
    assert ret == 0, stdout

    ret, stdout, stderr = cppcheck(['--inline-suppr', 'proj-inline-suppress/3.cpp'], cwd=__script_dir)
    assert stderr == ''
    assert ret == 0, stdout


def test_compile_commands_unused_function(tmpdir):
    compdb_file = create_unused_function_compile_commands(tmpdir)
    ret, stdout, stderr = cppcheck(['--enable=all', '--error-exitcode=1', '--project={}'.format(compdb_file)])
    assert 'unusedFunction' in stderr
    assert ret == 1, stdout


def test_compile_commands_unused_function_suppression(tmpdir):
    compdb_file = create_unused_function_compile_commands(tmpdir)
    ret, stdout, stderr = cppcheck(['--enable=all', '--inline-suppr', '--error-exitcode=1', '--project={}'.format(compdb_file)])
    assert 'unusedFunction' not in stderr
    assert ret == 0, stdout


def test_unmatched_suppression_ifdef():
    ret, stdout, stderr = cppcheck(['--enable=all', '--suppress=missingIncludeSystem', '--inline-suppr', '-DNO_ZERO_DIV', 'trac5704/trac5704a.c'], cwd=__script_dir)
    assert 'unmatchedSuppression' not in stderr
    assert ret == 0, stdout


def test_unmatched_suppression_ifdef_0():
    ret, stdout, stderr = cppcheck(['--enable=all', '--suppress=missingIncludeSystem', '--inline-suppr', 'trac5704/trac5704b.c'], cwd=__script_dir)
    assert 'unmatchedSuppression' not in stderr
    assert ret == 0, stdout


def test_build_dir(tmpdir):
    args = f'--cppcheck-build-dir={tmpdir} --enable=all --inline-suppr proj-inline-suppress/4.c'.split()

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert len(stderr) == 0, stderr
    assert ret == 0, stdout

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert len(stderr) == 0, stderr
    assert ret == 0, stdout


def test_build_dir_unused_template(tmpdir):
    args = f'--cppcheck-build-dir={tmpdir} --enable=all --inline-suppr proj-inline-suppress/template.cpp'.split()

    ret, stdout, stderr = cppcheck(args, cwd=__script_dir)
    assert len(stderr) == 0, stderr
    assert ret == 0, stdout


def test_suppress_unmatched_inline_suppression(): # 11172
    ret, stdout, stderr = cppcheck(['--enable=all', '--suppress=unmatchedSuppression', '--inline-suppr', 'proj-inline-suppress/2.c'], cwd=__script_dir)
    assert 'unmatchedSuppression' not in stderr
    assert ret == 0, stdout



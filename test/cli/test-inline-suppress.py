
# python -m pytest test-inline-suppress.py

import json
import os
import tempfile
from testutils import cppcheck

def create_unused_function_compile_commands():
    prjpath = os.path.realpath('proj-inline-suppress-unusedFunction')
    compile_commands = os.path.join(prjpath, 'compile_commands.json')
    j = [{'directory': prjpath,
          'command': '/usr/bin/c++ -I"' + prjpath + '" -o "' + os.path.join(prjpath, 'B.cpp.o') + '" -c "' + os.path.join(prjpath, 'B.cpp') + '"',
          'file': os.path.join(prjpath, 'B.cpp')},
         {'directory': prjpath,
          'command': '/usr/bin/c++ -I"' + prjpath + '" -o "' + os.path.join(prjpath, 'A.cpp.o') + '" -c "' + os.path.join(prjpath, 'A.cpp') + '"',
          'file': os.path.join(prjpath, 'A.cpp')}]
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(j, indent=4))

def test1():
    ret, stdout, stderr = cppcheck(['--inline-suppr', 'proj-inline-suppress'])
    assert ret == 0, stdout
    assert stderr == ''

def test2():
    ret, stdout, stderr = cppcheck(['proj-inline-suppress'])
    assert ret == 0, stdout
    assert len(stderr) > 0

def test_unmatched_suppression():
    ret, stdout, stderr = cppcheck(['--inline-suppr', '--enable=information', '--error-exitcode=1', 'proj-inline-suppress/2.c'])
    assert ret == 1
    assert 'Unmatched suppression: some_warning_id' in stderr

def test_unmatched_suppression_path_with_extra_stuf():
    ret, stdout, stderr = cppcheck(['--inline-suppr', '--enable=information', '--error-exitcode=1', './proj-inline-suppress/2.c'])
    assert ret == 1
    assert 'Unmatched suppression: some_warning_id' in stderr

def test_backwards_compatibility():
    ret, stdout, stderr = cppcheck(['proj-inline-suppress/3.cpp'])
    assert '[zerodiv]' in stderr

    ret, stdout, stderr = cppcheck(['--inline-suppr', 'proj-inline-suppress/3.cpp'])
    assert ret == 0, stdout
    assert stderr == ''

def test_compile_commands_unused_function():
    create_unused_function_compile_commands()
    ret, stdout, stderr = cppcheck(['--enable=all', '--error-exitcode=1', '--project=./proj-inline-suppress-unusedFunction/compile_commands.json'])
    assert ret == 1
    assert 'unusedFunction' in stderr

def test_compile_commands_unused_function_suppression():
    create_unused_function_compile_commands()
    ret, stdout, stderr = cppcheck(['--enable=all', '--inline-suppr', '--error-exitcode=1', '--project=./proj-inline-suppress-unusedFunction/compile_commands.json'])
    assert ret == 0, stdout
    assert 'unusedFunction' not in stderr


def test_unmatched_suppression_ifdef():
    ret, stdout, stderr = cppcheck(['--enable=all', '--suppress=missingIncludeSystem', '--inline-suppr', '-DNO_ZERO_DIV', 'trac5704/trac5704a.c'])
    assert ret == 0, stdout
    assert 'unmatchedSuppression' not in stderr


def test_unmatched_suppression_ifdef_0():
    ret, stdout, stderr = cppcheck(['--enable=all', '--suppress=missingIncludeSystem', '--inline-suppr', 'trac5704/trac5704b.c'])
    assert ret == 0, stdout
    assert 'unmatchedSuppression' not in stderr


def test_build_dir():
    with tempfile.TemporaryDirectory() as tempdir:
        args = f'--cppcheck-build-dir={tempdir} --enable=all --inline-suppr proj-inline-suppress/4.c'.split()

        ret, stdout, stderr = cppcheck(args)
        assert ret == 0, stdout
        assert len(stderr) == 0

        ret, stdout, stderr = cppcheck(args)
        assert ret == 0, stdout
        assert len(stderr) == 0

def test_build_dir_unused_template():
    with tempfile.TemporaryDirectory() as tempdir:
        args = f'--cppcheck-build-dir={tempdir} --enable=all --inline-suppr proj-inline-suppress/template.cpp'.split()

        ret, stdout, stderr = cppcheck(args)
        assert ret == 0, stdout
        assert len(stderr) == 0


def test_suppress_unmatched_inline_suppression(): # 11172
    ret, stdout, stderr = cppcheck(['--enable=all', '--suppress=unmatchedSuppression', '--inline-suppr', 'proj-inline-suppress/2.c'])
    assert ret == 0, stdout
    assert 'unmatchedSuppression' not in stderr



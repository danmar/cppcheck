
# python -m pytest test-helloworld.py

import os

from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'duplicates')

def test_project_duplicates():
    args = ['--project=compile_commands_duplicates.json']
    ret, stdout, _ = cppcheck(args, cwd=__proj_dir)
    assert ret == 0
    assert stdout.count('main.cpp') == 1

def test_project_duplicates_recheck():
    args = [
        '--project=compile_commands_duplicates.json',
        '--recheck-project-duplicates'
    ]
    ret, stdout, _ = cppcheck(args, cwd=__proj_dir)
    assert ret == 0
    assert stdout.count('main.cpp') > 1

def test_project_no_duplicates():
    args = ['--project=compile_commands_no_duplicates.json']
    ret, stdout, _ = cppcheck(args, cwd=__proj_dir)
    assert ret == 0
    assert stdout.count('main.cpp') > 1

def test_project_duplicates_builddir(tmpdir):
    args = [
        '--project=compile_commands_duplicates.json',
        f'--cppcheck-build-dir={tmpdir}'
    ]
    ret, _, _ = cppcheck(args, cwd=__proj_dir)
    assert ret == 0
    files = os.listdir(tmpdir)
    assert 'main.a1' in files
    assert 'main.a2' not in files

def test_project_no_duplicates_builddir(tmpdir):
    args = [
        '--project=compile_commands_no_duplicates.json',
        f'--cppcheck-build-dir={tmpdir}'
    ]
    ret, _, _ = cppcheck(args, cwd=__proj_dir)
    assert ret == 0
    files = os.listdir(tmpdir)
    assert 'main.a1' in files
    assert 'main.a2' in files

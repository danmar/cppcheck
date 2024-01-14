
# python3 -m pytest test-unused_function_test.py

import os
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

PROJECT_DIR = os.path.join(__script_dir, 'unusedFunction')


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

# TODO: test with project file
# TODO: test with FileSettings

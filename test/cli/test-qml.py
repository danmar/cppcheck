
# python3 -m pytest test-qml.py

import os
import pytest
from testutils import cppcheck

PROJECT_DIR = 'QML-Samples-TableView'


def test_unused_functions():
    ret, stdout, stderr = cppcheck(['--library=qt', '--enable=unusedFunction', PROJECT_DIR])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert '[unusedFunction]' in stderr
    assert 'fillSampleData' not in stderr


@pytest.mark.xfail
def test_unused_functions_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['--library=qt', '--enable=unusedFunction', '-j2', '--cppcheck-build-dir={}'.format(build_dir), PROJECT_DIR])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert '[unusedFunction]' in stderr
    assert 'fillSampleData' not in stderr

# TODO: test with project file
# TODO: test with FileSettings

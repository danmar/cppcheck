
# python3 -m pytest test-qml.py

import os
import pytest
from testutils import cppcheck

__script_dir = os.path.dirname(os.path.abspath(__file__))

PROJECT_DIR = os.path.join(__script_dir, 'QML-Samples-TableView')


def test_unused_functions():
    ret, stdout, stderr = cppcheck(['-q', '--template=simple', '--library=qt', '--enable=unusedFunction', PROJECT_DIR])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert stderr.splitlines() == [
        "{}/samplemodel.cpp:9:0: style: The function 'rowCount' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/samplemodel.cpp:15:0: style: The function 'data' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/samplemodel.cpp:38:0: style: The function 'roleNames' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout


@pytest.mark.xfail
def test_unused_functions_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['--template=simple', '--library=qt', '--enable=unusedFunction', '-j2', '--cppcheck-build-dir={}'.format(build_dir), PROJECT_DIR])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert stderr.splitlines() == [
        "{}/samplemodel.cpp:9:0: style: The function 'rowCount' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/samplemodel.cpp:15:0: style: The function 'data' is never used. [unusedFunction]".format(PROJECT_DIR),
        "{}/samplemodel.cpp:38:0: style: The function 'roleNames' is never used. [unusedFunction]".format(PROJECT_DIR)
    ]
    assert ret == 0, stdout

# TODO: test with project file
# TODO: test with FileSettings

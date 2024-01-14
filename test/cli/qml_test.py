
# python3 -m pytest test-qml.py

import os
import pytest
from testutils import cppcheck

PROJECT_DIR = 'QML-Samples-TableView'


def test_unused_functions():
    ret, stdout, stderr = cppcheck(['--template=simple', '--library=qt', '--enable=unusedFunction', PROJECT_DIR])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert stderr.splitlines() == [
        "QML-Samples-TableView/samplemodel.cpp:9:0: style: The function 'rowCount' is never used. [unusedFunction]",
        "QML-Samples-TableView/samplemodel.cpp:15:0: style: The function 'data' is never used. [unusedFunction]",
        "QML-Samples-TableView/samplemodel.cpp:38:0: style: The function 'roleNames' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout


@pytest.mark.xfail
def test_unused_functions_j(tmpdir):
    build_dir = os.path.join(tmpdir, 'b1')
    os.mkdir(build_dir)
    ret, stdout, stderr = cppcheck(['--template=simple', '--library=qt', '--enable=unusedFunction', '-j2', '--cppcheck-build-dir={}'.format(build_dir), PROJECT_DIR])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert stderr.splitlines() == [
        "QML-Samples-TableView/samplemodel.cpp:9:0: style: The function 'rowCount' is never used. [unusedFunction]",
        "QML-Samples-TableView/samplemodel.cpp:15:0: style: The function 'data' is never used. [unusedFunction]",
        "QML-Samples-TableView/samplemodel.cpp:38:0: style: The function 'roleNames' is never used. [unusedFunction]"
    ]
    assert ret == 0, stdout

# TODO: test with project file
# TODO: test with FileSettings

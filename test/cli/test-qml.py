
# python3 -m pytest test-qml.py

import os
from testutils import cppcheck

PROJECT_DIR = 'QML-Samples-TableView'

def test_unused_functions():
    ret, stdout, stderr = cppcheck(['--library=qt', '--enable=unusedFunction', PROJECT_DIR])
    # there are unused functions. But fillSampleData is not unused because that is referenced from main.qml
    assert '[unusedFunction]' in stderr
    assert 'fillSampleData' not in stderr

